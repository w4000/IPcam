/*
 * poslib.h
 *
 * purpose:
 *	POS library.
 **************************************************************************************/


#ifndef __POSLIB_H_
#define __POSLIB_H_

#ifdef ARCH_ARM
#define PACK_ATTRIBUTE		__attribute__ ((packed))
#else
#define PACK_ATTRIBUTE
#endif

// mmi_network_'s configuration
#define POS_MAX_SCR_LINE			15

#define POSDATA_CRLF				0
#define POSDATA_CR					1
#define POSDATA_LF					2
#define POSDATA_CUSTOM				3

#define POSDATA_HEX					0
#define POSDATA_STRING				1

#define POS_TRANS_START				1
#define POS_TRANS_DATA				0
#define POS_TRANS_END				3
#define POS_TRANS_NOTRANS			2

// ============================
// Search configuration
#define MAX_POFSEARCH_INDEX		500
#define LINE_PER_PAGE				10
#define SEARCH_BUFFER_SIZE			512
#define MAX_POS_LNDATA				128

// ============================
// General info
#define POS_TTY_DEVICE				"/dev/ttySL1"
#define POS_USBTTY_DEVICE			"/dev/ttyUSB"
#define POS_DEFAULT_PORT			9020

#define POS_INDEX_EXT				".pdx"
#define POS_DATA_EXT				".pof"

#define POS_DATA_SIZE				128
#define RECV_DATA_LEN				128

#define MAX_PDXSIZE 				(1*M)

#define POSINFODATA_MAX 		20		// Start, end data length
#define LNBRK_LENGTH 			6		// line break length
#define POSIP_MAX					16		// 192.168.000.001 + '\0'

#define MAX_POS_LINKLIST			32

typedef struct __datetime
{
	int date;
	int time;
} DATETIME;

typedef struct st_ttyinfo {
	int ttytype;			// Rear COM without MUX or with MUX
	int baudrate;
	int parity;
	int databit;
	int stopbit;
} ST_TTYINFO;

typedef struct st_dispinfo {
	int displine;			// display line
	int cleartm;			// display clear time
	int position;			// left top, right top, left bottom, right bottom
	int show;				// display show or hide
	int scroll;				// display scroll direction
	int fontcolor;			// font color (gray)
} ST_DISPINFO;

// POS = caution : use in SETUP struct
typedef struct st_posinfo {
	int camera;
	int conntype;				// tcpip or serial
	char posip[POSIP_MAX];		// tcpip POS device ip
	int protocol;

	int tstartchar;				// string or HEX
	char startchar[POSINFODATA_MAX];
	int tendchar;					// string or HEX
	char endchar[POSINFODATA_MAX];
	int tlnbrk;					// CR, LF, CRLF or custom
	char lnbrk[LNBRK_LENGTH];

	int save;				// save, don't save
	int port;				// tcpip port

	ST_DISPINFO livedispinfo;

	int reserved[4];		// reserved
} ST_POSINFO;

// use configuration structure in pos thread
typedef struct st_posinfo_tmp {
	int id;			// MUX device address
	int use;
	int camera;
	char posip[POSIP_MAX];
	int protocol;
	int conntype;
	int save;
	int port;

	int tstartchar;
	int szstartchar;
	char startchar[POSINFODATA_MAX];
	int tendchar;
	int szendchar;
	char endchar[POSINFODATA_MAX];
	int tlnbrk;
	int szlnbrk;
	char lnbrk[LNBRK_LENGTH];

	ST_DISPINFO liveinfo;
	ST_TTYINFO ttyinfo;
} ST_POSINFO_TMP;

// used playback for store SSF
#pragma pack(1)
// POS struct index and data		robert
typedef struct __pdxindex {
	unsigned long int offset;		// position
	unsigned long int timecode;		// save time
} PACK_ATTRIBUTE PDXINDEX;

typedef struct __pofinfo {
	int ch;
	__u64 time;					// save time
	int start;						// transaction start code
	int tlnbrk;
	int szlnbrk;
	char lnbrk[LNBRK_LENGTH];		// line break
	int szdata;					// buf size
	char data[0];					// save data
} PACK_ATTRIBUTE POFINFO;

typedef struct __pdxinfo {
	unsigned long int id;       // pre-defined unique id = 0xFFFB0001  EVENTID
	unsigned long int size;     // structure size = sizeof(PDXINFO) + (sizeof(PDXINFO) * (count_of_index - 1))
	unsigned short int count;    // count of index
	PDXINDEX index[0];
} PACK_ATTRIBUTE PDXINFO;
#pragma pack()



typedef struct st_preqposdata
{
	int time;
	int szdata;
	char data[MAX_POS_LNDATA];
} ST_PREQPOSDATA;

/* Last POS search data structure */
typedef struct st_possearchdata
{
	int ch;					// channel
	struct tm from;			// from time
	struct tm to;			// to time
	char keyword[32+1];		// keyword
} ST_SEARCHPOSDATA;

typedef struct st_search_pos_data
{
	int no;
	int date;
	int time;
	struct tm sttm;
	int szdata;
	char data[MAX_POS_LNDATA];
} ST_SEARCH_POSDATA;

typedef struct {
	int 	ch;
	__u64	time;
	int		start;
	int		tlnbrk;
	int		szInbrk;
	char	Inbrk[6];
	int		szData;
	char	data[0];

} __attribute__((__packed__)) POS_DATA;






typedef struct _pos_search_data {
	int 	time;
	int 	szdata;
	char* 	data;
	struct _pos_search_data* next;
} POS_SEARCH_DATA;


#endif	// __POSLIB_H_

