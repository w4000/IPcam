

#include "sys_inc.h"
#include "onvif.h"
#include "onvif_cm.h"
#include "onvif_cfg.h"
#include "xml_node.h"
#include "onvif_utils.h"


// #include "onvifserver.h"



/***************************************************************************************/
extern ONVIF_CFG g_onvif_cfg;
extern ONVIF_CLS g_onvif_cls;

#ifdef ANDROID
#define UUID_FILENAME   "/sdcard/uuid.txt"
#else
#define UUID_FILENAME   "uuid.txt"
#endif



/***************************************************************************************/
int is_auth=0;//w4000
int is_auth_w4000(void)
{
	return is_auth;
}
void set_auth_w4000(int val)
{

	if(val)
	 	is_auth=1;
	else
		is_auth=0;
		
}

void onvif_parse_information_cfg1(XMLN * p_node)
{
	XMLN * p_Manufacturer;
	XMLN * p_Model;
	XMLN * p_FirmwareVersion;
	XMLN * p_SerialNumber;
	XMLN * p_HardwareId;
	#if 1//w4000
	if(is_auth_w4000()==0)
		{
		printf("w4000 %s \r\n",__FUNCTION__);
		//return;
		}
	#endif
	//return;
	p_Manufacturer = _node_get(p_node, "Manufacturer");
	if (p_Manufacturer && p_Manufacturer->data)
	{
		strncpy(g_onvif_cfg.DeviceInformation.Manufacturer, p_Manufacturer->data, sizeof(g_onvif_cfg.DeviceInformation.Manufacturer)-1);
	}

	p_Model = _node_get(p_node, "Model");
	if (p_Model && p_Model->data)
	{
		strncpy(g_onvif_cfg.DeviceInformation.Model, p_Model->data, sizeof(g_onvif_cfg.DeviceInformation.Model)-1);
	}

	p_FirmwareVersion = _node_get(p_node, "FirmwareVersion");
	if (p_FirmwareVersion && p_FirmwareVersion->data)
	{
		strncpy(g_onvif_cfg.DeviceInformation.FirmwareVersion, p_FirmwareVersion->data, sizeof(g_onvif_cfg.DeviceInformation.FirmwareVersion)-1);
	}

	p_SerialNumber = _node_get(p_node, "SerialNumber");
	if (p_SerialNumber && p_SerialNumber->data)
	{
		strncpy(g_onvif_cfg.DeviceInformation.SerialNumber, p_SerialNumber->data, sizeof(g_onvif_cfg.DeviceInformation.SerialNumber)-1);
	}

	p_HardwareId = _node_get(p_node, "HardwareId");
	if (p_HardwareId && p_HardwareId->data)
	{
		strncpy(g_onvif_cfg.DeviceInformation.HardwareId, p_HardwareId->data, sizeof(g_onvif_cfg.DeviceInformation.HardwareId));
	}
}

void onvif_parse_information_cfg(XMLN * p_node)
{
	XMLN * p_Manufacturer;
	XMLN * p_Model;
	XMLN * p_FirmwareVersion;
	XMLN * p_SerialNumber;
	XMLN * p_HardwareId;

	#if 0 // w4000_info
	if (is_auth_w4000() == 0)
	{
		printf("w4000 %s \r\n", __FUNCTION__);
		// 데이터를 "unknown"으로 설정
		strncpy(g_onvif_cfg.DeviceInformation.Manufacturer, "unknown", sizeof(g_onvif_cfg.DeviceInformation.Manufacturer) - 1);
		strncpy(g_onvif_cfg.DeviceInformation.Model, "unknown", sizeof(g_onvif_cfg.DeviceInformation.Model) - 1);
		strncpy(g_onvif_cfg.DeviceInformation.FirmwareVersion, "unknown", sizeof(g_onvif_cfg.DeviceInformation.FirmwareVersion) - 1);
		strncpy(g_onvif_cfg.DeviceInformation.SerialNumber, "unknown", sizeof(g_onvif_cfg.DeviceInformation.SerialNumber) - 1);
		strncpy(g_onvif_cfg.DeviceInformation.HardwareId, "unknown", sizeof(g_onvif_cfg.DeviceInformation.HardwareId) - 1);
		return;
	}
	#endif

	p_Manufacturer = _node_get(p_node, "Manufacturer");
	if (p_Manufacturer && p_Manufacturer->data)
	{
		strncpy(g_onvif_cfg.DeviceInformation.Manufacturer, p_Manufacturer->data, sizeof(g_onvif_cfg.DeviceInformation.Manufacturer) - 1);
	}

	p_Model = _node_get(p_node, "Model");
	if (p_Model && p_Model->data)
	{
		strncpy(g_onvif_cfg.DeviceInformation.Model, p_Model->data, sizeof(g_onvif_cfg.DeviceInformation.Model) - 1);
	}

	p_FirmwareVersion = _node_get(p_node, "FirmwareVersion");
	if (p_FirmwareVersion && p_FirmwareVersion->data)
	{
		strncpy(g_onvif_cfg.DeviceInformation.FirmwareVersion, p_FirmwareVersion->data, sizeof(g_onvif_cfg.DeviceInformation.FirmwareVersion) - 1);
	}

	p_SerialNumber = _node_get(p_node, "SerialNumber");
	if (p_SerialNumber && p_SerialNumber->data)
	{
		strncpy(g_onvif_cfg.DeviceInformation.SerialNumber, p_SerialNumber->data, sizeof(g_onvif_cfg.DeviceInformation.SerialNumber) - 1);
	}

	p_HardwareId = _node_get(p_node, "HardwareId");
	if (p_HardwareId && p_HardwareId->data)
	{
		strncpy(g_onvif_cfg.DeviceInformation.HardwareId, p_HardwareId->data, sizeof(g_onvif_cfg.DeviceInformation.HardwareId) - 1);
	}
}

BOOL onvif_parse_user(XMLN * p_node, onvif_User * p_user)
{
	XMLN * p_username;
    XMLN * p_password;
    XMLN * p_userlevel;

	p_username = _node_get(p_node, "username");
	if (p_username && p_username->data)
	{
		strncpy(p_user->Username, p_username->data, sizeof(p_user->Username)-1);
	}
	else
	{
		return FALSE;
	}

	p_password = _node_get(p_node, "password");
	if (p_password && p_password->data)
	{
		printf("w4000 >>>> p_password  matched\n");
		//is_auth=1;
		strncpy(p_user->Password, p_password->data, sizeof(p_user->Password)-1);
	}
	else
		{
		//is_auth=0;
			printf("w4000 >>>> p_password not matched\n");
		}

	p_userlevel = _node_get(p_node, "userlevel");
	if (p_userlevel && p_userlevel->data)
	{
		p_user->UserLevel = onvif_StringToUserLevel(p_userlevel->data);
	}

	return TRUE;
}

void onvif_parse_h264_options(XMLN * p_video_encoder, VideoEncoder2ConfigurationList * p_v_enc_cfg)
{
	XMLN * p_h264;
	XMLN * p_gov_length;
	XMLN * p_h264_profile;

	p_h264 = _node_get(p_video_encoder, "h264");
	if (NULL == p_h264)
	{
		return;
	}

	p_gov_length = _node_get(p_h264, "gov_length");
	if (p_gov_length && p_gov_length->data)
	{
	    p_v_enc_cfg->Configuration.GovLengthFlag = 1;
		p_v_enc_cfg->Configuration.GovLength = atoi(p_gov_length->data);
	}

	p_h264_profile = _node_get(p_h264, "h264_profile");
	if (p_h264_profile && p_h264_profile->data)
	{
	    p_v_enc_cfg->Configuration.ProfileFlag = 1;
		strncpy(p_v_enc_cfg->Configuration.Profile, p_h264_profile->data, sizeof(p_v_enc_cfg->Configuration.Profile)-1);
	}
}

#ifdef MEDIA2_SUPPORT
void onvif_parse_h265_options(XMLN * p_video_encoder, VideoEncoder2ConfigurationList * p_v_enc_cfg)
{
	XMLN * p_h265;
	XMLN * p_gov_length;
	XMLN * p_h265_profile;

	p_h265 = _node_get(p_video_encoder, "h265");
	if (NULL == p_h265)
	{
		return;
	}

	p_gov_length = _node_get(p_h265, "gov_length");
	if (p_gov_length && p_gov_length->data)
	{
	    p_v_enc_cfg->Configuration.GovLengthFlag = 1;
		p_v_enc_cfg->Configuration.GovLength = atoi(p_gov_length->data);
	}

	p_h265_profile = _node_get(p_h265, "h265_profile");
	if (p_h265_profile && p_h265_profile->data)
	{
	    p_v_enc_cfg->Configuration.ProfileFlag = 1;
		strncpy(p_v_enc_cfg->Configuration.Profile, p_h265_profile->data, sizeof(p_v_enc_cfg->Configuration.Profile)-1);
	}
}
#endif

void onvif_parse_mpeg4_options(XMLN * p_video_encoder, VideoEncoder2ConfigurationList * p_v_enc_cfg)
{
	XMLN * p_mpeg4;
	XMLN * p_gov_length;
	XMLN * p_mpeg4_profile;

	p_mpeg4 = _node_get(p_video_encoder, "mpeg4");
	if (NULL == p_mpeg4)
	{
		return;
	}

	p_gov_length = _node_get(p_mpeg4, "gov_length");
	if (p_gov_length && p_gov_length->data)
	{
	    p_v_enc_cfg->Configuration.GovLengthFlag = 1;
		p_v_enc_cfg->Configuration.GovLength = atoi(p_gov_length->data);
	}

	p_mpeg4_profile = _node_get(p_mpeg4, "mpeg4_profile");
	if (p_mpeg4_profile && p_mpeg4_profile->data)
	{
	    p_v_enc_cfg->Configuration.ProfileFlag = 1;
		strncpy(p_v_enc_cfg->Configuration.Profile, p_mpeg4_profile->data, sizeof(p_v_enc_cfg->Configuration.Profile)-1);
	}
}

VideoSourceConfigurationList * onvif_parse_video_source_cfg(XMLN * p_video_source)
{
	int w = 0, h = 0;
	XMLN * p_width;
	XMLN * p_height;
	VideoSourceConfigurationList * p_v_src_cfg = NULL;

	p_width = _node_get(p_video_source, "width");
	if (p_width && p_width->data)
	{
		w = atoi(p_width->data);
	}

	p_height = _node_get(p_video_source, "height");
	if (p_height && p_height->data)
	{
		h = atoi(p_height->data);
	}

	if (w == 0 || h == 0)
	{
		return NULL;
	}

	p_v_src_cfg = onvif_find_VideoSourceConfiguration_by_size(g_onvif_cfg.v_src_cfg, w, h);
	if (p_v_src_cfg)
	{
		return p_v_src_cfg;
	}

	p_v_src_cfg = onvif_add_VideoSourceConfiguration(w, h);

	return p_v_src_cfg;
}

VideoEncoder2ConfigurationList * onvif_parse_video_encoder_cfg(XMLN * p_video_encoder)
{
    XMLN * p_width;
    XMLN * p_height;
    XMLN * p_quality;
    XMLN * p_session_timeout;
    XMLN * p_framerate;
    XMLN * p_encoding_interval;
    XMLN * p_bitrate_limit;
    XMLN * p_encoding;
    VideoEncoder2ConfigurationList * p_v_enc_cfg;
	VideoEncoder2ConfigurationList v_enc_cfg;

	memset(&v_enc_cfg, 0, sizeof(v_enc_cfg));

	p_width = _node_get(p_video_encoder, "width");
	if (p_width && p_width->data)
	{
		v_enc_cfg.Configuration.Resolution.Width = atoi(p_width->data);
	}

	p_height = _node_get(p_video_encoder, "height");
	if (p_height && p_height->data)
	{
		v_enc_cfg.Configuration.Resolution.Height = atoi(p_height->data);
	}


	p_session_timeout = _node_get(p_video_encoder, "session_timeout");
	if (p_session_timeout && p_session_timeout->data)
	{
		v_enc_cfg.Configuration.SessionTimeout = atoi(p_session_timeout->data);
	}

	p_framerate = _node_get(p_video_encoder, "framerate");
	if (p_framerate && p_framerate->data)
	{
		v_enc_cfg.Configuration.RateControl.FrameRateLimit = (float)atof(p_framerate->data);
	}

	p_encoding_interval = _node_get(p_video_encoder, "encoding_interval");
	if (p_encoding_interval && p_encoding_interval->data)
	{
		v_enc_cfg.Configuration.RateControl.EncodingInterval = atoi(p_encoding_interval->data);
	}

	p_bitrate_limit = _node_get(p_video_encoder, "bitrate_limit");
	if (p_bitrate_limit && p_bitrate_limit->data)
	{
		v_enc_cfg.Configuration.RateControl.BitrateLimit = atoi(p_bitrate_limit->data);
	}

	p_encoding = _node_get(p_video_encoder, "encoding");
	if (p_encoding && p_encoding->data)
	{
		strncpy(v_enc_cfg.Configuration.Encoding, p_encoding->data, sizeof(v_enc_cfg.Configuration.Encoding)-1);
		v_enc_cfg.Configuration.VideoEncoding = onvif_StringToVideoEncoding(p_encoding->data);
	}

	if (strcasecmp(v_enc_cfg.Configuration.Encoding, "MPEG4") == 0 ||
	    strcasecmp(v_enc_cfg.Configuration.Encoding, "MPV4-ES") == 0)
	{
	    strcpy(v_enc_cfg.Configuration.Encoding, "MPV4-ES");
	    v_enc_cfg.Configuration.VideoEncoding = VideoEncoding_MPEG4;

		onvif_parse_mpeg4_options(p_video_encoder, &v_enc_cfg);

		if (!v_enc_cfg.Configuration.ProfileFlag)
		{
		    v_enc_cfg.Configuration.ProfileFlag = 1;
            strcpy(v_enc_cfg.Configuration.Profile, "SP");
		}

		if (!v_enc_cfg.Configuration.GovLengthFlag)
		{
		    v_enc_cfg.Configuration.GovLengthFlag = 1;
            v_enc_cfg.Configuration.GovLength = 30;
		}
	}
	else if (strcasecmp(v_enc_cfg.Configuration.Encoding, "H264") == 0)
	{
		onvif_parse_h264_options(p_video_encoder, &v_enc_cfg);

		if (!v_enc_cfg.Configuration.ProfileFlag)
		{
		    v_enc_cfg.Configuration.ProfileFlag = 1;
            strcpy(v_enc_cfg.Configuration.Profile, "Main");
		}

		if (!v_enc_cfg.Configuration.GovLengthFlag)
		{
		    v_enc_cfg.Configuration.GovLengthFlag = 1;
            v_enc_cfg.Configuration.GovLength = 30;
		}
	}
#ifdef MEDIA2_SUPPORT
	else if (strcasecmp(v_enc_cfg.Configuration.Encoding, "H265") == 0)
	{
		onvif_parse_h265_options(p_video_encoder, &v_enc_cfg);

        // onvif media 1 service don't support H265, force to H264
		v_enc_cfg.Configuration.VideoEncoding = VideoEncoding_H264;

		if (!v_enc_cfg.Configuration.ProfileFlag)
		{
		    v_enc_cfg.Configuration.ProfileFlag = 1;
            strcpy(v_enc_cfg.Configuration.Profile, "Main");
		}

		if (!v_enc_cfg.Configuration.GovLengthFlag)
		{
		    v_enc_cfg.Configuration.GovLengthFlag = 1;
            v_enc_cfg.Configuration.GovLength = 30;
		}
	}
#else
    else if (strcasecmp(v_enc_cfg.Configuration.Encoding, "H265") == 0)
    {
        // onvif media 1 service don't support H265, force to H264
        v_enc_cfg.Configuration.VideoEncoding = VideoEncoding_H264;

        if (!v_enc_cfg.Configuration.ProfileFlag)
		{
		    v_enc_cfg.Configuration.ProfileFlag = 1;
            strcpy(v_enc_cfg.Configuration.Profile, "Main");
		}

		if (!v_enc_cfg.Configuration.GovLengthFlag)
		{
		    v_enc_cfg.Configuration.GovLengthFlag = 1;
            v_enc_cfg.Configuration.GovLength = 30;
		}
    }
#endif
    else if (strcasecmp(v_enc_cfg.Configuration.Encoding, "JPEG"))
	{
	    return NULL;
	}

	p_v_enc_cfg = onvif_find_VideoEncoder2Configuration_by_param(g_onvif_cfg.v_enc_cfg, &v_enc_cfg);
	if (p_v_enc_cfg)
	{
		return p_v_enc_cfg;
	}

	p_v_enc_cfg = onvif_add_VideoEncoder2Configuration(&v_enc_cfg);

	return p_v_enc_cfg;
}

void onvif_parse_profile(XMLN * p_profile)
{
    XMLN * p_video_source;
    XMLN * p_video_encoder;
    XMLN * p_stream_uri;
	ONVIF_PROFILE * profile;

	profile = onvif_add_profile(TRUE);
	if (NULL == profile)
	{
		return;
	}

	p_video_source = _node_get(p_profile, "video_source");
	if (p_video_source)
	{
		profile->v_src_cfg = onvif_parse_video_source_cfg(p_video_source);
		profile->v_src_cfg->Configuration.UseCount++;
	}

	p_video_encoder = _node_get(p_profile, "video_encoder");
	if (p_video_encoder)
	{
		profile->v_enc_cfg = onvif_parse_video_encoder_cfg(p_video_encoder);
		if (profile->v_enc_cfg)
		{
		    profile->v_enc_cfg->Configuration.UseCount++;
		}
	}


	p_stream_uri = _node_get(p_profile, "stream_uri");
	if (p_stream_uri && p_stream_uri->data && strlen(p_stream_uri->data) > 0)
	{
		strncpy(profile->stream_uri, p_stream_uri->data, sizeof(profile->stream_uri));
	}
}

void onvif_parse_event_cfg(XMLN * p_event)
{
	XMLN * p_renew_interval;
	XMLN * p_simulate_enable;

	p_renew_interval = _node_get(p_event, "renew_interval");
	if (p_renew_interval && p_renew_interval->data)
	{
		g_onvif_cfg.evt_renew_time = atoi(p_renew_interval->data);
	}

	p_simulate_enable = _node_get(p_event, "simulate_enable");
	if (p_simulate_enable && p_simulate_enable->data)
	{
		g_onvif_cfg.evt_sim_flag = atoi(p_simulate_enable->data);
	}
}

BOOL onvif_parse_server(XMLN * p_node, ONVIF_SRV * p_req)
{
    XMLN * p_serv_ip;
	XMLN * p_serv_port;

    p_serv_ip = _node_get(p_node, "server_ip");
	if (NULL == p_serv_ip)
	{
	    return FALSE;
	}
	else if (p_serv_ip->data)
	{
		strncpy(p_req->serv_ip, p_serv_ip->data, sizeof(p_req->serv_ip)-1);
	}

	p_serv_port = _node_get(p_node, "server_port");
	if (NULL == p_serv_port || NULL == p_serv_port->data)
	{
	    return FALSE;
	}
	else
	{
		p_req->serv_port = atoi(p_serv_port->data);
	}

	return TRUE;
}

void onvif_parse_cfg(char * xml_buff, int rlen)
{
	XMLN * p_node;
	XMLN * p_servers;
	XMLN * p_http_max_users;
#ifdef HTTPS
	XMLN * p_https_enable;
#endif
	XMLN * p_need_auth;
	XMLN * p_log_enable;
	XMLN * p_log_level;
	XMLN * p_information;
	XMLN * p_event;

	p_node = _hxml_parse(xml_buff, rlen);
	if (NULL == p_node)
	{
		return;
	}

    if (onvif_parse_server(p_node, &g_onvif_cfg.servs[0]))
    {
        g_onvif_cfg.servs_num++;
    }

	p_servers = _node_get(p_node, "servers");
	while (p_servers && strcmp(p_servers->name, "servers") == 0)
	{
	    int idx = g_onvif_cfg.servs_num;

	    if (onvif_parse_server(p_servers, &g_onvif_cfg.servs[idx]))
	    {
	        g_onvif_cfg.servs_num++;

	        if (g_onvif_cfg.servs_num >= ARRAY_SIZE(g_onvif_cfg.servs))
	        {
	            break;
	        }
	    }

	    p_servers = p_servers->next;
	}

	p_http_max_users = _node_get(p_node, "http_max_users");
	if (p_http_max_users && p_http_max_users->data)
	{
		g_onvif_cfg.http_max_users = atoi(p_http_max_users->data);
	}

	p_need_auth = _node_get(p_node, "need_auth");
	if (p_need_auth && p_need_auth->data)
	{
		g_onvif_cfg.need_auth = atoi(p_need_auth->data);
	}

	p_log_level = _node_get(p_node, "log_level");
	if (p_log_level && p_log_level->data)
	{
		g_onvif_cfg.log_level = atoi(p_log_level->data);
	}

	p_information = _node_get(p_node, "information");
	if (p_information)
	{
		onvif_parse_information_cfg(p_information);
	}

	p_event = _node_get(p_node, "event");
	if (p_event)
	{
		onvif_parse_event_cfg(p_event);
	}

	_node_del(p_node);
}

const char config_xml[] =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
"<config>"
"	<server_ip></server_ip>"
"	<server_port>88</server_port>"	// USE_ONVIF_PORT_PROXY
"	<http_max_users>16</http_max_users>"
"	<information>"
"		<Manufacturer>ecam</Manufacturer>"
"		<Model>IPCamera</Model>"
"		<FirmwareVersion>2.0</FirmwareVersion>"
"		<SerialNumber>123456</SerialNumber>"
"		<HardwareId>1.0</HardwareId>"
"	</information>"
"	<user>"
"		<username>admin</username>"
"		<password>admin</password>"
"		<userlevel>Administrator</userlevel>"
"	</user>"
"	<user>"
"		<username>test</username>"
"		<password>123456</password>"
"		<userlevel>User</userlevel>"
"	</user>"
"	<scope>onvif://www.onvif.org/location/country/korea</scope>"
"	<scope>onvif://www.onvif.org/type/video_encoder</scope>"
"	<scope>onvif://www.onvif.org/name/IP-Camera</scope>"
"	<scope>onvif://www.onvif.org/hardware/IP-Camera</scope>"
"	<event>"
"		<renew_interval>60</renew_interval>"
"	</event>"
"</config>";

void onvif_load_cfg(const char * filename)
{
	int len;
    int rlen;
    char * xml_buff;

	if(filename == NULL) {
		len = strlen(config_xml);
		xml_buff = (char *)malloc(len + 1);
		strcpy(xml_buff, config_xml);
		rlen = len;
		xml_buff[rlen] = '\0';
		// printf("%s \n", xml_buff);
		onvif_parse_cfg(xml_buff, rlen);
		free(xml_buff);
	}
	else {
		// read config file
		FILE * fp;
		fp = fopen(filename, "r");
		if (NULL == fp)
		{
			return;
		}

		fseek(fp, 0, SEEK_END);

		len = ftell(fp);
		if (len <= 0)
		{
			fclose(fp);
			return;
		}
		fseek(fp, 0, SEEK_SET);

		xml_buff = (char *) malloc(len + 1);
		if (NULL == xml_buff)
		{
			fclose(fp);
			return;
		}

		rlen = (int)fread(xml_buff, 1, len, fp);
		if (rlen > 0)
		{
			xml_buff[rlen] = '\0';
			onvif_parse_cfg(xml_buff, rlen);
		}
		else
		{
		}

		fclose(fp);

		free(xml_buff);
	}
}

BOOL onvif_read_device_uuid(char * buff, int bufflen)
{
    int len, rlen;
    FILE * fp;

	fp = fopen(UUID_FILENAME, "r");
	if (NULL == fp)
	{
		return FALSE;
	}

	fseek(fp, 0, SEEK_END);

	// get file length
	len = ftell(fp);
	if (len <= 0)
	{
		fclose(fp);
		return FALSE;
	}
	fseek(fp, 0, SEEK_SET);

    if (bufflen <= len)
    {
        printf("filelen = %d, bufflen = %d\r\n", len, bufflen);
    }
    else
    {
    	rlen = (int)fread(buff, 1, len, fp);

        // remove \r\n
    	while (rlen > 0 && (buff[rlen-1] == '\r' || buff[rlen-1] == '\n'))
    	{
    	    rlen--;
    	    buff[rlen] = '\0';
    	}
	}

	fclose(fp);

	return TRUE;
}

BOOL onvif_save_device_uuid(char * buff)
{
    FILE * fp;

	fp = fopen(UUID_FILENAME, "w");
	if (NULL == fp)
	{
	    printf("open file uuid.txt failed\r\n");
		return FALSE;
	}

	fwrite(buff, 1, strlen(buff), fp);

	fclose(fp);

	return TRUE;
}

#ifdef PROFILE_Q_SUPPORT

int onvif_read_device_state()
{
    int state = 0, rlen;
    char buff[8] = {'\0'};
    FILE * fp;

	fp = fopen("devst.txt", "r");
	if (NULL == fp)
	{
		return state;
	}

	rlen = (int)fread(buff, 1, 1, fp);
	if (1 == rlen)
	{
	    state = atoi(buff);
	}

	fclose(fp);

	return state;
}

BOOL onvif_save_device_state(int state)
{
    char buff[8] = {'\0'};
    FILE * fp;

	fp = fopen("devst.txt", "w");
	if (NULL == fp)
	{
	    printf("open file devst.txt failed\r\n");
		return FALSE;
	}

    sprintf(buff, "%d", state);
	fwrite(buff, 1, strlen(buff), fp);

	fclose(fp);

	return TRUE;
}

#endif // end of PROFILE_Q_SUPPORT

void parse_name_and_token(XMLN *xml, char *name, char *token)
{
	XMLN *p_name;
	XMLN *p_token;

	name[0] = '\0';
	token[0] = '\0';

	p_name = _node_get(xml, "name");
	if(p_name && p_name->data && strlen(p_name->data) > 0) {
		strncpy(name, p_name->data, 100);
	}

	p_token = _node_get(xml, "token");
	if(p_token && p_token->data && strlen(p_token->data) > 0) {
		strncpy(token, p_token->data, 100);
	}
}

void parse_profile(XMLN *p_profile)
{
	ONVIF_PROFILE *profile;
    VideoSourceConfigurationList * p_v_src_cfg;
    VideoEncoder2ConfigurationList * p_v_enc_cfg;

	XMLN *p_video_source;
	XMLN *p_video_encoder;
#if 0
	XMLN *p_audio_source;
	XMLN *p_audio_encoder;
#endif

	char name[100];
	char token[100];



	parse_name_and_token(p_profile, name, token);
	if(strlen(name) <= 0 && strlen(token) <= 0) {
		return;
	}

	profile = onvif_add_profile(FALSE);
	if(NULL == profile) {
		return;
	}

	if(strlen(name) > 0) {
		strncpy(profile->name, name, sizeof(profile->name));
	}
	if(strlen(token) > 0) {
		strncpy(profile->token, token, sizeof(profile->token));
	}

	p_video_source = _node_get(p_profile, "video_source");
	if(p_video_source) {
		parse_name_and_token(p_video_source, name, token);
		if(strlen(token) > 0) {
			p_v_src_cfg = onvif_find_VideoSourceConfiguration(g_onvif_cfg.v_src_cfg, token);
			if(NULL != p_v_src_cfg) {
				if(profile->v_src_cfg != p_v_src_cfg) {
					if(profile->v_src_cfg && profile->v_src_cfg->Configuration.UseCount > 0) {
						profile->v_src_cfg->Configuration.UseCount--;
					}

					p_v_src_cfg->Configuration.UseCount++;
					profile->v_src_cfg = p_v_src_cfg;
				}
			}
		}
	}

	p_video_encoder = _node_get(p_profile, "video_encoder");
	if(p_video_encoder) {
		parse_name_and_token(p_video_encoder, name, token);
		if(strlen(token) > 0) {
			p_v_enc_cfg = onvif_find_VideoEncoder2Configuration(g_onvif_cfg.v_enc_cfg, token);
			if(NULL != p_v_enc_cfg)	{
				if(profile->v_enc_cfg != p_v_enc_cfg) {
					if(profile->v_enc_cfg && profile->v_enc_cfg->Configuration.UseCount > 0) {
						profile->v_enc_cfg->Configuration.UseCount--;
					}

					p_v_enc_cfg->Configuration.UseCount++;
					profile->v_enc_cfg = p_v_enc_cfg;
					onvifserver_get_rtsp_uri(profile->stream_uri, video_encoder_token_to_ch(token));
				}
			}
		}
	}
}

void onvif_parse_profile_cfg(char * xml_buff, int rlen)
{
	XMLN * p_node;
	XMLN * p_profile;

	p_node = _hxml_parse(xml_buff, rlen);
	if(NULL == p_node) {
		return;
	}

	p_profile = _node_get(p_node, "profile");
	while (p_profile && strcmp(p_profile->name, "profile") == 0)
	{
	   	parse_profile(p_profile);

	    p_profile = p_profile->next;
	}

	_node_del(p_node);
}

void onvif_load_profile_cfg(const char * filename)
{
	int len;
    int rlen;
    char * xml_buff;

	// read config file
	FILE * fp;
	fp = fopen(filename, "r");
	if (NULL == fp)
	{
		return;
	}

	fseek(fp, 0, SEEK_END);

	len = ftell(fp);
	if (len <= 0)
	{
		fclose(fp);
		return;
	}
	fseek(fp, 0, SEEK_SET);

	xml_buff = (char *) malloc(len + 1);
	if (NULL == xml_buff)
	{
		fclose(fp);
		return;
	}

	rlen = (int)fread(xml_buff, 1, len, fp);
	if (rlen > 0)
	{
		xml_buff[rlen] = '\0';
		onvif_parse_profile_cfg(xml_buff, rlen);
	}
	else
	{
	}

	fclose(fp);

	free(xml_buff);
}

static const char *onvif_xml_header =
	"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
	"<config>\r\n";

static const char *onvif_xml_profile =
	"	<profile>\r\n"
	"		<name>%s</name>\r\n"
	"		<token>%s</token>\r\n"
	"		<video_source>\r\n"
	"			<token>%s</token>\r\n"
	"		</video_source>\r\n"
	"		<video_encoder>\r\n"
	"			<token>%s</token>\r\n"
	"		</video_encoder>\r\n"
	"	</profile>\r\n";

static const char *onvif_xml_footer =
	"</config>\r\n";

void onvif_save_profile_cfg(const char * filename, ONVIF_PROFILE *profiles)
{
	int len;
    int rlen;
	char buff[512];

	// write config file
	FILE * fp;
	remove(filename);
	fp = fopen(filename, "w");
	if(NULL == fp) {
		return;
	}

	fseek(fp, 0, SEEK_SET);
	fwrite(onvif_xml_header, 1, strlen(onvif_xml_header), fp);

	{
		ONVIF_PROFILE *p_next;
		ONVIF_PROFILE *p_tmp = profiles;
		while(p_tmp) {
			p_next = p_tmp->next;
			if(p_tmp->fixed == FALSE) {
				sprintf(buff, onvif_xml_profile,
						p_tmp->name, p_tmp->token,
						(p_tmp->v_src_cfg)?p_tmp->v_src_cfg->Configuration.token:"",
						(p_tmp->v_enc_cfg)?p_tmp->v_enc_cfg->Configuration.token:"");
				fwrite(buff, 1, strlen(buff), fp);
			}
			p_tmp = p_next;
		}
	}

	onvif_bprint("onvif_save_profile_cfg  buf : %s \n",buff);


	fwrite(onvif_xml_footer, 1, strlen(onvif_xml_footer), fp);

	fclose(fp);
}
