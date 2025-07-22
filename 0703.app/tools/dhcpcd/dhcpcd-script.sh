#!/bin/sh


if [ ${reason} == "BOUND" ]
then
	RESOLV_CONF="/etc/resolv.conf"

	NET_CONF="/usr/share/udhcpc/netinfo"
	/bin/rm -rf $NET_CONF

	[ -n "$new_broadcast_address" ] && BROADCAST="broadcast $new_broadcast_address"
	[ -n "$new_subnet_mask" ] && NETMASK="netmask $new_subnet_mask"


	echo "IP[$new_ip_address]" > $NET_CONF
	echo "MASK[$new_subnet_mask]" >> $NET_CONF

	if [ -n "$new_routers" ]
	then
		for i in $new_routers
		do
			echo "GW[$i]" >> $NET_CONF
		done
	fi

	echo -n > $RESOLV_CONF
	#[ -n "$domain" ] && echo domain $domain >> $RESOLV_CONF
	for i in $new_domain_name_servers
	do
		echo "DNS[$i]" >> $NET_CONF
	done
fi
