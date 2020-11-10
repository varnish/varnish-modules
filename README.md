[![CircleCI](https://circleci.com/gh/varnish/varnish-modules/tree/master.svg?style=svg)](https://app.circleci.com/pipelines/github/varnish/varnish-modules?branch=master)

# Varnish module collection by Varnish Software

This is a collection of modules ("vmods") extending Varnish VCL used for
describing HTTP request/response policies with additional capabilities.

Included:

* [`accept`](src/vmod_accept.vcc): Filter `accept`-like headers
* [`bodyaccess`](src/vmod_bodyaccess.vcc): Client request body access
* [`header`](src/vmod_header.vcc): Modify and change complex HTTP headers
* [`saintmode`](src/vmod_saintmode.vcc): 3.0-style saint mode
* [`str`](src/vmod_str.vcc): String operations
* [`tcp`](src/vmod_tcp.vcc): TCP connections tweaking
* [`var`](src/vmod_var.vcc): Variable support
* [`vstrottle`](src/vmod_vsthrottle.vcc): Request and bandwidth throttling
* [`xkey`](src/vmod_xkey.vcc): Advanced cache invalidations

Modules in this repository are feature-complete and are maintained to stay
compatible with new Varnish releases and to fix bugs.  We'll also gladly
considering pull requests for new features but we recommend opening an issue
first to discuss implementation plans.

## Installation

Source releases can be downloaded from the [github release page of the project](https://github.com/varnish/varnish-modules/releases)
and older releases are located [here](https://download.varnish-software.com/varnish-modules/).

Each release only targets one version of Varnish (visible in the realease title)
and the git repository sports a branch dedicated to each version, starting from
`6.0`. This means that if you want to compile modules for Varnish `6.3`,
you need to `git checkout` the `6.3` branch of this repository

Compiling, testing and installing relies on the `autotools` suite and requires
the usual autotools dependencies (`make automake libtool`), plus the Varnish
development files (`varnish-dev` or `varnish-devel`) and `python-sphinx`.

Note that packages names given here are purely indicative and may vary depending
on the OS you are using.

To compile:

``` bash
./bootstrap
./configure   # run "configure -h" first to list options
make
make check    # optional (tests)
make rst-docs # optional (docs)
make install  # optional (installation), run as root
```

A [Dockerfile](Dockerfile) is available as example.

## Usage

Each module has a different set of functions and usage, described in
each `src/vmod_*.vcc` file.

Note that `make rst-docs` will generate `reStructuredText` docs in the `docs/`
directory.

## Moved or replaced VMODs

VMODs in this category are no longer maintained because their
functionality is covered by other functionality or newer VMODs.

In this repository there is only two such vmods:

* ``cookie``, that is now part of Varnish (since 6.4)

* ``softpurge`` is now replaced by the ``purge`` VMOD in Varnish (since 5.2).
  Note that this VMOD is also in the "feature complete" category,
  since it is still needed for Varnish Cache 4.1.

## Administrativa

The goals of this collection are:

* Simplify access to vmod code for Varnish users. One package to install, not 6.
* Decrease the maintenance cost that comes with having 10 different git
  repositories, each with autotools and (previously) distribution packaging files.

Expressed non-goals are:

* Import vmods that require external libraries, like curl or geoip. This
  collection should be simple and maintenance free to run.
* Support older releases of Varnish Cache.
* Include every vmod under the sun. We'll add the important ones.

Addition of further vmods is decided on a case-by-case basis. Code quality and
maintenance requirements will be important in this decision.


## Contact

This code is maintained by Varnish Software. (https://www.varnish-software.com/)

Issues can be reported via the Github issue tracker.

Other inquires can be sent to opensource@__no_spam_please__varnish-software.com.

