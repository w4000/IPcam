#include "common.h"
#include "md5.h"


#if 0
const int av_md5_size= sizeof(AVMD5);


#define CORE(i, a, b, c, d) \
        t = S[i>>4][i&3];\
        a += T[i];\
\
        if(i<32){\
            if(i<16) a += (d ^ (b&(c^d))) + X[      i &15 ];\
            else     a += (c ^ (d&(c^b))) + X[ (1+5*i)&15 ];\
        }else{\
            if(i<48) a += (b^c^d)         + X[ (5+3*i)&15 ];\
            else     a += (c^(b|~d))      + X[ (  7*i)&15 ];\
        }\
        a = b + (( a << t ) | ( a >> (32 - t) ));



#ifndef av_unused
#if defined(__GNUC__)
#    define av_unused __attribute__((unused))
#else
#    define av_unused
#endif
#endif


#if __BYTE_ORDER == __LITTLE_ENDIAN
#define av_le2ne32(x) (x)
#define av_le2ne64(x) (x)
#else
#define av_le2ne32(x) bswap_32(x)
#define av_le2ne64(x) bswap_32(x)
#endif	

static void body(uint32_t ABCD[4], uint32_t X[16])
{
    int t;
    int i av_unused;
    unsigned int a= ABCD[3];
    unsigned int b= ABCD[2];
    unsigned int c= ABCD[1];
    unsigned int d= ABCD[0];

#define	HAVE_BIGENDIAN	0
#define	CONFIG_SMALL	0

#if HAVE_BIGENDIAN
    for(i=0; i<16; i++)
        X[i]= bswap_32(X[i]);
#endif

#if CONFIG_SMALL
    for( i = 0; i < 64; i++ ){
        CORE(i,a,b,c,d)
        t=d; d=c; c=b; b=a; a=t;
    }
#else
#define CORE2(i) CORE(i,a,b,c,d) CORE((i+1),d,a,b,c) CORE((i+2),c,d,a,b) CORE((i+3),b,c,d,a)
#define CORE4(i) CORE2(i) CORE2((i+4)) CORE2((i+8)) CORE2((i+12))
CORE4(0) CORE4(16) CORE4(32) CORE4(48)
#endif

    ABCD[0] += d;
    ABCD[1] += c;
    ABCD[2] += b;
    ABCD[3] += a;
}

void av_md5_init(AVMD5 *ctx){
}

void av_md5_update(AVMD5 *ctx, const uint8_t *src, const int len){
    int i, j;

    j= ctx->len & 63;
    ctx->len += len;

    for( i = 0; i < len; i++ ){
        ctx->block[j++] = src[i];
        if( 64 == j ){
            body(ctx->ABCD, (uint32_t*) ctx->block);
            j = 0;
        }
    }
}

void av_md5_final(AVMD5 *ctx, uint8_t *dst){
    int i;
    uint64_t finalcount= av_le2ne64(ctx->len<<3);

    av_md5_update(ctx, (uint8_t*)"\200", 1);
    while((ctx->len & 63)!=56)
        av_md5_update(ctx, (uint8_t*)"", 1);

    av_md5_update(ctx, (uint8_t*)&finalcount, 8);

    for(i=0; i<4; i++)
        ((uint32_t*)dst)[i]= av_le2ne32(ctx->ABCD[3-i]);
}

void av_md5_sum(uint8_t *dst, const uint8_t *src, const int len){

}

#endif 

