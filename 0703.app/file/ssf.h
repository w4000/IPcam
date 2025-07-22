
#ifndef __SSF_H
#define __SSF_H

#include "pos/poslib.h"

#include "soc_common.h"

#ifdef ARCH_ARM
#define PACK_ATTRIBUTE		__attribute__ ((packed))
#else
#define PACK_ATTRIBUTE
#endif

/*****************************************************************************************
 * SSF file format.
 *****************************************************************************************/
#define SSF_SSF_EXTENSION		".ssf"
#define SSF_IDX_EXTENSION		".idx"
#define SSF_EVT_EXTENSION		".edx"
#define SSF_RDX_EXTENSION		".rdx"

#if (DST == 1)
#define SSF_DST_EXTENSION		"D.ssf"
#endif

#define MAX_IFRMIDXSZ			230400
#define MAX_SSFFILESZ			(1.5 *G)

#define MAX_SSFHDRSZ			(1*K)
#define MAX_IDXFILESZ			(2*M)
#define MAX_FRAMESZ				(63*K)
#define ENCRYPT_LENGTH			80
#define MAX_RDXHDRSZ			(32*K)
#define MAX_FRAGMENTS			5

#define SSF_VIDEO				0
#define SSF_AUDIO				1
#define SSF_POS_DATA			2
#define SSF_LINUDIX_DATA		5
#define SSF_EVENT_DATA			6
#define SSF_THERMAL_DATA		7
#define SSF_LPR_DATA			8
#define SSF_META_DATA			9

#define SSF_ENCKEY				0x12345678	

#define SSFHDR_MINSZ			356			

#define IDXHDR_MINSZ			294			
#define IDXHDR_MINSZ2			10			


#define MAGIC_RDXHDR			0x12345678
#define RDXHDR_MINSZ			240 *16

// Macros to get SSF header.
#define SSFHDR_ID(ghdr)				LE32_TO_HOST((dword )((ghdr)->id))
#define SSFHDR_SIZE(ghdr)			LE32_TO_HOST((dword )((ghdr)->size))

// @fp : ptr to FILEPROP.
#define FILEPROP_YEAR(fp)   		LE16_TO_HOST((fp)->date.sd_year)
#define FILEPROP_MON(fp)   			((byte)((fp)->date.sd_month))
#define FILEPROP_DAY(fp)   			((byte)((fp)->date.sd_day))
#define FILEPROP_HOUR(fp)   		((byte)((fp)->date.sd_hour))
#define FILEPROP_MIN(fp)   			((byte)((fp)->date.sd_min))
#define FILEPROP_SEC(fp)   			((byte)((fp)->date.sd_sec))
#define FILEPROP_DURATION(fp)		LE32_TO_HOST((fp)->duration)
#define FILEPROP_ENCKEY(fp)			LE32_TO_HOST((fp)->enc_key)
#define FILEPROP_ENCMODE(fp)		((byte)((fp)->enc_mode))
#define FILEPROP_CHTITLE_SZ(fp)		((byte)((fp)->title_len))
#define FILEPROP_CHTITLE(fp)		((char *)((fp)->title))

/* @vp : ptr to VIDEOPROP. */
#define VIDEOPROP_XRES(vp)	LE16_TO_HOST((vp)->xres)
#define VIDEOPROP_YRES(vp)	LE16_TO_HOST((vp)->yres)
#define VIDEOPROP_IRATE(vp)		LE16_TO_HOST((vp)->irate)
#define	VIDEOPROP_SPSPPS(vp)	((vp)->var + 1)
#define	VIDEOPROP_SPSPPS_SZ		128


/* @unit : ptr to DATAUNIT. */
#define DATAUNIT_FRAGED_MINSZ			16	
#define DATAUNIT_OFFS_FRAGINFO		9
#define DATAUNIT_TYPE(unit)				(((DATAUNIT *)unit)->flag & 0x0F)
#define DATAUNIT_IS_IFRAME(unit)		(((DATAUNIT *)unit)->flag & 0x80)
#define DATAUNIT_IS_FRAGED(unit)		(((DATAUNIT *)unit)->flag & 0x40)
#define DATAUNIT_IS_JPEG(unit)			(((DATAUNIT *)unit)->flag & 0x40)
#define DATAUNIT_TOTNUM_FRAGED(unit)	(*((char *)unit + DATAUNIT_OFFS_FRAGINFO) >> 4)
#define DATAUNIT_NUM_FRAGED(unit)		(*((char *)unit + DATAUNIT_OFFS_FRAGINFO) & 0x0F)
#define DATAUNIT_TOTSZ_FRAGED(unit)	LE32_TO_HOST(*(dword *)((char *)unit + DATAUNIT_OFFS_FRAGINFO + 1))  
#define DATAUNIT_FRAMESZ(unit)			LE32_TO_HOST(((DATAUNIT *)unit)->size)
#define DATAUNIT_STAMP(unit)			LE32_TO_HOST(((DATAUNIT *)unit)->timecode)

#if __POS__
// PDXINFO
#define PDX_ID(idxp)				LE32_TO_HOST (((PDXINFO *) idxp)->id)
#define PDX_COUNT(idxp)				LE16_TO_HOST (((PDXINFO *) idxp)->count)

#define PDX_OFFSET(idxp, i)	({\
									PDXINFO * info = (PDXINFO *) idxp;	\
									int n = (int) i;	\
									\
									if (n >= info->count)	\
										n = info->count - 1;	\
									if (n < 0)	\
										n = 0;	\
									\
									LE32_TO_HOST (info->index [n].offset);	\
								})

#define PDX_STAMP(idxp, i)	({\
									PDXINFO * info = (PDXINFO *) idxp;	\
									int n = (int) i;	\
									\
									if (n >= info->count)	\
										n = info->count - 1;	\
									if (n < 0)	\
										n = 0;	\
									\
									LE32_TO_HOST (info->index [n].timecode);	\
								})
#endif

/*  @idx : ptr to IFRAMEINFO. */
#define	INDEX_RECINDEX(idxp)	(((IFRAMEINFO *) idxp)->rindex.data)

#define INDEX_ID(idxp)			LE32_TO_HOST (((IFRAMEINFO *) idxp)->id)
#define	INDEX_COUNT(idxp)		LE16_TO_HOST (((IFRAMEINFO *) idxp)->count)

#define INDEX_OFFSET(idxp, i)	({\
									IFRAMEINFO * info = (IFRAMEINFO *) idxp;	\
									int n = (int) i;	\
									\
									if (n >= info->count)	\
										n = info->count - 1;	\
									if (n < 0)	\
										n = 0;	\
									\
									LE32_TO_HOST (info->index [n].offset);	\
								})

#define INDEX_STAMP(idxp, i)	({\
									IFRAMEINFO * info = (IFRAMEINFO *) idxp;	\
									int n = (int) i;	\
									\
									if (n >= info->count)	\
										n = info->count - 1;	\
									if (n < 0)	\
										n = 0;	\
									\
									LE32_TO_HOST (info->index [n].timecode);	\
								})

#define INDEX_INTERVAL(idxp, i)	({\
									IFRAMEINFO * info = (IFRAMEINFO *) idxp;	\
									int n = (int) i;	\
									\
									if (n >= info->count)	\
										n = info->count - 1;	\
									if (n < 0)	\
										n = 0;	\
									\
									LE32_TO_HOST (info->index [n].interval);	\
								})

#define INDEX_SCALE(idxp, i)	({\
									IFRAMEINFO * info = (IFRAMEINFO *) idxp;	\
									int n = (int) i;	\
									\
									if (n >= info->count)	\
										n = info->count - 1;	\
									if (n < 0)	\
										n = 0;	\
									\
									LE32_TO_HOST (info->index [n].scale);	\
								})

#define	RECORD_RECINDEX(rdxp)	(((RECORDINFO *) rdxp)->data)

typedef enum {
	SSFID_GLOBALHDR   = 0xFFFF0000,
	SSFID_FILEPROP    = 0xFFF10000,
	SSFID_VIDEOPROP	  = 0xFFF20000,
	SSFID_AUDIOPROP   = 0xFFF30000,
	SSFID_IFRAMEPROP  = 0xFFF40000,
	SSFID_EVENTPROP   = 0xFFF50000,
	SSFID_DATAHDR     = 0xFFF60000,
	SSFID_IFRAMEINDEX = 0xFFFA0000,
	SSFID_EVENTINDEX  = 0xFFFB0000
} SSFID;

typedef enum {
	ENCRYPT_NONE     = 0x0000,
	ENCRYPT_XOR      = 0x0001,
	ENCRYPT_DWORDXOR = 0x0002,
	ENCRYPT_BLOWFISH = 0x0004,
	ENCRPYT_PWDWOCRC = 0x0008,
	ENCRYPT_SIMPLEBF = 0x0010,
	ENCRYPT_STDBF8   = 0x0011,
	ENCRYPT_STDBF16  = 0x0012,
	ENCRYPT_STDBF32  = 0x0013,
	ENCRYPT_STDBF160 = 0x0014,
	ENCRPYT_CRCVALID = 0x0080
} ENCRYPT_MODE;

typedef enum {
	SECURITY_ADMIN    = 0xA0,
	SECURITY_SERVER   = 0xA1,
	SECURITY_BACKUP   = 0xA2,
	SECURITY_OPERATOR = 0xA3,
	SECURITY_USER     = 0xA4
} SECURITY_MODE;

// *** Notice : Many structure's size will be variable          ***
// *** If you need more information, see specification document ***



// Global Header for .SSF file
#pragma pack(1)

typedef struct __SSFIDENT {
	char ident [32];
} PACK_ATTRIBUTE SSFIDENT;


typedef struct __SSFHEADER {
	dword  id;      // pre-defined header id = 0xFFFF0000
	dword  size;    // Total property objects size (see above)
} PACK_ATTRIBUTE SSFHEADER;

// Date information for saved stream
typedef struct __STREAMDATE {
	short  sd_gmt;   // Time bias to GMT standard time
	word   sd_year;  // Year
	byte   sd_month; // Month
	byte   sd_day;   // Day
	byte   sd_hour;  // Hour
	byte   sd_min;   // Minute
	byte   sd_sec;   // Second
	byte   sd_info;  // Bit 7: Daylight saving flag, 6-4: Reserved, 3-0: Day of week
	short  sd_dst;   // Time bias of Daylight saving
} PACK_ATTRIBUTE STREAMDATE;

// Dynamic parameters; normally user setting values
typedef struct __DYNPARAM {
	byte   quality;      // Video or Audio Quality
	byte   sens;         // Motion sensitivity
	word   mblock_size;  // mblock size (bytes)
	byte  *mblock;       // Motion check block bits
	byte   cblock_size;  // cblock size (bytes)
	byte  *cblock;       // Change extension data
} PACK_ATTRIBUTE DYNPARAM;

// File properties when set property
typedef struct __FILEPROP {
	dword      id;         // pre-defined unique id = 0xFFF10000
	dword      size;       // structure size
	byte       version;    // version number: (ex.) "1.5" -> 15, "2.0" -> 20
	char       site[8];     // site id
	STREAMDATE date;       // file creation date
	byte       enc_mode;   // encrypt mode, normally used none or blowfish (see ENCRYPT_MODE)
	dword      enc_key;    // encrypt key (randomly generated)
	dword      pwd_crc;    // CRC of the user password
	dword      duration;   // total stream duration to playback
	byte       pair;       // stream pair count (1: audio or video only, 2: audio and video)
	byte       security;   // security grade (see SECURITY_MODE)
	word       lang;       // language id (see langdef.h)
	byte       channel;    // the encoded channel number
	byte       title_len;  // title[] length with NULL terminator
	/*------------------ Variable length fields ----------------------------
	 *	string : Channel titile.
	 *	byte : Description length.
	 *	string : Description.
	 */
	char	   title[32];
	char       var[0];
} PACK_ATTRIBUTE FILEPROP;


typedef struct _GUID {
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[8];
} PACK_ATTRIBUTE GUID;


// Video properties
typedef struct __VIDEOPROP {
	dword     id;     // pre-defined unique id = 0xFFF20000
	dword     size;   // structure size
	GUID      clsid;  // video decoder CLSID (see file head)
	byte      seq;    // sequence number of video stream
	byte      type;   // stream type: 0 - none available
	                  // (bitwise)    1 - Interlaced frame
	                  //              2 - included B frame
	word      xres;   // Video X resolution
	word      yres;   // Videp Y resolution
	byte      irate;  // I-frame rate
	/*------------------ Variable length fields ----------------------------
	 *	DYNPARAM : Dynamic parameters.
	 *	byte : Extension data length.
	 *	string : Extension data.
	 */
	char	var[0];
} PACK_ATTRIBUTE VIDEOPROP;



// I-Frame index properties
typedef struct __IFRAMEPROP {
	dword  id;     // pre-defined unique id = 0xFFF40000
	dword  size;   // structure size
	dword  offset; // offset to index object
	byte   xsize;  // xdata size
	/*------------------ Variable length fields ----------------------------
	 *	string : Extension data.
	 */
	char	var[0];
} PACK_ATTRIBUTE IFRAMEPROP;

// Event index properties
typedef struct __EVENTPROP {
	dword  id;     // pre-defined unique id = 0xFFF50000
	dword  size;   // structure size
	dword  offset; // offset to index object
	byte   xsize;  // xdata size
	/*------------------ Variable length fields ----------------------------
	 *	string : Extension data.
	 */
	char	var[0];
} PACK_ATTRIBUTE EVENTPROP;

// Stream data
typedef struct __DATAUNIT {
	dword  	size;  	// structure size with stream data size
	byte      	flag;  	// stream flag (bit)
                     		// 7   : I-Frame
	                     // 6   : Fragment flag
	                     // 5   : Even/Odd flag (It will be set when Interaced Frame bit was set in type element of VIDEOPROP)
	                     // 4   : reserved
	                     // 3-0 : stream number (0-15)
	dword     timecode;  // time stamp in millisecond
	/*------------------ Variable length fields ----------------------------
	 *	byte  :    fragment information (bit)
	 *	dword :    Total size of stream, if it is not fragmented
	 *	DYNPARAM : Dynamic parameters. it is only occurred on the first fragmented stream.
	 *	byte : Extension data size.
	 *	byte[] : Extended data.
	 *	byte[] : Bitstream data.
	 */
	char	var[0];
} PACK_ATTRIBUTE DATAUNIT;

// Stream data entry
typedef struct __DATAHDR {
	dword  id;    // pre-defined unique id = 0xFFF60000
	dword  size;  // total data size = sizeof(DATAHDR) + (sizeof(DATAUNIT) * count_of_DATAUNIT)
	char	dataunit[0];
} PACK_ATTRIBUTE DATAHDR;

// I-Frame index
typedef struct __IFRAMEINDEX {
	dword  offset;   // offset to I-Frame
	dword  timecode; // time stamp to I-Frame
	dword  scale;
	dword  interval;
} PACK_ATTRIBUTE IFRAMEINDEX;

// I-Frame index for Backup
typedef struct __IFRAMEINDEX_BKUP {
	dword  offset;   // offset to I-Frame
	dword  timecode; // time stamp to I-Frame
} PACK_ATTRIBUTE IFRAMEINDEX_BKUP;

// information to Record index
typedef struct __RECORDINDEX {
	struct tm	createtm;
	__uint8		data [240];		// DM_TBLSZ = 24 * (60/ DM_RESOL), DM_RESOL = 6
} PACK_ATTRIBUTE RECORDINDEX;

/*
// information to Record info
typedef struct __RECORDINFO {
	__uint8		data [RDXHDR_MINSZ];		// DM_TBLSZ = 24 * (60/ DM_RESOL), DM_RESOL = 6
} PACK_ATTRIBUTE RECORDINFO;
*/
// information to Record info
typedef struct __RECORDINFO {
	int			magic;
	int			size;
	__u8		data [RDXHDR_MINSZ];		// DM_TBLSZ = 24 * (60/ DM_RESOL), DM_RESOL = 1
	__u8		min_data [RDXHDR_MINSZ *6];		// DM_TBLSZ = 24 * (60/ DM_RESOL), DM_RESOL = 1
} PACK_ATTRIBUTE RECORDINFO;

// information to I-Frame index
typedef struct __IFRAMEINFO {
	dword	id;       // pre-defined unique id = 0xFFFA0001
	dword	size;     // structure size = sizeof(IFRAMEINFO) + (sizeof(IFRAMEINDEX) * (count_of_index - 1))
	word	count;    // count of index
	RECORDINDEX	rindex;	  // record index table
	IFRAMEINDEX	index [0];
} PACK_ATTRIBUTE IFRAMEINFO;


// information to I-Frame index
typedef struct __IFRAMEINFO2 {
	dword	id;       // pre-defined unique id = 0xFFFA0000
	dword	size;     // structure size = sizeof(IFRAMEINFO2) + (sizeof(IFRAMEINDEX) * (count_of_index - 1))
	word	count;    // count of index
	IFRAMEINDEX	index [0];
} PACK_ATTRIBUTE IFRAMEINFO2;


// Event index
typedef struct __EVENINDEX {
	word   size;     // structure size
	dword  offset;   // offset to first frame
	dword  timecode; // time stamp to first frame
	byte   type;     // Event type
	                 // Type: 0 - Unknown
	                 //       1 - Sensor On
	                 //       2 - Sensor Off
	byte   num_sen;  // Number of sensors
	byte  *sen;      // sensors
	byte   num_out;  // Number of output
	byte  *out;      // outputs
	byte   xsize;    // xdata[] size
	byte  *xdata;    // extended data
} PACK_ATTRIBUTE EVENINDEX;

// information to Event index
typedef struct __EVENTINFO{
	dword     id;       // pre-defined unique id = 0xFFFB0000
	dword     size;     // structure size = sizeof(EVENTINFO) + (sizeof(EVENTINDEX) * (count_of_index - 1))
	word      count;    // count of index
} PACK_ATTRIBUTE EVENTINFO;
#pragma pack()


/*****************************************************************************************
 * SSF and index file creation struct.
 *****************************************************************************************/
#define SSF_BUILD_CH(ch)			(& g_file.ssf_build [g_file.ssf_index][(int)(ch)])
//#define SSFBUILD_CH(ch)				(& g_file.ssfbuild[(int)(ch)])
#define IDXBUF_IFRAMEINFO(build)	((IFRAMEINFO *) (build)->idxhdrs)
//#define SSF_RECORDING(ch)			(SSF_FD(SSF_BUILD_CH(ch)) >= 0 && IDX_FD(SSF_BUILD_CH(ch)) >= 0)
//#define SSF_RECORDING(ch)			(SSF_BUILD_CH(ch)(ch)->cam >= 0 && SSF_FD(SSFBUILD_CH(ch)) >= 0 && IDX_FD(SSFBUILD_CH(ch)) >= 0)
/*
#define SSF_RECORDING(ch)	\
				(	\
					(SSF_BUILD_CH(ch)->ssf >= 0) && \
					(SSF_BUILD_CH(ch)->idx >= 0)	\
				)
*/

typedef struct {
	uint64			filesz;

	boolean			intra_net;
	boolean			intra_ssf;

	__u64			savetm;				// saved last fetch time by encoder

	unsigned		prealrm_frms;

} ST_CHANNEL_INFO;


/* ST_SSFBUILD is used to create ssf/idx file.   */
typedef struct st_ssfbuild {
	pthread_mutex_t	mutex;			/* Do not add unit while closing ssf. */

	int 			cam;				// Recording cam cahnnel
	int				date, time;			// Date & time of recording file.

	int				fd_ssf_open_first;
	int				fd_idx_open_first;

	int				fd_ssf;
	//FILE*			fp_ssf;
	char  			ssfname [1<<8];	 // Full path name of wsf file.
	char			ssfhdrs [MAX_SSFHDRSZ];
	size_t			ssfhdrsz;

	/* SSF property headers pointing mmapped area.	*/
	FILEPROP		*filep;
	VIDEOPROP		*videop;
	IFRAMEPROP		*indexp;
	DATAHDR			*dataobj;

	int				fd_ssf_pos;
	int				fd_idx_pos;

	int				fd_idx;
	//FILE*			fp_idx;
	char 			idxname [1<<8];	// Full path name of idx file.
	char 			idxhdrs [MAX_SSFHDRSZ];
	size_t			idxhdrsz;

	/* IDX header pointing idxmap area. */
	IFRAMEINFO		*iframep;

#if __POS__
	int 			pdx;					// for POS DEVICE
	char 			pdxname[1<<8];			// Full path name of pdx file.
	char 			pdxhdrs [MAX_SSFHDRSZ];
	size_t			pdxhdrsz;

	PDXINFO 		*pdxinfop;
#endif

	/* For blowfish encryption. */
	BLOWFISH_CTX	ctx;


	/* Start time of ssf encoding */
	struct tm 		creatime;
	__u64			start;


	int				numalarm;	/* Number of alarm events occurred during recording. */
	int 			recmode;	/* Recording mode (0: continue, 1: sensor, 2: motion) */

	ST_CHANNEL_INFO	chInfo;

} ST_SSFBUILD;


/* ST_RDXBUILD is used to record index file.
 */
typedef struct st_rdxbuild {
	pthread_mutex_t	mutex;	/* Do not add unit while closing ssf. */

	/* Record index header */
	int 		fd_rdx;
	char 		rdxname [1<<8];	// Full path name of rdx file.
	char 		rdxhdrs [MAX_RDXHDRSZ];
	size_t 		rdxhdrsz;

	/* RDX header pointing rdxmap area. */
	RECORDINFO	*rdxp;

} ST_RDXBUILD;

/* Temporary store to build dataunit from raw frame data. */
#pragma pack(1)
typedef struct st_encunit {

	struct clist	list;

	int				ch;
	int				sz;				// Size of data in buf[]
	uint64_t		tm;				// The time when unit fetched.
	int				seq;			// I or P frame.
	int				type;			// Audio or video.

	int				save;			// Save in mass storage device or not.

	int				scale;			// set scale for decoder
	int 			interval;		// set interval for decoder

		/***************  The order of below fields are important. ********************/
	char			ssfhdr [64];		// Dataunit header. //
	char			buf [0];			// Raw frame data. //

} PACK_ATTRIBUTE ST_ENCUNIT;
#pragma pack()


#define SSFBUILD_CH(ch)				(& g_file.ssf_build [g_file.ssf_index][(int) (ch)])
#define SSF_FD(build)				((build)->fd_ssf)
#define IDX_FD(build)				((build)->fd_idx)
#if __POS__
#define PDX_FD(build)				((build)->pdx)
#endif
#define IDXBUF_IFRAMEINFO(build)	((IFRAMEINFO *) (build)->idxhdrs)
#define SSF_RECORDING(ch)			(SSFBUILD_CH(ch)->cam >= 0 && SSF_FD(SSFBUILD_CH(ch)) >= 0 && IDX_FD(SSFBUILD_CH(ch)) >= 0)

#if __POS__
#define SSFBUILD_INIT \
		{\
		.mutex = PTHREAD_MUTEX_INITIALIZER,\
		.cam = -1,\
		.date = 0,\
		.time = 0,\
		.fd_ssf_pos = 0,\
		.fd_idx_pos = 0,\
		.fd_ssf_open_first = 0,\
		.fd_idx_open_first = 0,\
		.fd_ssf = -1,\
		.ssfname = "",\
		.ssfhdrsz = 0,\
		.fd_idx = -1,\
		.idxname = "",\
		.idxhdrsz = 0,\
		.pdx = -1,\
		.pdxname = "",\
		.pdxhdrsz = 0\
		}
#else
#define SSFBUILD_INIT \
		{\
		.mutex = PTHREAD_MUTEX_INITIALIZER,\
		.cam = -1,\
		.date = 0,\
		.time = 0,\
		.fd_ssf_pos = 0,\
		.fd_idx_pos = 0,\
		.fd_ssf_open_first = 0,\
		.fd_idx_open_first = 0,\
		.fd_ssf = -1,\
		.ssfname = "",\
		.ssfhdrsz = 0,\
		.fd_idx = -1,\
		.idxname = "",\
		.idxhdrsz = 0,\
		}
#endif

#define RDXBUILD_INIT \
		{\
		.mutex = PTHREAD_MUTEX_INITIALIZER,\
		.fd_rdx = -1,\
		.rdxname = "",\
		}

// @build : Ptr to ST_SSFBUILD.
#define ssfbuild_init(build) \
	do {\
		ST_SSFBUILD __tmp__ = SSFBUILD_INIT;	\
		memcpy(build, &__tmp__, sizeof(ST_SSFBUILD));\
	} while (0)

// @build : Ptr to ST_RDXBUILD.
#define rdxbuild_init(build) \
	do {\
		ST_RDXBUILD __tmp__ = RDXBUILD_INIT;	\
		memcpy(build, &__tmp__, sizeof(ST_RDXBUILD));\
	} while (0)


// Flush the idx header.
#define SSFBUILD_FLUSH_IDXHDR_2(build) \
do {\
	lseek ((build)->fd_idx, 0, SEEK_SET);\
	write ((build)->fd_idx, (build)->idxhdrs, (build)->idxhdrsz);\
} while (0)


// Flush the idx header.
#define SSFBUILD_FLUSH_SSFHDR_2(build) \
do {\
	lseek ((build)->fd_ssf, 0, SEEK_SET);\
	write ((build)->fd_ssf, (build)->ssfhdrs, (build)->ssfhdrsz);\
} while (0)

#define SSFBUILD_FLUSH_SSFHDR_1(build) \
do {\
	off_t __pos__;\
	__pos__ = lseek ((build)->fd_ssf, 0, SEEK_CUR);\
	lseek ((build)->fd_ssf, 0, SEEK_SET);\
	write ((build)->fd_ssf, (build)->ssfhdrs, (build)->ssfhdrsz);\
	lseek ((build)->fd_ssf, __pos__, SEEK_SET);\
} while (0)

#if __POS__
// Flush the pdx header.
#define SSFBUILD_FLUSH_PDXHDR(build) \
do {\
	off_t __pos__;\
	__pos__ = lseek ((build)->pdx, 0, SEEK_CUR);\
	lseek ((build)->pdx, 0, SEEK_SET);\
	write ((build)->pdx, (build)->pdxhdrs, (build)->pdxhdrsz);\
	lseek ((build)->pdx, __pos__, SEEK_SET);\
} while (0)
#endif

// Flush the rdx header.
#define SSFBUILD_FLUSH_RDXHDR(build) \
do {\
	lseek ((build)->fd_rdx, 0, SEEK_SET);\
	write ((build)->fd_rdx, (build)->rdxhdrs, (build)->rdxhdrsz);\
} while (0)

struct st_encunit;

extern 	const GUID	h265_clsid;
extern 	const GUID	h264_clsid;
extern 	const GUID	jpeg_clsid;


extern void init_frame_buf();
extern int ssf_addlist (ST_SSFBUILD *, struct clist *);
extern int ssf_build(ST_SSFBUILD *, const char *, struct tm *, unsigned, PAYLOAD_TYPE_E enType, unsigned, unsigned, unsigned, unsigned);
extern int ssf_hdr_build (ST_SSFBUILD *build, struct tm *creatime, unsigned ch, PAYLOAD_TYPE_E enType, unsigned xres, unsigned yres, unsigned aud_samp_rate, unsigned aud_bits_rate);
extern int ssf_set_propobj(ST_SSFBUILD *);
extern int ssf_idx_build(ST_SSFBUILD *, ST_SSFBUILD *, unsigned, PAYLOAD_TYPE_E enType, unsigned, unsigned, unsigned, unsigned, struct timeval *);
extern int ssf_addunit(ST_SSFBUILD *build, ST_ENCUNIT *unit, VENC_STREAM_S *pstStream);
extern void ssfbuild_close(ST_SSFBUILD *);

extern int rdx_build(ST_RDXBUILD *build, const char *rdxfilename);
extern void rdxbuild_close(ST_RDXBUILD *);

extern char *get_propobj(char *, int, dword);

extern struct st_frame* _get_frame_buffer(int size);
extern void _free_frame_buffer(struct st_frame* p);

#endif
