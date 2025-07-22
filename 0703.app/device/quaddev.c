#include "common.h"


static unsigned quad_update_vidin (ST_QUADDEV *quad) 
{
	__u16 vid_in = 1;

	// TODO
	vid_in = 0x03; 
	
	return vid_in;

}


static unsigned quad_getvsig (ST_QUADDEV *quad) 
{
	unsigned int v = 0;

	v = 0;
	
	return 0;
}


// Update current video input state.
// 	Return mask of channels that status has changed.
static unsigned __quad_update_vidin (ST_QUADDEV *quad) 
{
	unsigned i, newstat, changed = 0;	
	
	pthread_mutex_lock (& quad->mutex);
	
	newstat = quad_update_vidin (quad);	
	
	if (-1 == (int) newstat)  // Error during detection.
		newstat = quad->vidin;
		
	//debug ("camin new stat=%x, %x\n", newstat, quad->vidin);
	
	for (i = 0; i < CAM_MAX; i++) {			
		if (bitisset (quad->vidin, i) && (! bitisset (newstat, i))) {
			bitset (changed, i);
			continue;			
		}
		
		if ((! bitisset (quad->vidin, i)) &&  bitisset (newstat, i)) {
			bitset (changed, i);
			continue;
		}
	}
	
	quad->vidin = newstat;
	debug("VIDEOIN (0x%02x)\n", quad->vidin);
	
	pthread_mutex_unlock (& quad->mutex);

	return changed;	
}


static void __quad_release (ST_QUADDEV *quad) 
{	
	pthread_mutex_destroy (& quad->mutex);
}

int quad_init (ST_QUADDEV *quad) 
{	
	pthread_mutex_init (& quad->mutex, NULL);
	
	quad->vsig = quad_getvsig (quad);		
	
	quad->vidin = quad_update_vidin (quad);	
	quad->motion = 0;

	quad->update_vidin = __quad_update_vidin;
	quad->release = __quad_release;

	return 0;
}
 
