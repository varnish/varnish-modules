..
.. NB:  This file is machine generated, DO NOT EDIT!
..
.. Edit vmod.vcc and run make instead
..

.. role:: ref(emphasis)

.. _vmod_softpurge(Soft):

==============
vmod_softpurge
==============

----------
purge vmod
----------

:Manual section: Soft

SYNOPSIS
========

import softpurge [from "path"] ;

DESCRIPTION
===========

``Softpurge`` is cache invalidation in Varnish that reduces TTL but
keeps the grace value of a resource.

This makes it possible to serve stale content to users if the backend
is unavailable and fresh content can not be fetched.

.. vcl-start

Example::

    vcl 4.0;
    import softpurge;

    backend default { .host = "192.0.2.11"; .port = "8080"; }

    sub vcl_recv {
        # Return early to avoid return(pass) by builtin VCL.
        if (req.method == "PURGE") {
            return (hash);
        }
    }

    sub vcl_backend_response {
        # Set object grace so we keep them around after TTL has expired.
        set beresp.grace = 10m;
    }

    sub vcl_hit {
        if (req.method == "PURGE") {
            softpurge.softpurge();
            return (synth(200, "Successful softpurge"));
        }
    }

    sub vcl_miss {
        if (req.method == "PURGE") {
            softpurge.softpurge();
            return (synth(200, "Successful softpurge"));
        }
    }

.. vcl-end

CONTENTS
========

* :ref:`func_softpurge`

.. _func_softpurge:

softpurge
---------

::

	VOID softpurge()

Performs a soft purge. Valid in vcl_hit and vcl_miss.

Example::

    sub vcl_hit {
        if (req.method == "PURGE") {
            softpurge.softpurge();
        }
    }


