

#ifndef _SHA1_H_
#define _SHA1_H_

typedef struct
{
  uint32    total[2];
  uint32    state[5];
  uint8     buffer[64];
} sha1_context;

#ifdef __cplusplus
extern "C" {
#endif

HT_API void sha1_starts(sha1_context * ctx);
HT_API void sha1_update(sha1_context * ctx, uint8 * input, uint32 length);
HT_API void sha1_finish(sha1_context * ctx, uint8 digest[20]);


#ifdef __cplusplus
}
#endif

#endif // _SHA1_H_


