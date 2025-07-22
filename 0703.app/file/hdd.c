#include "common.h"
#include "file/ssfsearch.h"
#include "file/hdd.h"
#include "lib/threadsafe.h"



int usb_detect_physical (ST_HDDINFO hddinfo [1]) {

	return 0;
}

int __device_detect_physical(ST_HDDINFO * hddinfo, int type) {

	FILE * fp = NULL;
	char model [32];

	assert (hddinfo);
	memset (model, 0, sizeof (model));


	if ((fp = fopen (DEV_SD_MEM_BASE, "r")) == NULL) {
		goto out;
	}


	return 0;

out :
	if (fp) {
		fclose (fp);
		fp = NULL;
	}

	return -1;
}

int _detect_physical (ST_HDDINFO hddinfo [1]) {
	int i = 0, tot = 0;

	for ( ; i < 1; i ++) {
		hddinfo [i].id = i;	// Allocate HDD id here.

		if (__device_detect_physical (& hddinfo [i], TYPE_SDCARD) < 0)
		{
			continue;
		}

		tot ++;
	}

	return tot;
}

int g_fsck	= 0;	



int hdd_umount (ST_HDDINFO hddinfo [1]) {
	int i = 0, tot = 0;

	sync();

	system("umount -l /var/www/sda1");

	g_file.nr_mount = 0;

	return tot;
}

