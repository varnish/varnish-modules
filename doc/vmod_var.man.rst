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

-------------------------
Variable VMOD for Varnish
-------------------------

:Manual section: 3

SYNOPSIS
========

import var [from "path"] ;


DESCRIPTION
===========

Association list in VCL. Can be used to mimick variables.


CONTENTS
========

* VOID clear(PRIV_TASK)
* STRING get(PRIV_TASK, STRING)
* DURATION get_duration(PRIV_TASK, STRING)
* INT get_int(PRIV_TASK, STRING)
* IP get_ip(PRIV_TASK, STRING)
* REAL get_real(PRIV_TASK, STRING)
* STRING get_string(PRIV_TASK, STRING)
* STRING global_get(STRING)
* VOID global_set(STRING, STRING)
* VOID set(PRIV_TASK, STRING, STRING)
* VOID set_duration(PRIV_TASK, STRING, DURATION)
* VOID set_int(PRIV_TASK, STRING, INT)
* VOID set_ip(PRIV_TASK, STRING, IP)
* VOID set_real(PRIV_TASK, STRING, REAL)
* VOID set_string(PRIV_TASK, STRING, STRING)

.. _func_set_string:

VOID set_string(PRIV_TASK, STRING, STRING)
------------------------------------------

Prototype
	VOID set_string(PRIV_TASK, STRING, STRING)
Description
	Sets the variable identified by S to the value T.
Example
	var.set_string("bar", "some random string");

.. _func_get_string:

STRING get_string(PRIV_TASK, STRING)
------------------------------------

Prototype
	STRING get_string(PRIV_TASK, STRING)
Description
	Returns the string identified by the supplied string.
Example
	set resp.http.foo = var.get_string("bar");

Similar functions
-----------------

There are similar functions named:
.. _func_get:

STRING get(PRIV_TASK, STRING)
-----------------------------

Prototype
	STRING get(PRIV_TASK, STRING)

.. _func_get_duration:

DURATION get_duration(PRIV_TASK, STRING)
----------------------------------------

Prototype
	DURATION get_duration(PRIV_TASK, STRING)

.. _func_get_int:

INT get_int(PRIV_TASK, STRING)
------------------------------

Prototype
	INT get_int(PRIV_TASK, STRING)

.. _func_get_ip:

IP get_ip(PRIV_TASK, STRING)
----------------------------

Prototype
	IP get_ip(PRIV_TASK, STRING)

.. _func_get_real:

REAL get_real(PRIV_TASK, STRING)
--------------------------------

Prototype
	REAL get_real(PRIV_TASK, STRING)

.. _func_global_get:

STRING global_get(STRING)
-------------------------

Prototype
	STRING global_get(STRING)

.. _func_global_set:

VOID global_set(STRING, STRING)
-------------------------------

Prototype
	VOID global_set(STRING, STRING)

.. _func_set:

VOID set(PRIV_TASK, STRING, STRING)
-----------------------------------

Prototype
	VOID set(PRIV_TASK, STRING, STRING)

.. _func_set_duration:

VOID set_duration(PRIV_TASK, STRING, DURATION)
----------------------------------------------

Prototype
	VOID set_duration(PRIV_TASK, STRING, DURATION)

.. _func_set_int:

VOID set_int(PRIV_TASK, STRING, INT)
------------------------------------

Prototype
	VOID set_int(PRIV_TASK, STRING, INT)

.. _func_set_ip:

VOID set_ip(PRIV_TASK, STRING, IP)
----------------------------------

Prototype
	VOID set_ip(PRIV_TASK, STRING, IP)

.. _func_set_real:

VOID set_real(PRIV_TASK, STRING, REAL)
--------------------------------------

Prototype
	VOID set_real(PRIV_TASK, STRING, REAL)

.. _func_clear:

VOID clear(PRIV_TASK)
---------------------

Prototype
	VOID clear(PRIV_TASK)
Description
	Clears out all the variables.
Example
	clrea();



