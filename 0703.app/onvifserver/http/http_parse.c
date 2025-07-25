

#include "sys_inc.h"
#include "http.h"
#include "http_parse.h"
#include "sys_buf.h"


typedef struct request_message_type_value
{
	HTTP_MT	msg_type;
	char	msg_str[32];
	int		msg_len;
} REQMTV;

static const REQMTV req_mtvs[] =
{
	{HTTP_MT_GET,			"GET",			3},
	{HTTP_MT_HEAD,			"HEAD",			4},
	{HTTP_MT_MPOST,			"M-POST",		6},
	{HTTP_MT_MSEARCH,		"M-SEARCH",		8},
	{HTTP_MT_NOTIFY,		"NOTIFY",		6},
	{HTTP_MT_POST,			"POST",			4},
	{HTTP_MT_SUBSCRIBE,		"SUBSCRIBE",	9},
	{HTTP_MT_UNSUBSCRIBE,	"UNSUBSCRIBE",	11}
};

HT_API BOOL http_is_http_msg(char * msg_buf)
{
	uint32 i;

	for (i=0; i<sizeof(req_mtvs)/sizeof(REQMTV); i++)
	{
		if (memcmp(msg_buf, req_mtvs[i].msg_str, req_mtvs[i].msg_len) == 0)
		{
			return TRUE;
		}
	}

	if (memcmp(msg_buf, "HTTP/1.1", strlen("HTTP/1.1")) == 0 || memcmp(msg_buf, "HTTP/1.0", strlen("HTTP/1.0")) == 0)
	{
		return TRUE;
	}

	return FALSE;
}

HT_API int http_pkt_find_end(char * p_buf)
{
	int end_off = 0;
	int end_len = 4;
	int http_pkt_finish = 0;

	while (p_buf[end_off] != '\0')
	{
		if ((p_buf[end_off] == '\r' && p_buf[end_off+1] == '\n') &&
			(p_buf[end_off+2] == '\r' && p_buf[end_off+3] == '\n'))
		{
		    end_len = 4;
			http_pkt_finish = 1;
			break;
		}
		else if (p_buf[end_off] == '\n' && p_buf[end_off+1] == '\n')
		{
		    end_len = 2;
			http_pkt_finish = 1;
			break;
		}

		end_off++;
	}

	if (http_pkt_finish)
	{
		return (end_off + end_len);
	}

	return 0;
}

HT_API void http_headl_parse(char * pline, int llen, HTTPMSG * p_msg)
{
	char	word_buf[256];
	int		word_len;
	int		next_word_offset;
	BOOL	bHaveNextWord;

	bHaveNextWord = GetLineWord(pline, 0, llen, word_buf, sizeof(word_buf), &next_word_offset, WORD_TYPE_STRING);
	word_len = (int)strlen(word_buf);

	if (word_len > 0 && word_len < 31)
	{
		memcpy(p_msg->first_line.header, pline, word_len);
		p_msg->first_line.header[word_len] = '\0';

		while (pline[next_word_offset] == ' ')
		{
			next_word_offset++;
		}

		p_msg->first_line.value_string = pline+next_word_offset;

		if (strcasecmp(word_buf,"HTTP/1.1") == 0 || strcasecmp(word_buf,"HTTP/1.0") == 0)
		{
			if (bHaveNextWord)
			{
				word_len = sizeof(word_buf);
				bHaveNextWord = GetLineWord(pline, next_word_offset, llen, word_buf, sizeof(word_buf), &next_word_offset, WORD_TYPE_NUM);
				word_len = (int)strlen(word_buf);
				if (word_len > 0)
				{
					p_msg->msg_type = 1;
					p_msg->msg_sub_type = atoi(word_buf);
				}
			}
		}
		else
		{
			uint32 i;

			p_msg->msg_type = 0;

			for (i=0; i<sizeof(req_mtvs)/sizeof(REQMTV); i++)
			{
				if (strcasecmp(word_buf, (char *)(req_mtvs[i].msg_str)) == 0)
				{
					p_msg->msg_sub_type = req_mtvs[i].msg_type;
					break;
				}
			}
		}
	}
}

HT_API int http_line_parse(char * p_buf, int max_len, char sep_char, PPSN_CTX * p_ctx)
{
	char word_buf[256];
	BOOL bHaveNextLine = TRUE;
	int  line_len = 0;
	int  parse_len = 0;

	char * ptr = p_buf;

	do {
	    int	next_word_offset = 0;
	    char nchar;
	    HDRV * pHdrV;

		if (GetSipLine(ptr, max_len, &line_len, &bHaveNextLine) == FALSE)
		{
			return -1;
		}

		if (line_len == 2)
		{
			return (parse_len + 2);
		}


		GetLineWord(ptr, 0, line_len-2, word_buf, sizeof(word_buf), &next_word_offset, WORD_TYPE_STRING);

		while (ptr[next_word_offset] == ' ')
		{
			next_word_offset++;
		}

		nchar = *(ptr + next_word_offset);

		if (nchar != sep_char) 
		{
			return -1;
		}

		next_word_offset++;

		while (ptr[next_word_offset] == ' ')
		{
			next_word_offset++;
		}

		pHdrV = hdrv_buf_get_idle();
		if (pHdrV == NULL)
		{
			return -1;
		}

		strncpy(pHdrV->header, word_buf, 32);
		pHdrV->value_string = ptr+next_word_offset;
		pps_ctx_ul_add(p_ctx, pHdrV);

		ptr += line_len;
		max_len -= line_len;
		parse_len += line_len;

	} while (bHaveNextLine);

	return parse_len;
}

HT_API BOOL http_get_headline_uri(HTTPMSG * rx_msg, char * p_uri, int size)
{
	int len;
	char * p_end;
	char * p_ptr = rx_msg->first_line.value_string;
	if (p_ptr == NULL)
	{
		return FALSE;
	}

	p_end = p_ptr;
	while (*p_end != ' ') p_end++;

	len = (int) (p_end - p_ptr);
	if (len >= size)
	{
		return FALSE;
	}

	memcpy(p_uri, p_ptr, len);
	p_uri[len] = '\0';

	return TRUE;
}

HT_API int http_ctt_parse(HTTPMSG * p_msg)
{
	int flag = 0;
	HTTPCTT w_ctx_type;

	HDRV * pHdrV = (HDRV *)pps_lookup_start(&(p_msg->hdr_ctx));
	while (pHdrV != NULL)
	{
		if (strcasecmp(pHdrV->header, "Content-Length") == 0)
		{
			p_msg->ctt_len = atol(pHdrV->value_string);
			flag++;
		}
		else if (strcasecmp(pHdrV->header, "Content-Type") == 0)
		{
			char type_word[64];
			int  next_tmp;

			BOOL nextword = GetLineWord(pHdrV->value_string, 0, (int)strlen(pHdrV->value_string), type_word, sizeof(type_word), &next_tmp, WORD_TYPE_STRING);

			if (strcasecmp(type_word, "application/sdp") == 0)
			{
				w_ctx_type = CTT_SDP;
			}
			else if (strcasecmp(type_word, "application/soap+xml") == 0)
			{
				w_ctx_type = CTT_XML;
			}
			else if (strcasecmp(type_word, "text/plain") == 0)
			{
				w_ctx_type = CTT_TXT;
			}
			else if (strcasecmp(type_word, "text/html") == 0)
			{
				w_ctx_type = CTT_HTM;
			}
			else if (strcasecmp(type_word, "application/octet-stream") == 0)
			{
				w_ctx_type = CTT_BIN;
			}
			else if (strcasecmp(type_word, "image/jpeg") == 0)
			{
				w_ctx_type = CTT_JPG;
			}
			else if (strcasecmp(type_word, "application/x-rtsp-tunnelled") == 0)
			{
				w_ctx_type = CTT_RTSP_TUNNELLED;
			}
			else if (strcasecmp(type_word, "multipart/x-mixed-replace") == 0)
			{
			    char * boundary = NULL;

				w_ctx_type = CTT_MULTIPART;

			    boundary = strstr(pHdrV->value_string, "boundary=");
                if (boundary)
                {
                    int offset = 0;
                    while (*(boundary+9+offset) == '-')
                    {
                        offset++;
                    }

                    strcpy(p_msg->boundary, boundary+9+offset);
                }
			}
			else
			{
				w_ctx_type = CTT_NULL;
			}

			p_msg->ctt_type = w_ctx_type;
			flag++;
		}

		pHdrV = (HDRV *)pps_lookup_next(&(p_msg->hdr_ctx), pHdrV);
	}
	pps_lookup_end(&(p_msg->hdr_ctx));

	if (p_msg->ctt_type && p_msg->ctt_len)
	{
		return 1;
	}

	return 0;
}

HT_API int http_msg_parse(char * msg_buf, int msg_buf_len, HTTPMSG * msg)
{
	BOOL bHaveNextLine;
	int  line_len = 0;
	char * p_buf = msg_buf;

	msg->msg_type = -1;

	if (GetSipLine(p_buf, msg_buf_len, &line_len, &bHaveNextLine) == FALSE)
	{
		return -1;
	}

	if (line_len > 0)
	{
		http_headl_parse(p_buf, line_len-2, msg);
	}

	if (msg->msg_type == (uint32)-1)
	{
		return -1;
	}

	p_buf += line_len;
	msg->hdr_len = http_line_parse(p_buf, msg_buf_len-line_len, ':', &(msg->hdr_ctx));
	if (msg->hdr_len <= 0)
	{
		return -1;
	}

	p_buf += msg->hdr_len;

	if (http_ctt_parse(msg) == 1 && msg->ctt_len > 0)
	{
	    int slen;
		HDRV * pHdrV;

		pHdrV = hdrv_buf_get_idle();
		if (pHdrV == NULL)
		{
			return -1;
		}

		strcpy(pHdrV->header, "");
		pHdrV->value_string = p_buf;

		pps_ctx_ul_add(&(msg->ctt_ctx), pHdrV);

		slen = (int)strlen(p_buf);
		if (slen != msg->ctt_len)
		{
		}
	}

	return (line_len + msg->hdr_len + msg->ctt_len);
}

HT_API int http_msg_parse_part1(char * p_buf, int buf_len, HTTPMSG * msg)
{
	BOOL bHaveNextLine;
	int line_len = 0;

	msg->msg_type = -1;

	if (GetSipLine(p_buf, buf_len, &line_len, &bHaveNextLine) == FALSE)
	{
		return -1;
	}

	if (line_len > 0)
	{
		http_headl_parse(p_buf, line_len-2, msg);
	}

	if (msg->msg_type == (uint32)-1)
	{
		return -1;
	}

	p_buf += line_len;
	msg->hdr_len = http_line_parse(p_buf, buf_len-line_len, ':', &(msg->hdr_ctx));
	if (msg->hdr_len <= 0)
	{
		return -1;
	}

	http_ctt_parse(msg);

	return (line_len + msg->hdr_len);
}

HT_API int http_msg_parse_part2(char * p_buf, int buf_len, HTTPMSG * msg)
{
    int slen;
	HDRV * pHdrV;

	pHdrV = hdrv_buf_get_idle();
	if (pHdrV == NULL)
	{
		return -1;
	}

	strcpy(pHdrV->header, "");
	pHdrV->value_string = p_buf;

	pps_ctx_ul_add(&(msg->ctt_ctx), pHdrV);

	slen = buf_len;

	if (ctt_is_string(msg->ctt_type))
	{
		slen = (int)strlen(p_buf);

		if (slen != msg->ctt_len)
		{
		}
	}

	return slen;
}

HT_API HDRV * http_find_headline(HTTPMSG * msg, const char * head)
{
    HDRV * line;

	if (msg == NULL || head == NULL)
	{
		return NULL;
	}

	line = (HDRV *)pps_lookup_start(&(msg->hdr_ctx));
	while (line != NULL)
	{
		if (strcasecmp(line->header, head) == 0)
		{
			pps_lookup_end(&(msg->hdr_ctx));
			return line;
		}

		line = (HDRV *)pps_lookup_next(&(msg->hdr_ctx), line);
	}
	pps_lookup_end(&(msg->hdr_ctx));

	return NULL;
}

HT_API HDRV * http_find_headline_next(HTTPMSG * msg, const char * head, HDRV * hrv)
{
    HDRV * line;

	if (msg == NULL || head == NULL)
	{
		return NULL;
	}

	line = (HDRV *)pps_lookup_start(&(msg->hdr_ctx));
	while (line != NULL)
	{
		if (line == hrv)
		{
			line = (HDRV *)pps_lookup_next(&(msg->hdr_ctx), line);
			break;
		}

		line = (HDRV *)pps_lookup_next(&(msg->hdr_ctx), line);
	}

	while (line != NULL)
	{
		if (strcasecmp(line->header, head) == 0)
		{
			pps_lookup_end(&(msg->hdr_ctx));
			return line;
		}

		line = (HDRV *)pps_lookup_next(&(msg->hdr_ctx), line);
	}
	pps_lookup_end(&(msg->hdr_ctx));

	return NULL;
}

HT_API char * http_get_headline(HTTPMSG * msg, const char * head)
{
	HDRV * p_hdrv = http_find_headline(msg, head);
	if (p_hdrv == NULL)
	{
		return NULL;
	}

	return p_hdrv->value_string;
}

void http_add_tx_line(HTTPMSG * tx_msg, const char * msg_hdr, const char * msg_fmt,...)
{
	va_list argptr;
	int slen;
	HDRV *pHdrV;

	if (tx_msg == NULL || tx_msg->msg_buf == NULL)
	{
		return;
	}

	pHdrV = hdrv_buf_get_idle();
	if (pHdrV == NULL)
	{
		return;
	}

	pHdrV->value_string = tx_msg->msg_buf + tx_msg->buf_offset;

	strncpy(pHdrV->header, msg_hdr, 31);

	va_start(argptr, msg_fmt);
#if	__LINUX_OS__
	slen = vsnprintf(pHdrV->value_string, 1600-tx_msg->buf_offset, msg_fmt, argptr);
#else
	slen = vsprintf(pHdrV->value_string, msg_fmt, argptr);
#endif
	va_end(argptr);

	if (slen < 0)
	{
		hdrv_buf_free(pHdrV);
		return;
	}

	pHdrV->value_string[slen] = '\0';
	tx_msg->buf_offset += slen + 1;

	pps_ctx_ul_add(&(tx_msg->hdr_ctx), pHdrV);
}

HT_API HDRV * http_find_ctt_headline(HTTPMSG * msg, const char * head)
{
    HDRV * line;

	if (msg == NULL || head == NULL)
	{
		return NULL;
	}

	line = (HDRV *)pps_lookup_start(&(msg->ctt_ctx));
	while (line != NULL)
	{
		if (strcasecmp(line->header, head) == 0)
		{
			pps_lookup_end(&(msg->ctt_ctx));
			return line;
		}

		line = (HDRV *)pps_lookup_next(&(msg->ctt_ctx), line);
	}
	pps_lookup_end(&(msg->ctt_ctx));

	return NULL;
}

HT_API char * http_get_ctt(HTTPMSG * msg)
{
    HDRV * line;

	if (msg == NULL)
	{
		return NULL;
	}

	line = (HDRV *)pps_lookup_start(&(msg->ctt_ctx));
	pps_lookup_end(&(msg->ctt_ctx));

	if (line)
	{
		return line->value_string;
	}

	return NULL;
}

HT_API BOOL http_get_auth_digest_info(HTTPMSG * rx_msg, HD_AUTH_INFO * p_auth)
{
	return FALSE;
}

/***********************************************************************/
static PPSN_CTX * msg_buf_fl = NULL;

HT_API BOOL http_msg_buf_init(int num)
{
	msg_buf_fl = pps_ctx_fl_init(num, sizeof(HTTPMSG), TRUE);
	if (msg_buf_fl == NULL)
	{
		return FALSE;
	}

	return TRUE;
}

HT_API void http_msg_buf_deinit()
{
	if (msg_buf_fl)
	{
		pps_fl_free(msg_buf_fl);
		msg_buf_fl = NULL;
	}
}

HT_API HTTPMSG * http_get_msg_buf()
{
	HTTPMSG * tx_msg = (HTTPMSG *)pps_fl_pop(msg_buf_fl);
	if (tx_msg == NULL)
	{
		return NULL;
	}

	memset(tx_msg, 0, sizeof(HTTPMSG));

	tx_msg->msg_buf = net_buf_get_idle();
	if (tx_msg->msg_buf == NULL)
	{

		http_free_msg_buf(tx_msg);
		return NULL;
	}

	http_msg_ctx_init(tx_msg);

	return tx_msg;
}

HT_API void http_msg_ctx_init(HTTPMSG * msg)
{
	pps_ctx_ul_init_nm(hdrv_buf_fl, &(msg->hdr_ctx));
	pps_ctx_ul_init_nm(hdrv_buf_fl, &(msg->ctt_ctx));
}

HT_API void http_free_msg_buf(HTTPMSG * msg)
{
	pps_fl_push(msg_buf_fl, msg);
}

HT_API uint32 http_idle_msg_buf_num()
{
	return msg_buf_fl->node_num;
}

HT_API HTTPMSG * http_get_msg_large_buf(int size)
{
	HTTPMSG * tx_msg = (HTTPMSG *)pps_fl_pop(msg_buf_fl);
	if (tx_msg == NULL)
	{
		return NULL;
	}

	memset(tx_msg, 0, sizeof(HTTPMSG));

	tx_msg->msg_buf = (char *)malloc(size);
	if (tx_msg->msg_buf == NULL)
	{

		http_free_msg_buf(tx_msg);
		return NULL;
	}

	http_msg_ctx_init(tx_msg);

	return tx_msg;
}

HT_API void http_free_msg(HTTPMSG * msg)
{
	if (msg == NULL)
	{
		return;
	}

	http_free_msg_content(msg);
	http_free_msg_buf(msg);
}

HT_API void http_free_msg_content(HTTPMSG * msg)
{
	if (msg == NULL)
	{
		return;
	}

	http_free_msg_ctx(msg, 0);
	http_free_msg_ctx(msg, 1);

	net_buf_free(msg->msg_buf);
}

HT_API void http_free_msg_ctx(HTTPMSG * msg, int type)
{
	PPSN_CTX * p_free_ctx = NULL;

	switch (type)
	{
	case 0:
		p_free_ctx = &(msg->hdr_ctx);
		break;

	case 1:
		p_free_ctx = &(msg->ctt_ctx);
		break;
	}

	if (p_free_ctx == NULL)
	{
		return;
	}

	hdrv_ctx_free(p_free_ctx);
}



