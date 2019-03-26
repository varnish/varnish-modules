..
.. NB:  This file is machine generated, DO NOT EDIT!
..
.. Edit vmod.vcc and run make instead
..


:tocdepth: 1

.. _vmod_header(3):

=====================================
VMOD header - Header VMOD for Varnish
=====================================

SYNOPSIS
========

.. parsed-literal::

  import header [from "path"]
  
  :ref:`vmod_header.append`
   
  :ref:`vmod_header.copy`
   
  :ref:`vmod_header.get`
   
  :ref:`vmod_header.remove`
   
DESCRIPTION
===========

Varnish Module for manipulation of duplicated HTTP headers, for instance
multiple Set-Cookie headers.

.. vcl-start

Example::

    vcl 4.0;
    import header;

    backend default { .host = "192.0.2.11"; .port = "8080"; }

    sub vcl_backend_response {
        if (beresp.http.Set-Cookie) {
            # Add another line of Set-Cookie in the response.
            header.append(beresp.http.Set-Cookie, "VSESS=abbabeef");

            # CMS always set this, but doesn't really need it.
            header.remove(beresp.http.Set-Cookie, "JSESSIONID=");
        }
    }

.. vcl-end


.. _vmod_header.append:

VOID append(HEADER, STRING)
---------------------------

Description
        Append an extra occurrence to an existing header.
Example
    ::
    header.append(beresp.http.Set-Cookie, "foo=bar")

.. _vmod_header.copy:

VOID copy(HEADER, HEADER)
-------------------------

Description
        Copy all source headers to a new header.
Example
    ::
    header.copy(beresp.http.set-cookie, beresp.http.x-old-cookie);

.. _vmod_header.get:

STRING get(HEADER header, STRING regex)
---------------------------------------

Description
        Fetches the value of the first `header` that matches the given
        regular expression `regex`.
Example
    ::
    set beresp.http.xusr = header.get(beresp.http.set-cookie,"user=");

.. _vmod_header.remove:

VOID remove(HEADER header, STRING regex)
----------------------------------------

Description
        Remove all occurrences of `header` that matches `regex`.
Example
    ::
    header.remove(beresp.http.set-cookie,"^(?!(funcookie=))");



ACKNOWLEDGEMENTS
================

The development of this plugin was made possible by the sponsorship of
Softonic, http://en.softonic.com/ .

Also thanks to Imo Klabun and Anders Nordby for bug reports.

BUGS
====

You can't use dynamic regular expressions, which also holds true for normal
regular expressions in regsub().
