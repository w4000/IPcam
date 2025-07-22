#include "common.h"

#include <unistd.h>

ssize_t pread(int fd, void *buf, size_t count, off_t offset);
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);


static int __flash_read_fs (ST_FLASH *dev, void *data, void *data_ext)  {

	_CAMSETUP *setup = (_CAMSETUP*)data;
	_CAMSETUP_EXT *setup_ext = (_CAMSETUP_EXT*)data_ext;
#ifdef USE_SETUP_ENCRYPTION
	return read_enc_setup(setup, setup_ext);
#else
	FILE *fp = NULL, *fp_ext = NULL;
	int sz = -1;
	int bIsBackupFile = 0, bIsBackupExtFile = 0;
	
#if defined(USE_ENCRYPT_CMD_LINE)
	
	int ret = -1;
	const char* setup_file = DEC_TMP_SETUPFILE;
	const char* setup_ext_file = DEC_TMP_SETUPFILE_EXT;
#else
	
	const char* setup_file = CGI_SETUPFILE;
	const char* setup_ext_file = CGI_SETUPFILE_EXT;
#endif	/* USE_ENCRYPT_CMD_LINE */

retry:
#if defined(USE_ENCRYPT_CMD_LINE)
	
	ret = decrypt_file(setup_file, CGI_SETUPFILE);
	if(ret < 0) {
		unlink(setup_file);
	}
	else {
	}
#endif	/* USE_ENCRYPT_CMD_LINE */
	
	fp = fopen(setup_file, "rb");
	
	if (!fp)
	{
		
		
		if (bIsBackupFile)
		{
			
			perror ("fopen");
			printf("fopen error (1): %s\n", setup_file);
			return -1;
		}
		else
		{
			
			__copyfile(CGI_SETUPFILE_BAK, CGI_SETUPFILE);
			
			bIsBackupFile = 1;
			goto retry;
		}
	}
	

	sz = fread((void *)setup ,1 ,sizeof(_CAMSETUP), fp);
	
	if (sz != sizeof(_CAMSETUP))
	{
		
		
		if (bIsBackupFile)
		{
			
			perror ("fread");
			printf("fread error (2) (%d : %d)\n", sz, sizeof(_CAMSETUP));
			sz = -1;
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
#if defined(USE_ENCRYPT_CMD_LINE)
	ret = decrypt_file(setup_ext_file, CGI_SETUPFILE_EXT);
	if(ret < 0) {
		unlink(setup_ext_file);
	}
	else {
	}
#endif	/* USE_ENCRYPT_CMD_LINE */

	fp_ext = fopen(setup_ext_file, "rb");
	if (!fp_ext)
	{
		if (!bIsBackupExtFile)
		{
			__copyfile(CGI_SETUPFILE_EXT_BAK, CGI_SETUPFILE_EXT);
			bIsBackupExtFile = 1;
			goto retry_ext;
		}
	}
	else
	{
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

	return sz;
#endif
}


static int __flash_write_fs (ST_FLASH *dev , void *data, void *data_ext)  {

	_CAMSETUP *setup = (_CAMSETUP*)data;
	_CAMSETUP_EXT *setup_ext = (_CAMSETUP_EXT*)data_ext;
#ifdef USE_SETUP_ENCRYPTION
	return write_enc_setup(setup, setup_ext);
#else
	FILE *fp = NULL, *fp_ext = NULL;
	int sz = -1;

#if defined(USE_ENCRYPT_CMD_LINE)
	const char* setup_bak_file = DEC_TMP_SETUPFILE;
	const char* setup_ext_bak_file = DEC_TMP_SETUPFILE_EXT;
#else
	const char* setup_bak_file = CGI_SETUPFILE_BAK;
	const char* setup_ext_bak_file = CGI_SETUPFILE_EXT_BAK;
#endif	/* USE_ENCRYPT_CMD_LINE */
	fp = fopen(setup_bak_file, "wb");

	if (!fp) {
		perror ("fopen");
		return -1;
	}

	sz = fwrite((void *)setup , 1, sizeof(_CAMSETUP), fp);
	if (sz != sizeof(_CAMSETUP)) {
		perror ("fwrite");
		sz = -1;
		goto err;
	}

	if (file_exist(setup_ext_bak_file))
		fp_ext = fopen(setup_ext_bak_file, "r+b");
	else
		fp_ext = fopen(setup_ext_bak_file, "wb");

	if (fp_ext)
	{
		fseek(fp_ext, 0L, SEEK_SET);
		sz = fwrite((void *)setup_ext, 1, sizeof(_CAMSETUP_EXT), fp_ext);
		if (sz != sizeof(_CAMSETUP_EXT)) {
			perror ("fwrite (ext)");
			sz = -1;
			goto err;
		}
	}
#if defined(USE_ENCRYPT_CMD_LINE)
	int ret = encrypt_file(CGI_SETUPFILE_BAK, setup_bak_file);
	if(ret < 0) {
		unlink(setup_bak_file);
	}
	else {
	}

	ret = encrypt_file(CGI_SETUPFILE_EXT_BAK, setup_ext_bak_file);
	if(ret < 0) {
		unlink(setup_ext_bak_file);
	}
	else {
	}
#endif	/* USE_ENCRYPT_CMD_LINE */
err:
	if (fp) {
		fflush(fp);
		fclose(fp);
	}
	if (fp_ext) {
		fflush(fp_ext);
		fclose(fp_ext);
	}

	if (sz >= 0)
	{
		__copyfile(CGI_SETUPFILE_BAK, CGI_SETUPFILE);
		__copyfile(CGI_SETUPFILE_EXT_BAK, CGI_SETUPFILE_EXT);
	}
	return sz;
#endif
}


int flash_open(const char *dev) {
	int fd;

	if ((fd = open(dev, O_RDWR, 0660)) < 0) {
		perror ("open");
		return -1;
	}

	return (fd);
}

void flash_close(int fd) {
	if (fd >= 0)
		close(fd);

	return;
}

int flash_write(int fd, unsigned long offset, const char *buf, int size) {

	assert (buf && size > 0 && fd >= 0);

	if (pwrite(fd, buf, size, offset) != size) {
		perror ("pwrite");
		return -1;
	}

	return 0;
}


int flash_read (int fd, unsigned long offset, char *buf, int size) {
	assert (buf && size > 0 && fd >= 0);

	if (pread(fd, buf, size, offset) != size) {
		perror ("pread");
		return -1;
	}

	return 0;
}



int flash_write_oob(int fd, unsigned long offset, char * buf, int size) {
	struct mtd_oob_buf	info;

	assert (fd >= 0);

	info.start = offset;
	info.length = size;
	info.ptr = (unsigned char*)buf;

	if (ioctl (fd, MEMWRITEOOB, (void *)& info) < 0) {
		perror ("ioctl");
		return -1;
	}

	return 0;
}


static int __flash_read (ST_FLASH *dev, unsigned long offset, char *buf, int size)  {

	return flash_read (dev->fd, offset, buf, size);
}

static int __flash_write (ST_FLASH *dev , unsigned long offset, char *buf, int size)  {

	unsigned col_wrcnt, page_size, sv_bcnt;	

	unsigned char oob_buf[64];

	page_size = dev->nandinfo.page_size;
	sv_bcnt = size;

	memset(oob_buf, 0xff, sizeof(oob_buf));

	while (size > 0)
	{
		col_wrcnt = page_size;

		if (size < page_size)
			memset(buf+size, 0xff, (page_size-size));

		flash_write (dev->fd, offset, buf, col_wrcnt);

		offset += col_wrcnt;
		size -= col_wrcnt;
	}

	if ((offset + size) >  dev->meminfo.size) {
		return -1;
	}

	return 0;
}

static void __flash_release (ST_FLASH *dev)  {
	flash_close (dev->fd);
	dev->fd = -1;
}

int flash_init (ST_FLASH *dev, char *devname) {
	assert (dev && devname);

	if ((dev->fd = flash_open (devname)) < 0) {
		perror("open");
		return -1;
	}


	if (ioctl (dev->fd, MEMGETINFO, (void *)&dev->meminfo) < 0) {
		perror ("ioctl");
		return -1;
	}


	dev->nandinfo.page_size = dev->meminfo.writesize;
	dev->nandinfo.chip_size = dev->meminfo.size;    
	dev->nandinfo.erase_size = dev->meminfo.erasesize;
	dev->nandinfo.option = 0;
	dev->read_fs = __flash_read_fs;
	dev->write_fs = __flash_write_fs;

	dev->read = __flash_read;
	dev->write = __flash_write;
	dev->release = __flash_release;

	return 0;
}


