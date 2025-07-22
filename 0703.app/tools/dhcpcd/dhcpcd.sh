#!/bin/sh

PATH=/edvr/dhcpcd
DHCPCD=/home/bin/dhcpcd
SCRIPT=${PATH}/dhcpcd-script.sh

if [ ."$2" == ."" ]; then
	TMOUT=3
else
	TMOUT=$2
fi

start() 
{
	echo -n "Run dhcpcd: "
	
	${DHCPCD} -c ${SCRIPT} -t ${TMOUT}

	var=0
	while [ ${var} -lt ${TMOUT} ]
	do
		echo -n "."
		/bin/sleep 1
		var=`/usr/bin/expr ${var} + 1`
		if [ -f /usr/share/udhcpc/netinfo ]; then
			break
		fi
	done	

	echo "Success"
}

stop() 
{
	echo -n "Stopping dhcpcd: "
	
	/usr/bin/killall dhcpcd
	/bin/rm -rf /usr/share/udhcpc/netinfo

	echo "Success"
}

case $1 in
	start) start;;
	stop) stop;;
	restart) stop; start;;
esac


