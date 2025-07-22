#/bin/sh


export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/lib:/home/lib2

rm /mnt/ipm/cgisess_* 2>/dev/null
rm /mnt/ipm/lighttpd-upload-* 2>/dev/null

# echo "udhcpc start "
# udhcpc
# echo "sleep "
# sleep 2



modprobe na51055_wdt wdt_time=60
cd /edvr

/edvr/ecam &
/edvr/upgr_load &








