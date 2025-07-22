#include <stdio.h>
#include <string.h>
#include "fcgi_common.h"
#include "api_video_quality_ae.h"
#include "aes.h"



static json_t *video_quality_ae(char *data, int id, char *attr)
{
	json_t *jae = json_object();
	json_t *jshutter = json_object();
	// json_t *jgain = json_object();

	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	load_setup(&setup, &setup_ext);

	NSETUP_ISP_AE_T *ae = &setup.nsetup.isp.ae;
	NSETUP_ISP_AE_SHUTTER_T *shutter = &ae->shutter;
	// NSETUP_ISP_AE_GAIN_T *gain = &ae->gain;

	json_object_set_new(jshutter, "min", json_integer(shutter->min.val));
	json_object_set_new(jshutter, "max", json_integer(shutter->max.val));
	json_object_set_new(jae, "shutter", jshutter);


	// json_object_set_new(jgain, "min", json_integer(gain->min.val));
	// json_object_set_new(jgain, "max", json_integer(gain->max.val));
	// json_object_set_new(jae, "gain", jgain);

	return jae;
}

static json_t *video_quality_ae_default(char *data, int id, char *attr)
{
	json_t *jae = json_object();
	json_t *jshutter = json_object();
	json_t *jgain = json_object();

	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	load_setup(&setup, &setup_ext);

	NSETUP_ISP_AE_T *ae = &setup.nsetup.isp.ae;
	NSETUP_ISP_AE_SHUTTER_T *shutter = &ae->shutter;
	// NSETUP_ISP_AE_GAIN_T *gain = &ae->gain;

	json_object_set_new(jshutter, "min", json_integer(shutter->min.init_val));
	json_object_set_new(jshutter, "max", json_integer(shutter->max.init_val));
   	json_object_set_new(jae, "shutter", jshutter);


	// json_object_set_new(jgain, "min", json_integer(gain->min.init_val));
	// json_object_set_new(jgain, "max", json_integer(gain->max.init_val));
   	// json_object_set_new(jae, "gain", jgain);




	return jae;
}

static json_t *video_quality_ae_range(char *data, int id, char *attr)
{
	json_t *jae = json_object();
	json_t *jshutter = json_object();
	json_t *jgain = json_object();
	json_t *jrange;

	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	load_setup(&setup, &setup_ext);

	NSETUP_ISP_AE_T *ae = &setup.nsetup.isp.ae;
	NSETUP_ISP_AE_SHUTTER_T *shutter = &ae->shutter;
	// NSETUP_ISP_AE_GAIN_T *gain = &ae->gain;

	jrange = json_range_minmax(shutter->min.min, shutter->min.max);
	json_object_set_new(jshutter, "min", jrange);

	jrange = json_range_minmax(shutter->max.min, shutter->max.max);
	json_object_set_new(jshutter, "max", jrange);

	json_object_set_new(jae, "shutter", jshutter);


	// jrange = json_range_minmax(gain->min.min, gain->min.max);
	// json_object_set_new(jgain, "min", jrange);

	// jrange = json_range_minmax(gain->max.min, gain->max.max);
	// json_object_set_new(jgain, "max", jrange);

	// json_object_set_new(jae, "gain", jgain);


	return jae;
}

static int get_video_quality_ae(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *sub = video_quality_ae(data, id, attr);

	if(sub == NULL) {
		goto Fail;
	}

   	json_object_set_new(root, "exposure", sub);

	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int get_video_quality_ae_default(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *sub = video_quality_ae_default(data, id, attr);

	if(sub == NULL) {
		goto Fail;
	}

   	json_object_set_new(root, "exposure_default", sub);

	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int get_video_quality_ae_range(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *sub = video_quality_ae_range(data, id, attr);

	if(sub == NULL) {
		goto Fail;
	}

	json_object_set_new(root, "exposure_range", sub);

	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int get_video_quality_ae_all(char *data, int id, char *attr, char *resp)
{
	// struct AES_ctx ctx;
	json_t *root = json_object();
	json_t *iq = video_quality_ae(data, id, attr);
	json_t *def = video_quality_ae_default(data, id, attr);
	json_t *range = video_quality_ae_range(data, id, attr);
	if(iq == NULL || def == NULL || range == NULL) {
		goto Fail;
	}

   	json_object_set_new(root, "exposure", iq);
   	json_object_set_new(root, "exposure_default", def);
   	json_object_set_new(root, "exposure_range", range);

	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}
	

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int put_video_quality_ae_shutter(NSETUP_ISP_AE_SHUTTER_T *shutter, json_t *root)
{
	int update = 0;

	int min;
	int max;

	min = json_obj_int(root, "min");
	max = json_obj_int(root, "max");
	

	if(min >= shutter->min.min && min <= shutter->min.max) {
		shutter->min.val = min;
		update = 1;
	}

	if(min >= shutter->max.min && min <= shutter->max.max) {
		shutter->max.val = max;
		update = 1;
	}

	return update;
}

static int put_video_quality_ae_gain(NSETUP_ISP_AE_GAIN_T *gain, json_t *root)
{
	int update = 0;

	int min;
	int max;

	min = json_obj_int(root, "min");
	max = json_obj_int(root, "max");

	if(min >= gain->min.min && min <= gain->min.max) {
		gain->min.val = min;
		update = 1;
	}

	if(min >= gain->max.min && min <= gain->max.max) {
		gain->max.val = max;
		update = 1;
	}

	return update;
}


static int put_video_quality_ae(char *data, int id, char *attr, char *resp)
{
	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	int update = 0;

	json_t *root;
	json_t *iq = NULL;
	json_t *value = NULL;
	json_error_t error;
	int ret = 200;

	root = json_loads(data, JSON_DISABLE_EOF_CHECK, &error);
	if(root == NULL) {
		ret = 400;
		goto Fail;
	}
	iq = json_object_get(root, "exposure");
	if(iq == NULL) {
		ret = 400;
		goto Fail;
	}

	load_setup(&setup, &setup_ext);
	init_setup_isp_notify(&setup_ext);

	NSETUP_ISP_AE_T *ae = &setup.nsetup.isp.ae;
	NSETUP_ISP_AE_SHUTTER_T *shutter = &ae->shutter;
	NSETUP_ISP_AE_GAIN_T *gain = &ae->gain;


	value = json_object_get(iq, "shutter");
	if(value != NULL) {
		update = put_video_quality_ae_shutter(shutter, value);
		if(update < 0) {
			goto Fail;
		}
	}

	value = json_object_get(iq, "gain");
	if(value != NULL) {
		update = put_video_quality_ae_gain(gain, value);
		if(update < 0) {
			goto Fail;
		}
	}

	if(update) {
		set_setup_isp_notify(&setup_ext, UPDATE_FCGI_ISP_AE);
		update_setup(&setup, &setup_ext);
		// system("date");
		notify_update_isp(UPDATE_FCGI_ISP_AE);
	}

	json_decref(root);

	return ret;
Fail:
	json_decref(root);
	return ret;
}

int api_video_quality_ae(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_ae(data, id, attr, resp);
	}

	if(strncasecmp(request_method, "PUT", 3) == 0) {
		return put_video_quality_ae(data, id, attr, resp);
	}

	return 405;
}

int api_video_quality_ae_default(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_ae_default(data, id, attr, resp);
	}

	return 405;
}

int api_video_quality_ae_all(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_ae_all(data, id, attr, resp);
	}

	return 405;
}

