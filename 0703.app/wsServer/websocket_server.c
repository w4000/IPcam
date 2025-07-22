
#include "common.h"
#include "websocket_server.h"
#include "include/ws.h"



pthread_mutex_t	g_mutex_websocket_session[MAX_WEBSOCKET_SESSION];
static int g_websocket_session_id[MAX_WEBSOCKET_SESSION];

static websocket_session_t g_websocket_session[MAX_WEBSOCKET_SESSION];

#define WEBSOCKET_RECV_BUFFER_SZ (256 * K)

void websocket_init()
{
	int i;
	for (i = 0; i < MAX_WEBSOCKET_SESSION; i++)
	{
		pthread_mutex_init(&g_mutex_websocket_session[i], NULL);
		g_websocket_session_id[i] = -1;
		g_websocket_session[i].session_fd = -1;
	}
}

void websocket_terminate()
{
	int i;
	for (i = 0; i < MAX_WEBSOCKET_SESSION; i++)
	{
		pthread_mutex_destroy(&g_mutex_websocket_session[i]);
	}
}


websocket_session_t* websocket_get_session(int idx)
{
	return &g_websocket_session[idx];
}

static int get_websocket_session_id(int fd)
{
	int ret = -1;
	int i;

	for (i = 0; i < MAX_WEBSOCKET_SESSION; i++)
	{
		if (g_websocket_session_id[i] == fd)
		{
			ret = i;
			goto out;
		}
	}

	for (i = 0; i < MAX_WEBSOCKET_SESSION; i++)
	{
		WEBSOCKET_SESSION_LOCK(i);
		if (g_websocket_session_id[i] == -1)
		{
			websocket_session_t *s = &g_websocket_session[i];
			if (s->session_fd != -1) {
				// printf("%s:%d \n", __func__, __LINE__); printf("[WEBSOCKET] session_fd error!\n");
			}

			s->session_fd = fd;
			s->status = 1;
			CCircularBuffer_Create(&s->queue, WEBSOCKET_RECV_BUFFER_SZ);

			g_websocket_session_id[i] = fd;
			ret = i;

			WEBSOCKET_SESSION_UNLOCK(i);
			goto out;
		}
		WEBSOCKET_SESSION_UNLOCK(i);
	}

	out:

	return ret;
}

static void remove_session_id(int fd)
{
	int i;
	// WEBSOCKET_SESSION_LOCK(i);
	for (i = 0; i < MAX_WEBSOCKET_SESSION; i++)
	{
		if (g_websocket_session_id[i] == fd)
		{
			websocket_session_t *s = &g_websocket_session[i];
			if (s)
			{
				s->session_fd = -1;
				s->status = 0;
				CCircularBuffer_Delete(&s->queue);
			}

			g_websocket_session_id[i] = -1;
			break;
		}
	}
	// WEBSOCKET_SESSION_UNLOCK(i);
}

/**
 * @brief Called when a client connects to the server.
 *
 * @param fd File Descriptor belonging to the client. The @p fd parameter
 * is used in order to send messages and retrieve informations
 * about the client.
 */
static void onopen(const char * forwarded_ip, int fd)
{
	char *cli;
	cli = ws_getaddress(fd);
	// printf("! %s>>> forwarded_ip? %s, cli? %s, fd? %d \n", __func__, forwarded_ip, cli, fd);
	if (cli)
	{
		// printf("%s:%d \n", __func__, __LINE__); printf("Connection opened, client: %d | addr: %s\n", fd, cli);

		if (IPCAM_THREAD (NETSVR)->state == STAT_RUNNING)
		{
			if (g_main.th.state == STAT_RUNNING)
			{
				int wsid = get_websocket_session_id(fd);

				if (wsid != -1) {
					// printf("%s:%d \n", __func__, __LINE__); printf("! [WEBSOCKET] WSID [%d]\n", wsid);

					if (strlen(forwarded_ip) > 0) {
						websocket_session_t *s = websocket_get_session(wsid);
						if (s) {
							// printf("%s:%d \n", __func__, __LINE__); printf("! [WEBSOCKET] forwarded_ip %s\n", forwarded_ip);
							snprintf(s->forwarded_ip, sizeof(s->forwarded_ip), "%s", forwarded_ip);
						}
					}

					if (__creat_streamsvr2 (0, SOCKET_TYPE_WEBSOCKET, wsid) == 0) {
						// printf("[WEBSOCKET] CREATE STREAM SVR ERROR!\n");
					}
				}
			}
		}

		free(cli);
	}
}

/**
 * @brief Called when a client disconnects to the server.
 *
 * @param fd File Descriptor belonging to the client. The @p fd parameter
 * is used in order to send messages and retrieve informations
 * about the client.
 */
static void onclose(int fd)
{
	char *cli;
	cli = ws_getaddress(fd);
	if (cli)
	{
		int wsid = get_websocket_session_id(fd);

		// printf("%s:%d \n", __func__, __LINE__); printf("! [WEBSOCKET] On closed, wsid = %d\n", wsid);

		// Terminate stream server
		{
			struct st_stream * svr = NULL;
			int slot;

			foreach_occupied_svr_slot(svr, slot)
			{
				if (svr->is_websocket && svr->websocket_session_id == wsid)
				{
					debug("-------------------------------------- terminate svr (wsid = %d) (svr=%p)\n", wsid, svr);

					svr->terminate = 1;
					break;
				}
			}
			end_foreach

			if (svr)
			{
				__u64 stm = get_time_msec();
				while (svr->inuse && get_time_msec() - stm < 5000)
				{
					debug("-------------------------------------- wait for svr close..(wsid = %d) (svr=%p)\n", wsid, svr);

					sleep_ex(1);
				}
			}
		}

		remove_session_id(fd);

		// printf("%s:%d \n", __func__, __LINE__); printf("! [WEBSOCKET] Connection closed, client fd: %d | addr: %s\n", fd, cli);
		free(cli);
	}
}

/**
 * @brief Called when a client connects to the server.
 *
 * @param fd File Descriptor belonging to the client. The
 * @p fd parameter is used in order to send messages and
 * retrieve informations about the client.
 *
 * @param msg Received message, this message can be a text
 * or binary message.
 *
 * @param size Message size (in bytes).
 *
 * @param type Message type.
 */
static void onmessage(int fd, const unsigned char *msg, uint64_t size, int type)
{
	#if 1 //debug
	char *cli;
	cli = ws_getaddress(fd);
	// printf("%s:%d \n", __func__, __LINE__); 
	// printf("! I receive a message! (size: %" PRId64 ", type: %d), from: %s/%d\n",
		// size, type, cli, fd);
	free(cli);
	#endif


#if 0
	{
		int i;
		printf("[web socket]\n");
		printf("\nrecv msg ======================================================\n");
		printf("text:\n");
		for (i=0; i<size; i++)
		{
			printf("%c", msg[i]);
		}


		printf("\n\nhex:\n");
		for (i=0; i<size; i++)
		{
			printf("%x", msg[i]);
		}

		printf("\n=========================================================== end\n\n");
	}
#endif


	int wsid = get_websocket_session_id(fd);


	if (wsid != -1)
	{
		websocket_session_t *s = &g_websocket_session[wsid];
		CCircularBuffer_Push(&s->queue, (unsigned char* const )msg, (int)size);
	}
	else
	{
		// printf("%s:%d \n", __func__, __LINE__); printf("[WEBSOCKET] onmessage sessid error!\n");
	}

}

static void test(void)
{
}

static void *websocket_server_thread (void * arg)
{

	// printf("! websocket_server_thread >>> sleep 2 \n");
	// sleep(2);
    // THREAD_OUT(0, __FUNCTION__, pthread_self());

	struct ws_events evs;
	evs.onopen    = &onopen;
	evs.onclose   = &onclose;
	evs.onmessage = &onmessage;
#if 1 // evt test	
	evs.test	  = &test;
	evs.val	      = 90;
#endif
	ws_socket(&evs, DEFAULT_PORT_WEBSOCKET, FALSE); /* Never returns. */

    // __close_thread("websocket_server");
    pthread_exit(NULL);
}

void websocket_start()
{
    static pthread_t		websocket_server_thread_tid;

    // if (__create_thread (&websocket_server_thread_tid, NULL, websocket_server_thread, NULL, "websocket_server") == 0) {





	if (pthread_create (&websocket_server_thread_tid, (void*)NULL, websocket_server_thread, NULL) == 0) {
	// if (pthread_create (&websocket_server_thread_tid, NULL, websocket_server_thread, NULL) == 0) {
		pthread_detach (websocket_server_thread_tid);
	}
}

void websocket_stop()
{
	ws_socket_stop();
}

