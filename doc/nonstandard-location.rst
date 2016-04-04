Installing to a non-standard location
=====================================

If you have installed Varnish to a non-standard directory, call
``autogen.sh`` and ``configure`` with ``PKG_CONFIG_PATH`` pointing to
the appropriate path. For example, when varnishd configure was called
with ``--prefix=$PREFIX``, use

 PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig
 export PKG_CONFIG_PATH

Make and install the vmods::
 
 make           # builds the vmods
 make install   # installs to `VMODDIR`
 make check     # runs the unit tests in ``src/tests/*.vtc``

