#include "fcgi_config.h"
#include "fcgi_stdio.h"

#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

extern char **environ;

#include "common2.h"
#include "openssl/md5.h"

#include "fcgiapp.h"
#include "fcgi_api_map.h"



int res_fcgapi_name(char *api_name)
{
	char *fcgiai;
	int ret = 0;
	fcgiai = strstr(api_name, "_fcgiapi");
	if (fcgiai)
	{
		fcgiai +=  strlen("_fcgiapi");
		sprintf(api_name, "api%s", fcgiai);

		ret = 1;
	}

	return ret;
}


int main ()
{
    FCGX_Stream *in, *out, *err;
    FCGX_ParamArray envp;


    int i;
	char resp[1*1024*1024];


    while(FCGX_Accept(&in, &out, &err, &envp) >= 0)
	{
		char api_name[128];
		int id = -1;
		char attr[64];
		char method[16];
		char *data = NULL;
        char *contentLength = FCGX_GetParam("CONTENT_LENGTH", envp);
		int len = 0;
		int ret_http = 404;
		
		

		get_requri2api(envp, api_name, &id, attr);
		get_reqenv(envp, "REQUEST_METHOD", method);
		if (!res_fcgapi_name(api_name))
		{
		 	sprintf(api_name, "api_name_null");
		}

		if(contentLength != NULL) {
			len = strtol(contentLength, NULL, 10);
		}

		if(len > 0) {
			data = (char *)malloc(len+1);
			FCGX_GetStr(data, len, in);
			data[len] = '\0';
		}



		resp[0] = '\0';
		for (i = 0; i < (sizeof(api_map) / sizeof(api_map[0])); i++) {

			// if (!strcmp(api_map[i].name, api_name) && api_map[i].func) {
			if (!strncmp(api_map[i].name, api_name, strlen(api_name)) && api_map[i].func) {

				
				//char cmd[128];
				//sprintf (cmd, "echo \"%s : %d > api_name? %s\" | cat >> /mnt/ipm/log.txt", __func__, __LINE__, api_name); 
				//__system (cmd);

				ret_http = api_map[i].func(method, data, id, attr, resp);
				break;
			}
		}


		{
			int content_type = 0;
			char header[128];
			FCGX_FPrintF(out, "%s\r\n%s", get_resp_header(header, ret_http, content_type), resp);
		}

		if(data != NULL) {
			free(data);
		}
    } /* while */


    return 0;
}
