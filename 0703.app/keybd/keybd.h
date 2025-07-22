/*
 * keybd.h
 *
 * purpose:
 *	keyboard procedure implementation.
 **************************************************************************************/
#ifndef __KEYBD_H
#define __KEYBD_H


/* KEYBD thread's resorces info */
typedef struct _st_keybd
{
	_CAM_THREAD_COMMON	th;

	int						fd;
	
} ST_KEYBD;


extern ST_KEYBD				g_keybd;

#endif
