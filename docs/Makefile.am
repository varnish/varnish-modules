
dist_man_MANS = vmod_cookie.3 \
	vmod_header.3 \
	vmod_saintmode.3 \
	vmod_softpurge.3 \
	vmod_tcp.3 \
	vmod_var.3 \
	vmod_vsthrottle.3 \
	vmod_xkey.3

%.3: %.man.rst
if HAVE_RST2MAN
	${RST2MAN} --halt=2 --exit-status=2 $< $@
else
	@echo "========================================"
	@echo "You need rst2man installed to make dist"
	@echo "========================================"
	@false
endif

CLEANFILES = $(builddir)/$(dist_man_MANS)
