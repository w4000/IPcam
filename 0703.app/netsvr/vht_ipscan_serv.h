
#ifndef NETSVR_VHT_IPSCAN_SERV_H_
#define NETSVR_VHT_IPSCAN_SERV_H_


// IP SCAN PORT(UDP)
#define 	VHT_GROUP 			"255.255.255.255"
#define		VHT_PORT_IPSCAN		64988

// Magic code.
#define		VHT_MAGIC	0x7668

// Protocol mode.
#define		VHT_MODE_REQ_GET_IPINFO		0x01
#define		VHT_MODE_REQ_SET_IPINFO		0x02
#define		VHT_MODE_RSP_GET_IPINFO		0x100
#define		VHT_MODE_REQ_GET_IPINFO_EXT	0x03	//(1.1.0.10 or later)
#define		VHT_MODE_RSP_GET_IPINFO_EXT	0x101	//(1.1.0.10 or later)


#if 1//! __DEVICE_NVR__
// Poll to read data
#define	SELECT_READ(__fd__, __msec__) \
	do {\
		fd_set rfds;		\
		struct timeval tv;	\
		tv.tv_sec = (__msec__) / 1000;		\
		tv.tv_usec = (__msec__ * 1000) % 1000000; \
		FD_ZERO (& rfds);	\
		FD_SET (__fd__, & rfds);	\
		if (select(__fd__ + 1, & rfds, NULL, NULL, & tv) <= 0)	\
			break; \
		if (! FD_ISSET(__fd__, & rfds)) \
			break;
#define SELECT_END \
	} while (0);
#endif


extern int ipscan_vht_reply (int sd);
extern void * ipscan_vht_thread (void * arg);


#endif /* NETSVR_VHT_IPSCAN_SERV_H_ */
