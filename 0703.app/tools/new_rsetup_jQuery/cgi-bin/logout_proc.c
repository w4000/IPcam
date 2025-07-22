#include "common2.h"
#include "jQuery_common.h"

int main () {
	cgi_init();
	cgi_session_start();
	cgi_redirect("./login.cgi");
	cgi_session_destroy();
	cgi_end();
	return 0;
}
