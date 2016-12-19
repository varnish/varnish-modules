ACLOCAL_AMFLAGS = -I m4 -I ${VARNISHAPI_DATAROOTDIR}/aclocal

SUBDIRS = src

doc_DATA = README.rst LICENSE

EXTRA_DIST = \
	$(doc_DATA) \
	docs \
	CHANGES.rst

.PHONY: docs

docs:
	$(MAKE) -C src/ docs

# lcov support
CODE_COVERAGE_OUTPUT_DIRECTORY = lcov
CODE_COVERAGE_IGNORE_PATTERN = "/usr/*"
CODE_COVERAGE_GENHTML_OPTIONS = --prefix $(abs_top_srcdir)

@CODE_COVERAGE_RULES@

lcov: check-code-coverage
