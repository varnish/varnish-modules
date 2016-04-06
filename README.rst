Varnish module collection by Varnish Software
=============================================

This is a collection of modules ("vmods") extending Varnish VCL used for
describing HTTP request/response policies with additional capabilities.

Included:

* Simpler handling of HTTP cookies
* Variable support
* Request and bandwidth throttling
* Modify and change complex HTTP headers
* 3.0-style saint mode,
* Advanced cache invalidations, and more.

This collection contains the following vmods (previously kept individually):
cookie, vsthrottle, header, saintmode, softpurge, tcp, var, xkey

Supported Varnish version is described in the `CHANGES.rst` file. Normally this
is the last public Varnish Cache release.


Installation
------------

Installation requires an installed version of Varnish Cache, including the
development files.

Source code is built with autotools::

    apt-get install libvarnishapi-dev || yum install varnish-libs-devel
    ./bootstrap
    ./configure
    make check
    make install

The resulting loadable modules (``libvmod_foo*.so`` files) will be installed to
the Varnish module directory. (default `/var/lib/varnish/vmods/`)

Installing to a non-standard location
-------------------------------------


If you have installed Varnish to a non-standard directory, call
``autogen.sh`` and ``configure`` with ``PKG_CONFIG_PATH`` pointing to
the appropriate path. For example, when varnishd configure was called
with ``--prefix=$PREFIX``, use

 PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig
 export PKG_CONFIG_PATH

Make and install the vmods::
 
 make           # builds the vmods
 make install   # installs to `VMODDIR`
 make check     # runs the unit tests in ``src/tests/*.vtc``


Usage
-----


Each module has a different set of functions and usage, described in
separate documents in `src/vmod_'.vcc`. For completeness, here is a snippet from
`src/vmod_cookie.vcc`::

    import cookie;

    sub vcl_recv {
            cookie.parse(req.http.cookie);
            cookie.filter_except("SESSIONID,PHPSESSID");
            set req.http.cookie = cookie.get_string();
            # Only SESSIONID and PHPSESSID are left in req.http.cookie at this point.
    }



Development
-----------

The source git tree lives on Github: https://github.com/varnish/varnish-modules

All source code is placed in the master git branch. Pull requests and issue
reporting are appreciated.


Administrativa
--------------

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

We encourage porting efforts to older Varnish Cache versions, if someone wants
to do that. (and maintain it!)


Contact
-------

This code is maintained by Varnish Software. (https://www.varnish-software.com/)

Issues can be reported via the Github issue tracker.

Other inquires can be sent to opensource@__no_spam_please__varnish-software.com.

