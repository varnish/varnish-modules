/*-
 * Copyright (c) 2012-2016 Varnish Software
 *
 * Author: Lasse Karstensen <lkarsten@varnish-software.com>
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
 *
 * Cookie VMOD that simplifies handling of the Cookie request header.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "vrt.h"
#include "vqueue.h"
#include "cache/cache.h"

#include "vcc_cookie_if.h"

struct cookie {
	unsigned magic;
#define VMOD_COOKIE_ENTRY_MAGIC 0x3BB41543
	char *name;
	char *value;
	VTAILQ_ENTRY(cookie) list;
};

struct whitelist {
	char *name;
	VTAILQ_ENTRY(whitelist) list;
};

struct vmod_cookie {
	unsigned magic;
#define VMOD_COOKIE_MAGIC 0x4EE5FB2E
	VTAILQ_HEAD(, cookie) cookielist;
};

static void
cobj_free(void *p)
{
	struct vmod_cookie *vcp;

	CAST_OBJ_NOTNULL(vcp, p, VMOD_COOKIE_MAGIC);
	FREE_OBJ(vcp);
}

static struct vmod_cookie *
cobj_get(struct vmod_priv *priv) {
	struct vmod_cookie *vcp;

	if (priv->priv == NULL) {
		ALLOC_OBJ(vcp, VMOD_COOKIE_MAGIC);
		AN(vcp);
		VTAILQ_INIT(&vcp->cookielist);
		priv->priv = vcp;
		priv->free = cobj_free;
	} else
		CAST_OBJ_NOTNULL(vcp, priv->priv, VMOD_COOKIE_MAGIC);

	return (vcp);
}

VCL_VOID
vmod_parse(VRT_CTX, struct vmod_priv *priv, VCL_STRING cookieheader) {
	struct vmod_cookie *vcp = cobj_get(priv);

	char *name, *value;
	const char *p, *sep;
	int i = 0;

	if (!cookieheader || *cookieheader == '\0') {
		VSLb(ctx->vsl, SLT_VCL_Log, "cookie: nothing to parse");
		return;
	}

	if (!VTAILQ_EMPTY(&vcp->cookielist)) {
		/* If called twice during the same request, clean out old state */
		vmod_clean(ctx, priv);
	}

	p = cookieheader;
	while (*p != '\0') {
		while (isspace(*p)) p++;
		sep = strchr(p, '=');
		if (sep == NULL)
			break;
		name = strndup(p, pdiff(p, sep));
		p = sep + 1;

		sep = p;
		while (*sep != '\0' && *sep != ';')
			sep++;
		value = strndup(p, pdiff(p, sep));

		vmod_set(ctx, priv, name, value);
		free(name);
		free(value);
		i++;
		if (*sep == '\0')
			break;
		p = sep + 1;
	}
	VSLb(ctx->vsl, SLT_VCL_Log, "cookie: parsed %i cookies.", i);
}

static struct cookie *
find_cookie(struct vmod_cookie *vcp, VCL_STRING name)
{
	struct cookie *cookie;
	VTAILQ_FOREACH(cookie, &vcp->cookielist, list) {
		CHECK_OBJ_NOTNULL(cookie, VMOD_COOKIE_ENTRY_MAGIC);
		if (!strcmp(cookie->name, name))
			break;
	}
	return(cookie);
}

VCL_VOID
vmod_set(VRT_CTX, struct vmod_priv *priv, VCL_STRING name, VCL_STRING value) {
	struct vmod_cookie *vcp = cobj_get(priv);

	/* Empty cookies should be ignored. */
	if (name == NULL || *name == '\0')
		return;
	if (value == NULL || *value == '\0')
		return;

	char *p;
	struct cookie *cookie = find_cookie(vcp, name);
	if (cookie != NULL) {
		p = WS_Printf(ctx->ws, "%s", value);
		if (p == NULL) {
			VSLb(ctx->vsl, SLT_VCL_Log,
					"cookie: Workspace overflow in set()");
		} else
			cookie->value = p;

		return;
	}

	cookie = WS_Alloc(ctx->ws, sizeof(struct cookie));
	if (cookie == NULL) {
		VSLb(ctx->vsl, SLT_VCL_Log,
				"cookie: unable to get storage for cookie");
		return;
	}
	cookie->magic = VMOD_COOKIE_ENTRY_MAGIC;
	cookie->name = WS_Printf(ctx->ws, "%s", name);
	cookie->value = WS_Printf(ctx->ws, "%s", value);
	if (cookie->name == NULL || cookie->value == NULL) {
		VSLb(ctx->vsl, SLT_VCL_Log,
				"cookie: unable to get storage for cookie");
		return;
	}
	VTAILQ_INSERT_TAIL(&vcp->cookielist, cookie, list);
}

VCL_BOOL
vmod_isset(VRT_CTX, struct vmod_priv *priv, const char *name) {
	struct vmod_cookie *vcp = cobj_get(priv);
	(void)ctx;

	if (name == NULL || *name == '\0')
		return(0);

	struct cookie *cookie = find_cookie(vcp, name);

	return (cookie ? 1 : 0);
}

VCL_STRING
vmod_get(VRT_CTX, struct vmod_priv *priv, VCL_STRING name) {
	struct vmod_cookie *vcp = cobj_get(priv);
	(void)ctx;

	if (name == NULL || *name == '\0')
		return(NULL);

	struct cookie *cookie = find_cookie(vcp, name);

	return (cookie ? cookie->value : NULL);
}


VCL_VOID
vmod_delete(VRT_CTX, struct vmod_priv *priv, VCL_STRING name) {
	struct vmod_cookie *vcp = cobj_get(priv);
	(void)ctx;

	if (name == NULL || *name == '\0')
		return;

	struct cookie *cookie = find_cookie(vcp, name);

	if (cookie != NULL)
		VTAILQ_REMOVE(&vcp->cookielist, cookie, list);
}

VCL_VOID
vmod_clean(VRT_CTX, struct vmod_priv *priv) {
	struct vmod_cookie *vcp = cobj_get(priv);
	(void)ctx;

	AN(&vcp->cookielist);
	VTAILQ_INIT(&vcp->cookielist);
}

VCL_VOID
vmod_filter_except(VRT_CTX, struct vmod_priv *priv, VCL_STRING whitelist_s) {
	struct cookie *cookieptr, *safeptr;
	struct vmod_cookie *vcp = cobj_get(priv);
	struct whitelist *whentry, *whsafe;
	char const *p = whitelist_s, *q;
	int whitelisted = 0;

	(void)ctx;

	VTAILQ_HEAD(, whitelist) whitelist_head;
	VTAILQ_INIT(&whitelist_head);

	/* Parse the supplied whitelist. */
	while (p && *p != '\0') {
		while (isspace(*p))
			p++;
		if (*p == '\0')
			break;

		q = p;
		while (*q != '\0' && *q != ',')
			q++;

		if (q == p) {
			p++;
			continue;
		}

		whentry = malloc(sizeof(struct whitelist));
		AN(whentry);
		whentry->name = strndup(p, q-p);
		AN(whentry->name);

		VTAILQ_INSERT_TAIL(&whitelist_head, whentry, list);

		p = q;
		if (*p != '\0')
			p++;
	}

	/* Filter existing cookies that isn't in the whitelist. */
	VTAILQ_FOREACH_SAFE(cookieptr, &vcp->cookielist, list, safeptr) {
		CHECK_OBJ_NOTNULL(cookieptr, VMOD_COOKIE_ENTRY_MAGIC);
		whitelisted = 0;
		VTAILQ_FOREACH(whentry, &whitelist_head, list) {
			if (strcmp(cookieptr->name, whentry->name) == 0) {
				whitelisted = 1;
				break;
			}
		}
		if (!whitelisted) {
			VTAILQ_REMOVE(&vcp->cookielist, cookieptr, list);
		}
	}

	VTAILQ_FOREACH_SAFE(whentry, &whitelist_head, list, whsafe) {
		VTAILQ_REMOVE(&whitelist_head, whentry, list);
		free(whentry->name);
		free(whentry);
	}
}


VCL_STRING
vmod_get_string(VRT_CTX, struct vmod_priv *priv) {
	struct cookie *curr;
	struct vsb *output;
	void *u;
	struct vmod_cookie *vcp = cobj_get(priv);

	output = VSB_new_auto();
	AN(output);

	VTAILQ_FOREACH(curr, &vcp->cookielist, list) {
		CHECK_OBJ_NOTNULL(curr, VMOD_COOKIE_ENTRY_MAGIC);
		AN(curr->name);
		AN(curr->value);
		VSB_printf(output, "%s%s=%s;",
		    (curr == VTAILQ_FIRST(&vcp->cookielist)) ? "" : " ",
		    curr->name, curr->value);
	}
	VSB_finish(output);

	u = WS_Alloc(ctx->ws, VSB_len(output) + 1);
	if (!u) {
		VSLb(ctx->vsl, SLT_VCL_Log, "cookie: Workspace overflow");
		VSB_delete(output);
		return(NULL);
	}

	strcpy(u, VSB_data(output));
	VSB_delete(output);
	return (u);
}


VCL_STRING
vmod_format_rfc1123(VRT_CTX, VCL_TIME ts, VCL_DURATION duration) {
        return VRT_TIME_string(ctx, ts + duration);
}
