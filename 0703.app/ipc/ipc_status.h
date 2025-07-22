#ifndef __IPC_STATUS__
#define __IPC_STATUS__

int ipc_status_init();

int ipc_status_set(const char *section, const char *key, const char *val);
int ipc_status_get_num(const char *section, const char *key);
const char *ipc_status_get_str(const char *section, const char *key);

#endif /*__IPC_STATUS__*/
