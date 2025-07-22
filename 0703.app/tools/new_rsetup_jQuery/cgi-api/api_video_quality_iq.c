#include <stdio.h>
#include <string.h>
#include <sys/un.h>
#include "fcgi_common.h"
#include "api_video_quality_iq.h"

#include "common2.h"

static json_t *video_quality_iq(char *data, int id, char *attr)
{
	json_t *jiq = json_object();
	json_t *jcolor = json_object();
	json_t *jnr = json_object();

	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	NSETUP_ISP_IQ_COLOR_T *color = &setup.nsetup.isp.iq.color;
	NSETUP_ISP_IQ_NR_T *nr = &setup.nsetup.isp.iq.nr;
	
	load_setup(&setup, &setup_ext);

	json_object_set_new(jcolor, "brightness", json_integer(color->brightness.val)); 
	json_object_set_new(jcolor, "contrast", json_integer(color->contrast.val)); 
	json_object_set_new(jcolor, "saturation", json_integer(color->saturation.val)); 
   	json_object_set_new(jiq, "color", jcolor);	
   	
	json_object_set_new(jnr, "denoise", json_integer(nr->denoise.val)); 
   	json_object_set_new(jiq, "nr", jnr);	
   	
	json_object_set_new(jiq, "sharpeness", json_integer(setup.nsetup.isp.iq.sharpeness.val));	
	// {"color": {"brightness": 100, "contrast": 100, "saturation": 100, }, "nr": {"denoise": 100}, "sharpeness": 100}



	return jiq;
}

static json_t *video_quality_iq_default(char *data, int id, char *attr)
{
	json_t *jiq = json_object();
	json_t *jcolor = json_object();
	json_t *jnr = json_object();

	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	NSETUP_ISP_IQ_COLOR_T *color = &setup.nsetup.isp.iq.color;
	NSETUP_ISP_IQ_NR_T *nr = &setup.nsetup.isp.iq.nr;
	
	load_setup(&setup, &setup_ext);

	json_object_set_new(jcolor, "brightness", json_integer(color->brightness.init_val)); 
	json_object_set_new(jcolor, "contrast", json_integer(color->contrast.init_val)); 
	json_object_set_new(jcolor, "saturation", json_integer(color->saturation.init_val)); 
	json_object_set_new(jiq, "color", jcolor);	
   	
	json_object_set_new(jnr, "denoise", json_integer(nr->denoise.init_val)); 
   	json_object_set_new(jiq, "nr", jnr);	
   	
	json_object_set_new(jiq, "sharpeness", json_integer(setup.nsetup.isp.iq.sharpeness.init_val));	
	

	return jiq;
}

static json_t *video_quality_iq_range(char *data, int id, char *attr)
{
	json_t *jiq = json_object();
	json_t *jcolor = json_object();
	json_t *jnr = json_object();
	json_t *jrange;

	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	NSETUP_ISP_IQ_COLOR_T *color = &setup.nsetup.isp.iq.color;
	NSETUP_ISP_IQ_NR_T *nr = &setup.nsetup.isp.iq.nr;
	
	load_setup(&setup, &setup_ext);

	jrange = json_range_minmax(color->brightness.min, color->brightness.max);
	json_object_set_new(jcolor, "brightness", jrange); 

	jrange = json_range_minmax(color->contrast.min, color->contrast.max);
	json_object_set_new(jcolor, "contrast", jrange); 
	
	jrange = json_range_minmax(color->saturation.min, color->saturation.max);
	json_object_set_new(jcolor, "saturation", jrange); 
	
	json_object_set_new(jiq, "color", jcolor);	
 
	jrange = json_range_minmax(nr->denoise.min, nr->denoise.max);
	json_object_set_new(jnr, "denoise", jrange); 

	json_object_set_new(jiq, "nr", jnr);	

	jrange = json_range_minmax(setup.nsetup.isp.iq.sharpeness.min, setup.nsetup.isp.iq.sharpeness.max);
	json_object_set_new(jiq, "sharpeness", jrange); 


	return jiq;
}

static int get_video_quality_iq(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *sub = video_quality_iq(data, id, attr);

	if(sub == NULL) {
		goto Fail;
	}

   	json_object_set_new(root, "image_quality", sub);	

	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int get_video_quality_iq_default(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *sub = video_quality_iq_default(data, id, attr);

	if(sub == NULL) {
		goto Fail;
	}

   	json_object_set_new(root, "image_quality_default", sub);	
	
	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int get_video_quality_iq_range(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *sub = video_quality_iq_range(data, id, attr);

	if(sub == NULL) {
		goto Fail;
	}
   	
	json_object_set_new(root, "image_quality_range", sub);	

	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int get_video_quality_iq_all(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *iq = video_quality_iq(data, id, attr);
	json_t *def = video_quality_iq_default(data, id, attr);
	json_t *range = video_quality_iq_range(data, id, attr);

	if(iq == NULL || def == NULL || range == NULL) {
		goto Fail;
	}

   	json_object_set_new(root, "image_quality", iq);	
   	json_object_set_new(root, "image_quality_default", def);	
   	json_object_set_new(root, "image_quality_range", range);	

	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int put_video_quality_iq_color(_CAMSETUP *setup, json_t *root)
{
	NSETUP_ISP_IQ_COLOR_T *color = &(setup->nsetup.isp.iq.color);
	int update = 0;

	int brightness;
	int contrast;
	int saturation;

	brightness = json_obj_int(root, "brightness");
	contrast = json_obj_int(root, "contrast");
	saturation = json_obj_int(root, "saturation");

	if(brightness >= color->brightness.min && brightness <= color->brightness.max) {
		color->brightness.val = brightness;
		update = 1;
	}

	if(contrast >= color->contrast.min && contrast <= color->contrast.max) {
		color->contrast.val = contrast;
		update = 1;
	}
	
	if(saturation >= color->saturation.min && saturation <= color->saturation.max) {
		color->saturation.val = saturation;
		update = 1;
	}

	return update;

}

static int put_video_quality_iq_nr(_CAMSETUP *setup, json_t *root)
{
	NSETUP_ISP_IQ_NR_T *nr = &(setup->nsetup.isp.iq.nr);

	int update = 0;
	int denoise;

	denoise = json_obj_int(root, "denoise");

	if(denoise >= nr->denoise.min && denoise <= nr->denoise.max) {
		nr->denoise.val = denoise;
		update = 1;
	}
	
	
	return update;
}

static int put_video_quality_iq(char *data, int id, char *attr, char *resp)
{
	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	int update = 0;
	json_t *root;
	json_t *iq = NULL;
	json_t *value = NULL;
	json_error_t error;
	int v;
	int ret = 200;

	root = json_loads(data, JSON_DISABLE_EOF_CHECK, &error);
	if(root == NULL) {
		ret = 400;
		goto Fail;
	}

	iq = json_object_get(root, "image_quality");
	if(iq == NULL) {
		ret = 400;
		goto Fail;
	}
	
	load_setup(&setup, &setup_ext);
	// init_setup_isp_notify(&setup_ext);

	value = json_object_get(iq, "color");
	if(value != NULL) {
		update = put_video_quality_iq_color(&setup, value);
		if(update < 0) {
			goto Fail;
		}
	}

	value = json_object_get(iq, "nr");
	if(value != NULL) {
		update = put_video_quality_iq_nr(&setup, value);
		if(update < 0) {
			goto Fail;
		}
	}

	v = json_obj_int(iq, "sharpeness");
	if(v >= setup.nsetup.isp.iq.sharpeness.min && v <= setup.nsetup.isp.iq.sharpeness.max) {
		setup.nsetup.isp.iq.sharpeness.val = v;
		update = 1;
	}


	if(update) {
		//set_setup_isp_notify(&setup_ext, UPDATE_FCGI_ISP_IQ);
		update_setup(&setup, &setup_ext);
		notify_update_isp(UPDATE_FCGI_ISP_IQ);
	}


	json_decref(root);
	return ret;
Fail:
	json_decref(root);
	return ret;
}

int api_video_quality_iq(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_iq(data, id, attr, resp);
	}

	if(strncasecmp(request_method, "PUT", 3) == 0) {
		return put_video_quality_iq(data, id, attr, resp);
	}

	return 405;
}

int api_video_quality_iq_range(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_iq_range(data, id, attr, resp);
	}

	return 405;
}

int api_video_quality_iq_default(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_iq_default(data, id, attr, resp);
	}

	return 405;
}

int api_video_quality_iq_all(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_iq_all(data, id, attr, resp);
	}

	return 405;
}
