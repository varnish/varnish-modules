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


The TCP vmod opens for access and modification of client TCP connection
attributes from VCL.

Primary use is for rate limiting (pacing) using the fq network scheduler on
recent Linux systems.

.. vcl-start

Example::

    vcl 4.0;

    import tcp;

    backend default { .host = "192.0.2.11"; .port = "8080"; }

    sub vcl_recv {
        # Shape (pace) the data rate to avoid filling router buffers for a
        # single client.
        if (req.url ~ ".mp4$") {
                tcp.set_socket_pace(1000);   # KB/s.
        }

        # We want to change the congestion control algorithm.
        if (tcp.get_estimated_rtt() > 300) {
            set req.http.x-tcp = tcp.congestion_algorithm("hybla");
        }
    }

.. vcl-end


CONTENTS
========

* :ref:`func_congestion_algorithm`
* :ref:`func_dump_info`
* :ref:`func_get_estimated_rtt`
* :ref:`func_set_socket_pace`

.. _func_congestion_algorithm:

congestion_algorithm
--------------------

::

	INT congestion_algorithm(STRING algorithm)

Set the client socket congestion control algorithm to S. Returns 0 on success, and -1 on error.

Example::

    sub vcl_recv {
        set req.http.x-tcp = tcp.congestion_algorithm("cubic");
    }


.. _func_dump_info:

dump_info
---------

::

	VOID dump_info()

Write the contents of the TCP_INFO data structure into varnishlog.

Example::

    tcp.dump_info();


Example varnishlog output::

        -   VCL_Log        tcpi: snd_mss=1448 rcv_mss=536 lost=0 retrans=0
        -   VCL_Log        tcpi2: pmtu=1500 rtt=152000 rttvar=76000 snd_cwnd=10 advmss=1448 reordering=3
        -   VCL_Log        getsockopt() returned: cubic




.. _func_get_estimated_rtt:

get_estimated_rtt
-----------------

::

	REAL get_estimated_rtt()

Get the estimated round-trip-time for the client socket. Unit: milliseconds.

Example::

    if (tcp.get_estimated_rtt() > 300) {
        std.log("client is far away.");
    }


.. _func_set_socket_pace:

set_socket_pace
---------------

::

	VOID set_socket_pace(INT)

Set socket pacing on client-side TCP connection to PACE KB/s. Network interface
used must be using a supported scheduler. (fq)

Example::

    tcp.set_socket_pace(1000);


