#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <cache/cache.h>
#include <miniobj.h>
#include <vqueue.h>

#include "vcc_accept_if.h"

struct vmod_accept_token {
	unsigned				magic;
#define TOKEN_MAGIC				0x1ba7712d
	char					*string;
	size_t					length;
	VTAILQ_ENTRY(vmod_accept_token)		list;
};

struct vmod_accept_rule {
	unsigned				magic;
#define RULE_MAGIC				0x04895617
	char					*fallback;
	VTAILQ_HEAD(, vmod_accept_token)	tokens;
	pthread_rwlock_t			mtx;
};

VCL_VOID v_matchproto_()
vmod_rule__init(VRT_CTX, struct vmod_accept_rule **rulep, const char *vcl_name,
		VCL_STRING fallback)
{
	struct vmod_accept_rule *rule;

	(void)ctx;
	(void)vcl_name;
	ALLOC_OBJ(rule, RULE_MAGIC);
	AN(rule);

	VTAILQ_INIT(&rule->tokens);
	AZ(pthread_rwlock_init(&rule->mtx, NULL));
	if (fallback == NULL)
		REPLACE(rule->fallback, "");
	else
		REPLACE(rule->fallback, fallback);
	*rulep = rule;
}


VCL_VOID
vmod_rule__fini(struct vmod_accept_rule **rulep)
{
	struct vmod_accept_rule *rule;
	struct vmod_accept_token *t, *token2;

	CHECK_OBJ_NOTNULL(*rulep, RULE_MAGIC);

	rule = *rulep;

	VTAILQ_FOREACH_SAFE(t, &rule->tokens, list, token2) {
		VTAILQ_REMOVE(&rule->tokens, t, list);
		free(t->string);
		FREE_OBJ(t);
	}

	AZ(pthread_rwlock_destroy(&rule->mtx));
	free(rule->fallback);
	free(rule);

	*rulep = NULL;
}

static struct vmod_accept_token *
match_token(struct vmod_accept_rule *rule, VCL_STRING s, size_t l)
{
	struct vmod_accept_token *t;

	CHECK_OBJ_NOTNULL(rule, RULE_MAGIC);
	AN(s);
	AN(l);

	VTAILQ_FOREACH(t, &rule->tokens, list) {
		AN(t->string);
		if (l != t->length)
			continue;
		if (!strncasecmp(t->string, s, l))
			break;
	}
	return (t);
}

#define ADD	1
#define REMOVE	0
static void
add_or_remove(struct vmod_accept_rule *rule, VCL_STRING s, unsigned action)
{
	struct vmod_accept_token *t;

	CHECK_OBJ_NOTNULL(rule, RULE_MAGIC);

	if (s == NULL || *s == '\0')
		return;

	AZ(pthread_rwlock_wrlock(&rule->mtx));

	t = match_token(rule, s, strlen(s));

	if (action == ADD && t == NULL) {
		ALLOC_OBJ(t, TOKEN_MAGIC);
		AN(t);
		REPLACE(t->string, s);
		t->length = strlen(s);
		VTAILQ_INSERT_HEAD(&rule->tokens, t, list);
	} else if (action == REMOVE && t != NULL) {
		VTAILQ_REMOVE(&rule->tokens, t, list);
		free(t->string);
		FREE_OBJ(t);
	}

	AZ(pthread_rwlock_unlock(&rule->mtx));
}

VCL_VOID
vmod_rule_add(VRT_CTX, struct vmod_accept_rule *rule, VCL_STRING s)
{
	(void)ctx;

	add_or_remove(rule, s, ADD);
}

VCL_VOID
vmod_rule_remove(VRT_CTX, struct vmod_accept_rule *rule, VCL_STRING s)
{
	(void)ctx;

	add_or_remove(rule, s, REMOVE);
}

enum tok_code {
	TOK_STR,
	TOK_EOS,
	TOK_ERR,
	TOK_COMMA,
	TOK_SEMI,
	TOK_EQ,
	TOK_OWS
};

static enum tok_code
next_token(const char **b, const char **e)
{
	const char *s;

	AN(b);
	AN(*b);
	AN(e);

	s = *b;
	if (isspace(*s)) {
		while (*s && isspace(*s))
			s++;
		*e = s;
		return (TOK_OWS);
	}
	*e = s + 1;

	switch (*s) {
		case '\0': *e = s; return (TOK_EOS);
		case ',' :	   return (TOK_COMMA);
		case ';' :	   return (TOK_SEMI);
		case '=' :	   return (TOK_EQ);
	}

	while (*s != '\0' && *s != ',' && *s != ';' && *s != '=' &&
			!isspace(*s))
		s++;
	*e = s;
	return (TOK_STR);
}

#define NEXT()					\
	do {					\
		AN(*nxtok);			\
		start = *nxtok;			\
		tc = next_token(&start, nxtok);	\
	} while (0)

#define NEXT_AFTER_OWS()		\
	do {				\
		NEXT();			\
		if (tc == TOK_OWS)	\
			NEXT();		\
	} while (0);

#define EXPECT(n)	if (tc != n) {return (2);}

/* 0 all good, got a token
 * 1 reached the end of string
 * 2 parsing error */
static unsigned
parse_accept(const char **b, const char **e, const char **nxtok, double *q)
{
	const char *start;
	unsigned expectq = 1;
	enum tok_code tc;
	char *eod;

	AN(b);
	AN(*b);
	AN(e);
	AN(nxtok);
	AN(q);

	*nxtok = *b;

	NEXT_AFTER_OWS();
	*b = start;
	*e = *nxtok;
	if (tc == TOK_EOS)
		return (1);
	EXPECT(TOK_STR);
	*q = 1;

	/* look for parameters */
	while (1) {
		/* comma and '\0' end the block cleanly, otherwise, we want a
		 * semi-colon */
		NEXT_AFTER_OWS();
		if (tc == TOK_EOS || tc == TOK_COMMA)
			return (0);
		EXPECT(TOK_SEMI);

		NEXT_AFTER_OWS();
		EXPECT(TOK_STR);
		if (*nxtok - start != 1 || *start != 'q')
			expectq = 0;

		NEXT();
		EXPECT(TOK_EQ);

		NEXT();
		EXPECT(TOK_STR);
		if (expectq) {
			/* testing that string starts with 0 or 1 avoids
			 * checking for NAN and INF */
			if ((start[0] != '0' && start[0] != '1') ||
					start[1] == 'x' ||
					start[1] == 'X')
				return (2);
			errno = 0;
			*q = strtod(start, &eod);
			if (errno || *q < 0 || *q > 1)
				return (2);
			*nxtok = eod;
		}
		expectq = 0;
	}
}
#undef NEXT
#undef NEXT_AFTER_OWS
#undef EXPECT

VCL_STRING
vmod_rule_filter(VRT_CTX, struct vmod_accept_rule *rule, VCL_STRING s)
{
	const char *candidate, *normalized, *b, *e, *nxtok = s;
	struct vmod_accept_token *t;
	double q, maxq = 0;
	unsigned r;

	CHECK_OBJ_NOTNULL(rule, RULE_MAGIC);
	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	candidate = rule->fallback;

	AZ(pthread_rwlock_rdlock(&rule->mtx));

	while (s) {
		b = nxtok;
		r = parse_accept(&b, &e, &nxtok, &q);
		if (r == 2)
			candidate = rule->fallback;
		if (r != 0)
			break;

		t = match_token(rule, b, e - b);
		if (t && q > maxq) {
			maxq = q;
			candidate = t->string;
		}
	}

	normalized = WS_Copy(ctx->ws, candidate, -1);
	AN(normalized);

	AZ(pthread_rwlock_unlock(&rule->mtx));

	return (normalized);
}
