ACLOCAL_AMFLAGS = -I m4 -I ${LIBVARNISHAPI_DATAROOTDIR}/aclocal

SUBDIRS = src

DISTCHECK_CONFIGURE_FLAGS = \
	VMOD_DIR='$${libdir}/varnish/vmods'

EXTRA_DIST = README.rst \
	LICENSE \
	docs \
	CHANGES.rst

doc_DATA = README.rst LICENSE

#dist_man_MANS = vmod_vsthrottle.3
#MAINTAINERCLEANFILES = $(dist_man_MANS)

#vmod_vsthrottle.3: README.rst

%.1 %.2 %.3 %.4 %.5 %.6 %.7 %.8 %.9:
if HAVE_RST2MAN
	${RST2MAN} --halt=2 --exit-status=2 $< $@
else
	@echo "========================================"
	@echo "You need rst2man installed to make dist"
	@echo "========================================"
	@false
endif

