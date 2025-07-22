//
//	md5.c
//
//	purpose
//		MD5 encryption/decryption.
//
//	Original
//		FFmpeg.

#ifndef __DVR_MD5_H__
#define __DVR_MD5_H__



typedef struct AVMD5{
    uint64_t len;
    uint8_t  block[64];
    uint32_t ABCD[4];
} AVMD5;



extern const int av_md5_size;

struct AVMD5;

void av_md5_init(struct AVMD5 *ctx);
void av_md5_update(struct AVMD5 *ctx, const uint8_t *src, const int len);
void av_md5_final(struct AVMD5 *ctx, uint8_t *dst);
void av_md5_sum(uint8_t *dst, const uint8_t *src, const int len);

#endif

