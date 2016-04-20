AM_CPPFLAGS = @VMOD_INCLUDES@ -I$(top_srcdir)/src/foreign -Wall -Werror -Wunused-parameter -Wmissing-prototypes

vmoddir = @VMOD_DIR@
vmod_LTLIBRARIES = libvmod_cookie.la \
	libvmod_header.la \
	libvmod_saintmode.la \
	libvmod_softpurge.la \
	libvmod_tcp.la \
	libvmod_var.la \
	libvmod_vsthrottle.la \
	libvmod_xkey.la


libvmod_cookie_la_LDFLAGS = $(AM_LDFLAGS) -module -export-dynamic -avoid-version -shared
libvmod_cookie_la_SOURCES = vcc_cookie_if.c vcc_cookie_if.h vmod_cookie.c
vcc_cookie_if.c vcc_cookie_if.h: @VMODTOOL@ $(top_srcdir)/src/vmod_cookie.vcc
	@VMODTOOL@ -w ../docs/ -o vcc_cookie_if $(top_srcdir)/src/vmod_cookie.vcc

libvmod_header_la_LDFLAGS = $(AM_LDFLAGS) -module -export-dynamic -avoid-version -shared
libvmod_header_la_SOURCES = vcc_header_if.c vcc_header_if.h vmod_header.c
vcc_header_if.c vcc_header_if.h: @VMODTOOL@ $(top_srcdir)/src/vmod_header.vcc
	@VMODTOOL@ -w ../docs/ -o vcc_header_if $(top_srcdir)/src/vmod_header.vcc

libvmod_vsthrottle_la_LDFLAGS = $(AM_LDFLAGS) -module -export-dynamic -avoid-version -shared
libvmod_vsthrottle_la_SOURCES = vcc_vsthrottle_if.c vcc_vsthrottle_if.h vmod_vsthrottle.c
vcc_vsthrottle_if.c vcc_vsthrottle_if.h: @VMODTOOL@ $(top_srcdir)/src/vmod_vsthrottle.vcc
	@VMODTOOL@ -w ../docs/ -o vcc_vsthrottle_if $(top_srcdir)/src/vmod_vsthrottle.vcc

libvmod_saintmode_la_LDFLAGS = $(AM_LDFLAGS) -module -export-dynamic -avoid-version -shared
libvmod_saintmode_la_SOURCES = vcc_saintmode_if.c vcc_saintmode_if.h vmod_saintmode.c
vcc_saintmode_if.c vcc_saintmode_if.h: @VMODTOOL@ $(top_srcdir)/src/vmod_saintmode.vcc
	@VMODTOOL@ -w ../docs/ -w ../docs/ -o vcc_saintmode_if $(top_srcdir)/src/vmod_saintmode.vcc

libvmod_softpurge_la_LDFLAGS = $(AM_LDFLAGS) -module -export-dynamic -avoid-version -shared
libvmod_softpurge_la_SOURCES = \
	vcc_softpurge_if.c \
	vcc_softpurge_if.h \
        foreign/hash/hash_slinger.h \
	vmod_softpurge.c
vcc_softpurge_if.c vcc_softpurge_if.h: @VMODTOOL@ $(top_srcdir)/src/vmod_softpurge.vcc
	@VMODTOOL@ -w ../docs/ -o vcc_softpurge_if $(top_srcdir)/src/vmod_softpurge.vcc

libvmod_tcp_la_LDFLAGS = $(AM_LDFLAGS) -module -export-dynamic -avoid-version -shared
libvmod_tcp_la_SOURCES = vcc_tcp_if.c vcc_tcp_if.h vmod_tcp.c
vcc_tcp_if.c vcc_tcp_if.h: @VMODTOOL@ $(top_srcdir)/src/vmod_tcp.vcc
	@VMODTOOL@ -w ../docs/ -o vcc_tcp_if $(top_srcdir)/src/vmod_tcp.vcc

libvmod_var_la_LDFLAGS = $(AM_LDFLAGS) -module -export-dynamic -avoid-version -shared
libvmod_var_la_SOURCES = vcc_var_if.c vcc_var_if.h vmod_var.c
vcc_var_if.c vcc_var_if.h: @VMODTOOL@ $(top_srcdir)/src/vmod_var.vcc
	@VMODTOOL@ -w ../docs/ -o vcc_var_if $(top_srcdir)/src/vmod_var.vcc


libvmod_xkey_la_LDFLAGS = $(AM_LDFLAGS) -module -export-dynamic -avoid-version -shared
libvmod_xkey_la_SOURCES = vcc_xkey_if.c vcc_xkey_if.h vmod_xkey.c
vcc_xkey_if.c vcc_xkey_if.h: @VMODTOOL@ $(top_srcdir)/src/vmod_xkey.vcc
	@VMODTOOL@ -w ../docs/ -o vcc_xkey_if $(top_srcdir)/src/vmod_xkey.vcc



VMOD_TESTS = $(top_srcdir)/src/tests/*/*.vtc
.PHONY: $(VMOD_TESTS)

$(top_srcdir)/src/tests/*/*.vtc:
	PATH=@LIBVARNISHAPI_SBINDIR@:$$PATH \
	@VARNISHTEST@ -Dvmoddir=$(vmoddir) -Dvmod_builddir=$(abs_top_builddir)/src -Dvmod_directors="directors" $@

check: $(VMOD_TESTS)

EXTRA_DIST = foreign *vcc $(VMOD_TESTS) \
	vtree.h

CLEANFILES = $(builddir)/vcc*if.c $(builddir)/vcc*if.h \
	$(builddir)/../docs/vmod_*.man.rst \
	$(builddir)/../docs/vmod_*.rst
