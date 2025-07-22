#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include "fcgi_common.h"
#include "load_setup.h"
#include "aes.h"

#if 1
void AES_CBC_encrypt_bufferEx2(struct AES_ctx *ctx,uint8_t* buf, uint8_t* out, uint32_t length)
{
  uintptr_t i;
  uint8_t *Iv = ctx->Iv;
  for (i = 0; i < length; i += AES_BLOCKLEN)
  {
    XorWithIv(buf, Iv);
    CipherEx((state_t*)buf, (state_t*)out, ctx->RoundKey);
    Iv = out;
    buf += AES_BLOCKLEN;
    out += AES_BLOCKLEN;
  }
  /* store Iv in ctx for next call */
  memcpy(ctx->Iv, Iv, AES_BLOCKLEN);
}
#endif 


int parse_attr(char *attr, char key_1[], char key_2[]){

	char *p;
	int cnt = 0;

	p = attr;
	while(*p != '\0'){
		if (*p == '_')
		{
			snprintf(key_1, cnt+1, "%s", attr);
			sprintf(key_2,"%s", attr+(cnt+1));

			return 1;
		}
		p++;
		cnt++;
	}
	sprintf(key_1, "%s", attr);
	return 0;
}

int get_api_version(void)
{
	return 1;
}

static char *status_code(int code)
{
	switch(code) {
		case 200:
			return "200 OK";
		case 400:
			return "400 Bad Request";
		case 405:
			return "405 Method Not Allowed";
		case 520:
			return "520 Unknown Error";
	}
	return "404 Not Found";
}

char *get_resp_header(char *header, int ret_http, int cont_type)	
{
	char content_type[128];
	if(cont_type == 0) {
		sprintf(content_type, "application/json");
	}
	else if(cont_type == 1) {
		sprintf(content_type, "application/xml");
	}
	else {
		sprintf(content_type, "text/plain");
	}

	sprintf(header, "Status: %s\r\n"
			"Content-type: %s\r\n",
			status_code(ret_http), content_type);

	return header;
}

static int replace_char(char *str, char target, char replace)
{
	char *p;
	for(p = str; (p = strchr(p, target)); ++p) {
		*p = replace;
	}
	return 0;
}

int get_reqenv_all(char **envp, char *out)
{
	char *p = out;
	for( ; *envp != NULL; envp++) {
		sprintf(p, "%s\r\n", *envp);
		p += strlen(*envp) + 1;
	}
	return 0;
}

int get_reqenv(char **envp, char *key, char *val)
{
	for( ; *envp != NULL; envp++) {
		if(strncasecmp(*envp, key, strlen(key)) == 0) {
			char *p = *envp;
			p += strlen(key) + 1;
			sprintf(val, "%s", p);
			return 0;
		}
	}
	val = '\0';
	return -1;
}

int get_requri2api(char **envp, char *val, int *id, char *attr)
{
	int i;
	char data[128];

	if(get_reqenv(envp, "REQUEST_URI", data) != 0) {
		val[0] = '\0';
		return -1;
	}

	replace_char(data, '?', '\0');
	replace_char(data, '/', '_');
	sprintf(val, "%s", (data[0] == '_')?data + 1:data);
	if(val[strlen(val)-1] == '_') {
		val[strlen(val)-1] = '\0';
	}

	char *p = NULL;
	for(i = 0; i < strlen(val); i++) {
		if(isdigit(val[i]) && val[i-1] == '_') {
			p = val + i;
			val[i-1] = '\0';
			break;
		}
	}

	if(p == NULL) {
		*id = -1;
		attr[0] = '\0';
		return 0;
	}

	char *stop = NULL;
	*id = strtol(p, &stop, 10);

	if(stop[0] != '_') {
		attr[0] = '\0';
		return 0;
	}

	sprintf(attr, "%s", stop + 1);

	return 0;
}

const char *json_obj_str(const json_t *object, const char *field)
{
	json_t *value;

	value = json_object_get(object, field);

	if(!json_is_string(value)) {
		return NULL;
	}

	return json_string_value(value);
}

const int json_obj_int(const json_t *object, const char *field)
{
	json_t *value;

	value = json_object_get(object, field);

	if(json_is_integer(value) == 0) {
		printf("error: json parsing, %s is not a integer!\n", field);
		return -65545;
	}

	return json_integer_value(value);
}


#if 1
int json_to_string(const json_t *object, char *resp)
{
	size_t size = json_dumpb(object, NULL, 0, 0);
	if (size == 0)
		return -1;

	char *dump = json_dumps(object, JSON_ENCODE_ANY);
	sprintf(resp, "%s", dump);
	free(dump);

	return 0;
}

int json_to_wstring(const json_t *object, char *resp)
{
	struct AES_ctx ctx;
	uint8_t key[] = "aaaaaaaaaaaaaaaa";
	uint8_t iv[]  = "bbbbbbbbbbbbbbbb";
	char _enc[1025*100];
	char enc[2];
	int i;


	size_t size = json_dumpb(object, NULL, 0, 0);
	if (size == 0)
		return -1;

	char *dump = json_dumps(object, JSON_ENCODE_ANY);
	sprintf(resp, "%s", dump);
	free(dump);


	memset(_enc, 0x00, 1025*100);
	_enc[0] = '\0';

	int Extra=strlen(resp) % 16;
	char *p = resp;
	int pos = strlen(resp);


	for (i=0; i<16-Extra; i++)
	{
		resp[pos] = '\0';
		pos++;
	}



	AES_init_ctx_iv(&ctx, key, iv);
	int ret = AES_CBC_encrypt_buffer(&ctx, p, strlen(resp)+(16-Extra));
	for (i=0; i<ret; i++)
	{
		sprintf(enc, "%02x", p[i]);
		strcat(_enc, enc);
	}



	sprintf(resp, "{\"enc\":{\"data\":\"%s\"}}", _enc);

	return 0;
}
#endif

json_t *json_range_minmax(int min, int max)
{
	json_t *jrange = json_object();

	json_object_set_new(jrange, "min", json_integer(min));
	json_object_set_new(jrange, "max", json_integer(max));

	return jrange;
}

json_t *json_range_custom(const char *r1, const char *r2, int r1_val, int r2_val)
{
	json_t *jrange = json_object();

	json_object_set_new(jrange, r1, json_integer(r1_val));
	json_object_set_new(jrange, r2, json_integer(r2_val));

	return jrange;
}

#if defined(NARCH)
json_t *json_language(int lang)
{
	json_t *json = NULL;
	json_error_t error;

	switch(lang) {
		case 1:
			json = json_load_file("/home/www/lang/lankor.json", 0, &error); break;
		case 2:
			json = json_load_file("/home/www/lang/lanjpn.json", 0, &error); break;
		default:
			json = json_load_file("/home/www/lang/laneng.json", 0, &error); break;
	}
	if(json == NULL) {
		return NULL;
	}

	return json;
}

json_t *json_range_array(NSETUP_DISP_VAL *disp_val, int count, int lang)
{
	int i;
	json_t *jarray = json_array();
	json_t *jfile = json_language(lang);
	json_t *la = json_object_get(jfile, "get");
	char disp_str[DISP_VAL_MAX];
	char *str = NULL;

	for(i = 0; i < count; i++) {
		if(disp_val[i].disp == '\0') break;
		strncpy(disp_str, disp_val[i].disp, DISP_VAL_MAX);
		if(la != NULL) {
			char *s = strchr(disp_str, '|');
			if(s != NULL) {
				s = s + 1;
				str = (char *)json_obj_str(la, s);
			}
			if(str != NULL) {
				strcpy(s, str);
				json_array_append_new(jarray, json_string(disp_str));
			}
			else {
				json_array_append_new(jarray, json_string(disp_val[i].disp));
			}
		}
		else {
			json_array_append_new(jarray, json_string(disp_val[i].disp));
		}
	}

	json_decref(jfile);
	return jarray;
}
#endif	/* NARCH */

void notify_update_setup(int type)
{
#if 0
#else
	ipc_client_send("ipc_main", IPC_ID_UPDATE_SETUP, sizeof(int), (void *)&type);
#endif
	return;
}


void notify_update_isp(int type)
{
	int skfd;
	struct sockaddr_un svr;

	skfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (skfd <= 0)
	{
		fprintf(stderr, "notify_update_isp : socket open error \n");
		return;
	}
	svr.sun_family = AF_UNIX;
	strcpy(svr.sun_path, "/tmp/camera_config");
	if (sendto(skfd, &type, 1, 0, (struct sockaddr *)&svr, sizeof(struct sockaddr_un)) < 0)
		fprintf(stderr,"cannot send msg to /tmp/camera_conifg [%d] \n", type);
	close(skfd); 
	//ipc_client_send("ipc_main", IPC_ID_UPDATE_ISP, sizeof(int), (void *)&type);	
}

bool is_ip_address(const char * address)
{
	int i;
	int index = 0;
	unsigned short byte_value;

	int total_len = (int)strlen(address);
	if (total_len > 15)
	{
		return FALSE;
	}

	for (i=0; i<4; i++)
	{
		if ((address[index] < '0') || (address[index] > '9'))
		{
			return FALSE;
		}

		if (((address[index +1] < '0') || (address[index +1] > '9')) && (address[index +1] != '.'))
		{
			if ((address[index +1] == '\0') && (i == 3))
			{
				return TRUE;
			}

			return FALSE;
		}

		if (address[index +1] == '.')
		{
			index+=2;
			continue;
		}

		if (((address[index +2] < '0') || (address[index +2] > '9')) && (address[index +2] != '.'))
		{
			if ((address[index +2] == '\0') && (i == 3))
			{
				return TRUE;
			}

			return FALSE;
		}

		if (address[index +2] == '.')
		{
			index+=3;
			continue;
		}

		if (i < 3)
		{
			if (address[index +3] != '.')
			{
				return FALSE;
			}
		}

		byte_value = (address[index] - '0') *100 + (address[index +1] -'0') *10 + (address[index +2] - '0');

		if (byte_value > 255)
		{
			return FALSE;
		}

		if (i < 3)
		{
			index += 4;
		}
		else
		{
			index += 3;
		}
	}

	if (index != total_len)
	{
		return FALSE;
	}

	return TRUE;
}



// Return 6 bytes of MAC.
int __if_get_mac (const char *dev, uchar * buff)
{
	int i;
    int sockfd = -1;
    uchar *pdata = NULL;
    struct ifreq  ifr;
    int ret = -1;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		goto out;

    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    ifr.ifr_hwaddr.sa_family = AF_INET;

    if (ioctl (sockfd, SIOCGIFHWADDR, &ifr) < 0)
		goto out;

    pdata = (uchar*)ifr.ifr_hwaddr.sa_data;

    for (i = 0; i < 6; i++)
		buff[i] = *pdata++;

    ret = 0;

out:
    if (sockfd >= 0)
		close (sockfd);

	return ret;
}

