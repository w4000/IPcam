#ifndef __PLAYBACK_H
#define __PLAYBACK_H

#include "lib/threadsafe.h"
#include "net/net_protocol.h"
#include "file/ssf.h"

#include "soc_common.h"

 typedef struct st_command {
	int	random;

	int	timestamp;	 
	int	playmode;		 
	int	playspeed;
} ST_COMMAND;


enum PLAYMODE {
	PLAYMODE_STOP,				 
	PLAYMODE_PLAY,				 
	PLAYMODE_PLAY_SOTRE	= 100,
};


typedef struct st_request {
	int date;	 
	int time; 
	char ch; 
	int playmode;
	int playspeed;
#if (DST == 1)
	int dst;
#endif
} ST_REQUEST;


#define PLAYOBJ_MAGIC	0x11115678

typedef struct st_playobj {
	int magic;

	struct st_request req;		

	char ssfname [1<<8];		
	char idxname [1<<8];		


	int		ssf;			
	FILE 	* ssf_fp;			
	off_t 	fptr;				
	char 	*mmap;				
	char 	*mmapend;			
	char 	*mmapptr;			

	int 	idx;				
	char 	*idxmap;			
	char 	*idxend;

	volatile int idxstart;	
	volatile int index;	
	volatile int offset;		


	char ssfhdr [MAX_SSFHDRSZ];

	FILEPROP	*filep;		
	VIDEOPROP	*videop;
	IFRAMEPROP	*indexp;
	DATAHDR		*dataobj;

	struct clist	buf_head;
	int			playbuf_numframe;		

	time_t	creatime_sec;		
	__u64	playedtm;			

	BLOWFISH_CTX	ctx;	

#if (DST == 1)
	boolean		ssf_dst;
#endif

} ST_PLAYOBJ;

#pragma pack(1)
typedef struct st_frame {
	struct	clist 	iframe;	

	__u64			stamp;			

	int				type;	

	int				refcnt;	
	int 			idxgop;			
	int				index;			

	int				hdrsz;
	int				rawsz;			
	int				bufsz;			

	int 			decrypt;
	int				save;		

	int 			scale;	
	int 			interval;		

	struct	phdr 	phdr;

#ifdef ARCH_ARM
	char	ch;
#else
	struct 	psendimg psendimg;
#endif

	char	data_unit[0];	 
} PACK_ATTRIBUTE ST_FRAME;


#pragma pack()

#define frame_init(buf, clock, typ, nr_gop, nr_idx, hdsz, size, bufsz, sv, sc, intv)	\
	({\
		assert ((buf));\
		init_clist(& ((buf)->iframe));\
		(buf)->stamp = clock;		\
		(buf)->type = typ;	\
		(buf)->refcnt = 0;		\
		(buf)->idxgop = nr_gop;\
		(buf)->index = nr_idx;	\
		(buf)->hdrsz = hdsz;	\
		(buf)->rawsz = size;	\
		(buf)->bufsz = bufsz;	\
		(buf)->decrypt = 0;		\
		(buf)->save = sv;		\
		(buf)->scale = sc;	\
		(buf)->interval = intv;	\
		(buf);	\
	})


#define __frame_release(buf) \
	({\
		assert ((buf));\
		if ((buf)) {\
			del_clist (& (buf)->iframe);\
			_free_frame_buffer(buf);\
		} else {\
		}\
	})

#define playback_frame_flush(playobj) \
	({\
		struct st_frame * buf = NULL;\
		assert (playobj);\
		while(! clist_empty (& (playobj)->buf_head)) {\
			buf = clist_entry ((playobj)->buf_head.next, struct st_frame, iframe);\
			__frame_release(buf);	\
		}\
		(playobj)->playbuf_numframe = 0;	\
	})


extern ST_PLAYOBJ  *playobj_init();
extern void __playobj_release(ST_PLAYOBJ *);
extern void playobj_release(ST_PLAYOBJ *);
extern int check_ssfhdr(void *, int);


#endif



