#ifndef __IPC_SERVER__
#define __IPC_SERVER__

int ipc_server_init(const char *id);
void ipc_server_fini();
int ipc_server_broadcast(int msg_id);

#endif /*__IPC_SERVER__*/
