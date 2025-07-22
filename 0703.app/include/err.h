/*
 * err.h
 *
 *   Purpose:
 *	- Define system global error codes.
 *	
 **************************************************************/
#ifndef __ERR_H
#define __ERR_H


enum ERR_CODE {
	ERR = 1,			
	ERR_MEM,		// Memory allocation error.
	ERR_NOSPC,		// No block device space left.
	ERR_MUTEX,		// Mutex deadlock error.
	ERR_DEV,		// Device access error.
	ERR_IO,			// File IO error.
	ERR_TIMEOUT,	// Timeout occurred.
	ERR_TIMEVAL,	// Invalid time value.
	ERR_SOCK,		// Socket IO error.
	ERR_DISCONN,	// Socket disconnected.
	ERR_MAXCONN,	// Network clients connection is already full.
	ERR_THREAD,		// Thread management related error.
	ERR_FRAME,		// Bad frame data.
	ERR_SSFHDR,		// Bad ssf header.
	ERR_IDXHDR,		// Bad idx header.
	ERR_PDXHDR,		// Bad pdx header.
	ERR_BADIDX,		// Index data of ssf file has broken.
	ERR_EOFRAME,	// Reached end of frame data in ssf file.
	ERR_NOFILE,		// File does not exist.
	ERR_NODIR,		// Directory does not exist.	
	ERR_NOCAM,		// Camera does not exist of a recording channel.
	ERR_NOREC,		// Camera is not in recording status.
	ERR_WAIT,		// Current frame is waiting for Multi-Channel time sync.
	ERR_CURREC		// File is currently being recorded.
};



#endif
