#ifndef __IPC_CLIENT__
#define __IPC_CLIENT__

int ipc_client_send(const char *id, unsigned int msg_id, int data_size, void *data);

int ipc_client_connect(const char *id);
void ipc_client_disconnect(int fd);
int ipc_client_request(int fd, unsigned int msg_id, int size, void *data);
int ipc_client_response(int fd, void *data, int size);

#endif /*__IPC_CLIENT__*/
