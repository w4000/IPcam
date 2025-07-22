#ifndef __LOAD_SETUP_H__
#define __LOAD_SETUP_H__

#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>

#include "setup/setup_encryption.h"

static int fd_lock(int fd)
{
	struct flock lock;

	lock.l_type = F_RDLCK; 
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;

	return fcntl(fd, F_SETLKW, &lock);
}

static int fd_unlock(int fd)
{
	struct flock lock;

	lock.l_type = F_UNLCK; 
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;

	return fcntl(fd, F_SETLK, &lock);
}

static int g_fd = -1;
static void cgi_setup_lock()
{
	g_fd = open(CGI_SETUPFILE_LOCK, O_CREAT|O_RDWR, 644);
	fd_lock(g_fd);
}

static void cgi_setup_unlock()
{
	if(g_fd < 0) {
		return;
	}
	fd_unlock(g_fd);
	close(g_fd);
	g_fd = -1;
}

int  _file_exist(const char *filename)
{
	struct stat st;

	if (lstat(filename, &st) < 0)
		return FALSE;

	return TRUE;
}

int init_setup_isp_notify(_CAMSETUP_EXT *setup_ext)
{
	int i;
	for (i = UPDATE_FCGI_ISP_IQ; i < UPDATE_FCGI_ISP_MAX; i++)
	{
		setup_ext->fcgi_update_notify_isp[i] = 0;
	}


	return 0;
}

int set_setup_isp_notify(_CAMSETUP_EXT *setup_ext, int id)
{
	setup_ext->fcgi_update_notify_isp[id] = 1;
	return 0;
}


int load_setup(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext)
{
#define SETUP_EXT_IMPLEMENTATION
#include "setup/setup_ext.h"
	if(setup_ext != NULL) {
		setup_ext_default(setup_ext);
	}
	return read_enc_setup(setup, setup_ext);
}

int load_setup_lock(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext)
{
	cgi_setup_lock();
	return load_setup(setup, setup_ext);
}

int load_setup_unlock()
{
	cgi_setup_unlock();
	return 0;
}

void update_setup(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext)
{
	write_enc_setup(setup, setup_ext);
}

void update_setup_unlock(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext)
{
	update_setup(setup, setup_ext);
	cgi_setup_unlock();
}


#endif /* __LOAD_SETUP_H__ */
