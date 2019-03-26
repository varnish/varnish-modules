..
.. NB:  This file is machine generated, DO NOT EDIT!
..
.. Edit vmod.vcc and run make instead
..


:tocdepth: 1

.. _vmod_cookie(3):

===================================
VMOD cookie - Varnish Cookie Module
===================================

SYNOPSIS
========

.. parsed-literal::

  import cookie [from "path"]
  
  :ref:`vmod_cookie.clean`
   
  :ref:`vmod_cookie.delete`
   
  :ref:`vmod_cookie.filter`
   
  :ref:`vmod_cookie.filter_except`
   
  :ref:`vmod_cookie.format_rfc1123`
   
  :ref:`vmod_cookie.get`
   
  :ref:`vmod_cookie.get_string`
   
  :ref:`vmod_cookie.isset`
   
  :ref:`vmod_cookie.parse`
   
  :ref:`vmod_cookie.set`
   
DESCRIPTION
===========

Handle HTTP cookies easier in Varnish VCL. (without regex)

Parses a cookie header into an internal data store, where per-cookie
get/set/delete functions are available. A filter_except() method removes all
but a set comma-separated list of cookies. A filter() method removes a comma-
separated list of cookies.

A convenience function for formatting the Set-Cookie Expires date field
is also included. If there are multiple Set-Cookie headers vmod-header
should be used.

The state loaded with cookie.parse() has a lifetime of the current request
or backend request context. To pass variables to the backend request, store
the contents as fake bereq headers.

.. vcl-start

Filtering example::

    vcl 4.0;

    import cookie;

    backend default { .host = "192.0.2.11"; .port = "8080"; }

    sub vcl_recv {
        if (req.http.cookie) {
            cookie.parse(req.http.cookie);
            # Either delete the ones you want to get rid of:
            cookie.delete("cookie2");
            # or filter everything but a few:
            cookie.filter_except("SESSIONID,PHPSESSID");

            # Store it back into req so it will be passed to the backend.
            set req.http.cookie = cookie.get_string();

            # If empty, unset so the builtin VCL can consider it for caching.
            if (req.http.cookie == "") {
                unset req.http.cookie;
            }
        }
    }

.. vcl-end


.. _vmod_cookie.clean:

VOID clean()
------------

Description
        Clean up previously parsed cookies. It is not necessary to run clean()
        in normal operations.
Example
        ::

                sub vcl_recv {
                        cookie.clean();
                }

.. _vmod_cookie.delete:

VOID delete(STRING cookiename)
------------------------------

Description
        Delete `cookiename` from internal vmod storage if it exists.

Example
        ::

		sub vcl_recv {
		    cookie.parse("cookie1: value1; cookie2: value2;");
		    cookie.delete("cookie2");
		    // get_string() will now yield "cookie1: value1";
		}

.. _vmod_cookie.filter:

VOID filter(STRING filterstring)
--------------------------------

Description
        Delete all cookies from internal vmod storage that are in the
        comma-separated argument cookienames.

Example
        ::

                sub vcl_recv {
                        cookie.parse("cookie1: value1; cookie2: value2; cookie3: value3");
                        cookie.filter("cookie1,cookie2");
                        // get_string() will now yield
                        // "cookie3: value3";
                }

.. _vmod_cookie.filter_except:

VOID filter_except(STRING filterstring)
---------------------------------------

Description
        Delete all cookies from internal vmod storage that is not in the
        comma-separated argument cookienames.
Example
        ::

                sub vcl_recv {
                        cookie.parse("cookie1: value1; cookie2: value2; cookie3: value3");
                        cookie.filter_except("cookie1,cookie2");
                        // get_string() will now yield
                        // "cookie1: value1; cookie2: value2;";
                }

.. _vmod_cookie.format_rfc1123:

STRING format_rfc1123(TIME now, DURATION timedelta)
---------------------------------------------------

Description
        Get a RFC1123 formatted date string suitable for inclusion in a
        Set-Cookie response header.

        Care should be taken if the response has multiple Set-Cookie headers.
        In that case the header vmod should be used.
Example
        ::

                sub vcl_deliver {
                        # Set a userid cookie on the client that lives for 5 minutes.
                        set resp.http.Set-Cookie = "userid=" + req.http.userid + "; Expires=" + cookie.format_rfc1123(now, 5m) + "; httpOnly";
                }

.. _vmod_cookie.get:

STRING get(STRING cookiename)
-----------------------------

Description
        Get the value of `cookiename`, as stored in internal vmod storage. If `cookiename` does not exist an empty string is returned.
Example
        ::

                import std;
                sub vcl_recv {
                        cookie.parse("cookie1: value1; cookie2: value2;");
                        std.log("cookie1 value is: " + cookie.get("cookie1"));
                }

.. _vmod_cookie.get_string:

STRING get_string()
-------------------

Description
        Get a Cookie string value with all cookies in internal vmod storage. Does
	not modify internal storage.
Example
        ::

                sub vcl_recv {
                        cookie.parse(req.http.cookie);
                        cookie.filter_except("SESSIONID,PHPSESSID");
                        set req.http.cookie = cookie.get_string();
                }

.. _vmod_cookie.isset:

BOOL isset(STRING cookiename)
-----------------------------

Description
        Check if `cookiename` is set in the internal vmod storage.

Example
        ::

                import std;
                sub vcl_recv {
                        cookie.parse("cookie1: value1; cookie2: value2;");
                        if (cookie.isset("cookie2")) {
                                std.log("cookie2 is set.");
                        }
                }

.. _vmod_cookie.parse:

VOID parse(STRING cookieheader)
-------------------------------

Description
        Parse the cookie string in `cookieheader`. If state already exists, clean() will be run first.
Example
        ::

                sub vcl_recv {
                        cookie.parse(req.http.Cookie);
                }

.. _vmod_cookie.set:

VOID set(STRING cookiename, STRING value)
-----------------------------------------

Description
        Set the internal vmod storage for `cookiename` to `value`.

Example
        ::

                sub vcl_recv {
                        cookie.set("cookie1", "value1");
                        std.log("cookie1 value is: " + cookie.get("cookie1"));
                }
