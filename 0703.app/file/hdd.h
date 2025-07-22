/*
 * hdd.h
 *
 * prupose :
 *		HDD management routines.
 *		Detecting, mounting, formatting.
 * 
 * 
 **********************************************************************/

#ifndef __HDD_H
#define __HDD_H

#define TYPE_HDD			0x0001
#define TYPE_USBDIRECT		0x0002		// USB direct access device like USB HDD/STICK.
#define TYPE_CDROM			0x0004
#define TYPE_SDCARD			0x0008		// SD CARD

#define FDISK_TOOL		"fdisk"
#define TUNE2FS_TOOL	"tune2fs"

#if (FS == 0)			
#define MKFS_TOOL		"mkfs.ext3"
#define FSCK_TOOL		"e2fsck"
#elif (FS == 1)
#define MKFS_TOOL		"mkdosfs"
#define FSCK_TOOL		"dosfsck"
#endif


// HDD critical temperature for checking fan
#define TEMPER_CHECK_FAN		65


enum FMT_STAGE {
	FMTSTAGE_UMOUNT,
	FMTSTAGE_MKFS,
	FMTSTAGE_CHECKDISK,
	FMTSTAGE_MAKESWAP,
	FMTSTAGE_DONE,
	FMTSTAGE_ERR,					// Error ouccured during backup progress.
	FMTSTAGE_CANCEL,				// Format Cancel	Add : Steve Jang.
};

enum FSCK_STAGE {
	FSCKSTAGE_UMOUNT,
	FSCKSTAGE_FSCK,
	FSCKSTAGE_DONE,
	FSCKSTAGE_ERR,					// Error ouccured during progress.
};

typedef struct __format {
		
	void * info;					// Formatting disk info.
	int count;						// format disk count (for multi-format)
	
	int stage;						// Current stage.
	int progress;					// Progress percentage in current stage. 0~100.

	boolean fmat_process;				// Current Format State...

} ST_FORMAT;


typedef struct __fsck {
		
	void * info;					// Checkking disk info.
	
	int stage;						// Current stage.
	int progress;					// Progress percentage in current stage. 0~100.
	
} ST_FSCK;


extern int g_fsck;
extern int hdd_umount (ST_HDDINFO  []);
extern int __hdd_detect_physical (ST_HDDINFO *, int);
extern int _detect_physical (ST_HDDINFO hddinfo [1]);
extern int usb_detect_physical (ST_HDDINFO hddinfo [1]);

#endif
