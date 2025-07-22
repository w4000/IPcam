#include "common.h"
#include "lib/net.h"
#include "lib/threadsafe.h"
#include "netsvr/auth.h"

#define USE_STREAM_THREAD

static struct st_cliinfo 	g_cliinfo_pool [52];
static char					g_cliinfo_inuse[52] = { 0, };


static int _check_http_protocol(struct st_stream * strsvr, ST_PHDR* phdr);
#if 0
#else
int get_param_val_with_end(const char* outbuf, const char* param, const char* endstr)
{
	char* p;
	if ((p = strcasestr(outbuf, param)) != NULL)
	{
		char* start = p + strlen(param);
		char* end = strcasestr(start, endstr);
		if (end)
		{
			char data[16];
			memset(data, 0, sizeof(data));
			if (end - start < 16) {
				strncpy(data, start, end - start);
				return atoi(data);
			}
		}
		else
			return -1;
	}
	return -1;
}
#endif




struct st_cliinfo* get_cliinfo_pool()
{
	int i;
	for (i = 0; i < 52; i++)
	{
		if (!g_cliinfo_inuse[i])
		{
			g_cliinfo_inuse[i] = 1;
			return &(g_cliinfo_pool[i]);
		}
	}

	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof (struct st_cliinfo));
	return (struct st_cliinfo *)_mem_malloc2 (sizeof (struct st_cliinfo));
}

void free_cliinfo_pool(struct st_cliinfo* msg)
{
	int i;
	for (i = 0; i < 52; i++)
	{
		if (&(g_cliinfo_pool[i]) == msg)
		{
			g_cliinfo_inuse[i] = 0;
			return;
		}
	}
	_mem_free(msg);
}



#ifdef _USE_WEBSOCKET_SERVER_
static struct st_cliinfo * get_cliinfo_websocket(struct st_stream* svr) {
	struct st_cliinfo * cli = NULL;

	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof (struct st_cliinfo));
	if ((cli = (struct st_cliinfo *) malloc (sizeof (struct st_cliinfo))) == NULL)
	{
		perror("malloc");
		return NULL;
	}

	pthread_mutex_lock (& g_netsvr.cliinfo_mutex);
	init_clist (& cli->list);

	if (svr->is_websocket)
	{
		char cliip[32] = "";
		if (strlen(svr->websocket_session->forwarded_ip) > 0)
		{
			snprintf(cliip, sizeof(cliip), "%s", svr->websocket_session->forwarded_ip);
		}
		else
		{
			char * tmpip = ws_getaddress(svr->websocket_session->session_fd);
			if (tmpip) {
				snprintf(cliip, sizeof(cliip), "%s", tmpip);
				free(tmpip);
			}
		}
		if (strlen(cliip) > 0) {

	#if __USE_WEBSOCKET_BY_TCP__
			if (strcmp(cliip, "0.0.0.0") == 0 || strcmp(cliip, "127.0.0.1") == 0)
				strncpy(cli->ip, get_last_websocket_tcp_ip(), sizeof(cli->ip));
			else
	#endif
				strncpy(cli->ip, cliip, sizeof(cli->ip));

		}
	}

	memset(cli->uid, 0, sizeof(cli->uid));
	cli->refcnt = 1;

	addto_clist_tail (& g_netsvr.cliinfo_head, & cli->list);
	pthread_mutex_unlock (& g_netsvr.cliinfo_mutex);

	return cli;
}
#endif




static struct st_cliinfo * get_cliinfo(struct sockaddr cliaddr) {
	struct clist volatile *tmp;
 	struct st_cliinfo *cli = NULL;

	pthread_mutex_lock (& g_netsvr.cliinfo_mutex);
	foreach_clist_entry (& g_netsvr.cliinfo_head, tmp)
	{
		cli = clist_entry(tmp, struct st_cliinfo, list);

		if(!memcmp(
				&((struct sockaddr_in *)&cliaddr)->sin_addr,
				&((struct sockaddr_in *)&cli->addr)->sin_addr,
			 	sizeof(struct in_addr))
		) {
			/* fprintf(stderr, "Found exitsing cliaddr\n"); */
			cli->refcnt++;
			pthread_mutex_unlock (& g_netsvr.cliinfo_mutex);
			return cli;
		}
	}
	pthread_mutex_unlock (& g_netsvr.cliinfo_mutex);

	if ((cli = get_cliinfo_pool()) == NULL)
		return NULL;

	pthread_mutex_lock (& g_netsvr.cliinfo_mutex);
	init_clist (& cli->list);
	memcpy (& cli->addr, & cliaddr, sizeof (cliaddr));
	cli->refcnt = 1;
	addto_clist_tail (& g_netsvr.cliinfo_head, & cli->list);
	pthread_mutex_unlock (& g_netsvr.cliinfo_mutex);

	return cli;
}

static void release_cliinfo(struct st_cliinfo *cliinfo) {
	assert (cliinfo);

	cliinfo->refcnt--;

	if (cliinfo->refcnt <= 0) {
		pthread_mutex_lock (& g_netsvr.cliinfo_mutex);
		del_clist(&cliinfo->list);
		pthread_mutex_unlock (& g_netsvr.cliinfo_mutex);
		free_cliinfo_pool(cliinfo);
	}
}

void flush_cliinfo(ST_NETSVR *netsvr) {
	assert (netsvr);

	while(!clist_empty(&netsvr->cliinfo_head)) {
		struct st_cliinfo *cliinfo = clist_entry(netsvr->cliinfo_head.next, struct st_cliinfo, list);
		pthread_mutex_lock (& g_netsvr.cliinfo_mutex);
		del_clist(&cliinfo->list);
		pthread_mutex_unlock (& g_netsvr.cliinfo_mutex);
		free_cliinfo_pool(cliinfo);
	}
}

#ifdef _USE_WEBSOCKET_SERVER_	//0717

static struct st_stream *__creat_streamsvr_core (int sd, SOCKET_TYPE type, int idx) {

	struct st_stream strsvr, * svr = NULL;
	int slot;
	int cam;


	assert (sd >= 0);
	memset(& strsvr, 0, sizeof(struct st_stream));
	strsvr.sock = -1;
	SS_CLRFLAG1 (& strsvr);
	strsvr.keepalive_chk_stamp = 0;



#ifdef _USE_WEBSOCKET_SERVER_
	strsvr.is_websocket = (type == SOCKET_TYPE_WEBSOCKET) ? 1 : 0;
	strsvr.websocket_session_id = idx;

	if (strsvr.is_websocket)
	{
		
		strsvr.websocket_session = websocket_get_session(strsvr.websocket_session_id);
	}
#endif


	strsvr.vod_sync = 0;
	strsvr.session_code = 0;
	strsvr.terminate = 0;

	

	struct sockaddr_in	client;	// Connected client address.
#ifdef _USE_WEBSOCKET_SERVER_
	if (strsvr.is_websocket)
	{
		strsvr.sock = -1;
	}
	else
#endif
	{
		// web socket not in 
		if ((strsvr.sock = accept_svrsock (sd, & client)) < 0) {
			printf("Failed to accept socket!\n");
			goto fail;
		}


#if 1
		{
			const int opt = 1;
			int ret = setsockopt(strsvr.sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
			if (ret == 0)
				printf("SET TCP NO_DELAY (ret = %d)\n", ret);
			else
				printf("Failed to SET TCP NO_DELAY (ret = %d)\n", ret);
		}
#endif
	}

	if (g_netsvr.numclients >= 32)
	{
		printf("g_netsvr.numclients = %d\n", g_netsvr.numclients);
		goto fail;
	}


	svr = NULL;
	foreach_unoccupied_svr_slot(svr, slot)
		break;
	end_foreach

	if (! svr) {
		printf("NO UNOCCUPIED SLOT FOR SOCKET!\n");
		goto fail;
	}

	* svr = strsvr;

	

	if (pthread_create (& svr->id, NULL, svr_thread, (void *) svr))  {
		perror ("pthread_creat");
		goto fail;
	}
	pthread_detach(svr->id);




#ifdef USE_STREAM_THREAD
	// Create streaming thread.
	if (pthread_create (& svr->stream_id, NULL, stream_thread, (void *) svr))  {
		perror ("pthread_creat");
		goto fail;
	}
	pthread_detach(svr->stream_id);
#endif


	svr->inuse = TRUE;
	g_netsvr.numclients++;

	return svr;



fail:
	printf ("Failed creating stream server.\n");

	if (svr) {
		svr->inuse = FALSE;
	}
	else if (g_netsvr.numclients >= 32) {
		char buf [256];
		printf("Reached Maximum connection\n");
		reply2client (&strsvr, strsvr.sock, buf, 0x00000000, RETMAXCONN, 0);
		sleep(1);//w4000_sleep(1);	
	}


	__release_stream (& strsvr);

	return NULL;
}



static pthread_mutex_t	g_create_streamsvr_mutex = PTHREAD_MUTEX_INITIALIZER;
struct st_stream *__creat_streamsvr2 (int sd, SOCKET_TYPE type, int idx)
{

	
	struct st_stream* svr;

	pthread_mutex_lock(&g_create_streamsvr_mutex);
	svr = __creat_streamsvr_core(sd, type, idx);
	pthread_mutex_unlock(&g_create_streamsvr_mutex);

	return svr;
}

#endif


struct st_stream *__creat_streamsvr (int sd, int type) {
	
	struct st_stream strsvr, * svr = NULL;
	struct sockaddr_in	client;	// Connected client address.
	int slot;
	int cam;

	assert (sd >= 0);


	memset(& strsvr, 0, sizeof(struct st_stream));

	strsvr.sock = -1;

	foreach_cam_ch (cam)
		SS_CLRFLAG (& strsvr, cam);
		init_live (& strsvr.live [cam]);
		init_vod (& strsvr.vod [cam]);

		strsvr.netstamp [cam] = 0;
	end_foreach


	strsvr.transbypass = 1;
	strsvr.terminate = 0;

	{
		if ((strsvr.sock = accept_svrsock (sd, & client)) < 0)
			goto fail;
	}

	svr = NULL;
	foreach_unoccupied_svr_slot(svr, slot)
		break;
	end_foreach

	if (! svr)
		goto fail;

	* svr = strsvr;

	if(g_main.isNetUpgrade == 1)
		goto fail;


	// Create server thread.
	if (pthread_create (& svr->id, NULL, svr_thread, (void *) svr))  {
		perror ("pthread_creat");
		goto fail;
	}
	pthread_detach(svr->id);

#ifdef USE_STREAM_THREAD
	// Create streaming thread.
	if (pthread_create (& svr->stream_id, NULL, stream_thread, (void *) svr))  {
		perror ("pthread_creat");
		goto fail;
	}
	pthread_detach(svr->stream_id);
#endif

	svr->inuse = TRUE;

	return svr;

fail:
	printf("Failed creating stream server.\n");

	if (svr) {
		svr->inuse = FALSE;
	}
	else if (g_netsvr.numclients >= 32) {
		char buf [256];
		printf("Reached Maximum connection : numclients = %d\n", g_netsvr.numclients);
		reply2client (NULL, strsvr.sock, buf, 0x00000000, RETMAXCONN, 0);
	}

	__release_stream (& strsvr);

	return NULL;
}

#ifdef USE_STREAM_CONSUMER
static int consumer_id[8] = {-1,-1,-1,-1,-1,-1,-1,-1};
int __creat_streamsvr_for_consumer(int ch_flag, consumer_cb cb)
{
	// not use .. 
	return -1;
}


void __release_streamsvr_for_consumer(int id)
{
}

#endif


void __release_stream(struct st_stream *svr) {
	int cam;

	assert (svr);

	// Release sockets.
	if (svr->sock >= 0) {
		printf("Socket close\n");
#ifdef USE_STREAM_CONSUMER
		if(svr->is_consumer == 0) {
			close(svr->sock);
		}
#else
		close(svr->sock);
#endif
		svr->sock = -1;
	}
#ifdef USE_STREAM_CONSUMER
	else {
		svr->sock = -1;
	}
#endif

	if (svr->cliinfo)
		release_cliinfo(svr->cliinfo);

	foreach_cam_ch (cam)
		release_live (& svr->live [cam]);
		release_vod (& svr->vod [cam]);
	end_foreach

}

void release_stream (ST_NETSVR *netsvr, struct st_stream *svr) {
	assert (netsvr);
	assert (svr);
	__release_stream(svr);
	svr->inuse = FALSE;


}


static void clean_up (void *arg)
{
	struct st_stream * svr  = (struct st_stream *) arg;

	send_msg (NETSVR, NETSVR, _CAMMSG_STRSVR_EXIT, (int)svr, 0 ,0);

}

void * svr_thread (void * arg)
{
	struct st_stream * strsvr = (struct st_stream *) arg;
	int i, ret = 0,  retsrv = 0;
	int killid = 0;

#define GET_SOCK_REQUESTED(strsvr) \
		({\
			(FD_ISSET ((strsvr)->sock, & rdset)) ? (strsvr)->sock :\
			({debug ("Should not be here.\n"); -1;});\
		})

	assert (strsvr);

	strsvr->svr_log_out_flag =1;

	while (! strsvr->inuse)
		sleep_ex (100);

	pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	pthread_cleanup_push (clean_up, (void *) arg);		


	do {
		sleep_ex(1);

		struct timeval timeout;
		fd_set rdset;

		FD_ZERO (& rdset);

		if (strsvr->sock >= 0)
			FD_SET (strsvr->sock, & rdset);

		timeout.tv_sec = 0;
		timeout.tv_usec = 100;

		int do_read = 0;

#if 0
#else

#ifdef _USE_WEBSOCKET_SERVER_
		if (strsvr->is_websocket)
		{
			if (CCircularBuffer_GetCount(&strsvr->websocket_session->queue) > 0)
			{
				// printf("! %s:%d >>> netsvr read \n", __func__, __LINE__);
				do_read = 1;
			}
		}
		else
#endif
		{
			if ((ret = select (strsvr->sock + 1, & rdset, NULL, NULL, & timeout)) == -1) {
				continue;
			}
		}

#endif
		if (ret == 0) {
			// Timeout occurred while waiting. Do nothing.
		}
		else {
			do_read = 1;
		}

		// printf("! do_read? %d \n", do_read);
		if(do_read) {// We got request from peer.
			ST_PHDR	phdr;
			phdr.id = 0;

			{
				if(phdr.id == 0) {
					killid = 0;
					if (readphdr (strsvr, strsvr->sock, & phdr) < 0) {
						printf ("timeout or disconnect by remote side\n");
						break; // Timeout or disconnect.
					}
				}


				int is_auth = 1;
#if 1  
				if (ISFLAG(strsvr, 0, __SS_AUTHED) != 1 && !strsvr->is_websocket)
#else 
				if (ISFLAG(strsvr, 0, __SS_AUTHED) != 1)
#endif 
				{
					switch (phdr.id)
					{
						case PUSRAUTH:
						case PUSRENCAUTH:
						case PUSRENCAUTH_REINF:
						case PUSRENCAUTH_UNI:

						case PREQDEVINFO:
						case PREQFIXINFO:
						case PREQDYNSTORAGE:
						case PREQTERMINATE:
						case PREQHEALTHINFO:
						case PREQBYPASSINFO:
						case PHEARTBEAT:
						case PREQPUBLICINFO:
						case PREQPUBLICINFO_UNI:
							break;

						default:
							{
								char sndbuf [1*K];
								reply2client(strsvr, strsvr->sock, sndbuf, REPLYID(phdr.id), RETNOPERM, 0);
								is_auth = 0;
							}
							break;
					}
				}

				if (!is_auth) {
					printf("auth failed: 0x%x\n", phdr.id);
					break;
				}


				killid = phdr.id;
				retsrv = handle_sysinfo_request (strsvr, phdr);
				if (retsrv < 0) {
					break;
				}


				if (retsrv == 1) {
					retsrv = handle_live_request (strsvr, phdr);
				}

				if (retsrv < 0) {
					break;
				}

				if (retsrv == 1)
					retsrv = handle_vod_request (strsvr, phdr);

				if (retsrv < 0) {
					break;
				}

				if (retsrv == 1) {
					retsrv = handle_auth_request (strsvr, phdr);
				}

				if (retsrv < 0) {
					break;
				}


				// if (retsrv == 1)
				if (retsrv == 1 || phdr.id == PLOGLAST)
				{
				}
			}
		}
#ifndef USE_STREAM_THREAD
		int i, ret;
		for (i = 0; i < 1; i++)	{
			foreach_cam_ch (cam)
				if (ISLIVE (strsvr, cam) && ISFLAG (strsvr, cam, __SS_SENDIMG)) {

					if (ret < 0) break;
				}
			end_foreach

			g_main.thread_alive_check[THREAD_ALIVE_CHECK_NETSVR] = 0;

			if (ret < 0) break;
		}
#endif

	} while (! strsvr->terminate);



	pthread_cleanup_pop (1);
	pthread_exit (NULL);

	return NULL;
}

void * stream_thread (void * arg)
{
	struct st_stream * strsvr = (struct st_stream *) arg;
	int ret = 0;
	int cam;

	assert (strsvr);
	strsvr->stream_log_out_flag =1;

	while (! strsvr->inuse)
		sleep_ex (100);

	pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	pthread_cleanup_push (clean_up, (void *) arg);		// Register clean-up routine when exit this thread.



	// static int _live[2] = {-1, -1};
	do {
		sleep_ex(1);

		foreach_cam_ch (cam)
			if (ISLIVE (strsvr, cam) && ISFLAG (strsvr, cam, __SS_SENDIMG)) {
				do_live (strsvr, cam, ret);
				if (ret < 0) break;
			}
		end_foreach

		g_main.thread_alive_check[THREAD_ALIVE_CHECK_NETSVR] = 0;

		if (ret < 0) break;
	} while (! strsvr->terminate);

	pthread_cleanup_pop (0);
	pthread_exit (NULL);

	return NULL;
}


static int net_sock_is_readable_data(struct st_stream * strsvr, int sock, int timeout_sec)
{
	int do_read = 0;

#if 0
#endif
	{
		if (sock >= 0)
		{
			int ret;

		#if _USE_SOCK_POLL_
		#else
			struct timeval timeout;
			fd_set rdset;
			fd_set errset;

			// Prepare select ().
			FD_ZERO (& rdset);
			FD_ZERO (& errset);

			FD_SET (sock, & rdset);
			FD_SET (sock, & errset);

			timeout.tv_sec  = timeout_sec;
			timeout.tv_usec = 0;	// 시간을 1만 줘도 느려짐..

			ret = select (sock + 1, & rdset, NULL, & errset, & timeout);
		#endif

			if (ret != -1) {
				if (ret == 0) {
					// Timeout occurred while waiting. Do nothing.
					printf("[HTTP] TIMEOUT\n");
				}
				else {
		#if _USE_SOCK_POLL_
		#else
					if (FD_ISSET(sock, & errset))
		#endif
					{
						printf("[HTTP] socket error!\n");

						//web->s_close = 1;
						do_read = -1; // Timeout or disconnect.
					}
					else
					{
						do_read = 1;
					}
				}
			}
		}
	}

	return do_read;
}





static int _safe_write_socket(int sock, char* buf, int r)
{
	int s = 0, w;
	while (r > 0)
	{
		w = write(sock, buf + s, r);
		if (w < 0)
		{
			if (errno != EAGAIN)
			{
				return -1;
			}
		}
		else if (w == 0)
		{
			return -1;
		}
		else
		{
			s += w;
			r -= w;
		}
	}

	return 0;
}

static int _ws_safe_write_socket(struct st_stream * strsvr, char* buf, int r)
{
	int connection_close = 0;

	if (strsvr)
	{
#if 0
#if _USE_P2P_ABINET
		if (strsvr->is_p2p_abinet)
		{
			if (__p2p_abinet_sendn_core_3(strsvr, buf, r) < 0)
				connection_close = 1;
		}
		else
#endif
#if _USE_SSL_SERVER_
		if (strsvr->is_ssl)
		{
			if (__ssl_safe_write_socket(&strsvr->ssl_client_info, buf, r) < 0)
				connection_close = 1;
		}
		else
#endif
#endif
		{
			if (_safe_write_socket(strsvr->sock, buf, r) < 0)
				connection_close = 1;
		}
	}

	return (connection_close) ? -1 : 0;
}




#if __USE_WEBSOCKET_BY_TCP__
static char g_last_websocket_tcp_ip[32] = "";
char* get_last_websocket_tcp_ip()
{
	return g_last_websocket_tcp_ip;
}


static int _check_ws_protocol(struct st_stream * strsvr, ST_PHDR* phdr)
{
	int is_connect_accept = 0;

// #ifdef _USE_WEBSOCKET_SERVER_
	if (strsvr)
	{
	#ifdef _USE_WEBSOCKET_SERVER_
		if (strsvr->is_websocket)
		{
			is_connect_accept = 0;
		}
		else
	#endif
		{
			is_connect_accept = 1;
		}
	}
	else
// #endif

	{
		is_connect_accept = 0;
	}






	int ws_ret = 0;

	if (is_connect_accept)
	{
		char http_header[32];

		int bufsz = 102400;
		// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, bufsz);
		char* buf = malloc(bufsz);
		if (buf)
		{
			memset(http_header, 0, sizeof(http_header));
			memcpy(http_header, phdr, sizeof(ST_PHDR));

			printf("[WS] %s\n", http_header);

			if (strcasestr(http_header, "GET ") && strcasestr(http_header, "/ws "))
			{
				struct sockaddr_in server_addr;
				int target_port = DEFAULT_PORT_WEBSOCKET;

				printf("[WS] target_port = %d\n", target_port);

				threadsafe_gethostbyname("127.0.0.1", &server_addr, target_port);

				strncpy(g_last_websocket_tcp_ip, strsvr->cliinfo->ip, sizeof(g_last_websocket_tcp_ip));

				int web_sock = -1;
				if ((web_sock = creat_clisock_nb (& server_addr, 0, -1, NULL)) < 0) {
					printf("printf: Failed to connect client socket.(3) : WEB SOCKET SERVER!!!!\n ");
					ws_ret = -1;
					goto out;
				}

				if (is_connect_accept == 1 || is_connect_accept == 3)
				{
					int opt = 1;
					ioctl (strsvr->sock, FIONBIO, & opt);	// Set nonblocking mode.
				}

				g_netsvr.numclients_http++;

				__u64 data_no_rw_tm = get_time_msec();

				int connection_close = 0;

				if (_safe_write_socket(web_sock, http_header, sizeof(ST_PHDR)) < 0)
					connection_close = 1;

				while (connection_close == 0)
				{
					{
						int len = 0;
						{
							int r = read(strsvr->sock, buf, bufsz);
							if (r < 0)
							{
								if (errno != EAGAIN)
								{
									connection_close = 1;
									break;
								}
							}
							else if (r == 0)
							{
								connection_close = 1;
								break;
							}
							else
								len = r;
						}

						if (len > 0)
						{
							data_no_rw_tm = get_time_msec();

							if (connection_close == 0)
							{
								if (_safe_write_socket(web_sock, buf, len) < 0)
									connection_close = 1;
							}
						}
					}

					if (connection_close == 0)
					{
						int r = read(web_sock, buf, bufsz);
						if (r < 0)
						{
							if (errno != EAGAIN)
							{
								connection_close = 1;
								break;
							}
						}
						else if (r == 0)
						{
							connection_close = 1;
							break;
						}
						else
						{
							data_no_rw_tm = get_time_msec();

							if (connection_close == 0)
							{
								if (_ws_safe_write_socket(strsvr, buf, r) < 0)
									connection_close = 1;
							}
						}
					}

					if (get_time_msec() - data_no_rw_tm > 30000)
					{
						printf("[WEB SOCKET] Timeout : %llu\n", get_time_msec() - data_no_rw_tm);
						connection_close = 1;
					}

					sleep_ex(1);
				}

				printf("[WEB SOCKET] _____CLOSE\n");

				if (web_sock >= 0)
					close(web_sock);

				ws_ret = 1;
			}
		}

	out:
		if (buf)
			free(buf);

		if (ws_ret && g_netsvr.numclients_http > 0)
			g_netsvr.numclients_http--;
	}

	return ws_ret;
}
#endif




static int _check_http_protocol(struct st_stream * strsvr, ST_PHDR* phdr)
{

	int http_ret = 0;


#if _USE_WEB_SERVICE_BY_TCP_PORT_
	int grow_sz = 10240;
	int http_header_sz = grow_sz;
	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, http_header_sz);
	char* http_header = malloc(http_header_sz);
	
	if (http_header)
	{
		memset(http_header, 0, http_header_sz);
		memcpy(http_header, phdr, min(http_header_sz, sizeof(ST_PHDR)));


		if (strcasestr(http_header, "HTTP/") || strcasestr(http_header, "GET ") || strcasestr(http_header, "POST ") || strcasestr(http_header, "PUT ") || strcasestr(http_header, "PATCH ") || strcasestr(http_header, "DELETE "))
		{
			struct sockaddr_in server_addr;
			int target_port = g_setup.net.port_web;

#if __USE_WEBSOCKET_BY_TCP__
			if (strcasestr(http_header, "GET ") && strcasestr(http_header, "/ws "))
			{
				http_ret = _check_ws_protocol(strsvr, phdr);
				goto out;
			}
#endif


			threadsafe_gethostbyname("127.0.0.1", &server_addr, target_port);

			int web_sock = -1;

			g_netsvr.numclients_http++;

			int flag = 0;
			int p = sizeof(ST_PHDR);

			while (1)
			{
				int rd;
	re_read_http:
				rd = recvmsgfrom (strsvr, strsvr->sock, http_header + p, 1);
				if (rd == 1)
				{
					// p2p_wait_start_tm = get_time_msec();

					if (http_header[p] == '\r')
					{
						if (flag == 2)
							flag = 3;
						else
							flag = 1;
					}
					else if (http_header[p] == '\n' && (flag == 1 || flag == 3))
					{
						if (flag == 3)
							flag = 4;
						else
							flag = 2;
					}
					else
						flag = 0;

// #if _USEprintf_
#if 0
					if (http_header[p] == 13)
						fprintf(stdout, "[\\r]");
					else if (http_header[p] == 10)
						fprintf(stdout, "[\\n]\n");
					else
						fprintf(stdout, "%c", http_header[p]);
#endif

					p++;

					if (http_header_sz - 1 <= p)
					{
						int new_size = http_header_sz + grow_sz;
						// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, new_size);
						char* new_header = malloc(new_size);
						if (!new_header)
							break;
						else
						{
							debug(">> new size = %d\n", new_size);

							memcpy(new_header, http_header, http_header_sz);
							free(http_header);
							http_header = new_header;
							http_header_sz = new_size;
						}
					}

					if (flag == 4)
					{
						printf("! [HTTP-Recv] len=%d, %d\n------------\n%s\n-----------\n", strlen(http_header), p, http_header);

						char* content = NULL;
						int content_length = 0;

						char* clp = strcasestr(http_header, "Content-Length");
						if (clp)
						{
							content_length = get_param_val_with_end(clp, ":", "\r\n");

							printf("! [HTTP] Content-length : %d\n", content_length);

							if (content_length > 0)
							{
								// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, content_length);
								content = malloc(content_length + 1);
								if (content)
								{
									memset(content, 0, content_length + 1);

									int w, s = 0, r = content_length;
									while (r > 0)
									{
										w = recvmsgfrom (strsvr, strsvr->sock, content + s, r);
										if (w < 0)
										{
											if (errno != EAGAIN)
												break;
										}
										else if (w == 0)
										{
											break;
										}
										else
										{
											s += w;
											r -= w;
										}
									}

									printf("! [HTTP-Recv] content = %s\n", content);
								}
							}
						}

						if (web_sock < 0)
						{
							if ((web_sock = creat_clisock_nb (& server_addr, 0, -1, NULL)) < 0) {
								printf("printf: Failed to connect client socket.(3)\n ");
								http_ret = -1;
								goto out;
							}
						}

						int w, n = 0, len = p;

						//WEBTUNNEL_DEBUG("session_send_data : id = %d, len = %d (%d, %d)\n%s", web->s_session_id, len, strlen(data->webdataptr), data->datasz, data->webdataptr);
						{
							printf("! ------------------------------------ send data\n%s", http_header);
							do
							{
								w = write(web_sock, http_header + n, len - n);
								if (w < 0)
								{
									if (errno != EAGAIN)
										break;
								}
								else if (w == 0)
								{
									break;
								}
								else
								{
									n += w;
									if (n == len)
										break;
								}
							}
							while (w > 0);
						}

						if (content)
						{
							printf("! ------------------------------------ send data (content)\n%s", content);

							n = 0;
							len = content_length;

							do
							{
								w = write(web_sock, content + n, len - n);
								if (w < 0)
								{
									if (errno != EAGAIN)
										break;
								}
								else if (w == 0)
								{
									break;
								}
								else
								{
									n += w;
									if (n == len)
										break;
								}
							}
							while (w > 0);

							free(content);
						}


						printf("! [HTTP] Send to web server...................................\n");

						int do_read_cnt = 0;

						do
						{
							int do_read = net_sock_is_readable_data(NULL, web_sock, 10);
							if (do_read <= 0)
								break;
							else if (do_read > 0)
							{
								printf("! [HTTP] do_read = %d\n", do_read);

								// 읽을 데이터가 있다면 읽어서 Client에 전송한다.
								int bufsz = 102400;
								// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, bufsz);
								char* buf = malloc(bufsz);
								if (buf)
								{
									do
									{
										memset(buf, 0, bufsz);

										int r = read(web_sock, buf, bufsz);
										if (r < 0)
										{
											if (errno != EAGAIN)
												break;
										}
										else if (r == 0)
											break;
										else
											do_read_cnt = 0;

#if 0
									#if _USE_P2P_ABINET_DEBUG_
										{
											int k, _len = strlen(buf);
											for (k = 0; k < _len; k++)
											{
												if ((buf[k] >= 32 && buf[k] <= 126) || buf[k] == '\r' || buf[k] == '\a')
													fprintf(stdout, "%c", buf[k]);
											}
										}
									#endif
#endif
										int s = 0;
										while (r > 0)
										{
											w = sendmsgto(strsvr, strsvr->sock, buf + s, r);
											if (w < 0)
											{
												if (errno != EAGAIN)
													break;
											}
											else if (w == 0)
											{
												break;
											}
											else
											{
												s += w;
												r -= w;
											}
										}
									}
									while (0);

									free(buf);
								}

								sleep_ex(1);

								if (net_sock_is_readable_data(NULL, web_sock, 0) != 1)
									break;

								do_read_cnt++;
							}
						}
						while (do_read_cnt < 100);

						printf("! [HTTP] Return to client.........................................\n");

						if (content_length > 0)
						{
							if (web_sock >= 0)
								close(web_sock);
							web_sock = -1;
						}

						if (do_read_cnt >= 100)
						{
							printf("do_read_cnt = %d\n", do_read_cnt);
							printf("Connect Close (1): rd = %d / flag = %d\n", rd, flag);
							break;
						}

						int do_read = net_sock_is_readable_data(strsvr, strsvr->sock, 3);
						if (do_read == 1)
						{
							printf("! Read Again...\n");

							// p2p_wait_start_tm = get_time_msec();

							memset(http_header, 0, http_header_sz);
							p = 0;
							goto re_read_http;
						}
						else
						{
							printf("! END...\n");
							break;
						}
					}
				}
				else
				{
					int is_close = 1;
					if (is_close)
					{
						printf("Connect Close (2): rd = %d / flag = %d\n", rd, flag);
						break;
					}
				}
			}

			//REL_OUT("[HTTP] %d\n%s\n", ret1, http_body);

			if (web_sock >= 0)
				close(web_sock);
			web_sock = -1;

			http_ret = 1;
		}
	}

out:
	if (http_header)
		free(http_header);

	if (http_ret && g_netsvr.numclients_http > 0)
		g_netsvr.numclients_http--;

#endif

	return http_ret;
}

