

#include "sys_inc.h"
#include "util.h"

/***************************************************************************************/
HT_API int get_if_nums()
{
#if __WINDOWS_OS__
#elif defined(IOS)
#elif __LINUX_OS__

	SOCKET socket_fd;
	struct ifreq *ifr;
	struct ifconf conf;
	char buff[BUFSIZ];
	int i, num, count = 0;
	
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd <= 0)
	{
		return 0;
	}
	
	conf.ifc_len = BUFSIZ;
	conf.ifc_buf = buff;
	
	ioctl(socket_fd, SIOCGIFCONF, &conf);
	
	num = conf.ifc_len / sizeof(struct ifreq);
	ifr = conf.ifc_req;
    
    for (i=0; i<num; i++)
    {
        if (ifr->ifr_addr.sa_family == AF_INET)
        {
            count++;
        }
        
        ifr++;
    }

	closesocket(socket_fd);
	
	return count;
	
#endif

	return 0;
}

HT_API uint32 get_if_ip(int index)
{
#if __WINDOWS_OS__
#elif defined(IOS)
#elif __LINUX_OS__

	int i;
	SOCKET socket_fd;
	struct ifreq *ifr;
	struct ifconf conf;
	char buff[BUFSIZ];
	int num, count = 0;
    uint32 ip_addr = 0;
	
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	conf.ifc_len = BUFSIZ;
	conf.ifc_buf = buff;
	
	ioctl(socket_fd, SIOCGIFCONF, &conf);
	
	num = conf.ifc_len / sizeof(struct ifreq);
	ifr = conf.ifc_req;
	
	for (i=0; i<num; i++)
	{
        if (ifr->ifr_addr.sa_family != AF_INET)
        {
            ifr++;
            continue;
        }
        
		if (count++ == index)
		{
			struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);

			ioctl(socket_fd, SIOCGIFFLAGS, ifr);
            
			if ((ifr->ifr_flags & IFF_LOOPBACK) != 0)
			{
				ip_addr = 0;
			}
			else
			{
				ip_addr = sin->sin_addr.s_addr;
			}

			break;
		}
		
		ifr++;
	}

	closesocket(socket_fd);
	
	return ip_addr;
	
#endif

	return 0;
}

HT_API uint32 get_route_if_ip(uint32 dst_ip)
{
#if __WINDOWS_OS__
#elif defined(IOS)
#elif __LINUX_OS__

	int i;
	SOCKET socket_fd;
	struct ifreq *ifr;
	struct ifconf conf;
	char buff[BUFSIZ];
	int num;
    uint32 ip_addr = 0;
	
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	conf.ifc_len = BUFSIZ;
	conf.ifc_buf = buff;
	
	ioctl(socket_fd, SIOCGIFCONF, &conf);
	
	num = conf.ifc_len / sizeof(struct ifreq);
	ifr = conf.ifc_req;
	
	for (i=0; i<num; i++)
	{
		struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);
		
		if (ifr->ifr_addr.sa_family != AF_INET)
		{
			ifr++;
			continue;
		}

		ioctl(socket_fd, SIOCGIFFLAGS, ifr);
		
		if (((ifr->ifr_flags & IFF_LOOPBACK) == 0) && (ifr->ifr_flags & IFF_UP))
		{
			ip_addr = sin->sin_addr.s_addr;
			break;
		}
		
		ifr++;
	}

	closesocket(socket_fd);
	
	return ip_addr;
	
#endif

	return 0;
}

HT_API uint32 get_default_if_ip()
{
	struct in_addr addr;
    addr.s_addr = get_route_if_ip(0);

    if (addr.s_addr != 0)
    {
    	return addr.s_addr;
    }
    else
    {
        int i;
    	int nums = get_if_nums();
    	
    	for (i = 0; i < nums; i++)
    	{
    		addr.s_addr = get_if_ip(i);
    		if (addr.s_addr != 0)
    		{
    			return addr.s_addr;
    		}
    	}
    }

    return 0;
}

HT_API const char * get_local_ip()
{
	struct in_addr addr;
    addr.s_addr = get_default_if_ip();

    if (addr.s_addr != 0)
    {
    	return inet_ntoa(addr);
    }
    else
    {
		int i;
    	int nums = get_if_nums();
    	for (i = 0; i < nums; i++)
    	{
    		addr.s_addr = get_if_ip(i);
    		if (addr.s_addr != 0)
    		{
    			return inet_ntoa(addr);
    		}
    	}
    }

    return 0;
}

HT_API int get_default_if_mac(uint8 * mac)
{
#ifdef IOS

#elif __WINDOWS_OS__
    
#elif __LINUX_OS__

	int i;
	SOCKET socket_fd;
	struct ifreq *ifr;
	struct ifreq ifreq;
	struct ifconf conf;
	char buff[BUFSIZ];
	int num;
	
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	conf.ifc_len = BUFSIZ;
	conf.ifc_buf = buff;
	
	ioctl(socket_fd, SIOCGIFCONF, &conf);
	
	num = conf.ifc_len / sizeof(struct ifreq);
	ifr = conf.ifc_req;
	
	for (i=0; i<num; i++)
	{
		if (ifr->ifr_addr.sa_family != AF_INET)
		{
			ifr++;
			continue;
		}
		
		ioctl(socket_fd, SIOCGIFFLAGS, ifr);
		
		if ((ifr->ifr_flags & IFF_LOOPBACK) != 0)
		{
			ifr++;
			continue;
		}

		strncpy(ifreq.ifr_name, ifr->ifr_name, sizeof(ifreq.ifr_name));

        if (ioctl(socket_fd, SIOCGIFHWADDR, &ifreq) < 0) 
        {
        	ifr++;
            continue;
        }

		memcpy(mac, &ifreq.ifr_hwaddr.sa_data, 6);		

		close(socket_fd);
		
		return 0;
	}

	close(socket_fd);
#endif	

	return -1;
}

HT_API const char * get_default_gateway()
{   
	static char gateway[32];	
	
#if 0
#elif __LINUX_OS__
	
    char line[100], *p, *c, *g, *saveptr;
	int ret = 0;
	
    FILE *fp = fopen("/proc/net/route" , "r");
	if (NULL == fp)
	{
		return NULL;
	}

	memset(gateway, 0, sizeof(gateway));
	
    while (fgets(line, 100, fp))
    {
        p = strtok_r(line, " \t", &saveptr);
        c = strtok_r(NULL, " \t", &saveptr);
        g = strtok_r(NULL, " \t", &saveptr);

        if (p != NULL && c != NULL)
        {
            if (strcmp(c, "00000000") == 0)
            {
                if (g)
                {
                    char *p_end;
                    int gw = strtol(g, &p_end, 16);
                    
                    struct in_addr addr;
                    addr.s_addr = gw;
                    
                    strcpy(gateway, inet_ntoa(addr));
                    ret = 1;
                }
                
                break;
            }
        }
    }

	fclose(fp);
	
    if (ret == 0)
    {
    	return NULL;
    }

#endif

	return gateway;
}

HT_API int get_prefix_len_by_mask(const char * mask)
{
	int i;
	int len = 0;
    uint32 n = inet_addr(mask);
	
    n = ntohl(n);    

    for (i = 0; i < 32; i++)
    {
        if (n & (1 << (31 - i)))
        {
            len++;
        }
        else
        {
            break;
        }
    }

    return len;
}


HT_API uint32 get_address_by_name(const char * host_name)
{
	uint32 addr = 0;

	if (is_ip_address(host_name))
	{
		addr = inet_addr(host_name);
	}	
	else
	{
		struct hostent * remoteHost = gethostbyname(host_name);
		if (remoteHost)
		{
			addr = *(unsigned long *)(remoteHost->h_addr);
		}	
	}

	return addr;
}

HT_API char * lowercase(char * str) 
{
	uint32 i;
	
	for (i = 0; i < strlen(str); ++i)
	{
		str[i] = tolower(str[i]);
	}
	
	return str;
}

HT_API char * uppercase(char * str)
{
	uint32 i;
	
	for (i = 0; i < strlen(str); ++i)
	{
		str[i] = toupper(str[i]);
	}
	
	return str;
}


HT_API int unicode(char ** dst, char * src) 
{
	char *ret;
	int l, i;
	
	if (!src) 
	{
		*dst = NULL;
		return 0;
	}
	
	l = MIN(64, (int)strlen(src));

	ret = (char *)malloc(2*l);

	for (i = 0; i < l; ++i)
	{
		ret[2*i] = src[i];
		ret[2*i+1] = '\0';
	}

	*dst = ret;
	
	return 2*l;
}

static char hextab[17] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 0};
static int hexindex[128] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,
        -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};


HT_API SOCKET tcp_connect(const char * hostname, int port, int timeout)
{
    int ret;
	SOCKET fd;
    char portstr[10];
    struct timeval tv;
    struct addrinfo hints = { 0 }, *ai, *cur_ai;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    snprintf(portstr, sizeof(portstr), "%d", port);
    
    ret = getaddrinfo(hostname, portstr, &hints, &ai);
    if (ret) 
    {
        return -1;
    }

    fd = -1;
    
    for (cur_ai = ai; cur_ai; cur_ai = cur_ai->ai_next) 
    {
        fd = socket(cur_ai->ai_family, cur_ai->ai_socktype, cur_ai->ai_protocol);
        if (fd < 0)
        {
            continue;
        }

        tv.tv_sec = timeout/1000;
	    tv.tv_usec = (timeout%1000) * 1000;
	    
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));
        
        if (connect(fd, cur_ai->ai_addr, (int)(cur_ai->ai_addrlen)) < 0) 
        {
            closesocket(fd);
            fd = -1;

            continue;
        } 

        break;  /* okay we got one */
    }

    freeaddrinfo(ai);
    
    return fd;   
}

HT_API SOCKET tcp_connect_timeout(uint32 rip, int port, int timeout)
{
	SOCKET cfd;	
	struct sockaddr_in addr;
	
#if __LINUX_OS__	
	uint32 starttime = sys_os_get_ms();
	struct timeval tv;
#elif __WINDOWS_OS__
    int flag = 0;
	unsigned long ul = 1;
#endif

	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if (cfd <= 0)
	{
		return 0;
    }    
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = rip;
	addr.sin_port = htons((uint16)port);

#if __LINUX_OS__
	
	tv.tv_sec = timeout/1000;
	tv.tv_usec = (timeout%1000) * 1000;
	
	setsockopt(cfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));

	while (connect(cfd, (struct sockaddr *)&addr, sizeof(addr)) == -1 && errno != EISCONN)
	{
        if (sys_os_get_ms() > starttime + timeout)
        {
        	closesocket(cfd);
            return -1;
        }
        
        //if (errno != EINTR) 
		if( errno != EWOULDBLOCK && errno != EINPROGRESS && errno != EAGAIN )
        {
            closesocket(cfd);
            return -1;
        }
    }
    
	return cfd;
	
#elif __WINDOWS_OS__    
    
	ioctlsocket(cfd, FIONBIO, &ul);

	if (connect(cfd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
	{
		fd_set set;
		struct timeval tv;
		
		tv.tv_sec = timeout/1000;
		tv.tv_usec = (timeout%1000) * 1000;
		
		FD_ZERO(&set);
		FD_SET(cfd, &set);

		if (select((int)(cfd+1), NULL, &set, NULL, &tv) > 0)
		{
			int err = 0, len = sizeof(int);
			
			getsockopt(cfd, SOL_SOCKET, SO_ERROR, (char *)&err, (socklen_t*) &len);

			if (err == 0)
			{
				flag = 1;
			}	
		}
	}
	else
	{
	    flag = 1;
	}

    ul = 0;
	ioctlsocket(cfd, FIONBIO, &ul);
		
	if (flag == 1)
	{
		return cfd;
	}
	else
	{
		closesocket(cfd);
		return 0;
	}

#endif	
}


#if __WINDOWS_OS__

#include <sys/timeb.h>

// used to make sure that static variables in gettimeofday() aren't initialized simultaneously by multiple threads
static long initializeLock_gettimeofday = 0;  

HT_API int gettimeofday(struct timeval * tp, int * tz)
{
	static LARGE_INTEGER tickFrequency, epochOffset;

	static BOOL isInitialized = FALSE;

	LARGE_INTEGER tickNow;

	QueryPerformanceCounter(&tickNow);
 
	if (!isInitialized) 
	{
		if (1 == InterlockedIncrement(&initializeLock_gettimeofday)) 
		{
			// For our first call, use "ftime()", so that we get a time with a proper epoch.
			// For subsequent calls, use "QueryPerformanceCount()", because it's more fine-grain.
			struct timeb tb;
			ftime(&tb);
			
			tp->tv_sec = (long)tb.time;
			tp->tv_usec = 1000*tb.millitm;

			// Also get our counter frequency:
			QueryPerformanceFrequency(&tickFrequency);

			// compute an offset to add to subsequent counter times, so we get a proper epoch:
			epochOffset.QuadPart = tp->tv_sec * tickFrequency.QuadPart + (tp->tv_usec * tickFrequency.QuadPart) / 1000000L - tickNow.QuadPart;

			// next caller can use ticks for time calculation
			isInitialized = TRUE; 
			
			return 0;
		} 
		else 
		{
			InterlockedDecrement(&initializeLock_gettimeofday);
			
			// wait until first caller has initialized static values
			while (!isInitialized)
			{
				Sleep(1);
			}
		}
	}

    // adjust our tick count so that we get a proper epoch:
    tickNow.QuadPart += epochOffset.QuadPart;

    tp->tv_sec =  (long)(tickNow.QuadPart / tickFrequency.QuadPart);
    tp->tv_usec = (long)(((tickNow.QuadPart % tickFrequency.QuadPart) * 1000000L) / tickFrequency.QuadPart);

    return 0;
}

#endif


