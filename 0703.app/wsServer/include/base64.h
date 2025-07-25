/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef BASE64_H
#define BASE64_H

#include <sys/types.h>

unsigned char * ws_base64_encode(const unsigned char *src, size_t len,
			      size_t *out_len);
unsigned char * ws_base64_decode(const unsigned char *src, size_t len,
			      size_t *out_len);

#endif /* BASE64_H */
