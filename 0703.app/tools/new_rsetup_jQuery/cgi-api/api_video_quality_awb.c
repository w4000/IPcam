#include <stdio.h>
#include <string.h>
#include "fcgi_common.h"
#include "api_video_quality_awb.h"

static json_t *video_quality_awb(char *data, int id, char *attr)
{
	json_t *jawb = json_object();
	json_t *jauto = json_object();
	json_t *jmanual = json_object();

	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	NSETUP_ISP_AWB_AUTO_T *auto_r = &setup.nsetup.isp.awb.auto_ratio;
	NSETUP_ISP_AWB_MANUAL_T *manual = &setup.nsetup.isp.awb.manual_gain;
	
	load_setup(&setup, &setup_ext);

	json_object_set_new(jawb, "mode", json_integer(setup.nsetup.isp.awb.mode.val)); 
	
	json_object_set_new(jauto, "r_ratio", json_integer(auto_r->r_ratio.val)); 
	json_object_set_new(jauto, "b_ratio", json_integer(auto_r->b_ratio.val)); 
   	json_object_set_new(jawb, "auto", jauto);	
   	
	json_object_set_new(jmanual, "r_gain", json_integer(manual->r_gain.val)); 
	json_object_set_new(jmanual, "g_gain", json_integer(manual->g_gain.val)); 
	json_object_set_new(jmanual, "b_gain", json_integer(manual->b_gain.val)); 
   	json_object_set_new(jawb, "manual", jmanual);	


	return jawb;
}

static json_t *video_quality_awb_default(char *data, int id, char *attr)
{
	json_t *jawb = json_object();
	json_t *jauto = json_object();
	json_t *jmanual = json_object();

	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	NSETUP_ISP_AWB_AUTO_T *auto_r = &setup.nsetup.isp.awb.auto_ratio;
	NSETUP_ISP_AWB_MANUAL_T *manual = &setup.nsetup.isp.awb.manual_gain;
	
	load_setup(&setup, &setup_ext);

	json_object_set_new(jawb, "mode", json_integer(setup.nsetup.isp.awb.mode.init_val)); 
	
	json_object_set_new(jauto, "r_ratio", json_integer(auto_r->r_ratio.init_val)); 
	json_object_set_new(jauto, "b_ratio", json_integer(auto_r->b_ratio.init_val)); 
   	json_object_set_new(jawb, "auto", jauto);	
   	
	json_object_set_new(jmanual, "r_gain", json_integer(manual->r_gain.init_val)); 
	json_object_set_new(jmanual, "g_gain", json_integer(manual->g_gain.init_val)); 
	json_object_set_new(jmanual, "b_gain", json_integer(manual->b_gain.init_val)); 
   	json_object_set_new(jawb, "manual", jmanual);	


	return jawb;

}

static json_t *video_quality_awb_range(char *data, int id, char *attr)
{
	json_t *jawb = json_object();
	json_t *jauto = json_object();
	json_t *jmanual = json_object();
	json_t *jrange;

	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	NSETUP_ISP_AWB_AUTO_T *auto_r = &setup.nsetup.isp.awb.auto_ratio;
	NSETUP_ISP_AWB_MANUAL_T *manual = &setup.nsetup.isp.awb.manual_gain;
	
	load_setup(&setup, &setup_ext);

	jrange = json_range_array(setup.nsetup.isp.awb.mode.disp_val, 6, setup.lang);
	json_object_set_new(jawb, "mode", jrange); 

	jrange = json_range_minmax(auto_r->r_ratio.min, auto_r->r_ratio.max);
	json_object_set_new(jauto, "r_ratio", jrange); 

	jrange = json_range_minmax(auto_r->b_ratio.min, auto_r->b_ratio.max);
	json_object_set_new(jauto, "b_ratio", jrange); 
	
	json_object_set_new(jawb, "auto", jauto);	
 
	jrange = json_range_minmax(manual->r_gain.min, manual->r_gain.max);
	json_object_set_new(jmanual, "r_gain", jrange); 

	jrange = json_range_minmax(manual->g_gain.min, manual->g_gain.max);
	json_object_set_new(jmanual, "g_gain", jrange); 
	
	jrange = json_range_minmax(manual->b_gain.min, manual->b_gain.max);
	json_object_set_new(jmanual, "b_gain", jrange); 
	
	json_object_set_new(jawb, "manual", jmanual);	


	return jawb;
}

static int get_video_quality_awb(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *sub = video_quality_awb(data, id, attr);

	if(sub == NULL) {
		goto Fail;
	}

   	json_object_set_new(root, "white_balance", sub);	

	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int get_video_quality_awb_default(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *sub = video_quality_awb_default(data, id, attr);

	if(sub == NULL) {
		goto Fail;
	}

   	json_object_set_new(root, "white_balance_default", sub);	
	
	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int get_video_quality_awb_range(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *sub = video_quality_awb_range(data, id, attr);

	if(sub == NULL) {
		goto Fail;
	}
   	
	json_object_set_new(root, "white_balance_range", sub);	

	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int get_video_quality_awb_all(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *awb = video_quality_awb(data, id, attr);
	json_t *def = video_quality_awb_default(data, id, attr);
	json_t *range = video_quality_awb_range(data, id, attr);

	if(awb == NULL || def == NULL || range == NULL) {
		goto Fail;
	}

   	json_object_set_new(root, "white_balance", awb);	
   	json_object_set_new(root, "white_balance_default", def);	
   	json_object_set_new(root, "white_balance_range", range);	

	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int put_video_quality_awb_auto(_CAMSETUP *setup, json_t *root)
{
	NSETUP_ISP_AWB_AUTO_T *auto_r = &(setup->nsetup.isp.awb.auto_ratio);

	int update = 0;

	int r_ratio;
	int b_ratio;

	r_ratio = json_obj_int(root, "r_ratio");
	b_ratio = json_obj_int(root, "b_ratio");

	if(r_ratio >= auto_r->r_ratio.min && r_ratio <= auto_r->r_ratio.max) {
		auto_r->r_ratio.val = r_ratio;
		update = 1;
	}
	if(b_ratio >= auto_r->b_ratio.min && b_ratio <= auto_r->b_ratio.max) {
		auto_r->b_ratio.val = b_ratio;
		update = 1;
	}

	return update;
}

static int put_video_quality_awb_manual(_CAMSETUP *setup, json_t *root)
{
	NSETUP_ISP_AWB_MANUAL_T *manual = &(setup->nsetup.isp.awb.manual_gain);

	int update = 0;

	int r_gain;
	int g_gain;
	int b_gain;

	r_gain = json_obj_int(root, "r_gain");
	g_gain = json_obj_int(root, "g_gain");
	b_gain = json_obj_int(root, "b_gain");

	if(r_gain >= manual->r_gain.min && r_gain <= manual->r_gain.max) {
		manual->r_gain.val = r_gain;
		update = 1;
	}
	if(g_gain >= manual->g_gain.min && g_gain <= manual->g_gain.max) {
		manual->g_gain.val = g_gain;
		update = 1;
	}
	if(b_gain >= manual->b_gain.min && b_gain <= manual->b_gain.max) {
		manual->b_gain.val = b_gain;
		update = 1;
	}

	return update;
}

static int put_video_quality_awb(char *data, int id, char *attr, char *resp)
{
	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	int update = 0;
	
	json_t *root;
	json_t *awb = NULL;
	json_t *value = NULL;
	json_error_t error;
	int v;
	int ret = 200;

	root = json_loads(data, JSON_DISABLE_EOF_CHECK, &error);
	if(root == NULL) {
		ret = 400;
		goto Fail;
	}

	awb = json_object_get(root, "white_balance");
	if(awb == NULL) {
		ret = 400;
		goto Fail;
	}
	
	load_setup(&setup, &setup_ext);
	init_setup_isp_notify(&setup_ext);

	v = json_obj_int(awb, "mode");
	if(/*TODO*/1) {
		setup.nsetup.isp.awb.mode.val = v;
		update = 1;
	}

	value = json_object_get(awb, "auto");
	if(value != NULL) {
		update = put_video_quality_awb_auto(&setup, value);
		if(update < 0) {
			goto Fail;
		}
	}

	value = json_object_get(awb, "manual");
	if(value != NULL) {
		update = put_video_quality_awb_manual(&setup, value);
		if(update < 0) {
			goto Fail;
		}
	}

	if(update) {
		set_setup_isp_notify(&setup_ext, UPDATE_FCGI_ISP_AWB);
		update_setup(&setup, &setup_ext);
		notify_update_isp(UPDATE_FCGI_ISP_AWB);
	}

	json_decref(root);
	return ret;
Fail:
	json_decref(root);
	return ret;
}

int api_video_quality_awb(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_awb(data, id, attr, resp);
	}

	if(strncasecmp(request_method, "PUT", 3) == 0) {
		return put_video_quality_awb(data, id, attr, resp);
	}

	return 405;
}

int api_video_quality_awb_range(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_awb_range(data, id, attr, resp);
	}

	return 405;
}

int api_video_quality_awb_default(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_awb_default(data, id, attr, resp);
	}

	return 405;
}

int api_video_quality_awb_all(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_awb_all(data, id, attr, resp);
	}

	return 405;
}
