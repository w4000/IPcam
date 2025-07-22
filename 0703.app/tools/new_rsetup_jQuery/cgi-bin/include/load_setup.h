
#ifndef LOAD_SETUP_H_
#define LOAD_SETUP_H_

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

int load_setup(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext)
{
#ifdef USE_SETUP_ENCRYPTION
	if(setup_ext != NULL) {
		setup_ext_default(setup_ext);
	}
	return read_enc_setup(setup, setup_ext);
#else
	FILE *fp = NULL, *fp_ext = NULL;
	int num = 0;
	int bIsBackupFile = 0, bIsBackupExtFile = 0;

#if defined(USE_ENCRYPT_CMD_LINE)
	const char* setup_file = DEC_TMP_SETUPFILE;
	const char* setup_ext_file = DEC_TMP_SETUPFILE_EXT;
#else
	const char* setup_file = CGI_SETUPFILE;
	const char* setup_ext_file = CGI_SETUPFILE_EXT;
#endif	/* USE_ENCRYPT_CMD_LINE */

retry:
	fp = fopen(setup_file, "rb");

	if (NULL == fp) {
		if (bIsBackupFile)
		{
			cgi_setup_unlock();
			perror ("fopen");
			return  -1;
		}
		else
		{
			__copyfile(CGI_SETUPFILE_BAK, CGI_SETUPFILE);
#if defined(USE_ENCRYPT_CMD_LINE)
			decrypt_file(setup_file, CGI_SETUPFILE);
#endif	/* USE_ENCRYPT_CMD_LINE */
			bIsBackupFile = 1;
			goto retry;
		}
	}

	num = fread((void *)setup , 1, sizeof(_CAMSETUP), fp);
	if (num != sizeof(_CAMSETUP)) {
		if (bIsBackupFile)
		{
			perror ("fread");
			printf("fread error (%d-%d)<br>", num, sizeof(_CAMSETUP));
			goto err;
		}
		else
		{
			fclose(fp);
			__copyfile(CGI_SETUPFILE_BAK, CGI_SETUPFILE);
			bIsBackupFile = 1;
			goto retry;
		}
	}

retry_ext:

	fp_ext = fopen(setup_ext_file, "rb");
	if (!fp_ext)
	{
		if (!bIsBackupExtFile)
		{
			__copyfile(CGI_SETUPFILE_EXT_BAK, CGI_SETUPFILE_EXT);
#if defined(USE_ENCRYPT_CMD_LINE)
			decrypt_file(setup_ext_file, CGI_SETUPFILE_EXT);
#endif	/* USE_ENCRYPT_CMD_LINE */
			bIsBackupExtFile = 1;
			goto retry_ext;
		}
	}

	if (setup_ext && fp_ext)
	{
		setup_ext_default(setup_ext);

		int bValidCheck = 0;
		int magic2[4], size2 = 0;
		memset(magic2, 0, sizeof(magic2));
		fread(magic2, 1, sizeof(magic2), fp_ext);
		if (magic2[0] == MAGIC_NUM_1 && magic2[1] == 0x00710225 && magic2[2] == 0x00951220 && magic2[3] == 0x00981029) {
			fread(&size2, 1, sizeof(int), fp_ext);
			if (size2 > 0) {
				int sz2, len2 = min(size2, sizeof(_CAMSETUP_EXT)) - sizeof(magic2) - sizeof(int);
				if (len2 > 0)
				{
					sz2 = fread(&(setup_ext->reserved), 1, len2, fp_ext);
					if (sz2 == len2) {
						memcpy(setup_ext->magic, magic2, sizeof(magic2));
						setup_ext->sizeOfStruct = size2;

						bValidCheck = 1;
					}
				}
			}
		}

		if (!bValidCheck)
		{
			if (!bIsBackupExtFile)
			{
				fclose(fp_ext);
				__copyfile(CGI_SETUPFILE_EXT_BAK, CGI_SETUPFILE_EXT);
				bIsBackupExtFile = 1;
				goto retry_ext;
			}
		}
	}

err:

	if (fp)
		fclose(fp);
	if (fp_ext)
		fclose(fp_ext);

	return num;
#endif
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
#ifdef USE_SETUP_ENCRYPTION
	write_enc_setup(setup, setup_ext);
#else
	FILE *fp = NULL, *fp_ext = NULL;
	int num;

#if defined(USE_ENCRYPT_CMD_LINE)
	const char* setup_bak_file = DEC_TMP_SETUPFILE;
	const char* setup_ext_bak_file = DEC_TMP_SETUPFILE_EXT;
#else
	const char* setup_bak_file = CGI_SETUPFILE_BAK;
	const char* setup_ext_bak_file = CGI_SETUPFILE_EXT_BAK;
#endif	/* USE_ENCRYPT_CMD_LINE */

	if (_file_exist(setup_bak_file))
		fp = fopen(setup_bak_file,"r+");
	else
		fp = fopen(setup_bak_file,"wb");

	if (NULL == fp) {
		perror ("fopen");
		printf("fopen error<br>");
		goto End;
	}

	if (fseek(fp, 0, SEEK_SET) == 0) {
		perror ("fseek");
	}

	num = fwrite(setup , 1, sizeof(_CAMSETUP), fp);
	if (num != sizeof(_CAMSETUP)) {
		perror ("fwrite");
		printf("fwrite error<br>");
		fclose(fp);
		goto End;
	}

	fclose(fp);

	__copyfile(CGI_SETUPFILE_BAK, CGI_SETUPFILE);


	if(setup_ext == NULL) {
		goto End;
	}

	if (_file_exist(setup_ext_bak_file))
		fp_ext = fopen(setup_ext_bak_file, "r+b");
	else
		fp_ext = fopen(setup_ext_bak_file, "wb");

	if (fp_ext)
	{
		setup_ext->magic[0] = MAGIC_NUM_1;
		setup_ext->magic[1] = 0x00710225;
		setup_ext->magic[2] = 0x00951220;
		setup_ext->magic[3] = 0x00981029;
		setup_ext->sizeOfStruct = sizeof(_CAMSETUP_EXT);

		fseek(fp_ext, 0L, SEEK_SET);
		num = fwrite(setup_ext , 1, sizeof(_CAMSETUP_EXT), fp_ext);
		if (num != sizeof(_CAMSETUP_EXT)) {
			perror ("fwrite ext");
			printf("fwrite ext error<br>");
			fclose(fp_ext);
			goto End;
		}

		fclose(fp_ext);

		__copyfile(CGI_SETUPFILE_EXT_BAK, CGI_SETUPFILE_EXT);
	}

End:
	return;
#endif
}

void update_setup_unlock(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext)
{
	update_setup(setup, setup_ext);
	cgi_setup_unlock();
}

#define INI_IMPLEMENTATION
#include "ini.h"

int get_ubenv(char *key)
{
	return 0;
}

void set_ubenv(char *key, int val)
{
	return;
}
#endif /* LOAD_SETUP_H_ */
