/*************************************************************************
 * auth.h
 *
 * Purpose:
 *	- Defines data structure for authorization server.
 ***************************************************************************/
 
#ifndef __AUTH_H
#define __AUTH_H

/* 
 * Authorization server speciif data. 
 */
typedef struct st_auth {
	/* No data for authoriztion server. */
} ST_AUTH;


extern int handle_auth_request (struct st_stream *, ST_PHDR phdr);
extern void release_auth(struct st_auth *);

#endif
