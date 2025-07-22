#include <stdio.h>
#include <string.h>
#include "fcgi_common.h"
#include "api_video_quality_blc.h"

static json_t *video_quality_blc(char *data, int id, char *attr)
{
	json_t *jbl = json_object();
	json_t *jblc = json_object();
	json_t *jwdr = json_object();

	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	NSETUP_ISP_BLC_BLC_T *blc = &setup.nsetup.isp.blc.blc;
	NSETUP_ISP_BLC_WDR_T *wdr = &setup.nsetup.isp.blc.wdr;
	
	load_setup(&setup, &setup_ext);

   	json_object_set_new(jblc, "mode", json_integer(blc->mode.val));	
   	json_object_set_new(jbl, "blc", jblc);	
   	
	json_object_set_new(jwdr, "mode", json_integer(wdr->mode.val)); 
	json_object_set_new(jwdr, "strength", json_integer(wdr->strength.val)); 
   	json_object_set_new(jbl, "wdr", jwdr);	
   	


	return jbl;
}

static json_t *video_quality_blc_default(char *data, int id, char *attr)
{
	json_t *jbl = json_object();
	json_t *jblc = json_object();
	json_t *jwdr = json_object();

	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	NSETUP_ISP_BLC_BLC_T *blc = &setup.nsetup.isp.blc.blc;
	NSETUP_ISP_BLC_WDR_T *wdr = &setup.nsetup.isp.blc.wdr;
	
	load_setup(&setup, &setup_ext);

	json_object_set_new(jblc, "mode", json_integer(blc->mode.init_val));	
   	json_object_set_new(jbl, "blc", jblc);	
   	
	json_object_set_new(jwdr, "mode", json_integer(wdr->mode.init_val)); 
	json_object_set_new(jwdr, "strength", json_integer(wdr->strength.init_val)); 
   	json_object_set_new(jbl, "wdr", jwdr);	
   	
	

	return jbl;
}

static json_t *video_quality_blc_range(char *data, int id, char *attr)
{
	json_t *jbl = json_object();
	json_t *jblc = json_object();
	json_t *jwdr = json_object();
	json_t *jrange;

	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	NSETUP_ISP_BLC_BLC_T *blc = &setup.nsetup.isp.blc.blc;
	NSETUP_ISP_BLC_WDR_T *wdr = &setup.nsetup.isp.blc.wdr;
	
	load_setup(&setup, &setup_ext);

	jrange = json_range_array(blc->mode.disp_val, 2, setup.lang);
	json_object_set_new(jblc, "mode", jrange); 
	json_object_set_new(jbl, "blc", jblc);	
 
	jrange = json_range_array(wdr->mode.disp_val, 3, setup.lang);
	json_object_set_new(jwdr, "mode", jrange); 

	jrange = json_range_minmax(wdr->strength.min, wdr->strength.max);
	json_object_set_new(jwdr, "strength", jrange); 
	
	json_object_set_new(jbl, "wdr", jwdr);	


	return jbl;
}

static int get_video_quality_blc(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *sub = video_quality_blc(data, id, attr);

	if(sub == NULL) {
		goto Fail;
	}

   	json_object_set_new(root, "blc", sub);	



	// {"blc": {"blc": {"mode": 0}, "wdr": {"mode": 0, "strength": 128}}}


	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}


	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int get_video_quality_blc_default(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *sub = video_quality_blc_default(data, id, attr);

	if(sub == NULL) {
		goto Fail;
	}

	char *dump = json_dumps(sub, JSON_ENCODE_ANY);

   	json_object_set_new(root, "blc_default", sub);	
	


	dump = json_dumps(root, JSON_ENCODE_ANY);



	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}


static int get_video_quality_blc_all(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *blc = video_quality_blc(data, id, attr);
	json_t *def = video_quality_blc_default(data, id, attr);
	json_t *range = video_quality_blc_range(data, id, attr);

	if(blc == NULL || def == NULL || range == NULL) {
		goto Fail;
	}

   	json_object_set_new(root, "blc", blc);	
   	json_object_set_new(root, "blc_default", def);	
   	json_object_set_new(root, "blc_range", range);	

	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int put_video_quality_blc_blc(_CAMSETUP *setup, json_t *root)
{
	NSETUP_ISP_BLC_BLC_T *blc = &setup->nsetup.isp.blc.blc;

	int update = 0;

	int mode;

	mode = json_obj_int(root, "mode");

	if(mode >= 0) {
		blc->mode.val = mode;
		update = 1;
	}

	return update;
}

static int put_video_quality_blc_wdr(_CAMSETUP *setup, json_t *root)
{
	NSETUP_ISP_BLC_WDR_T *wdr = &setup->nsetup.isp.blc.wdr;

	int update = 0;

	int mode;
	int strength;

	mode = json_obj_int(root, "mode");
	strength = json_obj_int(root, "strength");

	if(mode >= 0) {
		wdr->mode.val = mode;
		update = 1;
	}
	
	if(strength >= wdr->strength.min && strength <= wdr->strength.max) {
		wdr->strength.val = strength;
		update = 1;
	}
	
	return update;
}



static int put_video_quality_blc(char *data, int id, char *attr, char *resp)
{
	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	int update = 0;
	
	json_t *root;
	json_t *blc = NULL;
	json_t *value = NULL;
	json_error_t error;
	int ret = 200;

	root = json_loads(data, JSON_DISABLE_EOF_CHECK, &error);
	if(root == NULL) {
		ret = 400;
		goto Fail;
	}

	blc = json_object_get(root, "blc");
	if(blc == NULL) {
		ret = 400;
		goto Fail;
	}
	
	load_setup(&setup, &setup_ext);
	init_setup_isp_notify(&setup_ext);

	value = json_object_get(blc, "blc");
	if(value != NULL) {
		update = put_video_quality_blc_blc(&setup, value);
		if(update < 0) {
			goto Fail;
		}
	}

	value = json_object_get(blc, "wdr");
	if(value != NULL) {
		update = put_video_quality_blc_wdr(&setup, value);
		if(update < 0) {
			goto Fail;
		}
	}

	if(update) {
		set_setup_isp_notify(&setup_ext, UPDATE_FCGI_ISP_BLC);
		update_setup(&setup, &setup_ext);
		notify_update_isp(UPDATE_FCGI_ISP_BLC);
	}

	json_decref(root);
	return ret;
Fail:
	json_decref(root);
	return ret;
}

int api_video_quality_blc(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_blc(data, id, attr, resp);
	}

	if(strncasecmp(request_method, "PUT", 3) == 0) {
		return put_video_quality_blc(data, id, attr, resp);
	}

	return 405;
}

int api_video_quality_blc_range(char *request_method, char *data, int id, char *attr, char *resp)
{
	return 405;
}

int api_video_quality_blc_default(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_blc_default(data, id, attr, resp);
	}

	return 405;
}

int api_video_quality_blc_all(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_blc_all(data, id, attr, resp);
	}

	return 405;
}
