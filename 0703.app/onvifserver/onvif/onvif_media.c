

#include "sys_inc.h"
#include "onvif.h"
#include "onvif_media.h"
#include "onvif_utils.h"

#if defined(MEDIA_SUPPORT) || defined(MEDIA2_SUPPORT)

/***************************************************************************************/
extern ONVIF_CFG g_onvif_cfg;
extern ONVIF_CLS g_onvif_cls;



/***************************************************************************************/

/************************************************************************************
 *
 * @desc : Get snapshot JPEG image data,
 * @param :
 *	rlen [in, out], [in] the buff size, [out] the image data size
 *
*************************************************************************************/
ONVIF_RET trt_GetSnapshot(char * buff, int * rlen, char * profile_token)
{
    // int len = 0;
    // FILE * fp;
    // ONVIF_PROFILE * p_profile;



    return ONVIF_ERR_NoProfile;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoConfig
 *	ONVIF_ERR_ConfigModify
 *
*************************************************************************************/
ONVIF_RET trt_SetOSD(trt_SetOSD_REQ * p_req)
{
	OSDConfigurationList * p_osd = onvif_find_OSDConfiguration(g_onvif_cfg.OSDs, p_req->OSD.token);
	if (NULL == p_osd)
	{
		return ONVIF_ERR_NoConfig;
	}

	// todo : here add your handler code ...


	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_MaxOSDs
 *
*************************************************************************************/
ONVIF_RET trt_CreateOSD(trt_CreateOSD_REQ * p_req)
{
    OSDConfigurationList * p_osd = onvif_add_OSDConfiguration(&g_onvif_cfg.OSDs);
	if (NULL == p_osd)
	{
		return ONVIF_ERR_MaxOSDs;
	}

	memcpy(&p_osd->OSD, &p_req->OSD, sizeof(onvif_OSDConfiguration));

	snprintf(p_osd->OSD.token, 100, "OSD_00%d", g_onvif_cls.osd_idx++);

    // return the token
    strcpy(p_req->OSD.token, p_osd->OSD.token);

    // todo : here add your handler code ...


	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoConfig
 *
*************************************************************************************/
ONVIF_RET trt_DeleteOSD(trt_DeleteOSD_REQ * p_req)
{
    OSDConfigurationList * p_prev;
	OSDConfigurationList * p_osd = onvif_find_OSDConfiguration(g_onvif_cfg.OSDs, p_req->OSDToken);
	if (NULL == p_osd)
	{
		return ONVIF_ERR_NoConfig;
	}

	// todo : here add your handler code ...


	p_prev = g_onvif_cfg.OSDs;
	if (p_osd == p_prev)
	{
		g_onvif_cfg.OSDs = p_osd->next;
	}
	else
	{
		while (p_prev->next)
		{
			if (p_prev->next == p_osd)
			{
				break;
			}

			p_prev = p_prev->next;
		}

		p_prev->next = p_osd->next;
	}

	free(p_osd);

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 *	ONVIF_ERR_DeletionOfFixedProfile
 *
*************************************************************************************/
ONVIF_RET trt_DeleteProfile(trt_DeleteProfile_REQ * p_req)
{
    ONVIF_PROFILE * p_prev;
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	if (p_profile->fixed)
	{
		return ONVIF_ERR_DeletionOfFixedProfile;
	}

	p_prev = g_onvif_cfg.profiles;
	if (p_profile == p_prev)
	{
		g_onvif_cfg.profiles = p_profile->next;
	}
	else
	{
		while (p_prev->next)
		{
			if (p_prev->next == p_profile)
			{
				break;
			}

			p_prev = p_prev->next;
		}

		p_prev->next = p_profile->next;
	}

	if (p_profile->v_src_cfg && p_profile->v_src_cfg->Configuration.UseCount > 0)
	{
		p_profile->v_src_cfg->Configuration.UseCount--;
	}

	if (p_profile->v_enc_cfg && p_profile->v_enc_cfg->Configuration.UseCount > 0)
	{
		p_profile->v_enc_cfg->Configuration.UseCount--;
	}


	if (p_profile->multicasting)
	{
		// todo : stop multicast streaming ...
	}

	cam_setup_apply_profile(g_onvif_cfg.profiles); //trt_DeleteProfile

	free(p_profile);

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_ConfigModify
 *	ONVIF_ERR_NoConfig
 * 	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
ONVIF_RET trt_SetVideoSourceConfiguration(trt_SetVideoSourceConfiguration_REQ * p_req)
{
	VideoSourceList * p_v_src;
	VideoSourceConfigurationList * p_v_src_cfg = onvif_find_VideoSourceConfiguration(g_onvif_cfg.v_src_cfg, p_req->Configuration.token);
	if (NULL == p_v_src_cfg)
	{
		return ONVIF_ERR_NoConfig;
	}

	p_v_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->Configuration.SourceToken);
	if (NULL == p_v_src)
	{
		return ONVIF_ERR_NoConfig;
	}

	if (/*p_req->Configuration.Bounds.x < g_onvif_cfg.VideoSourceConfigurationOptions.BoundsRange.XRange.Min || */
		p_req->Configuration.Bounds.x > g_onvif_cfg.VideoSourceConfigurationOptions.BoundsRange.XRange.Max ||
		/*p_req->Configuration.Bounds.y < g_onvif_cfg.VideoSourceConfigurationOptions.BoundsRange.YRange.Min || */
		p_req->Configuration.Bounds.y > g_onvif_cfg.VideoSourceConfigurationOptions.BoundsRange.YRange.Max ||
		/*p_req->Configuration.Bounds.width < g_onvif_cfg.VideoSourceConfigurationOptions.BoundsRange.WidthRange.Min || */
		p_req->Configuration.Bounds.width > g_onvif_cfg.VideoSourceConfigurationOptions.BoundsRange.WidthRange.Max ||
		/*p_req->Configuration.Bounds.height < g_onvif_cfg.VideoSourceConfigurationOptions.BoundsRange.HeightRange.Min || */
		p_req->Configuration.Bounds.height > g_onvif_cfg.VideoSourceConfigurationOptions.BoundsRange.HeightRange.Max)
	{
		return ONVIF_ERR_ConfigModify;
	}

	p_v_src_cfg->Configuration.Bounds.x = p_req->Configuration.Bounds.x;
	p_v_src_cfg->Configuration.Bounds.y = p_req->Configuration.Bounds.y;
	p_v_src_cfg->Configuration.Bounds.width = p_req->Configuration.Bounds.width;
	p_v_src_cfg->Configuration.Bounds.height = p_req->Configuration.Bounds.height;

	strcpy(p_v_src_cfg->Configuration.Name, p_req->Configuration.Name);
    strcpy(p_v_src_cfg->Configuration.SourceToken, p_req->Configuration.SourceToken);

	if (p_req->Configuration.ExtensionFlag)
	{
	    memcpy(&p_v_src_cfg->Configuration.Extension, &p_req->Configuration.Extension, sizeof(onvif_VideoSourceConfigurationExtension));
	}

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoConfig
 * 	ONVIF_ERR_ConfigModify
 * 	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
ONVIF_RET trt_SetMetadataConfiguration(trt_SetMetadataConfiguration_REQ * p_req)
{
	MetadataConfigurationList * p_cfg = onvif_find_MetadataConfiguration(g_onvif_cfg.metadata_cfg, p_req->Configuration.token);
	if (NULL == p_cfg)
	{
		return ONVIF_ERR_NoConfig;
	}

	if (p_req->Configuration.SessionTimeout <= 0)
	{
		return ONVIF_ERR_ConfigModify;
	}

    strcpy(p_cfg->Configuration.Name, p_req->Configuration.Name);
    p_cfg->Configuration.SessionTimeout = p_req->Configuration.SessionTimeout;

    p_cfg->Configuration.AnalyticsFlag = p_req->Configuration.AnalyticsFlag;
    p_cfg->Configuration.Analytics = p_req->Configuration.Analytics;

    p_cfg->Configuration.PTZStatusFlag = p_req->Configuration.PTZStatusFlag;
    memcpy(&p_cfg->Configuration.PTZStatus, &p_req->Configuration.PTZStatus, sizeof(onvif_PTZFilter));

    p_cfg->Configuration.EventsFlag = p_req->Configuration.EventsFlag;
    memcpy(&p_cfg->Configuration.Events, &p_req->Configuration.Events, sizeof(onvif_EventSubscription));

    memcpy(&p_cfg->Configuration.Multicast, &p_req->Configuration.Multicast, sizeof(onvif_MulticastConfiguration));

	return ONVIF_OK;
}


#endif // end of defined(MEDIA_SUPPORT) || defined(MEDIA2_SUPPORT)

#ifdef MEDIA_SUPPORT

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_ProfileExists
 *	ONVIF_ERR_MaxNVTProfiles
 *
*************************************************************************************/
ONVIF_RET trt_CreateProfile(const char * lip, uint32 lport, trt_CreateProfile_REQ * p_req)
{
	ONVIF_PROFILE * p_profile = NULL;

	if (p_req->TokenFlag && p_req->Token[0] != '\0')
	{
		p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->Token);
		if (p_profile)
		{
			return ONVIF_ERR_ProfileExists;
		}
	}

	p_profile = onvif_add_profile(FALSE);
	if (p_profile)
	{
		strcpy(p_profile->name, p_req->Name);
		if (p_req->TokenFlag && p_req->Token[0] != '\0')
		{
			strcpy(p_profile->token, p_req->Token);
		}
		else
		{
			strcpy(p_req->Token, p_profile->token);
		}
	}
	else
	{
		return ONVIF_ERR_MaxNVTProfiles;
	}
	onvif_bprint("trt_CreateProfile  Token : %s \n",p_req->Token);
	cam_setup_apply_profile(g_onvif_cfg.profiles); //trt_CreateProfile

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 *	ONVIF_ERR_NoConfig
 *	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
ONVIF_RET trt_AddVideoSourceConfiguration(trt_AddVideoSourceConfiguration_REQ * p_req)
{
    VideoSourceConfigurationList * p_v_src_cfg;
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	p_v_src_cfg = onvif_find_VideoSourceConfiguration(g_onvif_cfg.v_src_cfg, p_req->ConfigurationToken);
	if (NULL == p_v_src_cfg)
	{
		return ONVIF_ERR_NoConfig;
	}

	if (p_profile->v_src_cfg != p_v_src_cfg)
	{
		if (p_profile->v_src_cfg && p_profile->v_src_cfg->Configuration.UseCount > 0)
		{
			p_profile->v_src_cfg->Configuration.UseCount--;
		}

		p_v_src_cfg->Configuration.UseCount++;
		p_profile->v_src_cfg = p_v_src_cfg;
	}

	cam_setup_apply_profile(g_onvif_cfg.profiles); // trt_AddVideoSourceConfiguration

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 *	ONVIF_ERR_NoConfig
 *	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
ONVIF_RET trt_AddVideoEncoderConfiguration(trt_AddVideoEncoderConfiguration_REQ * p_req)
{

    VideoEncoder2ConfigurationList * p_v_enc_cfg;
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	p_v_enc_cfg = onvif_find_VideoEncoder2Configuration(g_onvif_cfg.v_enc_cfg, p_req->ConfigurationToken);
	if (NULL == p_v_enc_cfg)
	{
		return ONVIF_ERR_NoConfig;
	}

	if (p_profile->v_enc_cfg != p_v_enc_cfg)
	{
		if (p_profile->v_enc_cfg && p_profile->v_enc_cfg->Configuration.UseCount > 0)
		{
			p_profile->v_enc_cfg->Configuration.UseCount--;
		}

		p_v_enc_cfg->Configuration.UseCount++;
		p_profile->v_enc_cfg = p_v_enc_cfg;
		onvifserver_get_rtsp_uri(p_profile->stream_uri, video_encoder_token_to_ch(p_req->ConfigurationToken));

	}

	cam_setup_apply_profile(g_onvif_cfg.profiles); //trt_AddVideoEncoderConfiguration

	return ONVIF_OK;
}


/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 *	ONVIF_ERR_NoConfig
 *	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
ONVIF_RET trt_RemoveVideoEncoderConfiguration(trt_RemoveVideoEncoderConfiguration_REQ * p_req)
{
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	if (p_profile->v_enc_cfg && p_profile->v_enc_cfg->Configuration.UseCount > 0)
	{
		p_profile->v_enc_cfg->Configuration.UseCount--;
	}

	cam_setup_apply_profile(g_onvif_cfg.profiles); // trt_RemoveVideoEncoderConfiguration

	p_profile->v_enc_cfg = NULL;

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 *	ONVIF_ERR_NoConfig
 *	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
ONVIF_RET trt_RemoveVideoSourceConfiguration(trt_RemoveVideoSourceConfiguration_REQ * p_req)
{
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	if (p_profile->v_src_cfg && p_profile->v_src_cfg->Configuration.UseCount > 0)
	{
		p_profile->v_src_cfg->Configuration.UseCount--;
	}

	cam_setup_apply_profile(g_onvif_cfg.profiles); // trt_RemoveVideoSourceConfiguration

	p_profile->v_src_cfg = NULL;

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_ConfigModify
 *	ONVIF_ERR_NoConfig
 * 	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

ONVIF_RET trt_SetVideoEncoderConfiguration(trt_SetVideoEncoderConfiguration_REQ * p_req)
{
	ONVIF_RET ret;
    int i = 0;
	int ch = 0;
    onvif_VideoResolution * p_VideoResolution;
	VideoEncoder2ConfigurationList * p_v_enc_cfg;

	p_v_enc_cfg = onvif_find_VideoEncoder2Configuration(g_onvif_cfg.v_enc_cfg, p_req->Configuration.token);
	if (NULL == p_v_enc_cfg)
	{
		return ONVIF_ERR_NoConfig;
	}
	ch = video_encoder_token_to_ch(p_req->Configuration.token);



	if (VideoEncoding_H264 == p_req->Configuration.Encoding)
	{
		if (p_req->Configuration.H264.GovLength < 1 ||
			p_req->Configuration.H264.GovLength > 60)
		{
			return ONVIF_ERR_ConfigModify;
		}

		p_VideoResolution = g_onvif_cfg.VideoEncoderChConfigurationOptions[ch].H264.ResolutionsAvailable;
	}


	ret = cam_setup_apply_video_encoder(ch, p_req);
	if(ret != ONVIF_OK) {
		return ret;
	}

	p_v_enc_cfg->Configuration.Resolution.Width = p_req->Configuration.Resolution.Width;
	p_v_enc_cfg->Configuration.Resolution.Height = p_req->Configuration.Resolution.Height;
	p_v_enc_cfg->Configuration.SessionTimeout = p_req->Configuration.SessionTimeout;
	p_v_enc_cfg->Configuration.VideoEncoding = p_req->Configuration.Encoding;

	if (VideoEncoding_H264 == p_req->Configuration.Encoding)
	{
	    strcpy(p_v_enc_cfg->Configuration.Encoding, "H264");
		p_v_enc_cfg->Configuration.GovLength = p_req->Configuration.H264.GovLength;
		strcpy(p_v_enc_cfg->Configuration.Profile, onvif_H264ProfileToString(p_req->Configuration.H264.H264Profile));
	}
	{
		p_v_enc_cfg->Configuration.RateControl.FrameRateLimit = (float)p_req->Configuration.RateControl.FrameRateLimit;
		p_v_enc_cfg->Configuration.RateControl.EncodingInterval = p_req->Configuration.RateControl.EncodingInterval;
			p_v_enc_cfg->Configuration.RateControl.BitrateLimit = p_req->Configuration.RateControl.BitrateLimit;
	}

	memcpy(&p_v_enc_cfg->Configuration.Multicast, &p_req->Configuration.Multicast, sizeof(onvif_MulticastConfiguration));

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 *	ONVIF_ERR_NoConfig
 *
*************************************************************************************/
ONVIF_RET trt_GetVideoEncoderConfigurationOptions(trt_GetVideoEncoderConfigurationOptions_REQ * p_req, trt_GetVideoEncoderConfigurationOptions_RES * p_res)
{
    ONVIF_PROFILE * p_profile = NULL;
    VideoEncoder2ConfigurationList * p_v_enc_cfg = NULL;
	int ch = 0;

    if (p_req->ProfileTokenFlag && p_req->ProfileToken[0] != '\0')
	{
		p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
		if (NULL == p_profile)
		{
			return ONVIF_ERR_NoProfile;
		}
		ch = video_encoder_token_to_ch(p_req->ProfileToken);
	}

	if (p_req->ConfigurationTokenFlag && p_req->ConfigurationToken[0] != '\0')
	{
		p_v_enc_cfg = onvif_find_VideoEncoder2Configuration(g_onvif_cfg.v_enc_cfg, p_req->ConfigurationToken);
		if (NULL == p_v_enc_cfg)
		{
			return ONVIF_ERR_NoConfig;
		}
		ch = video_encoder_token_to_ch(p_req->ConfigurationToken);
	}

#if 1
    memcpy(p_res, &g_onvif_cfg.VideoEncoderChConfigurationOptions[ch], sizeof(onvif_VideoEncoderConfigurationOptions));
#else
#endif

    return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 *	ONVIF_ERR_InvalidStreamSetup
 *	ONVIF_ERR_StreamConflict
 *	ONVIF_ERR_IncompleteConfiguration
 *	ONVIF_ERR_InvalidMulticastSettings
 *
*************************************************************************************/
ONVIF_RET trt_GetStreamUri(const char * lip, uint32 lport, trt_GetStreamUri_REQ * p_req, trt_GetStreamUri_RES * p_res)
{
    ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
    if (NULL == p_profile)
    {
        return ONVIF_ERR_NoProfile;
    }

    // set the media uri
	if (p_profile->stream_uri[0] == '\0')
	{
        return ONVIF_ERR_NoProfile;
	}
	else {
	    int offset = 0;
	    int len = sizeof(p_res->MediaUri.Uri);

	    if (p_req->StreamSetup.Transport.Protocol == TransportProtocol_HTTP)
	    {
	        offset += sprintf(p_res->MediaUri.Uri, "http://%s/%s", lip, p_profile->stream_uri);
	    }
	    else
	    {
			offset += sprintf(p_res->MediaUri.Uri, "rtsp://%s/%s", lip, p_profile->stream_uri);
        }

	    if (StreamType_RTP_Unicast == p_req->StreamSetup.Stream)
	    {
	        offset += snprintf(p_res->MediaUri.Uri+offset, len-offset, "&amp;t=%s", "unicast");
	    }
	    else if (StreamType_RTP_Multicast == p_req->StreamSetup.Stream)
	    {
	        offset += snprintf(p_res->MediaUri.Uri+offset, len-offset, "&amp;t=%s", "multicase");
	    }

	    if (TransportProtocol_TCP == p_req->StreamSetup.Transport.Protocol)
	    {
	        offset += snprintf(p_res->MediaUri.Uri+offset, len-offset, "&amp;p=%s", "tcp");
	    }
	    else if (TransportProtocol_RTSP == p_req->StreamSetup.Transport.Protocol)
	    {
	        offset += snprintf(p_res->MediaUri.Uri+offset, len-offset, "&amp;p=%s", "rtsp");
	    }
	    else if (TransportProtocol_HTTP == p_req->StreamSetup.Transport.Protocol)
	    {
	        offset += snprintf(p_res->MediaUri.Uri+offset, len-offset, "&amp;p=%s", "http");
	    }

        if (p_profile->v_enc_cfg)
        {
	        offset += snprintf(p_res->MediaUri.Uri+offset, len-offset, "&amp;ve=%s&amp;w=%d&amp;h=%d",
	            p_profile->v_enc_cfg->Configuration.Encoding,
	            p_profile->v_enc_cfg->Configuration.Resolution.Width,
	            p_profile->v_enc_cfg->Configuration.Resolution.Height);

	    }

	}

    p_res->MediaUri.InvalidAfterConnect = FALSE;
    p_res->MediaUri.InvalidAfterReboot = FALSE;
    p_res->MediaUri.Timeout = 60;

    return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 * 	ONVIF_ERR_IncompleteConfiguration
 *
*************************************************************************************/
ONVIF_RET trt_StartMulticastStreaming(const char * token)
{
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, token);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	// todo : start multicast streaming ...

	p_profile->multicasting = TRUE;

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 * 	ONVIF_ERR_IncompleteConfiguration
 *
*************************************************************************************/
ONVIF_RET trt_StopMulticastStreaming(const char * token)
{
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, token);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	// todo : stop multicast streaming ...

	p_profile->multicasting = FALSE;

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 * 	ONVIF_ERR_NoConfig
 * 	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
ONVIF_RET trt_AddMetadataConfiguration(trt_AddMetadataConfiguration_REQ * p_req)
{
	MetadataConfigurationList * p_metadata_cfg;
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	p_metadata_cfg = onvif_find_MetadataConfiguration(g_onvif_cfg.metadata_cfg, p_req->ConfigurationToken);
	if (NULL == p_metadata_cfg)
	{
		return ONVIF_ERR_NoConfig;
	}

    if (p_profile->metadata_cfg != p_metadata_cfg)
	{
		if (p_profile->metadata_cfg && p_profile->metadata_cfg->Configuration.UseCount > 0)
		{
			p_profile->metadata_cfg->Configuration.UseCount--;
		}

		p_metadata_cfg->Configuration.UseCount++;
		p_profile->metadata_cfg = p_metadata_cfg;
	}

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 * 	ONVIF_ERR_NoConfig
 * 	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
ONVIF_RET trt_RemoveMetadataConfiguration(const char * profile_token)
{
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, profile_token);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

    if (p_profile->metadata_cfg && p_profile->metadata_cfg->Configuration.UseCount > 0)
	{
		p_profile->metadata_cfg->Configuration.UseCount--;
	}

	p_profile->metadata_cfg = NULL;

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoVideoSource
 * 	ONVIF_ERR_NoVideoSourceMode
 *
*************************************************************************************/
ONVIF_RET trt_SetVideoSourceMode(trt_SetVideoSourceMode_REQ * p_req, trt_SetVideoSourceMode_RES * p_res)
{
    VideoSourceList * p_v_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSourceToken);
    if (NULL == p_v_src)
    {
        return ONVIF_ERR_NoVideoSource;
    }

    if (strcmp(p_v_src->VideoSourceMode.token, p_req->VideoSourceModeToken))
    {
        return ONVIF_ERR_NoVideoSourceMode;
    }

    // todo : handler set video source mode ...


    return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 * 	ONVIF_ERR_IncompleteConfiguration
 *
*************************************************************************************/
ONVIF_RET trt_GetSnapshotUri(const char * lip, uint32 lport, trt_GetSnapshotUri_REQ * p_req, trt_GetSnapshotUri_RES * p_res)
{
    ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
    if (NULL == p_profile)
    {
        return ONVIF_ERR_NoProfile;
    }

    // set the media uri

    sprintf(p_res->MediaUri.Uri, "%s://%s:%d/cgi-bin/wap_capture.cgi?%s",
        g_onvif_cfg.https_enable ? "https" : "http", lip, lport, p_profile->token);

    p_res->MediaUri.InvalidAfterConnect = FALSE;
    p_res->MediaUri.InvalidAfterReboot = FALSE;
    p_res->MediaUri.Timeout = 60;

    return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 *
*************************************************************************************/
ONVIF_RET trt_SetSynchronizationPoint(trt_SetSynchronizationPoint_REQ * p_req)
{
    ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
    if (NULL == p_profile)
    {
        return ONVIF_ERR_NoProfile;
    }

    // todo : add your handler code ...

    return ONVIF_OK;
}

#ifdef VIDEO_ANALYTICS

/**
 The possible return value
 	ONVIF_ERR_NoProfile
 	ONVIF_ERR_NoConfig
*/
ONVIF_RET trt_AddVideoAnalyticsConfiguration(trt_AddVideoAnalyticsConfiguration_REQ * p_req)
{
	VideoAnalyticsConfigurationList * p_va_cfg;
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	p_va_cfg = onvif_find_VideoAnalyticsConfiguration(g_onvif_cfg.va_cfg, p_req->ConfigurationToken);
	if (NULL == p_va_cfg)
	{
		return ONVIF_ERR_NoConfig;
	}

	if (p_profile->va_cfg != p_va_cfg)
	{
		if (p_profile->va_cfg && p_profile->va_cfg->Configuration.UseCount > 0)
		{
			p_profile->va_cfg->Configuration.UseCount--;
		}

		p_va_cfg->Configuration.UseCount++;
		p_profile->va_cfg = p_va_cfg;
	}

	// todo : add video analytics configuration code ...

	return ONVIF_OK;
}

/**
 The possible return value
 	ONVIF_ERR_NoProfile
*/
ONVIF_RET trt_RemoveVideoAnalyticsConfiguration(trt_RemoveVideoAnalyticsConfiguration_REQ * p_req)
{
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	if (p_profile->va_cfg && p_profile->va_cfg->Configuration.UseCount > 0)
	{
		p_profile->va_cfg->Configuration.UseCount--;
	}

	p_profile->va_cfg = NULL;

	// todo : remove video analytics configuration code ...

	return ONVIF_OK;
}

/**
 The possible return value
 	ONVIF_ERR_NoConfig
 	ONVIF_ERR_ConfigModify
 	ONVIF_ERR_ConfigurationConflict
*/
ONVIF_RET trt_SetVideoAnalyticsConfiguration(trt_SetVideoAnalyticsConfiguration_REQ * p_req)
{
	VideoAnalyticsConfigurationList * p_va_cfg;

	p_va_cfg = onvif_find_VideoAnalyticsConfiguration(g_onvif_cfg.va_cfg, p_req->Configuration.token);
	if (NULL == p_va_cfg)
	{
		return ONVIF_ERR_NoConfig;
	}

	// check the configuration parameters ...

	// save the video analytics configuration
	strcpy(p_va_cfg->Configuration.Name, p_req->Configuration.Name);

	onvif_free_Configs(&p_va_cfg->Configuration.AnalyticsEngineConfiguration.AnalyticsModule);
	onvif_free_Configs(&p_va_cfg->Configuration.RuleEngineConfiguration.Rule);

	p_va_cfg->Configuration.AnalyticsEngineConfiguration.AnalyticsModule = p_req->Configuration.AnalyticsEngineConfiguration.AnalyticsModule;
	p_va_cfg->Configuration.RuleEngineConfiguration.Rule = p_req->Configuration.RuleEngineConfiguration.Rule;

	// todo : set video analytics configuration code ...


	return ONVIF_OK;
}

#endif // VIDEO_ANALYTICS

#ifdef DEVICEIO_SUPPORT

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 * 	ONVIF_ERR_NoConfig
 * 	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
ONVIF_RET trt_AddAudioOutputConfiguration(trt_AddAudioOutputConfiguration_REQ * p_req)
{
    AudioOutputConfigurationList * p_a_output_cfg;
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	p_a_output_cfg = onvif_find_AudioOutputConfiguration(g_onvif_cfg.a_output_cfg, p_req->ConfigurationToken);
	if (NULL == p_a_output_cfg)
	{
		return ONVIF_ERR_NoConfig;
	}

	if (p_profile->a_output_cfg != p_a_output_cfg)
	{
		if (p_profile->a_output_cfg && p_profile->a_output_cfg->Configuration.UseCount > 0)
		{
			p_profile->a_output_cfg->Configuration.UseCount--;
		}

		p_a_output_cfg->Configuration.UseCount++;
		p_profile->a_output_cfg = p_a_output_cfg;
	}

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 * 	ONVIF_ERR_NoConfig
 * 	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
ONVIF_RET trt_RemoveAudioOutputConfiguration(trt_RemoveAudioOutputConfiguration_REQ * p_req)
{
    ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	if (p_profile->a_output_cfg && p_profile->a_output_cfg->Configuration.UseCount > 0)
	{
		p_profile->a_output_cfg->Configuration.UseCount--;
	}

	p_profile->a_output_cfg = NULL;

	return ONVIF_OK;
}

#endif // end of DEVICEIO_SUPPORT

#endif // end of MEDIA_SUPPORT


