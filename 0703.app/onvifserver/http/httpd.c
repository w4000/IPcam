

#include "sys_inc.h"
#include "http.h"
#include "base64.h"
#include "http_parse.h"
#include "httpd.h"
#include "onvif.h"
#include "onvif_cm.h"
#include "http_auth.h"

#ifdef HTTPD

/***************************************************************************************/

#define SERVER_NAME "ipcam"
#define SERVER_URL  "http://www.ipcam.com"
#define PROTOCOL 	"HTTP/1.1"
#define RFC1123FMT 	"%a, %d %b %Y %H:%M:%S GMT"

#ifdef ANDROID
#define WEBSPATH	"/mnt/html/"
#else
#define WEBSPATH	"./html/"
#endif

#define DEFFILE		"main.html"
#define MAX_OUTPUT  (1024*1024)


/***************************************************************************************/
HD_AUTH_INFO        g_http_auth;

extern ONVIF_CFG    g_onvif_cfg;

/***************************************************************************************/

int http_do_file(const char * filename, char * buff, int buflen)
{
    int len;
    int rlen;
    FILE * fp;

    fp = fopen(filename, "rb");
    if (!fp)
    {
        return 0;
    }

    fseek(fp, 0, SEEK_END);

	len = ftell(fp);
	if (len <= 0)
	{
		fclose(fp);
		return 0;
	}
	fseek(fp, 0, SEEK_SET);

    if (len >= buflen)
    {

        len = buflen - 1;
    }

	rlen = (int)fread(buff, 1, len, fp);

    fclose(fp);

    return rlen;
}

struct mime_handler mime_handlers[] = {
	{ "**.htm", "text/html", 1, http_do_file },
	{ "**.html", "text/html", 1, http_do_file },
	{ "**.js", "text/js", 0, http_do_file },
	{ "**.gif", "image/gif", 1, http_do_file },
	{ "**.jpg", "image/jpeg", 1, http_do_file },
	{ "**.jpeg", "image/gif", 1, http_do_file },
	{ "**.png", "image/png", 1, http_do_file },
	{ "**.css", "text/css", 0, http_do_file },
	{ "**.au", "audio/basic", 1, http_do_file },
	{ "**.wav", "audio/wav", 1, http_do_file },
	{ "**.avi", "video/x-msvideo", 1, http_do_file },
	{ "**.mov", "video/quicktime", 1, http_do_file },
	{ "**.mpeg", "video/mpeg", 1, http_do_file },
	{ "**.vrml", "model/vrml", 1, http_do_file },
	{ "**.midi", "audio/midi", 1, http_do_file },
	{ "**.mp3", "audio/mpeg", 1, http_do_file },
	{ "**.pac", "application/x-ns-proxy-autoconfig", 1, http_do_file },
	{ NULL, NULL, 0, NULL }
};

void http_send_msg(HTTPCLN * p_user, char * p_bufs, int tlen)
{
    send(p_user->cfd, p_bufs, tlen, 0);
}

void http_send_error(HTTPCLN * p_user, int status, const char* title, const char * exhdr, const char* text)
{
    char body[8*1024];
    char buff[10*1024];
    int offset = 0;
    int bodyoff = 0;
    int buflen = sizeof(buff);
    int bodylen = sizeof(body);
    time_t now;
    char timebuf[100];

    bodyoff += snprintf(body+bodyoff, bodylen-bodyoff, "<HTML>"
    				   "<HEAD>"
    				   "<TITLE>%d %s</TITLE>"
    				   "</HEAD>\n"
    				   "<BODY BGCOLOR=\"#cc9999\">"
    				   "<H4>%d %s</H4>\n", status, title, status, title);
    bodyoff += snprintf(body+bodyoff, bodylen-bodyoff, "%s\n", text);
    bodyoff += snprintf(body+bodyoff, bodylen-bodyoff, "<HR>\n"
    				   "<ADDRESS><A HREF=\"%s\">%s</A></ADDRESS>\n"
    				   "</BODY>"
    				   "</HTML>\n", SERVER_URL, SERVER_NAME);

    now = time(NULL);
    strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));

    offset += snprintf(buff+offset, buflen-offset, "%s %d %s\r\n", PROTOCOL, status, title);
    offset += snprintf(buff+offset, buflen-offset, "Server: %s\r\n", SERVER_NAME);
    offset += snprintf(buff+offset, buflen-offset, "Date: %s\r\n", timebuf);
    offset += snprintf(buff+offset, buflen-offset, "Content-Length: %d\r\n", bodyoff);
    offset += snprintf(buff+offset, buflen-offset, "Content-Type: %s\r\n", "text/html");
    offset += snprintf(buff+offset, buflen-offset, "Connection: close\r\n");

    if (exhdr)
    {
        offset += snprintf(buff+offset, buflen-offset, "%s", exhdr);
    }

    offset += snprintf(buff+offset, buflen-offset, "\r\n");

    strncpy(buff+offset, body, bodyoff);
    offset += bodyoff;
    buff[offset] = '\0';


    http_send_msg(p_user, buff, offset);
}


void http_do_output(HTTPCLN * p_user, const char * filename, struct mime_handler * p_handler)
{
    char * buff = (char *)malloc(MAX_OUTPUT);
    int offset = 0;
    int buflen = MAX_OUTPUT;
    time_t now;
    char timebuf[100];


    now = time(NULL);
    strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));

    offset += snprintf(buff+offset, buflen-offset, "%s %d %s\r\n", PROTOCOL, 200, "OK");
    offset += snprintf(buff+offset, buflen-offset, "Server: %s\r\n", SERVER_NAME);
    offset += snprintf(buff+offset, buflen-offset, "Date: %s\r\n", timebuf);
    offset += snprintf(buff+offset, buflen-offset, "Content-Type: %s\r\n", p_handler->mime_type);
    offset += snprintf(buff+offset, buflen-offset, "Connection: close\r\n");
    offset += snprintf(buff+offset, buflen-offset, "\r\n");

    if (p_handler->output)
    {
        offset += p_handler->output(filename, buff+offset, buflen-offset);
    }

    http_send_msg(p_user, buff, offset);

    free(buff);
}

int http_match_one(const char* pattern, int patternlen, const char* string)
{
    const char* p;

    for (p = pattern; p - pattern < patternlen; ++p, ++string)
    {
        if (*p == '?' && *string != '\0')
        {
            continue;
        }

        if (*p == '*')
        {
            int i, pl;
            ++p;

            if (*p == '*')
            {
                /* Double-wildcard matches anything. */
                ++p;
                i = (int)strlen(string);
            }
            else
            {
                /* Single-wildcard matches anything but slash. */
                i = (int)strcspn(string, "/");
            }

            pl = patternlen - (int)(p - pattern);

            for (; i >= 0; --i)
            {
                if (http_match_one(p, pl, &(string[i])))
                {
                    return 1;
                }
            }

            return 0;
        }

        if (*p != *string)
        {
            return 0;
        }
    }

    if (*string == '\0')
    {
        return 1;
    }

    return 0;
}


int http_match(const char* pattern, const char* string)
{
	const char * por;

    for (;;)
    {
        por = strchr(pattern, '|');

        if (por == NULL)
        {
            return http_match_one(pattern, (int)strlen(pattern), string);
        }

        if (http_match_one( pattern, (int)(por - pattern), string))
        {
            return 1;
        }

        pattern = por + 1;
    }
}

void http_process_request(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	int  len;
	char method[256] = {'\0'}, path[1000] = {'\0'}, protocol[100] = {'\0'};
	char filename[1000] = {'\0'}, * query, * file;

	struct mime_handler *handler;

	if (sscanf(rx_msg->msg_buf, "%[^ ] %[^ ] %[^ ]", method, path, protocol) != 3)
	{
        
        // printf("!%s:%d > Bad Request \n", __func__, __LINE__);
        http_send_error(p_user, 400, "Bad Request", NULL, "Can't parse request.");
        return;
    }

    if (strcasecmp(method, "get") != 0 && strcasecmp(method, "post") != 0)
    {
    	http_send_error(p_user, 501, "Not Implemented", NULL, "That method is not implemented.");
    	return;
    }

    if (path[0] != '/')
    {
        // printf("!%s:%d > Bad Request \n", __func__, __LINE__);
        http_send_error(p_user, 400, "Bad Request", NULL, "Bad filename.");
        return;
    }

	file = &(path[1]);
    len = (int)strlen(file);

    if (file[0] == '/' ||
    	strcmp(file, "..") == 0 ||
    	strncmp(file, "../", 3) == 0 ||
    	strstr(file, "/../") != NULL ||
    	strcmp( &(file[len-3]), "/.." ) == 0)
	{
        
        // printf("!%s:%d > Bad Request \n", __func__, __LINE__);
        http_send_error(p_user, 400, "Bad Request", NULL, "Illegal filename.");
        return;
    }

	if (file[0] == '\0' || file[len-1] == '/')
    {
        snprintf(&file[len], sizeof(path) - len - 1, DEFFILE);
	}

    path[sizeof(path) - 1] = '\0';

	if ((query = strstr(file, "?")) != NULL)
	{
		int len = (int)(query - file);

		if (len >= sizeof(filename))
		{
		    len = sizeof(filename) - 1;
		}

    	strncpy(filename, file, len);
    }
    else
    {
        strncpy(filename, file, sizeof(filename)-1);
    }


	for (handler = &mime_handlers[0]; handler->pattern; handler++)
	{
        if (http_match(handler->pattern, filename))
        {
        	if (handler->auth)
        	{

            }

            snprintf(filename, sizeof(filename)-1, "%s%s", WEBSPATH, file);
            http_do_output(p_user, filename, handler);

            break;
        }
    }

	if (NULL == handler->pattern)
	{
        http_send_error(p_user, 404, "Not Found", NULL, "File not found.");
    }

}


#endif




