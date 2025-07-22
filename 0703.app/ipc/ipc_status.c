#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdio.h> 
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "ipc_status.h"

#ifdef EXT_BUILD
#define INI_IMPLEMENTATION
#endif
#include "ini.h"

#define IPC_STATUS_KEY 0x18

static char *attach_to_shm()
{
	static key_t key = -1;
	static int shmid = -1;
	if(key < 0) {
		key = IPC_STATUS_KEY;
		
		// shmget returns an identifier in shmid 
		shmid = shmget(key, 2048, 0666|IPC_CREAT);
		if(shmid < 0) {
			perror("shmget failed: ");
			return NULL;
		}
	}

	// shmat to attach to shared memory 
	char *m = (char*)shmat(shmid, (void*)0, 0); 
	if(m == (char *)-1) {
		perror("shmat failed: ");
		return NULL;
	}

	return m;
}

static void detach_from_shm(char *m)
{
	//detach from shared memory  
	shmdt(m);
}

int ipc_status_init()
{
	int shmid = -1;

	shmid = shmget((key_t)IPC_STATUS_KEY, 2048, 0666);
	if(shmid < 0) {
		return 0;
	}
		
	shmctl(shmid, IPC_RMID, NULL); 

	return 0;
}

int ipc_status_set(const char *section, const char *key, const char *val)
{
	ini_t *ini;
	char *m = attach_to_shm();
	if(m == NULL) {
		return -1;
	}
	
	ini = ini_load(m, 0);
	if(ini == NULL) {
		goto Fail;
	}

	int isection = INI_GLOBAL_SECTION;
	if(section != NULL) {
		isection = ini_find_section(ini, section, 0);
		if(isection == INI_NOT_FOUND) {
			isection = ini_section_add(ini, section, 0);
			if(isection == INI_NOT_FOUND) {
				goto Fail;
			}
		}
	}
	
	int index = ini_find_property(ini, isection, key, 0);
	if(index != INI_NOT_FOUND) {
		ini_property_remove(ini, isection, index);
	}
	ini_property_add(ini, isection, key, 0, val, 0);

	int size = ini_save(ini, NULL, 0);
	ini_save(ini, m, size);

	ini_destroy(ini);
Fail:
	detach_from_shm(m);

	return 0;
}

const char *ipc_status_get_str(const char *section, const char *key)
{
	ini_t *ini;
	char *m = attach_to_shm();
	if(m == NULL) {
		return NULL;
	}

	ini = ini_load(m, 0);
	if(ini == NULL) {
		detach_from_shm(m);
		return NULL;
	}
	detach_from_shm(m);

	int isection = INI_GLOBAL_SECTION;
	if(section != NULL) {
		isection = ini_find_section(ini, section, 0);
		if(isection == INI_NOT_FOUND) {
			ini_destroy(ini);
			return NULL;
		}
	}

	int index = ini_find_property(ini, isection, key, 0);
	if(index == INI_NOT_FOUND) {
		ini_destroy(ini);
		return NULL;
	}

	char const* val = ini_property_value(ini, isection, index);

	ini_destroy(ini);

	return val;
}

int ipc_status_get_num(const char *section, const char *key)
{
	const char *val = ipc_status_get_str(section, key);

	if(val == NULL) {
		return -65500;
	}

	return atoi(val);
}
