#!/bin/sh

# OPENSSL=openssl
OPENSSL=/home/bin/openssl

CONF=/tmp/host_openssl.conf
AUTOKEY=/tmp/autokey.enter
SSL_PATH=/edvr/lighttpd/ssl
PEM_FILE=/tmp/server.pem

make_conf_file()
{
	echo -n "Make ${CONF} ($@)"
	echo "[ req ]" > ${CONF}
	echo "default_bits                   = 2048" >> ${CONF}
	echo "default_md                     = sha256" >> ${CONF}
	echo "default_keyfile                = ipc.key" >> ${CONF}
	echo "distinguished_name             = req_distinguished_name" >> ${CONF}
	echo "extensions                     = v3_user" >> ${CONF}
	echo " " >> ${CONF}
	echo "[ v3_user ]" >> ${CONF}
	echo "basicConstraints               = CA:FALSE" >> ${CONF}
	echo "authorityKeyIdentifier         = keyid,issuer" >> ${CONF}
	echo "subjectKeyIdentifier           = hash" >> ${CONF}
	echo "keyUsage                       = nonRepudiation, digitalSignature, keyEncipherment" >> ${CONF}
	echo "extendedKeyUsage               = serverAuth,clientAuth" >> ${CONF}
	echo "subjectAltName                 = @alt_names" >> ${CONF}
	echo " " >> ${CONF}
	echo "[ alt_names]" >> ${CONF}
	echo "DNS.1                          = *.dynlink.net" >> ${CONF}
	i=2
	for d in $@; do
		echo "DNS.${i}                   = ${d}" >> ${CONF}
		i=`expr ${i} + 1`
	done
	echo " " >> ${CONF}
	echo "[req_distinguished_name ]" >> ${CONF}
	echo "countryName                   = KR" >> ${CONF}
	echo "countryName_default           = KR" >> ${CONF}
	echo "countryName_min               = 2" >> ${CONF}
	echo "countryName_max               = 2" >> ${CONF}
	echo " " >> ${CONF}
	echo "organizationName              = ipc" >> ${CONF}
	echo "organizationName_default      = ipc" >> ${CONF}
	echo " " >> ${CONF}
	echo "organizationalUnitName        = IPC			#Organizational Unit Name (eg, section)" >> ${CONF}
	echo "organizationalUnitName_default= IPC			#lesstif SSL Project" >> ${CONF}
	echo " " >> ${CONF}
	echo "commonName                    = ipc" >> ${CONF}
	echo "commonName_default            = ipc" >> ${CONF}
	echo "commonName_max                = 64" >> ${CONF}
}

make_auto_key()
{
	echo "" > ${AUTOKEY}
	echo "" >> ${AUTOKEY}
	echo "" >> ${AUTOKEY}
	echo "" >> ${AUTOKEY}
}

make_key()
{
	if [ -d /tmp/server.pem ]; then
		return;
	fi
	cp ${SSL_PATH}/rootca_ipc.crt /tmp/ -a
	cp -a ${SSL_PATH}/rootca_ipc.crt /tmp/
	${OPENSSL} req -new -key ${SSL_PATH}/ipc.key -out /tmp/ipc.csr -config ${CONF} < ${AUTOKEY}
	${OPENSSL} x509 -sha256 -req -days 1825 -extensions v3_user -in /tmp/ipc.csr -CA /tmp/rootca_ipc.crt -CAcreateserial -CAkey ${SSL_PATH}/rootca_ipc.key -out /tmp/ipc.crt -extfile /tmp/host_openssl.conf

	cat /tmp/ipc.crt ${SSL_PATH}/ipc.key > /tmp/server.pem
}

echo -n "Make ${PEM_FILE} ($@)"
if [ -f ${PEM_FILE} ]; then
	echo -n " -> already exist!"
	exit 0
fi
make_conf_file $@
make_auto_key
make_key

exit 0
