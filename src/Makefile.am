AM_CFLAGS = $(VARNISHAPI_CFLAGS) $(CODE_COVERAGE_CFLAGS) -I$(top_srcdir)/src/foreign \
	-Wall \
	-Werror \
	-Wall \
	-Wno-format-y2k \
	-Wstrict-prototypes \
	-Wmissing-prototypes \
	-Werror=missing-field-initializers \
	-Wpointer-arith \
	-Wreturn-type \
	-Wwrite-strings \
	-Wcast-qual \
	-Wswitch \
	-Wshadow \
	-Wunused-parameter \
	-Wcast-align \
	-Wchar-subscripts \
	-Wnested-externs \
	-Wextra \
	-Wno-sign-compare

AM_CPPFLAGS = $(CODE_COVERAGE_CPPFLAGS)
AM_LDFLAGS = $(VARNISHAPI_LIBS) $(VMOD_LDFLAGS) $(CODE_COVERAGE_LDFLAGS)

vmod_LTLIBRARIES = \
	libvmod_bodyaccess.la \
	libvmod_cookie.la \
	libvmod_header.la \
	libvmod_saintmode.la \
	libvmod_tcp.la \
	libvmod_var.la \
	libvmod_vsthrottle.la \
	libvmod_xkey.la

if HAVE_RST2MAN
dist_man_MANS = \
	vmod_bodyaccess.3 \
	vmod_cookie.3 \
	vmod_header.3 \
	vmod_saintmode.3 \
	vmod_tcp.3 \
	vmod_var.3 \
	vmod_vsthrottle.3 \
	vmod_xkey.3
endif

generated_docs = $(dist_man_MANS:.3=.rst)

libvmod_bodyaccess_la_SOURCES = vmod_bodyaccess.c
libvmod_cookie_la_SOURCES = vmod_cookie.c
libvmod_header_la_SOURCES = vmod_header.c
libvmod_vsthrottle_la_SOURCES = vmod_vsthrottle.c
libvmod_saintmode_la_SOURCES = vmod_saintmode.c
libvmod_tcp_la_SOURCES = vmod_tcp.c
libvmod_var_la_SOURCES = vmod_var.c
libvmod_xkey_la_SOURCES = vmod_xkey.c

nodist_libvmod_bodyaccess_la_SOURCES = vcc_bodyaccess_if.c vcc_bodyaccess_if.h
nodist_libvmod_cookie_la_SOURCES = vcc_cookie_if.c vcc_cookie_if.h
nodist_libvmod_header_la_SOURCES = vcc_header_if.c vcc_header_if.h
nodist_libvmod_vsthrottle_la_SOURCES = vcc_vsthrottle_if.c vcc_vsthrottle_if.h
nodist_libvmod_saintmode_la_SOURCES = vcc_saintmode_if.c vcc_saintmode_if.h
nodist_libvmod_tcp_la_SOURCES = vcc_tcp_if.c vcc_tcp_if.h
nodist_libvmod_var_la_SOURCES = vcc_var_if.c vcc_var_if.h
nodist_libvmod_xkey_la_SOURCES = vcc_xkey_if.c vcc_xkey_if.h

@BUILD_VMOD_BODYACCESS@
@BUILD_VMOD_COOKIE@
@BUILD_VMOD_HEADER@
@BUILD_VMOD_SAINTMODE@
@BUILD_VMOD_SOFTPURGE@
@BUILD_VMOD_TCP@
@BUILD_VMOD_VAR@
@BUILD_VMOD_VSTHROTTLE@
@BUILD_VMOD_XKEY@

if WITH_VMOD_SOFTPURGE
vmod_LTLIBRARIES += libvmod_softpurge.la
if HAVE_RST2MAN
dist_man_MANS += vmod_softpurge.3
endif #HAVE_RST2MAN
libvmod_softpurge_la_SOURCES = vmod_softpurge.c foreign/hash/hash_slinger.h
nodist_libvmod_softpurge_la_SOURCES = vcc_softpurge_if.c vcc_softpurge_if.h
endif #WITH_VMOD_SOFTPURGE

rst-docs: $(generated_docs)
	cp $(generated_docs) $(top_srcdir)/docs/

doctest:
	for i in *.vcc; do \
		../test-vclsnippet $$i; \
		rst2html --halt=2 --exit-status=2 $$i 1>/dev/null; \
	done;

AM_TESTS_ENVIRONMENT = \
	PATH="$(VMOD_TEST_PATH)" \
	LD_LIBRARY_PATH="$(VARNISH_LIBRARY_PATH)"
TEST_EXTENSIONS = .vtc
VTC_LOG_COMPILER = varnishtest -v
AM_VTC_LOG_FLAGS = \
	-Dvmod_builddir=$(abs_top_builddir)/src \
	-Dvmod_directors="directors from \"$(VARNISHAPI_VMODDIR)/libvmod_directors.so\""

VMOD_TESTS = \
	tests/bodyaccess/test01.vtc \
	tests/bodyaccess/test02.vtc \
	tests/bodyaccess/test03.vtc \
	tests/bodyaccess/test04.vtc \
	tests/bodyaccess/test05.vtc \
	tests/cookie/01-set-delete-produce.vtc \
	tests/cookie/02-clean.vtc \
	tests/cookie/03-null.vtc \
	tests/cookie/04-keep.vtc \
	tests/cookie/05-filter.vtc \
	tests/cookie/05-get.vtc \
	tests/cookie/06-concurrent-query.vtc \
	tests/cookie/07-isset.vtc \
	tests/cookie/08-overflow.vtc \
	tests/cookie/10-parser.vtc \
	tests/cookie/30-format_1123.vtc \
	tests/cookie/31-get_regex.vtc \
	tests/cookie/32-filter_regex.vtc \
	tests/cookie/33-keep_regex.vtc \
	tests/cookie/34-vcl_discard.vtc \
	tests/cookie/r28.vtc \
	tests/header/append.vtc \
	tests/header/copy.vtc \
	tests/header/get.vtc \
	tests/header/import.vtc \
	tests/header/keep-others.vtc \
	tests/header/remove.vtc \
	tests/header/some-data.vtc \
	tests/header/regsub.vtc \
	tests/saintmode/test01.vtc \
	tests/saintmode/test02.vtc \
	tests/saintmode/test03.vtc \
	tests/saintmode/test04.vtc \
	tests/saintmode/test05.vtc \
	tests/tcp/01-dumpinfo.vtc \
	tests/tcp/02-congestion.vtc \
	tests/tcp/03-read-tcpinfo.vtc \
	tests/tcp/04-pacing.vtc \
	tests/var/test01.vtc \
	tests/var/test02.vtc \
	tests/var/test03.vtc \
	tests/var/test04.vtc \
	tests/var/test05.vtc \
	tests/vsthrottle/test01.vtc \
	tests/vsthrottle/test02.vtc \
	tests/vsthrottle/test03.vtc \
	tests/vsthrottle/test04.vtc \
	tests/xkey/test01.vtc \
	tests/xkey/test02.vtc \
	tests/xkey/test03.vtc \
	tests/xkey/test04.vtc \
	tests/xkey/test05.vtc \
	tests/xkey/test06.vtc \
	tests/xkey/test07.vtc \
	tests/xkey/test08.vtc \
	tests/xkey/test09.vtc

SOFTPURGE_TESTS = \
	tests/softpurge/01-load.vtc \
	tests/softpurge/02-simple-hit.vtc \
	tests/softpurge/03-simple-miss.vtc \
	tests/softpurge/10-softpurge-hit.vtc \
	tests/softpurge/11-really-purged.vtc \
	tests/softpurge/12-softpurge-variants.vtc \
	tests/softpurge/13-softpurge-variants-sickbackend.vtc \
	tests/softpurge/14-mind-the-refcounters.vtc

TESTS = $(VMOD_TESTS)
if WITH_VMOD_SOFTPURGE
TESTS += $(SOFTPURGE_TESTS)
endif

EXTRA_DIST = \
	foreign \
	vmod_config.h \
	$(VMOD_TESTS) \
	$(SOFTPURGE_TESTS) \
	vmod_bodyaccess.vcc \
	vmod_cookie.vcc \
	vmod_header.vcc \
	vmod_saintmode.vcc \
	vmod_softpurge.vcc \
	vmod_tcp.vcc \
	vmod_var.vcc \
	vmod_vsthrottle.vcc \
	vmod_xkey.vcc
