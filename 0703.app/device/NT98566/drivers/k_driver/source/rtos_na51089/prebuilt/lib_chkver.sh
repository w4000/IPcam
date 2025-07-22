#!/bin/bash

if [ "$1" = "-csv" ]
then

for LIBA in $(find . -name "*.a")
do
	VERINFO=`$OBJDUMP -h $LIBA | grep ".version.info.*2\*\*2" | awk -F" " '{print $2}' | awk -F"." '{print $5 "," $4}' | sed -n 's/_/./gp'`
	if [ -z "${VERINFO:-}" ]; then
		echo $LIBA
	else
		echo -e "$LIBA,$VERINFO"
	fi
done

else

for LIBA in $(find . -name "*.a")
do
	VERINFO=`$OBJDUMP -h $LIBA | grep ".version.info.*2\*\*2" | awk -F" " '{print $2}' | awk -F"." '{gsub(/_/,".",$4)}; {printf "%-50s %19s %s", "'${LIBA}'", $5, $4}'`
	if [ -z "${VERINFO:-}" ]; then
		echo $LIBA
	else
		echo -e "$VERINFO"
	fi
done

fi

