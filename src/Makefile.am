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

dist_man_MANS = vmod_cookie.3 vmod_header.3 vmod_saintmode.3 vmod_vsthrottle.3 vmod_softpurge.3 vmod_tcp.3 vmod_var.3 vmod_xkey.3

libvmod_cookie_la_LDFLAGS = $(AM_LDFLAGS) -module -export-dynamic -avoid-version -shared
libvmod_cookie_la_SOURCES = vcc_cookie_if.c vcc_cookie_if.h vmod_cookie.c
MAINTAINERCLEANFILES = $(dist_man_MANS)
vcc_cookie_if.c vcc_cookie_if.h vmod_cookie.man.rst: @VMODTOOL@ $(top_srcdir)/src/vmod_cookie.vcc
	@VMODTOOL@ -w ../doc/ -o vcc_cookie_if $(top_srcdir)/src/vmod_cookie.vcc

vmod_cookie.3: ../doc/vmod_cookie.man.rst
	${RST2MAN} $< $@

libvmod_header_la_LDFLAGS = $(AM_LDFLAGS) -module -export-dynamic -avoid-version -shared
libvmod_header_la_SOURCES = vcc_header_if.c vcc_header_if.h vmod_header.c
vcc_header_if.c vcc_header_if.h vmod_header.man.rst: @VMODTOOL@ $(top_srcdir)/src/vmod_header.vcc
	@VMODTOOL@ -w ../doc/ -o vcc_header_if $(top_srcdir)/src/vmod_header.vcc

vmod_header.3: ../doc/vmod_header.man.rst
	${RST2MAN} $< $@

libvmod_vsthrottle_la_LDFLAGS = $(AM_LDFLAGS) -module -export-dynamic -avoid-version -shared
libvmod_vsthrottle_la_SOURCES = vcc_vsthrottle_if.c vcc_vsthrottle_if.h vmod_vsthrottle.c
vcc_vsthrottle_if.c vcc_vsthrottle_if.h vmod_vsthrottle.man.rst: @VMODTOOL@ $(top_srcdir)/src/vmod_vsthrottle.vcc
	@VMODTOOL@ -w ../doc/ -o vcc_vsthrottle_if $(top_srcdir)/src/vmod_vsthrottle.vcc

vmod_vsthrottle.3: ../doc/vmod_vsthrottle.man.rst
	${RST2MAN} $< $@

libvmod_saintmode_la_LDFLAGS = $(AM_LDFLAGS) -module -export-dynamic -avoid-version -shared
libvmod_saintmode_la_SOURCES = vcc_saintmode_if.c vcc_saintmode_if.h vmod_saintmode.c
vcc_saintmode_if.c vcc_saintmode_if.h vmod_saintmode.man.rst: @VMODTOOL@ $(top_srcdir)/src/vmod_saintmode.vcc
	@VMODTOOL@ -w ../doc/ -o vcc_saintmode_if $(top_srcdir)/src/vmod_saintmode.vcc

vmod_saintmode.3: ../doc/vmod_saintmode.man.rst
	${RST2MAN} $< $@

libvmod_softpurge_la_LDFLAGS = $(AM_LDFLAGS) -module -export-dynamic -avoid-version -shared
libvmod_softpurge_la_SOURCES = \
	vcc_softpurge_if.c \
	vcc_softpurge_if.h \
        foreign/hash/hash_slinger.h \
	vmod_softpurge.c
vcc_softpurge_if.c vcc_softpurge_if.h vmod_softpurge.man.rst: @VMODTOOL@ $(top_srcdir)/src/vmod_softpurge.vcc
	@VMODTOOL@ -w ../doc/ -o vcc_softpurge_if $(top_srcdir)/src/vmod_softpurge.vcc

vmod_softpurge.3: ../doc/vmod_softpurge.man.rst
	${RST2MAN} $< $@


libvmod_tcp_la_LDFLAGS = $(AM_LDFLAGS) -module -export-dynamic -avoid-version -shared
libvmod_tcp_la_SOURCES = vcc_tcp_if.c vcc_tcp_if.h vmod_tcp.c
vcc_tcp_if.c vcc_tcp_if.h vmod_tcp.man.rst: @VMODTOOL@ $(top_srcdir)/src/vmod_tcp.vcc
	@VMODTOOL@ -w ../doc/ -o vcc_tcp_if $(top_srcdir)/src/vmod_tcp.vcc

vmod_tcp.3: ../doc/vmod_tcp.man.rst
	${RST2MAN} $< $@

libvmod_var_la_LDFLAGS = $(AM_LDFLAGS) -module -export-dynamic -avoid-version -shared
libvmod_var_la_SOURCES = vcc_var_if.c vcc_var_if.h vmod_var.c
vcc_var_if.c vcc_var_if.h vmod_var.man.rst: @VMODTOOL@ $(top_srcdir)/src/vmod_var.vcc
	@VMODTOOL@ -w ../doc/ -o vcc_var_if $(top_srcdir)/src/vmod_var.vcc

vmod_var.3: ../doc/vmod_var.man.rst
	${RST2MAN} $< $@

libvmod_xkey_la_LDFLAGS = $(AM_LDFLAGS) -module -export-dynamic -avoid-version -shared
libvmod_xkey_la_SOURCES = vcc_xkey_if.c vcc_xkey_if.h vmod_xkey.c
vcc_xkey_if.c vcc_xkey_if.h vmod_xkey.man.rst: @VMODTOOL@ $(top_srcdir)/src/vmod_xkey.vcc
	@VMODTOOL@ -w ../doc/ -o vcc_xkey_if $(top_srcdir)/src/vmod_xkey.vcc

vmod_xkey.3: ../doc/vmod_xkey.man.rst
	${RST2MAN} $< $@


VMOD_TESTS = $(top_srcdir)/src/tests/*/*.vtc
.PHONY: $(VMOD_TESTS)

$(top_srcdir)/src/tests/*/*.vtc:
	@VARNISHTEST@ -Dvmoddir=$(vmoddir) -Dvarnishd=@VARNISHD@ -Dvmod_builddir=$(abs_top_builddir)/src -Dvmod_directors="directors from \"$(vmoddir)/libvmod_directors.so\"" $@

check: $(VMOD_TESTS)

EXTRA_DIST = foreign *vcc $(VMOD_TESTS) \
	vtree.h

CLEANFILES = $(builddir)/vcc*if.c $(builddir)/vcc*if.h \
	$(builddir)/../doc/vmod_*.man.rst \
	$(builddir)/../doc/vmod_*.rst \
	$(dist_man_MANS)
