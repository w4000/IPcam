#include "common2.h"
#include "jQuery_common.h"
#include "ajax_common.h"

static void __write_actioninfo(int type)
{
	int fd;
	char str[4];

	unlink(ACTION_INFO);

	if((fd = open(ACTION_INFO, O_RDWR |O_CREAT, 0644)) == -1 )
	{
	}
	else
	{
		sprintf(str, "%d", type);
		write(fd, str, sizeof(str));

		close (fd);
		char cmd[128];
		sprintf (cmd, "echo \"file:%s, func:%s, line: %d > str? %d \" | cat >> /root/uplog.txt", __FILE__, __func__, __LINE__, atoi(str));
		system (cmd);
	}


}

static void notify_upgrade(int type)
{
#if 0
#else
	ipc_client_send("ipc_main", IPC_ID_UPDATE_SETUP, sizeof(int), (void *)&type);
#endif
	return;
}

static int write_file( void )
{
	FILE* fp;
	char buffer[2048],fname[0xff];
	int ret, length;

	mkdir ("/tmp/upgr/", 0755);
	sprintf(fname, "/tmp/upgr/%s.bin", "firmware");
	fp = fopen(fname, "wb");
	if(fp == NULL) return 0;

	ret = 0;
	while(1)
	{
		length = fread ( buffer, 1, 2048, stdin);
		if(length <= 0)
		{
			break;
		}
		fwrite ( buffer, 1, length, fp);
		ret += length;
	}

	fclose(fp);

	return ret;
}


/*=========================================================================================================
  int main(void)
==========================================================================================================*/
int main(void)
{
	cgi_init();
	cgi_session_start();

	cgi_init_headers();



	char *tmp_data;
	int file_length;
	tmp_data = getenv("CONTENT_LENGTH");
	if (tmp_data == NULL)
	{
		tmp_data = getenv("Content-Length");
		if (tmp_data == NULL)
		{
			printf("ERROR : 1");
			return 0;
		}
	}
	else {
		int content_len = atoi(tmp_data);
		if (0 >= content_len) {
			printf("ERROR : 1");
			return 0;
		}
	}

	__write_actioninfo(ACTIONINFO_UPGRADE_ONVIF);
	sleep(3);//w4000_sleep(3);

	notify_upgrade(UPDATE_SYSTEM_UPGRADE);
	sleep(3);

	file_length = write_file();
#if 0
#endif


	if (file_length <= 0) {
		printf("ERROR : 3(%s:%d)", tmp_data, atoi(tmp_data));
		cgi_end();
		return 0;
	}

	printf("#%d", file_length);
	fflush(stdout);
	__write_actioninfo(ACTIONINFO_UPGRADE_START);

	cgi_end();

	return 0;
}

