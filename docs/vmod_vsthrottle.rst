..
.. NB:  This file is machine generated, DO NOT EDIT!
..
.. Edit vmod.vcc and run make instead
..

.. role:: ref(emphasis)

.. _vmod_vsthrottle(3):

===============
vmod_vsthrottle
===============

---------------
Throttling VMOD
---------------

:Manual section: 3

SYNOPSIS
========

import vsthrottle [from "path"] ;

DESCRIPTION
===========

A Varnish vmod for rate-limiting traffic on a single Varnish
server. Offers a simple interface for throttling traffic on a per-key
basis to a specific request rate.

Keys can be specified from any VCL string, e.g. based on client.ip, a
specific cookie value, an API token, etc.

The request rate is specified as the number of requests permitted over
a period. To keep things simple, this is passed as two separate
parameters, 'limit' and 'period'.

This VMOD implements a `token bucket algorithm`_. State associated
with the token bucket for each key is stored in-memory using BSD's
red-black tree implementation.

Memory usage is around 100 bytes per key tracked.

.. _token bucket algorithm: http://en.wikipedia.org/wiki/Token_bucket


.. vcl-start

Example::

    vcl 4.0;
    import vsthrottle;

    backend default { .host = "192.0.2.11"; .port = "8080"; }

    sub vcl_recv {
        # Varnish will set client.identity for you based on client IP.

        if (vsthrottle.is_denied(client.identity, 15, 10s)) {
            # Client has exceeded 15 reqs per 10s
            return (synth(429, "Too Many Requests"));
        }

        # There is a quota per API key that must be fulfilled.
        if (vsthrottle.is_denied("apikey:" + req.http.Key, 30, 60s)) {
                return (synth(429, "Too Many Requests"));
        }

        # Only allow a few POST/PUTs per client.
        if (req.method == "POST" || req.method == "PUT") {
            if (vsthrottle.is_denied("rw" + client.identity, 2, 10s)) {
                return (synth(429, "Too Many Requests"));
            }
        }
    }

.. vcl-end

CONTENTS
========

* :ref:`func_is_denied`
* :ref:`func_remaining`

.. _func_is_denied:

is_denied
---------

::

	BOOL is_denied(STRING key, INT limit, DURATION period)

Arguments:

  - key: A unique identifier to define what is being throttled - more examples below
  - limit: How many requests in the specified period
  - period: The time period

Description
  Can be used to rate limit the traffic for a specific key to a
  maximum of 'limit' requests per 'period' time. A token bucket
  is uniquely identified by the triplet of its key, limit and
  period, so using the same key multiple places with different
  rules will create multiple token buckets.

Example
        ::

		sub vcl_recv {
			if (vsthrottle.is_denied(client.identity, 15, 10s)) {
				# Client has exceeded 15 reqs per 10s
				return (synth(429, "Too Many Requests"));
			}

			# ...
		}


.. _func_remaining:

remaining
---------

::

	INT remaining(STRING key, INT limit, DURATION period)

Arguments:
  - key: A unique identifier to define what is being throttled
  - limit: How many requests in the specified period
  - period: The time period

Description

  Get the current number of tokens for a given token bucket. This can
  be used to create a response header to inform clients of their
  current quota.


Example
  ::

     sub vcl_deliver {
	set resp.http.X-RateLimit-Remaining = vsthrottle.remaining(client.identity, 15, 10s);
     }

