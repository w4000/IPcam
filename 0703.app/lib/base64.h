//
//	base64
//
//	Purpose
//		Base64 encoding/decoding.
//

#ifndef	__DVR_BASE64_H__
#define	__DVR_BASE64_H__


/**
 * Calculate the output size needed to base64-encode x bytes.
 */
#define BASE64_SIZE(x)  (((x)+2) / 3 * 4 + 1)


extern int	base64_decode(uint8_t *out, const char *in, int out_size);
extern char *base64_encode(char *out, int out_size, const uint8_t *in, int in_size);

extern char *cm_base64_encode(char *out, int ccOut, unsigned char *in, int cbInp);
extern int cm_base64_decode(unsigned char *out, const char *in, int cbOut);


#endif

