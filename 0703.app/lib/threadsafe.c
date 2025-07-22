#include "common.h"
pthread_mutex_t	system_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_mem_chk_cnt = 0;



static inline void* _static__mem_malloc(size_t block, size_t cnt)
{
	void * p;

	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, block * cnt);
	p = malloc (block * cnt);

	if (p == NULL)
		perror ("malloc");

	return p;
}

static inline void* _static__mem_calloc(size_t block, size_t cnt)
{
	void * p;

	p = calloc (block, cnt);

	if (p == NULL)
		perror ("malloc");

	return p;
}

void print_mem_count(void)
{
	// printf("g_mem_chk_cnt: %d \n", g_mem_chk_cnt);
}

inline void * ___mem_malloc (size_t size)
{
	g_mem_chk_cnt++;
	void* p = _static__mem_malloc(1, size);
	return p;
}


inline void * ___mem_calloc (size_t block, size_t cnt)
{
	void *p = _static__mem_calloc(block, cnt);
	return p;
}

inline void * _mem_realloc (void * block, size_t size)
{
	void * p;

	p = realloc (block, size);
	if (p == NULL)
		perror ("realloc");

	return p;
}


void * _mem_memalign (size_t alignment, size_t size)
{
	void * p;

	p = memalign(alignment, size);
	if (p == NULL)
		perror ("memalign");

	return p;
}


void ___mem_free (void * ptr)
{
	g_mem_chk_cnt--;
	// pthread_mutex_lock (& system_lock);
	free (ptr);
	// pthread_mutex_unlock (& system_lock);
}

int __system (const char *command) {
	int ret;


	pthread_mutex_lock (& system_lock);
	ret = system (command);
	pthread_mutex_unlock (& system_lock);

	return ret;
}

time_t __mktime (struct tm * tm)
{
	if (tm)
		tm->tm_isdst = 0;

	return mktime (tm);
}

int threadsafe_gethostbyname(const char* szHostname, struct sockaddr_in* server_addr, int port)
{
	int ret = 1;

	if (strlen(szHostname))
	{
		struct hostent h_ent, *h_result;
		char buf[2048] = "";
		int h_ret, h_err;

		h_ret = gethostbyname2_r(szHostname, AF_INET, &h_ent, buf, sizeof(buf), &h_result, &h_err);
		if (h_ret == 0 && h_err == 0 && h_ent.h_addr_list[0]) {
			struct in_addr addr;
			char ipaddr[16];

			addr.s_addr = *((int*)h_ent.h_addr_list[0]);
			threadsafe_inet_ntoa(&addr, ipaddr, sizeof(ipaddr));

			if (server_addr)
			{
				server_addr->sin_family = AF_INET;
				server_addr->sin_port = (port == 0) ? htons(80) : htons(port);
				server_addr->sin_addr.s_addr = inet_addr(ipaddr);   // server_addr.sin_addr.s_addr = ..
			}

		} else {
			ret = -1;
		}
	}

	return ret;
}

char* threadsafe_inet_ntoa(const struct in_addr * src, char * buf, int bufsz)
{
	assert (src);
	assert (buf);
	assert (bufsz >= 16);

	if (! inet_ntop (AF_INET, (const void *) src, buf, (socklen_t) bufsz)) {
		strncpy (buf, "0.0.0.0", bufsz);
	}

	return buf;
}



inline void * __realloc_release (void * ptr, size_t size)
{
	return realloc(ptr, size);
}