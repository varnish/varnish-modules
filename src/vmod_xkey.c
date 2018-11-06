/*-
 * Copyright (c) 2015 Varnish Software
 * All rights reserved.
 *
 * Author: Martin Blix Grydeland <martin@varnish-software.com>
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CACHE_CACHE_VARNISHD_H
#  include <cache/cache_varnishd.h>
#else
#  include "vmod_config.h"
#endif

#include "vsha256.h"
#include "vtree.h"

#include "vcc_xkey_if.h"

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static int n_init = 0;

uintptr_t xkey_cb_handle;

struct xkey_hashkey {
	char				digest[DIGEST_LEN];
	VRBT_ENTRY(xkey_hashkey)		entry;
};
static int xkey_hashcmp(const struct xkey_hashkey *k1,
    const struct xkey_hashkey *k2);
VRBT_HEAD(xkey_hashtree, xkey_hashkey);
static struct xkey_hashtree xkey_hashtree = VRBT_INITIALIZER(&xkey_hashtree);
VRBT_PROTOTYPE(xkey_hashtree, xkey_hashkey, entry, xkey_hashcmp);

struct xkey_ptrkey {
	uintptr_t			ptr;
	VRBT_ENTRY(xkey_ptrkey)		entry;
};
static int xkey_ptrcmp(const struct xkey_ptrkey *k1,
    const struct xkey_ptrkey *k2);
VRBT_HEAD(xkey_octree, xkey_ptrkey);
static struct xkey_octree xkey_octree = VRBT_INITIALIZER(&xkey_octree);
VRBT_PROTOTYPE(xkey_octree, xkey_ptrkey, entry, xkey_ptrcmp)

struct xkey_hashhead;
struct xkey_ochead;
struct xkey_oc;

struct xkey_hashhead {
	struct xkey_hashkey		key;
	unsigned			magic;
#define XKEY_HASHHEAD_MAGIC		0x9553B65C
	VTAILQ_ENTRY(xkey_hashhead)	list;
	VTAILQ_HEAD(,xkey_oc)		ocs;
};

struct xkey_ochead {
	struct xkey_ptrkey		key;
	unsigned			magic;
#define XKEY_OCHEAD_MAGIC		0x1E62445D
	VTAILQ_ENTRY(xkey_ochead)	list;
	VTAILQ_HEAD(,xkey_oc)		ocs;
};

struct xkey_oc {
	unsigned			magic;
#define XKEY_OC_MAGIC			0xC688B0C0
	VTAILQ_ENTRY(xkey_oc)		list_ochead;
	VTAILQ_ENTRY(xkey_oc)		list_hashhead;
	struct xkey_hashhead		*hashhead;
	struct objcore			*objcore;
};

#define POOL_MAX 5
static struct {
	VTAILQ_HEAD(,xkey_hashhead)	hashheads;
	int				n_hashhead;
	VTAILQ_HEAD(,xkey_ochead)	ocheads;
	int				n_ochead;
	VTAILQ_HEAD(,xkey_oc)		ocs;
	int				n_oc;
} xkey_pool = {
	VTAILQ_HEAD_INITIALIZER(xkey_pool.hashheads), 0,
	VTAILQ_HEAD_INITIALIZER(xkey_pool.ocheads), 0,
	VTAILQ_HEAD_INITIALIZER(xkey_pool.ocs), 0
};

/*******************/

VRBT_GENERATE(xkey_hashtree, xkey_hashkey, entry, xkey_hashcmp);
VRBT_GENERATE(xkey_octree, xkey_ptrkey, entry, xkey_ptrcmp);

static int
xkey_hashcmp(const struct xkey_hashkey *k1, const struct xkey_hashkey *k2)
{
	return (memcmp(k1->digest, k2->digest, sizeof(k1->digest)));
}

static int
xkey_ptrcmp(const struct xkey_ptrkey *k1, const struct xkey_ptrkey *k2)
{
	if (k1->ptr < k2->ptr)
		return (-1);
	if (k1->ptr > k2->ptr)
		return (1);
	return (0);
}

static struct xkey_hashhead *
xkey_hashhead_new(void)
{
	struct xkey_hashhead *head;

	if (xkey_pool.n_hashhead > 0) {
		head = VTAILQ_FIRST(&xkey_pool.hashheads);
		CHECK_OBJ_NOTNULL(head, XKEY_HASHHEAD_MAGIC);
		VTAILQ_REMOVE(&xkey_pool.hashheads, head, list);
		xkey_pool.n_hashhead--;
	} else {
		ALLOC_OBJ(head, XKEY_HASHHEAD_MAGIC);
		AN(head);
		VTAILQ_INIT(&head->ocs);
	}
	return (head);
}

static void
xkey_hashhead_delete(struct xkey_hashhead **phead)
{
	struct xkey_hashhead *head;

	head = *phead;
	*phead = NULL;
	CHECK_OBJ_NOTNULL(head, XKEY_HASHHEAD_MAGIC);
	AN(VTAILQ_EMPTY(&head->ocs));
	if (xkey_pool.n_hashhead < POOL_MAX) {
		memset(&head->key, 0, sizeof(head->key));
		VTAILQ_INSERT_HEAD(&xkey_pool.hashheads, head, list);
		xkey_pool.n_hashhead++;
		return;
	}
	FREE_OBJ(head);
}

static struct xkey_ochead *
xkey_ochead_new(void)
{
	struct xkey_ochead *head;

	if (xkey_pool.n_ochead > 0) {
		head = VTAILQ_FIRST(&xkey_pool.ocheads);
		VTAILQ_REMOVE(&xkey_pool.ocheads, head, list);
		xkey_pool.n_ochead--;
	} else {
		ALLOC_OBJ(head, XKEY_OCHEAD_MAGIC);
		AN(head);
		VTAILQ_INIT(&head->ocs);
	}
	return (head);
}

static void
xkey_ochead_delete(struct xkey_ochead **phead)
{
	struct xkey_ochead *head;

	head = *phead;
	*phead = NULL;
	CHECK_OBJ_NOTNULL(head, XKEY_OCHEAD_MAGIC);
	AN(VTAILQ_EMPTY(&head->ocs));
	if (xkey_pool.n_ochead < POOL_MAX) {
		memset(&head->key, 0, sizeof(head->key));
		VTAILQ_INSERT_HEAD(&xkey_pool.ocheads, head, list);
		xkey_pool.n_ochead++;
		return;
	}
	FREE_OBJ(head);
}

static struct xkey_oc *
xkey_oc_new(void)
{
	struct xkey_oc *oc;

	if (xkey_pool.n_oc > 0) {
		oc = VTAILQ_FIRST(&xkey_pool.ocs);
		VTAILQ_REMOVE(&xkey_pool.ocs, oc, list_hashhead);
		xkey_pool.n_oc--;
	} else {
		ALLOC_OBJ(oc, XKEY_OC_MAGIC);
		AN(oc);
	}
	return (oc);
}

static void
xkey_oc_delete(struct xkey_oc **poc)
{
	struct xkey_oc *oc;

	oc = *poc;
	*poc = NULL;
	CHECK_OBJ_NOTNULL(oc, XKEY_OC_MAGIC);
	AZ(oc->objcore);
	if (xkey_pool.n_oc < POOL_MAX) {
		VTAILQ_INSERT_HEAD(&xkey_pool.ocs, oc, list_hashhead);
		xkey_pool.n_oc++;
		return;
	}
	FREE_OBJ(oc);
}

static struct xkey_hashhead *
xkey_hashtree_lookup(const unsigned char *digest, unsigned len)
{
	struct xkey_hashkey key, *pkey;
	struct xkey_hashhead *head = NULL;

	AN(digest);
	assert(len == sizeof(key.digest));
	memcpy(&key.digest, digest, len);
	pkey = VRBT_FIND(xkey_hashtree, &xkey_hashtree, &key);
	if (pkey != NULL)
		CAST_OBJ_NOTNULL(head, (void *)pkey, XKEY_HASHHEAD_MAGIC);
	return (head);
}

static struct xkey_hashhead *
xkey_hashtree_insert(const unsigned char *digest, unsigned len)
{
	struct xkey_hashkey *key;
	struct xkey_hashhead *head;

	AN(digest);
	head = xkey_hashhead_new();
	assert(len == sizeof(head->key.digest));
	memcpy(&head->key.digest, digest, len);
	key = VRBT_INSERT(xkey_hashtree, &xkey_hashtree, &head->key);
	if (key != NULL) {
		xkey_hashhead_delete(&head);
		CAST_OBJ_NOTNULL(head, (void *)key, XKEY_HASHHEAD_MAGIC);
	}
	return (head);
}

static struct xkey_ochead *
xkey_octree_lookup(uintptr_t ptr)
{
	struct xkey_ptrkey key, *pkey;
	struct xkey_ochead *head = NULL;

	AN(ptr);
	key.ptr = ptr;
	pkey = VRBT_FIND(xkey_octree, &xkey_octree, &key);
	if (pkey)
		CAST_OBJ_NOTNULL(head, (void *)pkey, XKEY_OCHEAD_MAGIC);
	return (head);
}

static struct xkey_ochead *
xkey_octree_insert(uintptr_t ptr)
{
	struct xkey_ptrkey *key;
	struct xkey_ochead *head;

	AN(ptr);
	head = xkey_ochead_new();
	head->key.ptr = ptr;
	key = VRBT_INSERT(xkey_octree, &xkey_octree, &head->key);
	if (key != NULL) {
		xkey_ochead_delete(&head);
		CAST_OBJ_NOTNULL(head, (void *)key, XKEY_OCHEAD_MAGIC);
	}
	return (head);
}

static void
xkey_insert(struct objcore *objcore, const unsigned char *digest,
    unsigned len)
{
	struct xkey_ochead *ochead;
	struct xkey_hashhead *hashhead;
	struct xkey_oc *oc;

	CHECK_OBJ_NOTNULL(objcore, OBJCORE_MAGIC);
	AN(digest);
	assert(len == DIGEST_LEN);

	ochead = xkey_octree_insert((uintptr_t)objcore);
	AN(ochead);
	hashhead = xkey_hashtree_insert(digest, len);
	AN(hashhead);
	oc = xkey_oc_new();
	AN(oc);

	VTAILQ_INSERT_TAIL(&ochead->ocs, oc, list_ochead);
	VTAILQ_INSERT_TAIL(&hashhead->ocs, oc, list_hashhead);
	oc->objcore = objcore;
	oc->hashhead = hashhead;
}

static void
xkey_remove(struct xkey_ochead **pochead)
{
	struct xkey_ochead *ochead;
	struct xkey_hashhead *hashhead;
	struct xkey_oc *oc, *oc2;

	ochead = *pochead;
	*pochead = NULL;
	CHECK_OBJ_NOTNULL(ochead, XKEY_OCHEAD_MAGIC);

	VTAILQ_FOREACH_SAFE(oc, &ochead->ocs, list_ochead, oc2) {
		hashhead = oc->hashhead;
		oc->hashhead = NULL;
		VTAILQ_REMOVE(&hashhead->ocs, oc, list_hashhead);
		if (VTAILQ_EMPTY(&hashhead->ocs)) {
			VRBT_REMOVE(xkey_hashtree, &xkey_hashtree,
			    &hashhead->key);
			xkey_hashhead_delete(&hashhead);
		}
		oc->objcore = NULL;
		VTAILQ_REMOVE(&ochead->ocs, oc, list_ochead);
		xkey_oc_delete(&oc);
	}
	AN(VTAILQ_EMPTY(&ochead->ocs));
	VRBT_REMOVE(xkey_octree, &xkey_octree, &ochead->key);
	xkey_ochead_delete(&ochead);
}

static void
xkey_cleanup(void)
{
	struct xkey_hashkey *hashkey;
	struct xkey_hashhead *hashhead;
	struct xkey_ptrkey *ockey;
	struct xkey_ochead *ochead;
	struct xkey_oc *oc;

	VRBT_FOREACH(hashkey, xkey_hashtree, &xkey_hashtree) {
		CAST_OBJ_NOTNULL(hashhead, (void *)hashkey,
		    XKEY_HASHHEAD_MAGIC);
		VTAILQ_CONCAT(&xkey_pool.ocs, &hashhead->ocs, list_ochead);
		VTAILQ_INSERT_HEAD(&xkey_pool.hashheads, hashhead, list);
	}
	VRBT_INIT(&xkey_hashtree);

	VRBT_FOREACH(ockey, xkey_octree, &xkey_octree) {
		CAST_OBJ_NOTNULL(ochead, (void *)ockey, XKEY_OCHEAD_MAGIC);
		VTAILQ_INSERT_HEAD(&xkey_pool.ocheads, ochead, list);
	}
	VRBT_INIT(&xkey_octree);

	while (!VTAILQ_EMPTY(&xkey_pool.hashheads)) {
		hashhead = VTAILQ_FIRST(&xkey_pool.hashheads);
		VTAILQ_REMOVE(&xkey_pool.hashheads, hashhead, list);
		FREE_OBJ(hashhead);
	}

	while (!VTAILQ_EMPTY(&xkey_pool.ocheads)) {
		ochead = VTAILQ_FIRST(&xkey_pool.ocheads);
		VTAILQ_REMOVE(&xkey_pool.ocheads, ochead, list);
		FREE_OBJ(ochead);
	}

	while (!VTAILQ_EMPTY(&xkey_pool.ocs)) {
		oc = VTAILQ_FIRST(&xkey_pool.ocs);
		VTAILQ_REMOVE(&xkey_pool.ocs, oc, list_ochead);
		FREE_OBJ(oc);
	}
}

/**************************/

static unsigned
xkey_tok(const char **b, const char **e)
{
	const char *t;

	AN(b);
	AN(e);

	t = *b;
	AN(t);

	while (*t == ',' || isblank(*t))
		t++;
	*b = t;

	while (*t != '\0' && *t != ',' && !isblank(*t))
		t++;
	*e = t;
	return (*b < *e);
}

static void
xkey_cb_insert(struct worker *wrk, struct objcore *objcore)
{
	SHA256_CTX sha_ctx;
	unsigned char digest[DIGEST_LEN];
	const char hdr_xkey[] = "xkey:";
	const char hdr_h2[] = "X-HashTwo:";
	const char *ep, *sp;

	CHECK_OBJ_NOTNULL(objcore, OBJCORE_MAGIC);

	HTTP_FOREACH_PACK(wrk, objcore, sp) {
		if (strncasecmp(sp, hdr_xkey, sizeof(hdr_xkey) - 1) &&
		    strncasecmp(sp, hdr_h2, sizeof(hdr_h2) - 1))
			continue;
		sp = strchr(sp, ':');
		AN(sp);
		sp++;
		while (xkey_tok(&sp, &ep)) {
			SHA256_Init(&sha_ctx);
			SHA256_Update(&sha_ctx, sp, ep - sp);
			SHA256_Final(digest, &sha_ctx);
			AZ(pthread_mutex_lock(&mtx));
			xkey_insert(objcore, digest, sizeof(digest));
			AZ(pthread_mutex_unlock(&mtx));
			sp = ep;
		}
	}
}

static void
xkey_cb_remove(struct objcore *objcore)
{
	struct xkey_ochead *ochead;

	CHECK_OBJ_NOTNULL(objcore, OBJCORE_MAGIC);

	AZ(pthread_mutex_lock(&mtx));
	ochead = xkey_octree_lookup((uintptr_t)objcore);
	if (ochead != NULL)
		xkey_remove(&ochead);
	AZ(pthread_mutex_unlock(&mtx));
}

#if HAVE_ENUM_EXP_EVENT_E
static void
xkey_cb(struct worker *wrk, struct objcore *objcore,
    enum exp_event_e event, void *priv)
{

	CHECK_OBJ_NOTNULL(wrk, WORKER_MAGIC);
	CHECK_OBJ_NOTNULL(objcore, OBJCORE_MAGIC);
	AZ(priv);

	switch (event) {
	case EXP_INSERT:
	case EXP_INJECT:
		xkey_cb_insert(wrk, objcore);
		break;
	case EXP_REMOVE:
		xkey_cb_remove(objcore);
		break;
	default:
		WRONG("enum exp_event_e");
	}
}
#else
static void
xkey_cb(struct worker *wrk, void *priv, struct objcore *oc, unsigned ev)
{

	CHECK_OBJ_NOTNULL(wrk, WORKER_MAGIC);
	CHECK_OBJ_NOTNULL(oc, OBJCORE_MAGIC);
	AZ(priv);
	AN(ev);

	switch (ev) {
	case OEV_INSERT:
		xkey_cb_insert(wrk, oc);
		break;
	case OEV_EXPIRE:
		xkey_cb_remove(oc);
		break;
	default:
		WRONG("Unexpected event");
	}
}
#endif

/**************************/

static VCL_INT
purge(VRT_CTX, VCL_STRING key, VCL_INT do_soft)
{
	SHA256_CTX sha_ctx;
	unsigned char digest[DIGEST_LEN];
	struct xkey_hashhead *hashhead;
	struct xkey_oc *oc;
	const char *ep, *sp;
	int i = 0;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	CHECK_OBJ_NOTNULL(ctx->req, REQ_MAGIC);
	CHECK_OBJ_NOTNULL(ctx->req->wrk, WORKER_MAGIC);

	if (!key || !*key)
		return (0);
	sp = key;
	AZ(pthread_mutex_lock(&mtx));
	while (xkey_tok(&sp, &ep)) {
		SHA256_Init(&sha_ctx);
		SHA256_Update(&sha_ctx, sp, ep - sp);
		SHA256_Final(digest, &sha_ctx);

		hashhead = xkey_hashtree_lookup(digest, sizeof(digest));
		if (hashhead != NULL) {
			VTAILQ_FOREACH(oc, &hashhead->ocs, list_hashhead) {
				CHECK_OBJ_NOTNULL(oc->objcore, OBJCORE_MAGIC);
				if (oc->objcore->flags & OC_F_BUSY)
					continue;
#if defined HAVE_OBJCORE_EXP
				if (do_soft && oc->objcore->exp.ttl <=
				    (ctx->now - oc->objcore->exp.t_origin))
					continue;
#else
				if (do_soft &&
				    oc->objcore->ttl <= (ctx->now - oc->objcore->t_origin))
					continue;
#endif
#if defined HAVE_OBJCORE_EXP && defined VARNISH_PLUS
				if (do_soft)
					EXP_Rearm(ctx->req->wrk, oc->objcore, ctx->now, 0,
					    oc->objcore->exp.grace, oc->objcore->exp.keep);
				else
					EXP_Rearm(ctx->req->wrk, oc->objcore,
					    oc->objcore->exp.t_origin, 0, 0, 0);
#elif defined HAVE_OBJCORE_EXP
				if (do_soft)
					EXP_Rearm(oc->objcore, ctx->now, 0,
					    oc->objcore->exp.grace, oc->objcore->exp.keep);
				else
					EXP_Rearm(oc->objcore, oc->objcore->exp.t_origin,
					    0, 0, 0);
#else
				if (do_soft)
					EXP_Rearm(oc->objcore, ctx->now, 0,
					    oc->objcore->grace, oc->objcore->keep);
				else
					EXP_Rearm(oc->objcore, oc->objcore->t_origin,
					    0, 0, 0);
#endif
				i++;
			}
		}
		sp = ep;
	}
	AZ(pthread_mutex_unlock(&mtx));

	return (i);
}

VCL_INT
vmod_purge(VRT_CTX, VCL_STRING key)
{
	return (purge(ctx, key, 0));
}

VCL_INT
vmod_softpurge(VRT_CTX, VCL_STRING key)
{
	return (purge(ctx, key, 1));
}

int
vmod_event(VRT_CTX, struct vmod_priv *priv, enum vcl_event_e e)
{
	(void)ctx;
	(void)priv;

	switch (e) {
	case VCL_EVENT_LOAD:
		AZ(pthread_mutex_lock(&mtx));
		if (n_init == 0)
#ifdef HAVE_ENUM_EXP_EVENT_E
			xkey_cb_handle =
			    EXP_Register_Callback(xkey_cb, NULL);
#else
			xkey_cb_handle = ObjSubscribeEvents(xkey_cb, NULL,
			    OEV_INSERT|OEV_EXPIRE);
#endif
		AN(xkey_cb_handle);
		n_init++;
		AZ(pthread_mutex_unlock(&mtx));
		break;
	case VCL_EVENT_DISCARD:
		AZ(pthread_mutex_lock(&mtx));
		assert(n_init > 0);
		n_init--;
		AN(xkey_cb_handle);
		if (n_init == 0) {
			/* Do cleanup */
#ifdef HAVE_ENUM_EXP_EVENT_E
			EXP_Deregister_Callback(&xkey_cb_handle);
#else
			ObjUnsubscribeEvents(&xkey_cb_handle);
#endif
			AZ(xkey_cb_handle);
			xkey_cleanup();
		}
		AZ(pthread_mutex_unlock(&mtx));
		break;
	default:
		break;
	}
	return (0);
}
