
/**
 * @dir include/
 * @brief wsServer include directory
 *
 * @file ws.h
 * @brief wsServer constants and functions.
 */

#ifndef WS_H
#define WS_H

	#ifndef bool
	#define bool int
	#endif

#ifdef __cplusplus
extern "C" {
#endif

	// #include <stdbool.h>
	#include <stdint.h>
	#include <inttypes.h>

	/**
	 * @name Global configurations
	 */
	/**@{*/
	/**
	 * @brief Max clients connected simultaneously.
	 */
	#define MAX_CLIENTS    8

	/**
	 * @brief Max number of `ws_server` instances running
	 * at the same time.
	 */
	#define MAX_PORTS      8
	/**@}*/

	/**
	 * @name Key and message configurations.
	 */
	/**@{*/
	/**
	 * @brief Message buffer length.
	 */
	#define MESSAGE_LENGTH 2048
	/**
	 * @brief Maximum frame/message length.
	 */
	#define MAX_FRAME_LENGTH (16*1024*1024)
	/**
	 * @brief WebSocket key length.
	 */
	#define WS_KEY_LEN     24
	/**
	 * @brief Magic string length.
	 */
	#define WS_MS_LEN      36
	/**
	 * @brief Accept message response length.
	 */
	#define WS_KEYMS_LEN   (WS_KEY_LEN + WS_MS_LEN)
	/**
	 * @brief Magic string.
	 */
	#define MAGIC_STRING   "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
	/**@}*/

	/**
	 * @name Handshake constants.
	 */
	/**@{*/
	/**
	 * @brief Alias for 'Sec-WebSocket-Key'.
	 */
	#define WS_HS_REQ      "Sec-WebSocket-Key"

	#define WS_HS_FORWARDED	"X-Forwarded-For"

	/**
	 * @brief Handshake accept message length.
	 */
	#define WS_HS_ACCLEN   130

	/**
	 * @brief Handshake accept message.
	 */
	#define WS_HS_ACCEPT                       \
		"HTTP/1.1 101 Switching Protocols\r\n" \
		"Upgrade: websocket\r\n"               \
		"Connection: Upgrade\r\n"              \
		"Sec-WebSocket-Accept: "
	/**@}*/

	/**
	 * @name Frame types.
	 */
	/**@{*/
	/**
	 * @brief Frame FIN.
	 */
	#define WS_FIN      128

	/**
	 * @brief Frame FIN shift
	 */
	#define WS_FIN_SHIFT  7

	/**
	 * @brief Continuation frame.
	 */
	#define WS_FR_OP_CONT 0

	/**
	 * @brief Text frame.
	 */
	#define WS_FR_OP_TXT  1

	/**
	 * @brief Binary frame.
	 */
	#define WS_FR_OP_BIN  2

	/**
	 * @brief Close frame.
	 */
	#define WS_FR_OP_CLSE 8

	/**
	 * @brief Ping frame.
	 */
	#define WS_FR_OP_PING 0x9

	/**
	 * @brief Pong frame.
	 */
	#define WS_FR_OP_PONG 0xA

	/**
	 * @brief Unsupported frame.
	 */
	#define WS_FR_OP_UNSUPPORTED 0xF
	/**@}*/

	/**
	 * @name Close codes
	 */
	/**@{*/
	/**
	 * @brief Normal close
	 */
	#define WS_CLSE_NORMAL  1000
	/**
	 * @brief Protocol error
	 */
	#define WS_CLSE_PROTERR 1002
	/**@}*/

	/**
	 * @name Connection states
	 */
	/**@{*/
	/**
	 * @brief Connection not established yet.
	 */
	#define WS_STATE_CONNECTING 0
	/**
	 * @brief Communicating.
	 */
	#define WS_STATE_OPEN       1
	/**
	 * @brief Closing state.
	 */
	#define WS_STATE_CLOSING    2
	/**
	 * @brief Closed.
	 */
	#define WS_STATE_CLOSED     3
	/**@}*/

	/**
	 * @name Timeout util
	 */
	/**@{*/
	/**
	 * @brief Nanoseconds macro converter
	 */
	#define MS_TO_NS(x) ((x)*1000000)
	/**
	 * @brief Timeout in milliseconds.
	 */
	#define TIMEOUT_MS (500)
	/**@}*/

	/**
	 * @name Handshake constants.
	 */
	/**@{*/
	/**
	 * @brief Debug
	 */
	// #ifndef WS_DEBUG
	// 	#ifdef VERBOSE_MODE
	// 	#define WS_DEBUG(...) fprintf(stderr, __VA_ARGS__)
	// 	#else
	// 	#define WS_DEBUG(...)
	// 	#endif
	// #endif
	/**@}*/

	#ifndef AFL_FUZZ
	#define CLI_SOCK(sock) (sock)
	#define SEND(fd,buf,len,idx) send_all((fd), (buf), (len), MSG_NOSIGNAL, (idx))
	#define RECV(fd,buf,len) recv((fd), (buf), (len), 0)
	#else
	#define CLI_SOCK(sock) (fileno(stdout))
	#define SEND(fd,buf,len,idx) write(fileno(stdout), (buf), (len))
	#define RECV(fd,buf,len) read((fd), (buf), (len))
	#endif

	/**
	 * @brief events Web Socket events types.
	 */
	struct ws_events
	{
		/**
		 * @brief On open event, called when a new client connects.
		 */
		void (*onopen)(const char * forwarded_ip, int fd);

		/**
		 * @brief On close event, called when a client disconnects.
		 */
		void (*onclose)(int);

		/**
		 * @brief On message event, called when a client sends a text
		 * or binary message.
		 */
		void (*onmessage)(int, const unsigned char *, uint64_t, int);
		void (*test)(void);
		int val;
	};

	/* Forward declarations. */
	extern int get_handshake_accept(char *wsKey, unsigned char **dest);
	extern int get_handshake_response(char *hsrequest, char **hsresponse);
	extern char *ws_getaddress(int fd);
	extern int ws_sendframe(int fd, const char *msg, uint64_t size, bool broadcast, int type);
	extern int ws_sendframe_txt(int fd, const char *msg, bool broadcast);
	extern int ws_sendframe_bin(int fd, const char *msg, uint64_t size,
		bool broadcast);
	extern int ws_get_state(int fd);
	extern int ws_close_client(int fd);
	extern int ws_socket(struct ws_events *evs, uint16_t port, int thread_loop);
	extern void ws_socket_stop();

#ifdef AFL_FUZZ
	extern int ws_file(struct ws_events *evs, const char *file);
#endif

#ifdef __cplusplus
}
#endif

#endif /* WS_H */
