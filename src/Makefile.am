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
	libvmod_accept.la \
	libvmod_bodyaccess.la \
	libvmod_header.la \
	libvmod_saintmode.la \
	libvmod_str.la \
	libvmod_tcp.la \
	libvmod_var.la \
	libvmod_vsthrottle.la \
	libvmod_xkey.la

if HAVE_RST2MAN
dist_man_MANS = \
	vmod_accept.3 \
	vmod_bodyaccess.3 \
	vmod_header.3 \
	vmod_saintmode.3 \
	vmod_str.3 \
	vmod_tcp.3 \
	vmod_var.3 \
	vmod_vsthrottle.3 \
	vmod_xkey.3
endif

generated_docs = $(dist_man_MANS:.3=.rst)

libvmod_accept_la_SOURCES = vmod_accept.c
libvmod_bodyaccess_la_SOURCES = vmod_bodyaccess.c
libvmod_header_la_SOURCES = vmod_header.c
libvmod_vsthrottle_la_SOURCES = vmod_vsthrottle.c
libvmod_saintmode_la_SOURCES = vmod_saintmode.c
libvmod_str_la_SOURCES = vmod_str.c
libvmod_tcp_la_SOURCES = vmod_tcp.c
libvmod_var_la_SOURCES = vmod_var.c
libvmod_xkey_la_SOURCES = vmod_xkey.c

nodist_libvmod_accept_la_SOURCES = vcc_accept_if.c vcc_accept_if.h
nodist_libvmod_bodyaccess_la_SOURCES = vcc_bodyaccess_if.c vcc_bodyaccess_if.h
nodist_libvmod_header_la_SOURCES = vcc_header_if.c vcc_header_if.h
nodist_libvmod_vsthrottle_la_SOURCES = vcc_vsthrottle_if.c vcc_vsthrottle_if.h
nodist_libvmod_saintmode_la_SOURCES = vcc_saintmode_if.c vcc_saintmode_if.h
nodist_libvmod_str_la_SOURCES = vcc_str_if.c vcc_str_if.h
nodist_libvmod_tcp_la_SOURCES = vcc_tcp_if.c vcc_tcp_if.h
nodist_libvmod_var_la_SOURCES = vcc_var_if.c vcc_var_if.h
nodist_libvmod_xkey_la_SOURCES = vcc_xkey_if.c vcc_xkey_if.h VSC_xkey.c VSC_xkey.h

vmod_xkey.3: VSC_xkey.rst

@BUILD_VMOD_ACCEPT@
@BUILD_VMOD_BODYACCESS@
@BUILD_VMOD_HEADER@
@BUILD_VMOD_SAINTMODE@
@BUILD_VMOD_STR@
@BUILD_VMOD_TCP@
@BUILD_VMOD_VAR@
@BUILD_VMOD_VSTHROTTLE@
@BUILD_VMOD_XKEY@
@BUILD_VSC_XKEY@

rst-docs: $(generated_docs)
	mkdir -p $(top_builddir)/docs/
	cp $(generated_docs) $(top_builddir)/docs/

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
	tests/accept/test01.vtc \
	tests/accept/test02.vtc \
	tests/bodyaccess/test01.vtc \
	tests/bodyaccess/test02.vtc \
	tests/bodyaccess/test03.vtc \
	tests/bodyaccess/test04.vtc \
	tests/bodyaccess/test05.vtc \
	tests/header/append.vtc \
	tests/header/append_maxhdr_overflow.vtc \
	tests/header/append_overflow.vtc \
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
	tests/str/test01.vtc \
	tests/str/test02.vtc \
	tests/str/test03.vtc \
	tests/str/test04.vtc \
	tests/str/test05.vtc \
	tests/str/test06.vtc \
	tests/str/test08.vtc \
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
	tests/xkey/test09.vtc \
	tests/xkey/test10.vtc \
	tests/xkey/test11.vtc \
	tests/xkey/test12.vtc \
	tests/xkey/test13.vtc

TESTS = $(VMOD_TESTS)

EXTRA_DIST = \
	foreign \
	vmod_config.h \
	$(VMOD_TESTS) \
	vmod_accept.vcc \
	vmod_bodyaccess.vcc \
	vmod_header.vcc \
	vmod_saintmode.vcc \
	vmod_str.vcc \
	vmod_tcp.vcc \
	vmod_var.vcc \
	vmod_vsthrottle.vcc \
	vmod_xkey.vcc \
	xkey.vsc
