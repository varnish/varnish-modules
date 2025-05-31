/*-
 * Copyright (c) 2011-2016 Varnish Software
 * Copyright 2019 UPLEX - Nils Goroll Systemoptimierung
 * All rights reserved.
 *
 * Authors: Kristian Lyngstol <kristian@bohemians.org>
 *          Geoffrey <geoffrey.simmons@uplex.de>
 *          Nils Goroll <nils.goroll@uplex.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "vmod_config.h"

#include "vcc_header_if.h"

/*
 * Returns true if the *hdr header is the one pointed to by *hh.
 *
 * FIXME: duplication from varnishd.
 */
static int
header_http_IsHdr(const txt *hh, const char *hdr)
{
	unsigned l;

	Tcheck(*hh);
	AN(hdr);
	l = hdr[0];
	assert(l == strlen(hdr + 1));
	assert(hdr[l] == ':');
	hdr++;
	return (!strncasecmp(hdr, hh->b, l));
}

/*
 * Return true if the hp->hd[u] header matches *hdr and the regex *re
 * matches the content.
 *
 * If re is NULL, content is not tested and as long as it's the right
 * header, a match is returned.
 */
static int
header_http_match(VRT_CTX, const struct http *hp, unsigned u, VCL_REGEX re,
    const char *hdr)
{
	const char *start;
	unsigned l;

	assert(hdr);
	assert(hp);

	Tcheck(hp->hd[u]);
	if (hp->hd[u].b == NULL)
		return (0);

	l = hdr[0];

	if (!header_http_IsHdr(&hp->hd[u], hdr))
		return (0);

	if (re == NULL)
		return (1);

	start = hp->hd[u].b + l;
	while (*start != '\0' && *start == ' ')
		start++;

	if (!*start)
		return (0);
	if (VRT_re_match(ctx, start, re))
		return (1);

	return (0);
}

/*
 * Returns the (first) header named as *hdr that also matches the regular
 * expression re.
 */
static unsigned
header_http_findhdr(VRT_CTX, const struct http *hp, const char *hdr, VCL_REGEX re)
{
        unsigned u;

        for (u = HTTP_HDR_FIRST; u < hp->nhd; u++) {
		if (header_http_match(ctx, hp, u, re, hdr))
			return (u);
        }
        return (0);
}

/*
 * Removes all copies of the header that matches *hdr with content that
 * matches re. Same as http_Unset(), plus regex.
 */
static void
header_http_Unset(VRT_CTX, struct http *hp, const char *hdr, VCL_REGEX re)
{
	unsigned u, v;

	for (v = u = HTTP_HDR_FIRST; u < hp->nhd; u++) {
		if (hp->hd[u].b == NULL)
			continue;
		if (header_http_match(ctx, hp, u, re, hdr))
			continue;
		if (v != u) {
			memcpy(&hp->hd[v], &hp->hd[u], sizeof *hp->hd);
			memcpy(&hp->hdf[v], &hp->hdf[u], sizeof *hp->hdf);
		}
		v++;
	}
	hp->nhd = v;
}

/*
 * Copies all occurrences of *hdr to a destination header *dst_h. Uses
 * vmod_header_append(), so all copies are kept intact.
 *
 * XXX: Not sure I like the idea of iterating a list of headers while
 * XXX: adding to it. It may be correct now, but perhaps not so much in
 * XXX: the future.
 */
static void
header_http_cphdr(VRT_CTX, const struct http *hp, const char *hdr,
    VCL_HEADER dst)
{
        unsigned u;
	const char *p;
	struct strands s;

	s.n = 1;

        for (u = HTTP_HDR_FIRST; u < hp->nhd; u++) {
		if (!header_http_match(ctx, hp, u, NULL, hdr))
			continue;

		p = hp->hd[u].b + hdr[0];
		while (*p == ' ' || *p == '\t')
			p++;
		s.p = &p;
                vmod_append(ctx, dst, &s);
        }
}

VCL_VOID
vmod_append(VRT_CTX, VCL_HEADER hdr, VCL_STRANDS s)
{
	struct http *hp;
	struct strands st[1];
	const char *p[s->n + 2];
	const char *b;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	/* prefix the strand with $hdr_name + space */
	p[0] = hdr->what + 1;
	p[1] = " ";
	AN(memcpy(p + 2, s->p, s->n * sizeof *s->p));
	st->n = s->n + 2;
	st->p = p;

	b = VRT_StrandsWS(ctx->ws, NULL, st);
	if (b == NULL) {
		VRT_fail(ctx, "vmod_header: workspace allocation failure");
		return;
	}

	hp = VRT_selecthttp(ctx, hdr->where);
	http_SetHeader(hp, b);
}

VCL_STRING
vmod_get(VRT_CTX, VCL_HEADER hdr, VCL_REGEX re)
{
	struct http *hp;
	unsigned u;
	const char *p;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	AN(re);

	hp = VRT_selecthttp(ctx, hdr->where);
	u = header_http_findhdr(ctx, hp, hdr->what, re);
	if (u == 0)
		return (NULL);
	p = hp->hd[u].b + hdr->what[0];
	while (*p == ' ' || *p == '\t')
		p++;
	return (p);
}

VCL_VOID
vmod_copy(VRT_CTX, VCL_HEADER src, VCL_HEADER dst)
{
	struct http *src_hp;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	src_hp = VRT_selecthttp(ctx, src->where);
	header_http_cphdr(ctx, src_hp, src->what, dst);
}

VCL_VOID
vmod_remove(VRT_CTX, VCL_HEADER hdr, VCL_REGEX re)
{
	struct http *hp;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	AN(re);

	hp = VRT_selecthttp(ctx, hdr->where);
	header_http_Unset(ctx, hp, hdr->what, re);
}

/* XXX: http_VSLH() and http_VSLH_del() copied from cache_http.c */

static void
http_VSLH(const struct http *hp, unsigned hdr)
{
	int i;

	if (hp->vsl != NULL) {
		assert(VXID_TAG(hp->vsl->wid));
		i = hdr;
		if (i > HTTP_HDR_FIRST)
			i = HTTP_HDR_FIRST;
		i += hp->logtag;
		VSLbt(hp->vsl, (enum VSL_tag_e)i, hp->hd[hdr]);
	}
}

static void
http_VSLH_del(const struct http *hp, unsigned hdr)
{
	int i;

	if (hp->vsl != NULL) {
		/* We don't support unsetting stuff in the first line */
		assert (hdr >= HTTP_HDR_FIRST);
		assert(VXID_TAG(hp->vsl->wid));
		i = (HTTP_HDR_UNSET - HTTP_HDR_METHOD);
		i += hp->logtag;
		VSLbt(hp->vsl, (enum VSL_tag_e)i, hp->hd[hdr]);
	}
}

VCL_VOID
vmod_regsub(VRT_CTX, VCL_HTTP hp, VCL_REGEX re,
    VCL_STRING sub, VCL_BOOL all)
{
	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	CHECK_OBJ_NOTNULL(hp, HTTP_MAGIC);
	AN(re);

	for (unsigned u = HTTP_HDR_FIRST; u < hp->nhd; u++) {
		const char *hdr;
		VCL_STRING rewrite;

		Tcheck(hp->hd[u]);
		hdr = hp->hd[u].b;
		if (!VRT_re_match(ctx, hdr, re))
			continue;
		rewrite = VRT_regsub(ctx, all, hdr, re, sub);
		if (rewrite == hdr)
			continue;
		http_VSLH_del(hp, u);
		hp->hd[u].b = rewrite;
		hp->hd[u].e = strchr(rewrite, '\0');
		http_VSLH(hp, u);
	}
}

/* basically the inverse of VRT_selecthttp() */
static enum gethdr_e
selectwhere(VRT_CTX, VCL_HTTP hp)
{
	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	CHECK_OBJ_NOTNULL(hp, HTTP_MAGIC);

	if (hp == ctx->http_req)
		return (HDR_REQ);
	if (hp == ctx->http_req_top)
		return (HDR_REQ_TOP);
	if (hp == ctx->http_bereq)
		return (HDR_BEREQ);
	if (hp == ctx->http_beresp)
		return (HDR_BERESP);
	if (hp ==  ctx->http_resp)
		return (HDR_RESP);
	WRONG("impossible VCL_HTTP");
}

// XXX would need to know the limit
const struct gethdr_s hdr_null[HDR_BERESP + 1] = {
	[HDR_REQ]	= { HDR_REQ,		"\0"},
	[HDR_REQ_TOP]	= { HDR_REQ_TOP,	"\0"},
	[HDR_RESP]	= { HDR_RESP,		"\0"},
	[HDR_OBJ]	= { HDR_OBJ,		"\0"},
	[HDR_BEREQ]	= { HDR_BEREQ,		"\0"},
	[HDR_BERESP]	= { HDR_BERESP,	"\0"}
};


VCL_HEADER
vmod_dyn(VRT_CTX, VCL_HTTP hp, VCL_STRING name)
{
	// usual assertions are in selectwhere()
	enum gethdr_e where = selectwhere(ctx, hp);
	char *what;
	const char *p;
	struct gethdr_s *hdr;
	size_t l;

	if (name == NULL || *name == '\0')
		return (&hdr_null[where]);

	p = strchr(name, ':');
	if (p != NULL)
		l = p - name;
	else
		l = strlen(name);

	assert(l <= CHAR_MAX);

	hdr = WS_Alloc(ctx->ws, sizeof *hdr);
	what = WS_Alloc(ctx->ws, l + 3);
	if (hdr == NULL || what == NULL) {
		VRT_fail(ctx, "out of workspace");
		// avoid null check in caller
		return (&hdr_null[where]);
	}

	what[0] = (char)l + 1;
	(void) strncpy(&what[1], name, l);
	what[l+1] = ':';
	what[l+2] = '\0';

	hdr->where = where;
	hdr->what = what;
	return (hdr);
}
