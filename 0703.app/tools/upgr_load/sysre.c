#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <locale.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <dirent.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <time.h>
#include <sys/uio.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include "../system_reboot.c"
#include "watchdog.h"

#define KILLALL_IPC			"killall -9 " TARGET_BIN
#define KILLALL_IPC_SIGINT	"killall -SIGINT " TARGET_BIN
#define RUN_IPC				"/edvr/" TARGET_BIN " &"

#define	TRUE				1
#define	FALSE				0
#define ACTION_INFO 		"/mnt/ipm/actioninfo"


#define UPGRADE_TIMEOUT			200
#define	UPLOAD_TIMEOUT			600
#define	UPGRADE_TIMEOUT_ONVIF	15


enum { TIMEOUT_RESTART_APP_FROM = 15, TIMEOUT_RESTART_APP_TO = 20, TIMEOUT_RESTART_APP_MAX = 30 };

static int quit = 0;

enum
{
	ACTIONINFO_NONE,
	ACTIONINFO_REBOOT = 5,
	ACTIONINFO_RESTART = 7,
	ACTIONINFO_UPGRADE_DOWNLOAD_DONE = 9,
	ACTIONINFO_UPGRADE_START,
	ACTIONINFO_UPGRADE_SUCCESS,
	ACTIONINFO_UPGRADE_FAIL,
	ACTIONINFO_UPGRADE_ONVIF,



	ACTIONINFO_MAX
};


int reboot_system (void)
{
	hw_reset ();
	quit = 1;

	return 0;
}

inline int  file_exist(const char *filename)
{
	if (access(filename, F_OK) == 0)
		return TRUE;
	else
		return FALSE;
}

void remove_file (const char * path)
{
	if (! file_exist (path))
		return;

	if (unlink (path) < 0)
		perror ("unlink");
}

void signalHandler(int sig)
{
    signal(SIGINT, SIG_DFL);
    quit = 1;
}

static int __write_actioninfo(int type)
{
	int fd = -1;
	char str[4];

	unlink(ACTION_INFO);

	fd = open(ACTION_INFO, O_RDWR |O_CREAT, 0644);
	if(fd < 0)
	{
		return -1;
	}
	else
	{
		sprintf(str, "%03d", type);
		write(fd, str, sizeof(str));

		close (fd);

		char cmd[128];
		sprintf (cmd, "echo \"file:%s, func:%s, line: %d > str? %d \" | cat >> /root/uplog.txt", __FILE__, __func__, __LINE__, atoi(str));
		system (cmd);

	}

	return 0;
}

void LinkToShm(char* name, unsigned char** ptr, int size)
{
	// retry:
	{
		int shm_fd = shm_open(name, O_RDONLY, 0660);
		if (shm_fd > 0)
		{
			printf( "Shared memory linked..1\n");
		}
		else
		{
			printf( "Failed to connect to shared memory..\n");
			// goto retry;
		}

		*ptr = (unsigned char*)mmap(NULL, size, PROT_READ, MAP_SHARED, shm_fd, 0);
		if (*ptr == MAP_FAILED)
			exit(0);

		close(shm_fd);
	}
}

static int kill_main_app (void)
{
	system(KILLALL_IPC_SIGINT);
	sleep(3);//w4000_sleep
	system(KILLALL_IPC);

	return 0;
}

static int restart_main_app ()
{
	system(RUN_IPC);
	sleep(10);		// 10 Sec


	return 0;
}

static int read_acition_info (char* str)
{
	int ret = -1;
	if (file_exist(ACTION_INFO))
	{
		int fd = open(ACTION_INFO, O_RDWR);

		if (fd >= 0)
		{
			ret = read(fd, str, 4);
			close(fd);
			fd = -1;
		}

		remove_file(ACTION_INFO);
	}

	return ret;
}

int confirm_action (int timeout, int ck)
{
	if (ck >= timeout)
	{
		return -1;
	}
	return 0;
}

int wait_action (int timeout, int *ck)
{
	char str[4];
	int ret = 0;
	int param = -1;

	while (*ck < timeout)
	{
		ret = read_acition_info (str);
		if (ret > 0)
		{
			param = atoi(str);

			if (param == ACTIONINFO_REBOOT || param == ACTIONINFO_UPGRADE_SUCCESS)
			{
				break;
			}
			else if(param == ACTIONINFO_UPGRADE_FAIL)	// 11 = OK / 12 = FAIL
			{
				return -1;
			}
		}

		sleep(1);	// 1000ms
	}
	return 0;
}

int start_upgrade (int timeout)
{
	int ck = 0;
	int ret = -1;

	sleep(5);

	ret = wait_action (timeout, &ck);
	if (ret < 0)
	{
		return -1;
	}

	ret = confirm_action(timeout, ck);
	if (ret < 0)
	{
		return -1;
	}

	sleep(3);//w4000_sleep(3);		// 3 Sec

	remove_file(ACTION_INFO);
	return 0;
}

int ready_to_upgrade (int timeout)
{
	int ck = 0;
	char str[4];
	int ret = 0;
	int param = -1;


	kill_main_app ();

	while (ck < timeout)
	{
		ret = read_acition_info (str);
		if (ret > 0)
		{
			param = atoi(str);

			if (param == ACTIONINFO_UPGRADE_START)
				break;
		}

		// printf("Firmware upgrading... \n");
		sleep(1);	// 1000ms
		if ((ck % 150) == 0)
		{
			watchdog_resettimer ();
		}
	}

	ret = confirm_action(timeout, ck);
	if (ret < 0)
	{
		return -1;
	}


	return 0;
}

int main(int argc, char *argv[])
{
	int nCnt = 0;
	int ret = 0;


	/* Initialize signal handler for SIGINT */
	signal(SIGINT, signalHandler);

	remove_file(ACTION_INFO);

	system("free");

	for (nCnt = 0; nCnt < 10; nCnt++)
	{
		sleep(1);		// 1 Sec
	}

	nCnt = 0;
	struct timeval start;
	gettimeofday(&start, NULL);
	

	watchdog_init();

	// Wait 3 sec
	while (!quit)
	{
		nCnt ++;
		sleep(1);

		if (file_exist(ACTION_INFO))
		{
			sleep(3);		// 3 Sec

			int param = -1;
			char str[4];

			ret = read_acition_info (str);
			if (ret < 0)
			{
				continue;
			}

			param = atoi(str);
			
			if (param  == ACTIONINFO_REBOOT) {

				// kill previous ipm
				printf( "reboot_system 1..\n");
				system(KILLALL_IPC);
				sleep(3);		// 3 Sec

				// reboot force
				reboot_system ();
				break;
			}
			else if (param  == ACTIONINFO_RESTART) {

				// kill previous ipm
				kill_main_app ();
				restart_main_app ();

				nCnt = 0;
			}
			else if (param == ACTIONINFO_UPGRADE_DOWNLOAD_DONE) {
				ret = ready_to_upgrade (UPLOAD_TIMEOUT);
				if (ret < 0)
				{
					__write_actioninfo(ACTIONINFO_RESTART);
					continue;
				}

				system(RUN_IPC);
				ret = start_upgrade (UPGRADE_TIMEOUT);
				if (ret < 0)
				{
					kill_main_app ();
					__write_actioninfo(ACTIONINFO_RESTART);
					continue;
				}
				break;
			}
		}


	}

	// initialize variable
	quit = 0;
	nCnt = 0;

	exit(0);
}

