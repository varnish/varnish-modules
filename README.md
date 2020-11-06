[![CircleCI](https://circleci.com/gh/varnish/varnish-modules/tree/master.svg?style=svg)](https://app.circleci.com/pipelines/github/varnish/varnish-modules?branch=master)

# Varnish module collection by Varnish Software

This is a collection of modules ("vmods") extending Varnish VCL used for
describing HTTP request/response policies with additional capabilities.

Included:

* `bodyaccess`: Client request body access
* `header`:Modify and change complex HTTP headers
* `saintmode`: 3.0-style saint mode
* `str`: String operations
* `tcp`: TCP connections tweaking
* `var`: Variable support
* `vstrottle`: Request and bandwidth throttling
* `xkey`: Advanced cache invalidations

Modules in this repository are generally not in active development by
Varnish Software. However, any security related bugs and issues will
always be a high priority. We'll also gladly considering pull requests for new
features but we recommend opening an issue first to discuss implementation
plans.

## Installation

Source releases can be downloaded from the [github release page of the project(]https://github.com/varnish/varnish-modules/releases)
and older releases are located [here](https://download.varnish-software.com/varnish-modules/).

Each release only targets one version of Varnish and the git repository sports a
branch dedicated to each version, starting from `6.0`. This means that if you
want to compile modules for Varnish `6.3`, you need to `git checkout` the `6.3`
branch of this repository

Compiling, testing and installing relies on the `autotools` suite and requires
the usual dependencies, plus the Varnish develpoment files and `python-sphinx`.

Here's a directly usable `Dockerfile` as an example:

``` dockerfile
FROM centos:7

# install Varnish 6.5 from https://packagecloud.io/varnishcache
RUN curl -s https://packagecloud.io/install/repositories/varnishcache/varnish65/script.rpm.sh | bash
# the epel repo contains jemalloc
RUN yum install -y epel-release
# install our dependencies
RUN yum install -y git make automake libtool python-sphinx varnish-devel
# download the top of the varnish-modules 6.5 branch
RUN git clone --branch 6.5 --single-branch https://github.com/varnish/varnish-modules.git
# jump into the directory
WORKDIR /varnish-modules
# prepare the build, build, check and install
RUN ./bootstrap && \
    ./configure && \
    make && \
    make check -j 4 && \
    make install
```

## Usage

Each module has a different set of functions and usage, described in
separate documents in `docs/`. For completeness, here is a snippet from
`docs/cookie.rst`::

    import cookie;

    sub vcl_recv {
            cookie.parse(req.http.cookie);
            cookie.filter_except("SESSIONID,PHPSESSID");
            set req.http.cookie = cookie.get_string();
            # Only SESSIONID and PHPSESSID are left in req.http.cookie at this point.
    }


## Moved or replaced VMODs

VMODs in this category are no longer maintained because their
functionality is covered by other functionality or newer VMODs.

In this repository there is only two such vmods:

* ``cookie``, that is now part of Varnish.

* ``softpurge`` is now replaced by the ``purge`` VMOD in Varnish.
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

