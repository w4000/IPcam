#ifndef __IPC_PROTOCOL__
#define __IPC_PROTOCOL__

#define IPC_PROTOCOL_MAGIC 0x90da6a

typedef struct _IPC_PROTOCOL_HEADER_T {
	unsigned int magic;		/* Fix: 0x90da6a */
	unsigned int id;		/* enum IPC_PROTCOL_ID_E */
	unsigned int data_size;	/* attached data size (exclude header size) */
} IPC_PROTOCOL_HEADER_T;

typedef enum IPC_PROTOCOL_ID_E {
	IPC_ID_FIN,
	IPC_ID_UPDATE_SETUP,
	IPC_ID_UPDATE_ISP,
	IPC_ID_COUNT

} IPC_PROTOCOL_ID_E;

#endif /*__IPC_PROTOCOL__*/
