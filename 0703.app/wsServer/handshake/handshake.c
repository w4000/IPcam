#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "../include/base64.h"
#include "../include/sha1.h"
#include "../include/ws.h"
#include "../lib/threadsafe.h"

/**
 * @dir src/handshake
 * @brief Handshake routines directory
 *
 * @file handshake.c
 * @brief Handshake routines.
 */

/**
 * @brief Gets the field Sec-WebSocket-Accept on response, by
 * an previously informed key.
 *
 * @param wsKey Sec-WebSocket-Key
 * @param dest source to be stored the value.
 *
 * @return Returns 0 if success and a negative number
 * otherwise.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
int get_handshake_accept(char *wsKey, unsigned char **dest)
{
	unsigned char hash[SHA1HashSize]; /* SHA-1 Hash.                   */
	SHA1Context ctx;                  /* SHA-1 Context.                */
	char *str;                        /* WebSocket key + magic string. */

	/* Invalid key. */
	if (!wsKey)
		return (-1);

	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(char) * (WS_KEY_LEN + WS_MS_LEN + 1));
	str = calloc(1, sizeof(char) * (WS_KEY_LEN + WS_MS_LEN + 1));
	if (!str)
		return (-1);

	strncpy(str, wsKey, WS_KEY_LEN);
	strcat(str, MAGIC_STRING);

	SHA1Reset(&ctx);
	SHA1Input(&ctx, (const uint8_t *)str, WS_KEYMS_LEN);
	SHA1Result(&ctx, hash);

	*dest = ws_base64_encode(hash, SHA1HashSize, NULL);
	*(*dest + strlen((const char *)*dest) - 1) = '\0';
	free(str);
	return (0);
}

/**
 * @brief Gets the complete response to accomplish a succesfully
 * handshake.
 *
 * @param hsrequest  Client request.
 * @param hsresponse Server response.
 *
 * @return Returns 0 if success and a negative number
 * otherwise.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
int get_handshake_response(char *hsrequest, char **hsresponse)
{
	unsigned char *accept; /* Accept message.     */
	char *saveptr;         /* strtok_r() pointer. */
	char *s;               /* Current string.     */
	int ret;               /* Return value.       */

	saveptr = NULL;
	for (s = strtok_r(hsrequest, "\r\n", &saveptr); s != NULL;
		 s = strtok_r(NULL, "\r\n", &saveptr))
	{
		if (strstr(s, WS_HS_REQ) != NULL)
			break;
	}

	/* Ensure that we have a valid pointer. */
	if (s == NULL)
		return (-1);

	saveptr = NULL;
	s       = strtok_r(s, " ", &saveptr);
	s       = strtok_r(NULL, " ", &saveptr);

	ret = get_handshake_accept(s, &accept);


	// printf("get_handshake_response \n=======================================\n");
	// printf("accept? %s \n", accept);
	// printf("================================================================\n");


	if (ret < 0)
		return (ret);

	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(char) * WS_HS_ACCLEN);
	*hsresponse = malloc(sizeof(char) * WS_HS_ACCLEN);
	if (*hsresponse == NULL)
		return (-1);

	strcpy(*hsresponse, WS_HS_ACCEPT);
	strcat(*hsresponse, (const char *)accept);
	strcat(*hsresponse, "\r\n\r\n");

	// printf("get_handshake_response \n=======================================\n");
	// printf("hsresponse? %s \n", hsresponse);
	// printf("================================================================\n");

	free(accept);
	return (0);
}
