/*
 * net.h
 *
 * Purpose:
 *	- Network socket related stuffs.
 *********************************************************/
#ifndef __NET_H
#define __NET_H

#define LISTENQ_LEN 5	// Linstening server socket's queue length.

#ifndef uchar
typedef unsigned char uchar;
#endif
// Accept connection request arrived at server socket.
// @s : server socket.
// @adr : Ptr to struct sockaddr_in. client's address will be stored here.
#define accept_svrsock(s, adr)	\
	({\
		int __sock__;\
		socklen_t __sz__ = sizeof (struct sockaddr_in);\
		__sock__ = accept ((s), (struct sockaddr *) (adr), & __sz__);\
		if (__sock__ >= 0)\
			setsocktimeout (__sock__);\
		__sock__;\
	})

#define accept_msvrsock(s, adr)	\
	({\
		int __sock__;\
		socklen_t __sz__ = sizeof (struct sockaddr_in);\
		__sock__ = accept ((s), (struct sockaddr *) (adr), & __sz__);\
		if (__sock__ >= 0)\
			setsocktimeout2 (__sock__);\
		__sock__;\
	})
	
extern int _creat_clisock(struct sockaddr_in *);
extern int set_fd_set(fd_set *, int [], size_t);
extern int __recvn(int, char *, int);
extern int __sendn(int, char *, int);
extern int __setsocktimeout(int, unsigned int);
extern int setsocktimeout(int);
extern int setsocktimeout2(int);
extern int if_get_ip(const char *, struct in_addr *);
extern int if_get_mac(const char *, uchar *);
extern int if_set_gateway (char * device, char *gw);
extern int if_set_net (char *, char *, char *, char *);

extern int __creat_clisock_nb (struct sockaddr_in * svraddr, int block, int window_sz, const char* eth, long wait_sec, long wait_usec);
extern int creat_clisock_nb (struct sockaddr_in * svraddr, int block, int window_sz, const char* eth);


extern int set_net_speed(int speed10Mbps);
extern int get_net_speed();
extern int get_net_auto_negotiation();

#endif
