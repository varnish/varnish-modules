..
.. NB:  This file is machine generated, DO NOT EDIT!
..
.. Edit vmod.vcc and run make instead
..

.. role:: ref(emphasis)

.. _vmod_var(3):

========
vmod_var
========

--------------------------------
Variable support for Varnish VCL
--------------------------------

:Manual section: 3

SYNOPSIS
========

import var [from "path"] ;


This VMOD implements basic variable support in VCL.

It supports strings, integers and real numbers. There are methods to get and
set each data type.

Global variables have a lifespan that extends across requests and
VCLs, for as long as the vmod is loaded.

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


CONTENTS
========

* :ref:`func_clear`
* :ref:`func_get`
* :ref:`func_get_duration`
* :ref:`func_get_int`
* :ref:`func_get_ip`
* :ref:`func_get_real`
* :ref:`func_get_string`
* :ref:`func_global_get`
* :ref:`func_global_set`
* :ref:`func_set`
* :ref:`func_set_duration`
* :ref:`func_set_int`
* :ref:`func_set_ip`
* :ref:`func_set_real`
* :ref:`func_set_string`

.. _func_set:

set
---

::

	VOID set(PRIV_TASK, STRING key, STRING value)

Set `key` to `value`.

.. _func_get:

get
---

::

	STRING get(PRIV_TASK, STRING)

Get `key` with data type STRING. If stored `key` is not a STRING an empty string is returned.

.. _func_global_set:

global_set
----------

::

	VOID global_set(STRING, STRING)

.. _func_global_get:

global_get
----------

::

	STRING global_get(STRING)

.. _func_set_int:

set_int
-------

::

	VOID set_int(PRIV_TASK, STRING key, INT value)

Set `key` to `value`.

.. _func_get_int:

get_int
-------

::

	INT get_int(PRIV_TASK, STRING key)

Get `key` with data type INT. If stored `key` is not an INT zero will be returned.

.. _func_set_string:

set_string
----------

::

	VOID set_string(PRIV_TASK, STRING key, STRING value)

Identical to set().

.. _func_get_string:

get_string
----------

::

	STRING get_string(PRIV_TASK, STRING key)

Identical to get().

.. _func_set_real:

set_real
--------

::

	VOID set_real(PRIV_TASK, STRING key, REAL value)

Set `key` to `value`.

.. _func_get_real:

get_real
--------

::

	REAL get_real(PRIV_TASK, STRING key)

Get `key` with data type REAL. If stored `key` is not a REAL zero will be returned.

.. _func_set_duration:

set_duration
------------

::

	VOID set_duration(PRIV_TASK, STRING key, DURATION value)

Set `key` to `value`.

.. _func_get_duration:

get_duration
------------

::

	DURATION get_duration(PRIV_TASK, STRING key)

Get `key` with data type DURATION. If stored `key` is not a DURATION zero will be returned.

.. _func_set_ip:

set_ip
------

::

	VOID set_ip(PRIV_TASK, STRING key, IP value)

Set `key` to `value`.

.. _func_get_ip:

get_ip
------

::

	IP get_ip(PRIV_TASK, STRING key)

Get `key` with data type IP. If stored `key` is not an IP null will be returned.

.. _func_clear:

clear
-----

::

	VOID clear(PRIV_TASK)

Clear all non-global variables.

