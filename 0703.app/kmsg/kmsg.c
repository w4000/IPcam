
// Purpose
//	Kerenl message parsing & handling
#include "common.h"
#include "kmsg/kmsg.h"

pthread_t	kmsg_tid;
static FILE * fp = NULL;

static void notify_upgrade(int type)
{
	int fd;
	char str[4];
	if((fd = open(CGI_UPGRADE, O_RDWR |O_CREAT ,0644)) == -1 ){
		perror ("open");
		printf("CGI_UPGRADE error<br>");
	}

	sprintf(str, "%d", type);
	write(fd, str, sizeof(str));

	close (fd);
	return;
}

static void clean_up (void *arg)
{
	if (fp) {
		fclose (fp);
		fp = NULL;
	}
}

#define ETH0_SYSFS_PATH                 "/sys/class/net/eth0/carrier"

int is_net_link()
{
	int link = -1;
	FILE *fp;

	fp = fopen(ETH0_SYSFS_PATH, "r");
	if(fp) {
		fscanf(fp, "%d", &link);
		fclose(fp);
		return link;
	}

	return 0;
}

void *kmsgproc (void *arg) {
	int nowlink = 0;
	int islink = -1;
	int64_t link_change_msec = 0;
	islink = is_net_link();
	nowlink = islink;

	pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	pthread_cleanup_push (clean_up, (void *) arg);		

	fp = fopen(KMSG_FILE, "r");
	if (fp == NULL)
		goto out;


	while (1) {
		usleep (100*1000);
		nowlink = is_net_link();

		if((islink != nowlink) && (get_tick_msec() - link_change_msec > 3000)) {
			if(nowlink) {
				notify_upgrade(UPDATE_NETWORK_LINK_STATUS);
				send_msg (MAIN, NETSVR, _CAMMSG_ETHERNET_LINK, 1, 0, 0);
			}
			else {
				send_msg (MAIN, NETSVR, _CAMMSG_ETHERNET_LINK, 0, 0, 0);
			}
			link_change_msec = get_tick_msec();
			islink = nowlink;
		}
	}

out:
	pthread_cleanup_pop (1);
	pthread_exit (NULL);
}
