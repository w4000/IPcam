#ifndef __BLOWFISH_H
#define __BLOWFISH_H


#define MAXKEYBYTES 56          /* 448 bits */

typedef struct {
  unsigned long P[16 + 2];
  unsigned long S[4][256];
} BLOWFISH_CTX;


/*
 * SSF modified version
 */
extern void SSF_Blowfish_Init (BLOWFISH_CTX *ctx, unsigned char *data, long len);
extern void SSF_Blowfish_Encrypt (BLOWFISH_CTX *ctx, unsigned long *data);

static inline void BF_Init(BLOWFISH_CTX *ctx, long *key)
{
	SSF_Blowfish_Init(ctx, (unsigned char*)key, sizeof(long));
	return;
}


#endif 

