AC_INIT([varnish-modules], [0.11.0])
AC_COPYRIGHT([Copyright (c) 2016 Varnish Software Group])

AC_PREREQ(2.63)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR(src/vmod_cookie.vcc)
AC_CONFIG_HEADERS(config.h)

AM_INIT_AUTOMAKE([1.11 foreign])
AM_SILENT_RULES([yes])
AM_EXTRA_RECURSIVE_TARGETS([rst-docs])

LT_INIT([dlopen disable-static])
LT_PREREQ([2.2.6])

AX_PTHREAD(,[AC_MSG_ERROR([Could not configure pthreads support])])
AX_CODE_COVERAGE

# Check for rst utilities
AC_CHECK_PROGS(RST2MAN, [rst2man rst2man.py], "no")
if test "x$RST2MAN" = "xno"; then
	AC_MSG_WARN([rst2man not found - not building man pages])
	RST2MAN=: # no-op
fi

# Will be available after Varnish 5.0
PKG_CHECK_VAR([VARNISHAPI_LIBDIR], [varnishapi], [libdir])
AC_SUBST([VARNISH_LIBRARY_PATH],
	[$VARNISHAPI_LIBDIR:$VARNISHAPI_LIBDIR/varnish])

VARNISH_PREREQ([4.1.4])
VARNISH_VMODS([
	bodyaccess
	cookie
	header
	saintmode
	softpurge
	tcp
	var
	vsthrottle
	xkey
])

save_CFLAGS="$CFLAGS"
CFLAGS="$VARNISHAPI_CFLAGS"
AC_CHECK_MEMBER(struct objcore.exp,
	[AC_DEFINE([HAVE_OBJCORE_EXP], [1],
		[Define if exp is present in the objcore struct])],
	[], [#include <cache/cache.h>])

AC_CHECK_TYPES([req_body_iter_f, objiterate_f],
	[], [], [#include <cache/cache.h>])
CFLAGS="$save_CFLAGS"

AC_CHECK_TYPE(struct tcp_info,
	[AC_DEFINE([HAVE_TCP_INFO], [1],
		[Define if struct tcp_info is present])],
	[], [[
		#include <sys/types.h>
		#include <sys/socket.h>
		#include <netinet/tcp.h>
		#include <netinet/in.h>
	]])

# Test suite
VMOD_TESTS=$(cd $srcdir/src && echo tests/*/*.vtc)
AC_SUBST([VMOD_TESTS])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
])
AC_OUTPUT
