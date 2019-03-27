Status of this repository
=========================

Modules in this repository are generally not in active development by
Varnish Software. However, any security related bugs and issues will
always be a high priority, and in some cases we will accept new
features to the VMODs

For Varnish Software, the maintainer of this repository, the VMODs are
categorized as follows.

VMODs under development
-----------------------

The VMODs in this category are ``tcp`` and ``vsthrottle``.

New features for these VMODs will be considered on a case-by-case
basis, and new features will be accepted if they fit together with
existing features in VMOD.

Feature complete VMODs
----------------------

The VMODs in this category are ``Cookie``, ``xkey``, ``saintmode``,
``softpurge``, ``bodyaccess``, ``header`` and ``var``.

This software in this repository is considered feature complete and we
do no longer accept contributions that introduce new features. The
software is maintained with security and bug fixes, and contributions
related to this will be accepted.

Deprecated VMODs
----------------

There are no VMODs in this category.

This software has been deprecated and is no longer maintained.
Contributions will not be accepted.


Moved or replaced VMODs
-----------------------

VMODs in this category are no longer maintained because their
functionality is covered by other functionality or newer VMODs.

In this repository there is only one such vmod:

* ``softpurge`` is now replaced by the ``purge`` VMOD in Varnish.
  Note that this VMOD is also in the "feature complete" category,
  since it is still needed for Varnish Cache 4.1.

General note
------------

If you want to contribute a feature for this software, please create
an issue and describe your implementation plans before you get
started. This allows us to verify that the contribution is in line
with the software's roadmap.

Future compatibility
--------------------

Varnish has no commitment to update this software to be compatible
with future versions of Varnish Cache. At the same time, the license
of the sotware allows anyone to improve the product in any way, for
example by making it compatible with future versions of Varnish Cache.


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
* Client request body access

This collection contains the following vmods (previously kept
individually): cookie, vsthrottle, header, saintmode, softpurge, tcp,
var, xkey, bodyaccess

Supported Varnish version is described in the `CHANGES.rst` file. Normally this
is the last public Varnish Cache release. See PORTING below for information on
support for other versions of Varnish.


Installation
------------

Source releases can be downloaded from:

    https://download.varnish-software.com/varnish-modules/


Installation requires an installed version of Varnish Cache, including the
development files. Requirements can be found in the `Varnish documentation`_.

.. _`Varnish documentation`: https://www.varnish-cache.org/docs/4.1/installation/install.html#compiling-varnish-from-source
.. _`Varnish Project packages`: https://www.varnish-cache.org/releases/index.html


Source code is built with autotools, you need to install the correct development packages first.
If you are using the official `Varnish Project packages`_::

    sudo apt-get install varnish-dev || sudo yum install varnish-devel

If you are using the distro provided packages::

    sudo apt-get install libvarnishapi-dev || sudo yum install varnish-libs-devel

Then proceed to the configure and build::

    ./configure
    make
    make check   # optional
    sudo make install


The resulting loadable modules (``libvmod_*.so`` files) will be installed to
the Varnish module directory. (default `/usr/lib/varnish/vmods/`)


Usage
-----

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



Development
-----------

The source git tree lives on Github: https://github.com/varnish/varnish-modules

All source code is placed in the master git branch. Pull requests and issue
reporting are appreciated.

Unlike building from releases, you need to first bootstrap the build system
when you work from git::

    ./bootstrap
    ./configure
    make
    make check # recommended

If the ``configure`` step succeeds but the ``make`` step fails, check for
warnings in the ``./configure`` output or the ``config.log`` file. You may be
missing bootstrap dependencies not required by release archives.

Porting
-------

We encourage porting of the module package to other versions of Varnish Cache.


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


Contact
-------

This code is maintained by Varnish Software. (https://www.varnish-software.com/)

Issues can be reported via the Github issue tracker.

Other inquires can be sent to opensource@__no_spam_please__varnish-software.com.

