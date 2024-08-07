/*-
 * Copyright (c) 2012-2016 Varnish Software
 *
 * Original author: Arianna Aondio <arianna.aondio@varnish-software.com>
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

#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cache/cache_varnishd.h>
#include <vcl.h>

#include "vre.h"
#include "vsb.h"
#include "vsha256.h"
#include "vcc_bodyaccess_if.h"

struct bodyaccess_log_ctx {
	struct vsl_log	*vsl;
	const char	*pfx;
	size_t		len;
};

static int
bodyaccess_log(struct bodyaccess_log_ctx *ctx, const void *ptr, size_t len)
{
	txt txtbody;
	const char *str;
	char *buf;
	size_t size, prefix_len;

	str = ptr;

	if (ctx->len > 0)
		size = ctx->len;
	else
		size = len;
	prefix_len = strlen(ctx->pfx);
	size += prefix_len;

	buf = malloc(size);
	AN(buf);

	while (len > 0) {
		if (len > ctx->len && ctx->len > 0)
			size = ctx->len;
		else
			size = len;

		memcpy(buf, ctx->pfx, prefix_len);
		memcpy(buf + prefix_len, str, size);

		txtbody.b = buf;
		txtbody.e = buf + prefix_len + size;

		VSLbt(ctx->vsl, SLT_Debug, txtbody);

		len -= size;
		str += size;
	}

	free(buf);

	return (0);
}

static int
bodyaccess_bcat_cb(void *priv, unsigned flush, const void *ptr, ssize_t len)
{

	AN(priv);

	(void)flush;
	return (VSB_bcat(priv, ptr, len));
}

static int
bodyaccess_log_cb(void *priv, unsigned flush, const void *ptr, ssize_t len)
{

	AN(priv);

	(void)flush;
	return (bodyaccess_log(priv, ptr, len));
}

static void
bodyaccess_bcat(VRT_CTX, struct vsb *vsb)
{
	int l;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	CHECK_OBJ_NOTNULL(ctx->req, REQ_MAGIC);

	l = VRB_Iterate(ctx->req->wrk, ctx->vsl, ctx->req, bodyaccess_bcat_cb,
	    vsb);
	AZ(VSB_finish(vsb));
	if (l < 0)
		VSLb(ctx->vsl, SLT_VCL_Error,
		    "Iteration on req.body didn't succeed.");
}

VCL_VOID
vmod_hash_req_body(VRT_CTX)
{
	struct vsb *vsb;
	txt txtbody;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	if (!ctx->req->req_body_cached) {
		VSLb(ctx->vsl, SLT_VCL_Error,
		   "Unbuffered req.body");
		return;
	}

	if (ctx->method != VCL_MET_HASH) {
		VSLb(ctx->vsl, SLT_VCL_Error,
		    "hash_req_body can only be used in vcl_hash{}");
		return;
	}

	vsb = VSB_new_auto();
	AN(vsb);

	bodyaccess_bcat(ctx, vsb);
	txtbody.b = VSB_data(vsb);
	txtbody.e = txtbody.b + VSB_len(vsb);
	SHA256_Update(ctx->specific, txtbody.b, txtbody.e - txtbody.b);
	VSLbt(ctx->vsl, SLT_Hash, txtbody);
	VSB_destroy(&vsb);
}

VCL_INT
vmod_len_req_body(VRT_CTX)
{
	uint64_t u;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	CHECK_OBJ_NOTNULL(ctx->req, REQ_MAGIC);

	if (!ctx->req->req_body_cached) {
		VSLb(ctx->vsl, SLT_VCL_Error,
		   "Unbuffered req.body");
		return (-1);
	}

	if (ctx->method != VCL_MET_RECV) {
		VSLb(ctx->vsl, SLT_VCL_Error,
		    "len_req_body can only be used in vcl_recv{}");
		return (-1);
	}

	AZ(ObjGetU64(ctx->req->wrk, ctx->req->body_oc, OA_LEN, &u));
	AZ(u > INT64_MAX);

	return ((VCL_INT)u);
}

VCL_INT
vmod_rematch_req_body(VRT_CTX, VCL_REGEX re)
{
	struct vsb *vsb;
	int i;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	AN(re);

	if (!ctx->req->req_body_cached) {
		VSLb(ctx->vsl, SLT_VCL_Error,
		   "Unbuffered req.body");
		return(-1);
	}

	if (ctx->method != VCL_MET_RECV) {
		VSLb(ctx->vsl, SLT_VCL_Error,
		    "rematch_req_body can be used only in vcl_recv{}");
		return (-1);
	}


	vsb = VSB_new_auto();
	AN(vsb);

	bodyaccess_bcat(ctx, vsb);

	i = VRE_match(re, VSB_data(vsb), VSB_len(vsb), 0, NULL);

	VSB_destroy(&vsb);

	if (i > 0)
		return (1);

	if (i == VRE_ERROR_NOMATCH )
		return (0);

	VSLb(ctx->vsl, SLT_VCL_Error, "Regexp matching returned %d", i);
	return (-1);

}

VCL_VOID
vmod_log_req_body(VRT_CTX, VCL_STRING prefix, VCL_INT length)
{
	struct bodyaccess_log_ctx log_ctx;
	int ret;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	CHECK_OBJ_NOTNULL(ctx->req, REQ_MAGIC);
	AN(ctx->vsl);

	if (!prefix)
		prefix = "";

	log_ctx.vsl = ctx->vsl;
	log_ctx.pfx = prefix;
	log_ctx.len = length;

	if (!ctx->req->req_body_cached) {
		VSLb(ctx->vsl, SLT_VCL_Error, "Unbuffered req.body");
		return;
	}

	ret = VRB_Iterate(ctx->req->wrk, ctx->vsl, ctx->req, bodyaccess_log_cb,
	    &log_ctx);

	if (ret < 0) {
		VSLb(ctx->vsl, SLT_VCL_Error,
		    "Iteration on req.body didn't succeed.");
		return;
	}
}
