#include "common.h"
#include "file/ssfsearch.h"

#define __USE_POLL_FOR_CODEC__

int check_ssfhdr (void * hdr, int hdrsz) {

	assert (hdrsz >= SSFHDR_MINSZ);
	if (SSFHDR_ID ((SSFHEADER *)(hdr + sizeof (SSFIDENT))) != SSFID_GLOBALHDR) {
	 	return -ERR_SSFHDR;
	}

	if (get_propobj(hdr, hdrsz, SSFID_FILEPROP) == NULL)
		return -ERR_SSFHDR;

	if (get_propobj(hdr, hdrsz, SSFID_VIDEOPROP) == NULL)
		return -ERR_SSFHDR;

	if (get_propobj(hdr, hdrsz, SSFID_IFRAMEPROP) == NULL)
		return -ERR_SSFHDR;

	if (get_propobj(hdr, hdrsz, SSFID_DATAHDR) == NULL)
		return -ERR_SSFHDR;

	return (sizeof (SSFIDENT) + sizeof (SSFHEADER) + SSFHDR_SIZE ((SSFHEADER *)(hdr + sizeof (SSFIDENT))));
}

#ifdef _RTSP_VOD_
	#define MAX_PLAYOBJ_POOL	(52 + 32 * 2)
#else
	#define MAX_PLAYOBJ_POOL	(52)
#endif

static ST_PLAYOBJ 	g_playobj_pool[MAX_PLAYOBJ_POOL];
static char			g_playobj_inuse[MAX_PLAYOBJ_POOL] = { 0, };

static ST_PLAYOBJ* get_playobj_pool()
{
	int i;
	for (i = 0; i < MAX_PLAYOBJ_POOL; i++)
	{
		if (!g_playobj_inuse[i])
		{
			g_playobj_inuse[i] = 1;
			return &(g_playobj_pool[i]);
		}
	}

	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof (ST_PLAYOBJ));
	ST_PLAYOBJ* p = _mem_malloc2 (sizeof (ST_PLAYOBJ));
	printf("NEW PLAY_OBJ POOL : %p\n", p);
	return p;
}

static void free_playobj_pool(ST_PLAYOBJ* msg)
{
	int i;
	for (i = 0; i < MAX_PLAYOBJ_POOL; i++)
	{
		if (&(g_playobj_pool[i]) == msg)
		{
			g_playobj_inuse[i] = 0;
			return;
		}
	}

	printf("FREE PLAY_OBJ POOL : %p\n", msg);
	_mem_free(msg);
}
ST_PLAYOBJ  * playobj_init () {
	ST_PLAYOBJ  *playobj = NULL;

	if ((playobj = get_playobj_pool())==NULL)
		return NULL;

	playobj->magic = PLAYOBJ_MAGIC;
	playobj->ssf = -1;
	playobj->ssf_fp = NULL;
	playobj->fptr = 0;
	playobj->mmap = (char *)-1;
	playobj->mmapend = NULL;
	playobj->mmapptr = NULL;
	playobj->idx = -1;
	playobj->idxmap = (char *)-1;
	playobj->idxend = NULL;
	playobj->idxstart = 0;
	playobj->index = 0;
	playobj->offset = 0;
	playobj->filep = NULL;		
	playobj->videop = NULL;
	playobj->indexp = NULL;
	playobj->dataobj = NULL;
	init_clist(&playobj->buf_head);
	playobj->playbuf_numframe = 0;
	playobj->creatime_sec = 0;
	playobj->playedtm = 0;
#if (DST == 1)
	playobj->ssf_dst = FALSE;
#endif

	return playobj;
}

void __playobj_release(ST_PLAYOBJ *playobj) {

	if (!playobj) {
		return;
	}

	if (playobj->mmap != (char *)-1) {
		msync (playobj->mmap, playobj->mmapend - playobj->mmap, MS_INVALIDATE | MS_SYNC);
		munmap (playobj->mmap, playobj->mmapend - playobj->mmap);
		playobj->mmap = (char *)-1;
		playobj->mmapend = NULL;
	}

	if (playobj->ssf_fp) {

		fclose (playobj->ssf_fp); 

		playobj->ssf_fp = NULL;
		playobj->ssf = -1;
	}


	if (playobj->idxmap != (char *)-1) {
		msync (playobj->idxmap, playobj->idxend - playobj->idxmap, MS_INVALIDATE | MS_SYNC);
		munmap (playobj->idxmap, playobj->idxend - playobj->idxmap);
		playobj->idxmap = (char *)-1;
		playobj->idxend = NULL;
	}

	if (playobj->idx >= 0) {
		if (close(playobj->idx) < 0)
			perror("close");
		playobj->idx = -1;
	}


	playobj->ssf_dst = FALSE;

	playback_frame_flush(playobj);
}

/* Release playback object.
 */
void playobj_release(ST_PLAYOBJ *playobj) {

	if (NULL == playobj)
		return;

	__playobj_release(playobj);
	free_playobj_pool(playobj);
}




