/*
 * netsvr.h
 *
 * purpose:
 *	- implement network server thread.
 ************************************************************************************/

#include "common.h"
#include "lib/threadsafe.h"
#include "lib/net.h"
#include "netsvr/notify.h"
#include "netsvr/server.h"
#include "netsvr/websocket.h"
#include "rtsp/RTSP_common.h"
#include "device/gpio.h"

#include "wsServer/websocket_server.h"

ST_NETSVR	g_netsvr;

struct tm g_sync_tm;
char g_ntpsvr[64] = "pool.ntp.org";


static void netsvr_release(ST_NETSVR *netsvr) {

	netsvr->th.state = STAT_TERMINATED;
	flush_msg_queue(&netsvr->th.msgq);

	flush_cliinfo(netsvr);


}

static int netsvr_init (ST_NETSVR *netsvr) {
	int i;
	assert (netsvr);

	netsvr->th.state = STAT_UNREADY;
	init_msg_queue(& netsvr->th.msgq, NETSVR);


	for (i = 0; i < 32; i ++)	{
		memset(&netsvr->strmsvr[i], 0, sizeof(struct st_stream));
		netsvr->strmsvr[i].inuse = FALSE;
	}

	netsvr->numclients = 0;
	init_clist (& netsvr->cliinfo_head);
	pthread_mutex_init (& netsvr->cliinfo_mutex, NULL);

	netsvr->nummclients = 0;
	init_clist (& netsvr->mcliinfo_head);
	pthread_mutex_init (& netsvr->mcliinfo_mutex, NULL);


	foreach_cam_ch (i)
		init_live_framebuf (& netsvr->live_frame_buf [i]);
	end_foreach


#ifdef	__USE_RTSP_SERVER__
	for (i = 0; i < 32; i ++)
		RTSPCTX_init (& (r_ctx [i]));
#endif


	netsvr->fw_upgrade = FALSE;

	// namba
	if (__system ("echo '180' > /proc/sys/net/ipv4/tcp_keepalive_time") == -1)
		return -ERR_DEV;

	netsvr->th.state = STAT_READY;
	netsvr->islink = is_net_link();

	return 0;
}



// Disconnect all connections and terminated stream service threads.
int terminate_strmsvr(ST_NETSVR *netsvr, int term_code) {
	int i;

	assert (netsvr);

	for (i=0; i < 32; i++)
		netsvr->strmsvr[i].terminate = term_code;

	for (i=0; i < 32; i++)
		netsvr->mstrmsvr[i].terminate = 1;

#ifdef	__USE_RTSP_SERVER__
	SVR_CTX	*slot = NULL;
	foreach_context(slot, i)
		slot->c_term = 1;
	end_foreach
#endif

	//w4000 sleep_ms (1000);
	return 0;
}


/**************************************************************************************
 * Message handling.
 **************************************************************************************/
static int handle_msg_start(struct st_message *msg) {
	assert (msg);

	// only resume thread
	if (msg->parm1) {
		setup_network_apply(&g_setup, &g_setup_ext, 1, 1, 0);

		if (g_setup.net.type == NETTYPE_DHCP) {
			setup_save (& DEV_MTD, & g_setup, & g_setup_ext);
			g_tmpset = g_setup;
			g_tmpset_ext = g_setup_ext;
		}
	}

	if (g_netsvr.th.state == STAT_RUNNING) {
		return 0;
	}

#ifdef	__USE_RTSP_SERVER__
	RTSP_open_listen ();
#endif


#ifdef _USE_WEBSOCKET_SERVER_
	{
		static int s_init_ws = 1;
		if (s_init_ws)
		{
			s_init_ws = 0;

#if 1
			websocket_init();
			websocket_start();
#endif 
		}
	}
#endif


	IPCAM_THREAD (NETSVR)->state = STAT_RUNNING;

	return 0;
}

static int handle_msg_stop(struct st_message *msg) {

	if (g_netsvr.th.state == STAT_READY) {
		return 0;
	}



#ifdef	__USE_RTSP_SERVER__
	if (rtsp_fd >= 0) {
		shutdown (rtsp_fd, SHUT_RDWR);
		close (rtsp_fd);
	}
#endif

	terminate_strmsvr (& g_netsvr, TERM_CODE_MSG_STOP);

	if (msg->parm1) {
		__system ("killall dhclient"); 	// Previous connection could be DHCP.
		stop_webserver();
	}

	IPCAM_THREAD (NETSVR)->state = STAT_READY;
	return 0;
}

static int handle_msg_rtsp_restart(struct st_message *msg) {
#ifdef	__USE_RTSP_SERVER__
	if (rtsp_fd >= 0) {
		shutdown (rtsp_fd, SHUT_RDWR);
		close (rtsp_fd);
	}

	RTSP_open_listen ();
#endif

	return 0;
}


int handle_msg_new_ssf(struct st_message *msg) {
	int cam = msg->parm1;

	return 0;

	if (g_netsvr.th.state == STAT_RUNNING) {
		struct st_stream *svr = NULL;
		int i;

		for (i = 0; i < 32; i ++) {
			svr = & g_netsvr.strmsvr[i];

			if (!svr)
				continue;

			if (! svr->inuse)
				continue;

			if (ISLIVE (svr, cam) && ISFLAG (svr, cam, __SS_SENDIMG))
				LIVE (svr, cam).transhdr = TRUE;
		}
	}

	return 0;
}


static int handle_msg_mstrsvr_creat(struct st_message *msg) {

	struct st_mstream *svr= NULL;

	svr = (struct st_mstream *)msg->parm1;

	g_netsvr.nummclients++;

	return 0;
}


#if 1





static void * handle_msg_tmsync_ntp_proc (void * arg)
{
	FILE * fp = NULL;
	char buf [256];
	char cmd[128];
	int off_sec, off_usec;
	int tm_restart = 0;

	sprintf(cmd, "/edvr/ntpdate %s", g_ntpsvr);

		char * tmp = NULL;
		if ((tmp = strstr (buf, "offset"))) {
			if (sscanf (tmp, "offset %d.%06d sec", & off_sec, & off_usec) > 0) {
				if (abs(off_sec) >= 1 || (abs(off_sec) == 0 && off_usec > 990000))	
					tm_restart = 1;
			} 
		} 


	datetime_systohc ();
	system("date");

	if (tm_restart)
		send_msg(NETSVR, TIMER, _CAMMSG_TM_RESTSRT, 1, 0, 0);

	return NULL;
}


static int handle_msg_tmsync_ntp (struct st_message * msg)
{
	pthread_t tid;
	int type;

	assert (msg);

	type = msg->parm3;

	printf("w4000 m %s, %d\n",__FUNCTION__,__LINE__);
	
	{
		strcpy(g_ntpsvr, g_setup.tmsync.ntpsvr);

		if (pthread_create (& tid, NULL, handle_msg_tmsync_ntp_proc, NULL)==0) {
			pthread_detach(tid);
		}
	}

	return 0;
}
#endif

static int handle_msg_strsvr_exit(struct st_message *msg) {

	struct st_stream *svr = (struct st_stream *)msg->parm1;


	release_stream (& g_netsvr, svr);

	if (g_netsvr.numclients > 0)
		g_netsvr.numclients--;



	return 0;
}

static int handle_msg_mstrsvr_exit(struct st_message *msg) {

	struct st_mstream *svr = (struct st_mstream *)msg->parm1;

	release_mstream (& g_netsvr, svr);

	g_netsvr.nummclients--;

	return 0;
}


#ifdef USE_STREAM_CONSUMER
int add_stream_consumer(int ch_flag, consumer_cb cb)
{
	int id = -1;
	id = __creat_streamsvr_for_consumer(ch_flag, cb);
	if (id < 0) {
		return id;
	}

	return id;
}

void del_stream_consumer(int id)
{
	__release_streamsvr_for_consumer(id);
}
#endif

void *netsvr(void *arg)
{
	int bIsExit = FALSE;
	struct st_message msg;

	int loop_count = 0;
	int link_status_change = 0;
	int select_error = 0;

	if (netsvr_init(&g_netsvr) < 0) {
		goto out;
	}


	while(1) {
		if (get_msg(& IPCAM_THREAD (NETSVR)->msgq, &msg) > 0) {
			g_main.thread_work_check[THREAD_ALIVE_CHECK_NETSVR] = msg.msgid;
			switch (msg.msgid) {
			case _CAMMSG_ETHERNET_LINK:
				g_netsvr.islink = msg.parm1;
				link_status_change++;
			
				break;

			case _CAMMSG_START:
				{
					int retry = 10;
					while(handle_msg_start(&msg) < 0) {
						usleep(300*1000);
						handle_msg_stop(&msg);
						if(retry-- <= 0) {
							break;
						}
					}
					if(g_netsvr.islink == 0) {
						g_netsvr.islink = is_net_link();
					}
				}
				break;

			case _CAMMSG_STOP:
				handle_msg_stop(&msg);
				break;

			case _CAMMSG_RTSP_RESTART:
				handle_msg_rtsp_restart(&msg);
				break;





			case _CAMMSG_MOTION:
				{
					int only_first_one = 0;

					send_msg (NETSVR, NETSVR, _CAMMSG_EVNT_MOTION2, msg.parm1, 1, msg.parm3);	// for SDK

					if (msg.parm3 == 1) // Only Start Event
						only_first_one = 1;
					else if (msg.parm3 != 0)
						msg.parm3 = 1;

				}
				break;

			case _CAMMSG_TMSYNC_NTP:
				handle_msg_tmsync_ntp (& msg);
				printf("w4000 m %s, %d\n",__FUNCTION__,__LINE__);
				break;

			case _CAMMSG_STRSVR_EXIT:
				handle_msg_strsvr_exit(&msg);
				if (bIsExit && g_netsvr.numclients == 0 && g_netsvr.nummclients == 0)
					goto out;
				break;

			case _CAMMSG_EXIT:
				terminate_strmsvr(&g_netsvr, TERM_CODE_MSG_EXIT);	
				bIsExit = TRUE;
				if (g_netsvr.numclients == 0 && g_netsvr.nummclients == 0)
					goto out;
				break;

			default :
				break;
			}
		}

		if (IPCAM_THREAD (NETSVR)->state == STAT_RUNNING) {

#ifdef __USE_RTSP_SERVER__
			RTSP_serv ();
#endif
		}

		g_main.thread_alive_check[THREAD_ALIVE_CHECK_NETSVR] = 0;

		if(loop_count++ > 100*60) {	
			loop_count = 0;
			if(link_status_change > 10) {
				send_msg (NETSVR, MAIN, _CAMMSG_REBOOT, 5, 0, 0);
			}
			link_status_change = 0;
		}

		sleep_ex (10);	
	}

out:
	netsvr_release(&g_netsvr);

	int ch;
	for (ch = 0; ch < MAX_ENC_CHANNEL; ch++)
	{
		encoder_prealrm_release_all_frame(ch);
		live_release_all_frame(ch);

	}

#ifdef __USE_RTSP_SERVER__
	{
		int i;
		for (i = 0; i < 32; i ++)
		{
			RTSPCTX_release (& (r_ctx [i]));
		}
	}
#endif

	pthread_exit(NULL);
}



int get_netsvr_client_number()
{
	return max(0, g_netsvr.numclients - g_netsvr.numclients_http);
}
