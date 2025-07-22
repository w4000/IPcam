#ifndef __SETUP_INI_H__
#define __SETUP_INI_H__

#include "lib/ini_impl.h"

INI_T setup_ini_load();
const char *setup_ini_get_str(const char *section, const char *key, char *def);
// int setup_ini_get_num(const char *section, const char *key, int def)
#endif /* __SETUP_INI_H__ */
