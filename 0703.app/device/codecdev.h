/*
 * codecdev.h
 *
 *   Purpose:
 *	- Provide accessing routines of video/audio codec device.
 *
 *******************************************************************************/


#ifndef __CODECDEV_H
#define __CODECDEV_H




#define MAX_FRAMEBUFSZ	(MAX_FRAMESZ * 2)

typedef struct st_codecdev {

	pthread_mutex_t mutex;	// Only one thread can access device at a time.

	unsigned	recmask;	// Channel masks being encoded by this device.
	unsigned	dispmask;	// Display-on channels mask.

	unsigned	numframe;	// Number of frame since chip start.
	boolean		rec;		// TRUE if chip is recording start state.
	boolean		dec;		// TRUE if chip is decoding start state

} ST_CODECDEV;

// Prevent encoding & decoding commands mess-up.
#define CODEC_LOCK(dev)		\
	do {\
		pthread_mutex_lock (& (dev)->mutex);\
	} while (0)

#define CODEC_UNLOCK(dev)	\
	do {\
		pthread_mutex_unlock (& (dev)->mutex);\
	} while (0)


#define FRAMEBUF_IS_VIDEO(fbuf)	(((fbuf)->flag & 0x000F) == 2)
#define FRAMEBUF_IS_AUDIO(fbuf)	(((fbuf)->flag & 0x000F) == 3)
#define FRAMEBUF_CH(fbuf)		(((fbuf)->flag & 0x00F0) >> 4)
#define FRAMEBUF_IS_SKIP(fbuf)	((fbuf)->flag & 0x0100)
#define FRAMEBUF_IS_LAST(fbuf)	((fbuf)->flag & 0x0800)
#define FRAMEBUF_IS_I(fbuf)		((((fbuf)->flag >> 12) & 0x0003) == 1)
#define FRAMEBUF_IS_P(fbuf)		((((fbuf)->flag >> 12) & 0x0003) == 2)
#define FRAMEBUF_IS_B(fbuf)		((((fbuf)->flag >> 12) & 0x0003) == 3)

/****************************************************************************
*  mpeg4 codec in solo6010
****************************************************************************/

#define PICTURE_START_KEY		0xffff

/* crypt macro */
#define CRYPT_POLY				0
#define WATERMARK_POLY		0

// scale
#define SCALE_D1				9
#define SCALE_2CIF				1
#define SCALE_CIF				2


#define G723_MULTI_COUNT		16
#define INTRA_HDRSZ				30
#define FRAME_HDRSZ			64

// buffer size
//#define ENC_BUFSZ				(1 << 20)
#define ENC_BUFSZ				(622080)		//(720*567*1.5)

enum {
	FRM_STATUS_0 = 0,
	FRM_STATUS_1,
	FRM_STATUS_2,
	FRM_STATUS_3,
	FRM_STATUS_4,
	FRM_STATUS_5,
	FRM_STATUS_6,
	FRM_STATUS_7,
	FRM_STATUS_8,
	FRM_STATUS_9,
	FRM_STATUS_10,
	FRM_STATUS_11,

	FRM_STATUS_MAX,
};

/* mpeg4 encode control */
typedef struct _st_mp4_enc_ctrl
{
	int channel;
	int scale;
	int qp;
	int gop_size;
	int interval;
} ST_MP4_ENC_CTRL;


/* codec data header of solo6010 */
typedef struct _st_frame_hdr
{
	unsigned long	status[FRM_STATUS_MAX];

	unsigned int	dummy[4];
} ST_FRAME_HDR;


// get frame header
#define GET_FRM_DATSZ(fbuf)			((fbuf)->status[FRM_STATUS_10])
#define GET_FRM_CH(fbuf)			((fbuf)->status[FRM_STATUS_9] & 0x1f)
#define GET_IS_IFRM(fbuf)			((((fbuf)->status[FRM_STATUS_0] >> 22) & 0x03) == 0 ? 1 : 0)
#define GET_FRM_SCALE(fbuf)			(((fbuf)->status[FRM_STATUS_1] >> 28) & 0x0f)
#define GET_FRM_INTV(fbuf)			(((fbuf)->status[FRM_STATUS_4] >> 20) & 0x03ff)

#define GET_FRM_VSZ(fbuf)			(((fbuf)->status[FRM_STATUS_1] & 0xff) * 16)
#define GET_FRM_HSZ(fbuf)			((((fbuf)->status[FRM_STATUS_1] >> 8) & 0xff) * 16)

#define GET_FRM_SEC(fbuf)			((fbuf)->status[FRM_STATUS_5])
#define GET_FRM_USEC(fbuf)			((fbuf)->status[FRM_STATUS_6])


#endif	// __CODECDEV_H
