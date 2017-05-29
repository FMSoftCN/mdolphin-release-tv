#!/bin/sh

echo n | libtoolize --copy --force
aclocal
autoheader
autoconf
automake --add-missing 

