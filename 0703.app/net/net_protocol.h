/*
 * net_protocol.h
 *
 * Purpose:
 *	- define diss network protocol format.
 *
 * written by:
 *	jungdh@sjcnc.com
 ***********************************************************************************/
#ifndef __NET_PROTOCOL_H
#define __NET_PROTOCOL_H

#include "file/ssf.h"

#define NET_PROT_REL		0
#define NET_PROT_MAJOR		0
#define NET_PROT_MINOR		0



/***********************************************************************************
 * Protocol IDs.
 ***********************************************************************************/

/* Each request and reply between client and server has it's own identification number(Protocol ID).
 * Protocol ID consists of 3 parts: Header ID(2 byte) + sub-type(1byte) + serial-num(1 byte).
 */
 // PREQBYPASSINFO
 // PUSRAUTH
 // PREQSTREAM
#define	PUSRAUTH				0x0A000001		// Request User authorization(client->Auth server).
#define	PREQSTREAM				0x0A000002		// Request a camera stream data.




#define	PSENDSTREAM				0x0A020101 		// Send file header/data to client.



// RELAY Protocol
#define REPLYID(reqid)			(0xA0000000 | (reqid))


//############################################################################################
#define PLOGFIRST				0xff00000		
//############################################################################################
#define PREQSWVER				0xff00001		
//############################################################################################
#define	PRECORDDATE				0xff00002		// Request record date list





#define PREQDEVINFO				0xff00003		// Request device information
#define PREQFIXINFO				0xff00004		// Request fixed system information
#define PREQDYNSTORAGE			0xff00005		// Request dynamic storage information

#define PREQUPGRADE				0xff00006		// Request firmware upgrade by remote side
#define PREQSCHEDBKUP			0xff00007		// Request scheduled backup

//############################################################################################
#define PREQREMOTESRCH			0xff00008		// Request remote search mode for SDK
#define PREQREMOTESRCHINFO		0xff00009		// Request current search info for SDK
#define PREQREMOTESRCHCTRL		0xff00010		// Request search control for SDK

#define PREQHEALTHINFO			0xff00020		// Request health information for SDK
//############################################################################################




#define PHEARTBEAT				0x0A7F0001 		// Send heart-beat to the server(client->Live/VOD server).




/***********************************************************************************
 * Protocol IDs. (for Jpeg)
 ***********************************************************************************/
// Each request and reply between client and server has it's own identification number(Protocol ID).
// Protocol ID consists of 3 parts: Header ID(2 byte) + sub-type(1byte) + serial-num(1 byte).
#define PMHEARTBEAT				0xff00030
#define PREQDVRINFO				0xff00040
#define PREQOSINFO				0xff00050
#define PREQPROTINFO			0xff00060

#define PMUSRAUTH				0xff00070
#define PREQMCLOSE				0xff00080
#define PREQJPGSTREAM			0xff00090
#define PREQMDISCONN			0xff00100

#define PREQMSTOP				0xff00200




#define PREQPUBLICINFO          0xff00300
#define PREQPUBLICINFO_UNI      0xff00400
#define PUSRENCAUTH             0xff00500
#define PUSRENCAUTH_UNI         0xff00600



#define PUSRENCAUTH_REINF      	0xff00700


/***********************************************************************************
 * Bypass Protocol IDs
 ***********************************************************************************/
#define PREQBYPASSINFO          0x0AF00000

/***********************************************************************************
 * New Focus Protocol IDs
 ***********************************************************************************/
#define PREQTERMINATE			0xff00800		// Request terminate socket

#define PLOGLAST				0xff00900		// Request last date/time of log data.
#define PREQCAMINFO				0x0BF00005		// Request CAM Info.


/***********************************************************************************
 * Result code will be sent to client. Defined according to Diss net protocol.
 ***********************************************************************************/

#define	RETFAIL			0x00000000
#define RETSUCCESS		0x00000001

#define	_RETSYSTEM		0xF0000000	/* system internal problem. */
#define	_RETUSER		0xF0010000	/* User certification related. */
#define _RETPERM		0xF0020000	/* Permission related. */
#define _RETCAM			0xF0030000	/* Camera related. */
#define _RETBACKUP		0xF0070000	/* Backup related. */
#define _RETDOWNLOAD	0xF0080000	/* Download related. */
#define _RETREMOTE		0xF0090000	/* Remote control related. */
#define _RETVOD			0xF00A0000	/* VOD service related. */
#define _RETCONN		0xF00B0000	/* Network connection related. */
#define _RETUPGR		0xF00C0000	/* Upgrade related. */
#define	_RETMISC		0xF00F0000	/* protocol ID related. */

#define RETNOUSR		_RETUSER | 0x1	/* No user account. */
#define	RETBADPWD		_RETUSER | 0x3	/* Wrong password. */
#define	RETDEFAULT		_RETUSER | 0x7	/* Default account */
#define	RETNOPERM		_RETPERM | 0x02	/* No permission for the requested service. */
#define	RETNOCAM		_RETCAM | 0x01	/* No camera. */


#define	RETMAXCONN		_RETCONN | 0x03	/* Maximum number of client connection has exceeded. */


#define	RETBADPROT		_RETMISC | 0x01	/* Unknown protocol ID. */
#define RETBADPARM		_RETMISC | 0x03	/* Invalid protocol parameter. */
#define	RETINTER		_RETMISC | 0xFF	/* Server internal error. */
#define	RETERRNET		_RETMISC | 0x20FF	/* Server network error. */



#define	PTZCTRL_PAN						0xffa00001
#define	PTZCTRL_TILT					0xffa00002
#define PTZCTRL_ZOOM					0xffa00004
#define PTZCTRL_FOCUS					0xffa00008
#define PTZCTRL_WIPER					0xffa00010
#define PTZCTRL_PUMP					0xffa00020
#define PTZCTRL_FAN						0xffa00040
#define PTZCTRL_HEATER					0xffa00080
#define PTZCTRL_CAMERA_POWER			0xffa00100
#define PTZCTRL_CAMERA_POWER_TIMEOFF	0xffa00200
#define PTZCTRL_CAMERA_POWER_CHECK		0xffa00400
#define PTZCTRL_IRIS					0xffa00800
#define PTZCTRL_LIGHT_POWER				0xffa01000
#define PTZCTRL_LIGHT_POWER_TIMEOFF		0xffa02000
#define PTZCTRL_LIGHT_POWER_CHECK		0xffa04000
#define PTZCTRL_AUTO_FOCUS				0xffa08000
#define PTZCTRL_EX_PRESET				0xffa00001
#define PTZCTRL_EX_PRESET_DEL			0xffa00002
#define PTZCTRL_EX_PRESET_ENG_NAME		0xffa00004
#define PTZCTRL_EX_PRESET_KOR_NAME		0xffa00008

#define PTZCTRL_EX_PRESET_NAME_NEEDED	0xffa00010
#define PTZCTRL_EX_PRESET_DUMMY02		0xffa00020
#define PTZCTRL_EX_PRESET_DUMMY03		0xffa00040
#define PTZCTRL_EX_PRESET_DUMMY04		0xffa00080

#define PTZCTRL_EX_SCAN					0xffa00100
#define PTZCTRL_EX_SCAN_TIME			0xffa00200
#define PTZCTRL_EX_SCAN_CHECK			0xffa00400
#define PTZCTRL_EX_SCAN_SETUP			0xffa00800

#define PTZCTRL_EX_SCAN_DELETE			0xffa01000
#define PTZCTRL_EX_SCAN_DUMMY01			0xffa02000
#define PTZCTRL_EX_SCAN_DUMMY02			0xffa04000
#define PTZCTRL_EX_DUMMY04				0xffa08000
#define PTZCTRL_EX_DUMMY05				0xffa10000
#define PTZCTRL_EX_DUMMY06				0xffa20000
#define PTZCTRL_EX_DUMMY07				0xffa40000
#define PTZCTRL_EX_DUMMY08				0xffa80000



enum TERM_CODE {
	TERM_CODE_MSG_STOP	= 100,
	TERM_CODE_MSG_EXIT,
	TERM_CODE_MSG_DISCONNECT_ALL_CLIENT,
	TERM_CODE_SENDMSGTO,
	TERM_CODE_RECVMSGFROM,
	TERM_CODE_SENDMSGTO2,
	TERM_CODE_RECVMSGFROM2,
	TERM_CODE_HANDLE_UNKNOWN_PROTOCOL2,
	TERM_CODE_COUNT
};




#define	MAX_NAME				(1<<6)
#define	MAX_USRID				(1<<5)
#define	MAX_PSETTITLE			(1<<5)
#define	MAX_PWD					MAX_USRID
#define	MAX_IPADDR				(1<<4)
#define MAX_CAM					32
#define	PRESET_TITLE_MAX 		32


#pragma pack(1)

typedef struct phdr {
	int	id;	
	int	err;
	int	len;
} PACK_ATTRIBUTE ST_PHDR;

typedef struct psendstream
{
	char	cam;
	char	type;		
	char data [0];		
} PACK_ATTRIBUTE ST_PSENDSTREAM;

typedef struct psendjpgstream
{
	char	cam;
	char	type;		
	short	width;
	short	height;
	int		date;
	int		time;
	int		rawsz;
	char data [0];		
} PACK_ATTRIBUTE ST_PSENDJPGSTREAM;


typedef struct st_netrequest {
	int 	date;
	int 	time;
	char 	dst;
	char 	ch;	 
} PACK_ATTRIBUTE ST_NETREQUEST;

typedef struct pusrauth {
	char	uid[MAX_USRID];
	char	upwd[MAX_PWD];
	char	oemcode;
	char	smscode;	
} PACK_ATTRIBUTE ST_PUSRAUTH;

typedef struct pusrauth_enc {
	char 	ver;
	char	uid[256];
	char	upwd[256];
	char	oemcode;
} PACK_ATTRIBUTE ST_PUSRAUTH_ENC;

/* PTZ info of a camera */
typedef struct ptzinfo {
	int		ptzact;
	int		ptzactext;
	char	maxsensor;
	char	maxalarm;
} PACK_ATTRIBUTE ST_PTZINFO;

typedef struct preset {
	char	num;
	char	title [32][MAX_PSETTITLE];
} PACK_ATTRIBUTE  ST_PRESET;

typedef struct pusrauth_resp {
	short	cammax; 	
	char	devtype;	
	int		cammask;  	
	int		recmask;	   	
	int		covertmask;	
	int		permmask;	

	struct ptzinfo	ptz [MAX_CAM];
	struct preset	pset [MAX_CAM];

	char	twoway;		
} PACK_ATTRIBUTE  ST_PUSRAUTH_RESP;

typedef struct auth_parti
{
	char	connect;
	char	rec_model;
	char	protect;
	char	frame_rate1;
	char	frame_rate2;

} PACK_ATTRIBUTE AUTH_PARTI;


typedef struct gdtour {
	__u8	point_info [GDTOUR_MAX * 2];
} PACK_ATTRIBUTE ST_GDTOUR;

typedef struct pgdtour_set
{
	__u8	cam;
	struct gdtour gdtour;
} PACK_ATTRIBUTE ST_PGDTOUR_SET;

/* Request and reply of PREQFASTPLAY */
typedef struct preqfastplay
{
	char 	cam;
	int		timestamp;
	char	direction;
} PACK_ATTRIBUTE ST_PREQFASTPLAY;

typedef struct preqfastplay_resp
{
	char 	cam;
	int		timestamp;
} PACK_ATTRIBUTE ST_PREQFASTPLAY_RESP;

/* Request and reply of PREQPLAY */
typedef struct preqplay
{
	char 	cam;
	int		timestamp;
	char	direction;
} PACK_ATTRIBUTE ST_PREQPLAY;

typedef struct preqplay_resp
{
	char 	cam;
	int		timestamp;
} PACK_ATTRIBUTE ST_PREQPLAY_RESP;

typedef struct preqgotoplay
{
	int		cam;
	int		date;
	int		time;
	char	scheme;		// 1: SEEK_EXACT, 0: SEEK_FORWARD or SEEK_BACKWARD
	char	dst;
} PACK_ATTRIBUTE ST_PREQGOTOPLAY;

typedef struct preqgoto_resp
{
	char 	cam;
	int		timestamp;
} PACK_ATTRIBUTE ST_PREQGOTO_RESP;

typedef struct vodtransctrl_resp
{
	char 	cam;
	char	frame_type;
	unsigned int	timestamp;
	int		reserved[4];
} PACK_ATTRIBUTE ST_VODTRANSCTRL_RESP;


typedef struct pptzctrl_preset
{
	__u8 	ch;
	__u8 	preset;
	__u16	automode;

} PACK_ATTRIBUTE ST_PPTZCTRL_PRESET;

#define PRESET_TITLE_MAX 32

typedef struct pptzctrl_preset_title
{
	__u8 	ch;
	__u8 	preset;
	char	title [PRESET_TITLE_MAX];

} PACK_ATTRIBUTE ST_PPTZCTRL_PRESET_TITLE;


typedef struct pptzctrl_aux
{
	char ch;
	char func;
	char index;
	char action;
} PACK_ATTRIBUTE ST_PPTZCTRL_AUX;


/* Request of protocol PMODCH. */
typedef struct pmodch
{
	char ch;
} PACK_ATTRIBUTE ST_PMODCH;

/* Request an reply of protocol PREQFILELIST */
typedef struct preqfilelist
{
	char	svrname[MAX_NAME];
	char	ch;
	int	date;
} PACK_ATTRIBUTE ST_PREQFILELIST;

typedef struct fileinfo {
	int	date;
	int	time;
	char	dst;
	int	size;
} PACK_ATTRIBUTE ST_FILEINFO;

typedef struct preqfilelist_resp
{
	char	ch;
	short num;
	struct fileinfo fileinfo[0];
} PACK_ATTRIBUTE ST_PREQFILELIST_RESP;


// 2way
typedef struct preq2way
{
	char startstop;	// 0 : STOP,   1 : START
	char	duplex;		// 0 : simplex, 1 : duplex.
	char reserved [4];
} PACK_ATTRIBUTE ST_PREQ2WAY;


typedef struct preqremotecopy {
	char cam;
	int date;
	int time;
} PACK_ATTRIBUTE ST_PREQREMOTECOPY;

typedef struct preqremotecopy_resp {
	char cam;
	int filesz;
	char data [0];
} PACK_ATTRIBUTE ST_PREQREMOTECOPY_RESP;

typedef struct preqstream
{
	char svrname [MAX_NAME];
	char type;		// 0 : Live , 1 : VOD
	char cam;
	int	date;
	int 	time;
	char dst;
	char dir;			//  0 : Normal FW, 1 : Fast FW, 2 : Normal BW, 3 : Fast BW
} PACK_ATTRIBUTE ST_PREQSTREAM;

typedef struct preqmstream
{
	char cam;
	char type;		// 0: Live, 1: VOD
	char ftype;		//0: jpeg, 1:H.264
	int	date;
	int time;
} PACK_ATTRIBUTE ST_PREQMSTREAM;


typedef struct preqstream_resp
{
	char	cam;
} PACK_ATTRIBUTE ST_PREQSTREAM_RESP;

typedef struct preqnextplay
{
	char cam;
} PACK_ATTRIBUTE ST_PREQNEXTPLAY;


typedef struct preqdisconn {
	char cam;
} ST_PREQDISCONN;

/* MMS PTZ action request.*/
typedef struct pmptzctrl
{
	unsigned char ch;
	unsigned char ptzaction;
} PACK_ATTRIBUTE ST_PMPTZCTRL;

typedef struct pmptzctrl_preset
{
	__u8 	ch;
	__u8 	preset;
	__u16	automode;

} PACK_ATTRIBUTE ST_PMPTZCTRL_PRESET;

#pragma pack()


#define setfield(ftype, fname, fval)	\
	ftype##field((fname), (fval))



#define intfield(fname, fval)	do {\
	unsigned __tmp;	\
	__tmp = *fval;	\
	__tmp |= (*(fval +1) << 8);	\
	__tmp |= (*(fval +2) << 16);	\
	__tmp |= (*(fval +3) << 24);	\
	fval += sizeof(int);	\
	fname = ntohl(__tmp);		\
	} while (0)


#define charfield(fname, fval)	do {\
	fname = *fval;	\
	fval+=sizeof(char);	\
	} while (0)

#define stringfield(fname, fval)	do { \
	fval+=transtr(fname, fval);	\
	} while (0)


#define setnetbuf(buf, ftype, fval)	\
	ftype##netbuf((buf),(fval))

#define	setnetbufraw(dest, len, src) \
	do {\
		memcpy(dest, src, len); \
		dest+=len;\
	} while (0)


#define intnetbuf(buf, fval)	do { \
	unsigned __tmp = htonl(fval);	\
	*(char*)buf = __tmp;	\
	buf +=sizeof(char);	\
	*(char*)buf = __tmp >> 8;	\
	buf +=sizeof(char);	\
	*(char*)buf = __tmp >> 16;	\
	buf +=sizeof(char);	\
	*(char*)buf = __tmp >> 24;	\
	buf +=sizeof(char);	\
	} while (0)

#define shortnetbuf(buf, fval)	do {\
	unsigned short __tmp = htons(fval);	\
	*(char*)buf = __tmp;	\
	buf +=sizeof(char);		\
	*(char*)buf = __tmp >> 8;	\
	buf +=sizeof(char);		\
	} while (0)


#define charnetbuf(buf, fval)	do {\
	*(char *)buf = fval;\
	buf+=sizeof(char);\
	} while (0)

#define stringnetbuf(buf, fval)	do {\
	buf+=transtr2(buf, fval);		\
	} while (0)

struct st_stream;
struct st_mstream;

extern int transtr(char *, char *);
extern int transtr2(char *, char *);
extern int transtr3(char *, char *, int);

extern int transip(char *, char *);
extern int transip2(char *, char *);

extern int readphdr(struct st_stream *, int, struct phdr *);
extern void attachphdr (char *, int, int, int);

extern int sendmsgto (struct st_stream *, int, char *, size_t);
extern int reply2client(struct st_stream *, int, char *, int, int, size_t);
extern int recvmsgfrom(struct st_stream *, int, char *, size_t);



#endif
