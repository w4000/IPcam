
#include <stdio.h>
#include <sys/sysinfo.h>
#include <sys/ucontext.h>

#include "common.h"
#include "file/hdd.h"
#include "lib/net.h"
#include "lib/upgr.h"
#include "lib/threadsafe.h"

#include "lib/export.h"

#include "rtsp/RTSP_common.h"
#include "device/gpio.h"
#include "isp/isp_ctrl.h"
#include "timer/timer.h"
#include "device/tty.h"

#include "kmsg/kmsg.h"
#include "ipc/ipc_protocol.h"
#include "ipc/ipc_server.h"
#include "ipc/ipc_status.h"
#include "file/ssf.h"

#include "_config/model_spec.h"

#define INI_IMPLEMENTATION
#include "ini.h"
#include <stdlib.h>


#ifdef USE_NEW_ONVIF
#include "onvifserver.h"
#endif


typedef struct _INI_PRIV_S {
	ini_t *ini;
	char file[128];
} INI_PRIV_S;

#define SETUP_EXT_IMPLEMENTATION
#include "setup/setup_ext.h"

#include "./tools/system_reboot.c"



_CAMMAIN	g_main;



#define is_digit(x) ((x) >= '0' && (x) <= '9')
#define is_hexalpha(x) (((x) >= 'a' && (x) <= 'f') || ((x) >= 'A' && (x) <= 'F'))
#define is_hexdigit(x) (is_digit(x) || is_hexalpha(x))
#define to_num(x) (((x) >= '0' && (x) <= '9') ? ((x)-'0') \
					: ((x) >= 'a' && (x) <= 'f') ? ((x)-'a'+10) \
					: ((x) >= 'A' && (x) <= 'F') ? ((x)-'A'+10) : 0)

static int32_t addr2line(char* program_name, int32_t name_length, void const *const addr);
static uint64_t hexString2Number(char *hex_string);


void sig_exit_handler(int sig)
{
    //pthread_t tid = pthread_self();
    signal(SIGINT, SIG_DFL);

   	g_timer.watdog.reboot (&g_timer.watdog);

   	g_main.bQuit = TRUE;
    send_msg (MAIN, MAIN, _CAMMSG_EXIT, 0, 0, 0);
}

/* SIG_USR1 handler.
 * Used by main thread for WWW remote setup.
 */
void sigusr1_handler(int signo)
{
}

void sigusr2_handler(int sig)
{
}




static char g_event_buf[1024], g_event_buf2[1024];


static int	_msg_evnt_motion (ST_MESSAGE *msg)
{
	int ch, startend;
	static int mot_cnt = 0;
	char mot[8];

	assert (msg);
	assert (VALIDCH (msg->parm1));

	if (g_main.th.state == STAT_READY)
		return 0;

	ch = msg->parm1;
	startend = msg->parm3;


	if(startend) {
		mot_cnt++;
		if(mot_cnt > 1000) {
			mot_cnt = 1;
		}
	}
	else {
		mot_cnt = 0;
	}
	sprintf(mot, "%d", mot_cnt);
	ipc_status_set("event", "motion", mot);

	return 0;
}



static int handle_msg_setsystm_direct (struct tm * tm, int byset, int isUtc)
{
	int diffsec = 0;

	setup_datetime_apply (& g_setup, tm, byset);

	return 0;
}


static int handle_msg_reboot_force (ST_MESSAGE * msg)
{
	// TODO
	return 0;
}

#if defined(NARCH)
enum { CHANGE_SETUP, KEEP_ENCODER, RESTART_ENCODER, };

static int changedVideoSetup(_CAMSETUP* curSetup, _CAMSETUP_EXT* cSetupExt, _CAMSETUP* cgiSetup, _CAMSETUP_EXT* cgiSetupExt)
{
	int ret = CHANGE_SETUP;
	int i;
	
	for(i = 0;i < 3;i++) {
		if(cgiSetup->cam[i].res != curSetup->cam[i].res) {
			return RESTART_ENCODER;
		}
		if(cgiSetup->cam[i].codec != curSetup->cam[i].codec) {
			return RESTART_ENCODER;
		}
	}

	for(i = 0;i < 3;i++) {


		if(cgiSetup->cam[i].fps != curSetup->cam[i].fps
				|| cgiSetup->cam [i].bitrate != curSetup->cam [i].bitrate
				|| cgiSetup->cam [i].ratectrl != curSetup->cam [i].ratectrl) {
			ret = KEEP_ENCODER;
			break;
		}
	}



	return ret;
}
#endif	/* NARCH */

static void * _handle_msg_dhcp_check(void *arg)
{
	if (g_setup.net.type == NETTYPE_DHCP && is_net_link())
	{
		if(is_dhcp_renew()) {
			_CAMSETUP	set = g_setup;
			_CAMSETUP_EXT	set_ext = g_setup_ext;
			setup_network_dhcp_apply (&set);
			if(set.net.ip.s_addr != g_setup.net.ip.s_addr)
			{
				char old_ip[32];
				char new_ip[32];
				old_ip [31] = '\0';
				new_ip [31] = '\0';
				strncpy (new_ip, inet_ntoa (set.net.ip), 31);
				strncpy (old_ip, inet_ntoa (g_setup.net.ip), 31);

				g_setup = g_tmpset = set;
				setup_save (& DEV_MTD, & g_setup, & g_setup_ext);
				setup_network_final_apply(&set, &set_ext, 1);
				send_msg (MAIN, NETSVR, _CAMMSG_STOP, 0, 0, 0);
				send_msg (MAIN, NETSVR, _CAMMSG_START, 0, 0, 0);
			}
		}
	}


	return NULL;
}

static int _msg_dhcp_check(ST_MESSAGE * msg)
{
	pthread_t tid;

	if (pthread_create (& tid, (void*)msg, _handle_msg_dhcp_check, NULL) == 0) {
		pthread_detach (tid);
		return 0;
	}

	return -1;
}


static const char *_sig_list[] = 
{
	"NULL",
	"SIGHUP",
	"SIGINT",
	"SIGQUIT",
	"SIGILL",
	"SIGTRAP",
	"SIGABRT",
	"SIGBUS",
	"SIGFPE",
	"SIGKILL",
	"SIGUSR1",
	"SIGSEGV",
	"SIGUSR2",
	"SIGPIPE",
	"SIGALRM",
	"SIGTERM",
	"SIGSTKFLT",
	"SIGCHLD",
	"SIGCONT",
	"SIGSTOP",
	"SIGTSTP",
	"SIGTTIN",
	"SIGTTOU",
	"SIGURG",
	"SIGXCPU",
	"SIGXFSZ",
	"SIGVTALRM",
	"SIGPROF",
	"SIGWINCH",
	"SIGIO",
	"SIGPWR",
	"SIGSYS"
};




#define BT_TRACE_SIZE	16

static int main_init (void)
{
	int i;


	memset(&g_main, 0, sizeof(g_main));

	g_main.th.tid = pthread_self ();
	g_main.th.state = STAT_UNREADY;

	init_msg_queue (& g_main.th.msgq, MAIN);

	g_main.bQuit = FALSE;


#if 0
	struct sigaction sa;
	sa.sa_handler = (void *)_nf_signal_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGABRT, &sa, NULL);
#else 
	// set_signal_handler(_nf_signal_handler, SIGABRT);
#endif 


	


	set_signal_handler(sig_exit_handler, SIGINT);
	set_signal_handler(SIG_IGN, SIGPIPE);
	set_signal_handler(sigusr1_handler,  SIGUSR1);
	set_signal_handler(sigusr2_handler,  SIGUSR2);

	g_encoder.th.state = STAT_UNREADY;
	g_file.th.state = STAT_UNREADY;
	g_timer.th.state = STAT_UNREADY;
	g_netsvr.th.state = STAT_UNREADY;


	flash_init (& g_main.flash, FLASH_DEVICE3);

	// Set default user (admin)
	sprintf(g_main.usrname, model_spec_string("SYSTEM_USER_NAME"));

	g_main.m_sensor = 0;

	g_main.mot = 0;

	g_main.vloss = 0;
	g_main.diskfull = FALSE;
	g_main.recfail = FALSE;
	g_main.alarm = 0;

	for (i = 0; i < RELAYOUT_MAX; i++) {
		g_main.relayend [i] = (__u64)0;
		g_main.relayforce [i] = 0;
	}


	g_main.sw_ver[0] = atoi(RELVER);
	g_main.sw_ver[1] = atoi(MAJVER);
	g_main.sw_ver[2] = atoi(MINVER);




	g_main.fixed_modified = FALSE;
	g_main.on_config = FALSE;
	g_main.usage_cpu = 0;
	g_main.free_mem = 0;

	g_main.system_fps = FPS_MAX;
	g_main.codec.numframe = 0;

	pthread_mutex_init(&g_main.rtsp_mutex, NULL);
	g_main.isDHCP = 0;

	g_main.th.state = STAT_READY;

	return 0;
}


static void main_release (void)
{
	int i;


	flush_msg_queue(& g_main.th.msgq);

	g_main.th.state = STAT_TERMINATED;
#if 1
	sleep(1);//w4000_sleep(1);
#endif
	// hdd_umount(g_file.hddinfo);

	{
		pthread_mutex_destroy(&g_main.rtsp_mutex);
	}

	{
		for (i = 0; i < 3; i++)
		{
			encoder_prealrm_release_all_frame(i);
			//live_release_all_frame(i);
		}
	}

	{
		int mm;
		struct st_message msg;
		for (i = 0; i < MSG_THREAD_MAX; i++)
		{
			if (MSGQ (i))
			{
				do
				{
					mm = get_msg (MSGQ (i), & msg);
				}
				while (mm > 0);
			}
		}
	}



}

void main_restore_setting (int force, int upgr_mode)//w4000_default
{
	_CAMSETUP tmp_setup;	// Temorary setup data.
	_CAMSETUP_EXT tmp_setup_ext;

	
	if (file_exist(CGI_UPGRADE))
		remove_file(CGI_UPGRADE, 0);
	
	setup_ini_load();
	

	if (force) {
		
		setup_load_default_netskip (& tmp_setup, & tmp_setup_ext, DEV_QUAD.vsig);
		
		setup_save (& DEV_MTD, & tmp_setup, & tmp_setup_ext);
		
		
	}
	else {
		
		if (setup_load(& DEV_MTD, & tmp_setup, & tmp_setup_ext) < 0) {
			
			
			if(setup_check_no_encryption(& tmp_setup, & tmp_setup_ext) < 0) {
				
				setup_load_default (& tmp_setup, & tmp_setup_ext, DEV_QUAD.vsig, 0);
				
				}		
			
			setup_save (& DEV_MTD, & tmp_setup, & tmp_setup_ext);
			
			
		}
	}

	
	
	setup_range_nsetup(&tmp_setup.nsetup);
	
	

	{

		if (tmp_setup.cam [0].bitrate > BITRATE_16000)
			tmp_setup.cam [0].bitrate = BITRATE_16000;
		if (tmp_setup.cam [1].bitrate > BITRATE_2000)
			tmp_setup.cam [1].bitrate = BITRATE_2000;
		if (tmp_setup.cam [2].bitrate > BITRATE_1100)
			tmp_setup.cam [2].bitrate = BITRATE_1100;


		if (tmp_setup.cam[1].fps > MAX_SUB_FPS) {
			tmp_setup.cam[1].fps = MAX_SUB_FPS;
		}
		

		
		{
			if (tmp_setup_ext.isp.exp.brightness == 0 || tmp_setup_ext.isp.exp.gain == 0)
			{
				set_isp_default(&tmp_setup_ext);
			}

			if(tmp_setup_ext.isp.exp.shutter == SS_AUTO || tmp_setup_ext.isp.exp.shutter == SS_ANTIFLICKER) {
				tmp_setup_ext.isp.exp.shutter_speed[0] = SS_30000;
				tmp_setup_ext.isp.exp.shutter_speed[1] = SS_30;
			}
			else if(tmp_setup_ext.isp.exp.shutter > SS_30 &&
					tmp_setup_ext.isp.exp.shutter_speed[0] == tmp_setup_ext.isp.exp.shutter_speed[1]) {	// ???? Manual Mode
				tmp_setup_ext.isp.exp.shutter = SS_30;
				tmp_setup_ext.isp.exp.shutter_speed[0] = SS_30000;
				tmp_setup_ext.isp.exp.shutter_speed[1] = SS_30;
			}

			if(tmp_setup_ext.isp.special.dnr < 1 || tmp_setup_ext.isp.special.dnr > 7) {
				tmp_setup_ext.isp.special.dnr = 4;
			}
		}
		
#ifdef USE_NEW_ONVIF
		if(tmp_setup_ext.onvif_port <= 0 || tmp_setup_ext.onvif_port > 65535) {
			tmp_setup_ext.onvif_port = 8000;
			tmp_setup_ext.onvif_use_https = 0;
			tmp_setup_ext.onvif_use_discovery = 1;
		}
		tmp_setup_ext.onvif_use_discovery = 1;
#endif
	
	}

	DEV_QUAD.vsig = tmp_setup.videotype;

	g_setup = tmp_setup;
	g_setup_ext = tmp_setup_ext;

	if_get_mac (ETHERNET_DEV, IPCAM_THREAD_MAIN->mac);
	
	if(upgr_mode == 0) {
		setup_apply (& g_setup, & g_setup_ext);
	}
	
	datetime_tzset (g_setup.tmzone);	// Set timezone. //w4000_tm
	
	datetime_hctosys();
	
	__system("date");


	setup_save (& DEV_MTD, & g_setup, & g_setup_ext);
	tmp_setup = g_setup;
	tmp_setup_ext = g_setup_ext;

}


static int _msg_cgiimport (ST_MESSAGE * msg)
{
	_CAMSETUP set;
	_CAMSETUP_EXT set_ext;
	int update_type;
	int update_type3;
	assert (msg);
	int i, use_system_account_flag;

#ifdef USE_NEW_ONVIF
	int update_type2 = msg->parm2;
#endif


	update_type = msg->parm1;
	update_type3 = msg->parm3;

	setup_lock();

#ifdef USE_SETUP_ENCRYPTION
	setup_ext_default(&set_ext);

	if(read_enc_setup(&set, &set_ext) < 0) {
		return -1;
	}
#else
	const char* setup_file = CGI_SETUPFILE;
	const char* setup_ext_file = CGI_SETUPFILE_EXT;

	FILE *fp = NULL, *fp_ext = NULL;
	if ((fp = fopen (setup_file, "rb")) == NULL) {
		perror ("fopen");
		setup_unlock();
		return -1;
	}

	if (fread ((void *) & set, 1, sizeof (set), fp) != sizeof (set)) {
		perror ("fread");
		fclose (fp);
		setup_unlock();
		return -1;
	}

	if (fp)
		fclose (fp);


	setup_ext_default(&set_ext);

	fp_ext = fopen (setup_ext_file, "rb");
	if (fp_ext)
	{
		int magic2[4], size2 = 0;
		memset(magic2, 0, sizeof(magic2));
		fread(magic2, 1, sizeof(magic2), fp_ext);
		if (magic2[0] == MAGIC_NUM_1 && magic2[1] == 0x00710225 && magic2[2] == 0x00951220 && magic2[3] == 0x00981029) {
			fread(&size2, 1, sizeof(int), fp_ext);
			if (size2 > 0) {
				int sz2, len2 = min(size2, sizeof(_CAMSETUP_EXT)) - sizeof(magic2) - sizeof(int);
				if (len2 > 0)
				{
					sz2 = fread(&(set_ext.reserved), 1, len2, fp_ext);
					if (sz2 != len2) {
						perror ("fread");
						fclose (fp_ext);
						setup_unlock();
						return -1;
					}

					memcpy(set_ext.magic, magic2, sizeof(magic2));
					set_ext.sizeOfStruct = size2;
				}
			}
		}
	}
	if (fp_ext)
		fclose (fp_ext);
#endif

	if (setup_check_valid (& set) == 0) {
		printf ("Invalid rsetup file!\n");
		setup_unlock();
		return -1;
	}

	g_main.fixed_modified = TRUE;


	

	switch (update_type) {

	
	case UPDATE_SETUP_CAM:
		{

			int bIsRtsp = 0, bIsPos = 0;
			int restart_server = 0;
			int bIsHttp = 0, bIsHttps = 0;
			int change_timesync = 0;
			int update = 0;

			// 

		int video_update = changedVideoSetup(&g_setup, &g_setup_ext, &set, &set_ext);
			printf("w4000 m %s, %d\n",__FUNCTION__,__LINE__);
		
		if (set.videotype != g_setup.videotype ||
		    video_update == RESTART_ENCODER) {



			g_setup.videotype = set.videotype;
			
			
			for(i = 0; i < 3; i++) {
				set.cam[i].fps = 30;
				set.cam [i].gop = 30;

				if(set.cam[i].fps > 30) {
					set.cam[i].fps = 30;
					set.cam [i].gop = 30;

				}
			}


				if(video_update == RESTART_ENCODER) {
					g_setup = g_tmpset = set;
					g_setup_ext = g_tmpset_ext = set_ext;
					
					setup_save (& DEV_MTD, & g_setup, & g_setup_ext);
				}


				
				send_msg(MAIN, ENCODER, _CAMMSG_EXIT, 0, 0, 0);
				sleep_ex (100000);
				// sleep(1);



				
				send_msg (MAIN, MAIN, _CAMMSG_REBOOT, 7, 0, 0);

				
				if(update_type2 == 0) {
					
					onvifserver_set_setup(&g_setup, &g_setup_ext);
					onvifserver_update_video_encoder();
				}


				goto set_out;
			}


			if(video_update == KEEP_ENCODER || set_ext.cgi_update_notify[UPDATE_CGI_VIDEO] == 1) {

				
				if (set_ext.cgi_update_notify[UPDATE_CGI_VIDEO])
					set_ext.cgi_update_notify[UPDATE_CGI_VIDEO] = 0;
				
				g_setup = g_tmpset = set;
				g_setup_ext = g_tmpset_ext = set_ext;

				setup_save (& DEV_MTD, & g_setup, & g_setup_ext);
				send_msg (MAIN, ENCODER, _CAMMSG_SETUP, 0, 0, 0);


				

				if(update_type2 == 0) {
					
					onvifserver_set_setup(&g_setup, &g_setup_ext);
					onvifserver_update_video_encoder();
				}


				
				goto set_out;
			}
			

			if (update_type3 == 1)
			{
				
				goto set_out;
			}

			
			
			for (i =0; i < 16; i++)
			{
				
				use_system_account_flag = 0;

				if (set.usr[i].avail ==0 && g_setup.usr[i].avail ==0)
					goto out;

				if ( (set.usr[i].avail != g_setup.usr[i].avail) ||
					(strcmp(set.usr[i].usrname, g_setup.usr[i].usrname) != 0) ||
					(strcmp(set.usr[i].usrname, g_setup.usr[i].usrname) != 0) ||
					(strcmp(set.usr[i].usrpwd, g_setup.usr[i].usrpwd) != 0) ||
					(strcmp(set.usr[i].usrdesc, g_setup.usr[i].usrdesc) != 0) )
				{
					if ( set.usr[i].avail < g_setup.usr[i].avail )
					{
						use_system_account_flag =2;
					}
					else if ( set.usr[i].avail > g_setup.usr[i].avail )
					{
						use_system_account_flag =1;
					}
					else
						use_system_account_flag =3;
				}

			}
out:	
			
#ifdef USE_NEW_ONVIF
#endif


			
			if(set.tmsync.usage != g_setup.tmsync.usage) {
				change_timesync = 1;
				printf("w4000 m %s, %d\n",__FUNCTION__,__LINE__);
				
				send_msg (TIMER, NETSVR, _CAMMSG_TMSYNC_NTP, 0, 0, 0);
			}
			
			if (set.update_tm) {
				
				
				struct tm tm;
				char tm_str[128];
				int datefmt = g_setup.datefmt;
				int timefmt = g_setup.timefmt; 

				memcpy(&tm, &IPCAM_THREAD_TIMER->current, sizeof(struct tm));
				TMToStringDelimiter(datefmt, timefmt, tm_str, &tm, " ");
				//printf("w4000 m %s, %d\n",__FUNCTION__,__LINE__);

				handle_msg_setsystm_direct(&set.sv_tm, 0, (set.update_tm > 1)?1:0);
				printf("w4000 m %s, %d\n",__FUNCTION__,__LINE__);

			}
			else if (set.tmsync.usage != TMS_NONE) {
				
				send_msg(NETSVR, TIMER, _CAMMSG_TM_RESTSRT, 1, 0, 0);
				printf("w4000 m %s, %d\n",__FUNCTION__,__LINE__);
			}

#ifdef USE_NEW_ONVIF
			
			if(update_type2 == 0) {
				// onvifserver_set_setup(&g_setup, &g_setup_ext);
				if(set.update_tm) {
					onvifserver_update_datetime();
				}
				if(change_timesync) {
					onvifserver_update_ntp();
					printf("w4000 m %s, %d\n",__FUNCTION__,__LINE__);
					send_msg (TIMER, NETSVR, _CAMMSG_TMSYNC_NTP, 0, 0, 0);
				}
			}
#endif
			if (set.update_tm)
			{
				set.update_tm = 0;
			}

			if(memcmp(&set_ext.rtsp_info, &g_setup_ext.rtsp_info, sizeof(ST_RTSP_INFO)) != 0) {
				bIsRtsp = 1;
			}

			
			if(set_ext.rtsp_info.rtsp_port != set_ext.rtsp_info.rtsp_port) {
				restart_server = 1;
			}

			if(g_setup.net.port_web != set.net.port_web) {
				restart_server = 1;
				bIsHttp = 1;
			}


			
			if (memcmp(&set_ext.rtsp_info, &g_setup_ext.rtsp_info, sizeof(ST_RTSP_INFO)) != 0)
			{
				g_setup = g_tmpset = set;
				g_setup_ext = g_tmpset_ext = set_ext;
				setup_save (& DEV_MTD, & g_setup, & g_setup_ext);

				
				send_msg (MAIN, NETSVR, _CAMMSG_RTSP_RESTART, 0, 0, 0);
				sleep(2);//w4000_sleep(2);
				goto set_out;
			}

			if((set_ext.cgi_update_notify[UPDATE_CGI_NET] == 1 || update_type3 == 2 )) {


				if (setup_network_apply(& set, & set_ext, 0, 1, 0) == 0)
				{
					

					set_ext.cgi_update_notify[UPDATE_CGI_NET] == 0;

					g_setup = g_tmpset = set;
					g_setup_ext = g_tmpset_ext = set_ext;
					setup_save (& DEV_MTD, & g_setup, & g_setup_ext);

					if(restart_server) {
						
						send_msg (MAIN, NETSVR, _CAMMSG_STOP, 0, 0, 0);
						send_msg (MAIN, NETSVR, _CAMMSG_START, 0, 0, 0);
					}

					// if(update_type2 == 0) 
					{
						if(bIsRtsp || bIsHttp) {

									
							onvifserver_set_setup(&g_setup, &g_setup_ext);
							onvifserver_update_network_protocol();
							onvifserver_update_video_encoder();
							onvifserver_update_user();
						}
					}
				}
				



				goto set_out;

			}
			
			// if (update)
			{
				
				
				g_setup = g_tmpset = set;
				g_setup_ext = g_tmpset_ext = set_ext;


				setup_save (& DEV_MTD, & g_setup, & g_setup_ext);
			}



		}
		break;

	case UPDATE_SYSTEM_UPGRADE:
		send_msg (MAIN, MAIN, _CAMMSG_EXIT, 0, 0, 0);
		break;

	case UPDATE_SYSTEM_REBOOT:
		send_msg (MAIN, MAIN, _CAMMSG_REBOOT, 5, 0, 0);
		sleep_ex (100000);
		break;


	case UPDATE_SYSTEM_DEFAULT:
#ifdef USE_DF2_SUPPORT
#endif
		setup_load_default (& g_tmpset, & g_tmpset_ext, DEV_QUAD.vsig, set_ext.factory_default_keep_ip);
		setup_save (& DEV_MTD, & g_tmpset, & g_tmpset_ext);
		g_setup = g_tmpset;
		g_setup_ext = g_tmpset_ext;

		send_msg(MAIN, ENCODER, _CAMMSG_EXIT, 0, 0, 0);
		sleep(1);//w4000_sleep(1);
		send_msg (MAIN, MAIN, _CAMMSG_REBOOT, 5, 0, 0);
		break;


	case UPDATE_NETWORK_LINK_STATUS:
		{
			if(is_net_link()) {
				send_msg (MAIN, NETSVR, _CAMMSG_STOP, 0, 0, 0);
				send_msg (MAIN, NETSVR, _CAMMSG_START, 1, 0, 0);
			}
		}
		break;




	default:
		break;
	}

set_out:
	setup_unlock();
	return 0;
}


static void __systemreboot(int rebootparam, int bForce)
{
	int fd;
	char str[4];

	remove_file(ACTION_INFO, 0);

	if((fd = open(ACTION_INFO, O_RDWR |O_CREAT, 0644)) == -1 )
	{
		perror ("open");
	}

	sprintf(str, "%03d", rebootparam);
	write(fd, str, sizeof(str));

	close (fd);

	if (rebootparam == ACTIONINFO_REBOOT || bForce)
		{
		printf( "reboot_system 2..\n");
		hw_reset();
		}
}

void w4000_reboot(void)
{
	printf( "reboot_system 5..\n");
__systemreboot(ACTIONINFO_REBOOT, 0);

}
static char __working_path[4096];

void __set_working_path(const char *working_path)
{
	if(working_path && access(working_path, F_OK) == 0) {
		strcpy(__working_path, working_path);
	}
}

static uint64_t hexString2Number(char *hex_string)
{
	char *p = hex_string;
	size_t len = 0;
	while (*p) { len++; p++; }
	p = hex_string;
	uint64_t num = 0ULL, nibble;

	//remove the prefix
	if (len >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
		p += 2;
	}

	for (; *p && is_hexdigit(*p); p++) {
		nibble = (uint64_t)to_num(*p);
		num = (num << 4) | nibble;
	}

	return num;
}

static int32_t addr2line(char* program_name, int32_t name_length, void const *const addr)
{
	static char addr2line_cmd[512] = {0};
	static char real_path[512];
	/* have addr2line map the address to the relent line in the code */
	if(program_name && name_length > 2 && program_name[0] == '.' && program_name[1] == '/' && __working_path[0] != 0) {
		sprintf(real_path, "%s/%s", __working_path, program_name+2);
		program_name = real_path;
	}

	if(access("/home/bin/addr2line", F_OK) == 0) {
		sprintf(addr2line_cmd, "/home/bin/addr2line -f -p -e %.256s %p", program_name, addr);
	} 
	// else if(access("/data/addr2line", F_OK) == 0) {
	// 	sprintf(addr2line_cmd, "/data/addr2line -f -p -e %.256s %p", program_name, addr);
	// } 
	else {
		fprintf(stdout, "ksi_test not found addr2line .....\n");
		return -1;
	}
	

	/* This will print a nicely formatted string specifying the
	 function and source line of the address */
	//fprintf(stderr, "%s\n", addr2line_cmd);
	// proxy_system(addr2line_cmd, 1, 3);
	return system(addr2line_cmd);
}


#if 0
#include <stdio.h>
#include <curl/curl.h>

// 데이터를 저장할 버퍼
size_t write_callback(void *buffer, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    fwrite(buffer, size, nmemb, stdout); // 표준 출력으로 바로 출력
    return total_size;
}

int main() 
{
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) 
	{
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.daum.net");
        // 받은 데이터를 표준 출력에 바로 쓰기
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() 실패: %s\n", curl_easy_strerror(res));

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}

#endif


int main(int argc, char *argv[])
{
	int ret = 1;
	int reboot = 0;
	int rebootparam = ACTIONINFO_REBOOT;
	int upgr_mode = 0;
	struct st_message msg;
	struct sysinfo sinfo;

	model_spec_reload();
	stop_dhcp_client();

	datetime_hctosys();
	sysinfo(&sinfo);

#ifdef USE_DF2_SUPPORT
	if(oemtool_init(oem_xml) < 0) {
		printf("\n");
	}
#endif
	printf("+++++++++++++++++++++++++++++++ Start ecam IPC-327 +++++++++++++++++++++++++++++++\n");
	
	ipc_server_init("ipc_main");
	
	ipc_status_init();
	
	init_frame_buf();

	

	char cwd[PATH_MAX];
	__set_working_path(getcwd(cwd, sizeof(cwd)));
	
	if (main_init() < 0)
	{
		reboot = 1;
		goto out;
	}
	upgr_mode = is_upgr_mode();
	if(upgr_mode == 0) {
		stop_webserver();
		main_restore_setting(0, upgr_mode);
		
	}
	

	system("rm /mnt/ipm/log.txt");
	
	
	
	if(upgr_mode == 0) {
		gpio_init();
		watchdog_work_init();
	}
	
	int upgr_type = 0;
	if (upgr_mode == 1 && upgr_function(&upgr_type) == 0)
	{
		__systemreboot(upgr_type, 1);
		return ret;
	}

	
	int skip_timer_thread = (upgr_type != 0);
	if (start_threads(skip_timer_thread) < 0) {
		printf("Failed starting threads\n");
		reboot = 1; 
		goto out;
	}

	if(g_setup.net.speed == 1) {
		set_net_speed(g_setup.net.speed);
	}


	isp_init();
	push_md_config(g_setup_ext.md_push.use, g_setup_ext.md_push.server_url, g_setup_ext.md_push.dwell_msec, g_setup_ext.md_push.interval_msec);
	send_msg(MAIN, MAIN, _CAMMSG_START, 0, 0, 0);



	// system("rmmod na51055_wdt");
	// printf("!!   ini test > %s \n", model_spec_string("TEST"));

	// medel_spec_value_add("TEST", "TTEST");
	// printf("!!   ini test > end \n");	

	// ready
	
	sleep(2); 
	while (!g_main.bQuit)
	{
		if (get_msg(MSGQ (MAIN), & msg) > 0) {
			g_main.thread_work_check[THREAD_ALIVE_CHECK_MAIN] = msg.msgid;

			if (msg.msgid)

			switch (msg.msgid) {

			case _CAMMSG_START:
				g_main.th.state = STAT_RUNNING;
				break;

			case _CAMMSG_STOP:
				g_main.th.state = STAT_READY;
				break;

			case _CAMMSG_EXIT:
				g_main.bQuit = TRUE;
				break;

			case _CAMMSG_MOTION:
				_msg_evnt_motion (& msg);
				continue;

			case _CAMMSG_CGI:
				_msg_cgiimport (& msg);
				break;

			case _CAMMSG_DHCP:
				_msg_dhcp_check (& msg);
				break;


			case _CAMMSG_REBOOT:
				if(g_main.isUpgrade && g_main.isNetUpgrade) {
					break;
				}
				g_main.bQuit = TRUE;
				rebootparam = msg.parm1;
				reboot = 1;
				break;

			default :
				break;
			}
		}



		g_main.thread_alive_check[THREAD_ALIVE_CHECK_MAIN] = 0;

	//w4000	sleep_ms(10);
	}
out:


	ipc_server_fini();
	setup_save (& DEV_MTD, & g_setup, & g_setup_ext);

	terminate_threads ();
	main_release();

	{
		isp_exit();
	}

#ifdef USE_DF2_SUPPORT
	oemtool_deinit();
#endif	/* USE_DF2_SUPPORT */

	if (reboot) {
		__systemreboot(rebootparam, 0);
	}

	return ret;
}


