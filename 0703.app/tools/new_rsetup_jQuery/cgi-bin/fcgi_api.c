#include "fcgi_config.h"
#include "fcgi_stdio.h"

#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


#include "common2.h"
#include "jQuery_common.h"
#include "openssl/md5.h"

#include "fcgiapp.h"
#include "fcgi_common.h"



int proc(FCGX_Stream *in, FCGX_Stream *out, FCGX_ParamArray envp)
{
	// todo . 
	int ret = 501;
	return ret;
}


int main ()
{
	int i;
	int sess = 0;
	int ret = 404;
	char *uri;
    FCGX_Stream *in, *out, *err;
    FCGX_ParamArray envp;

	FCGX_Init();
	
	while (FCGX_Accept(&in, &out, &err, &envp) >= 0) {
		ret = proc(in, out, envp);
	
HTTP_STATUS:
		if(ret != 200) {
			FCGX_FPrintF(out, "Status: %d\r\n", ret);
			FCGX_FPrintF(out, "Content-type: text/html\r\n\r\n");
		}

    } /* while */

    return 0;
}
