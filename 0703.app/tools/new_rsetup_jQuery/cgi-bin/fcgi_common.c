#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fcgi_common.h"

#if 0
#else
formvars *process_data_mod(char *query, formvars **start, formvars **last, const char delim, const char sep)
{
	register size_t position = 0, total_len = 0, i = 0;
	char *aux;
	formvars *data;

	if (query == NULL)
		return *start;

	total_len = strlen(query);
	aux = query;
	while (*query) {
		position = 0;
				
		data = (formvars *)malloc(sizeof(formvars));
		if (!data) {
			return NULL;
		}
			
		memset(data, 0, sizeof(formvars));							
		
		// Scans the string for the next 'delim' character
		while (*aux && (*aux != delim) && (*aux != sep)) {
			position++;
			aux++;
			i++;
		}

		if (*aux && (*aux != sep)) {
			aux++;
			i++;
		}

		data->name = (char *)malloc(position+1);
		if (data->name == NULL) {
			return NULL;
		}

		strncpy(data->name, query, position);
		data->name[position] = '\0';
	
		if (*aux == sep) {
			data->value = (char *)malloc(4);
			sprintf(data->value, "NNN");
			aux++;
			i++;
		}
		else {	
			query = aux;
			position = 0;
			while (*aux && (*aux != sep)) {
				if ((*aux == '%') && (i + 2 <= total_len)) {
					if (isalnum(aux[1]) && isalnum(aux[2])) {
						aux += 2;
						i += 2;
						position++;
					}
				}
				else			
					position++;

				aux++;
				i++;
			}

			if (*aux) {
				aux++;
				i++;
			}

			data->value = (char *)malloc(position+1);
			if (data->value == NULL) {
				return NULL;
			}

			char *tmp = cgi_unescape_special_chars(query);
			strncpy(data->value, tmp, position);
			free(tmp);
			data->value[position] = '\0';
		}

	
		slist_add(data, start, last);
		
		query = aux;
	}

	return *start;
}
#endif

formvars *get_params(FCGX_Stream *in, FCGX_Stream *out, FCGX_ParamArray envp)
{
	formvars *cgi_param_start = NULL;
	formvars *cgi_param_end = NULL;

	char *method = FCGX_GetParam("REQUEST_METHOD", envp);

	if(method == NULL || !strcasecmp("GET", method)) {
		char *dados = FCGX_GetParam("QUERY_STRING", envp);

		// Sometimes, GET comes with not any data
		if (dados == NULL || strlen(dados) < 1)
			return NULL;

		return process_data_mod(dados, &cgi_param_start, &cgi_param_end, '=', '&');
	}
	else if(!strcasecmp("POST", method)) {
		char *post_data;
		int content_length;
		char *contentLength = FCGX_GetParam("CONTENT_LENGTH", envp);

		content_length = atoi(contentLength);
		post_data = (char *)malloc(content_length + 1);
		FCGX_GetStr(post_data, content_length, in);
		post_data[content_length] = '\0';

		return process_data_mod(post_data, &cgi_param_start, &cgi_param_end, '=', '&');
	}

	return NULL;
}


void http_response(FCGX_Stream *out, const char *resp, int http_status)
{
	FCGX_FPrintF(out, "Status: %d\r\n", http_status);
	FCGX_FPrintF(out, "Content-type: text/plain;charset=UTF-8\r\n");
	FCGX_FPrintF(out, "\r\n%s\r\n", resp);
}
