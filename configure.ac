AC_INIT([varnish-modules], [0.9.1])
AC_COPYRIGHT([Copyright (c) 2016 Varnish Software Group])

AC_PREREQ(2.63)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR(src/vmod_cookie.vcc)
AM_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE([1.11 foreign])
AM_SILENT_RULES([yes])
AM_EXTRA_RECURSIVE_TARGETS([docs])

LT_INIT([dlopen disable-static])
LT_PREREQ([2.2.6])

AX_PTHREAD(,[AC_MSG_ERROR([Could not configure pthreads support])])
AX_CODE_COVERAGE

# Check for rst utilities
AC_CHECK_PROGS(RST2MAN, [rst2man rst2man.py], "no")
if test "x$RST2MAN" = "xno"; then
	AC_MSG_WARN([rst2man not found - not building man pages])
fi
AM_CONDITIONAL(HAVE_RST2MAN, [test "x$RST2MAN" != "xno"])

# Test suite
VMOD_TESTS="$(cd $srcdir/src && echo tests/*/*.vtc)"
AC_SUBST(VMOD_TESTS)

# Will be available after Varnish 5.0
PKG_CHECK_VAR([VARNISHAPI_LIBDIR], [varnishapi], [libdir])
AC_SUBST([VARNISH_LIBRARY_PATH],
	[$VARNISHAPI_LIBDIR:$VARNISHAPI_LIBDIR/varnish])

VARNISH_PREREQ([4.1.4])
VARNISH_VMODS([
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
AC_CHECK_MEMBERS(struct objcore.exp,
  [AC_DEFINE([HAVE_OBJCORE_EXP], [1],
    [Define if exp is present in the objcore struct])],
  [], [#include <cache/cache.h>])
CFLAGS="$save_CFLAGS"

AC_CONFIG_FILES([
	Makefile
	src/Makefile
])
AC_OUTPUT
