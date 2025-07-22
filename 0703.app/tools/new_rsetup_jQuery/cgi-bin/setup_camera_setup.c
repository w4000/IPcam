#include "common2.h"
#include "jQuery_common.h"
#include "ajax_common.h"

static _CAMSETUP r_setup;
static _CAMSETUP_EXT r_setup_ext;

static char page[MAX_PAGE_SZ];


int __system (const char *command) {
	int ret = 0;


	// pthread_mutex_lock (& system_lock);
	ret = system (command);
	// pthread_mutex_unlock (& system_lock);

	return ret;
}

static void replace_param(char* page, int lang)
{
	char str[1024], buf[1024];

	buf[0] = '\0';

	int width = PREVIEW_WIDTH, height = PREVIEW_HEIGHT;
	preview_size(&r_setup, &width, &height);


	sprintf(str, "%d", width);
	replace_form(page, "##PARAM_WEBMON_WIDTH##", str);
	sprintf(str, "%d", height);
	replace_form(page, "##PARAM_WEBMON_HEIGHT##", str);

	sprintf(str, g_errfmt, "printf");
	replace_form_once(page, "<!--##PARAM_ISP_ERROR##-->", str);
	replace_form(page,"##PARAM_SUPPORT_LPR##","false");




	// replace_form_once(page, "##PARAM_LOGIN_ID##", "admin");
	// replace_form_once(page, "##LOGIN_PWD##",       "1234");



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

static void replace_resource_string(char* page, int lang)
{
	char* szRES_CAMERA_CONFIG[] = RES_ID_ARRAY(RES_CAMERA_CONFIG);
	replace_form(page, "##RES_CAMERA_CONFIG##", szRES_CAMERA_CONFIG[lang]);

	char* szRES_IMAGE_QUALITY[] = RES_ID_ARRAY(RES_IMAGE_QUALITY);
	char* szRES_WHITE_BALANCE[] = RES_ID_ARRAY(RES_WHITE_BALANCE);
	char* szRES_EXPOSURE[] = RES_ID_ARRAY(RES_EXPOSURE);
	char* szRES_BACKLIGHT[] = RES_ID_ARRAY(RES_BACKLIGHT);
	char* szRES_DAY_NIGHT[] = RES_ID_ARRAY(RES_DAY_NIGHT);

	replace_form(page, "##RES_IMAGE_QUALITY##", szRES_IMAGE_QUALITY[lang]);
	replace_form(page, "##RES_WHITE_BALANCE##", szRES_WHITE_BALANCE[lang]);
	replace_form(page, "##RES_EXPOSURE##", szRES_EXPOSURE[lang]);
	replace_form(page, "##RES_BACKLIGHT##", szRES_BACKLIGHT[lang]);
	replace_form(page, "##RES_DAY_NIGHT##", szRES_DAY_NIGHT[lang]);
	//replace_form(page, "##RES_LPR_EXP##", szRES_DAY_NIGHT[lang]);

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
	load_form(JQUERY_HTML_SETUP_CAMERA_SETUP, page, lang);

	replace_param(page, lang);
	replace_resource_string(page, lang);

	if (ajax_check_output(page, lang) < 0) {
		if (lang != 0) {
			lang = 0;
			goto retry;
		}
	}

	cgi_end();
	return 0;
}
