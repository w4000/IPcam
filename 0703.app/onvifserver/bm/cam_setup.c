#include "sys_inc.h"
#include "onvif.h"
#include "onvif_cm.h"
#include "onvif_cfg.h"
#include "cam_setup.h"
#include "cam_timezone.h"

/***************************************************************************************/
extern ONVIF_CFG g_onvif_cfg;
extern ONVIF_CLS g_onvif_cls;
/***************************************************************************************/

static _CAMSETUP g_cam_setup;
static _CAMSETUP_EXT g_cam_setup_ext;
static int init_cam_setup_flag = 0;


enum {
	UPDATE_ONVIF_ISP_IQ,
	UPDATE_ONVIF_ISP_AE,
	UPDATE_ONVIF_ISP_BLC,
	UPDATE_ONVIF_ISP_AWB,
	UPDATE_ONVIF_ISP_DN,
	UPDATE_ONVIF_ISP_MAX
};

#define printf_setup()printf("w4000 %s, %d \n",__FUNCTION__,__LINE__)//w4000

void cam_setup_update(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext)
{
	init_cam_setup_flag = 1;
	memcpy(&g_cam_setup, setup, sizeof(_CAMSETUP));
	memcpy(&g_cam_setup_ext, setup_ext, sizeof(_CAMSETUP_EXT));
}

int check_cam_setup_update(void)
{
	return init_cam_setup_flag;
}

_CAMSETUP *cam_setup()
{
	return &g_cam_setup;
}

NSETUP_T *cam_nsetup()
{
	return &(g_cam_setup.nsetup);
}

_CAMSETUP_EXT *cam_setup_ext()
{
	return &g_cam_setup_ext;
}

#define ONVIF_SCOPE_FILE "/mnt/ipm/onvif_scope"

int scope_file_remove()
{
	FILE *pf = NULL;
	char scope_item[128];
	int i;
	printf_setup();

	if(access(ONVIF_SCOPE_FILE, F_OK) ==  0) {
		remove(ONVIF_SCOPE_FILE);
	}

	return 0;
}

int scope_file_update()
{printf_setup();
	FILE *pf = NULL;
	char scope_item[128];
	int i;

	remove(ONVIF_SCOPE_FILE);
	pf = fopen(ONVIF_SCOPE_FILE, "w");
	if(pf == NULL) {
		return -1;
	}

	for(i = 0; i < ARRAY_SIZE(g_onvif_cfg.scopes); i++)	{
		if(g_onvif_cfg.scopes[i].ScopeItem[0] == '\0') {
			break;
		}
		if(g_onvif_cfg.scopes[i].ScopeDef == ScopeDefinition_Fixed) {
			continue;
		}

		snprintf(scope_item, 128, "%s\n", g_onvif_cfg.scopes[i].ScopeItem);
		fwrite(scope_item, 1, strlen(scope_item), pf);
	}

	fclose(pf);
	return 0;
}

int scope_file_read()
{printf_setup();
	FILE *pf = NULL;
	char scope_item[128];
	int idx = 5;

	if(access(ONVIF_SCOPE_FILE, F_OK) < 0) {
		pf = fopen(ONVIF_SCOPE_FILE, "w");
		if(pf == NULL) {
			return -1;
		}
		snprintf(scope_item, 128, "onvif://www.onvif.org/name/%s\n", "21C26IP6");
		fwrite(scope_item, 1, strlen(scope_item), pf);
		snprintf(scope_item, 128, "onvif://www.onvif.org/location/country/korea\n");
		fwrite(scope_item, 1, strlen(scope_item), pf);
		fclose(pf);
	}

	pf = fopen(ONVIF_SCOPE_FILE, "r");
	if(pf == NULL) {
		return -1;
	}

	while(!feof(pf)) {
		if(fgets(scope_item, sizeof(scope_item), pf) == NULL) {
			break;
		}

		char *p = strchr(scope_item, '\n');
		if(p != NULL) {
			*p = '\0';
		}

		onvif_add_scope(scope_item, FALSE);
		idx++;
		if(idx >= 100) {
			break;
		}
	}

	idx++;
	for( ; idx < 100; idx++) {
		g_onvif_cfg.scopes[idx].ScopeItem[0] = '\0';
	}

	fclose(pf);

	return 0;
}


int convert_UTC_to_localtime(int index, onvif_DateTime *datetime, _CAMSETUP	*setup)
{
	const char* STR_REF	= "UTC";
	const char* PLUS	= "+";
	const char* MINUS	= "-";
	char* strUtc;
	int dir_day = 0;
	int hour = 0;
	int minute = 0;
	int up_down_flag = 0;
	int end_of_day = 0;
	int end_of_month = 12;

	strUtc = strstr(cam_tmzone2_str[index].ForStandardTime, STR_REF) + strlen(STR_REF);

	if(strUtc == NULL) {
		goto CONVERT_TIME_END;
	}
	else if(strncmp(strUtc, PLUS, strlen(PLUS)) == 0) {
		dir_day = 1;
		strUtc++;
	}
	else if(strncmp(strUtc, MINUS, strlen(MINUS)) == 0) {
		dir_day = -1;
		strUtc++;
	}
	else {
		goto CONVERT_TIME_END;
	}

	char tmp;
	tmp = strUtc[0];
	if(tmp >= '1' && tmp <= '9') {
		hour = tmp - '0';
		strUtc++;
		tmp = strUtc[0];
		if(tmp >= '1' && tmp <= '9') {
			hour = hour * 10 + (tmp - '0');
			strUtc++;
			tmp = strUtc[0];
		}
		hour *= dir_day;
	}

	if(tmp == ':') {
		strUtc++;
		char tmp1 = strUtc[0];
		char tmp2 = strUtc[1];
		minute = (tmp1 - '0') * 10 + (tmp1 - '0');
		minute *= dir_day;
	}

CONVERT_TIME_END:
	setup->sv_tm.tm_sec = datetime->Time.Second;
	if(dir_day > 0) {
		if(datetime->Time.Minute + minute > 60) {
			setup->sv_tm.tm_min = datetime->Time.Minute + minute - 60;
			up_down_flag = 1;
		}
		else {
			setup->sv_tm.tm_min = datetime->Time.Minute + minute;
		}

		if(datetime->Time.Hour + hour + up_down_flag > 24) {
			setup->sv_tm.tm_hour = datetime->Time.Hour + hour + up_down_flag - 24;
			up_down_flag = 1;
		}
		else {
			setup->sv_tm.tm_hour = datetime->Time.Hour + hour + up_down_flag;
			up_down_flag = 0;
		}

		if(datetime->Date.Month % 2 != 0) {
			end_of_day = 31;
		}
		else if(datetime->Date.Month != 2) {
			end_of_day = 30;
		}
		else if((datetime->Date.Year % 4 == 0) && ((datetime->Date.Year % 100 != 0) || (datetime->Date.Year % 400 == 0))) {
			end_of_day = 29;
		}
		else {
			end_of_day = 28;
		}

		if(datetime->Date.Day + up_down_flag > end_of_day) {
			setup->sv_tm.tm_mday = 1;
			up_down_flag = 1;
		}
		else {
			setup->sv_tm.tm_mday = datetime->Date.Day + up_down_flag;
			up_down_flag = 0;
		}

		if(datetime->Date.Month + up_down_flag > end_of_month) {
			setup->sv_tm.tm_mon = 1;
			up_down_flag = 1;
		}
		else {
			setup->sv_tm.tm_mon = datetime->Date.Month + up_down_flag - 1;
			up_down_flag = 0;
		}

		setup->sv_tm.tm_year = datetime->Date.Year - 1900 + up_down_flag;
	}
	else if(dir_day < 0) {	// dir_day < 0
		if(datetime->Time.Minute + minute < 0) {
			setup->sv_tm.tm_min = datetime->Time.Minute + minute + 60;
			up_down_flag = -1;
		}
		else {
			setup->sv_tm.tm_min = datetime->Time.Minute + minute;
		}

		if(datetime->Time.Hour + hour + up_down_flag < 0) {
			setup->sv_tm.tm_hour = datetime->Time.Hour + hour + up_down_flag + 24;
			up_down_flag = -1;
		}
		else {
			setup->sv_tm.tm_hour = datetime->Time.Hour + hour + up_down_flag;
			up_down_flag = 0;
		}

		if(datetime->Date.Day + up_down_flag < 1) {
			setup->sv_tm.tm_mday = 1;
			up_down_flag = -1;
		}
		else {
			setup->sv_tm.tm_mday = datetime->Date.Day + up_down_flag;
			up_down_flag = 0;
		}

		if(datetime->Date.Month + up_down_flag < 1) {
			setup->sv_tm.tm_mon = 1;
			up_down_flag = -1;
		}
		else {
			setup->sv_tm.tm_mon = datetime->Date.Month + up_down_flag - 1;
			up_down_flag = 0;
		}

		setup->sv_tm.tm_year = datetime->Date.Year - 1900 + up_down_flag;
	}
	else {	// dir_day == 0
		setup->sv_tm.tm_year = datetime->Date.Year - 1900;
		setup->sv_tm.tm_mon = datetime->Date.Month - 1;
		setup->sv_tm.tm_mday = datetime->Date.Day;
		setup->sv_tm.tm_hour = datetime->Time.Hour;
		setup->sv_tm.tm_min = datetime->Time.Minute;
		setup->sv_tm.tm_sec = datetime->Time.Second;
	}

}

int cam_setup_apply_datetime(onvif_DateTime *datetime)
{
	_CAMSETUP	*setup = &g_cam_setup;
	_CAMSETUP_EXT	*setup_ext = &g_cam_setup_ext;

	char* _TZ;
	static int i;
	int j;
	int bIsSetTZ = 0;

	_TZ = g_onvif_cfg.SystemDateTime.TimeZone.TZ;


	{
		for(i = 0; ; i++) {

			if(strcmp(cam_tmzone2_str[i].OfficialTimezoneName, "-") == 0)
				break;

			if(strcmp(cam_tmzone2_str[i].TZ, _TZ) == 0) {
				for(j = 0; j < TMZONE_MAX; j++)	{
					if(strcmp(cam_tmzone2_str[i].TimezoneName, cam_tmzone_set[j]) == 0)	{
						bIsSetTZ = 1;
						setup->tmzone = j;
						break;
					}
				}
				break;
			}
		}

		if(!bIsSetTZ) {
			for(i = 0; i < TMZONE_MAX; i++)	{

				if(strcasecmp(cam_tmzone_str[i][0], _TZ) == 0)	{
					bIsSetTZ = 1;
					setup->tmzone = j;
					break;
				}

			}
		}

		if(!bIsSetTZ)
			return ONVIF_ERR_InvalidTimeZone;
	}

	if(g_onvif_cfg.SystemDateTime.DateTimeType == SetDateTimeType_Manual) {
		
		convert_UTC_to_localtime(i, datetime, setup);
	}

	setup->dst = g_onvif_cfg.SystemDateTime.DaylightSavings;
	setup->tmsync.usage = (g_onvif_cfg.SystemDateTime.DateTimeType == SetDateTimeType_NTP)?TMS_NTPSVR:TMS_NONE;
	printf("w4000 m %s, %d\n",__FUNCTION__,__LINE__);
	setup->update_tm = 2;
	
	setup_save(&DEV_MTD, setup, setup_ext);
	
	send_msg(ONVIF, MAIN, _CAMMSG_CGI, 1, 1, 0);

	return 0;
}



int cam_setup_apply_factorydefault(int hard)
{

	return 0;
}

int cam_setup_apply_network_interface(tds_SetNetworkInterfaces_REQ *p_req)
{
	printf_setup();
	_CAMSETUP	*setup = &g_cam_setup;
	_CAMSETUP_EXT	*setup_ext = &g_cam_setup_ext;

	if(p_req->NetworkInterface.Enabled == FALSE) {
		return ONVIF_OK;
	}

	if(p_req->NetworkInterface.IPv4Flag) {
		setup->net.type = (p_req->NetworkInterface.IPv4.Config.DHCP == TRUE)?NETTYPE_DHCP:NETTYPE_LAN;
		if(setup->net.type == NETTYPE_LAN) {
			if(inet_aton(p_req->NetworkInterface.IPv4.Config.Address, &setup->net.ip) == 0) {
				return ONVIF_ERR_InvalidIPv4Address;
			}

			if(p_req->NetworkInterface.IPv4.Config.PrefixLength != 0) {
				unsigned long mask = (0xFFFFFFFF << (32 - p_req->NetworkInterface.IPv4.Config.PrefixLength)) & 0xFFFFFFFF;
				char subnet[32];
				sprintf(subnet, "%lu.%lu.%lu.%lu\n", mask >> 24, (mask >> 16) & 0xFF, (mask >> 8) & 0xFF, mask & 0xFF);
				inet_aton(subnet, &setup->net.netmask);
			}
		}
	}

	setup_save(&DEV_MTD, setup, setup_ext);
	setup_ext->cgi_update_notify[UPDATE_CGI_NET] = 1;
	
	send_msg(ONVIF, MAIN, _CAMMSG_CGI, 1, 1, 2);

	return 0;
}

int cam_setup_apply_network_protocol(tds_SetNetworkProtocols_REQ *p_req)
{printf_setup();
	
	_CAMSETUP	*setup = &g_cam_setup;
	_CAMSETUP_EXT	*setup_ext = &g_cam_setup_ext;

	if(p_req->NetworkProtocol.HTTPFlag)	{
		if(p_req->NetworkProtocol.HTTPEnabled) {
			cam_setup()->net.port_web = p_req->NetworkProtocol.HTTPPort[0];
		}
	}

	if(p_req->NetworkProtocol.RTSPFlag)	{
		cam_setup_ext()->rtsp_info.use_rtsp = p_req->NetworkProtocol.RTSPEnabled;
		cam_setup_ext()->rtsp_info.rtsp_port = p_req->NetworkProtocol.RTSPPort[0];
	}


	
	setup_save(&DEV_MTD, setup, setup_ext);

	
	send_msg(ONVIF, MAIN, _CAMMSG_CGI, 1, 1, 0);

	return 0;
}

int cam_setup_apply_network_default_gateway(tds_SetNetworkDefaultGateway_REQ *p_req)
{printf_setup();
	_CAMSETUP	*setup = &g_cam_setup;
	_CAMSETUP_EXT	*setup_ext = &g_cam_setup_ext;

	if(inet_aton(p_req->IPv4Address[0], &setup->net.gateway) == 0) {
		return ONVIF_ERR_InvalidIPv4Address;
	}

	setup_save(&DEV_MTD, setup, setup_ext);
	
	send_msg(ONVIF, MAIN, _CAMMSG_CGI, 1, 1, 2);
}

int cam_setup_apply_ntp(tds_SetNTP_REQ *p_req)
{printf_setup();
	_CAMSETUP	*setup = &g_cam_setup;
	_CAMSETUP_EXT	*setup_ext = &g_cam_setup_ext;

	if(p_req->NTPInformation.FromDHCP == FALSE) {
		strcpy(g_onvif_cfg.network.NTPInformation.NTPServer[0], cam_setup()->tmsync.ntpsvr);
		printf("w4000 m %s, %d\n",__FUNCTION__,__LINE__);
	}
	else {
		strcpy(g_onvif_cfg.network.NTPInformation.NTPServer[0], "pool.ntp.org");
		printf("w4000 m %s, %d\n",__FUNCTION__,__LINE__);
	}
	cam_setup()->tmsync.usage = TMS_NTPSVR;

	setup_save(&DEV_MTD, setup, setup_ext);
	
	send_msg(ONVIF, MAIN, _CAMMSG_CGI, 1, 1, 0);

	return 0;
}

int cam_setup_apply_user()
{printf_setup();

	int i;
	int j = 0;
	_CAMSETUP	*setup = &g_cam_setup;
	_CAMSETUP_EXT	*setup_ext = &g_cam_setup_ext;

	onvif_User *users = (onvif_User *)g_onvif_cfg.users;

	for(i = 0; i < ARRAY_SIZE(g_onvif_cfg.users); i++) {
		if(g_onvif_cfg.users[i].Username[0] == '\0') {
			continue;
		}
		memset(&setup->usr[j], 0, sizeof(setup->usr[j]));
		setup->usr[j].avail = 1;
		snprintf(setup->usr[j].usrname, USRNAME_MAX, users[i].Username);
		snprintf(setup->usr[j].usrpwd, USRPWD_MAX, users[i].Password);
		if(users[i].UserLevel == UserLevel_Administrator) {
			setup->usr[j].uperm.live = 1;
			setup->usr[j].uperm.setup = 1;
			setup->usr[j].uperm.vod = 1;
		}
		else {
			setup->usr[j].uperm.live = 1;
		}
		printf("w4000------------- %s, users[i].Username =%s \r\n",__FUNCTION__,users[i].Username);// is_auth_w4000(void)
		if(users[i].UserLevel == UserLevel_Administrator) {
			snprintf(setup->usr[j].usrdesc, USRDESC_MAX, "Administrator");
		}
		else if(users[i].UserLevel == UserLevel_Operator) {
			snprintf(setup->usr[j].usrdesc, USRDESC_MAX, "Operator");
		}
		else { //if(users[i].UserLevel == UserLevel_User) {
			snprintf(setup->usr[j].usrdesc, USRDESC_MAX, "User");
		}
		j++;
	}

	for( ; j < 10; j++) {
		memset(&setup->usr[j], 0, sizeof(setup->usr[j]));
	}

	setup_save(&DEV_MTD, setup, setup_ext);
	
	send_msg(ONVIF, MAIN, _CAMMSG_CGI, 1, 1, 0);

	return 0;
}


int cam_setup_apply_video_encoder(int ch, trt_SetVideoEncoderConfiguration_REQ *p_req)
{
	_CAMSETUP	*setup = &g_cam_setup;
	_CAMSETUP_EXT	*setup_ext = &g_cam_setup_ext;
	char resol_size[64];

	int codec = 0;
	int gop = 1;
	int fps = 1;
	int bitrate = 200;

	if(p_req->Configuration.Encoding == VideoEncoding_H265){
		codec = 1;
		gop = p_req->Configuration.H264.GovLength;
	}else{
		codec = 0;
		gop = p_req->Configuration.H264.GovLength;
	}

	sprintf(resol_size, "%dx%d", p_req->Configuration.Resolution.Width, p_req->Configuration.Resolution.Height);
	setup->cam[ch].res = def_resol_size_to_index(0, resol_size);
	setup->cam[ch].codec = codec;
	setup->cam[ch].gop = gop;


	{
		setup->cam[ch].fps = p_req->Configuration.RateControl.FrameRateLimit;
		setup->cam[ch].bitrate = def_bitrate_value_to_index(p_req->Configuration.RateControl.BitrateLimit);
	}



	setup_save(&DEV_MTD, setup, setup_ext);
	
	send_msg(ONVIF, MAIN, _CAMMSG_CGI, 1, 1, 1);

	return 0;
}

int cam_setup_apply_video_encoder2(int ch, tr2_SetVideoEncoderConfiguration_REQ *p_req)
{
	_CAMSETUP	*setup = &g_cam_setup;
	_CAMSETUP_EXT	*setup_ext = &g_cam_setup_ext;
	char resol_size[64];

	sprintf(resol_size, "%dx%d", p_req->Configuration.Resolution.Width, p_req->Configuration.Resolution.Height);
	setup->cam[ch].res = def_resol_size_to_index(0, resol_size);


	setup->cam[ch].codec = (strcasecmp(p_req->Configuration.Encoding, "H265") == 0)?1:0;
	if(p_req->Configuration.GovLengthFlag) {
		setup->cam[ch].gop = p_req->Configuration.GovLength;
	}
	
	{
		setup->cam[ch].fps = p_req->Configuration.RateControl.FrameRateLimit;
		setup->cam[ch].bitrate = def_bitrate_value_to_index(p_req->Configuration.RateControl.BitrateLimit);
		if(p_req->Configuration.RateControl.ConstantBitRateFlag) {
			setup->cam[ch].ratectrl = (p_req->Configuration.RateControl.ConstantBitRate)?RATECTRL_CBR:RATECTRL_VBR;
		}

	}

	setup_save(&DEV_MTD, setup, setup_ext);
	
	
	


	send_msg(ONVIF, MAIN, _CAMMSG_CGI, 1, 1, 1);

	return 0;
}

int cam_setup_apply_profile(ONVIF_PROFILE *profiles)
{printf_setup();
	onvif_save_profile_cfg(ONVIF_SETUP_FILE, profiles);
	return 0;
}

#ifdef NARCH
int cam_setup_apply_imaging(img_SetImagingSettings_REQ * p_req)
{
	int iq = 0;
	int ae = 0;
	int blc = 0;
	int awb = 0;

	_CAMSETUP	*setup = &g_cam_setup;
	_CAMSETUP_EXT	*setup_ext = &g_cam_setup_ext;
	NSETUP_T *nsetup = &g_cam_setup.nsetup;

	if(p_req->ImagingSettings.BacklightCompensationFlag) {
		if(nsetup->isp.blc.blc.mode.val != ((p_req->ImagingSettings.BacklightCompensation.Mode == BacklightCompensationMode_ON)?1:0)) {
			nsetup->isp.blc.blc.mode.val = (p_req->ImagingSettings.BacklightCompensation.Mode == BacklightCompensationMode_ON)?1:0;
			blc = 1;
		}
	}

	if(p_req->ImagingSettings.BrightnessFlag) {

		if(nsetup->isp.iq.color.brightness.val != p_req->ImagingSettings.Brightness) {
			nsetup->isp.iq.color.brightness.val = p_req->ImagingSettings.Brightness;
			iq = 1;
		}
	}

	if(p_req->ImagingSettings.ColorSaturationFlag) {
		if(nsetup->isp.iq.color.saturation.val != p_req->ImagingSettings.ColorSaturation) {
			nsetup->isp.iq.color.saturation.val = p_req->ImagingSettings.ColorSaturation;
			iq = 1;
		}
	}

	if(p_req->ImagingSettings.ContrastFlag) {
		if(nsetup->isp.iq.color.contrast.val != p_req->ImagingSettings.Contrast) {
			nsetup->isp.iq.color.contrast.val = p_req->ImagingSettings.Contrast;
			iq = 1;
		}
	}


	if(p_req->ImagingSettings.IrCutFilterFlag) {
	}

	if(p_req->ImagingSettings.SharpnessFlag) {
		if(nsetup->isp.iq.sharpeness.val != p_req->ImagingSettings.Sharpness) {
			nsetup->isp.iq.sharpeness.val = p_req->ImagingSettings.Sharpness;
			iq = 1;
		}
	}

	if(p_req->ImagingSettings.WhiteBalanceFlag)	{
		if(nsetup->isp.awb.mode.val != ((p_req->ImagingSettings.WhiteBalance.Mode == WhiteBalanceMode_MANUAL)?0:1)) {
			nsetup->isp.awb.mode.val = (p_req->ImagingSettings.WhiteBalance.Mode == WhiteBalanceMode_MANUAL)?0:1;
			awb = 1;
		}

		if(p_req->ImagingSettings.WhiteBalance.CrGainFlag) {
			if(nsetup->isp.awb.manual_gain.r_gain.val != p_req->ImagingSettings.WhiteBalance.CrGain) {
				nsetup->isp.awb.manual_gain.r_gain.val = p_req->ImagingSettings.WhiteBalance.CrGain;
				awb = 1;
			}
		}

		if(p_req->ImagingSettings.WhiteBalance.CbGainFlag)	{
			if(nsetup->isp.awb.manual_gain.b_gain.val = p_req->ImagingSettings.WhiteBalance.CbGain) {
				nsetup->isp.awb.manual_gain.b_gain.val = p_req->ImagingSettings.WhiteBalance.CbGain;
				awb = 1;
			}
		}
	}


	if(iq || ae || blc || awb) {
		setup_save(&DEV_MTD, setup, setup_ext);
	}

	if(iq) {
		send_msg(ONVIF, ENCODER, _CAMMSG_CGI, 0, 1, 0);
	}
	if(ae) {
		send_msg(ONVIF, ENCODER, _CAMMSG_CGI, 1, 1, 0);
	}
	if(blc) {
		send_msg(ONVIF, ENCODER, _CAMMSG_CGI, 2, 1, 0);
	}
	if(awb) {
		send_msg(ONVIF, ENCODER, _CAMMSG_CGI, 3, 1, 0);
	}

	return 0;
}

#else // NARCH
#endif // NARCH


