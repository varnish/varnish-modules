/*
Cookie VMOD for Varnish.

Simplifies handling of the Cookie request header.

Author: Lasse Karstensen <lasse@varnish-software.com>, July 2012.
*/

#include <stdlib.h>
#include <stdio.h>

#include "vrt.h"
#include "vqueue.h"
#include "cache/cache.h"

#include "vcc_cookie_if.h"

#define MAX_COOKIE_NAME 1024   /* name maxsize */
#define MAX_COOKIE_STRING 4096 /* cookie string maxlength */

struct cookie {
	unsigned magic;
#define VMOD_COOKIE_ENTRY_MAGIC 0x3BB41543
	char *name;
	char *value;
	VTAILQ_ENTRY(cookie) list;
};

struct whitelist {
	char name[MAX_COOKIE_NAME];
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
	CHECK_OBJ_NOTNULL(vcp, VMOD_COOKIE_MAGIC);

	char tokendata[MAX_COOKIE_STRING];
	char *token, *tokstate, *value, *sepindex, *dataptr;

	int i = 0;

	if (!cookieheader || strlen(cookieheader) == 0) {
		VSLb(ctx->vsl, SLT_VCL_Log, "cookie: nothing to parse");
		return;
	}

	VSLb(ctx->vsl, SLT_Debug, "cookie: cookie string is %zu bytes.", strlen(cookieheader));

	if (strlen(cookieheader) >= MAX_COOKIE_STRING) {
		VSLb(ctx->vsl, SLT_VCL_Log, "cookie: cookie string overflowed, abort");
		return;
	}

	if (!VTAILQ_EMPTY(&vcp->cookielist)) {
		/* If called twice during the same request, clean out old state */
		vmod_clean(ctx, priv);
	}

	/* strtok modifies source, fewer surprises. */
	strncpy(tokendata, cookieheader, sizeof(tokendata));
	dataptr = tokendata;

	while (1) {
		token = strtok_r(dataptr, ";", &tokstate);
		dataptr = NULL; /* strtok() wants NULL on subsequent calls. */

		if (token == NULL)
		    break;

		while (token[0] == ' ')
		    token++;

		sepindex = strchr(token, '=');
		if (sepindex == NULL) {
			/* No delimiter, this cookie is invalid. Next! */
			continue;
		}
		value = sepindex + 1;
		*sepindex = '\0';

		VSLb(ctx->vsl, SLT_Debug, "value length is %zu.", strlen(value));
		vmod_set(ctx, priv, token, value);
		i++;
	}
	VSLb(ctx->vsl, SLT_VCL_Log, "cookie: parsed %i cookies.", i);
}


VCL_VOID
vmod_set(VRT_CTX, struct vmod_priv *priv, VCL_STRING name, VCL_STRING value) {
	struct vmod_cookie *vcp = cobj_get(priv);
	CHECK_OBJ_NOTNULL(vcp, VMOD_COOKIE_MAGIC);

	/* Empty cookies should be ignored. */
	if (name == NULL || strlen(name) == 0)
		return;
	if (value == NULL || strlen(value) == 0)
		return;

	if (strlen(name) + 1 >= MAX_COOKIE_NAME) {
		VSLb(ctx->vsl, SLT_VCL_Log, "cookie: cookie string overflowed");
		return;
	}

	struct cookie *cookie;
	VTAILQ_FOREACH(cookie, &vcp->cookielist, list) {
		CHECK_OBJ_NOTNULL(cookie, VMOD_COOKIE_ENTRY_MAGIC);
		if (strcmp(cookie->name, name) == 0) {
			cookie->value = WS_Printf(ctx->ws, "%s", value);
			return;
		}
	}

	struct cookie *newcookie = (struct cookie *)WS_Alloc(ctx->ws, sizeof(struct cookie));
	if (newcookie == NULL) {
		VSLb(ctx->vsl, SLT_VCL_Log, "cookie: unable to get storage for cookie");
		return;
	}
	newcookie->magic = VMOD_COOKIE_ENTRY_MAGIC;
	newcookie->name = WS_Printf(ctx->ws, "%s", name);
	newcookie->value = WS_Printf(ctx->ws, "%s", value);

	VTAILQ_INSERT_TAIL(&vcp->cookielist, newcookie, list);
}

VCL_BOOL
vmod_isset(VRT_CTX, struct vmod_priv *priv, const char *name) {
	struct vmod_cookie *vcp = cobj_get(priv);
	CHECK_OBJ_NOTNULL(vcp, VMOD_COOKIE_MAGIC);
	(void)ctx;

	if (name == NULL || strlen(name) == 0)
		return(0);

	struct cookie *cookie;
	VTAILQ_FOREACH(cookie, &vcp->cookielist, list) {
		CHECK_OBJ_NOTNULL(cookie, VMOD_COOKIE_ENTRY_MAGIC);
		if (strcmp(cookie->name, name) == 0) {
			return 1;
		}
	}
	return 0;
}

VCL_STRING
vmod_get(VRT_CTX, struct vmod_priv *priv, VCL_STRING name) {
	struct vmod_cookie *vcp = cobj_get(priv);
	CHECK_OBJ_NOTNULL(vcp, VMOD_COOKIE_MAGIC);
	(void)ctx;

	if (name == NULL || strlen(name) == 0)
		return(NULL);

	struct cookie *cookie;
	VTAILQ_FOREACH(cookie, &vcp->cookielist, list) {
		CHECK_OBJ_NOTNULL(cookie, VMOD_COOKIE_ENTRY_MAGIC);
		if (strcmp(cookie->name, name) == 0) {
			return (cookie->value);
		}
	}
	return (NULL);
}


VCL_VOID
vmod_delete(VRT_CTX, struct vmod_priv *priv, VCL_STRING name) {
	struct vmod_cookie *vcp = cobj_get(priv);
	CHECK_OBJ_NOTNULL(vcp, VMOD_COOKIE_MAGIC);
	(void)ctx;

	if (name == NULL || strlen(name) == 0)
		return;

	struct cookie *cookie;
	VTAILQ_FOREACH(cookie, &vcp->cookielist, list) {
		CHECK_OBJ_NOTNULL(cookie, VMOD_COOKIE_ENTRY_MAGIC);
		if (strcmp(cookie->name, name) == 0) {
			VTAILQ_REMOVE(&vcp->cookielist, cookie, list);
			/* No way to clean up storage, let ws reclaim do it. */
			break;
		}
	}
}

VCL_VOID
vmod_clean(VRT_CTX, struct vmod_priv *priv) {
	struct vmod_cookie *vcp = cobj_get(priv);
	CHECK_OBJ_NOTNULL(vcp, VMOD_COOKIE_MAGIC);
	(void)ctx;

	AN(&vcp->cookielist);
	VTAILQ_INIT(&vcp->cookielist);
}

VCL_VOID
vmod_filter_except(VRT_CTX, struct vmod_priv *priv, VCL_STRING whitelist_s) {
	char buf[MAX_COOKIE_STRING];
	struct cookie *cookieptr;
	char *tokptr, *saveptr;
	int whitelisted = 0;
	struct vmod_cookie *vcp = cobj_get(priv);
	struct whitelist *whentry;

	(void)ctx;

	VTAILQ_HEAD(, whitelist) whitelist_head;
	VTAILQ_INIT(&whitelist_head);
	CHECK_OBJ_NOTNULL(vcp, VMOD_COOKIE_MAGIC);

	AN(whitelist_s);
	strcpy(buf, whitelist_s);
	tokptr = strtok_r(buf, ",", &saveptr);
	if (!tokptr) return;

	/* Parse the supplied whitelist. */
	while (1) {
		whentry = malloc(sizeof(struct whitelist));
		AN(whentry);
		strcpy(whentry->name, tokptr);
		VTAILQ_INSERT_TAIL(&whitelist_head, whentry, list);
		tokptr = strtok_r(NULL, ",", &saveptr);
		if (!tokptr) break;
	}

	/* Filter existing cookies that isn't in the whitelist. */
	VTAILQ_FOREACH(cookieptr, &vcp->cookielist, list) {
		CHECK_OBJ_NOTNULL(cookieptr, VMOD_COOKIE_ENTRY_MAGIC);
		whitelisted = 0;
		VTAILQ_FOREACH(whentry, &whitelist_head, list) {
			if (strlen(cookieptr->name) == strlen(whentry->name) &&
			    strcmp(cookieptr->name, whentry->name) == 0) {
				whitelisted = 1;
				break;
			}
		}
		if (!whitelisted) {
			VTAILQ_REMOVE(&vcp->cookielist, cookieptr, list);
		}
	}

	VTAILQ_FOREACH(whentry, &whitelist_head, list) {
		VTAILQ_REMOVE(&whitelist_head, whentry, list);
		free(whentry);
	}
}


VCL_STRING
vmod_get_string(VRT_CTX, struct vmod_priv *priv) {
	struct cookie *curr;
	struct vsb *output;
	void *u;
	struct vmod_cookie *vcp = cobj_get(priv);
	CHECK_OBJ_NOTNULL(vcp, VMOD_COOKIE_MAGIC);

	output = VSB_new_auto();
	AN(output);

	VTAILQ_FOREACH(curr, &vcp->cookielist, list) {
		CHECK_OBJ_NOTNULL(curr, VMOD_COOKIE_ENTRY_MAGIC);
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

