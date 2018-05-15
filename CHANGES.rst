This is a running log of changes to varnish-modules.

varnish-modules 0.15.0 (2018-05-15)
-----------------------------------

* Varnish 6 compatibility


varnish-modules 0.14.0 (2018-02-19)
-----------------------------------

* [vmod-var] Add set_backend() and get_backend()


varnish-modules 0.13.0 (2018-01-10)
-----------------------------------

* [vmod-vsthrottle] Add an optional block parameter to
  vsthrottle.is_denied, to permit blocking clients for a specified
  duration when the request rate threshold is reached.
* [vmod-vsthrottle] Add vsthrottle.blocked() for querying remaining
  time for a pending block.
* [vmod-header]: Fix a bug where we would crash if a non-existing
  header would be passed as the final argument to header.append()
* [vmod-saintmode] New function: is_healthy. Checks if the object is currently
  blacklisted for a saintmode director object.


varnish-modules 0.12.1 (2017-05-24)
-----------------------------------

* Bump configure.ac version number

varnish-modules 0.12.0 (2017-05-23)
-----------------------------------

* [vmod-bodyaccess] New vmod that enables simple matching and hashing
  on the client request body.
* [vmod-xkey] Some documentation improvements
* Build system fixes

varnish-modules 0.11.0 (2017-02-07)
-----------------------------------

* [vmod-saintmode] Fix a bug where saintmode.blacklist() would cause a
  crash if saintmode was not configured. (#54)
* [vmod-cookie] New cookie.filter() function to blacklist cookies.


varnish-modules 0.10.2 (2017-01-31)
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
