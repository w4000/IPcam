

#ifndef BASE64_H
#define BASE64_H

#ifdef __cplusplus
extern "C" {
#endif

HT_API int onvif_base64_encode(uint8 *source, uint32 sourcelen, char *target, uint32 targetlen);
HT_API int onvif_base64_decode(const char *source, uint32 sourcelen, uint8 *target, uint32 targetlen);

#ifdef __cplusplus
}
#endif


#endif /* BASE64_H */


