#ifndef __FCGI_COMMON__
#define __FCGI_COMMON__

#include "jansson.h"
#include "common2.h"

int get_api_version(void);

char *get_resp_header(char *header, int ret_http, int cont_type);	// 0:json, 1:xml, 2:plain text

int get_reqenv_all(char **envp, char *out);
int get_reqenv(char **envp, char *key, char *val);
int get_requri2api(char **envp, char *val, int *id, char *attr);

/* fcgi_api.c */
const char *json_obj_str(const json_t *object, const char *field);
const int json_obj_int(const json_t *object, const char *field);
int json_to_string(const json_t *object, char *resp);

void notify_update_setup(int type);
void notify_update_camera(int type);

bool is_ip_address(const char * address);
int parse_attr(char *attr, char key_1[], char key_2[]);

json_t *json_range_custom(const char *r1, const char *r2, int r1_val, int r2_val);
json_t *json_range_minmax(int min, int max);
int __if_get_mac (const char *dev, uchar * buff);


#if defined(NARCH)
json_t *json_range_array(NSETUP_DISP_VAL *disp_val, int count, int lang);
#endif	/* NARCH */



#endif /*__FCGI_COMMON__*/

