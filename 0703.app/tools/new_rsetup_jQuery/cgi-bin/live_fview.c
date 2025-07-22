
#include "common2.h"
#include "jQuery_common.h"



static _CAMSETUP r_setup;
static _CAMSETUP_EXT r_setup_ext;

static char page[MAX_PAGE_SZ];

static void replace_param(char* page, int lang)
{
	char str[1024], buf[1024*2];

	buf[0] = '\0';

	int usr_no = 0;

	sprintf(str, "%d", r_setup.cam[0].fps);
	replace_form_once(page, "##PARAM_FPS_MAIN##", str);
	sprintf(str, "%d", r_setup.cam[1].fps);
	replace_form_once(page, "##PARAM_FPS_SUB##", str);
	sprintf(str, "%d", r_setup.cam[2].fps);
	replace_form_once(page, "##PARAM_FPS_SUB2##", str);

	sprintf(str, "%s", inet_ntoa(r_setup.net.ip));
	replace_form_once(page, "##PARAM_IP_ADDR##", str);

	sprintf(str, "%d", r_setup.net.port_tcp);
	replace_form_once(page, "##PARAM_TCP_PORT##", str);

	if (cgi_session_var_exists("logon_user_no"))
		usr_no = atoi(cgi_session_var("logon_user_no"));

	{
	int i;
	char cmd[1024];

	char *name = NULL;
	char *passwd = NULL;
	for(i=0; i<10; i++)
	{

		if (strlen(SETUP_USR_NAME(&r_setup, i)) && strlen(SETUP_USR_PWD(&r_setup, i)))
		{
			name = SETUP_USR_NAME(&r_setup, i);
			passwd = SETUP_USR_PWD(&r_setup, i);
		}
	}

	if (name != NULL && passwd != NULL)
	{
		replace_form_once(page, "##PARAM_LOGIN_ID##", name);
		replace_form_once(page, "##LOGIN_PWD##", passwd);		
	}
	else 
	{
		replace_form_once(page, "##PARAM_LOGIN_ID##", "admin");
		replace_form_once(page, "##LOGIN_PWD##", "passwd");
	}

	}

#if 0
#else 
	buf[0] = 0;
	sprintf(str, "<option value=\"main\">##RES_STREAM_CH##1</option>");
	strcat(buf, str);
#endif 
	replace_form_once(page, "##PARAM_PROFILE##", buf);

	replace_form_once(page, "##PARAM_MODEL##", get_model(buf, &r_setup));
	replace_form_once(page, "##PARAM_SW_VERSION##", get_software_version(buf, &r_setup));

	sprintf(str, "%s", (r_setup.net.type == NETTYPE_DHCP) ? "DHCP" : "STATIC");
	replace_form_once(page, "##PARAM_NET_TYPE##", str);

	sprintf(str, "%s", inet_ntoa(r_setup.net.ip));
	replace_form_once(page, "##PARAM_NET_IP##", str);

	sprintf(str, "%s", inet_ntoa(r_setup.net.netmask));
	replace_form_once(page, "##PARAM_NET_SUBNET##", str);

	sprintf(str, "%s", inet_ntoa(r_setup.net.gateway));
	replace_form_once(page, "##PARAM_NET_GATEWAY##", str);

	unsigned char mac_addr[6];
	__if_get_mac(ETHERNET_DEV, mac_addr);
	sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X",
			mac_addr [0], mac_addr [1], mac_addr [2], mac_addr [3], mac_addr [4], mac_addr [5]);
	replace_form(page, "##PARAM_MACADDR##", str);

	char* szRES_ID[] = RES_ID_ARRAY(RES_ID);
	replace_form(page, "##RES_ID##", szRES_ID[lang]);


	replace_java_player_path(page);

}

static void replace_resource_string(char* page, int lang)
{
	char* szRES_LOGOUT[] = RES_ID_ARRAY(RES_LOGOUT);
	replace_form_once(page, "##RES_LOGOUT##", szRES_LOGOUT[lang]);

	char* szRES_STREAM_CH[] = RES_ID_ARRAY(RES_STREAM_CH);
	char* szRES_MAIN_STREAM[] = RES_ID_ARRAY(RES_MAIN_STREAM);
	char* szRES_SUB_STREAM[] = RES_ID_ARRAY(RES_SUB_STREAM);
	char* szRES_SUB2_STREAM[] = RES_ID_ARRAY(RES_SUB2_STREAM);
	char* szRES_LIVE_MONITORING[] = RES_ID_ARRAY(RES_LIVE_MONITORING);
	char* szRES_SETUP[] = RES_ID_ARRAY(RES_SETUP);
	char* szRES_TOOLS[] = RES_ID_ARRAY(RES_TOOLS);
	char* szRES_SYSTEM_INFO[] = RES_ID_ARRAY(RES_SYSTEM_INFO);
	char* szRES_HIDE_MENU[] = RES_ID_ARRAY(RES_HIDE_MENU);
	char* szRES_SHOW_MENU[] = RES_ID_ARRAY(RES_SHOW_MENU);
	char* szRES_MODEL[] = RES_ID_ARRAY(RES_MODEL);
	char* szRES_VERSION[] = RES_ID_ARRAY(RES_VERSION);
	char* szRES_NET_TYPE[] = RES_ID_ARRAY(RES_NET_TYPE);
	char* szRES_NET_IP[] = RES_ID_ARRAY(RES_NET_IP);
	char* szRES_NET_SUBNET[] = RES_ID_ARRAY(RES_NET_SUBNET);
	char* szRES_NET_GATEWAY[] = RES_ID_ARRAY(RES_NET_GATEWAY);
	char* szRES_MAC_ADDRESS[] = RES_ID_ARRAY(RES_MAC_ADDRESS);
	char* szRES_MOTION[] = RES_ID_ARRAY(RES_MOTION);
	char* szRES_SENSOR[] = RES_ID_ARRAY(RES_SENSOR);


	replace_form_once(page, "##RES_MAIN_STREAM##", szRES_MAIN_STREAM[lang]);
	replace_form_once(page, "##RES_SUB_STREAM##", szRES_SUB_STREAM[lang]);
	replace_form_once(page, "##RES_SUB2_STREAM##", szRES_SUB2_STREAM[lang]);
	replace_form(page, "##RES_SUB_STREAM##", szRES_SUB_STREAM[lang]);
	replace_form(page, "##RES_STREAM_CH##", szRES_STREAM_CH[lang]);
	replace_form(page, "##RES_LIVE_MONITORING##", szRES_LIVE_MONITORING[lang]);
	replace_form_once(page, "##RES_SETUP##", szRES_SETUP[lang]);
	replace_form_once(page, "##RES_TOOLS##", szRES_TOOLS[lang]);
	replace_form_once(page, "##RES_SYSTEM_INFO##", szRES_SYSTEM_INFO[lang]);
	replace_form_once(page, "##RES_HIDE_MENU##", szRES_HIDE_MENU[lang]);
	replace_form_once(page, "##RES_SHOW_MENU##", szRES_SHOW_MENU[lang]);
	replace_form_once(page, "##RES_MODEL##", szRES_MODEL[lang]);
	replace_form_once(page, "##RES_VERSION##", szRES_VERSION[lang]);
	replace_form_once(page, "##RES_NET_TYPE##", szRES_NET_TYPE[lang]);
	replace_form_once(page, "##RES_NET_IP##", szRES_NET_IP[lang]);
	replace_form_once(page, "##RES_NET_SUBNET##", szRES_NET_SUBNET[lang]);
	replace_form_once(page, "##RES_NET_GATEWAY##", szRES_NET_GATEWAY[lang]);
	replace_form_once(page, "##RES_MAC_ADDRESS##", szRES_MAC_ADDRESS[lang]);
	replace_form_once(page, "##RES_MOTION##", szRES_MOTION[lang]);
	replace_form_once(page, "##RES_SENSOR##", szRES_SENSOR[lang]);

	char* szRES_WEBGATE[] = RES_ID_ARRAY(RES_WEBGATE);
	replace_form(page, "##RES_WEBGATE##", szRES_WEBGATE[lang]);
	char* szRES_CANCEL[] = RES_ID_ARRAY(RES_CANCEL);
	replace_form(page, "##RES_CANCEL##", szRES_CANCEL[lang]);
	char* szRES_ASPECT_RATIO[] = RES_ID_ARRAY(RES_ASPECT_RATIO);	
	replace_form_once(page, "##RES_ASPECT_RATIO##", szRES_ASPECT_RATIO[lang]);
}



#include <sys/time.h>
#include <sys/resource.h>

int main(void)
{
	cgi_init();
	cgi_session_start();
	cgi_process_form();
	cgi_init_headers();


	load_setup (&r_setup, &r_setup_ext);
	int lang = r_setup.lang;

//retry:

	load_form(JQUERY_HTML_LIVE_MONITORING, page, lang);


	replace_param(page, lang);
	replace_resource_string(page, lang);
	printf("%s", page);
	cgi_end();


	return 0;
}

