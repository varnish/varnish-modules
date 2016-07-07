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

VOID set(PRIV_TASK, STRING, STRING)
-----------------------------------

Prototype
	VOID set(PRIV_TASK, STRING key, STRING value)

Set `key` to `value`.

.. _func_get:

STRING get(PRIV_TASK, STRING)
-----------------------------

Prototype
	STRING get(PRIV_TASK, STRING)

Get `key` with data type STRING. If stored `key` is not a STRING an empty string is returned.

.. _func_global_set:

VOID global_set(STRING, STRING)
-------------------------------

Prototype
	VOID global_set(STRING, STRING)

.. _func_global_get:

STRING global_get(STRING)
-------------------------

Prototype
	STRING global_get(STRING)

.. _func_set_int:

VOID set_int(PRIV_TASK, STRING, INT)
------------------------------------

Prototype
	VOID set_int(PRIV_TASK, STRING key, INT value)

Set `key` to `value`.

.. _func_get_int:

INT get_int(PRIV_TASK, STRING)
------------------------------

Prototype
	INT get_int(PRIV_TASK, STRING key)

Get `key` with data type INT. If stored `key` is not an INT zero will be returned.

.. _func_set_string:

VOID set_string(PRIV_TASK, STRING, STRING)
------------------------------------------

Prototype
	VOID set_string(PRIV_TASK, STRING key, STRING value)

Identical to set().

.. _func_get_string:

STRING get_string(PRIV_TASK, STRING)
------------------------------------

Prototype
	STRING get_string(PRIV_TASK, STRING key)

Identical to get().

.. _func_set_real:

VOID set_real(PRIV_TASK, STRING, REAL)
--------------------------------------

Prototype
	VOID set_real(PRIV_TASK, STRING key, REAL value)

Set `key` to `value`.

.. _func_get_real:

REAL get_real(PRIV_TASK, STRING)
--------------------------------

Prototype
	REAL get_real(PRIV_TASK, STRING key)

Get `key` with data type REAL. If stored `key` is not a REAL zero will be returned.

.. _func_set_duration:

VOID set_duration(PRIV_TASK, STRING, DURATION)
----------------------------------------------

Prototype
	VOID set_duration(PRIV_TASK, STRING key, DURATION value)

Set `key` to `value`.

.. _func_get_duration:

DURATION get_duration(PRIV_TASK, STRING)
----------------------------------------

Prototype
	DURATION get_duration(PRIV_TASK, STRING key)

Get `key` with data type DURATION. If stored `key` is not a DURATION zero will be returned.

.. _func_set_ip:

VOID set_ip(PRIV_TASK, STRING, IP)
----------------------------------

Prototype
	VOID set_ip(PRIV_TASK, STRING key, IP value)

Set `key` to `value`.

.. _func_get_ip:

IP get_ip(PRIV_TASK, STRING)
----------------------------

Prototype
	IP get_ip(PRIV_TASK, STRING key)

Get `key` with data type IP. If stored `key` is not an IP null will be returned.

.. _func_clear:

VOID clear(PRIV_TASK)
---------------------

Prototype
	VOID clear(PRIV_TASK)

Clear all non-global variables.
