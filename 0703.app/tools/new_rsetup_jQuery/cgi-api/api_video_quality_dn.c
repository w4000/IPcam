#include <stdio.h>
#include <string.h>
#include "fcgi_common.h"
#include "api_video_quality_dn.h"

static json_t *video_quality_dn(char *data, int id, char *attr)
{
	json_t *jdn = json_object();

	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;



	load_setup(&setup, &setup_ext);
	json_object_set_new(jdn, "mode", json_integer(setup.nsetup.isp.dn.mode.val));


	return jdn;
}

static json_t *video_quality_dn_default(char *data, int id, char *attr)
{
	json_t *jdn = json_object();

	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;

	load_setup(&setup, &setup_ext);
	json_object_set_new(jdn, "mode", json_integer(setup.nsetup.isp.dn.mode.init_val));



	return jdn;
}

static json_t *video_quality_dn_range(char *data, int id, char *attr)
{
	json_t *jdn = json_object();
	json_t *jrange;

	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;

	load_setup(&setup, &setup_ext);

	jrange = json_range_array(setup.nsetup.isp.dn.mode.disp_val, 3, setup.lang);
	json_object_set_new(jdn, "mode", jrange);


	return jdn;
}

static int get_video_quality_dn(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *sub = video_quality_dn(data, id, attr);


	if(sub == NULL) {
		goto Fail;
	}

   	json_object_set_new(root, "daynight", sub);

	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int get_video_quality_dn_default(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *sub = video_quality_dn_default(data, id, attr);

	if(sub == NULL) {
		goto Fail;
	}

   	json_object_set_new(root, "daynight_default", sub);

	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int get_video_quality_dn_range(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *sub = video_quality_dn_range(data, id, attr);

	if(sub == NULL) {
		goto Fail;
	}

	json_object_set_new(root, "daynight_range", sub);

	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int get_video_quality_dn_all(char *data, int id, char *attr, char *resp)
{
	json_t *root = json_object();
	json_t *dn = video_quality_dn(data, id, attr);
	json_t *def = video_quality_dn_default(data, id, attr);
	json_t *range = video_quality_dn_range(data, id, attr);

	if(dn == NULL || def == NULL || range == NULL) {
		goto Fail;
	}

   	json_object_set_new(root, "daynight", dn);
   	json_object_set_new(root, "daynight_default", def);
   	json_object_set_new(root, "daynight_range", range);

	if(json_to_wstring(root, resp) < 0) {
		goto Fail;
	}

	json_decref(root);
	return 200;
Fail:
	json_decref(root);
	return 520;
}

static int put_video_quality_dn(char *data, int id, char *attr, char *resp)
{
	_CAMSETUP setup;
	_CAMSETUP_EXT setup_ext;
	int update = 0;

	json_t *root;
	json_t *dn = NULL;
	json_error_t error;
	int v;
	int ret = 200;

	root = json_loads(data, JSON_DISABLE_EOF_CHECK, &error);
	if(root == NULL) {
		ret = 400;
		goto Fail;
	}

	dn = json_object_get(root, "daynight");
	if(dn == NULL) {
		ret = 400;
		goto Fail;
	}

	load_setup(&setup, &setup_ext);
	init_setup_isp_notify(&setup_ext);

	v = json_obj_int(dn, "mode");
	if(/*TODO*/1) {
		setup.nsetup.isp.dn.mode.val = v;
		update = 1;
	}


	if(update) {
		set_setup_isp_notify(&setup_ext, UPDATE_FCGI_ISP_DN);
		update_setup(&setup, &setup_ext);
		notify_update_isp(UPDATE_FCGI_ISP_DN);
	}

	json_decref(root);
	return ret;
Fail:
	json_decref(root);
	return ret;
}

int api_video_quality_dn(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_dn(data, id, attr, resp);
	}

	if(strncasecmp(request_method, "PUT", 3) == 0) {
		return put_video_quality_dn(data, id, attr, resp);
	}

	return 405;
}

int api_video_quality_dn_range(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_dn_range(data, id, attr, resp);
	}

	return 405;
}

int api_video_quality_dn_default(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_dn_default(data, id, attr, resp);
	}

	return 405;
}

int api_video_quality_dn_all(char *request_method, char *data, int id, char *attr, char *resp)
{
	if(strncasecmp(request_method, "GET", 3) == 0) {
		return get_video_quality_dn_all(data, id, attr, resp);
	}

	return 405;
}
