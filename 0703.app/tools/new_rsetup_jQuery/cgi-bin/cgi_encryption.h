/****************************************************************************
 * cgi_common.h
 *
 * purpose:
 *	- cgi common functions header.
 ****************************************************************************/
#ifndef __CGI_ENCRYPTION_H__
#define __CGI_ENCRYPTION_H__

#include <libcgi/cgi.h>
#include <libcgi/session.h>

extern int ___base64_decode(uint8_t *out, const char *in, int out_size);
extern char *__cm_base64_encode(char *out, int ccOut, unsigned char *in, int cbInp);

extern int public_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted);
extern int private_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted);
extern int private_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted);
extern int public_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted);

extern bool file_exist(const char *filename);

extern int get_enc_key(char *pub_key, char *priv_key, int include_pub_key_startend);
//extern int get_pub_key(char *key);
//extern int get_priv_key(char *key);

#endif	// __CGI_ENCRYPTION_H__

