

#ifndef	__H_HTTP_H__
#define	__H_HTTP_H__

#include "sys_buf.h"
#include "ppstack.h"

#ifdef HTTPS
#include "openssl/ssl.h"
#endif

/***************************************************************************************/
typedef enum http_request_msg_type
{
	HTTP_MT_NULL = 0,
	HTTP_MT_GET,
	HTTP_MT_HEAD,
	HTTP_MT_MPOST,
	HTTP_MT_MSEARCH,
	HTTP_MT_NOTIFY,
	HTTP_MT_POST,
	HTTP_MT_SUBSCRIBE,
	HTTP_MT_UNSUBSCRIBE,
} HTTP_MT;

/***************************************************************************************/
typedef enum http_content_type
{
	CTT_NULL = 0,
	CTT_SDP,
	CTT_TXT,
	CTT_HTM,
	CTT_XML,
	CTT_BIN,
	CTT_JPG,
	CTT_RTSP_TUNNELLED,
	CTT_MULTIPART
} HTTPCTT;

#define ctt_is_string(type)	(type == CTT_XML || type == CTT_HTM || type == CTT_TXT || type == CTT_SDP)


typedef struct _http_msg_content
{
	uint32	    msg_type;
	uint32	    msg_sub_type;
	HDRV        first_line;

	PPSN_CTX    hdr_ctx;
	PPSN_CTX    ctt_ctx;

	int         hdr_len;
	int         ctt_len;
	HTTPCTT     ctt_type;
	char        boundary[256];

	char *      msg_buf;
	int         buf_offset;

	uint32	    remote_ip;
	uint16	    remote_port;
} HTTPMSG;

/*************************************************************************/
typedef struct http_client
{
	SOCKET      cfd;                // client socket
	uint32	    rip;                // remote ip, network byte order
	uint32	    rport;              // remote port
	
	char        lip[128];           // local server address
	uint32      lport;              // local server port

	char        rcv_buf[4096];      // static receiving buffer
	char *      dyn_recv_buf;       // dynamic receiving buffer
	int         rcv_dlen;           // received data length
	int         hdr_len;            // http header length
	int	        ctt_len;            // context  length
	HTTPCTT     ctt_type;           // context type
	char *      p_rbuf;				// pointer to rcv_buf or dyn_recv_buf
	int         mlen;				// sizeof(rcv_buf) or size of dyn_recv_buf

    void *      p_srv;              // pointer to HTTPSRV
    int         use_count;          // use count
    
    BOOL        pass_through;       // pass through received data flag
	void *      p_userdata;         // user data
	uint32      protocol;           // protocol, rtsp over http, websocket etc

#ifdef HTTPS
	SSL *       ssl;                // https SSL 
#endif	
} HTTPCLN;

typedef struct http_req
{
    uint32      need_auth : 1;      // need auth flag
    uint32      https     : 1;      // https flag
    uint32      resv      : 30;
    
	SOCKET      cfd;                // client socket    
	uint32	    port;               // server port
	char	    host[256];          // server host
	char	    url[256];           // the request url
	char	    user[64];           // login username
	char	    pass[64];           // login password

	char 	    action[256];        // action
	char	    rcv_buf[4096];      // static receiving buffer
	char *	    dyn_recv_buf;       // dynamic receiving buffer
	int		    rcv_dlen;           // received data length
	int		    hdr_len;            // http header length            
	int		    ctt_len;            // context  length
	char        boundary[256];      // boundary, for CTT_MULTIPART
	char *	    p_rbuf;				// pointer to rcv_buf or dyn_recv_buf
	int		    mlen;				// sizeof(rcv_buf) or size of dyn_recv_buf

	HTTPMSG *   rx_msg;             // rx message

	int  	    auth_mode;    		// 0 - baisc; 1 - digest
	HD_AUTH_INFO auth_info;         // http auth information
	
#ifdef HTTPS
	SSL *       ssl;                // https SSL 
#endif
} HTTPREQ;

/*************************************************************************/

/* 
 * If rx_msg is NULL, the callback should call the http_free_used_cln function to delete p_cln
 * If the callback is responsible for deleting rx_msg, it returns TRUE, otherwise it returns FALSE
 */
typedef BOOL (*http_msg_callback)(void * p_srv, HTTPCLN * p_cln, HTTPMSG * rx_msg, void * userdata);

typedef void (*http_data_callback)(void * p_srv, HTTPCLN * p_cln, char * buff, int buflen, void * userdata);

typedef struct http_srv_s
{
    uint32      r_flag  : 1;        // data receiving flag
    uint32      https   : 1;        // https flag
    uint32      resv    : 30;
    
	SOCKET      sfd;                // server socket

    char        host[128];          // local server address
	int         sport;              // server port
	uint32	    saddr;              // server address, network byte order
    uint32      max_cln_nums;       // max client number

	PPSN_CTX *  cln_fl;             // client free list
	PPSN_CTX *  cln_ul;             // client used list

	pthread_t   rx_tid;             // data receiving thread id

    void *      mutex_cb;           // cabllback mutex
    http_msg_callback  msg_cb;      // http message callback
    http_data_callback data_cb;     // http data callback
	void *      msg_user;           // http message callback user data
	void *      data_user;          // http data callback user data

#ifdef EPOLL
    int         ep_fd;              // epoll fd
	struct epoll_event * ep_events; // epoll events
	int         ep_event_num;       // epoll event number    
#endif
	
#ifdef HTTPS
    char        cert_file[256];     // cert file name
    char        key_file[256];      // key file name
  	SSL_CTX *   ssl_ctx;            // ssl context
#endif
} HTTPSRV;



#endif	//	__H_HTTP_H__




