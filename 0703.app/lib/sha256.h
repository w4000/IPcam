
#ifndef __SHA256_H__
#define __SHA256_H__

#define C_CM_IM_SHA256_OUT_SIZ			32

typedef struct tag_t_cm_sha256_ctx
{           
	unsigned int	total[2];
	unsigned int	state[8];
	unsigned char	buffer[64];
} t_cm_sha256_ctx, *pt_cm_sha256_ctx;


extern void cm_sha256_starts(pt_cm_sha256_ctx ctx);
extern void cm_sha256_update(pt_cm_sha256_ctx ctx, const unsigned char* input, unsigned int length);
extern void cm_sha256_finish(pt_cm_sha256_ctx ctx, unsigned char digest[32]);

//-------------------------- OPENSSL을 이용한 SHA256 --------------------------
extern void
	SHA256_digest (char *data, int datsz, char digest [SHA256_DIGEST_LENGTH]) ;
extern void
	SHA256_digest_string (char *data, int datsz, char str [128]);
extern void
	SHA256_digest_string_sz (char *data, int datsz, char str [], int sz);
#endif
