#ifndef _AES_H_
#define _AES_H_

//#define AES_BLOCKLEN 16 //Block length in bytes AES is 128b block only
#define AES_BLOCKLEN 16
#define AES_KEYLEN 16   // Key length in bytes
#define AES_keyExpSize 176

#include <stdint.h>

// state - array holding the intermediate results during decryption.
typedef uint8_t state_t[4][4];

struct AES_ctx
{
  uint8_t RoundKey[AES_keyExpSize];
  uint8_t Iv[AES_BLOCKLEN];
};

void AES_init_ctx(struct AES_ctx* ctx, const uint8_t* key);
void AES_init_ctx_iv(struct AES_ctx* ctx, const uint8_t* key, const uint8_t* iv);
void AES_ctx_set_iv(struct AES_ctx* ctx, const uint8_t* iv);

int AES_CBC_encrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, uint32_t length);
void AES_CBC_decrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, uint32_t length);
void AES_CBC_encrypt_bufferEx(struct AES_ctx *ctx,uint8_t* buf, uint8_t* out, uint32_t length);


void CipherEx(state_t* state, state_t* out, uint8_t* RoundKey);
void XorWithIv(uint8_t* buf, uint8_t* Iv);

#endif //_AES_H_
