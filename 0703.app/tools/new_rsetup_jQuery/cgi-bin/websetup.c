
#include "common2.h"
#include "jQuery_common.h"

static _CAMSETUP r_setup;
static _CAMSETUP_EXT r_setup_ext;

static char page[MAX_PAGE_SZ];

static void replace_param(char* page, int lang)
{
	char buf[1024];
	char strbuf[1024];


	buf[0] = '\0';


	char info[128];
	char model[64];
	char base_model[64];
	char ver[64];
	sprintf(info, "<font color=white>%s</font> %s %s", get_base_model(base_model, &r_setup), get_model(model, &r_setup), get_software_version(ver, &r_setup));


}

static void replace_resource_string(char* page, int lang)
{

	char* szRES_SETUP[] = RES_ID_ARRAY(RES_SETUP);
	char* szRES_LOGOUT[] = RES_ID_ARRAY(RES_LOGOUT);
	replace_form_once(page, "##RES_LOGOUT##", szRES_LOGOUT[lang]);
	replace_form(page, "##RES_SETUP##", szRES_SETUP[lang]);

	char* szRES_VIDEO[] = RES_ID_ARRAY(RES_VIDEO);
	char* szRES_VIDEO_STREAM[] = RES_ID_ARRAY(RES_VIDEO_STREAM);
	char* szRES_VIDEO_SETUP[] = RES_ID_ARRAY(RES_VIDEO_SETUP);
	char* szRES_CAMERA_SETUP[] = RES_ID_ARRAY(RES_CAMERA_SETUP);

	replace_form(page, "##RES_VIDEO_STREAM##", szRES_VIDEO_STREAM[lang]);
	replace_form(page, "##RES_VIDEO_SETUP##", szRES_VIDEO_SETUP[lang]);
	replace_form(page, "##RES_CAMERA_SETUP##", szRES_CAMERA_SETUP[lang]);

	char* szRES_NETWORK[] = RES_ID_ARRAY(RES_NETWORK);
	char* szRES_INTERFACE[] = RES_ID_ARRAY(RES_INTERFACE);
	char* szRES_PORT[] = RES_ID_ARRAY(RES_PORT);
	char* szRES_RTSP[] = RES_ID_ARRAY(RES_RTSP);
	char* szRES_ONVIF[] = RES_ID_ARRAY(RES_ONVIF);

	replace_form(page, "##RES_NETWORK##", szRES_NETWORK[lang]);
	replace_form(page, "##RES_INTERFACE##", szRES_INTERFACE[lang]);
	replace_form(page, "##RES_PORT##", szRES_PORT[lang]);
	replace_form(page, "##RES_RTSP##", szRES_RTSP[lang]);
	replace_form(page, "##RES_ONVIF##", szRES_ONVIF[lang]);



	char* szRES_SERVICE[] = RES_ID_ARRAY(RES_SERVICE);
	replace_form(page, "##RES_SERVICE##", szRES_SERVICE[lang]);

	char* szRES_SENSOR_SETUP[] = RES_ID_ARRAY(RES_SENSOR_SETUP);
	replace_form(page, "##RES_SENSOR_SETUP##", szRES_SENSOR_SETUP[lang]);

	replace_form(page, "##RES_VIDEO_AUDIO##", szRES_VIDEO[lang]);



	char* szRES_SYSTEM[] = RES_ID_ARRAY(RES_SYSTEM);
	char* szRES_SYSTEM_INFO[] = RES_ID_ARRAY(RES_SYSTEM_INFO);
	char* szRES_DATE_TIME[] = RES_ID_ARRAY(RES_DATE_TIME);
	char* szRES_USER_MANAGEMENT[] = RES_ID_ARRAY(RES_USER_MANAGEMENT);
	char* szRES_SYSTEM_DEFAULT[] = RES_ID_ARRAY(RES_SYSTEM_DEFAULT);
	char* szRES_SYSTEM_REBOOT[] = RES_ID_ARRAY(RES_SYSTEM_REBOOT);
	char* szRES_SYSTEM_UPGRADE[] = RES_ID_ARRAY(RES_SYSTEM_UPGRADE);
	replace_form(page, "##RES_SYSTEM##", szRES_SYSTEM[lang]);
	replace_form(page, "##RES_SYSTEM_INFO##", szRES_SYSTEM_INFO[lang]);
	replace_form(page, "##RES_DATE_TIME##", szRES_DATE_TIME[lang]);
	replace_form(page, "##RES_USER_MANAGEMENT##", szRES_USER_MANAGEMENT[lang]);
	replace_form(page, "##RES_SYSTEM_DEFAULT##", szRES_SYSTEM_DEFAULT[lang]);
	replace_form(page, "##RES_SYSTEM_REBOOT##", szRES_SYSTEM_REBOOT[lang]);
	replace_form(page, "##RES_SYSTEM_UPGRADE##", szRES_SYSTEM_UPGRADE[lang]);



	char* szRES_MSG_FAILED_TO_RECEIVE_DATA[] = RES_ID_ARRAY(RES_MSG_FAILED_TO_RECEIVE_DATA);
	replace_form(page, "##RES_MSG_FAILED_TO_RECEIVE_DATA##", szRES_MSG_FAILED_TO_RECEIVE_DATA[lang]);



}

static void replace_option(char* page, int lang)
{


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

//retry:

	

	load_form(JQUERY_HTML_SETUP, page, lang);

	char* pdata = (char*)cgi_param("sidemenu");
	if (pdata)
		replace_form_once(page, "##PARAM_START_SUBMENU##", pdata);
	else
		replace_form_once(page, "##PARAM_START_SUBMENU##", "ipc_video_stream");


	replace_param(page, lang);
	replace_resource_string(page, lang);
	replace_option(page, lang);

	printf("%s", page);

	cgi_end();

	return 0;
}


