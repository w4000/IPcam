#ifndef __INI_IMPL_H__
#define __INI_IMPL_H__

#define INI_FAIL	(-99999)

typedef void* INI_T;

INI_T ini_init(const char *file);
void ini_fini(INI_T ini);

INI_T ini_load_str(const char *str);
void ini_save_str(INI_T ini, const char *file, char *str);

const char *ini_get_str(INI_T ini, const char *section, const char *key);
const int ini_get_num(INI_T ini, const char *section, const char *key);

int ini_set_str(INI_T ini, const char *section, const char *key, const char *val);
int ini_set_num(INI_T ini, const char *section, const char *key, int val);

int get_ini_section(INI_T ini, const char* str_section);

const char* get_ini_str_value(INI_T ini, int section, const char* key);
int get_ini_integer_value(INI_T ini, int section, const char* key);
int set_ini_str_value(INI_T ini, int section, const char* key, const char* value);
int set_ini_integer_value(INI_T ini, int section, const char* key, int value);
int save_ini_file(INI_T ini);
#endif /* __INI_IMPL_H__ */
