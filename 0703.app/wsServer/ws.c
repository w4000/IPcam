
#include "common.h"

#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
// #include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* clang-format off */
#ifndef _WIN32
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
typedef int socklen_t;
#endif
/* clang-format on */

/* Windows and macOS seems to not have MSG_NOSIGNAL */
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#include <unistd.h>

#include "include/ws.h"
// ws_socket 함수에서 아래 전역변수가 1로 변경
// ws_socket_stop 으로 0으로 변경
static int g_websocket_run_state = 0;

/**
 * @dir src/
 * @brief wsServer source code
 *
 * @file ws.c
 * @brief wsServer main routines.
 */

/**
 * @brief Opened g_ports.
 */
static int g_port_index;

/**
 * @brief Port entry in @ref ws_port structure.
 *
 * This defines the port number and events for a single
 * call to @ref ws_socket. This allows that multiples threads
 * can call @ref ws_socket, configuring different g_ports and
 * events for each call.
 */
struct ws_port
{
	int port_number;         /**< Port number.      */
	struct ws_events events; /**< Websocket events. */
};

/**
 * @brief ws_accept data.
 *
 * This defines a set of data that is used inside of each
 * accept routine, whether by the main routine or not.
 */


#if 0
struct ws_accept
{
	int sock;       /**< Socket number.               */
	int port_index; /**< Port index in the port list. */
};
#else 
typedef struct _WS_ACCEPT
{
	int sock;       /**< Socket number.               */
	int port_index; /**< Port index in the port list. */
}WS_ACCEPT;
#endif 





/**
 * @brief Ports list.
 */
static struct ws_port g_ports[MAX_PORTS];

/**
 * @brief Client socks.
 */
struct ws_connection
{
	int ws_client_sock; /**< Client socket FD.        */
	int port_index;  /**< Index in the port list.  */
	int state;       /**< WebSocket current state. */

	/* Timeout thread and locks. */
	pthread_mutex_t mtx_state;
	pthread_cond_t cnd_state_close;
	pthread_t thrd_tout;
	bool close_thrd;

	/* Send lock. */
	pthread_mutex_t mtx_snd;
};

/**
 * @brief Clients list.
 */
static struct ws_connection client_socks[MAX_CLIENTS];

/**
 * @brief WebSocket frame data
 */
struct ws_frame_data
{
	/**
	 * @brief Frame read.
	 */
	unsigned char frm[MESSAGE_LENGTH];
	/**
	 * @brief Processed message at the moment.
	 */
	unsigned char *msg;
	/**
	 * @brief Control frame payload
	 */
	unsigned char msg_ctrl[125];
	/**
	 * @brief Current byte position.
	 */
	size_t cur_pos;
	/**
	 * @brief Amount of read bytes.
	 */
	size_t amt_read;
	/**
	 * @brief Frame type, like text or binary.
	 */
	int frame_type;
	/**
	 * @brief Frame size.
	 */
	uint64_t frame_size;
	/**
	 * @brief Error flag, set when a read was not possible.
	 */
	int error;
	/**
	 * @brief Client socket file descriptor.
	 */
	int sock;
	/**
	 * @brief WS_HS_FORWARDED value
	 */
	char forwarded_ip[32];
};

/**
 * @brief Global mutex.
 */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Issues an error message and aborts the program.
 *
 * @param s Error message.
 */
#define panic(s)   \
	do             \
	{				\
		perror(s); \
		if (0) exit(-1);  \
	} while (0);

/**
 * @brief Shutdown and close a given socket.
 *
 * @param fd Socket file descriptor to be closed.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
static void close_socket(int fd)
{
#ifndef _WIN32
	shutdown(fd, SHUT_RDWR);
	close(fd);
#else
	closesocket(fd);
#endif
}

/**
 * @brief For a given client @p fd, returns its
 * client index if exists, or -1 otherwise.
 *
 * @param fd Client fd.
 *
 * @return Return the client index or -1 if invalid
 * fd.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
static int get_client_index(int fd)
{
	int i;
	pthread_mutex_lock(&mutex);
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (client_socks[i].ws_client_sock == fd)
			break;
	}
	pthread_mutex_unlock(&mutex);
	return (i == MAX_CLIENTS ? -1 : i);
}

/**
 * @brief Returns the current client state for a given
 * client @p idx.
 *
 * @param idx Client index.
 *
 * @return Returns the client state, -1 otherwise.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
static int get_client_state(int idx)
{
	int state;

	if (idx < 0 || idx >= MAX_CLIENTS)
		return (-1);

	pthread_mutex_lock(&client_socks[idx].mtx_state);
	state = client_socks[idx].state;
	pthread_mutex_unlock(&client_socks[idx].mtx_state);
	return (state);
}

/**
 * @brief Set a state @p state to the client index
 * @p idx.
 *
 * @param idx Client index.
 * @param state State to be set.
 *
 * @return Returns 0 if success, -1 otherwise.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
static int set_client_state(int idx, int state)
{
	if (idx < 0 || idx >= MAX_CLIENTS)
		return (-1);

	if (state < 0 || state > 3)
		return (-1);

	pthread_mutex_lock(&client_socks[idx].mtx_state);
	client_socks[idx].state = state;
	pthread_mutex_unlock(&client_socks[idx].mtx_state);
	return (0);
}

/**
 * @brief Send a given message @p buf on a socket @p sockfd.
 *
 * @param sockfd Target socket.
 * @param buf Message to be sent.
 * @param len Message length.
 * @param flags Send flags.
 * @param idx Client index, in order to serialize the messages
 *            to the same client.
 *
 * @return Returns 0 if success (i.e: all message was sent),
 * -1 otherwise.
 *
 * @note Technically this shouldn't be necessary, since send() should
 * block until all content is sent, since _we_ don't use 'O_NONBLOCK'.
 * However, it was reported (issue #22 on GitHub) that this was
 * happening, so just to be cautious, I will keep using this routine.
 */
static ssize_t send_all(int sockfd, const void *buf, size_t len,
	int flags, int idx)
{
	const char *p;
	ssize_t ret;

	// printf("! %s:%d >>> idx? %d  \n", __func__, __LINE__, idx);
	/* Sanity check. */
	if (idx < 0)
		return (-1);

	p = buf;
	pthread_mutex_lock(&client_socks[idx].mtx_snd);
		while (len)
		{
			ret = send(sockfd, p, len, flags);
			if (ret == -1)
			{
				pthread_mutex_unlock(&client_socks[idx].mtx_snd);
				return (-1);
			}
			p += ret;
			len -= ret;
		}
	pthread_mutex_unlock(&client_socks[idx].mtx_snd);
	return (0);
}

/**
 * @brief Close client connection (no close handshake, this should
 * be done earlier), set appropriate state and destroy mutexes.
 *
 * @param conn_idx Connection index, -1 if should use @p fd.
 * @param fd Optional fd parameter, used when there is no
 *           @p conn_idx.
 */
static void close_client(int conn_idx, int fd)
{
	if (conn_idx == -1)
		conn_idx = get_client_index(fd);

	/* Destroy client mutexes and clear fd 'slot'. */
	pthread_mutex_lock(&mutex);
		set_client_state(conn_idx, WS_STATE_CLOSED);
		close_socket(client_socks[conn_idx].ws_client_sock);

		client_socks[conn_idx].ws_client_sock = -1;
		pthread_cond_destroy(&client_socks[conn_idx].cnd_state_close);
		pthread_mutex_destroy(&client_socks[conn_idx].mtx_state);
		pthread_mutex_destroy(&client_socks[conn_idx].mtx_snd);
	pthread_mutex_unlock(&mutex);
}

/**
 * @brief Close time-out thread.
 *
 * For a given client, this routine sleeps until
 * TIMEOUT_MS and closes the connection or returns
 * sooner if already closed connection.
 *
 * @param p ws_connection Structure Pointer.
 *
 * @return Always NULL.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
static void *close_timeout(void *p)
{
	struct ws_connection *conn = p;
	struct timespec ts;
	int state;

	clock_gettime(CLOCK_REALTIME, &ts);
	pthread_mutex_lock(&conn->mtx_state);

	// ts.tv_nsec += MS_TO_NS(TIMEOUT_MS);
	ts.tv_sec += 5;

	/* Normalize the time. */
	while (ts.tv_nsec >= 1000000000)
	{
		ts.tv_sec++;
		ts.tv_nsec -= 1000000000;
	}

	while (conn->state != WS_STATE_CLOSED &&
		   pthread_cond_timedwait(&conn->cnd_state_close, &conn->mtx_state, &ts) !=
			   ETIMEDOUT)
		;

	state = conn->state;
	pthread_mutex_unlock(&conn->mtx_state);

	/* If already closed. */
	if (state == WS_STATE_CLOSED)
		goto quit;

	// 
	// printf("Timer expired, closing client %d\n", conn->ws_client_sock);

	// pthread_mutex_lock(&conn->mtx_state);
	close_client(-1, conn->ws_client_sock);
	// pthread_mutex_unlock(&conn->mtx_state);

quit:
	return (NULL);
}

/**
 * @brief For a valid client index @p idx, starts
 * the timeout thread and set the current state
 * to 'CLOSING'.
 *
 * @param idx Client index.
 *
 * @return Returns 0 if success, -1 otherwise.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
static int start_close_timeout(int idx)
{
	if (idx < 0 || idx >= MAX_CLIENTS)
		return (-1);

	pthread_mutex_lock(&client_socks[idx].mtx_state);

	if (client_socks[idx].state != WS_STATE_OPEN)
		goto out;

	client_socks[idx].state = WS_STATE_CLOSING;

	if (pthread_create(
			&client_socks[idx].thrd_tout, NULL, close_timeout, &client_socks[idx]))
	{
		pthread_mutex_unlock(&client_socks[idx].mtx_state);
		panic("Unable to create timeout thread\n");
	}
	client_socks[idx].close_thrd = TRUE;
out:
	pthread_mutex_unlock(&client_socks[idx].mtx_state);
	return (0);
}

/**
 * @brief Gets the IP address relative to a file descriptor opened
 * by the server.
 *
 * @param fd File descriptor target.
 *
 * @return Pointer the ip address, or NULL if fails.
 *
 * @note It is up the caller to free the returned string.
 */
char *ws_getaddress(int fd)
{
	struct sockaddr_in addr;
	socklen_t addr_size;
	char *client;

	addr_size = sizeof(struct sockaddr_in);
	if (getpeername(fd, (struct sockaddr *)&addr, &addr_size) < 0)
		return (NULL);

	
	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(char) * INET_ADDRSTRLEN);
	client = malloc(sizeof(char) * INET_ADDRSTRLEN);
	if (!client)
		return (NULL);

	if (!inet_ntop(AF_INET, &addr.sin_addr, client, INET_ADDRSTRLEN))
	{
		free(client);
		return (NULL);
	}
	return (client);
}

/**
 * @brief Creates and send an WebSocket frame with some payload data.
 *
 * This routine is intended to be used to create a websocket frame for
 * a given type e sending to the client. For higher level routines,
 * please check @ref ws_sendframe_txt and @ref ws_sendframe_bin.
 *
 * @param fd        Target to be send.
 * @param msg       Message to be send.
 * @param size      Binary message size.
 * @param broadcast Enable/disable broadcast.
 * @param type      Frame type.
 *
 * @return Returns the number of bytes written, -1 if error.
 *
 * @note If @p size is -1, it is assumed that a text frame is being sent,
 * otherwise, a binary frame. In the later case, the @p size is used.
 */
int ws_sendframe(int fd, const char *msg, uint64_t size, bool broadcast, int type)
{
	unsigned char *response; /* Response data.     */
	unsigned char frame[10]; /* Frame.             */
	uint8_t idx_first_rData; /* Index data.        */
	uint64_t length;         /* Message length.    */
	int idx_response;        /* Index response.    */
	ssize_t output;          /* Bytes sent.        */
	ssize_t send_ret;        /* Ret send function  */
	int sock;                /* File Descript.     */
	uint64_t i;              /* Loop index.        */
	int cur_port_index;      /* Current port index */

	frame[0] = (WS_FIN | type);
	length   = (uint64_t)size;

	/* Split the size between octets. */
	if (length <= 125)
	{
		frame[1]        = length & 0x7F;
		idx_first_rData = 2;
	}

	/* Size between 126 and 65535 bytes. */
	else if (length >= 126 && length <= 65535)
	{
		frame[1]        = 126;
		frame[2]        = (length >> 8) & 255;
		frame[3]        = length & 255;
		idx_first_rData = 4;
	}

	/* More than 65535 bytes. */
	else
	{
		frame[1]        = 127;
		frame[2]        = (unsigned char)((length >> 56) & 255);
		frame[3]        = (unsigned char)((length >> 48) & 255);
		frame[4]        = (unsigned char)((length >> 40) & 255);
		frame[5]        = (unsigned char)((length >> 32) & 255);
		frame[6]        = (unsigned char)((length >> 24) & 255);
		frame[7]        = (unsigned char)((length >> 16) & 255);
		frame[8]        = (unsigned char)((length >> 8) & 255);
		frame[9]        = (unsigned char)(length & 255);
		idx_first_rData = 10;
	}

	/* Add frame bytes. */
	idx_response = 0;
	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(unsigned char) * (idx_first_rData + length + 1));
	response     = malloc(sizeof(unsigned char) * (idx_first_rData + length + 1));
	if (!response)
	{
		return (-1);
	}

	for (i = 0; i < idx_first_rData; i++)
	{
		response[i] = frame[i];
		idx_response++;
	}

	/* Add data bytes. */
	for (i = 0; i < length; i++)
	{
		response[idx_response] = msg[i];
		idx_response++;
	}

	response[idx_response] = '\0';
	output                 = SEND(fd, response, idx_response, get_client_index(fd));

	if (output != -1 && (broadcast == TRUE) )
	{
		pthread_mutex_lock(&mutex);

		cur_port_index = -1;
		for (i = 0; i < MAX_CLIENTS; i++)
		{
			if (client_socks[i].ws_client_sock == fd)
			{
				cur_port_index = client_socks[i].port_index;
				break;
			}
		}

		for (i = 0; i < MAX_CLIENTS; i++)
		{
			sock = client_socks[i].ws_client_sock;
			// printf("! sock? %d \n", sock);

			if (
				(sock > -1) && (sock != fd) &&
				(client_socks[i].port_index == cur_port_index) &&
				get_client_state(i) == WS_STATE_OPEN)
			{
				if ((send_ret = SEND(sock, response, idx_response, i)) != -1)
					output += send_ret;
				else
				{
					output = -1;
					break;
				}
			}
		}
		pthread_mutex_unlock(&mutex);
	}

	free(response);

	if (output == 0)
	{
		// SEND의 경우 정상적으로 모든 데이터가 보내지면 0을 반환한다. (send_all 구현 참조)
		// websocket frame사이즈와 어플리케이션 레벨에서 보내는 데이터는 사이즈가 다르기 때문에
		// 정상적으로 모두 보내진 경우 실제 전송된 사이즈가 아닌 어플이 보내기를 요청한 사이즈로 반환한다.


		// printf("! %s:%d >>> return #1  \n", __func__, __LINE__);
		return size;
	}
	else
	{
		// printf("! %s:%d >>> return #2  \n", __func__, __LINE__);
		return ((int)output);
	}
}

/**
 * @brief Sends a WebSocket text frame.
 *
 * @param fd         Target to be send.
 * @param msg        Text message to be send.
 * @param broadcast  Enable/disable broadcast (0-disable/anything-enable).
 *
 * @return Returns the number of bytes written, -1 if error.
 */
int ws_sendframe_txt(int fd, const char *msg, bool broadcast)
{
	return ws_sendframe(fd, msg, (uint64_t)strlen(msg), broadcast, WS_FR_OP_TXT);
}

/**
 * @brief Sends a WebSocket binary frame.
 *
 * @param fd         Target to be send.
 * @param msg        Binary message to be send.
 * @param size       Message size (in bytes).
 * @param broadcast  Enable/disable broadcast (0-disable/anything-enable).
 *
 * @return Returns the number of bytes written, -1 if error.
 */
int ws_sendframe_bin(int fd, const char *msg, uint64_t size, bool broadcast)
{
	// printf("%s:%d >>> broadcast? %s \n", __func__, __LINE__, broadcast == TRUE? "TRUE":"FALSE"); 
	return ws_sendframe(fd, msg, size, broadcast, WS_FR_OP_BIN);
}

/**
 * @brief For a given @p fd, gets the current state for
 * the connection, or -1 if invalid.
 *
 * @param fd Client fd.
 *
 * @return Returns the connection state or -1 if
 * invalid @p fd.
 *
 * @see WS_STATE_CONNECTING
 * @see WS_STATE_OPEN
 * @see WS_STATE_CLOSING
 * @see WS_STATE_CLOSED
 */
int ws_get_state(int fd)
{
	int idx;

	if ((idx = get_client_index(fd)) == -1)
		return (-1);

	return (get_client_state(idx));
}

/**
 * @brief Close the client connection for the given @p fd
 * with normal close code (1000) and no reason string.
 *
 * @param fd Client fd.
 *
 * @return Returns 0 on success, -1 otherwise.
 *
 * @note If the client did not send a close frame in
 * TIMEOUT_MS milliseconds, the server will close the
 * connection with error code (1002).
 */
int ws_close_client(int fd)
{
	unsigned char clse_code[2];
	int cc;
	int i;

	/* Check if fd belongs to a connected client. */
	if ((i = get_client_index(fd)) == -1)
		return (-1);

	/*
	 * Instead of using do_close(), we use this to avoid using
	 * msg_ctrl buffer from wfd and avoid a race condition
	 * if this is invoked asynchronously.
	 */
	cc           = WS_CLSE_NORMAL;
	clse_code[0] = (cc >> 8);
	clse_code[1] = (cc & 0xFF);
	if (ws_sendframe(CLI_SOCK(fd), (const char *)clse_code, sizeof(char) * 2, FALSE,
			WS_FR_OP_CLSE) < 0)
	{
		// 
		printf("An error has occurred while sending closing frame! %d\n", errno);

		return (-1);
	}

	/*
	 * Starts the timeout thread: if the client did not send
	 * a close frame in TIMEOUT_MS milliseconds, the server
	 * will close the connection with error code (1002).
	 */
	start_close_timeout(i);
	return (0);
}

/**
 * @brief Checks is a given opcode @p frame
 * belongs to a control frame or not.
 *
 * @param frame Frame opcode to be checked.
 *
 * @return Returns 1 if is a control frame, 0 otherwise.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
static inline int is_control_frame(int frame)
{
	return (
		frame == WS_FR_OP_CLSE || frame == WS_FR_OP_PING || frame == WS_FR_OP_PONG);
}

/**
 * @brief Do the handshake process.
 *
 * @param wfd Websocket Frame Data.
 * @param p_index Client port index.
 *
 * @return Returns 0 if success, a negative number otherwise.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
static int do_handshake(struct ws_frame_data *wfd, int p_index)
{
	// printf("! do_handshake >>> recv \n");
	char *response; /* Handshake response message. */
	char *p;        /* Last request line pointer.  */
	ssize_t n;      /* Read/Write bytes.           */

	/* Read the very first client message. */
	if ((n = RECV(wfd->sock, wfd->frm, sizeof(wfd->frm) - 1)) < 0)
		return (-1);

	// printf("do_handshake recv \n=======================================\n");
	// printf("wfd->frm? %s \n", wfd->frm);
	// printf("===========================================================\n");

	/* Advance our pointers before the first next_byte(). */
	p = strstr((const char *)wfd->frm, "\r\n\r\n");
	if (p == NULL)
	{
		// 
		// printf("An empty line with \\r\\n was expected!\n");
		return (-1);
	}
	wfd->amt_read = n;
	wfd->cur_pos  = (size_t)((ptrdiff_t)(p - (char *)wfd->frm)) + 4;

	// X-Forwarded-For 값 파싱
	char forwarded_ip[32] = "";
	{
		char * headers = (char *)wfd->frm;
		char * s = strstr(headers, WS_HS_FORWARDED);

		if (s)
		{
			char * start_p = strstr(s, ": ");
			if (start_p)
			{
				start_p += 2; // strlen(": ");
				char * endp = strstr(start_p, "\r\n");

				if (endp)
				{
					strncpy(forwarded_ip, start_p, min(sizeof(forwarded_ip), endp - start_p));
				}
			}
		}

	}

	/* Get response. */
	if (get_handshake_response((char *)wfd->frm, &response) < 0)
	{
		// 
		// printf("Cannot get handshake response, request was: %s\n", wfd->frm);
		return (-1);
	}

	/* Valid request. */
	// printf("Handshaked, response: \n"
	// 	  "------------------------------------\n"
	// 	  "%s"
	// 	  "------------------------------------\n",
	// 	response);


	/* Send handshake. */
	if (SEND(wfd->sock, response, strlen(response),
		get_client_index(wfd->sock)) < 0)
	{
		free(response);
		// 
		// printf("As error has occurred while handshaking!\n");
		return (-1);
	}


	// 
	// printf("forwarded_ip: %s\n", forwarded_ip);
	/* Trigger events and clean up buffers. */
	g_ports[p_index].events.onopen(forwarded_ip, CLI_SOCK(wfd->sock));
	free(response);
	return (0);
}

/**
 * @brief Sends a close frame, accordingly with the @p close_code
 * or the message inside @p wfd.
 *
 * @param wfd Websocket Frame Data.
 * @param close_code Websocket close code.
 *
 * @return Returns 0 if success, a negative number otherwise.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
static int do_close(struct ws_frame_data *wfd, int close_code)
{
	int cc; /* Close code.           */

	/* If custom close-code. */
	if (close_code != -1)
	{
		cc = close_code;
		goto custom_close;
	}

	/* If empty or have a close reason, just re-send. */
	if (wfd->frame_size == 0 || wfd->frame_size > 2)
		goto send;

	/* Parse close code and check if valid, if not, we issue an protocol error. */
	if (wfd->frame_size == 1)
		cc = wfd->msg_ctrl[0];
	else
		cc = ((int)wfd->msg_ctrl[0]) << 8 | wfd->msg_ctrl[1];

	/* Check if it's not valid, if so, we send a protocol error (1002). */
	if ((cc < 1000 || cc > 1003) && (cc < 1007 || cc > 1011) &&
		(cc < 3000 || cc > 4999))
	{
		cc = WS_CLSE_PROTERR;

	custom_close:
		wfd->msg_ctrl[0] = (cc >> 8);
		wfd->msg_ctrl[1] = (cc & 0xFF);

		if (ws_sendframe(CLI_SOCK(wfd->sock), (const char *)wfd->msg_ctrl,
				sizeof(char) * 2, FALSE, WS_FR_OP_CLSE) < 0)
		{
			// 
			// printf("An error has occurred while sending closing frame!\n");
			return (-1);
		}
		return (0);
	}

	/* Send the data inside wfd->msg_ctrl. */
send:
	if (ws_sendframe(CLI_SOCK(wfd->sock), (const char *)wfd->msg_ctrl,
			wfd->frame_size, FALSE, WS_FR_OP_CLSE) < 0)
	{
		// 
		// printf("An error has occurred while sending closing frame!\n");
		return (-1);
	}
	return (0);
}

/**
 * @brief Send a pong frame in response to a ping frame.
 *
 * Accordingly to the RFC, a pong frame must have the same
 * data payload as the ping frame, so we just send a
 * ordinary frame with PONG opcode.
 *
 * @param wfd Websocket frame data.
 *
 * @return Returns 0 if success and a negative number
 * otherwise.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
static int do_pong(struct ws_frame_data *wfd, uint64_t frame_size)
{
	if (ws_sendframe(CLI_SOCK(wfd->sock), (const char *)wfd->msg_ctrl, frame_size,
			FALSE, WS_FR_OP_PONG) < 0)
	{
		wfd->error = 1;
		// printf("An error has occurred while ponging!\n");
		return (-1);
	}
	return (0);
}

/**
 * @brief Read a chunk of bytes and return the next byte
 * belonging to the frame.
 *
 * @param wfd Websocket Frame Data.
 *
 * @return Returns the byte read, or -1 if error.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
static inline int next_byte(struct ws_frame_data *wfd)
{
	ssize_t n;
	// printf("! next_byte >>> recv, wfd->frm sz? %d, wfd->amt_read? %d, wfd->cur_pos? %d  \n", sizeof(wfd->frm), wfd->amt_read, wfd->cur_pos);
	/* If empty or full. */
	if (wfd->cur_pos == 0 || wfd->cur_pos == wfd->amt_read)
	{
		// printf("!!!!!!!!!		recv \n");
		if ((n = RECV(wfd->sock, wfd->frm, sizeof(wfd->frm))) <= 0)
		{
			wfd->error = 1;
			// 
			// printf("An error has occurred while trying to read next byte\n");
			return (-1);
		}
		// printf("! next_byte >>> %x, %s \n", wfd->frm[wfd->cur_pos], wfd->frm);
		
#if 0
		{
			int i;
			for (i=0; i<2048; i++)
				printf("%c", wfd->frm[i]);

			printf ("\n\n");
			for (i=0; i<2048; i++)
				printf("%x", wfd->frm[i]);

		}
#endif 


		wfd->amt_read = (size_t)n;
		wfd->cur_pos  = 0;
	}


	// printf("! %s:%d >>> wfd->cur_pos? %d, %c, %x \n", __func__, __LINE__, wfd->cur_pos, wfd->frm[wfd->cur_pos], wfd->frm[wfd->cur_pos]);
	return (wfd->frm[wfd->cur_pos++]);
}

/**
 * @brief Skips @p frame_size bytes of the current frame.
 *
 * @param wfd Websocket Frame Data.
 * @param frame_size Amount of bytes to be skipped.
 *
 * @return Returns 0 if success, a negative number
 * otherwise.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
static int skip_frame(struct ws_frame_data *wfd, uint64_t frame_size)
{

	// printf("! skip_frame \n");
	uint64_t i;
	for (i = 0; i < frame_size; i++)
	{
		if (next_byte(wfd) == -1)
		{
			wfd->error = 1;
			return (-1);
		}
	}
	return (0);
}

/**
 * @brief Reads the current frame isolating data from control frames.
 * The parameters are changed in order to reflect the current state.
 *
 * @param wfd Websocket Frame Data.
 * @param opcode Frame opcode.
 * @param buf Buffer to be written.
 * @param frame_length Length of the current frame.
 * @param frame_size Total size of the frame (considering CONT frames)
 *                   read until the moment.
 * @param msg_idx Message index, reflects the current buffer pointer state.
 * @param masks Masks vector.
 * @param is_fin Is FIN frame indicator.
 *
 * @return Returns 0 if success, a negative number otherwise.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
static int read_frame(struct ws_frame_data *wfd,
	int opcode,
	unsigned char **buf,
	uint64_t *frame_length,
	uint64_t *frame_size,
	uint64_t *msg_idx,
	uint8_t *masks,
	int is_fin)
{
	// printf("! read_frame >>> frame_length? %d, is_fin? %d  \n", *frame_length, is_fin);


	unsigned char *tmp; /* Tmp message.     */
	unsigned char *msg; /* Current message. */
	int cur_byte;       /* Curr byte read.  */
	uint64_t i;         /* Loop index.      */

	msg = *buf;


	/* Decode masks and length for 16-bit messages. */
	if (*frame_length == 126)
		*frame_length = (((uint64_t)next_byte(wfd)) << 8) | next_byte(wfd);

	/* 64-bit messages. */
	else if (*frame_length == 127)
	{
		*frame_length =
			(((uint64_t)next_byte(wfd)) << 56) | /* frame[2]. */
			(((uint64_t)next_byte(wfd)) << 48) | /* frame[3]. */
			(((uint64_t)next_byte(wfd)) << 40) | (((uint64_t)next_byte(wfd)) << 32) |
			(((uint64_t)next_byte(wfd)) << 24) | (((uint64_t)next_byte(wfd)) << 16) |
			(((uint64_t)next_byte(wfd)) << 8) |
			(((uint64_t)next_byte(wfd))); /* frame[9]. */
	}

	*frame_size += *frame_length;

	// printf("! read_frame >>> frame_length? %d, is_fin? %d, frame_size? %ld  \n", *frame_length, is_fin, frame_size);

	/*
	 * Check frame size
	 *
	 * We need to limit the amount supported here, since if
	 * we follow strictly to the RFC, we have to allow 2^64
	 * bytes. Also keep in mind that this is still true
	 * for continuation frames.
	 */
	if (*frame_size > MAX_FRAME_LENGTH)
	{
		// 
		// printf("Current frame from client %d, exceeds the maximum\n"
		// 	  "amount of bytes allowed (%" PRId64 "/%d)!",
		// 	wfd->sock, *frame_size + *frame_length, MAX_FRAME_LENGTH);

		wfd->error = 1;
		return (-1);
	}

	/* Read masks. */
	masks[0] = next_byte(wfd);
	masks[1] = next_byte(wfd);
	masks[2] = next_byte(wfd);
	masks[3] = next_byte(wfd);

	/*
	 * Abort if error.
	 *
	 * This is tricky: we may have multiples error codes from the
	 * previous next_bytes() calls, but, since we're only setting
	 * variables and flags, there is no major issue in setting
	 * them wrong _if_ we do not use their values, thing that
	 * we do here.
	 */
	if (wfd->error)
		return (-1);

	/*
	 * Allocate memory.
	 *
	 * The statement below will allocate a new chunk of memory
	 * if msg is NULL with size total_length. Otherwise, it will
	 * resize the total memory accordingly with the message index
	 * and if the current frame is a FIN frame or not, if so,
	 * increment the size by 1 to accommodate the line ending \0.
	 */
	if (*frame_length > 0)
	{
		if (!is_control_frame(opcode))
		{
			tmp = __realloc(
				msg, sizeof(unsigned char) * (*msg_idx + *frame_length + is_fin));
			if (!tmp)
			{
				// 
				// printf("Cannot allocate memory, requested: % " PRId64 "\n",
				// 	(*msg_idx + *frame_length + is_fin));

				wfd->error = 1;
				return (-1);
			}
			msg  = tmp;
			*buf = msg;
		}

		/* Copy to the proper location. */
		// printf("! %s:%d >>> frame_length? %ld \n", __func__, __LINE__, *frame_length);

		for (i = 0; i < *frame_length; i++, (*msg_idx)++)
		{
			/* We were able to read? .*/
			cur_byte = next_byte(wfd);
			// printf("[%x, %d]", cur_byte, cur_byte);

			if (cur_byte == -1)
				return (-1);
			 
			// printf("[%x]", cur_byte);
			msg[*msg_idx] = cur_byte ^ masks[i % 4];

			// if (i < 12)
			// 	printf("%02x", msg[i]);
			// else 
			// 	printf("%c", msg[i]);
		}
	}

	/* If we're inside a FIN frame, lets... */
	if (is_fin && *frame_size > 0)
	{
		/* Increase memory if our FIN frame is of length 0. */
		if (!*frame_length && !is_control_frame(opcode))
		{
			tmp = __realloc(msg, sizeof(unsigned char) * (*msg_idx + 1));
			if (!tmp)
			{
				// printf("Cannot allocate memory, requested: %" PRId64 "\n",
				// 	(*msg_idx + 1));

				wfd->error = 1;
				return (-1);
			}
			msg  = tmp;
			*buf = msg;
		}
		msg[*msg_idx] = '\0';
	}

	return (0);
}

/**
 * @brief Reads the next frame, whether if a TXT/BIN/CLOSE
 * of arbitrary size.
 *
 * @param wfd Websocket Frame Data.
 * @param idx Websocket connection index.
 *
 * @return Returns 0 if success, a negative number otherwise.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
static int next_frame(struct ws_frame_data *wfd, int idx)
{
	unsigned char *msg_data; /* Data frame.                */
	unsigned char *msg_ctrl; /* Control frame.             */
	uint8_t masks_data[4];   /* Masks data frame array.    */
	uint8_t masks_ctrl[4];   /* Masks control frame array. */
	uint64_t msg_idx_data;   /* Current msg index.         */
	uint64_t msg_idx_ctrl;   /* Current msg index.         */
	uint64_t frame_length;   /* Frame length.              */
	uint64_t frame_size;     /* Current frame size.        */
	uint8_t opcode;          /* Frame opcode.              */
	uint8_t is_fin;          /* Is FIN frame flag.         */
	uint8_t mask;            /* Mask.                      */
	int cur_byte;            /* Current frame byte.        */

	msg_data        = NULL;
	msg_ctrl        = wfd->msg_ctrl;
	is_fin          = 0;
	frame_length    = 0;
	frame_size      = 0;
	msg_idx_data    = 0;
	msg_idx_ctrl    = 0;
	wfd->frame_size = 0;
	wfd->frame_type = -1;
	wfd->msg        = NULL;

	// printf("! next_frame \n");

	/* Read until find a FIN or a unsupported frame. */
	do
	{
		/*
		 * Obs: next_byte() can return error if not possible to read the
		 * next frame byte, in this case, we return an error.
		 *
		 * However, please note that this check is only made here and in
		 * the subsequent next_bytes() calls this also may occur too.
		 * wsServer is assuming that the client only create right
		 * frames and we will do not have disconnections while reading
		 * the frame but just when waiting for a frame.
		 */
		cur_byte = next_byte(wfd);
		if (cur_byte == -1)
			return (-1);

		is_fin = (cur_byte & 0xFF) >> WS_FIN_SHIFT;
		opcode = (cur_byte & 0xF);

		/*
		 * Check for RSV field.
		 *
		 * Since wsServer do not negotiate extensions if we receive
		 * a RSV field, we must drop the connection.
		 */
		if (cur_byte & 0x70)
		{
			// 
			// printf("RSV is set while wsServer do not negotiate extensions!\n");
			wfd->error = 1;
			break;
		}

		/*
		 * Check if the current opcode makes sense:
		 * a) If we're inside a cont frame but no previous data frame
		 *
		 * b) If we're handling a data-frame and receive another data
		 *    frame. (it's expected to receive only CONT or control
		 *    frames).
		 *
		 * It is worth to note that in a), we do not need to check
		 * if the previous frame was FIN or not: if was FIN, an
		 * on_message event was triggered and this function returned;
		 * so the only possibility here is a previous non-FIN data
		 * frame, ;-).
		 */
		if ((wfd->frame_type == -1 && opcode == WS_FR_OP_CONT) ||
			(wfd->frame_type != -1 && !is_control_frame(opcode) &&
				opcode != WS_FR_OP_CONT))
		{
			// printf("Unexpected frame was received!, opcode: %d, previous: %d\n",
			// 	opcode, wfd->frame_type);
			wfd->error = 1;
			break;
		}

		/* Check if one of the valid opcodes. */
		if (opcode == WS_FR_OP_TXT || opcode == WS_FR_OP_BIN ||
			opcode == WS_FR_OP_CONT || opcode == WS_FR_OP_PING ||
			opcode == WS_FR_OP_PONG || opcode == WS_FR_OP_CLSE)
		{
			/*
			 * Check our current state: if CLOSING, we only accept close
			 * frames.
			 *
			 * Since the server may, at any time, asynchronously, asks
			 * to close the client connection, we should terminate
			 * immediately.
			 */
			if (get_client_state(idx) == WS_STATE_CLOSING && opcode != WS_FR_OP_CLSE)
			{
				// printf(
				// 	"Unexpected frame received, expected CLOSE (%d), received: (%d)",
				// 	WS_FR_OP_CLSE, opcode);
				wfd->error = 1;
				break;
			}

			/* Only change frame type if not a CONT frame. */
			if (opcode != WS_FR_OP_CONT && !is_control_frame(opcode))
				wfd->frame_type = opcode;

			mask         = next_byte(wfd);
			frame_length = mask & 0x7F;
			frame_size   = 0;
			msg_idx_ctrl = 0;

			/*
			 * We should deny non-FIN control frames or that have
			 * more than 125 octets.
			 */
			if (is_control_frame(opcode) && (!is_fin || frame_length > 125))
			{
				// printf("Control frame bigger than 125 octets or not a FIN frame!\n");
				wfd->error = 1;
				break;
			}

			/* Normal data frames. */
			if (opcode == WS_FR_OP_TXT || opcode == WS_FR_OP_BIN ||
				opcode == WS_FR_OP_CONT)
			{
				// printf("! %s:%d >>> read_frame \n", __func__, __LINE__);
				read_frame(wfd, opcode, &msg_data, &frame_length, &wfd->frame_size,
					&msg_idx_data, masks_data, is_fin);
			}

			/*
			 * We _never_ send a PING frame, so it's not expected to receive a PONG
			 * frame. However, the specs states that a client could send an
			 * unsolicited PONG frame. The server just have to ignore the
			 * frame.
			 *
			 * The skip amount will always be 4 (masks vector size) + frame size
			 */
			else if (opcode == WS_FR_OP_PONG)
			{
				skip_frame(wfd, 4 + frame_length);
				is_fin = 0;
				continue;
			}

			/* We should answer to a PING frame as soon as possible. */
			else if (opcode == WS_FR_OP_PING)
			{
				// printf("! %s:%d >>> read_frame \n", __func__, __LINE__);
				if (read_frame(wfd, opcode, &msg_ctrl, &frame_length, &frame_size,
						&msg_idx_ctrl, masks_ctrl, is_fin) < 0)
					break;

				if (do_pong(wfd, frame_size) < 0)
					break;

				/* Quick hack to keep our loop. */
				is_fin = 0;
			}

			/* We interrupt the loop as soon as we find a CLOSE frame. */
			else
			{
				// printf("! %s:%d >>> read_frame \n", __func__, __LINE__);
				if (read_frame(wfd, opcode, &msg_ctrl, &frame_length, &frame_size,
						&msg_idx_ctrl, masks_ctrl, is_fin) < 0)
					break;

				/* Since we're aborting, we can scratch the 'data'-related
				 * vars here. */
				wfd->frame_size = frame_size;
				wfd->frame_type = WS_FR_OP_CLSE;
				free(msg_data);
				return (0);
			}
		}

		/* Anything else (unsupported frames). */
		else
		{
			// printf("Unsupported frame opcode: %d\n", opcode);
			/* We should consider as error receive an unknown frame. */
			wfd->frame_type = opcode;
			wfd->error      = 1;
		}

	} while (!is_fin && !wfd->error);

	/* Check for error. */
	if (wfd->error)
	{
		free(msg_data);
		wfd->msg = NULL;
		return (-1);
	}

	wfd->msg = msg_data;
	return (0);
}

/**
 * @brief Establishes to connection with the client and trigger
 * events when occurs one.
 *
 * @param vsock Client connection index.
 *
 * @return Returns @p vsock.
 *
 * @note This will be run on a different thread.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
static void *ws_establishconnection(void *vsock)
{

	// printf("! ws_establishconnection \n");
	struct ws_frame_data wfd; /* WebSocket frame data.   */
	int connection_index;     /* Client connect. index.  */
	int clse_thrd;            /* Time-out close thread.  */
	int p_index;              /* Port list index.        */
	int sock;                 /* File descriptor.        */

	connection_index = (int)(intptr_t)vsock;
	sock             = client_socks[connection_index].ws_client_sock;
	p_index          = client_socks[connection_index].port_index;

	/* Prepare frame data. */
	memset(&wfd, 0, sizeof(wfd));
	wfd.sock = sock;

	/* Do handshake. */
	if (do_handshake(&wfd, p_index) < 0)
	{
		// printf("! %s:%d >>> goto closed \n", __func__, __LINE__);
		goto closed;
	}

	/* Change state. */
	set_client_state(connection_index, WS_STATE_OPEN);

	// printf("! %s:%d >>> WS >>> WS_STATE_OPEN \n", __func__, __LINE__);

	/* Read next frame until client disconnects or an error occur. */
	while (next_frame(&wfd, connection_index) >= 0)
	{
		/* Text/binary event. */
		if ((wfd.frame_type == WS_FR_OP_TXT || wfd.frame_type == WS_FR_OP_BIN) &&
			!wfd.error)
		{
			// printf("! %s:%d >>> onmessage >>> wfd.frame_type? %d \n", __func__, __LINE__, wfd.frame_type);

			g_ports[p_index].events.onmessage(
				sock, wfd.msg, wfd.frame_size, wfd.frame_type);
		}

		/* Close event. */
		else if (wfd.frame_type == WS_FR_OP_CLSE && !wfd.error)
		{


			// printf("! %s:%d >>> WS_FR_OP_CLSE \n", __func__, __LINE__);
			/*		
			 * We only send a CLOSE frame once, if we're already
			 * in CLOSING state, there is no need to send.
			 */
			if (get_client_state(connection_index) != WS_STATE_CLOSING)
			{
				set_client_state(connection_index, WS_STATE_CLOSING);

				/* We only send a close frameSend close frame */
				do_close(&wfd, -1);
			}

			free(wfd.msg);
			break;
		}

		free(wfd.msg);
	}

	/*
	 * on_close events always occur, whether for client closure
	 * or server closure, as the server is expected to
	 * always know when the client disconnects.
	 */
	g_ports[p_index].events.onclose(sock);

closed:
	clse_thrd = client_socks[connection_index].close_thrd;

	/* Wait for timeout thread if necessary. */
	if (clse_thrd)
	{
		pthread_cond_signal(&client_socks[connection_index].cnd_state_close);
		pthread_join(client_socks[connection_index].thrd_tout, NULL);
	}

	/* Close connectin properly. */
	if (get_client_state(connection_index) != WS_STATE_CLOSED)
		close_client(connection_index, sock);

	return (vsock);
}

/**
 * @brief Main loop that keeps accepting new connections.
 *
 * @param data Accept thread data: sock and port index.
 *
 * @return Returns @p data.
 *
 * @note This may be run on a different thread.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
static void *_ws_accept(void *data)
{
	// struct ws_accept *accept_data; /* Accept thread data.    */
	WS_ACCEPT *accept_data; /* Accept thread data.    */
	
	struct sockaddr_in client;     /* Client.                */
	pthread_t client_thread;       /* Client thread.         */
	int connection_index;          /* Free connection slot.  */
	int new_sock;                  /* New opened connection. */
	int len;                       /* Length of sockaddr.    */
	int i;                         /* Loop index.            */

	connection_index = 0;
	accept_data      = data;
	len              = sizeof(struct sockaddr_in);

	g_websocket_run_state = 1;
	while (g_websocket_run_state)
	{
		/* Accept. */
		new_sock =
			accept(accept_data->sock, (struct sockaddr *)&client, (socklen_t *)&len);

		/* Adds client socket to socks list. */
		pthread_mutex_lock(&mutex);
		for (i = 0; i < MAX_CLIENTS; i++)
		{
			// printf("! client_socks[%d].ws_client_sock? %d \n", i, client_socks[i].ws_client_sock);
			if (client_socks[i].ws_client_sock == -1)		// close_client
			{
				client_socks[i].ws_client_sock = new_sock;
				client_socks[i].port_index  = accept_data->port_index;
				client_socks[i].state       = WS_STATE_CONNECTING;
				client_socks[i].close_thrd  = FALSE;
				connection_index            = i;

#if 0
				if (pthread_mutex_init(&client_socks[i].mtx_state, NULL))
					panic("! Error on allocating close mutex");
				if (pthread_cond_init(&client_socks[i].cnd_state_close, NULL))
					panic("! Error on allocating condition var\n");
				if (pthread_mutex_init(&client_socks[i].mtx_snd, NULL))
					panic("! Error on allocating send mutex");
#else 
				pthread_mutex_init(&client_socks[i].mtx_state, NULL);
				pthread_cond_init(&client_socks[i].cnd_state_close, NULL);
				pthread_mutex_init(&client_socks[i].mtx_snd, NULL);
#endif 
				break;
			}
		}
		pthread_mutex_unlock(&mutex);

		/* Client socket added to socks list ? */
		if (i != MAX_CLIENTS)
		{
			if (pthread_create(&client_thread, NULL, ws_establishconnection,
					(void *)(intptr_t)connection_index))
			{
				
				// panic("Could not create the client thread!");
			}

			pthread_detach(client_thread);
		}
		else
			close_socket(new_sock);



		sleep(1);
	}
	free(data);

	return (data);
}

/**
 * @brief Main loop for the server.
 *
 * @param evs  Events structure.
 * @param port Server port.
 * @param thread_loop If any value other than zero, runs
 *                    the accept loop in another thread
 *                    and immediately returns. If 0, runs
 *                    in the same thread and blocks execution.
 *
 * @return If @p thread_loop != 0, returns 0. Otherwise, never
 * returns.
 *
 * @note Note that this function can be called multiples times,
 * from multiples different threads (depending on the @ref MAX_PORTS)
 * value. Each call _should_ have a different port and can have
 * different events configured.
 */


typedef struct _IINI_PRIV_S {
	int sock;       /**< Socket number.               */
	int port_index; /**< Port index in the port list. */
} IINI_PRIV_S;


int ws_socket(struct ws_events *evs, uint16_t port, int thread_loop)
{
	// struct ws_accept *accept_data = NULL; /* Accept thread data.    */



	
	struct sockaddr_in server;     /* Server.                */
	pthread_t accept_thread;       /* Accept thread.         */
	int reuse;                     /* Socket option.         */

	/* Checks if the event list is a valid pointer. */
#if 1
	// if (evs == NULL)
	// 	panic("Invalid event list!");
#else 
	if (evs->onopen == NULL || evs->onclose == NULL || evs->onmessage == NULL)
		panic("Invalid event list!");
#endif 



#if 1
	IINI_PRIV_S *accept_data;
	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(IINI_PRIV_S));
	accept_data = (IINI_PRIV_S *)malloc(sizeof(IINI_PRIV_S));
	
	// if (accept_data == NULL || !accept_data)
	// else 
#else 
#endif 





	pthread_mutex_lock(&mutex);
	if (g_port_index >= MAX_PORTS)
	{
		pthread_mutex_unlock(&mutex);
		panic("too much websocket g_ports opened !");
	}
	accept_data->port_index = g_port_index;
	g_port_index++;
	pthread_mutex_unlock(&mutex);

	/* Copy events. */
	memcpy(&g_ports[accept_data->port_index].events, evs, sizeof(struct ws_events));
	g_ports[accept_data->port_index].port_number = port;


	int sock;
	/* Create socket. */
	// accept_data->sock = socket(AF_INET, SOCK_STREAM, 0);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	// if (accept_data->sock < 0)
	// 	printf("Could not create socket, %d \n", accept_data->sock);

	accept_data->sock = sock;
	/* Reuse previous address. */



	reuse = 1;
	// if (setsockopt(accept_data->sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse,
	// 		sizeof(reuse)) < 0)
	// if (setsockopt(accept_data->sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse,

#if 1
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse,
			4) < 0)
#else
	if (setsockopt(accept_data->sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse,
			sizeof(reuse)) < 0)
#endif
	{
		panic("setsockopt(SO_REUSEADDR) failed");
	}



	/* Prepare the sockaddr_in structure. */
	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_family      = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port        = htons(port);



	/* Bind. */
	// if (bind(accept_data->sock, (struct sockaddr *)&server, sizeof(server)) < 0)
	int ret = bind(sock, (struct sockaddr *)&server, sizeof(server)); 

	if (ret < 0)
	{
		panic("Bind failed");
	}

	/* Listen. */
	ret = listen(sock, 8);

	/* Wait for incoming connections. */
	// printf("Waiting for incoming connections...\n");
	memset(client_socks, -1, sizeof(client_socks));

	/* Accept connections. */
	if (!thread_loop)
		_ws_accept(accept_data);
	else
	{
		if (pthread_create(&accept_thread, NULL, _ws_accept, accept_data))
			panic("Could not create the client thread!");
		pthread_detach(accept_thread);
	}

	// if (accept_data->sock >= 0)
	if (sock >= 0)
	{
		// printf("ws_socket close!==========================================================================_+_+_+[%s:%d]\n", __FILE__, __LINE__);
		close(sock);
		sock = -1;
	}

	return (0);
}

void ws_socket_stop() {
	g_websocket_run_state = 0;
}

#ifdef AFL_FUZZ
/**
 * @brief WebSocket fuzzy test routine
 *
 * @param evs  Events structure.
 *
 * @param file File to be read.
 *
 * @return Returns 0, or crash.
 *
 * @note This is a 'fuzzy version' of the function @ref ws_socket.
 * This routine do not listen to any port nor accept multiples
 * connections. It is intended to read a stream of frames through a
 * file and process it as if they are coming from a socket.
 *
 * This behavior enables us to test wsServer against fuzzers, like
 * AFL, and see if it crashes, hangs or behaves normally, even under
 * weird conditions.
 */
int ws_file(struct ws_events *evs, const char *file)
{
	int sock;
	sock = open(file, O_RDONLY);
	if (sock < 0)
		panic("Invalid file\n");

	/* Copy events. */
	memcpy(&g_ports[0].events, evs, sizeof(struct ws_events));
	g_ports[0].port_number = 0;

	/* Clear client socks list. */
	memset(client_socks, -1, sizeof(client_socks));

	/* Set client settings. */
	client_socks[0].client_sock = sock;
	client_socks[0].port_index  = 0;
	client_socks[0].state       = WS_STATE_CONNECTING;
	client_socks[0].close_thrd  = FALSE;

	/* Initialize mutexes. */
	if (pthread_mutex_init(&client_socks[0].mtx_state, NULL))
		panic("!! Error on allocating close mutex");
	if (pthread_cond_init(&client_socks[0].cnd_state_close, NULL))
		panic("Error on allocating condition var\n");

	ws_establishconnection((void *)(intptr_t)0);
	return (0);
}
#endif
