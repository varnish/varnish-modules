/*-
 * Copyright (c) 2012-2016 Varnish Software
 *
 * Original author: Tollef Fog Heen <tfheen@varnish-software.com>
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

#include <sys/socket.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "vmod_config.h"

#include "vsa.h"

#include "vcc_var_if.h"

enum VAR_TYPE {
	STRING,
	INT,
	REAL,
	DURATION,
	IP,
	BACKEND
};

struct var {
	unsigned magic;
#define VAR_MAGIC 0x8A21A651
	char *name;
	enum VAR_TYPE type;
	union {
		char *STRING;
		int INT;
		double REAL;
		double DURATION;
		VCL_IP IP;
		VCL_BACKEND BACKEND;
	} value;
	VTAILQ_ENTRY(var) list;
};

struct var_head {
	unsigned magic;
#define VAR_HEAD_MAGIC 0x64F33E2F
	VTAILQ_HEAD(, var) vars;
};

static VTAILQ_HEAD(, var) global_vars = VTAILQ_HEAD_INITIALIZER(global_vars);
static pthread_mutex_t var_list_mtx = PTHREAD_MUTEX_INITIALIZER;

static void
vh_init(struct var_head *vh)
{

	AN(vh);
	memset(vh, 0, sizeof *vh);
	vh->magic = VAR_HEAD_MAGIC;
	VTAILQ_INIT(&vh->vars);
}

static struct var *
vh_get_var(struct var_head *vh, const char *name)
{
	struct var *v;

	AN(vh);
	AN(name);
	VTAILQ_FOREACH(v, &vh->vars, list) {
		CHECK_OBJ_NOTNULL(v, VAR_MAGIC);
		AN(v->name);
		if (strcmp(v->name, name) == 0)
			return v;
	}
	return NULL;
}

static struct var *
vh_get_var_alloc(struct var_head *vh, const char *name, const struct vrt_ctx *ctx)
{
	struct var *v;

	v = vh_get_var(vh, name);

	if (!v) {
		/* Allocate and add */
		v = (struct var*)WS_Alloc(ctx->ws, sizeof(struct var));
		AN(v);
		v->magic = VAR_MAGIC;
		v->name = WS_Copy(ctx->ws, name, -1);
		AN(v->name);
		VTAILQ_INSERT_HEAD(&vh->vars, v, list);
	}
	return v;
}

static void
free_func(VRT_CTX, void *p)
{
	struct var_head *vh;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	CAST_OBJ_NOTNULL(vh, p, VAR_HEAD_MAGIC);
	FREE_OBJ(vh);
}

static const struct vmod_priv_methods priv_task_methods[1] = {{
		.magic = VMOD_PRIV_METHODS_MAGIC,
		.type = "vmod_var_priv_task",
		.fini = free_func
}};

static struct var_head *
get_vh(struct vmod_priv *priv)
{
	struct var_head *vh;

	if (priv->priv == NULL) {
		ALLOC_OBJ(vh, VAR_HEAD_MAGIC);
		priv->priv = vh;
		priv->methods = priv_task_methods;
	} else
		CAST_OBJ_NOTNULL(vh, priv->priv, VAR_HEAD_MAGIC);

	return (vh);
}

VCL_VOID
vmod_set(const struct vrt_ctx *ctx, struct vmod_priv *priv, VCL_STRING name,
    VCL_STRING value)
{
	vmod_set_string(ctx, priv, name, value);
}

VCL_STRING
vmod_get(const struct vrt_ctx *ctx, struct vmod_priv *priv, VCL_STRING name)
{
	return vmod_get_string(ctx, priv, name);
}

VCL_VOID
vmod_set_string(const struct vrt_ctx *ctx, struct vmod_priv *priv,
    VCL_STRING name, VCL_STRING value)
{
	struct var *v;

	if (name == NULL)
		return;
	v = vh_get_var_alloc(get_vh(priv), name, ctx);
	AN(v);
	v->type = STRING;
	if (value == NULL)
		value = "";
	v->value.STRING = WS_Copy(ctx->ws, value, -1);
}

VCL_STRING
vmod_get_string(const struct vrt_ctx *ctx, struct vmod_priv *priv,
    VCL_STRING name)
{
	struct var *v;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	if (name == NULL)
		return (NULL);
	v = vh_get_var(get_vh(priv), name);
	if (!v || v->type != STRING)
		return NULL;
	return (v->value.STRING);
}

VCL_VOID
vmod_set_ip(const struct vrt_ctx *ctx, struct vmod_priv *priv, VCL_STRING name,
    VCL_IP ip)
{
	struct var *v;

	if (name == NULL)
		return;
	v = vh_get_var_alloc(get_vh(priv), name, ctx);
	AN(v);
	v->type = IP;
	AN(ip);
	v->value.IP = WS_Copy(ctx->ws, ip, vsa_suckaddr_len);;
}

VCL_IP
vmod_get_ip(const struct vrt_ctx *ctx, struct vmod_priv *priv, VCL_STRING name)
{
	struct var *v;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	if (name == NULL)
		return (NULL);
	v = vh_get_var(get_vh(priv), name);
	if (!v || v->type != IP)
		return NULL;
	return (v->value.IP);
}

#define VMOD_SET_X(vcl_type_u, vcl_type_l, ctype)	\
VCL_VOID						\
vmod_set_##vcl_type_l(const struct vrt_ctx *ctx, struct vmod_priv *priv,\
    const char *name, ctype value)					\
{									\
	struct var *v;							\
	if (name == NULL)						\
		return;							\
	v = vh_get_var_alloc(get_vh(priv), name, ctx);			\
	AN(v);								\
	v->type = vcl_type_u;						\
	v->value.vcl_type_u = value;					\
}

VMOD_SET_X(INT, int, VCL_INT)
VMOD_SET_X(REAL, real, VCL_REAL)
VMOD_SET_X(DURATION, duration, VCL_DURATION)
VMOD_SET_X(BACKEND, backend, VCL_BACKEND)

#define VMOD_GET_X(vcl_type_u, vcl_type_l, ctype)	\
ctype							\
vmod_get_##vcl_type_l(const struct vrt_ctx *ctx, struct vmod_priv *priv,\
    const char *name)							\
{									\
	struct var *v;							\
									\
	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);				\
									\
	if (name == NULL)						\
		return 0;						\
	v = vh_get_var(get_vh(priv), name);				\
									\
	if (!v || v->type != vcl_type_u)				\
		return 0;						\
	return (v->value.vcl_type_u);					\
}

VMOD_GET_X(INT, int, VCL_INT)
VMOD_GET_X(REAL, real, VCL_REAL)
VMOD_GET_X(DURATION, duration, VCL_DURATION)
VMOD_GET_X(BACKEND, backend, VCL_BACKEND)

VCL_VOID
vmod_clear(const struct vrt_ctx *ctx, struct vmod_priv *priv)
{
	struct var_head *vh;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	vh = get_vh(priv);
	vh_init(vh);
}

VCL_VOID
vmod_global_set(const struct vrt_ctx *ctx, VCL_STRING name, VCL_STRING value)
{
	struct var *v;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	if (name == NULL)
		return;

	AZ(pthread_mutex_lock(&var_list_mtx));
	VTAILQ_FOREACH(v, &global_vars, list) {
		CHECK_OBJ_NOTNULL(v, VAR_MAGIC);
		AN(v->name);
		if (strcmp(v->name, name) == 0)
			break;
	}
	if (v) {
		VTAILQ_REMOVE(&global_vars, v, list);
		free(v->name);
		v->name = NULL;
	} else
		ALLOC_OBJ(v, VAR_MAGIC);
	AN(v);
	v->name = strdup(name);
	AN(v->name);
	VTAILQ_INSERT_HEAD(&global_vars, v, list);
	if (v->type == STRING)
		free(v->value.STRING);
	v->value.STRING = NULL;
	v->type = STRING;
	if (value != NULL)
		v->value.STRING = strdup(value);

	AZ(pthread_mutex_unlock(&var_list_mtx));
}

VCL_STRING
vmod_global_get(const struct vrt_ctx *ctx, VCL_STRING name)
{
	struct var *v;
	const char *r = NULL;

	AZ(pthread_mutex_lock(&var_list_mtx));
	VTAILQ_FOREACH(v, &global_vars, list) {
		CHECK_OBJ_NOTNULL(v, VAR_MAGIC);
		AN(v->name);
		if (strcmp(v->name, name) == 0)
			break;
	}
	if (v && v->value.STRING != NULL) {
		r = WS_Copy(ctx->ws, v->value.STRING, -1);
		AN(r);
	}
	AZ(pthread_mutex_unlock(&var_list_mtx));
	return(r);
}
