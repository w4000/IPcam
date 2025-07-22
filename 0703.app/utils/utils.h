
#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>
#include "debug.h"
#include <netinet/in.h>

#define bool	int

#ifndef __cplusplus
#ifndef bool
typedef int						bool;
#endif
#endif

typedef int						boolean;
typedef unsigned char			byte;
typedef unsigned char			uchar;
typedef unsigned short int		word;
typedef unsigned short int		uns16;
typedef unsigned long int		dword;
typedef int64_t					int64;
typedef uint64_t				uint64;

typedef unsigned char			__uint8;
typedef unsigned short			__uint16;
typedef unsigned long			__uint32;
typedef unsigned long int		COLORREF;

#ifndef SUCCESS
#define SUCCESS             	0
#endif
#ifndef FAILURE
#define FAILURE            	 	-1
#endif

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

#define K	1024
#define M	(K*K)
#define G	(M*K)

#define ALIGN(value, align)  	((((value) + ((align)-1))/(align))*(align))
#define FLOOR(value, align)   	(((value) / (align))*(align))

// #define DEF2STR(x) #x

int replace_str(char *text,char *src,char *rep);

int start_webserver(int port);
void stop_webserver();

int run_dhcp_client(struct in_addr *request_ip, int timeout);
void stop_dhcp_client();
int is_dhcp_leasefail();
int is_dhcp_renew();

int start_sip();
void stop_sip();

int get_ubenv(char *key);
void set_ubenv(char *key, int val);


#endif /* UTILS_H_ */
