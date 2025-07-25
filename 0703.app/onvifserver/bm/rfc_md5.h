

#ifndef	RFC_MD5_H
#define	RFC_MD5_H

#define S11     7
#define S12     12
#define S13     17
#define S14     22
#define S21     5
#define S22     9
#define S23     14
#define S24     20
#define S31     4
#define S32     11
#define S33     16
#define S34     23
#define S41     6
#define S42     10
#define S43     15
#define S44     21

#define HASHLEN 16
typedef uint8 HASH[HASHLEN];
#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN+1];

/* MD5 context. */
typedef struct
{
	uint32  state[4];       /* state (ABCD) */
	uint32  count[2];       /* number of bits, modulo 2^64 (lsb first) */
	uint8   buffer[64];     /* input buffer */
} MD5_CTX;


#ifdef __cplusplus
extern "C"{
#endif

HT_API void MD5Init(MD5_CTX *);
HT_API void MD5Update(MD5_CTX *, uint8 *, uint32);
HT_API void MD5Final(uint8[16], MD5_CTX *);
HT_API void MD5String(uint8 * string, uint32 len, uint8 * result);

HT_API void BinToHexStr(HASH Bin, HASHHEX Hex);
HT_API BOOL HexStrToBin(char * str, HASH Bin);

#ifdef __cplusplus
}
#endif

#endif	// RFC_MD5_H




