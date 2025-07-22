#include "nsetup.h"
#include "oem_tool.h"
#include "oem_xml_define.h"

#ifdef NARCH

static void nsetup_set_range_val(NSETUP_RANGE_VALUE *range_value, int min, int max, int init_val)
{
	range_value->min = min;
	range_value->max = max;
	range_value->init_val = init_val;
}

static void nsetup_set_range_val_default(NSETUP_RANGE_VALUE *range_value)
{
	range_value->val = range_value->init_val;
}

static void nsetup_set_list_val_default(NSETUP_LIST_VALUE *list)
{
	list->val = list->init_val;
}

static void nsetup_init_isp_onoff_val(NSETUP_LIST_VALUE *list, int on)
{
	int i = 0;
	char valstr[64];
	list->init_val = on;

	const char disp_val[3][DISP_VAL_MAX] = {
		"OFF", "ON", "\0"
	};

	for(i = 0; i < 2; i++) {
		NSETUP_DISP_VAL *disp = &list->disp_val[i];
		disp->val = i;
		sprintf(valstr, "%d|%s", disp->val, disp_val[i]);
		snprintf(disp->disp, DISP_VAL_MAX, valstr);
	}
}


static void nsetup_init_isp_blc_mode(NSETUP_LIST_VALUE *mode)
{
	int i = 0;
	char valstr[64];
	mode->init_val = 0;

	const char disp_val[5][DISP_VAL_MAX] = {
		"OFF", "BLC", "WDR", "\0"
	};

	for(i = 0; i < 4; i++) {
		NSETUP_DISP_VAL *disp = &mode->disp_val[i];
		disp->val = i;
		sprintf(valstr, "%d|%s", disp->val, disp_val[i]);
		snprintf(disp->disp, DISP_VAL_MAX, valstr);
	}
}


static void nsetup_init_isp_blc_wdr_mode(NSETUP_LIST_VALUE *mode)
{
	int i = 0;
	char valstr[64];
	mode->init_val = 1;//w4000 1=auto

	const char disp_val[4][DISP_VAL_MAX] = {
		"OFF", "AUTO", "MANUAL", "\0"
	};

	for(i = 0; i < 3; i++) {
		//w4000 mode->init_val = 0;
		NSETUP_DISP_VAL *disp = &mode->disp_val[i];
		disp->val = i;
		sprintf(valstr, "%d|%s", disp->val, disp_val[i]);
		snprintf(disp->disp, DISP_VAL_MAX, valstr);
	}
}


static void nsetup_init_isp_awb_mode(NSETUP_LIST_VALUE *awb_mode)
{
	int i = 0;
	char valstr[64];
	awb_mode->init_val = 1;

	const char disp_val[7][DISP_VAL_MAX] = {
		"MANUAL", "AUTO", "DAYLIGHT", "CLOUDY", "TUNGSTEN", "SUNSET", "\0"
	};

	for(i = 0; i < 6; i++) {
		awb_mode->init_val = 1;
		NSETUP_DISP_VAL *disp = &awb_mode->disp_val[i];
		disp->val = i;
		sprintf(valstr, "%d|%s", disp->val, disp_val[i]);
		snprintf(disp->disp, DISP_VAL_MAX, valstr);
	}
}

static void nsetup_init_isp_dn_mode(NSETUP_LIST_VALUE *dn_mode, int init_val)
{
	int i = 0;
	char valstr[64];
	dn_mode->init_val = init_val;

	const char disp_val[8][DISP_VAL_MAX] = {
		"AUTO", "DAY", "NIGHT", "\0"
	};

	for(i = 0; i < 3; i++) {
		NSETUP_DISP_VAL *disp = &dn_mode->disp_val[i];
		disp->val = i;
		sprintf(valstr, "%d|%s", disp->val, disp_val[i]);
		snprintf(disp->disp, DISP_VAL_MAX, valstr);
	}
}

int setup_default_nsetup(NSETUP_T *nsetup)
{
	int i;
	setup_range_nsetup(nsetup);

	/** IQ **/
	nsetup_set_range_val_default(&nsetup->isp.iq.color.brightness);
	nsetup_set_range_val_default(&nsetup->isp.iq.color.contrast);
	nsetup_set_range_val_default(&nsetup->isp.iq.color.saturation);
	nsetup_set_range_val_default(&nsetup->isp.iq.nr.denoise);
	nsetup_set_range_val_default(&nsetup->isp.iq.sharpeness);

	/** AE **/
	nsetup_set_range_val_default(&nsetup->isp.ae.shutter.min);
	nsetup_set_range_val_default(&nsetup->isp.ae.shutter.max);
	// nsetup_set_range_val_default(&nsetup->isp.ae.gain.min);
	nsetup_set_range_val_default(&nsetup->isp.ae.gain.max);

	/** BLC **/
	nsetup_set_list_val_default(&nsetup->isp.blc.blc.mode);
	nsetup_set_list_val_default(&nsetup->isp.blc.wdr.mode);
	nsetup_set_range_val_default(&nsetup->isp.blc.wdr.strength);
	nsetup_set_list_val_default(&nsetup->isp.blc.shdr.mode);

	/** AWB **/
	nsetup_set_list_val_default(&nsetup->isp.awb.mode);
	nsetup_set_range_val_default(&nsetup->isp.awb.auto_ratio.r_ratio);
	nsetup_set_range_val_default(&nsetup->isp.awb.auto_ratio.b_ratio);
	nsetup_set_range_val_default(&nsetup->isp.awb.manual_gain.r_gain);
	nsetup_set_range_val_default(&nsetup->isp.awb.manual_gain.g_gain);
	nsetup_set_range_val_default(&nsetup->isp.awb.manual_gain.b_gain);

	/** DAY&NIGHT **/
	nsetup_set_list_val_default(&nsetup->isp.dn.mode);


	return 0;
}

int setup_range_nsetup(NSETUP_T *nsetup)//w4000_default
{
	int val;


	nsetup_set_range_val(&nsetup->isp.iq.color.brightness, 1, 100, 50);
	nsetup_set_range_val(&nsetup->isp.iq.color.contrast, 1, 100, 50);
	nsetup_set_range_val(&nsetup->isp.iq.color.saturation, 1, 100,65);//w4000_default 50);

	nsetup_set_range_val(&nsetup->isp.iq.nr.denoise, 0, 200, 100);
	nsetup_set_range_val(&nsetup->isp.iq.sharpeness, 0, 200,50);//w4000_default 100);
	nsetup_set_range_val(&nsetup->isp.ae.shutter.min, 30, 33333, 30);
	nsetup_set_range_val(&nsetup->isp.ae.shutter.max, 30, 33333, 33333);
	nsetup_init_isp_onoff_val(&nsetup->isp.blc.blc.mode, 0);//w4000_default auto 0);
	nsetup_init_isp_blc_wdr_mode(&nsetup->isp.blc.wdr.mode);
	nsetup_set_range_val(&nsetup->isp.blc.wdr.strength, 0, 255, 128);
	nsetup_init_isp_onoff_val(&nsetup->isp.blc.shdr.mode, 0);
	nsetup_init_isp_awb_mode(&nsetup->isp.awb.mode);
	nsetup_set_range_val(&nsetup->isp.awb.auto_ratio.r_ratio, 1, 400, 100);
	nsetup_set_range_val(&nsetup->isp.awb.auto_ratio.b_ratio, 1, 400, 100);
	nsetup_set_range_val(&nsetup->isp.awb.manual_gain.r_gain, 1, 2047, 100);
	nsetup_set_range_val(&nsetup->isp.awb.manual_gain.g_gain, 1, 2047, 100);
	nsetup_set_range_val(&nsetup->isp.awb.manual_gain.b_gain, 1, 2047, 100);
	nsetup_init_isp_dn_mode(&nsetup->isp.dn.mode, 1);
	// nsetup_set_range_val(&nsetup->isp.ae.gain.min, 0, 20, 0);
	// nsetup_set_range_val(&nsetup->isp.ae.gain.max, 0, 20, 20);
	


	return 0;
}


#endif
