/* 
 * vod.h
 *
 * Purpose:
 *
 ***********************************************************/
 
#ifndef __VOD_H
#define __VOD_H

#include "../file/playback.h"

#define VOD(svr, cam)		((svr)->vod [cam])

/* Macros to get stream server status.
 *	@svr: pointer to struct st_stream. 
 */



/* struct st_vod contains a vod service related information, and included by
 * struct st_stream that represents a stream service.
 * VOD server does read-ahead frame data, and accumulated frames are linked 
 * by buf_head. Most recently read-aheaded frame's index is included in index_reada.
 */
typedef struct st_vod 
{		
	ST_COMMAND	cmd;		// Client's command to be applied to requested stream.
		
	ST_PLAYOBJ	* playobj;
	
	int 		remote_copy;	// TRUE when remote copy is requested.
	int		prevmode;

	int		transhdr;		// Transfer wsf header to remote client.
} ST_VOD;




extern struct st_vod *init_vod (struct st_vod *);
extern int handle_vod_request (struct st_stream *, ST_PHDR phdr);
extern struct st_frame *init_frame(struct st_frame *, int, int, int, char *);
extern int handle_stream (struct st_stream *strsvr, int sd, struct phdr *ph);
extern int handle_heartbeat(struct st_stream *, int, struct phdr *);
extern int handle_reqdisconn(struct st_stream *, int, struct phdr *);
extern int handle_stop_play(struct st_stream *, int, struct phdr *);
extern int handle_vod_stream (struct st_stream *, int, struct phdr *);
extern void release_frame(struct st_frame *);
extern void release_vod(struct st_vod *);


#endif
