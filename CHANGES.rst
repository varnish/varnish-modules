This is a running log of changes to varnish-modules.

varnish-modules 0.10.0 (unreleased)
-----------------------------------

* Improved documentation and examples for vmod-saintmode, vmod-cookie,
  vmod-xkey, vmod-var, vmod-softpurge, vmod-header, vmod-tcp and
  vmod-vsthrottle
* Various improvements in the build system
* [vmod-cookie] Fix test case overflow issue for 32 bit systems (#35)
* [vmod-vsthrottle] Fix OS X build issue (#37)
* [vmod-saintmode] Add new log records for when vmod-saintmode marks a
  backend as unhealthy (#43)
* [vmod-saintmode] Added saintmode.status() which outputs a JSON
  status string for use in vcl_synth (#43)
* [vmod-vsthrottle] Added vsthrottle.remaining() which returns the
  current number of tokens for a given bucket.
* Correct build with 4.1.4-beta1, 5.0.0 and master at rev dfcf44c6.
* [vmod-xkey] Add support for purging multiple keys in a single invocation
* [vmod-cookie] Fix a bug where we would crash on malicious input

varnish-modules 0.9.1 (2016-07-07)
----------------------------------

Changes since 0.9.0:

* Example for vmod-saintmode has been improved. (Issue #16)
* Forgotten vmod-var documentation added. (Issue #24)
* Licenses added to source files. (#9)
* [vmod-cookie] Bugfixes from libvmod-cookie.git forgotten on initial import applied.
  More robust filter parsing, superfluous debug log entries removed, avoid
  reading past the end of invalid cookie headers, avoid invalid memory reference in filter_except().

This release is intended to work with Varnish Cache 4.1.3 and higher.


varnish-modules 0.9.0 (2016-03-04)
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
