#ifndef _BASE64_P_H_
#define _BASE64_P_H_

int base64encode_len(int len);
int base64encode(char *coded_dst, const char *plain_src, int len_plain_src);

int base64decode_len(const char * coded_src);
int base64decode(char *plain_dst, const char *coded_src);

#endif //_BASE64_P_H_
