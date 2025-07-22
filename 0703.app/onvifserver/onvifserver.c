#include "sys_inc.h"
#include "onvif.h"
#include "onvif_srv.h"
#include "http_parse.h"
#include "onvif_event.h"
#include "onvifserver.h"
#include "cam_setup.h"
#include "onvif_utils.h"

extern ONVIF_CFG g_onvif_cfg;
extern ONVIF_CLS g_onvif_cls;

_CAMSETUP g_onvif_setup;
_CAMSETUP_EXT g_onvif_setup_ext;

#if 0//w4000
int is_valid_user(const char *username, const char *password)
{
	ST_USR *users = cam_setup()->usr;
	for (int i = 0; i < 10; i++)
	{
		if (!users[i].avail)
			continue;
		
		if (strcmp(users[i].usrname, username) == 0 &&
			strcmp(users[i].usrpwd, password) == 0)
		{
			return 1; // 로그인 성공
		}
	}
	return 0; // 로그인 실패
}
int get_user_level(const char *username)
{
	ST_USR *users = cam_setup()->usr;
	for (int i = 0; i < 10; i++)
	{
		if (!users[i].avail)
			continue;
		if (strcmp(users[i].usrname, username) == 0)
		{
			if (users[i].uperm.setup && users[i].uperm.vod)
				return 2; // 관리자급
			else
				return 1; // 일반 사용자
		}
	}
	return 0; // 사용자 없음
	onvifserver_update_user();//w4000
}

int authorized = mg_http_check_digest_auth(hm, domain, fp);
if (authorized == 1) {
    // 로그인 성공 상태
} else {
    // 로그인 실패 (401 Unauthorized로 응답 필요)
}

	if (!mg_http_is_authorized(hm, path, "MyDeviceRealm", "/etc/onvif_users", MG_AUTH_FLAG_IS_GLOBAL_PASS_FILE))
{
    mg_http_send_digest_auth_request(nc, "MyDeviceRealm");
    return;
}

#endif
void onvifserver_set_setup(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext)

{	
	cam_setup_update(setup, setup_ext);

}

void onvifserver_get_setup(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext)

{	memcpy(setup, cam_setup(), sizeof(_CAMSETUP));
	memcpy(setup_ext, cam_setup_ext(), sizeof(_CAMSETUP_EXT));
}

void onvifserver_get_rtsp_uri(char *uri, int ch)

{	_CAMSETUP_EXT _setup_ext;
	
	memcpy(&_setup_ext, cam_setup_ext(), sizeof(_CAMSETUP_EXT));

	if (ch < 3 && check_cam_setup_update() != 0)
	
{		snprintf(uri, 32, "%s", _setup_ext.rtsp_uri[ch]);
	}
	else 
	
{		snprintf(uri, 32, "%s%d", (char*)model_spec_string("RTSP_URL"), (ch+1));
	}
}


static int g_isinit = 0;
int onvifserver_init(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext)

{	int webPort = 0;
	onvifserver_deinit();
    sys_buf_init(32);
    http_msg_buf_init(16);

	onvifserver_set_setup(setup, setup_ext);
	webPort = setup->net.port_web;

	if(webPort <= 0 || webPort > 65535)
{		webPort = 80;
	}

	onvif_start(webPort);

	// Initialize event status
	g_onvif_cls.last_event_value[ONVIF_EVENT_MOTION] = (bitisset(IPCAM_THREAD_MAIN->mot, 0))?1:0;
	g_onvif_cls.last_event_value[ONVIF_EVENT_MOTION_REGION] = (bitisset(IPCAM_THREAD_MAIN->mot, 0))?1:0;
	g_onvif_cls.last_event_value[ONVIF_EVENT_SENSOR_IN] = 0;
	g_onvif_cls.last_event_value[ONVIF_EVENT_ALARM_OUT] = 0;

	g_isinit = 1;

	return 0;
}

void onvifserver_deinit()

{	if(g_isinit == 0)
{		return;
	}
	onvif_stop();
	http_msg_buf_deinit();
	sys_buf_deinit();
	g_isinit = 0;
}


int onvifserver_make_event(NotificationMessageList *p_message, onvif_PropertyOperation op, ONVIF_EVENT_E event, int value)

{	SimpleItemList * p_simpleitem;

	strcpy(p_message->NotificationMessage.Dialect, "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet");
	if(event == ONVIF_EVENT_MOTION)
{		strcpy(p_message->NotificationMessage.Topic, "tns1:VideoSource/MotionAlarm");
	}
	else if(event == ONVIF_EVENT_MOTION_REGION)
{		strcpy(p_message->NotificationMessage.Topic, "tns1:RuleEngine/MotionRegionDetector/Motion");
	}
	else
{		return -1;
	}
	p_message->NotificationMessage.Message.PropertyOperationFlag = 1;
	p_message->NotificationMessage.Message.PropertyOperation = op;
	p_message->NotificationMessage.Message.UtcTime = time(NULL);

	p_message->NotificationMessage.Message.SourceFlag = 1;

	p_simpleitem = onvif_add_SimpleItem(&p_message->NotificationMessage.Message.Source.SimpleItem);
	if (p_simpleitem)
	
{		if(event == ONVIF_EVENT_MOTION)
{			strcpy(p_simpleitem->SimpleItem.Name, "Source");
			if (g_onvif_cfg.v_src)
				strcpy(p_simpleitem->SimpleItem.Value, g_onvif_cfg.v_src->VideoSource.token);
			else
				strcpy(p_simpleitem->SimpleItem.Value, "V_SRC_000");
		}
		else if(event == ONVIF_EVENT_MOTION_REGION)
{			strcpy(p_simpleitem->SimpleItem.Name, "VideoSource");
			if (g_onvif_cfg.v_src)
				strcpy(p_simpleitem->SimpleItem.Value, g_onvif_cfg.v_src->VideoSource.token);
			else
				strcpy(p_simpleitem->SimpleItem.Value, "V_SRC_000");
		}
	}

	if(event == ONVIF_EVENT_MOTION_REGION)
{		p_simpleitem = onvif_add_SimpleItem(&p_message->NotificationMessage.Message.Source.SimpleItem);
		if (p_simpleitem)
		
{			strcpy(p_simpleitem->SimpleItem.Name, "RuleName");
			strcpy(p_simpleitem->SimpleItem.Value, "MotionRegion");
		}
	}

	p_message->NotificationMessage.Message.DataFlag = 1;

	p_simpleitem = onvif_add_SimpleItem(&p_message->NotificationMessage.Message.Data.SimpleItem);
	if (p_simpleitem)
	
{		if(event == ONVIF_EVENT_MOTION)
{			strcpy(p_simpleitem->SimpleItem.Name, "State");
			strcpy(p_simpleitem->SimpleItem.Value, (value)?"true":"false");
		}
		else if(event == ONVIF_EVENT_MOTION_REGION)
{			strcpy(p_simpleitem->SimpleItem.Name, "State");
			strcpy(p_simpleitem->SimpleItem.Value, (value)?"true":"false");
		}
	}

	return 0;
}

void onvifserver_notify_event(ONVIF_EVENT_E event, int value)

{	NotificationMessageList * p_message = onvif_add_NotificationMessage(NULL);

	g_onvif_cls.last_event_value[event] = value;

	if(p_message)
{		if(onvifserver_make_event(p_message, PropertyOperation_Changed, event, value) == 0)
{			onvif_put_NotificationMessage(p_message);
		}
	}

	if(event == ONVIF_EVENT_MOTION)
{		p_message = onvif_add_NotificationMessage(NULL);
		event = ONVIF_EVENT_MOTION_REGION;

		g_onvif_cls.last_event_value[event] = value;

		if(p_message)
{			if(onvifserver_make_event(p_message, PropertyOperation_Changed, event, value) == 0)
{				onvif_put_NotificationMessage(p_message);
			}
		}
	}
}

static void _init_NotificationMessage(EUA *p_eua, char *topic)

{	ONVIF_EVENT_E topic_event = ONVIF_EVENT_COUNT;

	if(soap_strcmp(topic, "VideoSource/MotionAlarm") == 0)
{		topic_event = ONVIF_EVENT_MOTION;
	}
	else if(soap_strcmp(topic, "RuleEngine/MotionRegionDetector/Motion") == 0)
{		topic_event = ONVIF_EVENT_MOTION_REGION;
	}

	if(topic_event >= ONVIF_EVENT_COUNT)
{		return;
	}

	NotificationMessageList * p_message = onvif_add_NotificationMessage(NULL);
	if(p_message)
{		if(onvifserver_make_event(p_message, PropertyOperation_Initialized, topic_event, g_onvif_cls.last_event_value[topic_event]) == 0)
{			onvif_put_InitNotificationMessage(p_eua, p_message);
		}
	}
}

void onvifserver_init_NotificationMessage(const char *p_eua, char *topic_expression)

{	int i;
	EUA *eua = (EUA *)p_eua;
	char *tmp = topic_expression;
	char *p;
	char topic[256];

	if(topic_expression == NULL)
{		for(i = 0; i < ONVIF_EVENT_COUNT; i++)
{			NotificationMessageList * p_message = onvif_add_NotificationMessage(NULL);
			if(p_message)
{				if(onvifserver_make_event(p_message, PropertyOperation_Initialized, i, g_onvif_cls.last_event_value[i]) == 0)
{					onvif_put_InitNotificationMessage(eua, p_message);
				}
			}
		}
	}
	else
{		while(p)
{			memset(topic, 0, sizeof(topic));
			strncpy(topic, tmp, p-tmp);

			_init_NotificationMessage(eua, topic);

			tmp = p+1;
			p = strchr(tmp, '|');
		}

		if(tmp)
{			_init_NotificationMessage(eua, tmp);
		}
	}

}

void onvifserver_update_basic()

{	setup_ini_load();
	g_onvif_cfg.http_max_users = 16;
	g_onvif_cfg.log_enable = 0;//1;
	//w4000 g_onvif_cfg.log_level = HT_LOG_WARN;
	
#if 0//w4000
	if(is_auth_w4000()==0)
		{
		printf("w4000 %s \r\n",__FUNCTION__);
		//return;
		}
#endif

	snprintf(g_onvif_cfg.DeviceInformation.Manufacturer, 64, "%s", setup_ini_get_str("cam_setup", "SYSTEM_MANUFACTURER", "IPCAM"));
	snprintf(g_onvif_cfg.DeviceInformation.Model, 64, "%s", setup_ini_get_str("cam_setup", "INFO_MODEL_NAME", "21C26IP6"));
	snprintf(g_onvif_cfg.DeviceInformation.FirmwareVersion, 64, "v%d.%02d.%02d", atoi(RELVER), atoi(MAJVER), atoi(MINVER));
	printf("%s \n", g_onvif_cfg.DeviceInformation.FirmwareVersion);
	unsigned char mac_addr[6];
	if_get_mac("eth0", mac_addr);
	snprintf(g_onvif_cfg.DeviceInformation.HardwareId, 64, "%02X%02X%02X%02X%02X%02X", mac_addr[0],mac_addr[1],mac_addr[2],mac_addr[3],mac_addr[4],mac_addr[5]);	snprintf(g_onvif_cfg.DeviceInformation.SerialNumber, 64, "%s-%s", g_onvif_cfg.DeviceInformation.HardwareId, def_resol_string());
}

void onvifserver_update_datetime()

{	
	g_onvif_cfg.SystemDateTime.DateTimeType = (cam_setup()->tmsync.usage == TMS_NTPSVR)?SetDateTimeType_NTP:SetDateTimeType_Manual;
    g_onvif_cfg.SystemDateTime.DaylightSavings = (cam_setup()->dst)?TRUE:FALSE;
    g_onvif_cfg.SystemDateTime.TimeZoneFlag = (cam_setup()->tmzone == 0)?FALSE:TRUE;
	onvif_get_timezone(g_onvif_cfg.SystemDateTime.TimeZone.TZ, sizeof(g_onvif_cfg.SystemDateTime.TimeZone.TZ));
}

void onvifserver_update_user()

{	int i;
	int idx = 0;
	for(i = 0; i < 10 && idx < MAX_USERS; i++)
	{		
		if(!cam_setup()->usr[i].avail)
		{			
			continue;
		}
			g_onvif_cfg.users[idx].PasswordFlag = 1;
			g_onvif_cfg.users[idx].fixed = FALSE;	// TRUE: Fixed user, delete is not allowed
			snprintf(g_onvif_cfg.users[idx].Username, 100, cam_setup()->usr[i].usrname);
			snprintf(g_onvif_cfg.users[idx].Password, 100, cam_setup()->usr[i].usrpwd);
			onvif_UserLevel level = UserLevel_User;

			g_onvif_cfg.users[idx].UserLevel = level;
			idx++;
		
		printf("w4000------------- %s,cam_setup()->usr[i].usrname =%s \r\n",__FUNCTION__,cam_setup()->usr[i].usrname);// is_auth_w4000(void)
	}

	for(; idx < MAX_USERS; idx++)
	{		
		memset(&g_onvif_cfg.users[idx], 0, sizeof(g_onvif_cfg.users[idx]));
	}
}

void onvifserver_update_scopes()

{	setup_ini_load();
	g_onvif_cfg.scopes[0].ScopeDef = ScopeDefinition_Fixed;
	snprintf(g_onvif_cfg.scopes[0].ScopeItem, 128, "onvif://www.onvif.org/type/video_encoder");
	g_onvif_cfg.scopes[1].ScopeDef = ScopeDefinition_Fixed;
	snprintf(g_onvif_cfg.scopes[1].ScopeItem, 128, "onvif://www.onvif.org/type/audio_encoder");
	g_onvif_cfg.scopes[2].ScopeDef = ScopeDefinition_Fixed;
	snprintf(g_onvif_cfg.scopes[2].ScopeItem, 128, "onvif://www.onvif.org/type/Network_Video_Transmitter");
	g_onvif_cfg.scopes[3].ScopeDef = ScopeDefinition_Fixed;
	snprintf(g_onvif_cfg.scopes[3].ScopeItem, 128, "onvif://www.onvif.org/Profile/Streaming");
	g_onvif_cfg.scopes[4].ScopeDef = ScopeDefinition_Fixed;
	snprintf(g_onvif_cfg.scopes[4].ScopeItem, 128, "onvif://www.onvif.org/hardware/%s", setup_ini_get_str("cam_setup", "INFO_MODEL_NAME", "21C26IP6"));
#ifdef MEDIA2_SUPPORT
    onvif_add_scope("onvif://www.onvif.org/Profile/T", TRUE);
#endif

	scope_file_read();//w4000_info
}

void onvifserver_update_network_protocol()

{	g_onvif_cfg.network.NetworkProtocol.RTSPEnabled = cam_setup_ext()->rtsp_info.use_rtsp;
	g_onvif_cfg.network.NetworkProtocol.HTTPPort[0] = cam_setup()->net.port_web;
	g_onvif_cfg.network.NetworkProtocol.HTTPSPort[0] = cam_setup()->net.port_https;
	g_onvif_cfg.network.NetworkProtocol.RTSPPort[0] = cam_setup_ext()->rtsp_info.rtsp_port;
}

void onvifserver_update_ntp()

{    g_onvif_cfg.network.NTPInformation.FromDHCP = FALSE;
    strcpy(g_onvif_cfg.network.NTPInformation.NTPServer[0], cam_setup()->tmsync.ntpsvr);
}

static VideoSourceConfigurationList *onvif_update_video_source_cfg(int ch)

{	int w = 0, h = 0;
	VideoSourceConfigurationList * p_v_src_cfg = NULL;

	w = 1920;
	h = 1080;

	if (w == 0 || h == 0)
	
{		return NULL;
	}

	p_v_src_cfg = onvif_find_VideoSourceConfiguration_by_size(g_onvif_cfg.v_src_cfg, w, h);
	if (p_v_src_cfg)
	
{		return p_v_src_cfg;
	}

	p_v_src_cfg = onvif_add_VideoSourceConfiguration(w, h);

	return p_v_src_cfg;
}

static int _onvif_update_video_encoder(VideoEncoder2ConfigurationList * p_v_enc_cfg, int ch)

{	int changed = 0;



	if(p_v_enc_cfg->Configuration.Resolution.Width != def_resol_index_to_width(30, cam_setup()->cam[ch].res) ||
			p_v_enc_cfg->Configuration.Resolution.Height != def_resol_index_to_height(30, cam_setup()->cam[ch].res))

{		p_v_enc_cfg->Configuration.Resolution.Width = def_resol_index_to_width(30, cam_setup()->cam[ch].res);
		p_v_enc_cfg->Configuration.Resolution.Height = def_resol_index_to_height(30, cam_setup()->cam[ch].res);
		changed = 1;
	}
	p_v_enc_cfg->Configuration.SessionTimeout = 10;

	// if(p_v_enc_cfg->Configuration.RateControl.FrameRateLimit != cam_setup()->cam[ch].fps) 
	
{		p_v_enc_cfg->Configuration.RateControl.FrameRateLimit = cam_setup()->cam[ch].fps;
		
		changed = 1;
	}
	p_v_enc_cfg->Configuration.RateControl.EncodingInterval = cam_setup()->cam[ch].gop;
	


	if(p_v_enc_cfg->Configuration.RateControl.BitrateLimit != def_bitrate_index_to_value(cam_setup()->cam[ch].bitrate))
{		p_v_enc_cfg->Configuration.RateControl.BitrateLimit = def_bitrate_index_to_value(cam_setup()->cam[ch].bitrate);
		changed = 1;
	}
	snprintf(p_v_enc_cfg->Configuration.Encoding, 64, (cam_setup()->cam[ch].codec == 0)?"H264":"H265");

	int encoding = VideoEncoding_H264;
	switch(cam_setup()->cam[ch].codec)
{		case 0:
			encoding = VideoEncoding_H264;
		break;
		case 1:
			encoding = VideoEncoding_H265;
		break;
	}

	if(p_v_enc_cfg->Configuration.VideoEncoding != encoding)
{		changed = 1;
	}
	p_v_enc_cfg->Configuration.VideoEncoding = encoding;


	p_v_enc_cfg->Configuration.GovLengthFlag = 1;
	if(p_v_enc_cfg->Configuration.GovLength != cam_setup()->cam[ch].gop) {		p_v_enc_cfg->Configuration.GovLength = cam_setup()->cam[ch].gop;
		changed = 1;
	}
	p_v_enc_cfg->Configuration.ProfileFlag = 1;
	snprintf(p_v_enc_cfg->Configuration.Profile, 64, "High");

	return changed;
}

static VideoEncoder2ConfigurationList *onvif_update_video_encoder_cfg(int ch)

{    VideoEncoder2ConfigurationList * p_v_enc_cfg;
	VideoEncoder2ConfigurationList v_enc_cfg;

	memset(&v_enc_cfg, 0, sizeof(v_enc_cfg));

	_onvif_update_video_encoder(&v_enc_cfg, ch);

	p_v_enc_cfg = onvif_find_VideoEncoder2Configuration_by_param(g_onvif_cfg.v_enc_cfg, &v_enc_cfg);
	if (p_v_enc_cfg)
	
{		return p_v_enc_cfg;
	}

	p_v_enc_cfg = onvif_add_VideoEncoder2Configuration(&v_enc_cfg);

	return p_v_enc_cfg;
}
static AudioSourceConfigurationList *onvif_update_audio_source_cfg(int ch)

{	return NULL;
}

static AudioEncoder2ConfigurationList *onvif_update_audio_encoder_cfg(int ch)

{	return NULL;
}



void onvifserver_init_profile()

{	int ch;

	onvif_free_profiles(&g_onvif_cfg.profiles);
	onvifserver_get_setup(&g_onvif_setup, &g_onvif_setup_ext);
	g_onvif_cls.v_src_idx = 0;
	g_onvif_cls.v_enc_idx = 0;
	g_onvif_cls.profile_idx = 0;
	g_onvif_cls.max_profile = 8;
	if(g_onvif_cls.max_profile < 3)
{		g_onvif_cls.max_profile = 3;
	}

	for(ch = 0; ch < 2; ch++)
{		ONVIF_PROFILE * profile;
		profile = onvif_add_profile(TRUE);
		if(NULL == profile)
{			return;
		}

		profile->v_src_cfg = onvif_update_video_source_cfg(ch);
		profile->v_src_cfg->Configuration.UseCount++;

		
		profile->v_enc_cfg = onvif_update_video_encoder_cfg(ch);
		if(profile->v_enc_cfg)
{			profile->v_enc_cfg->Configuration.UseCount++;
		}

#ifdef AUDIO_SUPPORT
		profile->a_src_cfg = onvif_update_audio_source_cfg(ch);
		if(profile->a_src_cfg)
{			profile->a_src_cfg->Configuration.UseCount++;
		}

		profile->a_enc_cfg = onvif_update_audio_encoder_cfg(ch);
		if(profile->a_enc_cfg)
{			profile->a_enc_cfg->Configuration.UseCount++;
		}
#endif
		
		onvifserver_get_rtsp_uri(profile->stream_uri, ch);

	}

	onvif_load_profile_cfg(ONVIF_SETUP_FILE);
}

void video_encoder_ch_to_token(int ch, char *token)

{	snprintf(token, 100, "V_ENC_00%d", ch);
}

int video_encoder_token_to_ch(char *token)

{	char prefix[100];
	int ch = 0;
	char *p = token + (strlen(token) - 3);
	ch = strtol(p, NULL, 10);
	return ch;
}

void onvifserver_update_video_encoder()

{	int ch;
	VideoEncoder2ConfigurationList * p_v_enc_cfg;
	char token[100];

	for(ch = 0; ch < 3; ch++)
{		video_encoder_ch_to_token(ch, token);
		p_v_enc_cfg = onvif_find_VideoEncoder2Configuration(g_onvif_cfg.v_enc_cfg, token);
		if(NULL == p_v_enc_cfg)	
{			continue;
		}

		if(_onvif_update_video_encoder(p_v_enc_cfg, ch))
{			onvif_MediaConfigurationChangedNotify(token, "VideoEncoder");
		}
	}

}

void onvifserver_update_isp()

{	g_onvif_cfg.ImagingSettings.BacklightCompensation.Mode = (cam_nsetup()->isp.blc.blc.mode.val == 1)?BacklightCompensationMode_ON:BacklightCompensationMode_OFF;
	g_onvif_cfg.ImagingSettings.Brightness = cam_nsetup()->isp.iq.color.brightness.val;
	g_onvif_cfg.ImagingSettings.ColorSaturation = cam_nsetup()->isp.iq.color.saturation.val;
	g_onvif_cfg.ImagingSettings.Contrast = cam_nsetup()->isp.iq.color.contrast.val;
	g_onvif_cfg.ImagingSettings.Sharpness = cam_nsetup()->isp.iq.sharpeness.val;

	g_onvif_cfg.ImagingSettings.Exposure.MinExposureTime = cam_nsetup()->isp.ae.shutter.min.val;
	g_onvif_cfg.ImagingSettings.Exposure.MaxExposureTime = cam_nsetup()->isp.ae.shutter.max.val;

	g_onvif_cfg.ImagingSettings.WhiteBalance.Mode = (cam_nsetup()->isp.awb.mode.val == 0)?WhiteBalanceMode_MANUAL:WhiteBalanceMode_AUTO;
	// g_onvif_cfg.ImagingSettings.WhiteBalance.CbGain = cam_nsetup()->isp.awb.manual_gain.b_gain.val;
	// g_onvif_cfg.ImagingSettings.WhiteBalance.CrGain = cam_nsetup()->isp.awb.manual_gain.r_gain.val;
	
	
}


