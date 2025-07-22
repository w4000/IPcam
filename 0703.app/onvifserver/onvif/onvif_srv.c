

#include "sys_inc.h"
#include "hxml.h"
#include "xml_node.h"
#include "onvif_probe.h"
#include "http.h"
#include "http_srv.h"
#include "http_parse.h"
#include "onvif_device.h"
#include "onvif.h"
#include "onvif_timer.h"
#include "onvif_srv.h"
#include "onvif_event.h"
#include "soap.h"
#ifdef PROFILE_G_SUPPORT
#include "onvif_recording.h"
#endif
#ifdef HTTPD
#include "httpd.h"
#endif

/***************************************************************************************/
static HTTPSRV http_srv[MAX_SERVERS];
extern ONVIF_CLS g_onvif_cls;
extern ONVIF_CFG g_onvif_cfg;

/***************************************************************************************/

BOOL onvif_http_msg_cb(void * p_srv, HTTPCLN * p_cln, HTTPMSG * p_msg, void * p_userdata)
{
    if (p_msg)
    {
        OIMSG msg;
		memset(&msg, 0, sizeof(OIMSG));

		msg.msg_src = ONVIF_MSG_SRC;
		msg.msg_dua = (char *)p_cln;
		msg.msg_buf = (char *)p_msg;

		if (hqBufPut(g_onvif_cls.msg_queue, (char *)&msg) == FALSE)
		{
			return  FALSE;
		}
    }
    else if (p_cln)
    {
        OIMSG msg;
		memset(&msg, 0, sizeof(OIMSG));

		msg.msg_src = ONVIF_DEL_UA_SRC;
		msg.msg_dua = (char *)p_cln;

		if (hqBufPut(g_onvif_cls.msg_queue, (char *)&msg) == FALSE)
		{
			return  FALSE;
		}
    }

    return TRUE;
}
#if 0
void analyze_ppsn_ctx(const char *title, PPSN_CTX ctx)//w4000
{
    if (!ctx) {
        printf("[%s] PPSN_CTX is NULL\n", title);
        return;
    }

    printf("====== %s (PPSN_CTX) 분석 시작 ======\n", title);
    printf("fl_base     : %p\n", ctx->fl_base);
    printf("head_node   : %u\n", ctx->head_node);
    printf("tail_node   : %u\n", ctx->tail_node);
    printf("node_num    : %u\n", ctx->node_num);
    printf("low_offset  : %u\n", ctx->low_offset);
    printf("high_offset : %u\n", ctx->high_offset);
    printf("unit_size   : %u\n", ctx->unit_size);
    printf("ctx_mutex   : %p\n", ctx->ctx_mutex);
    printf("push_cnt    : %u\n", ctx->push_cnt);
    printf("pop_cnt     : %u\n", ctx->pop_cnt);
    printf("====== %s 분석 종료 ======\n", title);
}
void analyze_httpmsg_content(HTTPMSG *p_msg)//w4000
{
    printf("====== HTTPMSG 분석 시작 ======\n");

    printf("msg_type      : %u\n", p_msg->msg_type);
    printf("msg_sub_type  : %u\n", p_msg->msg_sub_type);

    if (p_msg->first_line.value_string)
        printf("First line    : %s\n", p_msg->first_line.value_string);
    else
        printf("First line    : (null)\n");

    printf("Header length : %d\n", p_msg->hdr_len);
    printf("Content length: %d\n", p_msg->ctt_len);
    printf("Content type  : %d\n", p_msg->ctt_type);

    if (p_msg->boundary[0])
        printf("Boundary      : %s\n", p_msg->boundary);
    else
        printf("Boundary      : (none)\n");

    printf("Remote IP     : %u.%u.%u.%u\n",
           (p_msg->remote_ip >> 24) & 0xFF,
           (p_msg->remote_ip >> 16) & 0xFF,
           (p_msg->remote_ip >> 8) & 0xFF,
           (p_msg->remote_ip) & 0xFF);

    printf("Remote Port   : %u\n", p_msg->remote_port);
    printf("Buffer Offset : %d\n", p_msg->buf_offset);

    if (p_msg->msg_buf && p_msg->buf_offset > 0)
    {
        printf("---- HTTP Raw Message (Partial) ----\n");
        fwrite(p_msg->msg_buf, 1, (p_msg->buf_offset < 1024) ? p_msg->buf_offset : 1024, stdout);
        printf("\n---- END ----\n");
    }
    else
    {
        printf("No message buffer or empty.\n");
    }
    // PPSN_CTX 분석
    analyze_ppsn_ctx("Header Context", &p_msg->hdr_ctx);
    analyze_ppsn_ctx("Content Context", &p_msg->ctt_ctx);

    printf("====== HTTPMSG 분석 종료 ======\n");
}
#endif
void onvif_http_msg_handler(HTTPCLN * p_cln, HTTPMSG * p_msg)
{
    char * post = p_msg->first_line.value_string;


   // analyze_httpmsg_content(p_msg);  // 구조 전체 분석 //w4000

	if (strstr(post, "SystemRestore"))	// must be the same with onvif_StartSystemRestore
	{
		SystemRestore(p_cln, p_msg);
	}
	else if (strstr(post, "SupportInfo"))
	{
	    GetSupportInfo(p_cln, p_msg);
	}
	else if (strstr(post, "SystemBackup"))
	{
	    GetSystemBackup(p_cln, p_msg);
	}
	else if (p_msg->ctt_type == CTT_XML)
	{
	    process_request(p_cln, p_msg);
	}
#ifdef HTTPD
	else
	{
	    http_process_request(p_cln, p_msg);
	}
#endif
}

void * onvif_task(void * argv)
{
    OIMSG stm;

	while (1)
	{
		if (hqBufGet(g_onvif_cls.msg_queue, (char *)&stm))
		{
			HTTPCLN * p_cln = (HTTPCLN *)stm.msg_dua;

			#if 0//w4000 is_auth_w4000
			printf("[ONVIF] Message received. Source: %d\n", stm.msg_src);

			if (stm.msg_buf != NULL)
			{
				HTTPMSG * msg = (HTTPMSG *)stm.msg_buf;
				printf("[ONVIF] HTTPMSG:\n");
				printf("  Type: 	   %d\n", msg->msg_type);
				printf("  Subtype:	   %d\n", msg->msg_sub_type);
				printf("  Remote IP:   %d.%d.%d.%d\n",
					   (msg->remote_ip >> 24) & 0xFF,
					   (msg->remote_ip >> 16) & 0xFF,
					   (msg->remote_ip >> 8) & 0xFF,
					   msg->remote_ip & 0xFF);
				printf("  Remote Port: %d\n", msg->remote_port);
				printf("  Content-Type:%d\n", msg->ctt_type);
				printf("  Header Len:  %d\n", msg->hdr_len);
				printf("  Content Len: %d\n", msg->ctt_len);
				if (msg->msg_buf && msg->buf_offset > 0)
				{
					printf("  Raw Buffer (first 256 bytes):\n%.*s\n",
						   (msg->buf_offset < 256) ? msg->buf_offset : 256,
						   msg->msg_buf);
				}
			}
			#endif

			switch (stm.msg_src)
			{
			case ONVIF_MSG_SRC:
				onvif_http_msg_handler(p_cln, (HTTPMSG *)stm.msg_buf);

				if (stm.msg_buf)
				{
				    http_free_msg((HTTPMSG *)stm.msg_buf);
				}

				if (p_cln)
				{
				    http_free_used_cln((HTTPSRV *)p_cln->p_srv, p_cln);
				}
				break;

			case ONVIF_DEL_UA_SRC:
			    http_free_used_cln((HTTPSRV *)p_cln->p_srv, p_cln);
				break;

			case ONVIF_TIMER_SRC:
				onvif_timer();
				break;

			case ONVIF_EXIT:
			    goto EXIT;
			}
		}
	}

EXIT:

    g_onvif_cls.tid_main = 0;

	return NULL;
}

int onvif_start1(void)
{
    int i, cnt = 0;
	int ret = 0;

    onvif_init();


    g_onvif_cls.msg_queue = hqCreate(100, sizeof(OIMSG), HQ_GET_WAIT);
	if (g_onvif_cls.msg_queue == NULL)
	{
		return -1;
	}

    g_onvif_cls.tid_main = sys_os_create_thread((void *)onvif_task, NULL);

    for(i = 0; i < g_onvif_cfg.servs_num; i++) {
		do{
			if(http_srv_init(&http_srv[i], g_onvif_cfg.servs[i].serv_ip, g_onvif_cfg.servs[i].serv_port,
						g_onvif_cfg.http_max_users, g_onvif_cfg.https_enable, "ssl.ca", "ssl.key") < 0) {
				ret = -1;
			}
			else {
				http_set_msg_callback(&http_srv[i], onvif_http_msg_cb, NULL);
				ret = 0;
				break;
			}
			usleep(300*1000);
		} while(cnt++ < 20);
    }

	onvif_timer_init();

	onvif_start_discovery();

	return ret;
}

void onvif_start(int webPort)
{
	int cnt = 0;
	onvif_init_cfg(NULL, webPort);


	do {
		if(onvif_start1() == 0) {
			break;
		}
		onvif_stop();
		usleep(500*1000);
	} while(cnt++ < 5);
}

void onvif_free_device()
{
    onvif_free_NetworkInterfaces(&g_onvif_cfg.network.interfaces);

    onvif_free_VideoSources(&g_onvif_cfg.v_src);

    onvif_free_VideoSourceConfigurations(&g_onvif_cfg.v_src_cfg);

    onvif_free_VideoEncoder2Configurations(&g_onvif_cfg.v_enc_cfg);


    onvif_free_profiles(&g_onvif_cfg.profiles);

    onvif_free_OSDConfigurations(&g_onvif_cfg.OSDs);

    onvif_free_MetadataConfigurations(&g_onvif_cfg.metadata_cfg);

#ifdef MEDIA2_SUPPORT
    onvif_free_Masks(&g_onvif_cfg.mask);
#endif

#ifdef VIDEO_ANALYTICS
    onvif_free_VideoAnalyticsConfigurations(&g_onvif_cfg.va_cfg);
#endif

#ifdef PROFILE_G_SUPPORT
    onvif_free_Recordings(&g_onvif_cfg.recordings);

    onvif_free_RecordingJobs(&g_onvif_cfg.recording_jobs);
#endif

#ifdef PROFILE_C_SUPPORT
    onvif_free_AccessPoints(&g_onvif_cfg.access_points);
    onvif_free_Doors(&g_onvif_cfg.doors);
    onvif_free_Areas(&g_onvif_cfg.areas);
#endif

#ifdef DEVICEIO_SUPPORT
    onvif_free_VideoOutputs(&g_onvif_cfg.v_output);
    onvif_free_VideoOutputConfigurations(&g_onvif_cfg.v_output_cfg);
    onvif_free_AudioOutputs(&g_onvif_cfg.a_output);
    onvif_free_AudioOutputConfigurations(&g_onvif_cfg.a_output_cfg);
    onvif_free_RelayOutputs(&g_onvif_cfg.relay_output);
    onvif_free_DigitalInputs(&g_onvif_cfg.digit_input);
    onvif_free_SerialPorts(&g_onvif_cfg.serial_port);
#endif

    onvif_free_VideoEncoder2ConfigurationOptions(&g_onvif_cfg.v_enc_cfg_opt);

#ifdef CREDENTIAL_SUPPORT
    onvif_free_Credentials(&g_onvif_cfg.credential);
#endif

#ifdef ACCESS_RULES
    onvif_free_AccessProfiles(&g_onvif_cfg.access_rules);
#endif

#ifdef SCHEDULE_SUPPORT
    onvif_free_Schedules(&g_onvif_cfg.schedule);
    onvif_free_SpecialDayGroups(&g_onvif_cfg.specialdaygroup);
#endif

#ifdef RECEIVER_SUPPORT
    onvif_free_Receivers(&g_onvif_cfg.receiver);
#endif
}

int static stop_cnt =0;
void onvif_stop()
{
    int i;

    OIMSG stm;
    memset(&stm, 0, sizeof(stm));

	stm.msg_src = ONVIF_EXIT;

    hqBufPut(g_onvif_cls.msg_queue, (char *)&stm);

    while (g_onvif_cls.tid_main)
    {
        usleep(10*1000);
    }

    onvif_bye();
	onvif_stop_discovery();

	onvif_timer_deinit();

    for (i = 0; i < g_onvif_cfg.servs_num; i++)
    {
	    http_srv_deinit(&http_srv[i]);
	}

    hqDelete(g_onvif_cls.msg_queue);

    onvif_eua_deinit();

#ifdef PROFILE_G_SUPPORT
    onvif_FreeSearchs();
#endif

    onvif_free_device();



}


