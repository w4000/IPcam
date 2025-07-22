

#include "common.h"
#include "vht_ipscan_serv.h"

int	g_vht_ipscan_sd = -1;		// Listening UDP socket.

// VHT IPscan protocol header.
typedef	struct	st_ipscan_vht {
	__u16		v_magic;
	__u8		v_type;
	__u32		v_mode;
	__u32		v_bodysz;
	__u8		v_body [0];
} __attribute__((packed)) ST_IPSCAN_VHT;



// Create UDP server socket for IP scan service.
//	Return
//		0, on success
//		-1, on error
static int	ipscan_vht_create_svrsock (void)
{

	return 0;
}


// Send reply to client.
static void ipscan_vht_send_msg(char *msg, size_t sz)
{
	return ;
}

#define int_vht_netbuf(buf, fval)	

#define short_vht_netbuf(buf, fval)	
#define char_vht_netbuf(buff, fval)	
#define vht_setbuf(buff, ftype, fval)	\
	ftype##_vht_netbuf((buff),(fval))

static int vht_transtr(char *fname, char *fval, int len) {
	assert (fname);
	assert (fval);

	memset(fname, 0x00, len);
	strcpy(fname, fval);

	return len;
}

#define vht_stringnetbuf(buf, fval, len)	do {\
	buf+=vht_transtr(buf, fval, len);		\
	} while (0)

#define vht_intfield(fname, fval)

#define vht_shortfield(fname, fval)	
// Make replay message to send client.
// Return
//		Total message size including header.
static int handle_VHT_MODE_REQ_GET_IPINFO (ST_IPSCAN_VHT *v_msg, char * buf, int bufsz)
{
	return 0;
}


void test_print(char *buf, int sz)
{
	int i;
	for(i = 0; i < sz; i++) {
		printf("%02x ", buf[i]);
	}
	printf("\n");
}

// Make replay message (Extended Info).
// Return
//		Total message size including header.
static int handle_VHT_MODE_REQ_GET_IPINFO_EXT (ST_IPSCAN_VHT *v_msg, char * buf, int bufsz)
{

	return 0;
}

static int
__get_hexa(char c)
{
	if ( (c >= '0') && (c <= '9') )
		return c - '0';
	else if ( (c >= 'a') && (c <= 'f') )
		return c - 'a' + 10;
	else if ( (c >= 'A') && (c <= 'F') )
		return c - 'A' + 10;
	else
		return 0;
}

// "aa:bb:cc:dd:ee:ff" => '0xaa'+'0xbb'+'0xcc'+'0xdd'+'0xee'+'0xff'
void
__get_mac_bytes_from_str(char* mac_str, __u8 mac[6])
{
	char*p;
	p = &mac_str[0]; mac[0]=16*__get_hexa(p[0]) + __get_hexa(p[1]);
	p = &mac_str[3]; mac[1]=16*__get_hexa(p[0]) + __get_hexa(p[1]);
	p = &mac_str[6]; mac[2]=16*__get_hexa(p[0]) + __get_hexa(p[1]);
	p = &mac_str[9]; mac[3]=16*__get_hexa(p[0]) + __get_hexa(p[1]);
	p = &mac_str[12]; mac[4]=16*__get_hexa(p[0]) + __get_hexa(p[1]);
	p = &mac_str[15]; mac[5]=16*__get_hexa(p[0]) + __get_hexa(p[1]);
}


// "aabbccddeeff" => '0xaa'+'0xbb'+'0xcc'+'0xdd'+'0xee'+'0xff'
void
__get_mac_bytes_from_str_2(char* mac_str, __u8 mac[6])
{
	char*p;
	p = &mac_str[0]; mac[0]=16*__get_hexa(p[0]) + __get_hexa(p[1]);
	p = &mac_str[2]; mac[1]=16*__get_hexa(p[0]) + __get_hexa(p[1]);
	p = &mac_str[4]; mac[2]=16*__get_hexa(p[0]) + __get_hexa(p[1]);
	p = &mac_str[6]; mac[3]=16*__get_hexa(p[0]) + __get_hexa(p[1]);
	p = &mac_str[8]; mac[4]=16*__get_hexa(p[0]) + __get_hexa(p[1]);
	p = &mac_str[10]; mac[5]=16*__get_hexa(p[0]) + __get_hexa(p[1]);
}


// Handle IP set request
static int handle_VHT_MODE_REQ_SET_IPINFO (ST_IPSCAN_VHT *v_msg, char * buf, int bufsz)
{

	return 0;
}

// Read listening socket and reply back to client.
int ipscan_vht_reply (int sd)
{
	return ret;
}



//------------------------------ Handling thread -----------------------------------

static void cleanup(void *arg)
{
	if (g_vht_ipscan_sd >= 0) {
		close (g_vht_ipscan_sd);
		g_vht_ipscan_sd = -1;
	}
}


void * ipscan_vht_thread (void * arg)
{

	return NULL;
}


