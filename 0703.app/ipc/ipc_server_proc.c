#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include "common.h"
#include "ipc_protocol.h"
#include "ipc_server.h"
#include "ipc_server_proc.h"



int server_proc(int fd, int id, void *buff, int size)
{
	switch(id) {
		case IPC_ID_UPDATE_SETUP:
			{
				int param;
				param = *((int *)buff);
				
				send_msg(MAIN, MAIN, _CAMMSG_CGI, param, 0, 0);
			}
			break;
		case IPC_ID_UPDATE_ISP:
			{
				int ret = 0;
				int param;
				param = *((int *)buff);
				
				send_msg(MAIN, ENCODER, _CAMMSG_CGI, param, 0, 0);
			}
			break;
	}

	return 0;
}
