
#include "common2.h"
#include "jQuery_common.h"
#include "ajax_common.h"

static _CAMSETUP r_setup;
static _CAMSETUP_EXT r_setup_ext;

static char page[MAX_PAGE_SZ];

static void replace_param(char* page, int lang)
{


	enum { RSA_SIZE = 1024 };
	char str[RSA_SIZE], buf[RSA_SIZE * 30];
	int i;

	buf[0] = '\0';
	for (i = 0; i < 10; i++) {

#if 0
#else 
		snprintf(str, sizeof(str), ",u_id_%d: $(\"#u_id_%d\").val()\n", i, i);
		strsafecat(buf, str, sizeof(buf));
		snprintf(str, sizeof(str), ",u_pwd_%d: $(\"#u_pwd_%d\").val()\n", i, i);
		strsafecat(buf, str, sizeof(buf));
		snprintf(str, sizeof(str), ",u_desc_%d: $(\"#u_desc_%d\").val()\n", i, i);
		strsafecat(buf, str, sizeof(buf));
#endif

	}
	replace_form(page, "//##PARAM_USER_DATA##", buf);


	buf[0] = '\0';
	for (i = 0; i < 10; i++) {
		

#if 0
#else 
		snprintf(str, sizeof(str), "<input type=\"hidden\" id=\"u_id_%d\" value=\"%s\" >\n", i, (unsigned char*)SETUP_USR_NAME(&r_setup, i));
		strsafecat(buf, str, sizeof(buf));



		snprintf(str, sizeof(str), "<input type=\"hidden\" id=\"u_pwd_%d\" value=\"%s\">\n", i, (unsigned char*)SETUP_USR_PWD(&r_setup, i));
		strsafecat(buf, str, sizeof(buf));

		
		snprintf(str, sizeof(str), "<input type=\"hidden\" id=\"u_desc_%d\" value=\"%s\" >\n", i, (unsigned char*)SETUP_USR_DESC(&r_setup, i));
		strsafecat(buf, str, sizeof(buf));
#endif 


	}
	replace_form(page, "<!--##PARAM_USER_VALUE##-->", buf);


	// const char *need_priv_key = "1";
	char *need_priv_key = "1";
	replace_form(page, "##PARAM_NEED_PRIVATE_KEY##", need_priv_key);
}

static void replace_resource_string(char* page, int lang)
{
	char* szRES_ENTER_NEW_USER[] = RES_ID_ARRAY(RES_ENTER_NEW_USER);
	char* szRES_MSG_USER_IS_NEEDED[] = RES_ID_ARRAY(RES_MSG_USER_IS_NEEDED);
	char* szRES_MSG_PASSWORD[] = RES_ID_ARRAY(RES_MSG_PASSWORD);
	char* szRES_MSG_NOT_MATCH_PWD[] = RES_ID_ARRAY(RES_MSG_NOT_MATCH_PWD);
	char* szRES_MSG_USER_EXIST[] = RES_ID_ARRAY(RES_MSG_USER_EXIST);
	char* szRES_MSG_MAX_NUM[] = RES_ID_ARRAY(RES_MSG_MAX_NUM);
	char* szRES_USER_CONFIG[] = RES_ID_ARRAY(RES_USER_CONFIG);
	char* szRES_NO[] = RES_ID_ARRAY(RES_NO);
	char* szRES_USER_ID[] = RES_ID_ARRAY(RES_USER_ID);
	char* szRES_DESCRIPTION[] = RES_ID_ARRAY(RES_DESCRIPTION);
	char* szRES_PERMISSION[] = RES_ID_ARRAY(RES_PERMISSION);
	char* szRES_NEW[] = RES_ID_ARRAY(RES_NEW);
	char* szRES_ADD[] = RES_ID_ARRAY(RES_ADD);
	char* szRES_MODIFY[] = RES_ID_ARRAY(RES_MODIFY);
	char* szRES_PASSWORD[] = RES_ID_ARRAY(RES_PASSWORD);
	char* szRES_PWD_CONFIRM[] = RES_ID_ARRAY(RES_PWD_CONFIRM);
	char* szRES_LIVE_MONITORING[] = RES_ID_ARRAY(RES_LIVE_MONITORING);
	char* szRES_SETUP[] = RES_ID_ARRAY(RES_SETUP);

	replace_form(page, "##RES_ENTER_NEW_USER##", szRES_ENTER_NEW_USER[lang]);
	replace_form(page, "##RES_MSG_USER_IS_NEEDED##", szRES_MSG_USER_IS_NEEDED[lang]);
	replace_form(page, "##RES_MSG_PASSWORD##", szRES_MSG_PASSWORD[lang]);
	replace_form(page, "##RES_MSG_NOT_MATCH_PWD##", szRES_MSG_NOT_MATCH_PWD[lang]);
	replace_form(page, "##RES_MSG_USER_EXIST##", szRES_MSG_USER_EXIST[lang]);
	replace_form(page, "##RES_MSG_MAX_NUM##", szRES_MSG_MAX_NUM[lang]);
	replace_form(page, "##RES_USER_CONFIG##", szRES_USER_CONFIG[lang]);
	replace_form(page, "##RES_NO##", szRES_NO[lang]);
	replace_form(page, "##RES_USER_ID##", szRES_USER_ID[lang]);
	replace_form(page, "##RES_DESCRIPTION##", szRES_DESCRIPTION[lang]);
	replace_form(page, "##RES_PERMISSION##", szRES_PERMISSION[lang]);
	replace_form(page, "##RES_NEW##", szRES_NEW[lang]);
	replace_form(page, "##RES_ADD##", szRES_ADD[lang]);
	replace_form(page, "##RES_MODIFY##", szRES_MODIFY[lang]);
	replace_form(page, "##RES_PASSWORD##", szRES_PASSWORD[lang]);
	replace_form(page, "##RES_PWD_CONFIRM##", szRES_PWD_CONFIRM[lang]);
	replace_form(page, "##RES_LIVE_MONITORING##", szRES_LIVE_MONITORING[lang]);
	replace_form(page, "##RES_SETUP##", szRES_SETUP[lang]);

	char* szRES_MSG_NEED_ONE_USER[] = RES_ID_ARRAY(RES_MSG_NEED_ONE_USER);
	replace_form(page, "##RES_MSG_NEED_ONE_USER##", szRES_MSG_NEED_ONE_USER[lang]);

	char* szRES_MSG_USERID1[] = RES_ID_ARRAY(RES_MSG_USERID1);
	char* szRES_MSG_USERID2[] = RES_ID_ARRAY(RES_MSG_USERID2);
	char* szRES_MSG_PASSWORD1[] = RES_ID_ARRAY(RES_MSG_PASSWORD1);
	char* szRES_MSG_PASSWORD2[] = RES_ID_ARRAY(RES_MSG_PASSWORD2);
	char* szRES_MSG_PASSWORD3[] = RES_ID_ARRAY(RES_MSG_PASSWORD3);
	// char* szRES_MSG_PASSWORD3_1[] = RES_ID_ARRAY(RES_MSG_PASSWORD3_1);
	char* szRES_MSG_PASSWORD4[] = RES_ID_ARRAY(RES_MSG_PASSWORD4);

	replace_form(page, "##RES_MSG_USERID1##", szRES_MSG_USERID1[lang]);
	replace_form(page, "##RES_MSG_USERID2##", szRES_MSG_USERID2[lang]);
	replace_form(page, "##RES_MSG_PASSWORD1##", szRES_MSG_PASSWORD1[lang]);
	replace_form(page, "##RES_MSG_PASSWORD2##", szRES_MSG_PASSWORD2[lang]);
	replace_form(page, "##RES_MSG_PASSWORD3##", szRES_MSG_PASSWORD3[lang]);
	// replace_form(page, "##RES_MSG_PASSWORD3_1##", szRES_MSG_PASSWORD3_1[lang]);
	replace_form(page, "##RES_MSG_PASSWORD4##", szRES_MSG_PASSWORD4[lang]);

	char* szRES_SAVE[] = RES_ID_ARRAY(RES_SAVE);
	replace_form(page, "##RES_SAVE##", szRES_SAVE[lang]);
	replace_form(page, "##EXCEPTION_ID##", "");

}

//#define _DEBUG_OUTPUT_

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
	load_form(JQUERY_HTML_SETUP_AJAX_SYSTEM_USER, page, lang);

	replace_param(page, lang);
	replace_resource_string(page, lang);

	if (ajax_check_output(page, lang) < 0) {
		if (lang != 0) {
			lang = 0;
			goto retry;
		}
	}

#ifdef _DEBUG_OUTPUT_
	fprintf(fp, page);
	fprintf(fp, "end\r\n"); fflush(fp);
	fclose(fp);
#endif

	cgi_end();
	return 0;
}
