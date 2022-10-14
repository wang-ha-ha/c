#!/bin/sh

export LD_LIBRARY_PATH=/html/httpd
cd /html/httpd
./lighttpd -f ./lighttpd.conf
