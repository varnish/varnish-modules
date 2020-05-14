..
.. NB:  This file is machine generated, DO NOT EDIT!
..
.. Edit vmod.vcc and run make instead
..


:tocdepth: 1

.. _vmod_var(3):

===========================================
VMOD var - Variable support for Varnish VCL
===========================================

SYNOPSIS
========

.. parsed-literal::

  import var [from "path"]
  
  :ref:`vmod_var.set`
   
  :ref:`vmod_var.get`
   
  :ref:`vmod_var.global_set`
   
  :ref:`vmod_var.global_get`
   
  :ref:`vmod_var.set_int`
   
  :ref:`vmod_var.get_int`
   
  :ref:`vmod_var.set_string`
   
  :ref:`vmod_var.get_string`
   
  :ref:`vmod_var.set_real`
   
  :ref:`vmod_var.get_real`
   
  :ref:`vmod_var.set_duration`
   
  :ref:`vmod_var.get_duration`
   
  :ref:`vmod_var.set_ip`
   
  :ref:`vmod_var.get_ip`
   
  :ref:`vmod_var.set_backend`
   
  :ref:`vmod_var.get_backend`
   
  :ref:`vmod_var.clear`
   
This VMOD implements basic variable support in VCL.

It supports strings, integers, real numbers, durations, IP addresses
and backends. There are methods to get and set each data type.

Global variables have a lifespan that extends across requests and
VCLs, for as long as the vmod is loaded. Global variables only support
strings.

The remaining functions have PRIV_TASK lifespan and are local to a single
request or backend request.

.. vcl-start

Example::

    vcl 4.0;
    import var;

    backend default { .host = "192.0.2.11"; .port = "8080"; }

    sub vcl_recv {
        # Set and get some values.
        var.set("foo", "bar");
        set req.http.x-foo = var.get("foo");

        var.set_int("ten", 10);
        var.set_int("five", 5);
        set req.http.twenty = var.get_int("ten") + var.get_int("five") + 5;

        # VCL will use the first token to decide final data type. Headers are strings.
        # set req.http.X-lifetime = var.get_int("ten") + " seconds"; #  Won't work.
        set req.http.X-lifetime = "" + var.get_int("ten") + " seconds";  # Works!

        var.set_duration("timedelta", 1m);  # 60s
        set req.http.d1 = var.get_duration("timedelta");

        var.set_ip("endpoint", client.ip);
        set req.http.x-client = var.get_ip("endpoint");

        # Unset all non-global variables.
        var.clear();

        # Demonstrate use of global variables as state flags.
        if (req.url ~ "/close$") {
            var.global_set("open", "no");
        }
        else if (req.url ~ "/open$") {
            var.global_set("open", "yes");
        }

        if (var.global_get("open") != "yes") {
            return (synth(200, "We are currently closed, sorry!"));
        }
    }

.. vcl-end


.. _vmod_var.set:

VOID set(STRING key, STRING value)
----------------------------------

Set `key` to `value`.

.. _vmod_var.get:

STRING get(STRING)
------------------

Get `key` with data type STRING. If stored `key` is not a STRING an empty string is returned.

.. _vmod_var.global_set:

VOID global_set(STRING, STRING)
-------------------------------



.. _vmod_var.global_get:

STRING global_get(STRING)
-------------------------



.. _vmod_var.set_int:

VOID set_int(STRING key, INT value)
-----------------------------------

Set `key` to `value`.

.. _vmod_var.get_int:

INT get_int(STRING key)
-----------------------

Get `key` with data type INT. If stored `key` is not an INT zero will be returned.

.. _vmod_var.set_string:

VOID set_string(STRING key, STRING value)
-----------------------------------------

Identical to set().

.. _vmod_var.get_string:

STRING get_string(STRING key)
-----------------------------

Identical to get().

.. _vmod_var.set_real:

VOID set_real(STRING key, REAL value)
-------------------------------------

Set `key` to `value`.

.. _vmod_var.get_real:

REAL get_real(STRING key)
-------------------------

Get `key` with data type REAL. If stored `key` is not a REAL zero will be returned.

.. _vmod_var.set_duration:

VOID set_duration(STRING key, DURATION value)
---------------------------------------------

Set `key` to `value`.

.. _vmod_var.get_duration:

DURATION get_duration(STRING key)
---------------------------------

Get `key` with data type DURATION. If stored `key` is not a DURATION zero will be returned.

.. _vmod_var.set_ip:

VOID set_ip(STRING key, IP value)
---------------------------------

Set `key` to `value`.

.. _vmod_var.get_ip:

IP get_ip(STRING key)
---------------------

Get `key` with data type IP. If stored `key` is not an IP null will be returned.

.. _vmod_var.set_backend:

VOID set_backend(STRING key, BACKEND value)
-------------------------------------------

Set `key` to `value`.

.. _vmod_var.get_backend:

BACKEND get_backend(STRING key)
-------------------------------

Get `key` with data type BACKEND. If stored `key` is not a BACKEND,
null will be returned.

.. _vmod_var.clear:

VOID clear()
------------

Clear all non-global variables.
