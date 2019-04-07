/*-
 * Copyright (c) 2012-2016 Varnish Software
 *
 * Original author: Lasse Karstensen <lkarsten@varnish-software.com>
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
#include "cache/cache.h"
#include "vrt.h"
#include "vcl.h"
#include "hash/hash_slinger.h"

#include "vcc_softpurge_if.h"

void
vmod_softpurge(VRT_CTX)
{
	struct objcore *oc, **ocp;
	struct objhead *oh;
	unsigned spc, nobj, n;
	double now;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	if (ctx->method != VCL_MET_HIT && ctx->method != VCL_MET_MISS) {
		VSLb(ctx->vsl, SLT_VCL_Error, "softpurge() is only "
		    "available in vcl_hit{} and vcl_miss{}");
		return;
	}

	CHECK_OBJ_NOTNULL(ctx->req, REQ_MAGIC);
	CHECK_OBJ_NOTNULL(ctx->req->wrk, WORKER_MAGIC);
	oh = ctx->req->objcore->objhead;
	CHECK_OBJ_NOTNULL(oh, OBJHEAD_MAGIC);
	spc = WS_Reserve(ctx->ws, 0);
	if (spc < sizeof *ocp) {
		WS_Release(ctx->ws, 0);
		VRT_fail(ctx, "insufficient workspace");
		return;
	}

	nobj = 0;
	ocp = (void*)ctx->ws->f;
	now = ctx->req->t_prev;
	Lck_Lock(&oh->mtx);
	assert(oh->refcnt > 0);
#if ! defined HAVE_OBJCORE_HSH_LIST
	VTAILQ_FOREACH(oc, &oh->objcs, list) {
		CHECK_OBJ_NOTNULL(oc, OBJCORE_MAGIC);
		assert(oc->objhead == oh);
		if (oc->flags & OC_F_BUSY)
			continue;
		if (oc->exp_flags & OC_EF_DYING)
			continue;
		if (spc < sizeof *ocp)
			break;
		oc->refcnt++;
		spc -= sizeof *ocp;
		ocp[nobj++] = oc;
	}
#else
	VTAILQ_FOREACH(oc, &oh->objcs, hsh_list) {
		CHECK_OBJ_NOTNULL(oc, OBJCORE_MAGIC);
		assert(oc->objhead == oh);
		if (oc->flags & (OC_F_BUSY|OC_F_DYING))
			continue;
		if (spc < sizeof *ocp)
			break;
		oc->refcnt++;
		spc -= sizeof *ocp;
		ocp[nobj++] = oc;
	}
#endif
	Lck_Unlock(&oh->mtx);

	for (n = 0; n < nobj; n++) {
		oc = ocp[n];
		CHECK_OBJ_NOTNULL(oc, OBJCORE_MAGIC);
#if defined VARNISH_PLUS && defined HAVE_OBJCORE_EXP
		EXP_Rearm(ctx->req->wrk, oc, now, 0, oc->exp.grace, oc->exp.keep);
#elif defined HAVE_OBJCORE_EXP
		EXP_Rearm(oc, now, 0, oc->exp.grace, oc->exp.keep);
#else
		EXP_Rearm(oc, now, 0, oc->grace, oc->keep);
#endif
		(void)HSH_DerefObjCore(ctx->req->wrk, &oc);

	}
	WS_Release(ctx->ws, 0);
}
