#!/bin/sh

if [ -f /tmp/_key/priv.key.pem ]; then
	exit 0
fi

mkdir -p /tmp/_key
export OPENSSL_CONF=/etc/ssl/openssl.cnf
/home/bin/openssl genrsa -out /tmp/_key/priv.key.pem 1024
/home/bin/openssl rsa -pubout -in /tmp/_key/priv.key.pem -out /tmp/_key/pub.key.pem

exit 0
