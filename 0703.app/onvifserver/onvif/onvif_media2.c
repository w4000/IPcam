

#include "sys_inc.h"
#include "onvif_media2.h"
#include "onvif_media.h"
#include "onvif_event.h"

#ifdef MEDIA2_SUPPORT

/************************************************************************************/
extern ONVIF_CLS g_onvif_cls;
extern ONVIF_CFG g_onvif_cfg;

/***********************************************************************************/

void onvif_MediaProfileChangedNotify(ONVIF_PROFILE * p_profile)
{
    NotificationMessageList * p_message = onvif_init_NotificationMessage3(
        "tns1:Media/ProfileChanged", PropertyOperation_Changed,
        "Token", p_profile->token, NULL, NULL,
        NULL, NULL, NULL, NULL);
    if (p_message)
    {
        onvif_put_NotificationMessage(p_message);
    }
}

void onvif_MediaConfigurationChangedNotify(const char * token, const char * type)	
{
    NotificationMessageList * p_message = onvif_init_NotificationMessage3(
        "tns1:Media/ConfigurationChanged", PropertyOperation_Changed,
        "Token", token, "Type", type,
        NULL, NULL, NULL, NULL);
    if (p_message)
    {
        onvif_put_NotificationMessage(p_message);
    }
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoConfig
 *	ONVIF_ERR_ConfigModify
 *	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
ONVIF_RET _SetVideoEncoderConfiguration(tr2_SetVideoEncoderConfiguration_REQ * p_req)
{
	ONVIF_RET ret;
	int i = 0;
	int ch = 0;
	VideoEncoder2ConfigurationList * p_v_enc_cfg;
    VideoEncoder2ConfigurationOptionsList * p_option;

	p_v_enc_cfg = onvif_find_VideoEncoder2Configuration(g_onvif_cfg.v_enc_cfg, p_req->Configuration.token);
	if (NULL == p_v_enc_cfg)
	{
		return ONVIF_ERR_NoConfig;
	}
	ch = video_encoder_token_to_ch(p_req->Configuration.token);

    p_option = onvif_find_VideoEncoder2ConfigurationOptions(g_onvif_cfg.v_enc_ch_cfg_opt[ch], p_req->Configuration.Encoding);
    if (p_option)
    {

    	if (p_req->Configuration.GovLengthFlag)
    	{
			if(p_req->Configuration.GovLength < 1 || p_req->Configuration.GovLength > 60) {
				return ONVIF_ERR_ConfigModify;
			}
    	}

    	for (i = 0; i < ARRAY_SIZE(p_option->Options.ResolutionsAvailable); i++)
    	{
    		if (p_option->Options.ResolutionsAvailable[i].Width == p_req->Configuration.Resolution.Width &&
    			p_option->Options.ResolutionsAvailable[i].Height == p_req->Configuration.Resolution.Height)
    		{
    			break;
    		}
    	}

    	if (i == ARRAY_SIZE(p_option->Options.ResolutionsAvailable))
    	{
    		return ONVIF_ERR_ConfigModify;
    	}
	}

	ret = cam_setup_apply_video_encoder2(ch, p_req);
	if(ret != ONVIF_OK) {
		return ret;
	}
	// p_req->Configuration.RateControlFlag = temp;

	p_v_enc_cfg->Configuration.Resolution.Width = p_req->Configuration.Resolution.Width;
	p_v_enc_cfg->Configuration.Resolution.Height = p_req->Configuration.Resolution.Height;
	strcpy(p_v_enc_cfg->Configuration.Encoding, p_req->Configuration.Encoding);
	strcpy(p_v_enc_cfg->Configuration.Name, p_req->Configuration.Name);


	if (strcasecmp(p_req->Configuration.Encoding, "H264") == 0)
    {
        p_v_enc_cfg->Configuration.VideoEncoding = VideoEncoding_H264;
    }
    else if (strcasecmp(p_req->Configuration.Encoding, "H265") == 0)
    {
        p_v_enc_cfg->Configuration.VideoEncoding = VideoEncoding_H265;
    }

    if (p_req->Configuration.GovLengthFlag)
    {
	    p_v_enc_cfg->Configuration.GovLength = p_req->Configuration.GovLength;
	}

	{
	    p_v_enc_cfg->Configuration.GovLengthFlag = 1;
	}

	if (p_req->Configuration.ProfileFlag)
	{
	    strcpy(p_v_enc_cfg->Configuration.Profile, p_req->Configuration.Profile);
    }


	// if (p_req->Configuration.RateControlFlag)
	{
		p_v_enc_cfg->Configuration.RateControl.FrameRateLimit = p_req->Configuration.RateControl.FrameRateLimit;
		p_v_enc_cfg->Configuration.RateControl.BitrateLimit = p_req->Configuration.RateControl.BitrateLimit;

        if (p_req->Configuration.RateControl.ConstantBitRateFlag)
        {
		    p_v_enc_cfg->Configuration.RateControl.ConstantBitRate = p_req->Configuration.RateControl.ConstantBitRate;
		}
	}


	if (p_req->Configuration.MulticastFlag)
	{
		memcpy(&p_v_enc_cfg->Configuration.Multicast, &p_req->Configuration.Multicast, sizeof(onvif_MulticastConfiguration));
	}

    onvif_MediaConfigurationChangedNotify(p_req->Configuration.token, "VideoEncoder");

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 * 	ONVIF_ERR_NoConfig
 *	ONVIF_ERR_IncompatibleConfiguration
 *
*************************************************************************************/
ONVIF_RET _GetVideoEncoderConfigurationOptions(tr2_GetVideoEncoderConfigurationOptions_REQ * p_req, tr2_GetVideoEncoderConfigurationOptions_RES * p_res)
{
    ONVIF_PROFILE * p_profile = NULL;
    VideoEncoder2ConfigurationList * p_v_enc_cfg = NULL;
    VideoEncoder2ConfigurationOptionsList * p_option;
	int ch = 0;

    if (p_req->GetConfiguration.ConfigurationTokenFlag)
    {
        p_v_enc_cfg = onvif_find_VideoEncoder2Configuration(g_onvif_cfg.v_enc_cfg, p_req->GetConfiguration.ConfigurationToken);
        if (NULL == p_v_enc_cfg)
        {
            return ONVIF_ERR_NoConfig;
        }
		ch = video_encoder_token_to_ch(p_req->GetConfiguration.ConfigurationToken);
    }

    if (p_req->GetConfiguration.ProfileTokenFlag)
    {
        p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->GetConfiguration.ProfileToken);
        if (NULL == p_profile)
        {
            return ONVIF_ERR_NoProfile;
        }
		ch = video_encoder_token_to_ch(p_req->GetConfiguration.ProfileToken);
    }

#if 1
	p_option = g_onvif_cfg.v_enc_ch_cfg_opt[ch];

	while (p_option)
	{
	    VideoEncoder2ConfigurationOptionsList * p_item = onvif_add_VideoEncoder2ConfigurationOptions(&p_res->Options);
        if (p_item)
        {
			p_res->Options->ch = ch;
            memcpy(&p_item->Options, &p_option->Options, sizeof(onvif_VideoEncoder2Configuration));
        }

	    p_option = p_option->next;
	}

#else
#endif
	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoConfig
 *	ONVIF_ERR_MaxNVTProfiles
 *	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
#include "onvif_utils.h"
ONVIF_RET _CreateProfile(const char * lip, uint32 lport, tr2_CreateProfile_REQ * p_req, tr2_CreateProfile_RES * p_res)
{
	onvif_bprint("_CreateProfile\r\n");

    int i;
	ONVIF_PROFILE * p_profile = NULL;

	p_profile = onvif_add_profile(FALSE);
	if (p_profile)
	{
		strcpy(p_profile->name, p_req->Name);
		strcpy(p_res->Token, p_profile->token);

		// add configuration to the new profile
		for (i = 0; i < p_req->sizeConfiguration; i++)
		{
			if (strcasecmp(p_req->Configuration[i].Type, "all") == 0)
			{
			    ONVIF_PROFILE * p_refer = NULL;
    			if (p_req->Configuration[i].TokenFlag)
    			{
    				p_refer = onvif_find_profile(g_onvif_cfg.profiles, p_req->Configuration[i].Token);
    			}
    			else
    			{
    				p_refer = g_onvif_cfg.profiles;
    			}

    			if (NULL == p_refer)
    			{
    				continue;
    			}

				p_profile->v_src_cfg = p_refer->v_src_cfg;
				if (p_profile->v_src_cfg)
				{
					p_profile->v_src_cfg->Configuration.UseCount++;
				}

				p_profile->v_enc_cfg = p_refer->v_enc_cfg;
				if (p_profile->v_enc_cfg)
				{
					p_profile->v_enc_cfg->Configuration.UseCount++;
				}

				p_profile->metadata_cfg = p_refer->metadata_cfg;
				if (p_profile->metadata_cfg)
				{
					p_profile->metadata_cfg->Configuration.UseCount++;
				}

#ifdef DEVICEIO_SUPPORT
				p_profile->a_output_cfg = p_refer->a_output_cfg;
				if (p_profile->a_output_cfg)
				{
					p_profile->a_output_cfg->Configuration.UseCount++;
				}
#endif

#ifdef VIDEO_ANALYTICS
				p_profile->va_cfg = p_refer->va_cfg;
				if (p_profile->va_cfg)
				{
					p_profile->va_cfg->Configuration.UseCount++;
				}
#endif

			}
			else if (strcasecmp(p_req->Configuration[i].Type, "VideoSource") == 0)
			{
			    VideoSourceConfigurationList * p_refer = NULL;
    			if (p_req->Configuration[i].TokenFlag)
    			{
    				p_refer = onvif_find_VideoSourceConfiguration(g_onvif_cfg.v_src_cfg, p_req->Configuration[i].Token);
    			}
    			else
    			{
    				p_refer = g_onvif_cfg.v_src_cfg;
    			}

    			if (NULL == p_refer)
    			{
    				continue;
    			}

				p_profile->v_src_cfg = p_refer;
				if (p_profile->v_src_cfg)
				{
					p_profile->v_src_cfg->Configuration.UseCount++;
				}
			}
			else if (strcasecmp(p_req->Configuration[i].Type, "VideoEncoder") == 0)
			{
			    VideoEncoder2ConfigurationList * p_refer = NULL;
    			if (p_req->Configuration[i].TokenFlag)
    			{
    				p_refer = onvif_find_VideoEncoder2Configuration(g_onvif_cfg.v_enc_cfg, p_req->Configuration[i].Token);
    			}
    			else
    			{
    				p_refer = g_onvif_cfg.v_enc_cfg;
    			}

    			if (NULL == p_refer)
    			{
    				continue;
    			}

				p_profile->v_enc_cfg = p_refer;
				if (p_profile->v_enc_cfg)
				{
					p_profile->v_enc_cfg->Configuration.UseCount++;
				}
			}
			else if (strcasecmp(p_req->Configuration[i].Type, "AudioSource") == 0)
			{
			}
			else if (strcasecmp(p_req->Configuration[i].Type, "AudioEncoder") == 0)
			{
			}
			else if (strcasecmp(p_req->Configuration[i].Type, "AudioOutput") == 0)
			{
#ifdef DEVICEIO_SUPPORT
                AudioOutputConfigurationList * p_refer = NULL;
    			if (p_req->Configuration[i].TokenFlag)
    			{
    				p_refer = onvif_find_AudioOutputConfiguration(g_onvif_cfg.a_output_cfg, p_req->Configuration[i].Token);
    			}
    			else
    			{
    				p_refer = g_onvif_cfg.a_output_cfg;
    			}

    			if (NULL == p_refer)
    			{
    				continue;
    			}

				p_profile->a_output_cfg = p_refer;
				if (p_profile->a_output_cfg)
				{
					p_profile->a_output_cfg->Configuration.UseCount++;
				}
#endif
			}
			else if (strcasecmp(p_req->Configuration[i].Type, "AudioDecoder") == 0)
			{
			}
			else if (strcasecmp(p_req->Configuration[i].Type, "Metadata") == 0)
			{
			    MetadataConfigurationList * p_refer = NULL;
    			if (p_req->Configuration[i].TokenFlag)
    			{
    				p_refer = onvif_find_MetadataConfiguration(g_onvif_cfg.metadata_cfg, p_req->Configuration[i].Token);
    			}
    			else
    			{
    				p_refer = g_onvif_cfg.metadata_cfg;
    			}

    			if (NULL == p_refer)
    			{
    				continue;
    			}

				p_profile->metadata_cfg = p_refer;
				if (p_profile->metadata_cfg)
				{
					p_profile->metadata_cfg->Configuration.UseCount++;
				}
			}
			else if (strcasecmp(p_req->Configuration[i].Type, "Analytics") == 0)
			{
#ifdef VIDEO_ANALYTICS
                VideoAnalyticsConfigurationList * p_refer = NULL;
    			if (p_req->Configuration[i].TokenFlag)
    			{
    				p_refer = onvif_find_VideoAnalyticsConfiguration(g_onvif_cfg.va_cfg, p_req->Configuration[i].Token);
    			}
    			else
    			{
    				p_refer = g_onvif_cfg.va_cfg;
    			}

    			if (NULL == p_refer)
    			{
    				continue;
    			}

				p_profile->va_cfg = p_refer;
				if (p_profile->va_cfg)
				{
					p_profile->va_cfg->Configuration.UseCount++;
				}
#endif
			}
			else if (strcasecmp(p_req->Configuration[i].Type, "PTZ") == 0)
			{
			}
		}

		// setup the new profile stream uri
		if (g_onvif_cfg.profiles)
		{
			strcpy(p_profile->stream_uri, g_onvif_cfg.profiles->stream_uri);
		}
	}
	else
	{
		return ONVIF_ERR_MaxNVTProfiles;
	}


	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 *	ONVIF_ERR_DeletionOfFixedProfile
 *
*************************************************************************************/
ONVIF_RET _DeleteProfile(tr2_DeleteProfile_REQ * p_req)
{
    trt_DeleteProfile_REQ req;
    ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->Token);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

    onvif_MediaProfileChangedNotify(p_profile);

    strcpy(req.ProfileToken, p_req->Token);

	return trt_DeleteProfile(&req);
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 * 	ONVIF_ERR_NoConfig
 *	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
ONVIF_RET _AddConfiguration(tr2_AddConfiguration_REQ * p_req)
{
    int i;
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
    if (NULL == p_profile)
    {
        return ONVIF_ERR_NoProfile;
    }

    if (p_req->NameFlag)
    {
        strcpy(p_profile->name, p_req->Name);
    }

    // add configuration to the new profile
    for (i = 0; i < p_req->sizeConfiguration; i++)
    {
        if (strcasecmp(p_req->Configuration[i].Type, "all") == 0)
        {
            ONVIF_PROFILE * p_refer = NULL;
            if (p_req->Configuration[i].TokenFlag)
            {
                p_refer = onvif_find_profile(g_onvif_cfg.profiles, p_req->Configuration[i].Token);
            }
            else
            {
                p_refer = g_onvif_cfg.profiles;
            }

            if (NULL == p_refer)
            {
                continue;
            }

            p_profile->v_src_cfg = p_refer->v_src_cfg;
            if (p_profile->v_src_cfg)
            {
                p_profile->v_src_cfg->Configuration.UseCount++;
            }

            p_profile->v_enc_cfg = p_refer->v_enc_cfg;
            if (p_profile->v_enc_cfg)
            {
                p_profile->v_enc_cfg->Configuration.UseCount++;
            }

            p_profile->metadata_cfg = p_refer->metadata_cfg;
            if (p_profile->metadata_cfg)
            {
                p_profile->metadata_cfg->Configuration.UseCount++;
            }


#ifdef DEVICEIO_SUPPORT
            p_profile->a_output_cfg = p_refer->a_output_cfg;
            if (p_profile->a_output_cfg)
            {
                p_profile->a_output_cfg->Configuration.UseCount++;
            }
#endif

#ifdef VIDEO_ANALYTICS
            p_profile->va_cfg = p_refer->va_cfg;
            if (p_profile->va_cfg)
            {
                p_profile->va_cfg->Configuration.UseCount++;
            }
#endif
        }
        else if (strcasecmp(p_req->Configuration[i].Type, "VideoSource") == 0)
        {
            VideoSourceConfigurationList * p_refer = NULL;
            if (p_req->Configuration[i].TokenFlag)
            {
                p_refer = onvif_find_VideoSourceConfiguration(g_onvif_cfg.v_src_cfg, p_req->Configuration[i].Token);
            }
            else
            {
                p_refer = g_onvif_cfg.v_src_cfg;
            }

            if (NULL == p_refer)
            {
                continue;
            }

            p_profile->v_src_cfg = p_refer;
            if (p_profile->v_src_cfg)
            {
                p_profile->v_src_cfg->Configuration.UseCount++;
            }
        }
        else if (strcasecmp(p_req->Configuration[i].Type, "VideoEncoder") == 0)
        {
            VideoEncoder2ConfigurationList * p_refer = NULL;
            if (p_req->Configuration[i].TokenFlag)
            {
                p_refer = onvif_find_VideoEncoder2Configuration(g_onvif_cfg.v_enc_cfg, p_req->Configuration[i].Token);
            }
            else
            {
                p_refer = g_onvif_cfg.v_enc_cfg;
            }

            if (NULL == p_refer)
            {
                continue;
            }

            p_profile->v_enc_cfg = p_refer;
            if (p_profile->v_enc_cfg)
            {
                p_profile->v_enc_cfg->Configuration.UseCount++;
            }
        }
        else if (strcasecmp(p_req->Configuration[i].Type, "AudioSource") == 0)
        {
        }
        else if (strcasecmp(p_req->Configuration[i].Type, "AudioEncoder") == 0)
        {
        }
        else if (strcasecmp(p_req->Configuration[i].Type, "AudioOutput") == 0)
        {
#ifdef DEVICEIO_SUPPORT
            AudioOutputConfigurationList * p_refer = NULL;
            if (p_req->Configuration[i].TokenFlag)
            {
                p_refer = onvif_find_AudioOutputConfiguration(g_onvif_cfg.a_output_cfg, p_req->Configuration[i].Token);
            }
            else
            {
                p_refer = g_onvif_cfg.a_output_cfg;
            }

            if (NULL == p_refer)
            {
                continue;
            }

            p_profile->a_output_cfg = p_refer;
            if (p_profile->a_output_cfg)
            {
                p_profile->a_output_cfg->Configuration.UseCount++;
            }
#endif
        }
        else if (strcasecmp(p_req->Configuration[i].Type, "AudioDecoder") == 0)
        {
        }
        else if (strcasecmp(p_req->Configuration[i].Type, "Metadata") == 0)
        {
            MetadataConfigurationList * p_refer = NULL;
            if (p_req->Configuration[i].TokenFlag)
            {
                p_refer = onvif_find_MetadataConfiguration(g_onvif_cfg.metadata_cfg, p_req->Configuration[i].Token);
            }
            else
            {
                p_refer = g_onvif_cfg.metadata_cfg;
            }

            if (NULL == p_refer)
            {
                continue;
            }

            p_profile->metadata_cfg = p_refer;
            if (p_profile->metadata_cfg)
            {
                p_profile->metadata_cfg->Configuration.UseCount++;
            }
        }
        else if (strcasecmp(p_req->Configuration[i].Type, "Analytics") == 0)
        {
#ifdef VIDEO_ANALYTICS
            VideoAnalyticsConfigurationList * p_refer = NULL;
            if (p_req->Configuration[i].TokenFlag)
            {
            	p_refer = onvif_find_VideoAnalyticsConfiguration(g_onvif_cfg.va_cfg, p_req->Configuration[i].Token);
            }
            else
            {
            	p_refer = g_onvif_cfg.va_cfg;
            }

            if (NULL == p_refer)
            {
                continue;
            }

            p_profile->va_cfg = p_refer;
            if (p_profile->va_cfg)
            {
                p_profile->va_cfg->Configuration.UseCount++;
            }
#endif
        }
        else if (strcasecmp(p_req->Configuration[i].Type, "PTZ") == 0)
        {
        }
    }


	onvif_MediaProfileChangedNotify(p_profile);

    return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoProfile
 *	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
ONVIF_RET _RemoveConfiguration(tr2_RemoveConfiguration_REQ * p_req)
{
	int i;
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	for (i = 0; i < p_req->sizeConfiguration; i++)
	{
		if (strcasecmp(p_req->Configuration[i].Type, "all") == 0)
		{
			if (p_profile->v_src_cfg && p_profile->v_src_cfg->Configuration.UseCount > 0)
			{
				p_profile->v_src_cfg->Configuration.UseCount--;
				p_profile->v_src_cfg = NULL;
			}

			if (p_profile->v_enc_cfg && p_profile->v_enc_cfg->Configuration.UseCount > 0)
			{
				p_profile->v_enc_cfg->Configuration.UseCount--;
				p_profile->v_enc_cfg = NULL;
			}

			if (p_profile->metadata_cfg && p_profile->metadata_cfg->Configuration.UseCount > 0)
			{
				p_profile->metadata_cfg->Configuration.UseCount--;
				p_profile->metadata_cfg = NULL;
			}


#ifdef DEVICEIO_SUPPORT
			if (p_profile->a_output_cfg && p_profile->a_output_cfg->Configuration.UseCount > 0)
			{
				p_profile->a_output_cfg->Configuration.UseCount--;
				p_profile->a_output_cfg = NULL;
			}
#endif

#ifdef VIDEO_ANALYTICS
			if (p_profile->va_cfg && p_profile->va_cfg->Configuration.UseCount > 0)
			{
				p_profile->va_cfg->Configuration.UseCount--;
				p_profile->va_cfg = NULL;
			}
#endif

		}
		else if (strcasecmp(p_req->Configuration[i].Type, "VideoSource") == 0)
		{
			if (p_profile->v_src_cfg && p_profile->v_src_cfg->Configuration.UseCount > 0)
			{
				p_profile->v_src_cfg->Configuration.UseCount--;
				p_profile->v_src_cfg = NULL;
			}
		}
		else if (strcasecmp(p_req->Configuration[i].Type, "VideoEncoder") == 0)
		{
			if (p_profile->v_enc_cfg && p_profile->v_enc_cfg->Configuration.UseCount > 0)
			{
				p_profile->v_enc_cfg->Configuration.UseCount--;
				p_profile->v_enc_cfg = NULL;
			}
		}
		else if (strcasecmp(p_req->Configuration[i].Type, "AudioSource") == 0)
		{
		}
		else if (strcasecmp(p_req->Configuration[i].Type, "AudioEncoder") == 0)
		{
		}
		else if (strcasecmp(p_req->Configuration[i].Type, "AudioOutput") == 0)
		{
#ifdef DEVICEIO_SUPPORT
			if (p_profile->a_output_cfg && p_profile->a_output_cfg->Configuration.UseCount > 0)
			{
				p_profile->a_output_cfg->Configuration.UseCount--;
				p_profile->a_output_cfg = NULL;
			}
#endif
		}
		else if (strcasecmp(p_req->Configuration[i].Type, "AudioDecoder") == 0)
		{
		}
		else if (strcasecmp(p_req->Configuration[i].Type, "Metadata") == 0)
		{
			if (p_profile->metadata_cfg && p_profile->metadata_cfg->Configuration.UseCount > 0)
			{
				p_profile->metadata_cfg->Configuration.UseCount--;
				p_profile->metadata_cfg = NULL;
			}
		}
		else if (strcasecmp(p_req->Configuration[i].Type, "Analytics") == 0)
		{
#ifdef VIDEO_ANALYTICS
			if (p_profile->va_cfg && p_profile->va_cfg->Configuration.UseCount > 0)
			{
				p_profile->va_cfg->Configuration.UseCount--;
				p_profile->va_cfg = NULL;
			}
#endif
		}
		else if (strcasecmp(p_req->Configuration[i].Type, "PTZ") == 0)
		{
		}
	}


    onvif_MediaProfileChangedNotify(p_profile);

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoConfig
 *	ONVIF_ERR_ConfigModify
 *	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
ONVIF_RET _SetVideoSourceConfiguration(tr2_SetVideoSourceConfiguration_REQ * p_req)
{
    ONVIF_RET ret;
	trt_SetVideoSourceConfiguration_REQ req;

	memcpy(&req.Configuration, &p_req->Configuration, sizeof(onvif_VideoSourceConfiguration));
	req.ForcePersistence = TRUE;

	ret = trt_SetVideoSourceConfiguration(&req);

    if (ONVIF_OK == ret)
    {
	    onvif_MediaConfigurationChangedNotify(req.Configuration.token, "VideoSource");
	}

    // todo : here add your handler code ...


	return ret;
}

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoConfig
 *	ONVIF_ERR_ConfigModify
 *	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
ONVIF_RET _SetMetadataConfiguration(tr2_SetMetadataConfiguration_REQ * p_req)
{
    ONVIF_RET ret;
	trt_SetMetadataConfiguration_REQ req;

	memcpy(&req.Configuration, &p_req->Configuration, sizeof(onvif_MetadataConfiguration));
	req.ForcePersistence = TRUE;

	ret = trt_SetMetadataConfiguration(&req);

    if (ONVIF_OK == ret)
    {
	    onvif_MediaConfigurationChangedNotify(p_req->Configuration.token, "Metadata");
	}

    // todo : here add your handler code ...


	return ret;
}

/************************************************************************************
 *
 * Possible error:
 *	ONVIF_ERR_NoConfig
 *
*************************************************************************************/
ONVIF_RET _GetVideoEncoderInstances(tr2_GetVideoEncoderInstances_REQ * p_req, tr2_GetVideoEncoderInstances_RES * p_res)
{
    int total = 0;
    ONVIF_PROFILE * p_profile;
	VideoSourceConfigurationList * p_v_src = onvif_find_VideoSourceConfiguration(g_onvif_cfg.v_src_cfg, p_req->ConfigurationToken);
	if (NULL == p_v_src)
	{
		return ONVIF_ERR_NoConfig;
	}

	// todo : modify the p_res ...

    // get the stream nums
    p_profile = g_onvif_cfg.profiles;
    while (p_profile)
    {
        if (p_profile->v_src_cfg && strcmp(p_profile->v_src_cfg->Configuration.token, p_req->ConfigurationToken) == 0)
        {
            total++;
        }

        p_profile = p_profile->next;
    }

	p_res->Info.Total = total;

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 *	ONVIF_ERR_NoProfile
 *
*************************************************************************************/
ONVIF_RET _SetSynchronizationPoint(tr2_SetSynchronizationPoint_REQ * p_req)
{
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	// todo : here add your handler code ...


	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 *	ONVIF_ERR_NoVideoSource
 *	ONVIF_ERR_NoVideoSourceMode
 *
*************************************************************************************/
ONVIF_RET _SetVideoSourceMode(tr2_SetVideoSourceMode_REQ * p_req, tr2_SetVideoSourceMode_RES * p_res)
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

    // todo : here add your handler code ...


    return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 *	ONVIF_ERR_NoProfile
 *	ONVIF_ERR_IncompleteConfiguration
 *
*************************************************************************************/
ONVIF_RET _GetSnapshotUri(const char * lip, uint32 lport, tr2_GetSnapshotUri_REQ * p_req, tr2_GetSnapshotUri_RES * p_res)
{
    ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
    if (NULL == p_profile)
    {
        return ONVIF_ERR_NoProfile;
    }

    // set the media uri

    sprintf(p_res->Uri, "%s://%s:%d/cgi-bin/wap_capture.cgi?%s",
        g_onvif_cfg.https_enable ? "https" : "http", lip, lport, p_profile->token);

    return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 *	ONVIF_ERR_NoProfile
 *	ONVIF_ERR_InvalidStreamSetup
 *	ONVIF_ERR_StreamConflict
 *	ONVIF_ERR_InvalidMulticastSettings
 *	ONVIF_ERR_IncompleteConfiguration
 *
*************************************************************************************/
ONVIF_RET _GetStreamUri(const char * lip, uint32 lport, tr2_GetStreamUri_REQ * p_req, tr2_GetStreamUri_RES * p_res)
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
	    int len = sizeof(p_res->Uri);

	    if (strcasecmp(p_req->Protocol, "RtspOverHttp") == 0)
	    {
	        offset += snprintf(p_res->Uri, len, "http://%s/%s", lip, p_profile->stream_uri);
	    }
	    else
	    {
			offset += snprintf(p_res->Uri, len, "rtsp://%s/%s", lip, p_profile->stream_uri);
	    }

	    if (strcasecmp(p_req->Protocol, "RtspUnicast") == 0)
	    {
	        offset += snprintf(p_res->Uri+offset, len-offset, "&amp;t=%s", "unicast");
	        offset += snprintf(p_res->Uri+offset, len-offset, "&amp;p=%s", "udp");
	    }
	    else if (strcasecmp(p_req->Protocol, "RtspMulticast") == 0)
	    {
	        offset += snprintf(p_res->Uri+offset, len-offset, "&amp;t=%s", "multicase");
	        offset += snprintf(p_res->Uri+offset, len-offset, "&amp;p=%s", "udp");
	    }
	    else if (strcasecmp(p_req->Protocol, "RTSP") == 0)
	    {
	        offset += snprintf(p_res->Uri+offset, len-offset, "&amp;t=%s", "unicast");
	        offset += snprintf(p_res->Uri+offset, len-offset, "&amp;p=%s", "rtsp");
	    }
	    else if (strcasecmp(p_req->Protocol, "RtspOverHttp") == 0)
	    {
	        offset += snprintf(p_res->Uri+offset, len-offset, "&amp;t=%s", "unicast");
	        offset += snprintf(p_res->Uri+offset, len-offset, "&amp;p=%s", "http");
	    }

        if (p_profile->v_enc_cfg)
        {
	        offset += snprintf(p_res->Uri+offset, len-offset, "&amp;ve=%s&amp;w=%d&amp;h=%d",
	            p_profile->v_enc_cfg->Configuration.Encoding,
	            p_profile->v_enc_cfg->Configuration.Resolution.Width,
	            p_profile->v_enc_cfg->Configuration.Resolution.Height);

	    }

	}

    return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 *	ONVIF_ERR_NoConfig
 *	ONVIF_ERR_MaxMasks
 *	ONVIF_ERR_InvalidPolygon
 *
*************************************************************************************/
ONVIF_RET _CreateMask(tr2_CreateMask_REQ * p_req)
{
    MaskList * p_mask;
    VideoSourceConfigurationList * p_v_cfg = onvif_find_VideoSourceConfiguration(g_onvif_cfg.v_src_cfg, p_req->Mask.ConfigurationToken);
    if (NULL == p_v_cfg)
    {
        return ONVIF_ERR_NoConfig;
    }

	p_mask = onvif_add_Mask(&g_onvif_cfg.mask);
	if (NULL == p_mask)
	{
		return ONVIF_ERR_MaxMasks;
	}

    if (p_req->Mask.Polygon.sizePoint <= 0)
    {
        return ONVIF_ERR_InvalidPolygon;
    }

	memcpy(&p_mask->Mask, &p_req->Mask, sizeof(onvif_Mask));

	snprintf(p_mask->Mask.token, 100, "MASK_00%d", g_onvif_cls.mask_idx++);

    // return the token
    strcpy(p_req->Mask.token, p_mask->Mask.token);

    // todo : here add your handler code ...


	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 *	ONVIF_ERR_NoConfig
 *
*************************************************************************************/
ONVIF_RET _DeleteMask(tr2_DeleteMask_REQ * p_req)
{
    MaskList * p_prev;
	MaskList * p_mask = onvif_find_Mask(g_onvif_cfg.mask, p_req->Token);
	if (NULL == p_mask)
	{
		return ONVIF_ERR_NoConfig;
	}

	// todo : here add your handler code ...


	p_prev = g_onvif_cfg.mask;
	if (p_mask == p_prev)
	{
		g_onvif_cfg.mask = p_mask->next;
	}
	else
	{
		while (p_prev->next)
		{
			if (p_prev->next == p_mask)
			{
				break;
			}

			p_prev = p_prev->next;
		}

		p_prev->next = p_mask->next;
	}

	free(p_mask);

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 *	ONVIF_ERR_NoConfig
 *	ONVIF_ERR_ConfigModify
 *	ONVIF_ERR_InvalidPolygon
 *
*************************************************************************************/
ONVIF_RET _SetMask(tr2_SetMask_REQ * p_req)
{
    MaskList * p_mask = onvif_find_Mask(g_onvif_cfg.mask, p_req->Mask.token);
	if (NULL == p_mask)
	{
		return ONVIF_ERR_NoConfig;
	}

    if (p_req->Mask.ConfigurationToken[0] == '\0' || p_req->Mask.Polygon.sizePoint <= 0)
    {
        return ONVIF_ERR_InvalidArgVal;
    }

	// todo : here add your handler code ...


	memcpy(&p_mask->Mask, &p_req->Mask, sizeof(onvif_Mask));

	return ONVIF_OK;
}

/************************************************************************************
 *
 * Possible error:
 *	ONVIF_ERR_NoProfile
 *	ONVIF_ERR_IncompleteConfiguration
 *
*************************************************************************************/
ONVIF_RET _StartMulticastStreaming(tr2_StartMulticastStreaming_REQ * p_req)
{
    ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
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
 *	ONVIF_ERR_NoProfile
 *
*************************************************************************************/
ONVIF_RET _StopMulticastStreaming(tr2_StopMulticastStreaming_REQ * p_req)
{
    ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	// todo : stop multicast streaming ...

	p_profile->multicasting = FALSE;

	return ONVIF_OK;
}

#ifdef DEVICEIO_SUPPORT

/************************************************************************************
 *
 * Possible error:
 * 	ONVIF_ERR_NoConfig
 *	ONVIF_ERR_ConfigModify
 *	ONVIF_ERR_ConfigurationConflict
 *
*************************************************************************************/
ONVIF_RET _SetAudioOutputConfiguration(tr2_SetAudioOutputConfiguration_REQ * p_req)
{
    AudioOutputList * p_output;
    AudioOutputConfigurationList * p_cfg = onvif_find_AudioOutputConfiguration(g_onvif_cfg.a_output_cfg, p_req->Configuration.token);
    if (NULL == p_cfg)
    {
        return ONVIF_ERR_NoAudioOutput;
    }

    p_output = onvif_find_AudioOutput(g_onvif_cfg.a_output, p_req->Configuration.OutputToken);
    if (NULL == p_output)
    {
        return ONVIF_ERR_NoAudioOutput;
    }

    if (p_req->Configuration.OutputLevel < p_cfg->Options.OutputLevelRange.Min ||
        p_req->Configuration.OutputLevel > p_cfg->Options.OutputLevelRange.Max)
    {
        return ONVIF_ERR_ConfigModify;
    }

    // todo : here add your handler code ...


    strcpy(p_cfg->Configuration.Name, p_req->Configuration.Name);
    strcpy(p_cfg->Configuration.OutputToken, p_req->Configuration.OutputToken);
    if (p_req->Configuration.SendPrimacyFlag)
    {
        strcpy(p_cfg->Configuration.SendPrimacy, p_req->Configuration.SendPrimacy);
    }
    p_cfg->Configuration.OutputLevel = p_req->Configuration.OutputLevel;

    onvif_MediaConfigurationChangedNotify(p_req->Configuration.token, "AudioOutput");

    return ONVIF_OK;
}

#endif // end of DEVICEIO_SUPPORT

#endif // MEDIA2_SUPPORT



