..
.. NB:  This file is machine generated, DO NOT EDIT!
..
.. Edit vmod.vcc and run make instead
..

.. role:: ref(emphasis)

.. _vmod_tcp(3):

========
vmod_tcp
========

--------
TCP vmod
--------

:Manual section: 3

SYNOPSIS
========

import tcp [from "path"] ;

DESCRIPTION
===========

The TCP vmod is a set of functions that accesses and changes attributes on the
(client) TCP connection. It allows you to access TCP_INFO data, and also change
the congestion control algorithm used.

Varnish Cache 4.1 and newer is supported.

The VMOD is Linux-specific and requires a recent kernel to be useful. (>=3.13)


CONTENTS
========

* :ref:`func_congestion_algorithm`
* :ref:`func_dump_info`
* :ref:`func_get_estimated_rtt`
* :ref:`func_set_socket_pace`

.. _func_congestion_algorithm:

INT congestion_algorithm(STRING)
--------------------------------

Prototype
	INT congestion_algorithm(STRING algorithm)

Set the client socket congestion control algorithm to S. Returns 0 on success, and -1 on error.

Example::

    sub vcl_recv {
        set req.http.x-tcp = tcp.congestion_algorithm("cubic");
    }


.. _func_dump_info:

VOID dump_info()
----------------

Prototype
	VOID dump_info()

Write the contents of the TCP_INFO data structure into varnishlog.

Example
        ::

                tcp.dump_info();


.. _func_get_estimated_rtt:

REAL get_estimated_rtt()
------------------------

Prototype
	REAL get_estimated_rtt()

Get the estimated round-trip-time for the client socket. Unit: milliseconds.

Example::

                if (tcp.get_estimated_rtt() > 300) {
                    std.log("client is far away.");
                }


.. _func_set_socket_pace:

VOID set_socket_pace(INT)
-------------------------

Prototype
	VOID set_socket_pace(INT)

Set socket pacing on client-side TCP connection to PACE KB/s. Network interface
used must be using a supported scheduler. (fq)

Example::

                tcp.set_socket_pace(1000);

Usage
-----

In your VCL you could then use this vmod along the following lines::

        import tcp;

        sub vcl_recv {
                tcp.dump_info();
                if (tcp.get_estimated_rtt() > 300) {
                    set req.http.x-tcp = tcp.congestion_algorithm("hybla");
                }
        }

Example varnishlog output from dump_info()::

        -   VCL_Log        tcpi: snd_mss=1448 rcv_mss=536 lost=0 retrans=0
        -   VCL_Log        tcpi2: pmtu=1500 rtt=152000 rttvar=76000 snd_cwnd=10 advmss=1448 reordering=3
        -   VCL_Log        getsockopt() returned: cubic
