..
.. NB:  This file is machine generated, DO NOT EDIT!
..
.. Edit vmod.vcc and run make instead
..

.. role:: ref(emphasis)

.. _vmod_header(3):

===========
vmod_header
===========

-----------------------
Header VMOD for Varnish
-----------------------

:Manual section: 3

SYNOPSIS
========

import header [from "path"] ;

DESCRIPTION
===========

Varnish Module (vmod) for manipulation of duplicated headers (for instance
multiple set-cookie headers).

CONTENTS
========

* VOID append(HEADER, STRING_LIST)
* VOID copy(HEADER, HEADER)
* STRING get(PRIV_CALL, HEADER, STRING)
* VOID remove(PRIV_CALL, HEADER, STRING)
* STRING version()

.. _func_append:

VOID append(HEADER, STRING_LIST)
--------------------------------

Prototype
	VOID append(HEADER, STRING_LIST)
Description
        Append lets you add an extra occurrence of an existing header.
Example
	::

		header.append(beresp.http.Set-Cookie,"foo=bar")

.. _func_copy:

VOID copy(HEADER, HEADER)
-------------------------

Prototype
	VOID copy(HEADER, HEADER)
Description
        Copies all of the source headers to a new header.
Example
	::

		header.copy(beresp.http.set-cookie, beresp.http.x-old-cookie);

.. _func_get:

STRING get(PRIV_CALL, HEADER, STRING)
-------------------------------------

Prototype
	STRING get(PRIV_CALL, HEADER, STRING)
Description
        Get fetches the value of the first `header` that matches the given
        regular expression.
Example
	::

		set beresp.http.xusr = header.get(beresp.http.set-cookie,"user=");

.. _func_remove:

VOID remove(PRIV_CALL, HEADER, STRING)
--------------------------------------

Prototype
	VOID remove(PRIV_CALL, HEADER, STRING)
Description
        remove() removes all occurences of `header` that matches the given
        regular expression. The example is a white-list of "funcookie=".
Example
	::

	        header.remove(beresp.http.set-cookie,"^(?!(funcookie=))");


.. _func_version:

STRING version()
----------------

Prototype
	STRING version()
Description
        Returns the string constant version-number of the header vmod.
Example
	::

		set resp.http.X-header-version = header.version();

ACKNOWLEDGEMENTS
================

The development of this plugin was made possible by the sponsorship of
Softonic, http://en.softonic.com/ .

Also thanks to: Imo Klabun and Anders Nordby for bug reports.

BUGS
====

You can't use dynamic regular expressions, which also holds true for normal
regular expressions in regsub(), but VCL isn't able to warn you about this
when it comes to vmods yet.

Some overlap with varnishd exists, this will be mended as Varnish 3.0
evolves.

SEE ALSO
========

* varnishd(1)
* vcl(7)
* https://github.com/varnish/libvmod-header
