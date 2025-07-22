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
#include "ipc_protocol.h"
#include "ipc_client.h"


int ipc_client_send(const char *id, unsigned int msg_id, int data_size, void *data)
{
	int sfd;
	struct sockaddr_un clientaddr;
	int client_len;

	char path[128];
	sprintf(path, "/tmp/%s", id);

	sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(sfd < 0) {
		perror("socket error");
		return -1;
	}
	bzero(&clientaddr, sizeof(clientaddr));
	clientaddr.sun_family = AF_UNIX;
	strcpy(clientaddr.sun_path, path);
	client_len = sizeof(clientaddr);


	/* connect */
	if(connect(sfd, (struct sockaddr *)&clientaddr, client_len) < 0) {

		perror("connect error");
		return -1;
	}

	{
		IPC_PROTOCOL_HEADER_T header;
		header.magic = IPC_PROTOCOL_MAGIC;
		header.id = msg_id;
		header.data_size = data_size;
		write(sfd, &header, sizeof(IPC_PROTOCOL_HEADER_T));

		if(data_size > 0) {
			write(sfd, data, data_size);
		}

		header.id = IPC_ID_FIN;
		header.data_size = 0;
		write(sfd, &header, sizeof(IPC_PROTOCOL_HEADER_T));
	}
	close(sfd);

	return 0;

}

int ipc_client_connect(const char *id)
{
	int sfd;
	struct sockaddr_un clientaddr;
	int client_len;

	char path[128];
	sprintf(path, "/tmp/%s", id);

	sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(sfd < 0) {
		perror("socket error");
		return -1;
	}
	bzero(&clientaddr, sizeof(clientaddr));
	clientaddr.sun_family = AF_UNIX;
	strcpy(clientaddr.sun_path, path);
	client_len = sizeof(clientaddr);

	/* connect */
	if(connect(sfd, (struct sockaddr *)&clientaddr, client_len) < 0) {
		perror("connect error");
		return -1;
	}

	return sfd;
}

void ipc_client_disconnect(int fd)
{
	IPC_PROTOCOL_HEADER_T header;
	header.id = IPC_ID_FIN;
	header.data_size = 0;
	write(fd, &header, sizeof(IPC_PROTOCOL_HEADER_T));

	close(fd);
}

int ipc_client_request(int fd, unsigned int msg_id, int size, void *data)
{
	int ret = 0;

	IPC_PROTOCOL_HEADER_T header;
	header.magic = IPC_PROTOCOL_MAGIC;
	header.id = msg_id;
	header.data_size = size;
	ret = write(fd, &header, sizeof(IPC_PROTOCOL_HEADER_T));

	if(size > 0 && data != NULL) {
		ret = write(fd, data, size);
	}

	return ret;
}

int ipc_client_response(int fd, void *data, int size)
{
	int ret = 0;
	int rd_cnt = 0;
	char *p = data;

	do {
		ret = read(fd, p, size-rd_cnt);
		if(ret < 0) {
			if(errno == EWOULDBLOCK || errno == EAGAIN) {
				usleep(1000);
				continue;
			}
			rd_cnt = ret;
			perror("read:");
			break;
		}
		rd_cnt += ret;
		p += ret;
	} while(rd_cnt < size);

	return rd_cnt;
}
