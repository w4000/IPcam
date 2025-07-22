#ifndef __FCGI_SESSION_H__
#define __FCGI_SESSION_H__

#ifdef __cplusplus
"C" {
#endif

#include "fcgiapp.h"
#include "cgi.h"
// general purpose linked list. Actualy isn't very portable
// because uses only 'name' and 'value' variables to store data.
// Problably, in a future release, this will be replaced by 
// another type of struct
#if 0
typedef struct formvarsA {
        char *name;
        char *value;
        struct formvarsA *next;
} formvars;
#endif
extern formvars *cookies_start;
extern formvars *cookies_last;

// Cookie functions
int fcgi_add_cookie(const char *name, const char *value, const char *max_age, const char *path, const char *domain, const int secure, FCGX_Stream *out);
formvars *fcgi_get_cookies(FCGX_ParamArray envp);
char *fcgi_cookie_value(const char *cookie_name);

// General purpose string functions
int strnpos(char *s, char *ch, unsigned int count);
int strpos(char *s, char *ch);
char *strdel(char *s, int start, int count);
char **explode(char *src, const char *token, int *total);
char *substr(char *src, const int start, const int count);
char *stripnslashes(char *s, int n);
char *addnslashes(char *s, int n);
char *stripnslashes(char *s, int n);
char *str_nreplace(char *str, const char *delim, const char *with, int n);
char *str_replace(char *str, const char *delim, const char *with);
char *addslashes(char *str);
char *stripslashes(char *str);
char *str_base64_encode(char *str);
char *str_base64_decode(char *str);
char *recvline(FILE *fp);
char *md5(const char *str);

//void slist_add(formvars *item, formvars **start, formvars **last);
//int slist_delete(char *name, formvars **start, formvars **last);
//char *slist_item(const char *name, formvars *start);

//void slist_free(formvars **start);
 
// Session stuff
#ifndef SESSION_FILE_PREFIX
#define SESSION_FILE_PREFIX "cgisess_"
#endif

//const char *session_error_message[];
extern int session_lasterror;

extern formvars *sess_list_start;

extern char SESSION_SAVE_PATH[255];
extern char SESSION_COOKIE_NAME[50];
 
void fcgi_session_set_max_idle_time(unsigned long seconds);
int fcgi_session_destroy(FCGX_Stream *out);
int fcgi_session_register_var(const char *name, const char *value);
int fcgi_session_alter_var(const char *name, const char *new_value);
int fcgi_session_var_exists(const char *name);
int fcgi_session_unregister_var(char *name);
int fcgi_session_start(FCGX_ParamArray envp, FCGX_Stream *out);
int fcgi_session_end();
void fcgi_session_cookie_name(const char *cookie_name);
char *fcgi_session_var(const char *name);
void fcgi_session_save_path(const char *path);
 
#ifdef __cplusplus
}
#endif

#endif // __FCGI_SESSION_H__
