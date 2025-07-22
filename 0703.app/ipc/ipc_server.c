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
#include "list.h"


typedef struct {
	int fd;
	struct list_head entry;
} IPC_CLIENT_S;

static struct list_head g_clients;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int recv_data(int fd, void *data, int size)
{
	int ret = 0;
	int rd_cnt = 0;

	do {
		ret = read(fd, data, size-rd_cnt);
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
	} while(rd_cnt < size);

	return rd_cnt;
}

int g_cfd;
static void *server_thread(void *arg)
{
	int fd;
	int rd_cnt;
	IPC_PROTOCOL_HEADER_T header;
	char buff[64];
	char *p_buf;
	char *p_large_buf;

	fd = *((int*)arg);

	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(IPC_CLIENT_S));
	IPC_CLIENT_S *client = (IPC_CLIENT_S *)malloc(sizeof(IPC_CLIENT_S));
	client->fd = fd;

	pthread_mutex_lock(&mutex);
	INIT_LIST_HEAD(&client->entry);
	list_add_tail(&client->entry, &g_clients);
	pthread_mutex_unlock(&mutex);

	while(1) {
		rd_cnt = recv_data(fd, &header, sizeof(IPC_PROTOCOL_HEADER_T));
		if(rd_cnt < 0) {
			perror("recv error: ");
			break;
		}


		if(header.magic != IPC_PROTOCOL_MAGIC) {
			continue;
		}

		if(header.id == IPC_ID_FIN) {
			break;
		}

		if(header.data_size > 0) {
			if(header.data_size > 64) {
				// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, header.data_size);
				p_buf = (char *)malloc(header.data_size);
				p_large_buf = p_buf;
			}
			else {
				p_buf = buff;
				p_large_buf = NULL;
			}

			rd_cnt = recv_data(fd, p_buf, header.data_size);
			if(rd_cnt < 0) {
				if(p_large_buf != NULL) {
					free(p_large_buf);
					p_buf = NULL;
				}
				perror("recv error: ");
				break;
			}
		}
		else {
			p_buf = NULL;
		}

		pthread_mutex_lock(&mutex);
		if(server_proc(fd, header.id, p_buf, header.data_size) < 0) {
			if(p_large_buf != NULL) {
				free(p_large_buf);
				p_buf = NULL;
			}
			break;
		}
		pthread_mutex_unlock(&mutex);

		if(p_large_buf != NULL) {
			free(p_large_buf);
			p_buf = NULL;
		}
	}
	// fprintf(stderr, "Disconnected client(%04X)\n", fd);

	struct list_head *pos;
	IPC_CLIENT_S *item;

	pthread_mutex_lock(&mutex);
	list_for_each(pos, &g_clients) {
		item = list_entry(pos, IPC_CLIENT_S, entry);
		if(item->fd == fd) {
			list_del(&item->entry);
			free(item);
			break;
		}
	}
	pthread_mutex_unlock(&mutex);

	close(fd);
	return NULL;
}

static int g_run_thread = 0;
static int g_thread_status = 0;
static int g_fd;
static void *ipc_server_thread(void *arg)
{
	int fd, cfd;
	int th_id;
	pthread_t thread_t;
	struct sockaddr_un clientaddr;
	socklen_t client_len;

	fd = *((int*)arg);

	g_run_thread = 1;
	g_thread_status = 1;
	while(g_run_thread) {
		client_len = sizeof(clientaddr);
		memset(&clientaddr, 0, sizeof(clientaddr));
		cfd = accept(fd, (struct sockaddr *)&clientaddr, &client_len);
		if(cfd < 0) {
			if(errno == EWOULDBLOCK || errno == EAGAIN) {
				usleep(1000);
				continue;
			}
			printf("accept error");
			continue;
		}

		g_cfd = cfd;
		th_id = pthread_create(&thread_t, NULL, server_thread,
				(void *)&g_cfd);
		if(th_id != 0) {
			printf("thread create error");
			continue;
		}
		pthread_detach(thread_t);
	}
	g_thread_status = 0;


	close(fd);

	return NULL;
}

int ipc_server_init(const char *id)
{
	int fd;
	int sockstat;
	struct sockaddr_un serveraddr;
	int stat;
	char path[128];

	INIT_LIST_HEAD(&g_clients);

	int th_id;
	pthread_t thread_t;

	sprintf(path, "/tmp/%s", id);

	if(access(path, F_OK) == 0) {
		unlink(path);
	}

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(fd < 0) {
		perror("socket error");
		return -1;
	}
	sockstat = fcntl(fd, F_GETFL, NULL);
    fcntl(fd, F_SETFL, sockstat | O_NONBLOCK);

	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sun_family = AF_UNIX;
	strcpy(serveraddr.sun_path, path);

	stat = bind(fd, (struct sockaddr *)&serveraddr,
			sizeof(serveraddr));
	if(stat < 0) {
		perror("bind error");
		return -1;
	}

	stat = listen(fd, 5);
	if(stat < 0) {
		perror("listen error");
		return -1;
	}


	g_fd = fd;
	th_id = pthread_create(&thread_t, NULL, ipc_server_thread,
			(void *)&g_fd);
	if(th_id != 0) {
		perror("thread create error");
		return -1;
	}
	pthread_detach(thread_t);

	return 0;
}

void ipc_server_fini()
{
	g_run_thread = 0;
	while(g_thread_status) {
		usleep(100000);
	}

	IPC_CLIENT_S *item;
	struct list_head *pos;

	pthread_mutex_lock(&mutex);
	list_for_each(pos, &g_clients) {
		item = list_entry(pos, IPC_CLIENT_S, entry);
		list_del(&item->entry);
		free(item);
	}
	pthread_mutex_unlock(&mutex);
}

int ipc_server_broadcast(int msg_id)
{
	struct list_head *pos;
	IPC_CLIENT_S *item;

	pthread_mutex_lock(&mutex);
	list_for_each(pos, &g_clients) {
		item = list_entry(pos, IPC_CLIENT_S, entry);
		server_proc(item->fd, msg_id, NULL, 0);
	}
	pthread_mutex_unlock(&mutex);

	return 0;
}
