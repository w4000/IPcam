
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "netsvr/websocket.h"

#include "ini.h"
#include "debug.h"

int replace_str(char *text,char *src,char *rep)
{
	char *pfound;
	char page_buf[1024];

	if ((strlen(src) == 0)/* || (strlen(rep) == 0)*/)
		return -1;

	while ((pfound = strstr(text, src)) != NULL) {

		strcpy(page_buf, text);
		page_buf[pfound -text] = '\0';

		strcat(page_buf, rep);

		pfound += strlen(src);

		strcat(page_buf, pfound);
		strcpy(text, page_buf);
	}

	return 0;
}

#define minimum(a, b) (((a) < (b)) ? (a) : (b))
static char* get_param_str_with_end(const char* outbuf, const char* param, const char* endstr, char* data, int datasz)
{
	char* p;
	if ((p = (char *)strcasestr(outbuf, param)) != NULL)
	{
		char* start = p + strlen(param);
		char* end = (char *)strcasestr(start, endstr);
		if (end)
		{
			strncpy(data, start, minimum(datasz, end - start));
			data[minimum(datasz, end - start)] = 0;
			return data;
		}
		else
			return NULL;
	}
	return NULL;
}


#include <netinet/in.h>
static void make_ssl_key()
{
	char cmd[128];
	char local_ip[32];
	char ext_ip[32];
	struct in_addr localip;
	if_get_ip("eth0", &localip);
	threadsafe_inet_ntoa(&localip, local_ip, sizeof(local_ip));


	sprintf(cmd, "/edvr/lighttpd/ssl.sh %s %s", local_ip, ext_ip);
	__system (cmd);
}

int start_webserver(int port)
{

	make_ssl_key();
	if(port <= 0 || port > 65535) {
		return -1;
	}

	char cmd[128];
	sprintf(cmd, "/edvr/lighttpd/lighttpd.sh restart %d", port);
	__system (cmd);



	return 0;
}

void stop_webserver()
{
	__system ("/edvr/lighttpd/lighttpd.sh stop");
}

void stop_dhcp_client()
{
	__system("rm -rf /usr/share/udhcpc/netinfo");
	__system ("killall udhcpc"); // Previous connection could be DHCP.
}

int run_dhcp_client(struct in_addr *request_ip, int timeout)
{
	int i;
	char cmd[128];

	stop_dhcp_client();

	if(request_ip != NULL && inet_ntoa(*request_ip) != 0) {
		char ip[32];
		ip [31] = '\0';
		strncpy(ip, (char *)inet_ntoa(*request_ip), 31);
		sprintf(cmd, "udhcpc -i eth0 -r %s -t %d -T 1 -b ", ip, timeout);
	}
	else {
		sprintf(cmd, "udhcpc -i eth0 -t %d -T 1 -b ", timeout);
	}
	__system(cmd);

	for(i = 0; i < timeout+1; i++) {
		printf("Wait dhcp response %d\n", i);
		if(is_dhcp_leasefail()) {
			printf("DHCP lease fail!!!\n");
			break;
		}

		if(access("/usr/share/udhcpc/netinfo", F_OK) != -1) {
			printf("DHCP bound!!!\n");
			break;
		}

		sleep(1);
	}

	return 0;
}

int is_dhcp_leasefail()
{
	if(access("/tmp/leasefail", F_OK) != -1) {
		remove("/tmp/leasefail");
		return 1;
	}
	return 0;
}

int is_dhcp_renew()
{
	if(access("/tmp/renew", F_OK) != -1) {
		remove("/tmp/renew");
		return 1;
	}

	return 0;
}

int start_sip()
{
	char cmd[128];
	sprintf(cmd, "/edvr/sip/sip.sh restart");
	return __system(cmd);
}

void stop_sip()
{
	char cmd[128];
	sprintf(cmd, "/edvr/sip/sip.sh stop");
	__system(cmd);
}

int get_ubenv(char *key)
{
	return 0;
}

void set_ubenv(char *key, int val)
{
	return;
}
