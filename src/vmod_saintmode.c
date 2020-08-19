/*-
 * Copyright (c) 2013-2015 Varnish Software
 * All rights reserved.
 *
 * Author: Dag Haavi Finstad <daghf@varnish-software.com>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vmod_config.h"

#include "vsb.h"
#include "vtim.h"

#include "vcc_saintmode_if.h"

static VCL_BOOL v_matchproto_(vdi_healthy_f)
healthy(VRT_CTX, VCL_BACKEND, VCL_TIME *);
static VCL_BACKEND v_matchproto_(vdi_resolve_f)
resolve(VRT_CTX, VCL_BACKEND);

struct trouble {
	unsigned		magic;
#define TROUBLE_MAGIC		0x4211ab21
	uint8_t			digest[DIGEST_LEN];
	double			timeout;
	VTAILQ_ENTRY(trouble)	list;
};

struct vmod_saintmode_saintmode {
	unsigned				magic;
#define VMOD_SAINTMODE_MAGIC			0xa03756e4
	const struct director			*sdir;
	const struct director			*be;
	pthread_mutex_t				mtx;
	unsigned				threshold;
	unsigned				n_trouble;
	VTAILQ_ENTRY(vmod_saintmode_saintmode)	list;
	VTAILQ_HEAD(, trouble)			troublelist;
};

static const struct vdi_methods vmod_saintmode_methods[1] = {{
	.magic =		VDI_METHODS_MAGIC,
	.type =			"saintmode",
	.healthy =		healthy,
	.resolve =		resolve
}};


struct saintmode_objs {
	unsigned				magic;
#define SAINTMODE_OBJS_MAGIC			0x9aa7beec
	VTAILQ_HEAD(, vmod_saintmode_saintmode) sm_list;
};

VCL_BACKEND
vmod_saintmode_backend(VRT_CTX, struct vmod_saintmode_saintmode *sm) {
	CHECK_OBJ_NOTNULL(sm, VMOD_SAINTMODE_MAGIC);
	CHECK_OBJ_NOTNULL(sm->sdir, DIRECTOR_MAGIC);
	(void)ctx;
	return (sm->sdir);
}

static struct vmod_saintmode_saintmode *
find_sm(const struct saintmode_objs *sm_objs,
    const struct director *be) {
	struct vmod_saintmode_saintmode *sm;

	CHECK_OBJ_NOTNULL(sm_objs, SAINTMODE_OBJS_MAGIC);
	CHECK_OBJ_NOTNULL(be, DIRECTOR_MAGIC);

	VTAILQ_FOREACH(sm, &sm_objs->sm_list, list) {
		CHECK_OBJ_NOTNULL(sm, VMOD_SAINTMODE_MAGIC);
		CHECK_OBJ_NOTNULL(sm->be, DIRECTOR_MAGIC);
		if (sm->be == be)
			return (sm);
	}

	return (NULL);
}

VCL_VOID
vmod_blacklist(VRT_CTX, struct vmod_priv *priv, VCL_DURATION expires) {
	struct trouble *tp;
	struct saintmode_objs *sm_objs;
	struct vmod_saintmode_saintmode *sm;

	if (priv->priv == NULL) {
		VSLb(ctx->vsl, SLT_VCL_Error, "saintmode.blacklist(): "
		    "Saintmode is not configured");
		return;
	}

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	if (ctx->method != VCL_MET_BACKEND_RESPONSE) {
		VSLb(ctx->vsl, SLT_VCL_Error, "saintmode.blacklist() called"
		    " outside of vcl_backend_response");
		return;
	}

	CHECK_OBJ_NOTNULL(ctx->bo, BUSYOBJ_MAGIC);
	CAST_OBJ_NOTNULL(sm_objs, priv->priv, SAINTMODE_OBJS_MAGIC);
	sm = find_sm(sm_objs, ctx->bo->director_resp);
	if (!sm) {
		VSLb(ctx->vsl, SLT_VCL_Error, "Error: saintmode.blacklist(): "
		    "Saintmode not configured for this backend.");
		return;
	}

	ALLOC_OBJ(tp, TROUBLE_MAGIC);
	AN(tp);
	memcpy(tp->digest, ctx->bo->digest, sizeof tp->digest);
	tp->timeout = ctx->bo->t_prev + expires;
	pthread_mutex_lock(&sm->mtx);
	VTAILQ_INSERT_HEAD(&sm->troublelist, tp, list);
	sm->n_trouble++;

	VSLb(ctx->vsl, SLT_VCL_Log, "saintmode: object put on blacklist "
	    "for backend %s for %.2f seconds", sm->be->vcl_name, expires);

	pthread_mutex_unlock(&sm->mtx);

}

VCL_STRING
vmod_status(VRT_CTX, struct vmod_priv *priv)
{
	struct saintmode_objs *sm_objs;
	struct vmod_saintmode_saintmode *sm;
	struct vsb vsb[1];

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	CAST_OBJ_NOTNULL(sm_objs, priv->priv, SAINTMODE_OBJS_MAGIC);

	WS_VSB_new(vsb, ctx->ws);
	VSB_cat(vsb, "{\n\t\"saintmode\": [\n");

	VTAILQ_FOREACH(sm, &sm_objs->sm_list, list) {
		CHECK_OBJ_NOTNULL(sm, VMOD_SAINTMODE_MAGIC);
		CHECK_OBJ_NOTNULL(sm->be, DIRECTOR_MAGIC);
		pthread_mutex_lock(&sm->mtx);
		VSB_cat(vsb, "\t\t{ ");
		VSB_printf(vsb, "\"name\": \"%s\", ", sm->sdir->vcl_name);
		VSB_printf(vsb, "\"backend\": \"%s\", ", sm->be->vcl_name);
		VSB_printf(vsb, "\"count\": \"%u\", ", sm->n_trouble);
		VSB_printf(vsb, "\"threshold\": \"%u\" ", sm->threshold);
		VSB_cat(vsb, "}");
		if (VTAILQ_NEXT(sm, list) != NULL)
			VSB_cat(vsb, ",");
		pthread_mutex_unlock(&sm->mtx);
		VSB_cat(vsb, "\n");
	}

	return (WS_VSB_finish(vsb, ctx->ws, NULL));
}

VCL_INT
vmod_saintmode_blacklist_count(VRT_CTX, struct vmod_saintmode_saintmode *sm)
{
	unsigned c = 0;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	CHECK_OBJ_NOTNULL(sm, VMOD_SAINTMODE_MAGIC);
	pthread_mutex_lock(&sm->mtx);
	c = sm->n_trouble;
	pthread_mutex_unlock(&sm->mtx);
	return (c);
}

static unsigned
is_digest_healthy(const struct director *dir,
		  const uint8_t* digest,
		  double t_prev,
		  struct vsl_log* vsl)
{
	struct trouble *tr;
	struct trouble *tr2;
	unsigned retval, bl;
	struct vmod_saintmode_saintmode *sm;
	VTAILQ_HEAD(, trouble)  troublelist;
	double now;

	CHECK_OBJ_NOTNULL(dir, DIRECTOR_MAGIC);
	CAST_OBJ_NOTNULL(sm, dir->priv, VMOD_SAINTMODE_MAGIC);
	CHECK_OBJ_NOTNULL(sm->be, DIRECTOR_MAGIC);

	now = t_prev;
	retval = 1;
	VTAILQ_INIT(&troublelist);
	pthread_mutex_lock(&sm->mtx);
	VTAILQ_FOREACH_SAFE(tr, &sm->troublelist, list, tr2) {
		CHECK_OBJ_NOTNULL(tr, TROUBLE_MAGIC);

		if (tr->timeout < now) {
			VTAILQ_REMOVE(&sm->troublelist, tr, list);
			VTAILQ_INSERT_HEAD(&troublelist, tr, list);
			sm->n_trouble--;
			continue;
		}

		if (digest && !memcmp(tr->digest, digest, sizeof tr->digest)) {
			retval = 0;
			break;
		}
	}

	bl = !retval;
	if (sm->threshold <= sm->n_trouble)
		retval = 0;
	pthread_mutex_unlock(&sm->mtx);

	if (vsl) {
		if (bl)
			VSLb(vsl, SLT_VCL_Log,
			     "saintmode: unhealthy: object blacklisted for "
			     "backend %s", sm->be->vcl_name);
		else if (retval == 0)
			VSLb(vsl, SLT_VCL_Log,
			     "saintmode: unhealthy: hit threshold for "
			     "backend %s", sm->be->vcl_name);
	}

	VTAILQ_FOREACH_SAFE(tr, &troublelist, list, tr2)
		FREE_OBJ(tr);

	return retval;
}

/* All adapted from PHK's saintmode implementation in Varnish 3.0 */
static VCL_BOOL v_matchproto_(vdi_healthy_f)
healthy(VRT_CTX, VCL_BACKEND dir, VCL_TIME *changed)
{
	struct vmod_saintmode_saintmode *sm;
	const struct busyobj *bo;
	unsigned retval;
	const uint8_t* digest;
	double t_prev;
	struct vsl_log* vsl;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	CHECK_OBJ_NOTNULL(dir, DIRECTOR_MAGIC);
	CAST_OBJ_NOTNULL(sm, dir->priv, VMOD_SAINTMODE_MAGIC);
	CHECK_OBJ_NOTNULL(sm->be, DIRECTOR_MAGIC);
	bo = ctx->bo;
	CHECK_OBJ_ORNULL(bo, BUSYOBJ_MAGIC);

	/* Saintmode is disabled, or list is empty */
	if (sm->threshold == 0 || sm->n_trouble == 0)
		return (VRT_Healthy(ctx, sm->be, changed));

	if (!bo) {
		digest = NULL;
		t_prev = VTIM_real();
		vsl = NULL;
	} else {
		digest = bo->digest;
		t_prev = bo->t_prev;
		vsl = ((struct busyobj *)TRUST_ME(bo))->vsl;
	}

	retval = is_digest_healthy(dir, digest, t_prev, vsl);
	return (retval ? VRT_Healthy(ctx, sm->be, changed) : 0);
}

VCL_BOOL
vmod_saintmode_is_healthy(VRT_CTX, struct vmod_saintmode_saintmode *sm)
{
	uint8_t* digest;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	CHECK_OBJ_NOTNULL(sm, VMOD_SAINTMODE_MAGIC);
	CHECK_OBJ_NOTNULL(sm->sdir, DIRECTOR_MAGIC);

	if (ctx->req != NULL) {
		if (ctx->method == VCL_MET_RECV || ctx->method == VCL_MET_HASH)
			digest = NULL;
		else
			digest = ctx->req->digest;

		return  is_digest_healthy(sm->sdir, digest,
					  ctx->req->t_prev, ctx->req->vsl);
	} else
		return healthy(ctx, sm->sdir, NULL);
}

static VCL_BACKEND v_matchproto_(vdi_resolve_f)
resolve(VRT_CTX, VCL_BACKEND dir) {
	struct vmod_saintmode_saintmode *sm;

	CHECK_OBJ_NOTNULL(dir, DIRECTOR_MAGIC);
	CAST_OBJ_NOTNULL(sm, dir->priv, VMOD_SAINTMODE_MAGIC);

	if (!healthy(ctx, dir, NULL))
		return (NULL);

	return (sm->be);
}

VCL_VOID
vmod_saintmode__init(VRT_CTX, struct vmod_saintmode_saintmode **smp,
    const char *vcl_name, struct vmod_priv *priv, VCL_BACKEND be,
    VCL_INT threshold) {
	struct vmod_saintmode_saintmode *sm;
	struct saintmode_objs *sm_objs;
	(void)ctx;

	AN(smp);
	AZ(*smp);
	ALLOC_OBJ(sm, VMOD_SAINTMODE_MAGIC);
	AN(sm);
	*smp = sm;

	sm->threshold = threshold;
	sm->n_trouble = 0;
	AZ(pthread_mutex_init(&sm->mtx, NULL));
	CHECK_OBJ_NOTNULL(be, DIRECTOR_MAGIC);
	sm->be = be;
	VTAILQ_INIT(&sm->troublelist);

	sm->sdir = VRT_AddDirector(ctx, vmod_saintmode_methods, sm, "%s", vcl_name);

	if (!priv->priv) {
		ALLOC_OBJ(sm_objs, SAINTMODE_OBJS_MAGIC);
		AN(sm_objs);
		VTAILQ_INIT(&sm_objs->sm_list);
		priv->priv = sm_objs;
		priv->free = free;
	}

	CAST_OBJ_NOTNULL(sm_objs, priv->priv, SAINTMODE_OBJS_MAGIC);
	VTAILQ_INSERT_TAIL(&sm_objs->sm_list, sm, list);
}

VCL_VOID
vmod_saintmode__fini(struct vmod_saintmode_saintmode **smp) {
	struct trouble *tr, *tr2;
	struct vmod_saintmode_saintmode *sm;

	AN(smp);
	CHECK_OBJ_NOTNULL(*smp, VMOD_SAINTMODE_MAGIC);
	sm = *smp;
	*smp = NULL;

	VTAILQ_FOREACH_SAFE(tr, &sm->troublelist, list, tr2) {
		CHECK_OBJ_NOTNULL(tr, TROUBLE_MAGIC);
		VTAILQ_REMOVE(&sm->troublelist, tr, list);
		FREE_OBJ(tr);
	}

	VRT_DelDirector(&sm->sdir);
	AZ(pthread_mutex_destroy(&sm->mtx));

	/* We can no longer refer to the sm_objs after this
	 * free. Should be fine, as this fini function will be called
	 * first when the VCL is getting unloaded. */
	FREE_OBJ(sm);
}
