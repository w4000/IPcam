/*
 * watchdog.c
 *
 *   Purpose:
 *	 - System watchdog implementation.
 *
 *****************************************************************************/
#include "watchdog.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "./watchdog.h"

/***********************************************************************************
 * Member functions of object.
 ***********************************************************************************/

static int load_driver = 1;

int watchdog_resettimer () {

	int fd = open("/dev/watchdog", O_RDWR);
	if (fd >= 0)
	{
		ioctl(fd, WDIOC_KEEPALIVE, NULL);
		close(fd);
	}



	return 0;
}

void watchdog_init () {


	int timeout = 89;
	struct watchdog_info ident;
	int fd = open("/dev/watchdog", O_RDWR);
	if(fd < 0) {
		load_driver = 0;
		return;
	}

	if(ioctl(fd, WDIOC_GETSUPPORT, &ident) != 0) {
		printf("Not Support WDIOC_GETSUPPORT!!!\n");
		goto End;
	}


	if(ioctl(fd, WDIOC_SETTIMEOUT, &timeout) != 0) {
		printf("Not Support WDIOC_SETTIMEOUT!!!\n");
		goto End;
	}

End:
	if(fd >= 0) {
		close(fd);
	}
	return;
}
