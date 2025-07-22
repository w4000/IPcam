#!/bin/sh

NAME=mume
DIR=/usr/local/magux/iot
PIDFILE=/tmp/$NAME.pid
DAEMON=${DIR}/${NAME}
STOP_SIGNAL=INT
LOG=/tmp/$NAME.log

export PATH="${PATH:+$PATH:}/usr/sbin:/sbin"

common_opts="--quiet --pidfile $PIDFILE"

do_start(){
	start-stop-daemon -S $common_opts --make-pidfile --background --startas \
		/bin/sh -- -c "exec $DAEMON $DAEMON_ARGS > $LOG 2>&1"
}

do_stop(){
	opt=${@:-}
	start-stop-daemon -K $common_opts --signal $STOP_SIGNAL --oknodo $opt 
}

case "$1" in
	start)
		echo -n "Starting daemon: "$NAME
		do_start
		echo "."
		;;
	stop)
		echo -n "Stopping daemon: "$NAME
		do_stop
		echo "."
		;;
	restart)
		echo -n "Restarting daemon: "$NAME
		do_stop --retry 30
		do_start
		echo "."
		;;
	*)
		echo "Usage: "$1" {start|stop|restart}"
		exit 1
esac

exit 0
