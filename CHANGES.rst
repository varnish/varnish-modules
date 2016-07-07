This is a running log of changes to varnish-modules.

varnish-modules 0.9.1 (unreleased)
----------------------------------

Changes since 0.9.0:

* Example for vmod-saintmode has been improved. (Issue #16)
* Forgotten vmod-var documentation added. (Issue #24)
* Licenses added to source files. (#9)
* [vmod-cookie] Bugfixes from libvmod-cookie.git forgotten on initial import applied.
  More robust filter parsing, superfluous debug log entries removed, avoid
  reading past the end of invalid cookie headers, avoid invalid memory reference in filter_except().

This release is intended to work with Varnish Cache 4.1.3 and higher.


varnish-modules 0.9.0 (2014-03-04)
----------------------------------

Initial release. This package contains the source files extracted from
the following git repositories and commit identifiers:

* b772825 in libvmod-cookie.git
* 86da3be in libvmod-header.git
* d8658c9 in libvmod-saintmode.git
* e6c8ce1 in libvmod-softpurge.git
* 8add5f8 in libvmod-tcp.git
* c99cb30 in libvmod-var.git
* 52c5d64 in libvmod-xkey.git

This release is intended to work with Varnish Cache 4.1.2 and higher.
