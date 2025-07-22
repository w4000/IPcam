#ifndef __THREAD_SAFE_H
#define __THREAD_SAFE_H


extern inline void * __realloc_release (void * ptr, size_t size);
#define __realloc(ptr, sz)	__realloc_release((ptr), (sz))


#define BEGIN_SYSTEM(fp, cmd, buf)\
	do {\
		if ((fp = popen (cmd, "r")) == NULL) {\
			perror ("popen");\
			pthread_mutex_unlock (& system_lock);\
			break;\
		}\
		while (fgets (buf, sizeof (buf), fp)) {\
			debug (buf);



#define END_SYSTEM\
		}\
		pclose (fp);\
		fp = NULL;\
	} while (0);





	extern inline void * __malloc_release (size_t);
	extern inline void __free_release (void *);

	#define __malloc(sz)	__malloc_release((sz))
	#define __free(ptr)	__free_release((ptr))


void print_mem_count(void);

#define _mem_malloc2(size)		___mem_malloc(size)
#define _mem_calloc2(block, cnt)	___mem_calloc(block, cnt)
#define _mem_free(buf)					___mem_free(buf)

extern pthread_mutex_t		system_lock;
extern void prepare (void);
extern void parent (void);
extern void child (void);
extern inline void * ___mem_malloc (size_t);
extern inline void * ___mem_calloc (size_t, size_t);
extern inline void   ___mem_free (void *);
extern inline void * _mem_realloc (void *, size_t);
extern void * _mem_memalign (size_t, size_t);
extern int __system (const char *);
extern time_t __mktime (struct tm *);
extern time_t __mktime2 (struct tm *);

#include <netinet/in.h>
extern int threadsafe_gethostbyname(const char* szHostname, struct sockaddr_in* server_addr, int port);
extern char* threadsafe_inet_ntoa(const struct in_addr * src, char * buf, int bufsz);

#endif
