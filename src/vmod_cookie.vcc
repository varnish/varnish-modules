$Module cookie 3 Varnish Cookie Module
DESCRIPTION
===========

Handle HTTP cookies easier in Varnish VCL. (without regex)

Parses a cookie header into an internal data store, where per-cookie
get/set/delete functions are available. A filter_except() method removes all
but a set comma-separated list of cookies.

A convenience function for formatting the Set-Cookie Expires date field
is also included. It might be needed to use libvmod-header if there might
be multiple Set-Cookie response headers.

Important: The state loaded with cookie.parse() can only be guaranteed
within a single VCL call. Do the cookie operations in vcl_recv, and store
any additional variables you need later on as fake request headers.

It is currently not safe/tested to call this VMOD in any fetch threads.
(vcl_backend_fetch and similar)

$Function VOID clean(PRIV_TASK)
Description
        Clean up all previously parse()-d cookies. Probably of limited
        use. It is not necessary to run clean() in normal operation.
Example
        ::

                sub vcl_recv {
                        cookie.clean();
                }

$Function VOID delete(PRIV_TASK, STRING)
Description
        Delete a cookie from internal vmod storage if it exists.

Example
        ::

		sub vcl_recv {
		    cookie.parse("cookie1: value1; cookie2: value2;");
		    cookie.delete("cookie2");
		    // get_string() will now yield "cookie1: value1";
		}

$Function VOID filter_except(PRIV_TASK, STRING)
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

$Function STRING format_rfc1123(TIME, DURATION)
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

$Function STRING get(PRIV_TASK, STRING)
Description
        Get the value of a cookie, as stored in internal vmod storage. If the cookie name does not exists, an empty string is returned.

Example
        ::

                import std;
                sub vcl_recv {
                        cookie.parse("cookie1: value1; cookie2: value2;");
                        std.log("cookie1 value is: " + cookie.get("cookie1"));
                }

$Function STRING get_string(PRIV_TASK)
Description
        Get a Cookie string value with all cookies in internal vmod storage.
Example
        ::

                sub vcl_recv {
                        cookie.parse(req.http.cookie);
                        cookie.filter_except("SESSIONID,PHPSESSID");
                        set req.http.cookie = cookie.get_string();
                }

$Function BOOL isset(PRIV_TASK, STRING)
Description
        Check if a given cookie is set in the internal vmod storage.

Example
        ::

                import std;
                sub vcl_recv {
                        cookie.parse("cookie1: value1; cookie2: value2;");
                        if (cookie.isset("cookie2")) {
                                std.log("cookie2 is set.");
                        }
                }

$Function VOID parse(PRIV_TASK, STRING)
Description
        Parse the cookie string in string S. The parsed values are only guaranteed
        to exist within a single VCL function. Implicit clean() if run more than once.
Example
        ::

                sub vcl_recv {
                        cookie.parse(req.http.Cookie);
                }



$Function VOID set(PRIV_TASK, STRING, STRING)
Description
        Set the internal vmod storage value for a cookie to a value.

Example
        ::

                sub vcl_recv {
                        cookie.set("cookie1", "value1");
                        std.log("cookie1 value is: " + cookie.get("cookie1"));
                }
