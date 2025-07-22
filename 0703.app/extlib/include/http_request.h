#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* http_cgi_request.h */
typedef int (*cb_http_cgi_request)(void* userParam, const char* data);

struct _ipaddr {
	char str_ipaddr[32];
	char str_mac[32];
	int size;
	int port;
	int connectionType;
};

typedef enum {
	MAC_ADDR,
	CONN_TYPE,
} _PAYLOAD_TYPE;

int is_http_test (struct _ipaddr* ipaddr);
int check_ip_address(struct _ipaddr *_ip_addr, const char *cmd_str);

#endif /*__HTTP_REQUEST_H__*/
