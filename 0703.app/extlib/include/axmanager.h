#ifndef _AXMANAGER_H_
#define _AXMANAGER_H_

#include <stdio.h>
#include <stdlib.h>

//type defines
typedef int AXRET;

//error defines
#define AXRET_SUCCESS			0
#define AXRET_FAIL				1

//values
#define AXVAL_ON 	1
#define AXVAL_OFF	0

//Macros
#define IN
#define OUT
#define INOUT


#define AXVAL_MAX_ARRAY 	10
#define AUTOEXEC_MAXCMD 31 // max 30, last one is always null

//error defines
#define AXRET_ERR_MEMALLOC				0x30000010
#define AXRET_ERR_INSTANCE				0x30000011
#define AXRET_ERR_INVALID_SIZE			0x30000012
#define AXRET_ERR_INVALID_PORT			0x30000013
#define	AXRET_ERR_ALREADY_INSTANCE		0x30000014
#define	AXRET_ERR_UART_NORESPONSE		0x30000015
#define AXRET_ERR_INVALID_PARAM			0x30000016
#define AXRET_ERR_GET_EXTRACONF			0x30000017
#define AXRET_ERR_FILEOPEN				0x30000018
#define AXRET_ERR_TYPE_MISMATCH			0x30000019
#define AXRET_ERR_UART_TRANSMIT			0x30000020
#define AXRET_ERR_INVALID_FILESIZE		0x30000021

//functions
AXRET AXIO_INIT(IN char *tty);
AXRET AXIO_TERM();

char *AXIO_SDK_VERSION();
AXRET AXIO_VPN_VERSION(OUT char *version, IN int len);
//char *AXIO_VPN_VERSION();

////////////////////////////////////////////////////////////////////////////////
#define AX_IPFILTER_MAX_RULE 10
struct axs_ipfilter_rule{
	char ip[16];
	char netmask[16];
};

struct axs_ipfilter_ruleset{
	int count;
	struct axs_ipfilter_rule *rule[AX_IPFILTER_MAX_RULE];
};

struct axs_ipconfig{
	int dhcp;
	char ipv4[16];
	char netmask[16];
	char gw[16];
};

struct axs_masquerade_camera_ip{
	int use;
	char ip[16];
};

struct axs_config{
	char server_ip[16];
	int server_port;
	char cipher_suite[32];
	struct axs_ipconfig camera_ip;
	struct axs_ipfilter_ruleset camera_ruleset;
	struct axs_ipconfig poe_ip;
	struct axs_ipfilter_ruleset poe_ruleset;
	struct axs_masquerade_camera_ip masq_camera_ip;
	int bypass_mode;
	int verbose;
};

AXRET AXIO_SET_CONFIG(IN struct axs_config *config, IN int config_size);
AXRET AXIO_GET_CONFIG(OUT struct axs_config  *config, IN int config_size);




////////////////////////////////////////////////////////////////////////////////
//VPN Command
AXRET AXIO_COMMAND_LOGIN();
AXRET AXIO_COMMAND_REBOOT();
AXRET AXIO_COMMAND_RESET();

#endif
