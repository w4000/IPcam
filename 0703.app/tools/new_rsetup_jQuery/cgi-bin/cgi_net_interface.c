
#include "common2.h"
#include "jQuery_common.h"
#include "ajax_common.h"

static _CAMSETUP r_setup;
static _CAMSETUP_EXT r_setup_ext;

static char page[MAX_PAGE_SZ];
static void replace_option(char* page, int lang)
{
	
}

static void replace_param(char* page, int lang)
{
	char str[1024], buf[1024];
	char *selptr;

	buf[0] = '\0';

	selptr = (r_setup.net.type == NETTYPE_LAN)? "selected" : "" ;
	sprintf(str, "<option %s value=1>%s</option>\n", selptr, DFS_LAN);
	strcat(buf, str);

	selptr = (r_setup.net.type == NETTYPE_DHCP)? "selected" : "" ;
	sprintf(str, "<option %s value=2>%s</option>\n", selptr, DFS_DHCP);
	strcat(buf, str);

	replace_form_once(page, "<!--##PARAM_NETWORK_TYPE##-->", buf);

	sprintf(str, "%s", inet_ntoa(r_setup.net.ip));
	replace_form(page, "##PARAM_IPADDR##", str);

	sprintf(str, "%s", inet_ntoa(r_setup.net.netmask));
	replace_form(page, "##PARAM_SUBNET##", str);

	sprintf(str, "%s", inet_ntoa(r_setup.net.gateway));
	replace_form(page, "##PARAM_GATEWAY##", str);

	unsigned char mac_addr[6];
	__if_get_mac(ETHERNET_DEV, mac_addr);
	sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X",
			mac_addr [0], mac_addr [1], mac_addr [2], mac_addr [3], mac_addr [4], mac_addr [5]);
	replace_form(page, "##PARAM_MACADDR##", str);

	buf[0] = '\0';

	selptr = (r_setup.net.speed == 0)? "selected" : "" ;
	sprintf(str, "<option %s value=0>10/100Base-TX</option>\n", selptr);
	strcat(buf, str);

	selptr = (r_setup.net.speed == 1)? "selected" : "" ;
	sprintf(str, "<option %s value=1>10Base-T</option>\n", selptr);
	strcat(buf, str);

	replace_form_once(page, "<!--##PARAM_NETWORK_SPEED##-->", buf);

	// Webgate
}

static void replace_resource_string(char* page, int lang)
{
	char* szRES_NETWORK_CONFIG[] = RES_ID_ARRAY(RES_NETWORK_CONFIG);
	char* szRES_NETWORK_TYPE[] = RES_ID_ARRAY(RES_NETWORK_TYPE);
	char* szRES_IP_ADDRESS[] = RES_ID_ARRAY(RES_IP_ADDRESS);
	char* szRES_SUBNET_MASK[] = RES_ID_ARRAY(RES_SUBNET_MASK);
	char* szRES_GATEWAY[] = RES_ID_ARRAY(RES_GATEWAY);
	char* szRES_MAC_ADDRESS[] = RES_ID_ARRAY(RES_MAC_ADDRESS);
	char* szRES_MSG_IP_ADDR[] = RES_ID_ARRAY(RES_MSG_IP_ADDR);
	char* szRES_MSG_SUBNET_MASK[] = RES_ID_ARRAY(RES_MSG_SUBNET_MASK);
	char* szRES_MSG_GATEWAY[] = RES_ID_ARRAY(RES_MSG_GATEWAY);
	char* szRES_NETWORK_SPEED[] = RES_ID_ARRAY(RES_NETWORK_SPEED);

	replace_form(page, "##RES_NETWORK_CONFIG##", szRES_NETWORK_CONFIG[lang]);
	replace_form(page, "##RES_NETWORK_TYPE##", szRES_NETWORK_TYPE[lang]);
	replace_form(page, "##RES_IP_ADDRESS##", szRES_IP_ADDRESS[lang]);
	replace_form(page, "##RES_SUBNET_MASK##", szRES_SUBNET_MASK[lang]);
	replace_form(page, "##RES_GATEWAY##", szRES_GATEWAY[lang]);
	replace_form(page, "##RES_MAC_ADDRESS##", szRES_MAC_ADDRESS[lang]);
	replace_form(page, "##RES_MSG_IP_ADDR##", szRES_MSG_IP_ADDR[lang]);
	replace_form(page, "##RES_MSG_SUBNET_MASK##", szRES_MSG_SUBNET_MASK[lang]);
	replace_form(page, "##RES_MSG_GATEWAY##", szRES_MSG_GATEWAY[lang]);
	replace_form(page, "##RES_NETWORK_SPEED##", szRES_NETWORK_SPEED[lang]);

	char* szRES_WEBGATE_WARNING2[] = RES_ID_ARRAY(RES_WEBGATE_WARNING2);
	replace_form(page, "##RES_WEBGATE_WARNING2##", szRES_WEBGATE_WARNING2[lang]);

	char* szRES_SAVE[] = RES_ID_ARRAY(RES_SAVE);
	replace_form(page, "##RES_SAVE##", szRES_SAVE[lang]);
}

int main(void)
{

	cgi_init();
	cgi_session_start();

	cgi_process_form();
	cgi_init_headers();

	if (!check_session()) {
		cgi_end();
		return 0;
	}

	load_setup (&r_setup, &r_setup_ext);
	int lang = r_setup.lang;

retry:
	load_form(JQUERY_HTML_SETUP_AJAX_NET_INTERFACE, page, lang);

	replace_param(page, lang);
	replace_resource_string(page, lang);
	replace_option(page, lang);

	if (ajax_check_output(page, lang) < 0) {
		if (lang != 0) {
			lang = 0;
			goto retry;
		}
	}

	cgi_end();
	return 0;
}
