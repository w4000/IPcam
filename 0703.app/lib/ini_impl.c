#include "ini.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ini_impl.h"
#include "utils/debug.h"

typedef struct _INI_PRIV_S {
	ini_t *ini;
	char file[128];
} INI_PRIV_S;

static int is_valid_ini(INI_T ini)
{
	INI_PRIV_S *priv;
	
	if(ini == NULL) {
		return -1;
	}

	priv = (INI_PRIV_S *)ini;
	if(priv->ini == NULL) {
		return -1;
	}

	return 0;
}

INI_T ini_init(const char *file)
{
	INI_PRIV_S *priv;
	ini_t *ini;
	int size;
	char *data;
	FILE *fp = fopen(file, "r");
	if(fp == NULL) {
		fp = fopen(file, "w");
		if(fp == NULL) {
			goto Fail;
		}
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, size + 1);
	data = (char*)malloc(size + 1);
	fread(data, 1, size, fp);
	data[size] = '\0';
	fclose(fp);

	ini = ini_load(data, 0);
	//char cmd[1024];
	// sprintf (cmd, "echo \"func:%s, line: %d > data? %s\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__, data); 
	//system (cmd);

	free(data);

	if(ini == NULL) {
		goto Fail;
	}

	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(INI_PRIV_S));
	priv = (INI_PRIV_S *)malloc(sizeof(INI_PRIV_S));
	priv->ini = ini;
	sprintf(priv->file, "%s", file);

	return (INI_T)priv;

Fail:
	// printf("%s::%d err!!!\n", __FUNCTION__, __LINE__);
	return NULL;
}

void ini_fini(INI_T ini)
{
	INI_PRIV_S *priv;
	
	if(is_valid_ini(ini) < 0) {
		return;
	}

	priv = (INI_PRIV_S *)ini;

	int size = ini_save(priv->ini, NULL, 0);

	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, size);
	char *data = (char *)malloc(size);
	size = ini_save(priv->ini, data, size);
	ini_destroy(priv->ini);

	FILE* fp = fopen(priv->file, "w");
	fwrite(data, 1, size, fp);
	fclose(fp);
	free(data);

	free(priv);
}

INI_T ini_load_str(const char *str)
{
	INI_PRIV_S *priv;
	ini_t *ini;

	ini = ini_load(str, 0);

	if(ini == NULL) {
		goto Fail;
	}

	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(INI_PRIV_S));
	priv = (INI_PRIV_S *)malloc(sizeof(INI_PRIV_S));
	priv->ini = ini;

	return (INI_T)priv;

Fail:
	// printf("%s::%d err!!!\n", __FUNCTION__, __LINE__);
	return NULL;
}

void ini_save_str(INI_T ini, const char *file, char *str)
{
	INI_PRIV_S *priv;
	
	if(is_valid_ini(ini) < 0) {
		return;
	}

	priv = (INI_PRIV_S *)ini;
	
	int index = ini_find_property(priv->ini, INI_GLOBAL_SECTION, "MD5", 0);
	if(index != INI_NOT_FOUND) {
		ini_property_remove(priv->ini, INI_GLOBAL_SECTION, index);
	}

	int size = ini_save(priv->ini, NULL, 0);

	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, size);
	char *data = (char *)malloc(size);
	size = ini_save(priv->ini, data, size);
	ini_destroy(priv->ini);
	free(priv);

	FILE* fp = fopen(file, "w");
	fwrite(data, 1, size, fp);
	fflush(fp);
	fclose(fp);

	if(str != NULL) {
		sprintf(str, "%s", data);
	}

	free(data);
}

const char *ini_get_str(INI_T ini, const char *section, const char *key)
{
	//char cmd[1024];
	if(is_valid_ini(ini) < 0) {
		return NULL;
	}


	INI_PRIV_S *priv = (INI_PRIV_S *)ini;
	int isection = 0;
	if(section != NULL) {
		isection = ini_find_section(priv->ini, section, 0);
		// printf("! %s:%d > isection? %d\n", __func__, __LINE__, isection);

		//sprintf (cmd, "echo \"func:%s, line: %d > isection? %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__, isection); 
		//system (cmd);		
		if(isection == INI_NOT_FOUND) {
			return NULL;
		}
	}

	//sprintf (cmd, "echo \"func:%s, line: %d > isection? %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__, isection); 
	//system (cmd);

	isection = 0;

	int index = ini_find_property(priv->ini, isection, key, 0);
	// printf("! %s:%d > index? %d\n", __func__, __LINE__, index);
	//sprintf (cmd, "echo \"func:%s, line: %d > index? %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__, index); 
	//system (cmd);


	if(index == INI_NOT_FOUND) {
		return NULL;
	}

	char const* val = ini_property_value(priv->ini, isection, index );
	
	return val;
}

const int ini_get_num(INI_T ini, const char *section, const char *key)
{
	const char* val = ini_get_str(ini, section, key);
	
	if(val == NULL) {
		return INI_FAIL;
	}

	return atoi(val);
}

int ini_set_str(INI_T ini, const char *section, const char *key, const char *val)
{
	if(is_valid_ini(ini) < 0) {
		return -1;
	}

	INI_PRIV_S *priv = (INI_PRIV_S *)ini;
	int isection = INI_GLOBAL_SECTION;
	if(section != NULL) {
		isection = ini_find_section(priv->ini, section, 0);
		if(isection == INI_NOT_FOUND) {
			isection = ini_section_add(priv->ini, section, 0);
			if(isection == INI_NOT_FOUND) {
				return INI_FAIL;
			}
		}
	}

	int index = ini_find_property(priv->ini, isection, key, 0);
	if(index != INI_NOT_FOUND) {
		ini_property_remove(priv->ini, isection, index);
	}
	ini_property_add(priv->ini, isection, key, 0, val, 0);
	
	return 0;
}

int ini_set_num(INI_T ini, const char *section, const char *key, int val)
{
	const char sval[16] = "";
	
	sscanf(sval, "%d", &val);
	return ini_set_str(ini, section, key, sval);
}

int get_ini_section(INI_T ini, const char* str_section)
{
	if(is_valid_ini(ini) < 0) {
		return INI_NOT_FOUND;
	}

	INI_PRIV_S *priv = (INI_PRIV_S *)ini;
	int section = ini_find_section(priv->ini, str_section, 0);
	if(INI_NOT_FOUND == section) {
		section = ini_section_add(priv->ini, str_section, 0);
	}
	return section;
}

const char* get_ini_str_value(INI_T ini, int section, const char* key)
{
	if(is_valid_ini(ini) < 0) {
		return NULL;
	}

	INI_PRIV_S *priv = (INI_PRIV_S *)ini;
	char* result = NULL;
	int index = ini_find_property(priv->ini, section, key, 0);
	if(index == INI_NOT_FOUND) {
		return NULL;
	}
	else {
		result = (char*)ini_property_value(priv->ini, section, index);
	}
	return (const char*)result;
}

int get_ini_integer_value(INI_T ini, int section, const char* key)
{
	int result = -1;
	if(is_valid_ini(ini) < 0) {
		return result;
	}

	INI_PRIV_S *priv = (INI_PRIV_S *)ini;
	const char* c_value = get_ini_str_value(priv, section, key);

	if(c_value != NULL) {
		result = atoi(c_value);
	}

	return result;
}

int set_ini_str_value(INI_T ini, int section, const char* key, const char* value)
{
	if(is_valid_ini(ini) < 0) {
		return -1;
	}

	INI_PRIV_S *priv = (INI_PRIV_S *)ini;
	int index = ini_find_property(priv->ini, section, key, 0);
	if(index == INI_NOT_FOUND) {
		ini_property_add(priv->ini, section, key, 0, value, 0);
	}
	else {
		ini_property_value_set(priv->ini, section, index, value, 0);
	}
	return 0;
}

int set_ini_integer_value(INI_T ini, int section, const char* key, int value)
{
	if(is_valid_ini(ini) < 0) {
		return -1;
	}

	INI_PRIV_S *priv = (INI_PRIV_S *)ini;
	char c_value[128];
	snprintf(c_value, sizeof(c_value), "%d", value);
	set_ini_str_value(priv, section, (char*)key, c_value);

	return 0;
}

int save_ini_file(INI_T ini)
{
	if(is_valid_ini(ini) < 0) {
		return -1;
	}
	INI_PRIV_S *priv = (INI_PRIV_S *)ini;

	int size = ini_save(priv->ini, NULL, 0);
	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, size);
	char *data = (char *)malloc(size);
	FILE* fp = fopen(priv->file, "w");

	size = ini_save(priv->ini, data, size);
	fwrite(data, 1, size, fp);

	ini_destroy(priv->ini);
	fclose(fp);
	free(data);
	return 0;
}
