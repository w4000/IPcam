#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "setup_ini.h"
#include "../lib/ini_impl.h"

#define SETUP_INI_FILE "/edvr/cam_setup.ini"
static INI_T setup_ini = NULL;


INI_T setup_ini_load()
{
	// char cmd[1024];
	// sprintf (cmd, "echo \"func:%s, line: %d \" | cat >> /mnt/ipm/log.txt", __func__, __LINE__); 
	// system (cmd);

	setup_ini = ini_init(SETUP_INI_FILE);

	return 0;
}

const char *setup_ini_get_str(const char *section, const char *key, char *def)
{
	// if (setup_ini == NULL);
	// {
	// 	setup_ini_load();
	// }

	//char cmd[1024];
	const char *val;

	val = ini_get_str(setup_ini, section, key);

	// sprintf (cmd, "echo \"func:%s, line: %d > key? %s, val? %s\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__, key, val); 
	// system (cmd);

	if(val == NULL) {
		ini_set_str(setup_ini, section, key, def);
		val = ini_get_str(setup_ini, section, key);
		
		//sprintf (cmd, "echo \"func:%s, line: %d > val? %s\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__, val); 
		//system (cmd);

	}

	// sprintf (cmd, "echo \"func:%s, line: %d > val? %s\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__, val); 
	// system (cmd);


	return val;
}

int setup_ini_set_str(const char *section, const char *key, char *val)
{
	return ini_set_str(setup_ini, NULL, key, val);
}




const char *setup_ini_get_str2(const char *section, const char *key)
{
	if (setup_ini == NULL)
	{
		setup_ini_load();
	}

	//char cmd[1024];
	const char *val;

	val = ini_get_str(setup_ini, section, key);


	//sprintf (cmd, "echo \"func:%s, line: %d > val? %s\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__, val); 
	//system (cmd);
	

	return val;
}
