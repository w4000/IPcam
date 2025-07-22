
#include "common2.h"
#include "jQuery_common.h"
#include "ajax_common.h"
#include "setup_ini.h"
#include "ini.h"
#include <stdlib.h>

typedef struct _INI_PRIV_S {
	ini_t *ini;
	char file[128];
} INI_PRIV_S;

#define TEMP_UPNPC_EXT_IP	"/tmp/upnpc_extip"
#define TEMP_UPNPC_RESULT	"/tmp/upnpc_result"

static _CAMSETUP sv_setup;
static _CAMSETUP_EXT sv_setup_ext;


//char debug[1024];


static void notify_upgrade(int type)
{
#if 0
#else
	ipc_client_send("ipc_main", IPC_ID_UPDATE_SETUP, sizeof(int), (void *)&type);
#endif
	return;
}

static void __write_actioninfo(int type)
{
	int fd;
	char str[4];

	unlink(ACTION_INFO);
	fd = open(ACTION_INFO, O_RDWR |O_CREAT, 0644);

	if(fd >= 0) {
		sprintf(str, "%d", type);
		write(fd, str, sizeof(str));

		close (fd);

	}
}

#define OK_DOMAIN_EMPTY							(10)
#define OK_DOMAIN_UPDATED						(11)
#define OK_DOMAIN_EXIST_SAME_USER				(20)
#define OK_DOMAIN_DELETED						(30)

#define FAIL_DOMAIN_ADD_TO_DNS					(-11)
#define FAIL_DOMAIN_ADD_TO_DB					(-12)
#define FAIL_DOMAIN_ADD_TO_DB_EX				(-13)
#define FAIL_REQUIRED_FIELD_MISSING				(-14)
#define FAIL_DOMAIN_UPDATE_TO_DB				(-21)
#define FAIL_DOMAIN_UPDATE_TO_DB_EX				(-22)

#define FAIL_MAC_EXIST_BUT_DIFFRENT_USER		(-31)
#define FAIL_DOMAIN_EXIST_BUT_DIFFRENT_USER		(-41)
#define FAIL_DOMAIN_EXIST_BUT_DIFFRENT_MAC		(-51)
#define FAIL_REMOVE_DOMAIN_DIFFRENT_USER		(-61)

#define FAIL_NO_KEY								(-101)
#define FAIL_INVALID_KEY						(-102)
#define FAIL_NOT_MATCHED_KEY					(-103)

#define FAIL_DNS_ERR							(-901)
#define FAIL_NO_ROUTE_TO_HOST					(-903)

const char* errfmt =
			"<table border='0' cellspacing='0' cellpadding='2px' width='100%%'>"
				"<tr>"
					"<td bgcolor='#FEF8F6' align='center' valign='center'>"
						"<div class='ui-widget'>"
							"<div class='ui-state-%s ui-corner-all' style='padding: 0 .7em;'>"	//highlight
								"<p>%s%s%s</p>"
							"</div>"
						"</div>"
					"</td>"
				"</tr>"
			"</table>";

char notice[1024];
char* ui[] = { "highlight", "error" };

int decrypt_by_private(char* pdata, unsigned char* enc_data, int enc_data_size, char* dec_data, int dec_data_size, char* priv_key)
{
	int enc_data_len = ___base64_decode(enc_data, pdata, enc_data_size);
	if (enc_data_len < 0) {
		return -1;
	}

	memset(dec_data, 0, dec_data_size);
	private_decrypt(enc_data, enc_data_len, (unsigned char *)priv_key, (unsigned char *)dec_data);
	return 0;
}

int get_client_info(void)
{

	return 0;
}




int main(void)
{
	cgi_init();


	cgi_process_form();
	cgi_init_headers();


	int i, ret = -1;
	char *pdata;

	char szAdditionalMsg1[256] = "", szAdditionalMsg2[256] = "";

	load_setup(&sv_setup, &sv_setup_ext);

	int lang = SETUP_LANG(&sv_setup);

// decrypt
	char category[1024];
	memset(category, 0, sizeof(category));

	unsigned char priv_key[2048];
	memset(priv_key, 0, sizeof(priv_key));
	{
		FILE* fp_key = fopen("/tmp/_key/priv.key.pem", "rt");
		if (fp_key)
		{
			fread(priv_key, 1, sizeof(priv_key), fp_key);
			fclose(fp_key);
		}
		else
		{
			ret = -1;
			goto fn_out;
		}

		pdata = (char*)cgi_param("category");

		if (pdata == NULL) {
			ret = -1;
			goto fn_out;
		}

		char *random_key = (char*)cgi_param("rnd_key");
		char *encrypt_key = (char*)cgi_param("enc_key");
		char *session_key = (char*)cgi_param("ses_key");
		char *_session_key = (char*)cgi_param("rsa_session");

		if (!random_key || !encrypt_key || !session_key || !_session_key) {
			ret = -1;
			goto enc_out;
		}

		unsigned char enc_key[2048];
		int enc_key_len = ___base64_decode(enc_key, encrypt_key, sizeof(enc_key));

		char dec_key[1024];
		memset(dec_key, 0, sizeof(dec_key));

		private_decrypt(enc_key, enc_key_len, priv_key, (unsigned char *)dec_key);

		if (strcmp(dec_key, random_key) != 0) {
			ret = -1;
			goto fn_out;
		}

		memset(dec_key, 0, sizeof(dec_key));
		enc_key_len = ___base64_decode(enc_key, session_key, sizeof(enc_key));
		private_decrypt(enc_key, enc_key_len, priv_key, (unsigned char *)dec_key);

		if (strcmp(dec_key, _session_key) != 0) {
			ret = -1;
			goto fn_out;
		}

		unsigned char enc_category[1024];
		enc_key_len = ___base64_decode(enc_category, pdata, sizeof(enc_category));

		private_decrypt(enc_category, enc_key_len, priv_key, (unsigned char *)category);

		pdata = category;
	}

enc_out:
	get_client_info();

	if (pdata) {
		if (strcmp(pdata, "lanGuage") == 0) {
			pdata = (char*)cgi_param("lang");
			if (pdata != NULL)
			{
				SETUP_LANG(&sv_setup) = atoi(pdata);
			}

			update_setup(&sv_setup, &sv_setup_ext);

			notify_upgrade(1);
			ret = 0;
		}
		else
		if (strcmp(pdata, "vStream") == 0) {
			// video
			char var[16];
			PIC_SIZE_E ps[3] = {0};
			int fps[3] = {0};
			for (i = 0; i < CAM_MAX; i++) {
				fps[i] = sv_setup.cam[i].fps;
				sprintf(var, "v_fps_%d", i);
				pdata = (char*)cgi_param(var);
				if(pdata != NULL)
					fps[i] = atoi(pdata);

				ps[i] = sv_setup.cam[i].res;
				sprintf(var, "v_resol_%d", i);
				pdata = (char*)cgi_param(var);
				if(pdata != NULL)
					ps[i] = atoi(pdata);

				if(fps[i] > 30)
					fps[i] = 30;
			}


			for (i = 0; i < CAM_MAX; i++) {

				sprintf(var, "v_codec_%d", i);
				pdata = (char*)cgi_param(var);
				if (pdata != NULL)
					sv_setup.cam[i].codec = atoi(pdata);

				sprintf(var, "v_ratectl_%d", i);
				pdata = (char*)cgi_param(var);
				if (pdata != NULL)
					sv_setup.cam [i].ratectrl = atoi(pdata);

				sprintf(var, "v_resol_%d", i);
				pdata = (char*)cgi_param(var);
				if (pdata != NULL)
					sv_setup.cam[i].res = atoi(pdata);

				sprintf(var, "v_fps_%d", i);
				pdata = (char*)cgi_param(var);
				if (pdata != NULL)
					sv_setup.cam[i].fps = atoi(pdata);


				sprintf(var, "v_max_bitrate_%d", i);
				pdata = (char*)cgi_param(var);
				if (pdata != NULL)
					sv_setup.cam [i].bitrate = atoi(pdata);

				sprintf(var, "v_gop_%d", i);
				pdata = (char*)cgi_param(var);
				if (pdata != NULL)
					sv_setup.cam [i].gop = atoi(pdata);
			}

			pdata = (char*)cgi_param("cam_title");
			if (pdata != NULL) 
			{
				//printf("======================== %s \n",pdata);//w4000
				strncpy(sv_setup.cam [0].title, pdata, 64);
			}
			else
				{
				//printf("======================== %s \n",pdata);
				strncpy(sv_setup.cam [0].title, "", 64);
			}

#if defined(NARCH)
			pdata = (char*)cgi_param("disp_osd");
			sv_setup.cam [0].title_osd.usage = (pdata != NULL)? atoi(pdata) : 0;

			pdata = (char*)cgi_param("osd_color");
			sv_setup.cam [0].title_osd.color = (pdata != NULL)? atoi(pdata) : 0;

			pdata = (char*)cgi_param("osd_pos");
			sv_setup.cam [0].title_osd.position = (pdata != NULL)? atoi(pdata) : 0;
#endif



#if defined(NARCH)
			int max_fps = 30;
#else
#endif	/* NARCH */
			for (i=0; i<CAM_MAX; i++) {
				if (sv_setup.cam[i].fps > max_fps)
					sv_setup.cam[i].fps = max_fps;
			}

			
			sv_setup_ext.cgi_update_notify[UPDATE_CGI_VIDEO] = 1;
			update_setup(&sv_setup, &sv_setup_ext);
			notify_upgrade(1);


			ret = 0;


		}
		else if (strcmp(pdata, "netInterFace") == 0) {
			pdata = (char*)cgi_param("nettype");
			sv_setup.net.type = atoi(pdata);

			pdata = (char*)cgi_param("speed");
			if (pdata)
				sv_setup.net.speed = atoi(pdata);

			if (sv_setup.net.type == NETTYPE_LAN) {
				pdata = (char*)cgi_param("ipaddr");
				if (pdata)
					inet_aton (pdata, &sv_setup.net.ip);

				pdata = (char*)cgi_param("netmask");
				if (pdata)
					inet_aton (pdata, &sv_setup.net.netmask);

				pdata = (char*)cgi_param("gateway");
				if (pdata)
					inet_aton (pdata, &sv_setup.net.gateway);


			} else {
			}

			sv_setup_ext.cgi_update_notify[UPDATE_CGI_NET] = 1;
			update_setup(&sv_setup, &sv_setup_ext);
			notify_upgrade(1);

			ret = 0;
		}
		else if (strcmp(pdata, "netPort") == 0) {


			pdata = (char*)cgi_param("net_prot");
			if (pdata) {
				sv_setup.net.port_tcp = (pdata)? atoi(pdata) : 8001;
			}


			pdata = (char*)cgi_param("web_port");
			if (pdata) {
				sv_setup.net.port_web = (pdata)? atoi(pdata) : 80;
			}


			// pdata = (char*)cgi_param("rtsp_port");
			// if (pdata) {
			// 	sv_setup_ext.rtsp_info.rtsp_port = (pdata)? atoi(pdata) : 554;
			// 	if (sv_setup_ext.rtsp_info.rtsp_port == 0)
			// 		sv_setup_ext.rtsp_info.rtsp_port = 554;
			// }



			//sprintf (debug, "echo \"func:%s, line: %d > sv_setup.net.port_web? %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__, sv_setup.net.port_web); 
			//system (debug);

			sv_setup_ext.cgi_update_notify[UPDATE_CGI_NET] = 1;
			update_setup(&sv_setup, &sv_setup_ext);
			notify_upgrade(1);

			ret = 0;
		}
		else if (strcmp(pdata, "netProto") == 0) {

			pdata = (char*)cgi_param("use_rtsp");
			sv_setup_ext.rtsp_info.use_rtsp = (pdata)? atoi(pdata) : 0;

			pdata = (char*)cgi_param("rtsp_security");
			sv_setup_ext.rtsp_info.use_security = (pdata)? atoi(pdata) : 0;

			pdata = (char*)cgi_param("is_rtsp_security");
			sv_setup_ext.rtsp_info.use_rtsp_security_select_set = (pdata) ? atoi(pdata) : 0;


			pdata = (char*)cgi_param("rtsp_port");
			int rtsp_port = (pdata)? atoi(pdata) : 554;
			if (rtsp_port == 0)
				rtsp_port = 554;



			
			pdata = (char*)cgi_param("rtsp_addr1");
			if(pdata) 
			{
				char *p;
				if ((p = strstr(pdata, "rtsp://"))) 
				{
					p += strlen ("rtsp://");
					if ((p = strchr(p, '/'))) 
					{
						p++; 
						snprintf(sv_setup_ext.rtsp_uri[0], 32, "%s", p);
					}
				}
			}
			else 
			{
				snprintf(sv_setup_ext.rtsp_uri[0], 32, "%s%d", (char*)model_spec_string("RTSP_URL"), 1);
			}

			
			pdata = (char*)cgi_param("rtsp_addr2");
			if(pdata) 
			{
				char *p;
				if ((p = strstr(pdata, "rtsp://"))) 
				{
					p += strlen ("rtsp://");
					if ((p = strchr(p, '/'))) 
					{
						p++; 
						snprintf(sv_setup_ext.rtsp_uri[1], 32, "%s", p);
					}
				}
			}
			else 
			{
				snprintf(sv_setup_ext.rtsp_uri[1], 32, "%s%d", (char*)model_spec_string("RTSP_URL"), 2);
			}


			
			pdata = (char*)cgi_param("rtsp_addr3");
			if(pdata) 
			{
				char *p;
				if ((p = strstr(pdata, "rtsp://"))) 
				{
					p += strlen ("rtsp://");
					if ((p = strchr(p, '/'))) 
					{
						p++; 
						snprintf(sv_setup_ext.rtsp_uri[2], 32, "%s", p);
					}
				}
			}
			else 
			{
				snprintf(sv_setup_ext.rtsp_uri[2], 32, "%s%d", (char*)model_spec_string("RTSP_URL"), 3);
			}



			if (rtsp_port != sv_setup.net.port_tcp && rtsp_port != sv_setup.net.port_web
					&& rtsp_port != sv_setup.net.port_web)
				sv_setup_ext.rtsp_info.rtsp_port = rtsp_port;

			pdata = (char*)cgi_param("onvif_security");
			sv_setup_ext.onvif_security = (pdata)? atoi(pdata) : 0;
#ifdef USE_NEW_ONVIF
			pdata = (char*)cgi_param("onvif_use_https");
			sv_setup_ext.onvif_use_https = (pdata)? atoi(pdata) : 0;

			pdata = (char*)cgi_param("onvif_port");
			if(pdata) {
				int onvif_port = atoi(pdata);
				if (onvif_port != sv_setup.net.port_tcp && onvif_port != sv_setup.net.port_web
						&& onvif_port != sv_setup_ext.rtsp_info.rtsp_port)
					sv_setup_ext.onvif_port = onvif_port;
			}
#endif

			sv_setup_ext.cgi_update_notify[UPDATE_CGI_NET] = 1;
			update_setup(&sv_setup, &sv_setup_ext);
			notify_upgrade(1);

			ret = 0;
		}
		else if (strcmp(pdata, "system_info") == 0) {

		}
		else if (strcmp(pdata, "timeNdate") == 0) {
			int bUpdate_tm = FALSE;
			pdata = (char*)cgi_param("use_settime");
			bUpdate_tm = (pdata != NULL)? atoi(pdata) : 0;

			sv_setup.update_tm = bUpdate_tm;
			if (bUpdate_tm) {
				pdata = (char*)cgi_param("settime_year");
				sv_setup.sv_tm.tm_year = ((pdata) ? atoi(pdata) : 2013) - 1900;

				pdata = (char*)cgi_param("settime_month");
				sv_setup.sv_tm.tm_mon = ((pdata) ? atoi(pdata) : 1) - 1;

				pdata = (char*)cgi_param("settime_day");
				sv_setup.sv_tm.tm_mday = atoi(pdata);

				pdata = (char*)cgi_param("settime_hour");
				sv_setup.sv_tm.tm_hour = atoi(pdata);

				pdata = (char*)cgi_param("settime_min");
				sv_setup.sv_tm.tm_min = atoi(pdata);

				pdata = (char*)cgi_param("settime_sec");
				sv_setup.sv_tm.tm_sec = atoi(pdata);

				pdata = (char*)cgi_param("settime_timezone");
				sv_setup.tmzone = (pdata != NULL)? atoi(pdata) : 0;

			}

			pdata = (char*)cgi_param("date_format");
			sv_setup.datefmt = (pdata != NULL)? atoi(pdata) : 0;

			pdata = (char*)cgi_param("time_format");
			sv_setup.timefmt = (pdata != NULL)? atoi(pdata) : 0;

			pdata = (char*)cgi_param("timesync_mode");
			sv_setup.tmsync.usage = (pdata != NULL) ? atoi(pdata) : 0;
			//printf("w4000 m %s, %d\n",__FUNCTION__,__LINE__);

			pdata = (char*)cgi_param("use_ntp");
			if (sv_setup.tmsync.usage > 0 && atoi(pdata) > 0) {
				if (sv_setup.tmsync.usage == TMS_NTPSVR) {		// NTP Server
#if 1			
					pdata = (char*)cgi_param("timesync_ntp_cycle");
					sv_setup.tmsync.syncycle = (pdata != NULL)? atoi(pdata) : 1;
#endif 
					pdata = (char*)cgi_param("timesync_ntp_server");
					strncpy(sv_setup.tmsync.ntpsvr, pdata? pdata : "", 32);
				}
			}
			else 
			{
				sv_setup.tmsync.usage = 0;
			}

			sv_setup_ext.cgi_update_notify[UPDATE_CGI_TIME] = 1;
			update_setup(&sv_setup, &sv_setup_ext);
			notify_upgrade(1);

			ret = 0;
		}
		else if (strcmp(pdata, "userInfo") == 0) {
			// const int DATA_SIZE	= 1024;
			memset(&sv_setup.usr, 0x0, sizeof(sv_setup.usr));
			char var[32];
			for (i = 0; i < 10; i++) {
				sprintf(var, "u_id_%d", i);
				pdata = (char*)cgi_param(var);

				if (pdata == NULL || strlen(pdata) == 0) {
					SETUP_USR_AVAIL(&sv_setup, i) = 0;
				}
				else {
					SETUP_USR_AVAIL(&sv_setup, i) = 1;

					strncpy(SETUP_USR_NAME(&sv_setup, i), pdata, USRNAME_MAX);

					sprintf(var, "u_desc_%d", i);
					pdata = (char*)cgi_param(var);
					strncpy(SETUP_USR_DESC(&sv_setup, i), pdata? pdata : "", USRDESC_MAX);

					sprintf(var, "u_pwd_%d", i);
					pdata = (char*)cgi_param(var);
					strncpy(SETUP_USR_PWD(&sv_setup, i), (pdata != NULL)? pdata : "", USRPWD_MAX);

				}
			}


			sv_setup_ext.cgi_update_notify[UPDATE_CGI_USER] = 1;
			update_setup(&sv_setup, &sv_setup_ext);
			notify_upgrade(1);

			ret = 0;
		}
		else if (strcmp(pdata, "dFault") == 0) {

			pdata = (char*)cgi_param("keep_ip");
			sv_setup_ext.factory_default_keep_ip = (pdata)? atoi(pdata) : 0;

			update_setup(&sv_setup, &sv_setup_ext);
			notify_upgrade(UPDATE_SYSTEM_DEFAULT);
			cgi_end();
			return 0;
		}
		else if (strcmp(pdata, "reBoot") == 0) {
			notify_upgrade(UPDATE_SYSTEM_REBOOT);
			cgi_end();
			return 0;
		}
		else if (strcmp(pdata, "system_upgrade") == 0) {
			// char cmd[128];
			__write_actioninfo(ACTIONINFO_UPGRADE_DOWNLOAD_DONE);
			unlink (FAIL_INFO);
			ret = 0;

			cgi_end();
		}
	}

fn_out:
	{
		char* szRES_MSG_UPDATE_OK[] = RES_ID_ARRAY(RES_MSG_UPDATE_OK);
		char* szRES_MSG_UPDATE_FAIL[] = RES_ID_ARRAY(RES_MSG_UPDATE_FAIL);

		sprintf(notice, errfmt, (ret >= 0) ? ui[0] : ui[1], (ret >= 0) ? szRES_MSG_UPDATE_OK[lang] : szRES_MSG_UPDATE_FAIL[lang], szAdditionalMsg1, szAdditionalMsg2);

	}

retry:
	if (ajax_check_output(notice, lang) < 0) {
		if (lang != 0) {
			lang = 0;
			goto retry;
		}
	}

	cgi_end();


	return 0;

}

