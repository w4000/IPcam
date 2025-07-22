/*
 * posmodule.h
 *
 * purpose:
 *	for POS module.
 **************************************************************************************/


#ifndef __POSMODULE_H_
#define __POSMODULE_H_

// Module info
#define POS_MOD_MAX				16
#define __BUFSIZE					256

// POS return code
#define POS_ISRETCODE			(-101)

// Protocol define
#define __STARTCODE			0xAA
#define __ENDCODE			0xEE

#define __ACK					0x0A06
#define __NAK					0x0A15

#define __CORRECT_CONN		0x0B00
#define __TRANSCODE			0x0B01
#define __TRANSSTART			0x0B02
#define __TRANSEND			0x0B03
#define __TRANSCLEAR			0x0B04

#define __ERR_CRC			0xFF00
#define __ERR_DATLEN			0xFF01
#define __ERR_WRONGCMD		0xFF02
#define __ERR_TIMEOUT		0xFF03

#define __ERR_NOTCORRENT	0xFFF0

#define __ERR_ERROR			0xFFFF

#define __ERRCODE			0xFF
#define __RESPONCODE			0x0A
#define __CMDCODE			0x0B

enum POS_TYPE {
	POS_TYPE_COM = 1,

	POS_TYPE_TCPIP,

	POS_TYPE_MAX
};


// POS module tell us self information in this format.
typedef struct __POSMODINFO 
{
	char name[50];			// module name
	char version[10];		// module version
	char protocol_name[50];	// protocol name
} POSMODULEINFO;

// not used  ========================
typedef struct __POSMODOPT
{
	int szlnbrk;
	char lnbrk[20];
	int conntype;
} POSMODOPT;
// ================================

// parsing protocol LIB
typedef struct st_poslib {
	void *pos_mod;									// LIB
	int (* pos_parse)(char *, char *, int, int *);					// data buffer, retcode
	unsigned int (* pos_get_retcode)();
	void (* pos_get_module_info)(POSMODULEINFO *);
} ST_POSLIB;

//extern ST_POSLIB pos_lib[POS_MAX];
extern char * pos_modname [POS_MOD_MAX];

extern int pos_mod_info (char *, POSMODULEINFO *);
extern int pos_mod_init (int, ST_POSLIB *);
extern void pos_mod_release (ST_POSLIB *);
extern int postty_open(const char *, int, int, int, int);
extern void postty_close(int);

#endif	// __POSMODULE_H_
