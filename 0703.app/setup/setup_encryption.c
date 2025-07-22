#include <fcntl.h>
#include <unistd.h>

#include "common.h"
#include "openssl/md5.h"

#define AES_MAGIC		0x79110919
#define AES_KEY_BIT 	128
#define AES_KEY_BYTE 	16
#define AES_BLOCK_SIZE	16

static unsigned char aes_key[AES_KEY_BYTE] = "ecam!!!";

#define ENCSETSIZE(sz) ((((sz)/AES_BLOCK_SIZE)+1)*AES_BLOCK_SIZE)

typedef struct _SETUP_AES_HEADER_S {
	unsigned int magic;
	unsigned int size;
	unsigned int opt;
} SETUP_AES_HEADER_S;

static void dump_data(char *title, char *src, char *dst, int size)
{
#if 0
	int i;
	printf("==================================================================\n");
	printf("==================================================================\n");
	printf("==================================================================\n");
	printf("%s :: ", title);
	for(i = 0; i < size; i++) {
		printf("%02x ", src[i]);
	}
	printf(" ---> ");
	for(i = 0; i < size; i++) {
		printf("%02x ", dst[i]);
	}
	printf("\n");
	printf("==================================================================\n");
	printf("==================================================================\n");
	printf("==================================================================\n");
#endif
}

static int __setup_decryption(void *src, void *dst, int size)
{
	AES_KEY key;
	unsigned char iv_aes[AES_BLOCK_SIZE] = {0};

	memset(iv_aes, 0, sizeof(iv_aes));
	AES_set_decrypt_key(aes_key, AES_KEY_BIT, &key);
	AES_cbc_encrypt(src, dst, size, &key, iv_aes, AES_DECRYPT);
	dump_data("dec", src, dst, 16);

	return 0;
}

static int __setup_encryption(void *src, void *dst, int size)
{
	AES_KEY key;
	int enc_size = ENCSETSIZE(size);// + 256;
	unsigned char iv_aes[AES_BLOCK_SIZE] = {0};

	memset(iv_aes, 0, sizeof(iv_aes));
	AES_set_encrypt_key(aes_key, AES_KEY_BIT, &key);
	AES_cbc_encrypt(src, dst, enc_size, &key, iv_aes, AES_ENCRYPT);
	dump_data("enc", src, dst, 16);

	return enc_size;
}

static int __read_setup(const char *file, const char *bak_file, void *data, int data_size)
{
	FILE *fp = NULL;
	int sz = -1;
	int bIsBackupFile = 0;
	SETUP_AES_HEADER_S hdr;

	void *r_data = 0;
	void *dec_data = 0;

retry:

	fp = fopen(file, "rb");
	if(!fp) {
		if(bIsBackupFile) {
			perror ("fopen");
			return -1;
		}
		else {
			__copyfile(bak_file, file);
			bIsBackupFile = 1;
			goto retry;
		}
	}

	sz = fread((void *)&hdr, 1, sizeof(SETUP_AES_HEADER_S), fp);
	if(sz != sizeof(SETUP_AES_HEADER_S)) {
		if(bIsBackupFile) {
			perror ("fread");
			printf("fread error");
			sz = -1;
			goto err;
		}
		else {
			fclose(fp);
			__copyfile(bak_file, file);
			bIsBackupFile = 1;
			goto retry;
		}
	}

	if(hdr.magic != AES_MAGIC) {
		sz = -1;
		goto err;
	}

	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, hdr.size);
	r_data = malloc(hdr.size);

	sz = fread((void *)r_data, 1, hdr.size, fp);
	if(sz != hdr.size) {
		if(bIsBackupFile) {
			perror ("fread");
			printf("fread error");
			sz = -1;
			goto err;
		}
		else {
			fclose(fp);
			__copyfile(bak_file, file);
			bIsBackupFile = 1;
			goto retry;
		}
	}

	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, hdr.size);
	dec_data = malloc(hdr.size);

	if(__setup_decryption(r_data, dec_data, hdr.size) < 0) {
		sz = -1;
		goto err;
	}

	memcpy(data, dec_data, data_size);

err:

	if(fp) {
		fclose(fp);
	}

	if(r_data) {
		free(r_data);
	}

	if(dec_data) {
		free(dec_data);
	}

	return sz;
}

int read_enc_setup_basic(_CAMSETUP *setup)
{
	return __read_setup(CGI_SETUPFILE, CGI_SETUPFILE_BAK, (void *)setup, sizeof(_CAMSETUP));
}

int read_enc_setup_ext(_CAMSETUP_EXT *setup_ext)
{
	int bIsBackupExtFile = 0;

retry:
	if(__read_setup(CGI_SETUPFILE_EXT, CGI_SETUPFILE_EXT_BAK, (void *)setup_ext, sizeof(_CAMSETUP_EXT)) < 0) {
		return -1;
	}

	if(setup_ext->magic[0] != MAGIC_NUM_1 ||
			setup_ext->magic[1] != 0x00710225 ||
			setup_ext->magic[2] != 0x00951220 ||
			setup_ext->magic[3] != 0x00981029) {
		if(bIsBackupExtFile == 0) {
			bIsBackupExtFile = 1;
			__copyfile(CGI_SETUPFILE_EXT_BAK, CGI_SETUPFILE_EXT);
			goto retry;
		}
		return -1;
	}

	return 0;
}

int read_enc_setup(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext)
{
	int ret;

	ret = read_enc_setup_basic(setup);
	if(ret < 0) {
		return ret;
	}

	if(setup_ext == NULL) {
		return ret;
	}

	ret = read_enc_setup_ext(setup_ext);
	if(ret < 0) {
		return ret;
	}

	return ret;
}

static int __write_setup(const char *file, const char *bak_file, void *data, int data_size)
{
	FILE *fp = NULL;
	int sz = -1;
	SETUP_AES_HEADER_S hdr;
	void *w_data = 0;
	int w_size;

	fp = fopen(bak_file, "wb");
	if(!fp) {
		perror ("fopen");
		return -1;
	}

	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, ENCSETSIZE(data_size) + 256);
	w_data = malloc(ENCSETSIZE(data_size) + 256);
	w_size = __setup_encryption(data, w_data, data_size);

	hdr.magic = AES_MAGIC;
	hdr.size = w_size;
	hdr.opt = 0;

	sz = fwrite((void *)&hdr , 1, sizeof(SETUP_AES_HEADER_S), fp);
	if(sz != sizeof(SETUP_AES_HEADER_S)) {
		perror ("fwrite");
		sz = -1;
		goto err;
	}

	sz = fwrite((void *)w_data , 1, w_size, fp);
	if(sz != w_size) {
		perror ("fwrite");
		sz = -1;
		goto err;
	}


err:
	if(fp)
		fclose(fp);

	if(w_data) {
		free(w_data);
	}

	if(sz >= 0) {
		__copyfile(bak_file, file);
	}

	return 0;
}


int write_enc_setup_basic(_CAMSETUP *setup)
{
	return __write_setup(CGI_SETUPFILE, CGI_SETUPFILE_BAK, setup, sizeof(_CAMSETUP));
}

int write_enc_setup_ext(_CAMSETUP_EXT *setup_ext)
{
	return __write_setup(CGI_SETUPFILE_EXT, CGI_SETUPFILE_EXT_BAK, setup_ext, sizeof(_CAMSETUP_EXT));
}

int write_enc_setup(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext)
{
	int ret;

	ret = write_enc_setup_basic(setup);
	if(ret < 0) {
		return ret;
	}


	if(setup_ext == NULL) {
		return ret;
	}

	ret = write_enc_setup_ext(setup_ext);
	if(ret < 0) {
		return ret;
	}

	return ret;
}

// USE_ENCRYPT_CMD_LINE

int decrypt_file(const char* dest, const char* src)
{
	if(src == NULL || access(src, F_OK) != 0) {
		if(src == NULL) {
		}
		if(access(src, F_OK) != 0) {
		}
		return -1;
	}
	const char* dec_key = "11223344556677889900998877665544";
						   
	char cmd[256];
	const char* encCmd = "/home/bin/openssl enc -aes-128-cbc";

	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "%s -d -in %s -out %s -K \'%s\' -iv 00 -p", encCmd, src, dest, dec_key);
	popen(cmd, "r");

	return 0;
}

int encrypt_file(const char* dest, const char* src)
{
	if(src == NULL || access(src, F_OK) != 0) {
		if(src == NULL) {
		}
		if(access(src, F_OK) != 0) {
		}
		return -1;
	}
	const char* enc_key = "11223344556677889900998877665544";
	char cmd[256];
	const char* encCmd = "/home/bin/openssl enc -aes-128-cbc";

	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "%s -in %s -out %s -K \'%s\' -iv 00 -p", encCmd, src, dest, enc_key);
	popen(cmd, "r");

	return 0;
}


