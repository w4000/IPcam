#ifndef __LOG_H
#define	__LOG_H

enum	log_type {
/*------------------- Log type "system" ------------------*/
	LOGTYPE_SYSTEM = 0,			// System log ex)power on/off, rec on/off, ...
	LOGTYPE_SYS_START,	
	LOGTYPE_SYS_END,
	LOGTYPE_SYS_RESTART,
	LOGTYPE_SYS_LOGIN,	
	LOGTYPE_SYS_LOGOUT,
	LOGTYPE_SYS_UPGRADE,
	LOGTYPE_SYS_DISKFMAT,
	LOGTYPE_SYS_DISKFAIL,
	LOGTYPE_SYS_DATADEL,
	LOGTYPE_SYS_PRINT,			// Printer out.
	LOGTYPE_SYS_REMCONF,		// Remote configuration initiated.
	LOGTYPE_SYS_PANICBG,		// Begin pannic recording.
	LOGTYPE_SYS_PANICEND,		// End of pannic recording.	
	LOGTYPE_SYS_EVTNOTIFY,
	LOGTYPE_SYS_START_WATCHDOG_REBOOT,	// system start by watchdog reboot
	LOGTYPE_SYS_START_WATCHDOG_RESET,	// system start by watchdog reset

/*------------------- Log type "event" ---------------------*/
	LOGTYPE_EVNT	= 50,
	LOGTYPE_EVNT_SENSOR,
	LOGTYPE_EVNT_MOT,			//  Motion event.
	LOGTYPE_EVNT_VIDLOSS,
	LOGTYPE_EVNT_DISKFULL,		
	LOGTYPE_EVNT_RECFAIL,
	LOGTYPE_EVEN_DISKSMART,
	LOGTYPE_EVNT_POS,
	LOGTYPE_EVNT_LOGON_FAIL,
	
/*------------------- Log type "setup" ---------------------*/
	LOGTYPE_SETUP = 100,
	LOGTYPE_SETUP_BG,		// Begin system setup.
	LOGTYPE_SETUP_END,		// End system setup.
	LOGTYPE_SETUP_TMSET,	// Time setting changed.
	
/*------------------- Log type "Network" ---------------------*/
	LOGTYPE_NET = 150,
	LOGTYPE_NET_START,			// Network Conn Start
	LOGTYPE_NET_END,			// Network Conn End

/*------------------- Log type "Recording" ---------------------*/
	LOGTYPE_REC = 200,
	LOGTYPE_REC_CONTINUE,		// Continuous Recording
	LOGTYPE_REC_MOTION,			// Motion Recording
	LOGTYPE_REC_SENSOR,			// Sensor Recording
	LOGTYPE_REC_EMERG,
	
	
// These types are not appeared in log file. 
	LOGTYPE_ANY = 254,
	LOGTYPE_INVAIL = 255,
};


/* main types of log */
#define LOGBIT_ALL				0xffff

#define LOGBIT_SYSTEM 			1
#define LOGBIT_SETUP 			(1 << 1)
#define LOGBIT_NETWORK			(1 << 2)
#define LOGBIT_EVNT				(1 << 3)
#define LOGBIT_REC				(1 << 4)

/* sub types of log */
#define LOGBIT_SUB_ALL			0xffff

#define LOGBIT_SYS_START		1
#define	LOGBIT_SYS_END			(1 << 1)
#define	LOGBIT_SYS_RESTART		(1 << 2)
#define	LOGBIT_SYS_LOGIN		(1 << 3)
#define	LOGBIT_SYS_LOGOUT		(1 << 4)
#define	LOGBIT_SYS_UPGRADE		(1 << 5)
#define	LOGBIT_SYS_DISKFMAT		(1 << 6)
#define	LOGBIT_SYS_DISKFAIL		(1 << 7)
#define	LOGBIT_SYS_DATADEL		(1 << 8)
#define	LOGBIT_SYS_PRINT		(1 << 9)	// Printer out.
#define	LOGBIT_SYS_REMCONF		(1 << 10)	// Remote configuration initiated.
#define	LOGBIT_SYS_PANICBG		(1 << 11)	// Begin pannic recording.
#define	LOGBIT_SYS_PANICEND		(1 << 12)	// End of pannic recording.	
#define	LOGBIT_SYS_EVTNOTIFY	(1 << 13)

#define	LOGBIT_EVNT_SENSOR		1
#define	LOGBIT_EVNT_MOTION		(1 << 1)	//  Motion event.
#define	LOGBIT_EVNT_VIDLOSS		(1 << 2)
#define	LOGBIT_EVNT_DISKFULL	(1 << 3)
#define	LOGBIT_EVNT_RECFAIL		(1 << 4)
#define	LOGBIT_EVEN_DISKSMART	(1 << 5)
#define	LOGBIT_EVNT_POS			(1 << 6)
	
#define	LOGBIT_SETUP_BG			1			// Begin system setup.
#define	LOGBIT_SETUP_END		(1 << 1)	// End system setup.
#define	LOGBIT_SETUP_TMSET		(1 << 2)	// Time setting changed.

#define	LOGBIT_NET_START		1			// Start Network connection.
#define	LOGBIT_NET_END			(1 << 1)	// End Network Connection.

#define	LOGBIT_REC_CONT			1			// Continuous recording.
#define	LOGBIT_REC_SENS			(1 << 1)	// Recording by sensor.
#define	LOGBIT_REC_MOT			(1 << 2)	// Recording by motion event.


/*****************************************************************************
 * Log file structure.
 * Log file is consisted of header and array of fixed number of log entry.
 *****************************************************************************/
 
#pragma pack(1)

typedef struct _logent {
	__u8		typ1;						// log message type.
	__u8		typ2;						// log message sub type.
	unsigned	info;						// Misc log info.
	char		info2[32];					// Misc log info 2.
	struct	tm tm;
} PACK_ATTRIBUTE ST_LOGENT;

typedef struct _logent_for_mobile {
	unsigned char    typ1;               // log message type.                 1 Byte
	unsigned char    typ2;               // log message sub type.            1 Byte
	unsigned int     info;               // Misc log info.                       4 Bytes
	char             info2[32];          // Misc log info 2.                     32 Bytes
	unsigned int     data[4];            // Misc log data.                      16 Bytes
	char             usr[32];            // usr info.                             32 Bytes
	int				sec;
	int				min;
	int				hour;
	int				day;
	int				month;
	int				year;
	char			skip[20];
} PACK_ATTRIBUTE ST_LOGENT_FOR_MOBILE;


typedef struct _loghdr {
	char	magic[16];		// Log file signature.
	unsigned	nument;		// Total number of log entries.
	unsigned	numsys;		// Number of system type log entries.
	unsigned	numevnt;	// Number of event type log entries.
	unsigned	numsetup;	// Number of setup type log entries.
	unsigned	numnet;		// Number of network type log entries.
	unsigned 	numrec;		// Number of record type log entries.
	unsigned	reserved [4]; // Reserved for future use.
	
	struct	tm	date;		// Creating date of log file. 
							// To maintain a log file for each day.							
} PACK_ATTRIBUTE ST_LOGHDR;

#pragma pack()


#define LOGFILE_PATH					"/mnt/sda1/log/"
#define RDXFILE_PATH					"/mnt/sda1/"

// Get full path name of a rdx file with given date.
#define RDXFILE_GET_NAME(_buf, _date)	\
do {\
	char __tmp__ [16];\
	itoa (_date, __tmp__);\
	sprintf (_buf, "%s%s/%s.rdx", RDXFILE_PATH, __tmp__, __tmp__);\
} while (0)

// Get full path name of a log file with given date.
#define LOGFILE_GET_NAME(_buf, _date)	\
do {\
	char __tmp__ [16];\
	itoa (_date, __tmp__);\
	sprintf (_buf, "%s%s.log", LOGFILE_PATH, __tmp__);\
} while (0)
	

#define LOGHDR_MAGIC			"log file"

#define LOGENT_IS_TYP1(ent, type)	((ent)->typ1 == (type))
#define LOGENT_IS_TYP2(ent, type)	((ent)->typ2 == (type))
#define LOGENT_IS_TYPE(ent, typ1, typ2)	(LOGENT_IS_TYP1(ent, typ1) && LOGENT_IS_TYP2(ent, typ2))


typedef struct _log {
	int fd;					// log file descriptor.
	char	*mmap;			// Mmapped log file header.
	char	*mmapend;		// End of mmap.
	
	int		readidx;		// Current entry index for reading.
} ST_LOG;

#define LOG_INIT			{ .fd = -1, .mmap = (char *)-1, .mmapend = (char *)-1, .readidx = 0}
#define __LOG_HDR(log)		((ST_LOGHDR *)((log)->mmap))
#define LOG_NUMENT(log)		(__LOG_HDR(log)->nument)
#define LOG_NUMSYS(log)		(__LOG_HDR(log)->numsys)
#define LOG_NUMEVNT(log)	(__LOG_HDR(log)->numevnt)
#define LOG_NUMSETUP(log)	(__LOG_HDR(log)->numsetup)
#define LOG_NUMNET(log)		(__LOG_HDR(log)->numnet)
#define LOG_NUMREC(log)		(__LOG_HDR(log)->numrec)
#define LOG_VALIDIDX(log,idx) ((0 <= (idx)) && ((idx) < LOG_NUMENT (log)))
#define LOG_DATE(log)		(__LOG_HDR(log)->date)



extern void log_init_ (ST_LOG *);
extern void log_close_ (ST_LOG *);
extern int log_read_entry_prev (ST_LOG *, ST_LOGENT *, int, int);

extern int log_read_entry_type(__u8 typ1, int type);
extern int log_read_entry_subtype(__u8 typ1, __u8 typ2, int type2);

extern int log_read_entry_eventtype(__u8 typ1, int type);

extern int log_read_entry_next4 (ST_LOG *, ST_LOGENT *);
extern int log_read_entry_prev4 (ST_LOG *, ST_LOGENT *);





#endif
