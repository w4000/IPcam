

#include "sys_inc.h"
#include "http.h"
#include "http_cln.h"


BOOL http_cln_tx(HTTPREQ * p_req, const char * p_data, int len)
{
    int slen;

	if (p_req->cfd <= 0)
	{
		return FALSE;
    }

	slen = send(p_req->cfd, p_data, len, 0);
	if (slen != len)
	{
		return FALSE;
    }

	return TRUE;
}

BOOL http_onvif_req(HTTPREQ * p_req, const char * p_xml, int len)
{
    int offset = 0;
	char bufs[1024 * 4];

	if (len > 3072)
	{
		return FALSE;
	}

	offset += sprintf(bufs+offset, "POST %s HTTP/1.1\r\n", p_req->url);
	offset += sprintf(bufs+offset, "Host: %s:%d\r\n", p_req->host, p_req->port);
	offset += sprintf(bufs+offset, "User-Agent: ecam/1.0\r\n");
	offset += sprintf(bufs+offset, "Content-Type: application/soap+xml; charset=utf-8; action=\"%s\"\r\n", p_req->action);
	offset += sprintf(bufs+offset, "Content-Length: %d\r\n", len);
	offset += sprintf(bufs+offset, "Connection: close\r\n\r\n");

	//printf("w4000 %x, %s \n",p_req->need_auth,p_req->user);//is_auth_w4000(void)
	//printf("------------------------------------\n");//is_auth_w4000(void)
	if (p_xml && len > 0)
	{
		memcpy(bufs+offset, p_xml, len);
		offset += len;
	}

	bufs[offset] = '\0';


	return http_cln_tx(p_req, bufs, offset);
}


BOOL http_onvif_trans(HTTPREQ * p_req, int timeout, const char * bufs, int len)
{
	BOOL ret = FALSE;

	if (p_req->https)
	{
		return FALSE;
	}

	p_req->cfd = tcp_connect_timeout(inet_addr(p_req->host), p_req->port, timeout);
	if (p_req->cfd <= 0)
	{
		goto FAILED;
    }


	ret = http_onvif_req(p_req, bufs, len);

FAILED:

	if (p_req->cfd > 0)
	{
		closesocket(p_req->cfd);
		p_req->cfd = 0;
	}


	return ret;
}



