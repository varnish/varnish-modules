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
	if (p != NULL)
		p[n] = '\0';
	return (p);
}

VCL_STRING
vmod_reverse(VRT_CTX, VCL_STRING s)
{
	char *p;
	size_t l, l2;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	if (s == NULL)
		return (NULL);

	l = l2 = strlen(s);
	if (l >= WS_ReserveSize(ctx->ws, l + 1)) {
		WS_Release(ctx->ws, 0);
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

static unsigned
isin(char c, const char *set)
{
	const char *p = set;

	while (*p) {
		if (*p == c)
			return (1);
		p++;
	}

	return (0);
}

VCL_STRING
vmod_split(VRT_CTX, VCL_STRING s, VCL_INT i, VCL_STRING sep)
{
	const char *b, *e = s;
	char *p;
	unsigned nomore = 0, n;
	int inc = 1;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	if (s == NULL || sep == NULL || i == 0)
		return (NULL);

	/* depending on the direction, set e to be just left or just right of
	 * the string */
	if (i < 0) {
		inc = -1;
		e += strlen(s);
	} else
		e--;

	while (1) {
		b = e + inc;
		while (isin(*b, sep)) {
			if ((inc > 0 && *b == '\0') || (inc < 0 && b == s))
				return (NULL);
			b += inc;
		}

		e = b + inc;
		while (!isin(*e, sep)) {
			if ((inc > 0 && *e == '\0') || (inc < 0 && e == s)) {
				nomore = 1;
				break;
			}
			e += inc;
		}

		i -= inc;
		if (i == 0)
			break;
		if (nomore)
			return (NULL);
	}

	if (e > b) {
		assert(inc == 1);
		n = e - b;
	} else {
		assert(inc == -1);
		n = b - e;
		b = e + 1;
	}

	p = WS_Copy(ctx->ws, b, n + 1);
	if (p != NULL)
		p[n] = '\0';
	return (p);
}
