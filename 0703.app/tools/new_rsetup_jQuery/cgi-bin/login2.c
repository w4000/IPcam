#include "common2.h"
#include "jQuery_common.h"

static _CAMSETUP r_setup;
static _CAMSETUP_EXT r_setup_ext;


int main(void)
{
	char url[128];
	char *id, *pwd;
	char *plain_id, *plain_pwd;
	char *first_page;
	char *no_enc;
	int first_page_live = 1;
	char no[8];
	int i, perm = -1, user_no = -1;
	int plain_login = 0;

	cgi_init();
	cgi_session_start();
	cgi_process_form();



	load_setup (&r_setup, &r_setup_ext);

	plain_id = (char*)cgi_param("login_id");
	plain_pwd = (char*)cgi_param("login_pwd");
	no_enc = (char*)cgi_param("no_enc");




	id = (char*)cgi_param("user_id");
	if (id == NULL){
		if(plain_id != NULL && strlen(plain_id) > 0) {
			id = plain_id;
		}
		else {
			cgi_redirect("./login.cgi?auth=0");
			cgi_end();
			return 0;
		}
	}

	pwd = (char*)cgi_param("user_pwd");
	if (pwd == NULL){
		if(id != NULL && plain_pwd != NULL && strlen(plain_pwd) > 0) {
			pwd = plain_pwd;
			plain_login = 1;
		}
		else {
			cgi_redirect("./login.cgi?auth=0");
			cgi_end();
			return 0;
		}
	}



	first_page = (char*)cgi_param("first_page");
	if(first_page != NULL) {
		if(atoi(first_page) == 1) {
			first_page_live = 0;
		}
		else {
			first_page_live = 1;
		}
	}

	load_setup(&r_setup, &r_setup_ext);

	do
	{
		// int usr_cnt = 0;
		{
			for (i = 0; i < 10; i++) {
				
				if (strcmp(id, SETUP_USR_NAME(&r_setup, i)) == 0 &&
						strcmp(pwd, SETUP_USR_PWD(&r_setup, i)) == 0)
				{
						perm += (1 << 0);
						perm += (1 << 1);
						perm += (1 << 2);

					user_no = i+1;
					break;
				}
			}
		}


		if (perm < 0) {
			{
				sprintf(url, "./login.cgi?auth=0");
				break;
			}
		}

		if (cgi_session_var_exists("logon_user_no"))
		{
			cgi_session_unregister_var("logon_user_no");
		}


		sprintf(no, "%d", user_no);
		cgi_session_register_var("logon_user_no", no);

		sprintf(no, "%d", perm);
		cgi_session_register_var("logon_user_perm", no);
		sprintf(url, "./websetup.cgi");
		
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
	}
	while (0);

	
	cgi_redirect(url);
	cgi_end();

	return 0;
}
