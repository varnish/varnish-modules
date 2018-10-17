/*
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
 */

#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vmod_config.h"

#include "vcc_tcp_if.h"

#ifndef TCP_CONGESTION
#define TCP_CONGESTION	13
#endif

#define TCP_CA_NAME_MAX 16

/*
 * Based on the information found here:
 *   http://linuxgazette.net/136/pfeiffer.html
*/
void vmod_dump_info(VRT_CTX) {
	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	if (ctx->req == NULL) {
	    return;
	}
#ifdef HAVE_TCP_INFO
	CHECK_OBJ_NOTNULL(ctx->req, REQ_MAGIC);
	CHECK_OBJ_NOTNULL(ctx->req->sp, SESS_MAGIC);
	AN(ctx->req->sp->fd);

	struct tcp_info tcpinfo;
	socklen_t tlen = sizeof(struct tcp_info);
	if (getsockopt(ctx->req->sp->fd, SOL_TCP, TCP_INFO,
	    (void*)&tcpinfo, &tlen) < 0) {
		VSLb(ctx->vsl, SLT_VCL_Error, "getsockopt() failed");
		return;
	}

	VSLb(ctx->vsl, SLT_VCL_Log,
	    "tcpi: snd_mss=%i rcv_mss=%i lost=%i retrans=%i",
	    tcpinfo.tcpi_snd_mss, tcpinfo.tcpi_rcv_mss,
	    tcpinfo.tcpi_lost, tcpinfo.tcpi_retrans);

	VSLb(ctx->vsl, SLT_VCL_Log,
	    "tcpi2: pmtu=%i rtt=%i rttvar=%i snd_cwnd=%i advmss=%i reordering=%i",
	    tcpinfo.tcpi_pmtu, tcpinfo.tcpi_rtt, tcpinfo.tcpi_rttvar,
	    tcpinfo.tcpi_snd_cwnd, tcpinfo.tcpi_advmss, tcpinfo.tcpi_reordering);
#endif
}

/* TODO: Use a vmod object for these getters. */
VCL_REAL vmod_get_estimated_rtt(VRT_CTX) {
	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	if (ctx->req == NULL) {
	    return(0.0);
	}
#ifdef HAVE_TCP_INFO
	CHECK_OBJ_NOTNULL(ctx->req, REQ_MAGIC);
	CHECK_OBJ_NOTNULL(ctx->req->sp, SESS_MAGIC);
	AN(ctx->req->sp->fd);

	struct tcp_info tcpinfo;
	socklen_t tlen = sizeof(struct tcp_info);
	if (getsockopt(ctx->req->sp->fd, SOL_TCP, TCP_INFO,
	    (void*)&tcpinfo, &tlen) < 0) {
		VSLb(ctx->vsl, SLT_VCL_Error, "getsockopt() failed");
		return(0.0);
	}
	/*
	 * This should really take into account the rtt variance as well,
	 * but I haven't got a clear view of how that would best be done within
	 * the VCL constraints.
	*/
	return (tcpinfo.tcpi_rtt / 1000);
#else
	return (-1.);
#endif
}


// http://sgros.blogspot.com/2012/12/controlling-which-congestion-control.html
// https://fasterdata.es.net/host-tuning/linux/

VCL_INT vmod_congestion_algorithm(VRT_CTX, VCL_STRING new) {
#ifdef HAVE_TCP_INFO
	char strategy[TCP_CA_NAME_MAX + 1];
#endif

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	if (ctx->req == NULL) {
	    return(-1);
	}
#ifdef HAVE_TCP_INFO
	CHECK_OBJ_NOTNULL(ctx->req, REQ_MAGIC);
	CHECK_OBJ_NOTNULL(ctx->req->sp, SESS_MAGIC);
	AN(ctx->req->sp->fd);

	strncpy(strategy, new, TCP_CA_NAME_MAX);
	strategy[TCP_CA_NAME_MAX] = '\0';
	socklen_t l = strlen(strategy);
	if (setsockopt(ctx->req->sp->fd, IPPROTO_TCP, TCP_CONGESTION, strategy, l) < 0) {
		VSLb(ctx->vsl, SLT_VCL_Error,
		    "TCP_CONGESTION setsockopt() for \"%s\" failed.", strategy);
		return(-1);
	}

#  ifndef NDEBUG
	l = TCP_CA_NAME_MAX;
	if (getsockopt(ctx->req->sp->fd, IPPROTO_TCP, TCP_CONGESTION,
	    strategy, &l) < 0) {
		VSLb(ctx->vsl, SLT_VCL_Error, "getsockopt() failed.");
	} else {
		VSLb(ctx->vsl, SLT_VCL_Log, "getsockopt() returned: %s", strategy);
	}
#  endif
	return(0);
#else
	(void)new;
	return (-1);
#endif /* ifdef HAVE_TCP_INFO */
}

/*
 * net.ipv4.tcp_allowed_congestion_control = cubic reno
 * net.ipv4.tcp_available_congestion_control = cubic reno
 * net.ipv4.tcp_congestion_control = cubic
 *
 * */

VCL_VOID
vmod_set_socket_pace(VRT_CTX, VCL_INT rate)
{
#ifndef SO_MAX_PACING_RATE
#define SO_MAX_PACING_RATE 47
#endif

        CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
#ifdef HAVE_TCP_INFO
        int pacerate = rate * 1024;

        if (setsockopt(ctx->req->sp->fd, SOL_SOCKET, SO_MAX_PACING_RATE, &pacerate,
            sizeof(pacerate)) != 0)
                VSLb(ctx->vsl, SLT_VCL_Error, "set_socket_pace(): Error setting pace rate.");
	else
                VSLb(ctx->vsl, SLT_VCL_Log, "vmod-tcp: Socket paced to %jd KB/s.", (intmax_t)rate);

#  ifndef NDEBUG
        int retval;
        unsigned int current_rate = 0;
        socklen_t f = sizeof(current_rate);
        retval = getsockopt(ctx->req->sp->fd, SOL_SOCKET, SO_MAX_PACING_RATE,
                &current_rate, &f);
        VSLb(ctx->vsl, SLT_VCL_Log, "getsockopt() %i %i", retval, current_rate);
#  endif
#else
	(void)rate;
#endif /* ifdef HAVE_TCP_INFO */
}
