
#ifndef SETUP_EXT_H_
#define SETUP_EXT_H_

#ifdef SETUP_EXT_IMPLEMENTATION
#undef SETUP_EXT_IMPLEMENTATION

#include <sys/un.h>
#include "setup_ini.h"


const char *setup_ini_get_str(const char *section, const char *key, char *def);

SETUP_EXT_ISP default_setup_ext_isp =
{
	.mode = 0,		// 0:Auto, 1:Manual

	.wb= {
		.mode		= 0,
		.Saturation	= 127,
		.RGStrength	= 127,
		.BGStrength	= 127,
		.RGain		= 225,
		.GrGain		= 127,
		.GbGain		= 127,
		.BGain		= 225,
	},

	.bl = {
		.mode		= 0,		
		.wdr_level	= 1,
	},

	.exp = {
		.lens_mode	= 0,
		.shutter	= 0,
		.brightness	= 127,
		.gain		= 200,
	},

	.tdn = {
		.mode		= TDN_AUTO_EXP,
		.delay		= 3,
	},

	.special = {
		.dnr		= 4,		// 1~7
		.strength	= 128,
		.sharpness	= 255,
		.gamma		= 1,
	}
};

void set_isp_default(_CAMSETUP_EXT *setup_ext)
{
	
	setup_ext->isp = default_setup_ext_isp;

#ifndef USE_DF2_SUPPORT
	setup_ext->isp.tdn.mode = 0; 
#endif


	setup_ext->isp.exp.shutter_speed[0] = SS_30000;
	setup_ext->isp.exp.shutter_speed[1] = SS_30;



#if NARCH
	setup_ext->isp.wb.mode = ISP_AWB_AUTO;
	setup_ext->isp.wb.RGStrength = 100;
	setup_ext->isp.wb.RBStrength = 100;
	setup_ext->isp.wb.RGain = 256;
	setup_ext->isp.wb.GrGain = 256;
	setup_ext->isp.wb.GbGain = 256;
	setup_ext->isp.wb.BGain = 256;
#endif
}

void set_isp_default_except_color(_CAMSETUP_EXT *setup_ext)
{
	SETUP_EXT_ISP isp_bk;
	isp_bk = setup_ext->isp;

	setup_ext->isp = default_setup_ext_isp;



	setup_ext->isp.exp.brightness = isp_bk.exp.brightness;	// f:1~255
	setup_ext->isp.special.sharpness = isp_bk.special.sharpness;	// f:0~255
}


void setup_ext_osd_default(_CAMSETUP_EXT *setup_ext)
{
	
	SETUP_EXT_OSD *osd = &setup_ext->osd;
	// Title
	osd->osd_item[OSD_TITLE].enable = 0;
	osd->osd_item[OSD_TITLE].pos_x = 0;
	osd->osd_item[OSD_TITLE].pos_y = 0;
	osd->osd_item[OSD_TITLE].color = 0;
	osd->osd_item[OSD_TITLE].text[0] = '\0';
	// DateTime
	osd->osd_item[OSD_DATETIME].enable = 0;
	osd->osd_item[OSD_DATETIME].pos_x = 0;
	osd->osd_item[OSD_DATETIME].pos_y = 0;
	osd->osd_item[OSD_DATETIME].color = 0;
	osd->osd_item[OSD_DATETIME].text[0] = '\0';
	
}

void setup_ext_default (_CAMSETUP_EXT *setup_ext)
{
	// printf("setup default start \n");
	//char cmd[1024];
	int i;
	memset(setup_ext, 0, sizeof(_CAMSETUP_EXT));
	
	setup_ini_load();
	usleep(100);	// 1000);
	

	setup_ext->magic[0] = MAGIC_NUM_1;
	setup_ext->magic[1] = 0x00710225;
	setup_ext->magic[2] = 0x00951220;
	setup_ext->magic[3] = 0x00981029;
	setup_ext->sizeOfStruct = sizeof(_CAMSETUP_EXT);
	setup_ext->reserved = 0;

#ifdef USE_NEW_ONVIF
	setup_ext->onvif_port = 88;
	setup_ext->onvif_use_https = 0;
	setup_ext->onvif_use_discovery = 1;
#endif

	setup_ext->onvif_security = 1;


	setup_ext->rtsp_info.use_rtsp = 1;
	setup_ext->rtsp_info.use_security = 1;
	setup_ext->rtsp_info.use_rtsp_security_select_set = 1;
	setup_ext->rtsp_info.rtsp_port = (int)atoi(setup_ini_get_str("cam_setup", "RTSP_PORT", "554"));

	//{
	//	sprintf (cmd, "echo \"model_spec_key_val_add :line %d > rtsp_port ? %d\" | cat >> /mnt/ipm/log.txt", __LINE__, setup_ext->rtsp_info.rtsp_port);
	//	system(cmd);
	//}

	for(int i = 0; i < 3; i++) {
		sprintf(setup_ext->rtsp_uri[i], "%s%d", setup_ini_get_str("cam_setup", "RTSP_URL", "livestream"), i+1);
	}

	set_isp_default(setup_ext);

	for (i = 0; i < 3; i++) {
		setup_ext->caminfo_ext[i].Iref_mode = 0;
#ifdef USE_DF2_SUPPORT
		setup_ext->caminfo_ext[i].encryption = 0;
#else
#endif
	}
	setup_ext_osd_default(setup_ext);
}
#endif /* SETUP_EXT_IMPLEMENTATION */

#endif /* SETUP_EXT_H_ */
