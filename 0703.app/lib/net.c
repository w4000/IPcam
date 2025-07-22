/*
 * net.c
 *
 *	Purpose :
 *		Network library functions.
 *
 ***********************************************************************/
#include "common.h"
#include "net.h"
#include "../setup/setup.h"

/***********************************************************************
 * Server socket creation and initialization.
 **********************************************************************/

/* Create a server socket of internet address family and return
 * the socket.
 * Return:
 *	socket descriptor, on success.
 *	-1, on failure.
 * note : socket option SO_REUSEADDR applied.
 *
 */

#define	IF_BIND_SOCKET(sd, ethname) \
do {\
	if (strlen(ethname)) {	\
		struct ifreq ifr;	\
		memset(&ifr, 0, sizeof(struct ifreq));	\
		snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), ethname);	\
		ioctl(sd, SIOCGIFINDEX, &ifr);	\
		if (setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE,  (void*)&ifr, sizeof(struct ifreq)) < 0) \
			perror("setsockopt: setting SO_BINDTODEVICE");\
	}\
} while (0)




/***************************************************************************
 * Socket configuration.
 ***************************************************************************/
/* Set socket option SO_KEEPALIVE, SO_SNDTIMEO, SO_RCVTIMEO
 * so that prevent idle connection.
 */
int __setsocktimeout(int sd, unsigned int sec) {
//#define __USE_SOCK_KEEPALIVE__
	struct timeval rcv, snd;

	return 0;

#ifdef __USE_SOCK_KEEPALIVE__
	int optval;
    socklen_t optlen = sizeof(optval);

    optval = 1;
    optlen = sizeof(optval);
#endif

	assert (sd >= 0);

	rcv.tv_sec = snd.tv_sec = sec;
	rcv.tv_usec = snd.tv_usec = 0;

	if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &rcv, sizeof(rcv))==-1) {
		perror("setsockopt");
		return -ERR_SOCK;
	}
	if (setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, &snd, sizeof(snd))==-1) {
		perror("setsockopt");
		return -ERR_SOCK;
	}

#if 1
	{

		int size = 0;
		socklen_t len = sizeof(int);
		getsockopt(sd, SOL_SOCKET, SO_SNDBUF, &size, &len);
		size = 0x3ffff;
		setsockopt(sd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));

		size = 0;
		getsockopt(sd, SOL_SOCKET, SO_SNDBUF, &size, &len);
	}
#endif


#ifdef __USE_SOCK_KEEPALIVE__
	if (setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
	    perror("setsockopt");
		return -ERR_SOCK;
	}
#endif

	return 0;
}

int setsocktimeout(int sd) {
#define DEFAULT_SOCK_TIMEOUT_SEC	30
	assert (sd >= 0);

	return __setsocktimeout(sd, (unsigned int)DEFAULT_SOCK_TIMEOUT_SEC);
}

int setsocktimeout2(int sd) {
#define DEFAULT_MSOCK_TIMEOUT_SEC	30
	assert (sd >= 0);

	return __setsocktimeout(sd, (unsigned int)DEFAULT_MSOCK_TIMEOUT_SEC);
}


/***************************************************************************
 * Receive and send through socket.
 ***************************************************************************/

/* Read n bytes from socket.
 * return
 *	0 on success, -ERR_CODE on failure.
 * NOTE
 *	recv() returns 0 when the connection is closed.
 *	Also, socket should have option SO_RCVTIMEO, so that EWOULDBLOCK returned
 *	when time out occurs.
 */
int __recvn (int sd, char * buf, int len) {
	int left = len, retry = 10,  tot = 0;
	int res = 0;

	assert (buf);
	assert (sd >= 0);
	assert (len > 0);

	while (left > 0) {

		res = recv (sd, buf + tot, left, 0);

		if (res < 0) {
			/* perror("send"); */
			if (errno==EWOULDBLOCK)
				return -ERR_TIMEOUT;
			if (retry-- < 0)
				return -ERR_SOCK;
			else
				continue;
		}
		else if (res == 0) {  /* socket disconnected */
			return -ERR_DISCONN;
		}
		else {
			left -= res;
			tot += res;
		}
	}
#ifndef SUPPORT_WEB_9010_PORT
	return 0;
#else
	return res;
#endif
}


/* Send n byte to socket.
 * NOTE
 *	Socket should have option SO_SNDTIMEO, so that EWOULDBLOCK returned
 *	when time out occurs.
 * return
 *	0 on success, -ERR_CODE on failure.
 */
#if 1
int __sendn(int sd, char * msg, int len)
{
	int ret = 0;
	int left = len, tot = 0;
	int res = 0;
	__u64 start_tm = get_time_msec();
#if !defined(NARCH)//(VPN_MODE_ON == 1)
	static int timeout_flag = 0;
	struct timeval now;
#endif	/* VPN_MODE_ON */

	assert (sd >= 0);
	assert (msg);

	while (left > 0) {
		res = send (sd, msg + tot, left, MSG_NOSIGNAL | MSG_DONTWAIT);

		if (res < 0) {
			if (errno==EAGAIN || errno==EWOULDBLOCK)
			{
#if !defined(NARCH)
				gettimeofday(&now, NULL);
#endif
				if (get_time_msec() - start_tm > 15 * 1000) {
#if !defined(NARCH)//(VPN_MODE_ON == 1)
					gettimeofday(&now, NULL);
					printf("[Halt] [sockid = %d] left(size) = %d(%d); current time = %ld.%ld\n", sd, left, len, now.tv_sec, now.tv_usec);
					timeout_flag = 0;
#endif	/* VPN_MODE_ON */
					ret = -ERR_TIMEOUT;	// transmission time out occurred.
					goto err;
				}
				else
				{
#if !defined(NARCH)//(VPN_MODE_ON == 1)
					if (timeout_flag == 0)
					{
						gettimeofday(&now, NULL);
						timeout_flag = 1;
					}
#endif	/* VPN_MODE_ON */
					continue;
				}
			}
			else
			{
#if !defined(NARCH)//(VPN_MODE_ON == 1)
				gettimeofday(&now, NULL);
#endif	/* VPN_MODE_ON */
				perror ("Internal Send Error");

				if (errno==EPIPE) {
					ret = -ERR_DISCONN;	// socket disconnected
				}
				else
					ret = -ERR_SOCK;

				goto err;
			}
		}
		else if (res == 0) {
			ret = -ERR_DISCONN;
			goto err;
		}
		else {
			left -= res;
			tot += res;
		}
	}

err:
#ifndef SUPPORT_WEB_9010_PORT
	return ret;
#else
	return res;
#endif	/* SUPPORT_WEB_9010_PORT */
}
#else
#define SEND_UNIT_SZ	4*1024
int __sendn(int sd, char * msg, int len)
{
	int ret = 0;
	int left = len, tot = 0;
	int res = 0;
	__u64 start_tm = get_time_msec();

	assert (sd >= 0);
	assert (msg);

	while (left > 0) {
		if(left < SEND_UNIT_SZ) {
			res = send (sd, msg + tot, left, MSG_NOSIGNAL | MSG_DONTWAIT);
		}
		else {
			res = send (sd, msg + tot, SEND_UNIT_SZ, MSG_NOSIGNAL | MSG_DONTWAIT);
		}

		if (res < 0) {
			if (errno==EAGAIN || errno==EWOULDBLOCK)
			{
				printf("send Error1 = %d\n", res);
				if (get_time_msec() - start_tm > 15 * 1000) {
					ret = -ERR_TIMEOUT;	// transmission time out occurred.
					goto err;
				}
				else
					continue;
			}
			else
			{
				printf("send Error2 = %d\n", res);
				perror ("Internal Send Error");

				if (errno==EPIPE)
					ret = -ERR_DISCONN;	// socket disconnected
				else
					ret = -ERR_SOCK;

				goto err;
			}

		}
		else if (res == 0) {
			ret = -ERR_DISCONN;
			goto err;
		}
		else {
			left -= res;
			tot += res;
		}
	}

err:

	return ret;
}
#endif


/***************************************************************************
 * Getting interface address.
 ***************************************************************************/
int if_get_ip(const char *dev, struct in_addr * addr)
{
	int   sockfd = -1;

    struct ifreq ifr;
    struct sockaddr_in *sap;
    int ret = -1;

    assert (dev);
    assert (addr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		goto out;

    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ-1] = '\0';
    ifr.ifr_addr.sa_family = AF_INET;

    if (ioctl (sockfd, SIOCGIFADDR, &ifr) < 0)
		goto out;

    sap  = (struct sockaddr_in*)&ifr.ifr_addr;
    * addr = * ((struct in_addr *)& sap->sin_addr);

    ret = 0;

out:
    if (sockfd >= 0)
		close (sockfd);

    return ret;
}


// Return 6 bytes of MAC.
int if_get_mac (const char *dev, uchar * buff)
{
	int i;
    int sockfd = -1;
    uchar *pdata = NULL;
    struct ifreq  ifr;
    int ret = -1;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		goto out;

    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    ifr.ifr_hwaddr.sa_family = AF_INET;

    if (ioctl (sockfd, SIOCGIFHWADDR, &ifr) < 0)
		goto out;

    pdata = (uchar*)ifr.ifr_hwaddr.sa_data;

    for (i = 0; i < 6; i++)
		buff[i] = *pdata++;


    ret = 0;

out:
    if (sockfd >= 0)
		close (sockfd);

	return ret;
}


int if_set_gateway (char * device, char *gw)
{
	char cmd [128];
	int cnt = 0;

	assert (gw);


	sprintf(cmd, "/sbin/route del default gw 0.0.0.0 %s", device);
	while (__system (cmd) == 0) {
		if(cnt++ > 10) {
			break;
		}
	}

	sprintf(cmd, "/sbin/route add default gw %s %s", gw, device);
	if (__system (cmd) != 0){
		return -1;
	}

	return 0;
}

int if_set_net (char * device, char * ip, char * netmask, char *gw)
{
	char cmd [96];

	assert (ip);
	assert (netmask);
	assert (gw);

	sprintf(cmd, "/sbin/ifconfig %s %s netmask %s", device, ip, netmask);
	printf("%s\n", cmd);
	if (__system (cmd) != 0){
		//return -1;
	}

	if(if_set_gateway(device, gw) < 0) {
		if(if_set_gateway(device, ip) < 0) {
			return -1;
		}
	}

	return 0;
}


int __creat_clisock_nb (struct sockaddr_in * svraddr, int block, int window_sz, const char* eth, long wait_sec, long wait_usec)
{
	int		opt;
	int		r = -1;
    fd_set	rset, wset;
    struct timeval	tval;

	int		sd = -1;  

	if ((sd=socket(AF_INET, SOCK_STREAM, 0)) == -1) {   
		goto err;
	}

	if (eth)
		IF_BIND_SOCKET (sd, eth);

	opt = 1;
	ioctl (sd, FIONBIO, & opt);	


	r = connect(sd, (struct sockaddr *)svraddr, sizeof(struct sockaddr_in));
	if (r < 0) {
		if (errno != EINPROGRESS)
			goto err;
	}
	else if (r == 0) 
		goto success;

	FD_ZERO (& rset);
	FD_SET (sd, & rset);
	wset = rset;
	tval.tv_sec = wait_sec;
	tval.tv_usec = wait_usec;

	r = select (sd + 1, & rset, & wset, NULL, & tval);

	if (r == 0) { 
		goto err;
	}
	else if (FD_ISSET (sd, & rset) || FD_ISSET (sd, & wset)) {
		int	error = 1;
		socklen_t	len = sizeof(error);

		getsockopt (sd, SOL_SOCKET, SO_ERROR, & error, & len);
		if (! error)
			goto success;
	}

err:
	if (sd >=0 )
		close(sd);
	return -1;

success:
	if (block)	{
		opt = 0;
		ioctl (sd, FIONBIO, & opt);
	}

	__setsocktimeout (sd, 10);

#if _USE_SET_SOCKET_BUFFER_SZ_
	if (window_sz > 0) {
	#if _USE_SET_SOCKET_SND_BUFFER_SZ_
		setsockopt (sd, SOL_SOCKET, SO_SNDBUF, (char *) & window_sz, sizeof (window_sz));
	#endif	/* _USE_SET_SOCKET_RCV_BUFFER_SZ_ */
	#if _USE_SET_SOCKET_RCV_BUFFER_SZ_
		setsockopt (sd, SOL_SOCKET, SO_RCVBUF, (char *) & window_sz, sizeof (window_sz));
	#endif	/* _USE_SET_SOCKET_RCV_BUFFER_SZ_ */
	}
#endif	/* _USE_SET_SOCKET_BUFFER_SZ_ */

	{
		struct linger ling;
		ling.l_onoff = 1;
		ling.l_linger = 0;
		setsockopt(sd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
	}
	return sd;
}


int creat_clisock_nb (struct sockaddr_in * svraddr, int block, int window_sz, const char* eth)
{
	return __creat_clisock_nb (svraddr, block, window_sz, eth, 3, 0);
}

extern int set_net_speed(int speed10Mbps)
{
	char cmd[128];

	if(speed10Mbps == 1 && get_net_speed() == 10) {
		return 0;
	}

	if(speed10Mbps) {
		sprintf(cmd, "ethtool -s eth0 speed 10 duplex full autoneg off");
	}
	else {
		sprintf(cmd, "ethtool -s eth0 speed 100 duplex full autoneg on");
	}

	__system(cmd);

	return 0;
}

#define ETH0_SYSFS_SPEED_PATH "/sys/class/net/eth0/speed"

extern int get_net_speed()
{
	int speed = 100;
	FILE *fp;

	fp = fopen(ETH0_SYSFS_SPEED_PATH, "r");
	if(fp) {
		fscanf(fp, "%d", &speed);
		fclose(fp);
	}

	return speed;
}

extern int get_net_auto_negotiation()
{
	FILE *fp = NULL;
	char cmd[128];
	char buf[1024];
	char *p;
	int ret = 0;


	
	sprintf(cmd, "ethtool eth0 > /tmp/eth0_status");
	__system(cmd);

	sprintf(cmd, "cat /tmp/eth0_status");

	BEGIN_SYSTEM(fp, cmd, buf)
		p = strstr(buf, "Auto-negotiation: ");
		if(p != NULL) {
			if(strstr(p, "off") != NULL)
				ret = 0;
			else
				ret = 1;
			break;
		}
	END_SYSTEM

	if(fp) {
		fclose(fp);
		fp = NULL;
	}

	return ret;
}
