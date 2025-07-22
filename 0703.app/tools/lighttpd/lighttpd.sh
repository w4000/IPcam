#!/bin/sh

LIGHTTPD=/home/bin/lighttpd
CONF=/etc/lighttpd.conf
MODULE_PATH=/home/lib
PORT=$2
HTTPS_PORT=$3
RETURN=0

conf_file() 
{
#	if [ ! -f ${CONF} ]; then
		echo -n " Default setting..."
		cp /edvr/lighttpd/lighttpd.conf.default ${CONF}
#	fi
}

change_port()
{
	if [ ."${PORT}" == ."" ]; then
		return 0
	fi

	OFFSET=1
	OFFSET_ANNOTATION=4
	# MAIN_PORT => default HTTP
	# SUB_PORT => default HTTPS
	MAIN_PORT_LINE=`grep -n 'server.port' ${CONF} | sed -e 's;:.*$;;'`
	MAIN_PORT_LINE=`echo ${MAIN_PORT_LINE} ${OFFSET} | awk '{print $1+$2}'`
	SUB_PORT_LINE=`grep -n 'var.httpsport =' ${CONF} | sed -e 's;:.*$;;'`
	DIS_SUB_PORT_LINE=`grep -n '#var.httpsport =' ${CONF} | sed -e 's;:.*$;;'`
	#echo ""
	#echo "SUB_PORT_LINE = "$SUB_PORT_LINE
	#echo "DIS_SUB_PORT_LINE = "$DIS_SUB_PORT_LINE

	if [ ${PORT} -eq ${PORT} 2> /dev/null ]; then
		if [ ${PORT} -eq 0 ]; then
			if [ ."${HTTPS_PORT}" == ."" -o ${HTTPS_PORT} -eq 0 ]; then
				# should shut down lighttpd
				echo " HTTP Disabled and HTTPS Disabled"
				RETURN=1
				return 1
			else
				echo " HTTP Disabled and HTTPS Enabled"
				echo " Port:" ${PORT}
				echo " HTTPS Port:" ${HTTPS_PORT}
				sed -i "s/\(server.port *= *\).*/\1${HTTPS_PORT}/" ${CONF}
				sed -i "${MAIN_PORT_LINE},${MAIN_PORT_LINE}"" s/disable/enable/" ${CONF}
				# to prevent applying annotation duplicately
				if [ ."${DIS_SUB_PORT_LINE}" == ."" ]; then
					echo " HTTPS annotation enabled"
					sed -i "${SUB_PORT_LINE},${SUB_PORT_LINE}"" s/var.httpsport/#var.httpsport/" ${CONF}
					SUB_PORT_LINE_START=`echo ${SUB_PORT_LINE} ${OFFSET} | awk '{print $1+$2}'`
					SUB_PORT_LINE_END=`echo ${SUB_PORT_LINE_START} ${OFFSET_ANNOTATION} | awk '{print $1+$2}'`
					sed -i "${SUB_PORT_LINE_START},${SUB_PORT_LINE_END}"" s/^/#/" ${CONF}
				fi
				return 0
			fi
		else
			echo " HTTP Enabled"
			echo " Port:" ${PORT}
			sed -i "s/\(server.port *= *\).*/\1${PORT}/" ${CONF}
			sed -i "${MAIN_PORT_LINE},${MAIN_PORT_LINE}"' s/enable/disable/' ${CONF}
		fi
	fi

	if [ ."${HTTPS_PORT}" == ."" ]; then
		if [ ."${DIS_SUB_PORT_LINE}" == ."" ]; then
			#echo " HTTPS annotation enabled"
			sed -i "${SUB_PORT_LINE},${SUB_PORT_LINE}"" s/var.httpsport/#var.httpsport/" ${CONF}
			SUB_PORT_LINE_START=`echo ${SUB_PORT_LINE} ${OFFSET} | awk '{print $1+$2}'`
			SUB_PORT_LINE_END=`echo ${SUB_PORT_LINE_START} ${OFFSET_ANNOTATION} | awk '{print $1+$2}'`
			sed -i "${SUB_PORT_LINE_START},${SUB_PORT_LINE_END}"" s/^/#/" ${CONF}
		fi
		return 0
	fi

	if [ ${HTTPS_PORT} -eq ${HTTPS_PORT} 2> /dev/null -a ${HTTPS_PORT} -ne 0 ]; then
		echo " HTTPS Enabled"
		echo " HTTPS Port:" ${HTTPS_PORT}
		sed -i "s/\(var.httpsport *= *\).*/\1${HTTPS_PORT}/" ${CONF}
		if [ -z ${DIS_SUB_PORT_LINE} ]; then
			#echo " DIS_SUB_PORT_LINE is NULL"
			echo ""
		else
			#echo " HTTPS annotation disabled"
			sed -i "${DIS_SUB_PORT_LINE},${DIS_SUB_PORT_LINE}"" s/#var.httpsport/var.httpsport/" ${CONF}
			DIS_SUB_PORT_LINE_START=`echo ${DIS_SUB_PORT_LINE} ${OFFSET} | awk '{print $1+$2}'`
			DIS_SUB_PORT_LINE_END=`echo ${DIS_SUB_PORT_LINE_START} ${OFFSET_ANNOTATION} | awk '{print $1+$2}'`
			sed -i "${DIS_SUB_PORT_LINE_START},${DIS_SUB_PORT_LINE_END}"" s/^#//" ${CONF}
		fi
	fi
}

create_enc_key()
{
	/edvr/lighttpd/create_enc_key.sh
}

start() 
{
	echo -n "Starting lighttpd:"
	conf_file
	change_port
	# echo "RETURN = "$RETURN
	if [ $RETURN -eq 1 ]; then
		echo "  [Fail - Change Port]"
		return 1
	fi
	create_enc_key
	${LIGHTTPD} -f ${CONF} -m ${MODULE_PATH}
	if test $? != 0; then
		echo "  [Fail]"
		return 1
	else
		touch -f /tmp/
		echo "  [Success]"
	fi
}

stop() 
{
	echo -n "Stopping lighttpd:"
	if [ -f /tmp/lighttpd/lighttpd.pid ]; then
		PID=`cat /tmp/lighttpd/lighttpd.pid`
		kill ${PID}
	fi

	if test $? != 0; then
		echo "  [Fail]"
	else
		echo "  [Success]"
	fi
}

if [ ! -d /tmp/lighttpd ]; then
	mkdir -p /tmp/lighttpd
fi

case $1 in
	start) start;;
	stop) stop;;
	restart) stop; start;;
esac

exit 0
