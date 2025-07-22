
#ifndef AJAX_COMMON_H_
#define AJAX_COMMON_H_

#include "cgi.h"
#include "setup/setup.h"

#include "jQuery_language.h"


extern void ajax_conv(char* out_buf, char* page, int lang);
extern int ajax_check_output(char* page, int lang);
extern void ajax_printout(char* page, int lang);
extern char* ajax_conv_utf8(char* page, int lang);

#endif /* AJAX_COMMON_H_ */
