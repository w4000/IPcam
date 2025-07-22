#ifndef __MSERVER_H
#define __MSERVER_H

//--------------------------- Stream serving thread info ---------------------------------
// Each stream server is created whenever a stream request has arrived. 
struct st_mstream {
	boolean	inuse;					// Is this struct being used?
			
	pthread_t id;					// Thread ID. used to attach data socket to the stream server.			
	volatile int sock;				// Remote communication socket.
		
	struct st_cliinfo	*cliinfo;	
	
	int flag [CAM_MAX];			// property of each camera

	struct st_live live [CAM_MAX];	// Live service properties

	int terminate;				// Should we terminate connection ?

} __attribute__((__packed__));

struct _NETSVR;
extern void * msvr_thread (void *);
extern void flush_mcliinfo(struct _NETSVR *);
extern struct st_mstream *__creat_mstreamsvr (int, int);
extern void __release_mstream(struct st_mstream *);
extern void release_mstream(struct _NETSVR *, struct st_mstream *);

#endif
