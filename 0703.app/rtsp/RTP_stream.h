
#ifndef		___STREAM_H
#define		___STREAM_H

#include "RTSP_common.h"

typedef	struct st_NALU
{
	struct clist	u_list;				 

	ST_FRAME*		st_frame_ref;		 
	int				last;
	int				u_index;
	int				u_IsRtcp;
	int				u_plsz;				 
	__u8			*u_start;			 
	int				u_left;				 

	__u8			u_rtsp [4];	
	__u8			u_rtp [12];	 
	__u8			u_fu [2];	 
	__u8			u_p [0];		 

}__attribute__((packed)) ST_NALU;


typedef	struct st_NALU_H265
{
	struct clist	u_list;				 

	ST_FRAME*		st_frame_ref;		 
	int				last;
	int				u_index;
	int				u_IsRtcp;
	int				u_plsz;				 
	__u8			*u_start;			 
	int				u_left;			

	__u8			u_rtsp [4];  
	__u8			u_rtp [12];	 
	__u8			u_fu [3];	 
	__u8			u_p [0];		 

}__attribute__((packed)) ST_NALU_H265;

typedef	struct st_NALU_JPEG
{
	struct clist	u_list;				
	ST_FRAME*		st_frame_ref;	
	int				last;

	int				u_index;
	int				u_IsRtcp;

	int				u_plsz;			

	__u8			*u_start;		
	int				u_left;			

	__u8			u_rtsp [4];
	__u8			u_rtp [12];	
	__u8			u_fu [140];
	__u8			u_p [0];	

}__attribute__((packed)) ST_NALU_JPEG;

#if 1
static inline char *__find_startcode (char *p, char *e)
{
    char *a = p + 4 - ((intptr_t) p & 3);

    for (e -= 3; p < a && p < e; p++) {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1)
            return p;
    }

    for (e -= 3; p < e; p += 4) {
        __u32 x = *(__u32 *) p;
        if ((x - 0x01010101) & (~x) & 0x80808080) { 
            if (p[1] == 0) {
                if (p[0] == 0 && p[2] == 1)
                    return p;
                if (p[2] == 0 && p[3] == 1)
                    return p+1;
            }
            if (p[3] == 0) {
                if (p[2] == 0 && p[4] == 1)
                    return p+2;
                if (p[4] == 0 && p[5] == 1)
                    return p+3;
            }
        }
    }

    for (e += 3; p < e; p++) {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1)
            return p;
    }

    return e + 3;
}


static inline char *find_startcode (char *p, char *e)
{
	char	*o= __find_startcode (p, e);
	if (p < o && o < e && ! o[-1]) 
		o--;
	
	return o;
}

static inline char *find_startcode_skip (char *p, char *e, int skip)
{
	int i = 0;
	char *o = p;
	if(skip == 0) skip = 1;
	for(i = 0; i < skip; i++) {
		if(i != 0) while(!*(o++));
		o = find_startcode(o, e);
	}
	
	return o;
}

#endif



#endif

