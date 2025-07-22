#!/bin/sh

CONF_FILE=$1

if [ ."${CONF_FILE}" == ."" ]; then
	CONF_FILE=rootca_openssl.conf
fi

#####################################################################
# Reference: https://www.lesstif.com/pages/viewpage.action?pageId=6979614

#####################################################################
# make rootca
/home/bin/openssl genrsa -aes256 -out ipc.key.enc 2048

cp  ipc.key.enc  ipc.key
/home/bin/openssl rsa -in ipc.key.enc -out ipc.key

/home/bin/openssl req -new -key ipc.key.enc -out ecam.csr -config ${CONF_FILE}

/home/bin/openssl x509 -sha256 -req -days 3650 -extensions v3_ca -set_serial 1 -in ecam.csr -signkey ipc.key.enc -out ecam.crt -extfile ${CONF_FILE}

#openssl x509 -text -in ecam.crt

#####################################################################
# make ssl
/home/bin/openssl genrsa -aes256 -out ipc.key.enc 2048

cp ipc.key.enc ipc.key
/home/bin/openssl rsa -in ipc.key.enc -out ipc.key

cp ipc.key ../
cp ecam.crt ../
cp ipc.key ../

rm ipc*
rm ecam*
