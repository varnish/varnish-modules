..
.. NB:  This file is machine generated, DO NOT EDIT!
..
.. Edit vmod.vcc and run make instead
..

.. role:: ref(emphasis)

.. _vmod_saintmode(3):

==============
vmod_saintmode
==============

---------------------------
Saint mode backend director
---------------------------

:Manual section: 3

SYNOPSIS
========

import saintmode [from "path"] ;


DESCRIPTION
===========

This VMOD provides saintmode functionality for Varnish Cache 4.1 and
newer. The code is in part based on Poul-Henning Kamp's saintmode
implementation in Varnish 3.0.

Saintmode lets you deal with a backend that is failing in random ways
for specific requests. It maintains a blacklist per backend, marking
the backend as sick for specific objects. When the number of objects
marked as sick for a backend reaches a set threshold, the backend is
considered sick for all requests. Each blacklisted object carries a
TTL, which denotes the time it will stay blacklisted.

Saintmode in Varnish 4.1 is implemented as a director VMOD. We instantiate a
saintmode object and give it a backend as an argument. The resulting object can
then be used in place of the backend, with the effect that it also has added
saintmode capabilities.

Any director will then be able to use the saintmode backends, and as
backends marked sick are skipped by the director, this provides a way
to have fine grained health status on the backends, and making sure that
retries get a different backend than the one which failed.

.. vcl-start

Example::

    vcl 4.0;

    import saintmode;
    import directors;

    backend tile1 { .host = "192.0.2.11"; .port = "80"; }
    backend tile2 { .host = "192.0.2.12"; .port = "80"; }

    sub vcl_init {
        # Instantiate sm1, sm2 for backends tile1, tile2
        # with 10 blacklisted objects as the threshold for marking the
        # whole backend sick.
        new sm1 = saintmode.saintmode(tile1, 10);
        new sm2 = saintmode.saintmode(tile2, 10);

        # Add both to a director. Use sm0, sm1 in place of tile1, tile2.
        # Other director types can be used in place of random.
        new imagedirector = directors.random();
        imagedirector.add_backend(sm1.backend(), 1);
        imagedirector.add_backend(sm2.backend(), 1);
    }

    sub vcl_backend_fetch {
        # Get a backend from the director.
        # When returning a backend, the director will only return backends
        # saintmode says are healthy.
        set bereq.backend = imagedirector.backend();
    }

    sub vcl_backend_response {
        if (beresp.status >= 500) {
            # This marks the backend as sick for this specific
            # object for the next 20s.
            saintmode.blacklist(20s);
            # Retry the request. This will result in a different backend
            # being used.
            return (retry);
        }
    }

.. vcl-end


CONTENTS
========

* :ref:`func_blacklist`
* :ref:`obj_saintmode`
* :ref:`func_saintmode.backend`
* :ref:`func_saintmode.blacklist_count`
* :ref:`func_status`

.. _func_blacklist:

blacklist
---------

::

	VOID blacklist(PRIV_VCL, DURATION expires)

Marks the backend as sick for a specific object. Used in vcl_backend_response.
Corresponds to the use of ``beresp.saintmode`` in Varnish 3.0. Only available
in vcl_backend_response.

Example::

    sub vcl_backend_response {
        if (beresp.http.broken-app) {
            saintmode.blacklist(20s);
            return (retry);
        }
    }


.. _func_status:

status
------

::

	STRING status(PRIV_VCL)

Returns a JSON formatted status string suitable for use in vcl_synth.

::

   sub vcl_recv {
       if (req.url ~ "/saintmode-status") {
           return (synth(700, "OK"));
       }
   }

   sub vcl_synth {
       if (resp.status == 700) {
           synthetic(saintmode.status());
           return (deliver);
       }
   }

Example JSON output:

   ::

      {
	"saintmode" : [
            { "name": "sm1", "backend": "foo", "count": "3", "threshold": "10" },
            { "name": "sm2", "backend": "bar", "count": "2", "threshold": "5" }
	]
      }


.. _obj_saintmode:

saintmode
---------

::

	new OBJ = saintmode(PRIV_VCL, BACKEND backend, INT threshold)

Constructs a saintmode director object. The ``threshold`` argument sets
the saintmode threshold, which is the maximum number of items that can be
blacklisted before the whole backend is regarded as sick. Corresponds with the
``saintmode_threshold`` parameter of Varnish 3.0.

Example::

    sub vcl_init {
        new sm = saintmode.saintmode(b, 10);
    }


.. _func_saintmode.backend:

saintmode.backend
-----------------

::

	BACKEND saintmode.backend()

Used for assigning the backend from the saintmode object.

Example::

    sub vcl_backend_fetch {
        set bereq.backend = sm.backend();
    }


.. _func_saintmode.blacklist_count:

saintmode.blacklist_count
-------------------------

::

	INT saintmode.blacklist_count()

Returns the number of objects currently blacklisted for a saintmode
director object.

Example:

::

   sub vcl_deliver {
       set resp.http.troublecount = sm.blacklist_count();
   }

