
#include "common.h"
#include "file/ssfsearch.h"
#include "file/log.h"
#include "file/hdd.h"


#ifdef _DEBUG_OUT
static struct timeval 		g__tm_val;
static __u64				g__sTime[10], g__eTime[10];

#endif

ST_FILE		g_file;

#define _DIRECT_WRITE_

#ifndef _DIRECT_WRITE_
extern char g_write_buf[MAX_WRITE_BUF_CNT][MAX_WRITE_BUF_SZ];
extern int  g_write_buf_used[MAX_WRITE_BUF_CNT];
extern int  g_write_buf_stack;
#endif


static int file_init (ST_FILE *file) {
	int i, j;

	assert (file);


#ifndef _DIRECT_WRITE_
	memset(g_write_buf, 0, sizeof(g_write_buf));
	memset(g_write_buf_used, 0, sizeof(g_write_buf_used));
#endif

	file->th.state = STAT_UNREADY;
	init_msg_queue(& file->th.msgq, FILEMAN);


	for (i = 0; i < 1; i ++)
		hddinfo_init (& file->hddinfo [i], i);

	file->min_ssf_buf_size = MIN_SSF_BUF_SIZE_SD;

	file->nr_hdd = _detect_physical (file->hddinfo);
	if (file->nr_hdd < 1)
	{
		file->min_ssf_buf_size = MIN_SSF_BUF_SIZE_USB;

		file->nr_hdd = usb_detect_physical (file->hddinfo);
	}

	file->nr_odd = 0;					
	file->nr_mount = 0;				

	file->totsz = 0;					
	file->usedsz = 0;					

	file->hdd = 0;					
	memset(file->fmt_hdd , 0, sizeof(file->fmt_hdd));

	for (j = 0; j < 2; j++)
	{
		foreach_cam_ch (i)
			ssfbuild_init (& file->ssf_build [j][i]);
		end_foreach

		rdxbuild_init (& file->rdx_build[j]);
	}

	file->ssf_index = 0;
	file->rdx_index = 0;

	file->first_date = 0;
	file->first_time = 0;


	file->ssf_buf_idx = 0;
	memset(file->ssf_buf_pos, 0, sizeof(file->ssf_buf_pos));
	memset(file->ssf_buf_pos_flush, 0, sizeof(file->ssf_buf_pos_flush));
	memset(file->ssf_buf, 0, sizeof(file->ssf_buf));

	file->idx_buf_idx = 0;
	memset(file->idx_buf_pos, 0, sizeof(file->idx_buf_pos));
	memset(file->idx_buf, 0, sizeof(file->idx_buf));

	pthread_mutex_init (& file->ssf_write_mutex, NULL);
	pthread_mutex_init (& file->ssf_buf_mutex, NULL);
	pthread_mutex_init (& file->idx_buf_mutex, NULL);



	file->th.state = STAT_READY;
	return 0;
}


void close_all_files()
{
	int i, j;

	for (j = 0; j < 2; j++)
	{
		rdxbuild_close (& g_file.rdx_build [j]);

		foreach_cam_ch (i)
			ssfbuild_close(& g_file.ssf_build [j][i]);
		end_foreach
	}

}

static void file_release(ST_FILE *file)
{
	assert (file);

	file->th.state = STAT_TERMINATED;
	flush_msg_queue(& file->th.msgq);

	close_all_files();


	pthread_mutex_destroy(& g_file.ssf_write_mutex);
	pthread_mutex_destroy(& g_file.ssf_buf_mutex);
	pthread_mutex_destroy(& g_file.idx_buf_mutex);

}



boolean __isrecordingssf (char * path, int ch)
{
	return FALSE;
}

boolean isrecordingssf (char * path)
{
	int ch;

	foreach_cam_ch (ch)
		if (__isrecordingssf (path, ch))
			return TRUE;
	end_foreach

	return FALSE;
}


static int handle_msg_del_oldest_ssf (struct st_message * msg)
{
	char deldir[1<<8]="", delfile[1<<8]="";
	char firstdir[1<<8] = "";

 	int date, time;

	assert (msg);

	if (! IPCAM_THREAD_RUNNING (FILEMAN))
		return 0;
	
	if (!HDD_RW_ENABLE)
		return 0;

	{
		if (find_ssfdir (deldir, & date, 0, SEEK_EXACT|SEEK_FORWARD) == 0) {

			int ch;

			for(ch = 0; ch < CAM_MAX; ch ++) {
				char idxfile[1<<8]="", * ptr;

				if (find_ssf (delfile, & time, date, 0, ch, 0, SEEK_EXACT | SEEK_FORWARD) < 0) {
					continue;
				}

				if (isrecordingssf(delfile)) {

					if ((find_ssf2 (delfile, NULL, NULL, date, time, ch, 0, SEEK_FORWARD)) < 0)
						continue;
				}

				remove_file (delfile, 1);

				strcpy (idxfile, delfile);

				if ((ptr = strstr (idxfile, SSF_SSF_EXTENSION))) {
					strcpy (ptr, SSF_IDX_EXTENSION);
					remove_file (idxfile, 1);
				}
			}

			{
				int	fileset [MAX_SSF], ret;
				char logfile [64];
				char rdxfile [64];

				if ((ret = get_filelist (deldir, fileset, NULL, NULL, -1)) == 0) {
					remove_file (deldir, 1);

					// LOGFILE_GET_NAME (logfile, date);
					remove_file (logfile, 1);

					RDXFILE_GET_NAME (rdxfile, date);
					// remove_file (rdxfile, 1);
				}
			}
		}


		if (find_ssfdir (firstdir, & date, 0, SEEK_FORWARD) == 0) {
			if (find_ssf (firstdir, & time, date, 0, -1, 0, SEEK_EXACT | SEEK_FORWARD) == 0) {
				g_file.first_date = date;
				g_file.first_time = time;
			}
		}
	}




	return 0;
}




#ifndef _DIRECT_WRITE_

static int handle_msg_flush_file(struct st_message *msg) {
	int fd;
	char* hdrs;
	int hdrssz;

	if (!msg)
		return -1;

	if (msg->parm1 < 0 || msg->parm2 == NULL || msg->parm3 == NULL)
		return -1;

	fd = msg->parm1;
	hdrs = (char*)msg->parm2;
	hdrssz = msg->parm3;

	off_t __pos__;
	__pos__ = lseek (fd, 0, SEEK_CUR);
	lseek (fd, 0, SEEK_SET);
	write (fd, hdrs, hdrssz);
	lseek (fd, __pos__, SEEK_SET);

	_mem_free(hdrs);

	return 0;
}

static int handle_msg_close_file(struct st_message *msg) {
	unsigned filesz;
	int fd;
	char* fname;

	if (!msg)
		return -1;

	if (msg->parm1 < 0 || msg->parm2 == NULL)
		return -1;

	fd = msg->parm1;
	fname = (char*)msg->parm2;

	filesz = getfilesz(fd);
	fsync (fd);
	close (fd);
	fd = -1;

	if (filesz <= SSFHDR_MINSZ) {
		remove_file (fname);
	}


	_mem_free(fname);

	return 0;
}
#endif

inline int write_buf_to_file()
{
	int ret_io = 0;
	int bIsWrite = 0;

	static int err_cnt = 0;

	if (g_file.ssf_build [g_file.ssf_index][0].fd_ssf_pos > 0)
	{
		int lastIdx = g_file.ssf_buf_idx;
		int ssf_buf_size = g_file.ssf_buf_pos_flush[lastIdx];

		pthread_mutex_lock(&g_file.ssf_buf_mutex);
		if (ssf_buf_size > 0)
			g_file.ssf_buf_idx = (g_file.ssf_buf_idx + 1) % 2;
		pthread_mutex_unlock(&g_file.ssf_buf_mutex);

		if (ssf_buf_size > 0)
		{
			pthread_mutex_lock(&g_file.ssf_write_mutex);
			if (g_file.ssf_build [g_file.ssf_index][0].fd_ssf >= 0)
			{

				ret_io = write(g_file.ssf_build [g_file.ssf_index][0].fd_ssf, g_file.ssf_buf[lastIdx], g_file.ssf_buf_pos_flush[lastIdx]);
				if (ret_io < 0) {

					if (errno == ENOSPC) {
						err_cnt++;
						send_msg (ENCODER, FILEMAN, _CAMMSG_DEL_OLDEST_SSF, 0, 0, 0);
					}
					else if (errno == EIO){
						err_cnt++;
					}
				}
				else {
					ret_io = fsync(g_file.ssf_build [g_file.ssf_index][0].fd_ssf);
					if (ret_io < 0) {
						err_cnt++;
						if (errno == EIO) {
						}
					}
				}

			}
			g_file.ssf_buf_pos[lastIdx] = g_file.ssf_buf_pos_flush[lastIdx] = 0;
			pthread_mutex_unlock(&g_file.ssf_write_mutex);
		}
		else
		{
			//sleep_ms(10);
		}

		bIsWrite = 1;
	}

	if (g_file.ssf_build [g_file.ssf_index][0].fd_idx_pos > 0)
	{
		int lastIdx = g_file.idx_buf_idx;
		int idx_buf_size = g_file.idx_buf_pos[lastIdx];

		pthread_mutex_lock(&g_file.idx_buf_mutex);
		if (idx_buf_size > 0)
			g_file.idx_buf_idx = (g_file.idx_buf_idx + 1) % 2;
		pthread_mutex_unlock(&g_file.idx_buf_mutex);

		if (idx_buf_size > 0)
		{
			pthread_mutex_lock(&g_file.ssf_write_mutex);
			if (g_file.ssf_build [g_file.ssf_index][0].fd_idx >= 0)
			{

				write(g_file.ssf_build [g_file.ssf_index][0].fd_idx, g_file.idx_buf[lastIdx], g_file.idx_buf_pos[lastIdx]);

			}
			g_file.idx_buf_pos[lastIdx] = 0;	
			pthread_mutex_unlock(&g_file.ssf_write_mutex);
		}
		else
		{
		}

		bIsWrite = 1;
	}

	if(err_cnt >= 10) {
		send_msg (FILEMAN, MAIN, _CAMMSG_REBOOT, 5, 0, 0);
	}

	return bIsWrite;
}


void *fileproc (void *arg)
{
	static int _test_fd = -1;

	if (file_init (& g_file) < 0) {
		goto out;
	}

	while(1) {
		struct st_message msg;

		if (get_msg (MSGQ (FILEMAN), & msg) > 0) {
			g_main.thread_work_check[THREAD_ALIVE_CHECK_FILE] = msg.msgid;



			switch (msg.msgid) {

			case _CAMMSG_START:
				sleep(2);//w4000_sleep(2);
				g_file.th.state = STAT_RUNNING;


				break;

			case _CAMMSG_STOP:
				g_file.th.state = STAT_READY;
				break;


			case _CAMMSG_DEL_OLDEST_SSF:
				handle_msg_del_oldest_ssf (& msg);
				break;


#ifndef _DIRECT_WRITE_

			case _CAMMSG_FLUSH_FILE:
				handle_msg_flush_file (&msg);
				break;

			case _CAMMSG_CLOSE_FILE:
				handle_msg_close_file(& msg);
				break;
#endif


			case _CAMMSG_EXIT:
				goto out;

			default :
				break;
			}
		}

		if (g_main.isNetUpgrade)
		{
			//w4000 sleep_ms(100);
		}
		else
		{
			if (!write_buf_to_file())
			{
				//w4000 sleep_ms (100);
			}
		}

		sleep_ex (10);

		g_main.thread_alive_check[THREAD_ALIVE_CHECK_FILE] = 0;



	}
out:

	if (_test_fd >= 0)
		close(_test_fd);
	_test_fd = -1;

	file_release(&g_file);
	pthread_exit(NULL);
}

