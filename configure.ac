AC_INIT([varnish-modules], [0.23.0])
AC_COPYRIGHT([Copyright (c) 2016-2021 Varnish Software Group])

AC_PREREQ(2.63)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR(src/vmod_var.vcc)
AC_CONFIG_HEADERS(config.h)

AM_INIT_AUTOMAKE([1.12.2 foreign])
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
AM_CONDITIONAL(HAVE_RST2MAN, [test "$RST2MAN" != no])

# Will be available after Varnish 5.0
PKG_CHECK_VAR([VARNISHAPI_LIBDIR], [varnishapi], [libdir])
AC_SUBST([VARNISH_LIBRARY_PATH],
	[$VARNISHAPI_LIBDIR:$VARNISHAPI_LIBDIR/varnish])

VARNISH_PREREQ([7.4.0])
VARNISH_VMODS([
	accept
	bodyaccess
	header
	saintmode
	str
	tcp
	var
	vsthrottle
	xkey
])
VARNISH_COUNTERS([xkey])

save_CPPFLAGS=$CPPFLAGS
CPPFLAGS=$VARNISHAPI_CFLAGS

AC_CHECK_HEADERS([cache/cache_varnishd.h])

CPPFLAGS=$save_CPPFLAGS

AC_CHECK_TYPE([struct tcp_info],
	[AC_DEFINE([HAVE_TCP_INFO], [1],
		[Define if struct tcp_info is present])],
	[], [[
		#include <sys/types.h>
		#include <sys/socket.h>
		#include <netinet/tcp.h>
		#include <netinet/in.h>
	]])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
])
AC_OUTPUT
