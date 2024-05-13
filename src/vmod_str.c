#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cache/cache.h"

#include "vtim.h"
#include "vcc_str_if.h"

VCL_INT
vmod_count(VRT_CTX, VCL_STRING s)
{
	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	if (s == NULL)
		return (-1);
	else
		return (strlen(s));
}


VCL_BOOL
vmod_startswith(VRT_CTX, VCL_STRING s1, VCL_STRING s2)
{
	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	if (s1 == NULL || s2 == NULL)
		return (0);
	while (*s2) {
		if (*s1 != *s2)
			return (0);
		s1++;
		s2++;
	}
	return (1);
}

VCL_BOOL
vmod_endswith(VRT_CTX, VCL_STRING s1, VCL_STRING s2)
{
	const char *p;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	if (s1 == NULL || s2 == NULL)
		return (0);
	p = s1 + strlen(s1) - strlen(s2);

	if (p < s1)
		return (0);

	return (!strcmp(p, s2));
}

VCL_BOOL
vmod_contains(VRT_CTX, VCL_STRING s1, VCL_STRING s2)
{
	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	if (s1 == NULL || s2 == NULL)
		return (0);
	if (strstr(s1, s2))
		return (1);
	else
		return (0);
}

VCL_STRING
vmod_take(VRT_CTX, VCL_STRING s, VCL_INT n, VCL_INT o)
{
	char *p;
	size_t l;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	if (s == NULL)
		return (NULL);

	l = strlen(s);

	if (o < 0 || (o == 0 && n < 0))		/* anchor right?*/
		o += l;

	if (n < 0) {				/* make n positive */
		n = -n;
		o -= n;
	}

	if (o + n < 0 || (o > 0 && o > l) || n == 0)	/* easy special cases*/
		return ("");

	if (o < 0) {				/* clip before string */
		n += o;
		o = 0;
	}
	if (o + n > l)				/* clip after string */
		n = l - o;

	s += o;

	p = WS_Copy(ctx->ws, s, n + 1);
	if (p == NULL)
		VRT_fail(ctx, "str.substr(): Out of workspace");
	else
		p[n] = '\0';
	return (p);
}

VCL_STRING
vmod_reverse(VRT_CTX, VCL_STRING s)
{
	char *p;
	size_t l, l2;
	unsigned u;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	if (s == NULL)
		return (NULL);

	l = l2 = strlen(s);
	u = WS_ReserveSize(ctx->ws, l + 1);
	if (u > 0 && u < l + 1) {
		WS_Release(ctx->ws, 0);
		u = 0;
	}
	if (u == 0) {
		VRT_fail(ctx, "str.reverse(): Out of workspace");
		return (NULL);
	}

	p = ctx->ws->f;
	while(l2) {
		p[l - l2] = s[l2 - 1];
		l2--;
	}
	p[l] ='\0';

	WS_Release(ctx->ws, l + 1);

	return (p);
}

VCL_STRING
vmod_split(VRT_CTX, VCL_STRING s, VCL_INT i, VCL_STRING sep)
{
	const char *lim, *b, *e;
	unsigned fld;
	int inc;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	if (s == NULL || *s == '\0' || i == 0 || sep == NULL || *sep == '\0')
		return (NULL);

	if (i > 0) {
		inc = 1;
		fld = i;
		lim = s + strlen(s);
	} else {
		inc = -1;
		fld = -i;
		lim = s - 1;
		s = lim + strlen(s);
	}

	do {
		while (s != lim && strchr(sep, *s) != NULL)
			s += inc;
		b = s;
		while (s != lim && strchr(sep, *s) == NULL)
			s += inc;
		e = s;
		fld -= (b != e);
	} while (s != lim && fld > 0);

	if (fld > 0)
		return (NULL);

	if (i < 0) {
		s = b;
		b = e + 1;
		e = s + 1;
	}

	assert(b < e);
	s = WS_Printf(ctx->ws, "%.*s", (int)(e - b), b);
	if (s == NULL)
		VRT_fail(ctx, "str.split(): Out of workspace");
	return (s);
}

/* The following function takes a pointer to a char pointer. It will
 * update the char pointer so that it points to the first
 * non-separator character, and then return the number of
 * non-separator characters from this point in the string. This lets
 * the vmod function do its work without touching the input strings,
 * and consume to no workspace.
 */

static int
tokenize(const char **begin, const char *separators)
{
	const char *b;
	int l;

	b = *begin;
	while (*b != '\0' && strchr(separators, *b))
		b++;
	if (*b == '\0') {
		*begin = NULL;
		return (0);
	}
	l = 0;
	while (b[l] != '\0' && !strchr(separators, b[l]))
		l++;
	*begin = b;
	return (l);
}

VCL_BOOL
vmod_token_intersect(VRT_CTX, struct arg_vmod_str_token_intersect *args)
{
	const char * separators;
	const char *token1, *token2;
	int l1, l2;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	if (!args->str1 || !args->str2)
		return (0);

	separators = (args->valid_separators ? args->separators : " ,");
	for (token1 = args->str1; *token1 != '\0'; token1++) {
		l1 = tokenize(&token1, separators);
		if (l1 == 0)
			return (0);
		// we have a token in str1, search for it in str2
		for (token2 = args->str2; *token2 != '\0'; token2++) {
			l2 = tokenize(&token2, separators);
			if (l2 == 0)
				break;
			if (l2 == l1 && 0 == strncmp(token1, token2, l2))
				return (1);
			token2 += l2;
			// did we jump all the way to a null character?
			if (*token2 == '\0')
				break;
		}
		token1 += l1;
		// we might have jumped to the null character.
		if (*token1 == '\0')
			return (0);
	}
	return (0); // not found
}
