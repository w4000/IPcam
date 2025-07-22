#include "common.h"
#include "file/log.h"


#if 1
#include "../tools/new_rsetup_jQuery/cgi-bin/jQuery_language.h"
#else 
#endif 


void log_init_ (ST_LOG * log) 
{
	ST_LOG tmp = LOG_INIT;
	
	assert (log);
	
	* log = tmp;	
}

void log_close_ (ST_LOG *log) 
{
	assert (log);

	if (log->mmap != (char *)-1) {
		msync (log->mmap, log->mmapend - log->mmap, MS_INVALIDATE | MS_SYNC);
		munmap(log->mmap, log->mmapend - log->mmap);
	}
				
	if (log->fd >= 0) {
		close (log->fd);
		log->fd = -1;
	}
	
}



static int __log_read_entry (ST_LOG *log, ST_LOGENT *buf, int idx) {
	
	assert (log);
	assert (buf);	
	
	if (! LOG_VALIDIDX(log,idx))
		return -EINVAL;
	
	lseek (log->fd, sizeof (ST_LOGHDR) + idx * sizeof (ST_LOGENT), SEEK_SET);
	
	if (read (log->fd, buf, sizeof (ST_LOGENT)) != sizeof (ST_LOGENT)) {
		perror ("read");
		return -1;
	}

	buf->info = LE32_TO_HOST (buf->info);
	buf->tm.tm_sec = LE32_TO_HOST (buf->tm.tm_sec);
	buf->tm.tm_min = LE32_TO_HOST (buf->tm.tm_min);
	buf->tm.tm_hour = LE32_TO_HOST (buf->tm.tm_hour);
	buf->tm.tm_mday = LE32_TO_HOST (buf->tm.tm_mday);
	buf->tm.tm_mon = LE32_TO_HOST (buf->tm.tm_mon);
	buf->tm.tm_year = LE32_TO_HOST (buf->tm.tm_year);
	buf->tm.tm_wday = LE32_TO_HOST (buf->tm.tm_wday);
	buf->tm.tm_yday = LE32_TO_HOST (buf->tm.tm_yday);
	buf->tm.tm_isdst = LE32_TO_HOST (buf->tm.tm_isdst);	

	return 0;
}	



int log_read_entry_prev (ST_LOG *log, ST_LOGENT *buf, int type, int type2) {
}

int log_read_entry_type(__u8 typ1, int type)
{
	if ((typ1 == LOGTYPE_SYSTEM) && (type & LOGBIT_SYSTEM))
		return 0;

	if ((typ1 == LOGTYPE_EVNT) && (type & LOGBIT_EVNT))
		return 0;

	if ((typ1 == LOGTYPE_SETUP) && (type & LOGBIT_SETUP))
		return 0;

	if ((typ1 == LOGTYPE_NET) && (type & LOGBIT_NETWORK))
		return 0;

	if ((typ1 == LOGTYPE_REC) && (type & LOGBIT_REC))	
		return 0;
	
	return -1;
}

int log_read_entry_eventtype(__u8 typ1, int type)
{
	if ((typ1 == LOGTYPE_EVNT_SENSOR) && (type & LOGBIT_EVNT_SENSOR))
		return 0;

	if ((typ1 == LOGTYPE_EVNT_MOT) && (type & LOGBIT_EVNT_MOTION))
		return 0;

	if ((typ1 == LOGTYPE_EVNT_VIDLOSS) && (type & LOGBIT_EVNT_VIDLOSS))
		return 0;

	if ((typ1 == LOGTYPE_EVNT_DISKFULL) && (type & LOGBIT_EVNT_DISKFULL))
		return 0;	

	return -1;
}

int log_read_entry_subtype(__u8 typ1, __u8 typ2, int type2)
{
	if (type2 & (1 << (typ2 - typ1 -1)))
		return 0;
	
	return -1;
}



int log_read_entry_next4 (ST_LOG *log, ST_LOGENT * buf) {
}

int log_read_entry_prev4 (ST_LOG *log, ST_LOGENT * buf) {
}

