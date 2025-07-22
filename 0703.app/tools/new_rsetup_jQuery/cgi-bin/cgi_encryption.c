/****************************************************************************
 * cgi_comon.c
 *
 * purpose:
 *	- cgi common functions.
 ****************************************************************************/
#include "common.h"
#include "cgi_encryption.h"

#include <openssl/rsa.h>

//#define USE_HI_CIPHER
#ifdef USE_HI_CIPHER
#include "hi_unf_cipher.h"
#endif

static int padding = RSA_PKCS1_PADDING;

#ifdef USE_HI_CIPHER
static RSA * createRSA(unsigned char * key,int public)
{
    RSA *rsa = NULL;
    BIO *keybio = BIO_new_mem_buf(key, -1);
    if (keybio)
    {
		if(public)
			rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa,NULL, NULL);
		else
			rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa,NULL, NULL);

		BIO_free( keybio );
    }

    return rsa;
}

int public_encrypt(char *data, int data_len, unsigned char *key, unsigned char *encrypted)
{
	HI_S32 ret = HI_SUCCESS;
    HI_U8  u8Enc[256];
    HI_U32 u32OutLen;
    HI_UNF_CIPHER_RSA_PUB_ENC_S stRsaEnc;
	HI_UNF_CIPHER_RSA_ENC_SCHEME_E enScheme = HI_UNF_CIPHER_RSA_ENC_SCHEME_RSAES_PKCS1_V1_5;

	RSA * rsa = createRSA(key, 1);
    if(rsa == NULL) {
		return HI_FAILURE;
    }

	ret = HI_UNF_CIPHER_Init();
	if(HI_SUCCESS != ret) {
		return HI_FAILURE;
	}

    memset(&stRsaEnc, 0, sizeof(HI_UNF_CIPHER_RSA_PUB_ENC_S));

    stRsaEnc.enScheme = enScheme;
    stRsaEnc.stPubKey.pu8N = (HI_U8)rsa->n->d;
    stRsaEnc.stPubKey.pu8E = (HI_U8)rsa->e->d;
    stRsaEnc.stPubKey.u16NLen = rsa->n->dmax;
    stRsaEnc.stPubKey.u16ELen = rsa->e->dmax;
	ret = HI_UNF_CIPHER_RsaPublicEnc(&stRsaEnc, data, data_len, u8Enc, &u32OutLen);

    if ( HI_SUCCESS != ret )
	{
	    printf("failed\n");
		return HI_FAILURE;
	}

   	memcpy(encrypted, u8Enc, u32OutLen);

    HI_UNF_CIPHER_DeInit();

	RSA_free( rsa );

    return HI_SUCCESS;
}


int private_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted)
{
	int result = -1;
    RSA * rsa = createRSA(key,0);
    if (rsa) {
    	result = RSA_private_decrypt(data_len,enc_data,decrypted,rsa,padding);
    	RSA_free( rsa );
    }
    return result;
}

int private_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted)
{
	int result = -1;
	RSA * rsa = createRSA(key,0);
    if (rsa) {
    	result = RSA_private_encrypt(data_len,data,encrypted,rsa,padding);
    	RSA_free( rsa );
    }
    return result;
}

int public_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted)
{
	int result = -1;
	RSA * rsa = createRSA(key,1);
    if (rsa) {
    	result = RSA_public_decrypt(data_len,enc_data,decrypted,rsa,padding);
    	RSA_free( rsa );
    }
    return result;
}
#else /*USE_HI_CIPHER*/
static RSA * createRSA(unsigned char * key,int public)
{
    RSA *rsa = NULL;
    BIO *keybio = BIO_new_mem_buf(key, -1);
    if (keybio)
    {
		if(public)
			rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa,NULL, NULL);
		else
			rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa,NULL, NULL);

		BIO_free( keybio );
    }

    return rsa;
}

int public_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted)
{
	int result = -2;
    RSA * rsa = createRSA(key,1);
    if (rsa) {
    	result = RSA_public_encrypt(data_len,data,encrypted,rsa,padding);
    	RSA_free( rsa );
    }
    return result;
}


int private_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted)
{
	int result = -1;
    RSA * rsa = createRSA(key,0);
    if (rsa) {
    	result = RSA_private_decrypt(data_len,enc_data,decrypted,rsa,padding);
    	RSA_free( rsa );
    }
    return result;
}

int private_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted)
{
	int result = -1;
	RSA * rsa = createRSA(key,0);
    if (rsa) {
    	result = RSA_private_encrypt(data_len,data,encrypted,rsa,padding);
    	RSA_free( rsa );
    }
    return result;
}

int public_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted)
{
	int result = -1;
	RSA * rsa = createRSA(key,1);
    if (rsa) {
    	result = RSA_public_decrypt(data_len,enc_data,decrypted,rsa,padding);
    	RSA_free( rsa );
    }
    return result;
}
#endif /*USE_HI_CIPHER*/

// Base64
#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

static const uint8_t map2[] =
{
    0x3e, 0xff, 0xff, 0xff, 0x3f, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01,
    0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1a, 0x1b,
    0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
    0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33
};

int ___base64_decode(uint8_t *out, const char *in, int out_size)
{
    int i, v;
    uint8_t *dst = out;

    v = 0;
    for (i = 0; in[i] && in[i] != '='; i++) {
        unsigned int index= in[i]-43;
        if (index>=FF_ARRAY_ELEMS(map2) || map2[index] == 0xff)
            return -1;
        v = (v << 6) + map2[index];
        if (i & 3) {
            if (dst - out < out_size) {
                *dst++ = v >> (6 - 2 * (i & 3));
            }
        }
    }

    return dst - out;
}

char * __cm_base64_encode(char *out, int ccOut, unsigned char *in, int cbInp)
{
	static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	char *ret, *dst;
	unsigned i_bits = 0;
	int i_shift = 0;
	int bytes_remaining = cbInp;

	if (cbInp >= UINT_MAX / 4 || ccOut < (cbInp+2) / 3 * 4 + 1)
		return NULL;

	ret = dst = out;
	while (bytes_remaining) {
		i_bits = (i_bits << 8) + *in++;
		bytes_remaining--;
		i_shift += 8;

		do {
			*dst++ = b64[(i_bits << 6 >> i_shift) & 0x3f];
			i_shift -= 6;
		} while (i_shift > 6 || (bytes_remaining == 0 && i_shift > 0));

	} while ((dst - ret) & 3)

	*dst++ = '=';
	*dst = '\0';

	return ret;
}

void trim_keyword(char* keyword)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		if (keyword[strlen(keyword)-1] == 0xa)
			keyword[strlen(keyword)-1] = 0;
		if (keyword[strlen(keyword)-1] == '\n')
			keyword[strlen(keyword)-1] = 0;
		if (keyword[strlen(keyword)-1] == '\a')
			keyword[strlen(keyword)-1] = 0;
		if (keyword[strlen(keyword)-1] == ' ')
			keyword[strlen(keyword)-1] = 0;
	}
}

bool file_exist(const char *filename)
{
#if 1
	return (access(filename, F_OK) == 0);
#else
#endif
}

int get_enc_key(char *pub_key, char *priv_key, int include_pub_key_startend)
{
	char cmd[128];
	const char* encCmd = "/home/bin/openssl enc -aes-128-cbc";

	enum { RSA_SIZE = 1024 };
	char str[RSA_SIZE];
	char _pub_key[RSA_SIZE];
	char _priv_key[RSA_SIZE];

	memset(_pub_key, 0, sizeof(_pub_key));
	memset(_priv_key, 0, sizeof(_priv_key));

	const char* pub_key_file = "/tmp/_key/pub.key.pem";
	FILE* fp_key = fopen(pub_key_file, "rt");
	if (!fp_key) {
		system("mkdir -p /tmp/_key");
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd), "%s genrsa -out /tmp/_key/priv.key.pem %d", encCmd, RSA_SIZE);
		system(cmd);
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd), "%s rsa -pubout -in /tmp/_key/priv.key.pem -out /tmp/_key/pub.key.pem", encCmd);
		system(cmd);

		fp_key = fopen(pub_key_file, "rt");
	}

	if (fp_key) {
		if(include_pub_key_startend) {
			fread(_pub_key, 1, sizeof(_pub_key), fp_key);
		}
		else {
			while (fgets (str, sizeof (str), fp_key)) {
				trim_keyword(str);
				if (strstr(str, "-----") == NULL)
					strsafecat(_pub_key, str, sizeof(_pub_key));
			}
		}
		fclose(fp_key);
	}

	if (pub_key != NULL) {
		sprintf(pub_key, "%s", _pub_key);
	}

	fp_key = fopen("/tmp/_key/priv.key.pem", "rt");
	if (fp_key)
	{
		while (fgets (str, sizeof (str), fp_key)) {
			trim_keyword(str);
			if (strstr(str, "-----") == NULL)
				strsafecat(_priv_key, str, sizeof(_priv_key));
		}
		fclose(fp_key);
	}

	if (priv_key != NULL) {
		sprintf(priv_key, "%s", _priv_key);
	}

	return 0;
}
