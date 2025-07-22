/*
 * quaddev.h
 *
 *   Purpose:
 *		- Provide accessing routines of quad mux device.
 *
 *******************************************************************************/

#ifndef __QUADDEV_H
#define __QUADDEV_H


typedef struct st_quaddev {
	
	pthread_mutex_t	mutex;
	
	unsigned	vsig;						// NTSC/PAL	
	unsigned	vidin;						// Current input video channel mask.	
	
	__u16		motion;						// Current motion status of channels.
	
	unsigned (* update_vidin) (struct st_quaddev *); // Update current video loss state.
	void (* release) (struct st_quaddev *);

} ST_QUADDEV;


extern int quad_init (ST_QUADDEV *);

#endif	// __QUADDEV_H
