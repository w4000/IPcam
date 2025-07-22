#include "common2.h"
#include "jQuery_common.h"
//w4000_iframe

static _CAMSETUP r_setup;
static _CAMSETUP_EXT r_setup_ext;

static char page[MAX_PAGE_SZ];


int __system (const char *command) {
	int ret = 0;


	// pthread_mutex_lock (& system_lock);
	// ret = system (command);
	// pthread_mutex_unlock (& system_lock);

	return ret;
}


int create_session_key(char session_key[])
{
	char* ascii = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghiklmnopqrstuvwxyz";
	int i;
	memset(session_key, 0, 65);
	srand( time(NULL) );

	for (i = 0; i < 64; i++)
	{
		session_key[i] = ascii[rand() % strlen(ascii)];
	}

	FILE* fp_key = fopen("/tmp/_key/web.login.session.key", "wt");
	if (fp_key)
	{
		fprintf(fp_key, "%s\n", session_key);
		fclose(fp_key);
	}

	return 0;
}

static void replace_param(char* page, int lang, int arg, int remain_flag)
{
	char str[1024], buf[1024];
	char *selptr;
	int i;

	char** lang_val = get_lang_val(0);

	buf[0] = '\0';
	for (i = 0; i < MAX_LANGUAGE; i++) {
		selptr = (r_setup.lang == i)? "selected" : "";
		sprintf(str, "<option %s value=\"%d\">%s</option>\n", selptr, i, lang_val[i]);
		strcat(buf, str);
	}
	replace_form_once(page, "##PARAM_LANGUAGE##", buf);



	{
		char pub_key[2048];
		char priv_key[2048];
		char session_key[65];
		get_enc_key(pub_key, priv_key, 0);
		create_session_key(session_key);
		replace_form_once(page, "##PARAM_RSA_PUBLIC_KEY##", pub_key);
		replace_form_once(page, "##PARAM_RSA_SESSION_KEY##", session_key);
	}


}

static void replace_resource_string(char* page, int lang)
{
	char* szRES_LANGUAGE[] = RES_ID_ARRAY(RES_LANGUAGE);
	replace_form_once(page, "##RES_LANGUAGE##", szRES_LANGUAGE[lang]);

	char* g_szResLogin_Title[] = RES_ID_ARRAY(RES_LOGIN_TITLE);
	char* g_szResLogin_Title2[] = RES_ID_ARRAY(RES_LOGIN_TITLE2);
	char* g_szResLogin_UserID[] = RES_ID_ARRAY(RES_LOGIN_USERID);
	char* g_szResLogin_Password[] = RES_ID_ARRAY(RES_LOGIN_PASSWORD);

	replace_form(page, "##RES_LOGIN_TITLE##", g_szResLogin_Title[lang]);
	replace_form_once(page, "##RES_LOGIN_TITLE2##", g_szResLogin_Title2[lang]);
	replace_form_once(page, "##RES_LOGIN_USERID##", g_szResLogin_UserID[lang]);
	replace_form_once(page, "##RES_LOGIN_PASSWORD##", g_szResLogin_Password[lang]);

	char* szRES_SETUP[] = RES_ID_ARRAY(RES_SETUP);
	replace_form_once(page, "##RES_SETUP##", szRES_SETUP[lang]);

	char info[128];
	char model[64];
	char ver[64];
#if defined(OEM_ECAM)
	sprintf(info, "%s %s", get_model(model, &r_setup), get_software_version(ver, &r_setup));
#else
#endif

}

int main(void)
{
	int lang = 0;
	int remain_flag = 0;
	//char cmd[1024];

	cgi_init();
	cgi_session_start();
	cgi_process_form();

	//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__); 
	//__system (cmd);

	if (cgi_session_var_exists("logon_user_no"))
		cgi_session_unregister_var("logon_user_no");
	if (cgi_session_var_exists("logon_user_perm"))
		cgi_session_unregister_var("logon_user_perm");

#if 0//w4000
	// ?? 로그아웃 시 login_status.txt 삭제
	remove("/mnt/ipm/login_status.txt");
printf("w4000----------- remove(/mnt/ipm/login_status.txt");
#endif
	//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__); __system (cmd);

	cgi_init_headers();
	//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__);__system (cmd);
	load_setup (&r_setup, &r_setup_ext);
		//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__); __system (cmd);
	lang = r_setup.lang;
	//sprintf (cmd, "echo \"%s : %d\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__); __system (cmd);
//retry:
	load_form(JQUERY_HTML_LOGIN, page, lang);

	int arg = 0;
	char *pdata = (char *)cgi_param("arg");
	if(pdata) {
		arg = atoi(pdata);
	}
	pdata = (char *)cgi_param("remain_flag");
	if(pdata) {
		remain_flag = atoi(pdata);
	}
	replace_resource_string(page, lang);
	replace_param(page, lang, arg, remain_flag);
	printf("%s", page);
	cgi_end();

#if 0
		// ? 로그인 상태 확인 후 파일 생성
	if (cgi_session_var_exists("logon_user_no") && cgi_session_var_exists("logon_user_perm"))
	{
		FILE *fp = fopen("/mnt/ipm/login_status.txt", "w");
		if (fp)
			fclose(fp); // 내용 없이 파일만 생성

			
			printf("w4000----------- open(/mnt/ipm/login_status.txt");
	}
#endif
	return 0;
}
