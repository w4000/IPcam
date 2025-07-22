#include "common.h"
#include "lib/net.h"


/* Transform string fromat.
 * Strings in "Diss" network protocol contain 2 byte lenth data in front of pure string.
 * Transform protocol string format to NULL terminated normal string.
 * Argument:
 *	@fname : destination.
 *	@fval : source.
 * Return:
 *	length of pure data string including NULL + 2(length itself), on sucess.
 *	-ERR_CODE. on error.
 */

static int __websocket_recvn(struct st_stream * svr, const char * buf, int len);





int transtr(char *fname, char *fval) {
	unsigned short len=0;

	assert (fname);
	assert (fval);
#ifdef ARCH_ARM
	len = *fval | *(fval +1) << 8;
	len = ntohs(len);
#endif	/* ARCH_ARM */

	strncpy(fname, fval+sizeof(short), (size_t)len);
	fname[len]='\0';

	return (len+sizeof(short));
}

int transtr2(char *fname, char *fval) {
	unsigned short len=0;
	unsigned short dmy;		

	assert (fname);
	assert (fval);

	len = strlen(fval)+1;
	strncpy(fname+sizeof(short), fval,(size_t)len);

#ifdef ARCH_ARM
	dmy = htons(len);

	*fname = (char)dmy;
	*(fname +1) = (char)(dmy >> 8);
#else
	(*(short *)fname) = htons(len);
#endif

	return (len+sizeof(short));
}


/* Reverse translation of transtr.
 * Argument:
 *	@fname : destination.
 *	@fval: source.
 * Return:
 *	length of pure data string including NULL + 2(length itself).
 */
int transtr3(char *fname, char *fval, int len) {
	assert (fname);
	assert (fval);

	memset(fname, 0x00, len);
	strcpy(fname, fval);

	return len;
}


int transip(char *fname, char *fval) {
	char ip_addr[32];

	sprintf(ip_addr, "%d.%d.%d.%d", *fval, *(fval +1), *(fval+2), *(fval +3));
	memcpy(fname, ip_addr, 4);

	return 4;
}


int transip2(char *fname, char *fval) {
	int ipaddr[4];
	__u8 tmp[4];

	sscanf(fval, "%d.%d.%d.%d", &ipaddr[0], &ipaddr[1], &ipaddr[2], &ipaddr[3]);

	tmp[0] = (__u8)ipaddr[0];
	tmp[1] = (__u8)ipaddr[1];
	tmp[2] = (__u8)ipaddr[2];
	tmp[3] = (__u8)ipaddr[3];

	memcpy(fname, tmp, 4);

	return 4;
}

/***********************************************************************************
 * Reading and writing protocol header.
 ***********************************************************************************/

// Read protocol header from socket.
int readphdr (struct st_stream * svr, int sd, struct phdr *phdr) {
	struct phdr buf;
	int ret = 0;


	ret = recvmsgfrom (svr, sd, (char *) & buf, sizeof (struct phdr));

	phdr->id = __ARCH_READ_NET__ (buf.id);
	phdr->err = __ARCH_READ_NET__ (buf.err);
	phdr->len = __ARCH_READ_NET__ (buf.len);


	return ret;
}
/* Attach protocol header in front of message to be sent to client.
 */
void attachphdr (char *buf, int id, int err, int len)
{
	setnetbuf(buf, int, id);
	setnetbuf(buf, int, err);
	setnetbuf(buf, int, len);
}


/***********************************************************************************
 * High level message transmission and receive.
 ***********************************************************************************/

#if _USE_WEBSOCKET_SERVER_
static int __websocket_sendn(struct st_stream * svr, char * msg, int len)
{
#if 0
	// printf("! __websocket_sendn 	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	printf("send msg ======================================================\n");
	int i;
	int first = 1;
	for (i; i<len; i++)
	{
		if (i < 12)
			printf("%02x", msg[i]);
		else 
		{
			if (first)	{printf("\n"); first = 0;}

			printf("%c", msg[i]);
		}

		if ((((i+1)%4) == 0) && i != 0 && i <= 11)
		{
			// printf("(%d)", i);
			printf("\n");
		}

	}

	printf("\n===============================================================\n\n\n");
	// printf("! %s:%d >>> svr->websocket_session? %d, svr->websocket_session->session_fd? %d \n", __func__, __LINE__, svr->websocket_session, svr->websocket_session->session_fd);

#endif

	int ret = -ERR_SOCK;
	if (svr->websocket_session)
	{
		ret = ws_sendframe_bin(svr->websocket_session->session_fd, msg, len, FALSE);
	}
	return ret;
}
#endif


int sendmsgto (struct st_stream * svr, int sd, char * buf, size_t sz)
{
	int ret =0;

	assert (buf);
	assert (sd >= 0);

	// printf("! sendmsgto >>> is_websocket? %d \n", svr->is_websocket);
#if _USE_WEBSOCKET_SERVER_	
	if (svr->is_websocket) {
		ret = __websocket_sendn (svr, buf, sz);
	}
	else 
#endif
	{
		ret = __sendn (sd, buf, sz);
	}

	if (ret < 0)
	{
		if (svr)
			svr->terminate = TERM_CODE_SENDMSGTO;
	}

	return ret;
}


/* Receive message from client.
 * Argument:
 *	@sd : socket.
 *	@buf : data buffer.
 *	@sz : data size to receive.
 *
 * Return:
 *	0 on success, -ERR_CODE on failure.
 */

int recvmsgfrom (struct st_stream * svr, int sd, char * buf, size_t sz) {
	int ret;

	assert (buf);

	{
		assert (sd >= 0);
	#if _USE_WEBSOCKET_SERVER_
		if (svr->is_websocket)
		{
			ret = __websocket_recvn (svr, buf, sz);

#if 0
			{
				int i;
				printf("sz? %d \n", sz);
				printf("recv msg ======================================================\n");
				for (i=0; i<sz; i++)
				{
					if (sz == 12)
						printf("%02x", buf[i]);
					else 
						printf("%c", buf[i]);

					if ((((i+1)%4) == 0) && i != 0 && i <= 11)
					{
						// printf("(%d)", i);
						printf("\n");
					}
				}
				printf("---------------------------------------------------------------\n\n");

				for (i=0; i<sz; i++)
				{
					printf("%c", buf[i]);
				}
				printf("\n=========================================================== end\n");
			}
#endif

		}
		else
	#endif
		{
			ret = __recvn (sd, buf, sz);
		}
		// printf("! %s:%d >>> ret? %d \n", __func__, __LINE__, ret);


	}
	if (ret < 0)
	{
		// printf("! %s:%d >>> buf? %s \n", __func__, __LINE__, buf);
		switch (ret)
		{
		case -ERR_SOCK:
			printf ("Socket Error!\n");
			break;
		case -ERR_DISCONN:
			printf ("Socket disconnected\n");
			break;
		case -ERR_TIMEOUT:
			printf ("Socket timed out\n");
			break;
		}
		if (svr)
		{
			svr->terminate = TERM_CODE_RECVMSGFROM;
		}
	}
	return ret;
}


/* Send reply message to client.
 * Argument:
 *	@sd : socket to send message.
 *	@buf : data buffer to send.
 *	@id : reply ID to be inserted in protocol header.
 *	@err : result code.
 *	@sz : size of pure message data.
 *
 * Return:
 *	0 on success, -ERR_CODE on failure.
 * Note:
 *	Caller should fill pure data starting from buf+sizeof(struct phdr),
 *	because protocol header is inserted in here.
 */
int reply2client(struct st_stream * svr, int sd, char *buf, int id, int err, size_t sz) {

	assert (buf);
	assert (sd >= 0);
	assert (sz >= 0);

	attachphdr(buf, id , err, sz);

#if 0
	if (id != 0xaa020101)
		printf ("SVR: PID replied=0x%08x, err=0x%08x, sz=%d\n", id, err, sz);
#endif

	return sendmsgto(svr, sd, buf, sz +sizeof(struct phdr));
}








#if _USE_WEBSOCKET_SERVER_

static int __websocket_recvn(struct st_stream * svr, const char * buf, int len)
{
	int ret = -ERR_SOCK;

	WEBSOCKET_SESSION_LOCK(svr->websocket_session_id);
	{
		if (!svr->terminate)
		{
			CCircularBuffer_t *queue = &(svr->websocket_session->queue);
			if (queue)
			{
				if (CCircularBuffer_IsEmpty(queue))
				{
					ret = 0;
				}
				else
				{
					ret = CCircularBuffer_Pop(queue, (__u8*)buf, len);
					if (ret < 0)
						ret = -ERR_SOCK;
				}
			}
		}
	}
	WEBSOCKET_SESSION_UNLOCK(svr->websocket_session_id);

	return ret;
}

#endif
