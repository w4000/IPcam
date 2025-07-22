#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(ONCE_INI_DEFINE)
// for building discovery
#define INI_IMPLEMENTATION
#endif	/* ONCE_INI_DEFINE */
#include "ini.h"

#include "oem.h"
#if defined(NARCH)
#include "oem_xml_define.h"
#include "oem_tool.h"
#endif	/* NARCH */
#include "model_spec.h"

static ini_t *g_ini = NULL;

// #define MODEL_INI "/edvr/cam_setup.ini"
#define MODEL_INI "/edvr/cam_setup.ini"

const void *model_spec_key_val_add(const char *key, char *val);

//char cmd[1024];



static int load_model_default_ini()
{
	if(g_ini != NULL) {
		return 0;
	}

	int size;
	char *data;
	FILE *fp;
	{
		fp = fopen(MODEL_INI, "r");
	}
	if(fp == NULL) {
		goto Fail;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);


	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, size +1);
	data = (char*)malloc(size + 1);
	fread(data, 1, size, fp);
	data[size] = '\0';
	fclose(fp);
	printf("w4000 = load_model_default_ini ========================= \n %s \n ",data);//w4000_info

	g_ini = ini_load(data, 0);
	free(data);

	if(g_ini == NULL) {
		printf("%s:%d > (g_ini == NULL\n", __func__, __LINE__);
		goto Fail;
	}

	return 0;

Fail:
	printf("ini load fail, %s\n", MODEL_INI);
	g_ini = NULL;
	return -1;

}


static int load_model_ini()
{
	if(g_ini != NULL) {
		return 0;
	}

	int size;
	char *data;
	FILE *fp;
	{
		fp = fopen(MODEL_INI, "r");
	}
	if(fp == NULL) {
		goto Fail;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, size +1);
	data = (char*)malloc(size + 1);
	fread(data, 1, size, fp);


	data[size] = '\0';

	// printf("data? %s \n", data);

	// sprintf (cmd, "echo \"load_model_ini:line %d > data? %s\" | cat >> /mnt/ipm/log.txt", __LINE__, data);
	// system (cmd);

	fclose(fp);

	g_ini = ini_load(data, 0);
	free(data);


	

	if(g_ini == NULL) {
		// printf("ini init fail\n");
		goto Fail;
	}

	return 0;

Fail:
	// printf("ini load fail, %s\n", MODEL_INI);
	g_ini = NULL;
	return -1;

}

int model_spec_reload()
{
	if(g_ini != NULL)
		ini_destroy(g_ini);

	g_ini = NULL;

	load_model_ini();
	return 0;
}

int model_spec_all(char *str)
{
	if(load_model_ini() < 0) {
		// printf("ini init fail .\n");
		return -1;
	}

	int size = ini_save(g_ini, NULL, 0);
	size = ini_save(g_ini, str, size);

	return size;
}

const char *model_spec_string(const char *key)
{

	//sprintf (cmd, "echo \"model_spec_string ... key:%s \" | cat >> /mnt/ipm/log.txt", key);
	//system (cmd);


	if(load_model_ini() < 0) {
		// printf("ini init fail ..\n");
		//sprintf (cmd, "echo \"ini init fail ..\" | cat >> /mnt/ipm/log.txt");
		//system (cmd);

		return NULL;
	}
	int index = ini_find_property(g_ini, 1, key, -1);

	//sprintf (cmd, "echo \"model_spec_string ... index:%d \" | cat >> /mnt/ipm/log.txt", index);
	//system (cmd);


	if(index < 0) {
		// printf("! ini init fail ... (index:%d)\n", index);
		//sprintf (cmd, "echo \"ini init fail ... (key:%s, index:%d) ..\" | cat >> /mnt/ipm/log.txt", key, index);
		//system (cmd);

		if (strcmp(key, "RTSP_PORT") == 0)
			return "554";
		else if (strcmp(key, "RTSP_URL") == 0)
			return "livestream";
		else 
			return "0";
	}



	char const* val = ini_property_value(g_ini, 1, index );
		//sprintf (cmd, "echo \"model_spec_string ... index:%d, val:%s \" | cat >> /mnt/ipm/log.txt", index, val);
		//system (cmd);
	return val;
}


const int model_spec_int(const char *key)
{
	const char* val = model_spec_string(key);

	if(val == NULL) {
		// printf("model spec init fail\n");
		return -1;
	}

	return atoi(val);
}

const void *model_spec_key_val_add(const char *key, char *val)
{
	char data[512] = {0x00, };
	FILE *fp = NULL;
	if(load_model_ini() < 0) {
		return NULL;
	}


	//sprintf (cmd, "echo \"model_spec_key_val_add :line %d > val? %s\" | cat >> /mnt/ipm/log.txt", __LINE__, val);
	//system(cmd);


	ini_property_add(g_ini, INI_GLOBAL_SECTION, key, 0, val, 0);

	int size = model_spec_all(data);

	//sprintf (cmd, "echo \"model_spec_key_val_add :line %d > size? %s\" | cat >> /mnt/ipm/log.txt", __LINE__, size);
	//system(cmd);

	{
		fp = fopen(MODEL_INI, "w");
	}


	//sprintf (cmd, "echo \"model_spec_key_val_add :line %d > data? %s\" | cat >> /mnt/ipm/log.txt", __LINE__, data);
	//system(cmd);

	fwrite(data, 1, size, fp);
	fclose(fp);

	return NULL;
}

const void *model_spec_val_add(const char *key, char *val)
{
	char data[512] = {0x00, };
	FILE *fp = NULL;
	if(load_model_ini() < 0) {
		return NULL;
	}

	//sprintf (cmd, "echo \"model_spec_val_add :line %d > val? %s\" | cat >> /mnt/ipm/log.txt", __LINE__, val);
	//system(cmd);


	int findIndex = ini_find_property(g_ini, INI_GLOBAL_SECTION, key, 0);

	//sprintf (cmd, "! echo \"model_spec_val_add :line %d > findIndex? %d\" | cat >> /mnt/ipm/log.txt", __LINE__, findIndex);
	//system(cmd);

	ini_property_value_set(g_ini, INI_GLOBAL_SECTION, findIndex, val, 0);

	int size = model_spec_all(data);

	//sprintf (cmd, "echo \"model_spec_val_add :line %d > size? %s\" | cat >> /mnt/ipm/log.txt", __LINE__, size);
	//system(cmd);

	{
		fp = fopen(MODEL_INI, "w");
	}

	fwrite(data, 1, size, fp);

	//sprintf (cmd, "echo \"model_spec_val_add :line %d > data? %s\" | cat >> /mnt/ipm/log.txt", __LINE__, data);
	//system(cmd);

	fclose(fp);

	return NULL;
}


const char *base_model_name()
{
	static char *model = NULL;

	if(model == NULL) {
		model = strdup(MODEL_NAME);
	}

	return model;
}

const char *model_name()
{
	static char *model = NULL;
#if 0
#else 
	if(model == NULL) {
		if(model_spec_string("INFO_MODEL_NAME") == NULL) {
			model = strdup(MODEL_NAME);
		}
	}
#endif

	//sprintf (cmd, "echo \"model_name > %s\" | cat >> /mnt/ipm/log.txt", model);
	//system (cmd);


	// printf("model? %s \n", model);
	return model;
}

const char *oem_name()
{
	static char *oem = NULL;

	if(oem == NULL) {
		if(model_spec_string("SYSTEM_MANUFACTURER") == NULL) {
			oem = strdup(OEM_NAME);
		}
	}


	// printf("oem? %s \n", oem);
	return oem;
}

int medel_spec_value_add(const char *key, char *val)
{

	//sprintf (cmd, "echo \"medel_spec_value_add:line %d > start >> key? %s, val? %s 		!!!!!!!!!!!!!\" | cat >> /mnt/ipm/log.txt", __LINE__, key, val);
	//system (cmd);

	int ret = 0;
	if(load_model_ini() < 0) {
		//sprintf (cmd, "echo \"medel_spec_value_add:line %d > ini not ready\" | cat >> /mnt/ipm/log.txt", __LINE__);
		//system (cmd);

		ret = -1;
	}

	// int index = ini_find_property(g_ini, INI_GLOBAL_SECTION, key, -1);
	// sprintf (cmd, "echo \"medel_spec_value_add:line %d > index? %d\" | cat >> /mnt/ipm/log.txt", __LINE__, index);
	// system (cmd);

	// if(index < 0) {
	// 	ret = -2;
	// }


	// const char *v = model_spec_string(key);
	// if(v == NULL) {
	// 	ret = -3;
	// }

	// sprintf (cmd, "echo \"medel_spec_value_add:line %d > v? %s\" | cat >> /mnt/ipm/log.txt", __LINE__, v);
	// system (cmd);


	// model_spec_key_val_add(key, (char *)val);
	model_spec_val_add(key, (char *)val);
	
	//sprintf (cmd, "echo \"medel_spec_value_add:line %d > ret? %d\" | cat >> /mnt/ipm/log.txt", __LINE__, ret);
	//system (cmd);

	return ret;
}


int medel_spec_value_comp(const char *key, char *comp)
{
	if(load_model_ini() < 0) {
		return -1;
	}

	if (strcmp(model_spec_string(key), comp) == 0)
	{
		return 1;
	}

	return 0;
}
