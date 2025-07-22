/*
 * file.h
 *
 * purpose:
 *	- Maintain HDD's freespace.
 ************************************************************************************/
#ifndef __FILE_H
#define __FILE_H

#include "log.h"

#define HDD_CHECK_INTERVAL_SEC	30	//5

#define HDD_FREE_THRESHOLD_1ST	120000		 	
#define HDD_FREE_THRESHOLD		100000		 	
#define HDD_KBYES_SIZE(blks, blksz)	(blks * (blksz/K)) 

#define HDD_TOTKBSZ				(IPCAM_THREAD_FILE->totsz)
#define HDD_USEDKBSZ			(IPCAM_THREAD_FILE->usedsz)
#define HDD_RW_ENABLE			(IPCAM_THREAD_FILE->nr_mount)

#define KILOBYTE_FROM_BYTES(x)	(((double) (x)) / K)
#define MEGABYTE_FROM_BYTES(x)	(((double) (x)) / M)
#define GIGABYTE_FROM_BYTES(x)	(((double) (x)) / G)
#define GIGABYTE_FROM_KBYTE(x)	((double) ((double) (x) / (double) (1 << 20)))

#define KILOBYTE2_FROM_BYTES(x)	(((double) (x)) / (K / (double)10))
#define MEGABYTE2_FROM_BYTES(x)	(((double) (x)) / (M / (double)10))
#define GIGABYTE2_FROM_BYTES(x)	(((double) (x)) / (G / (double)10))
#define GIGABYTE2_FROM_KBYTE(x)	((double) ((double) (x) / ((double) (1 << 20) / (double)10)))

typedef struct _hddinfo 
{	
	int		id;				
	boolean	detect;			
	boolean	mount;			
	boolean	rw;				

	char	swapname[32];	
	char  	devname[32]; 
	char	mpoint [32];
	
	char  	cmd_fdisk[64]; 
	char		cmd_fsck[64];	
	char  	cmd_mkfs[64]; 
	char 	cmd_tune[64];	
	char  	cmd_temper[64]; 
	char  	cmd_health[64];
	char  	cmd_smarton[64];
	
	char		model [32];		// Model name.
	uint64	blocks;			
	uint64	bfree;			
	uint64	bsize;		
	
	uint64	totkb;		
	uint64	freekb;		

	int		health;		
	int		temper;	

} ST_HDDINFO;

typedef ST_HDDINFO ST_ODDINFO;

#define HDDINFO_INIT \
{\
	.id = 0,\
	.detect = FALSE,\
	.mount = FALSE,\
	.rw = FALSE,\
	.swapname ="",\
	.devname = "",\
	.mpoint = "",\
	.cmd_fdisk = "",\
	.cmd_fsck = "",\
	.cmd_mkfs = "",\
	.cmd_tune = "",\
	.cmd_temper = "",\
	.cmd_health = "",\
	.cmd_smarton = "",\
	.model = "",\
	.blocks = 0,\
	.bfree = 0,\
	.bsize = 0,\
	.totkb = 0,\
	.freekb = 0,\
	.health = 0,\
	.temper = 0\
}

#define hddinfo_init(info,n) \
	do {\
		ST_HDDINFO __tmp__ = HDDINFO_INIT;	\
		* (info) = __tmp__;\
		(info)->id = (n);\
	} while (0)

#define oddinfo_init(info,n) hddinfo_init(info,n)

#define MAX_SSF_BUF_SIZE		(102400*10)
#define MIN_SSF_BUF_SIZE_SD		(102400*2)		
#define MIN_SSF_BUF_SIZE_USB	(102400/2)

#define MAX_IDX_BUF_SIZE	(256)

typedef struct _FILE {
	
	_CAM_THREAD_COMMON	th;
	
	ST_HDDINFO		hddinfo [1];		
	
	int				nr_hdd;					
	int				nr_odd;					
	int				nr_mount;				
	
	uint64			totsz;					
	uint64			usedsz;					
	
	int				hdd;					

	int				fmt_hdd [1];
	ST_SSFBUILD		ssf_build [2][3];	
	int				ssf_index;
	
	ST_RDXBUILD 	rdx_build[2];			
	int				rdx_index;
	
	int				first_date;
	int				first_time;


	pthread_mutex_t	ssf_write_mutex;

	pthread_mutex_t	ssf_buf_mutex;
	int				ssf_buf_idx;
	int				ssf_buf_pos[2];
	int				ssf_buf_pos_flush[2];
	char			ssf_buf[2][MAX_SSF_BUF_SIZE];

	pthread_mutex_t	idx_buf_mutex;
	int				idx_buf_idx;
	int				idx_buf_pos[2];
	char			idx_buf[2][MAX_IDX_BUF_SIZE];

	unsigned int	min_ssf_buf_size;

	ST_LOG			evtlog;					

	/*------------------------------------------------------*/
	int				check_usb_undetect;

} ST_FILE;

extern ST_FILE g_file;
extern void *fileproc (void *arg);
extern int request_del_file(int, char *);
extern boolean __isrecordingssf (char *, int);
extern boolean isrecordingssf (char *);
extern void close_all_files();
#endif
