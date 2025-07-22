/*
 * websocket.c
 * 
 *****************************************************************************/
#include "common.h"
#include "lib/video_pool.h"
#include "websocket.h"
#include "websocket_protocol.h"

#define MG_ENABLE_THREADS 1
#define MG_ENABLE_HTTP_WEBSOCKET 1
#include "lib/mongoose.h"

struct mg_mgr_userdata {
	int port;
	int running;
};
struct mg_mgr_userdata *g_mgr_userdata = NULL;

struct mg_connection_userdata {
	int ch;
	int send_video;
	int skip_frame;
	vp_h vh;
};

int start_video(void *arg, WS_PACKET *packet)
{
	struct mg_connection *nc = (struct mg_connection *)arg;
	struct mg_connection_userdata *ud = (struct mg_connection_userdata *)nc->user_data;

	if(ud->send_video == 0) {
		json_unpack(packet->param, "{s:i}", "ch", &ud->ch);
		ud->vh = video_pool_handle_init(ud->ch);
		ud->send_video = 1;
		ud->skip_frame = 0;
	}

	return 200;
}

int stop_video(void *arg, WS_PACKET *packet)
{
	struct mg_connection *nc = (struct mg_connection *)arg;
	struct mg_connection_userdata *ud = (struct mg_connection_userdata *)nc->user_data;

	if(ud->send_video == 1) {
		video_pool_handle_deinit(ud->vh);
		ud->send_video = 0;
	}
	
	return 200;
}

int ping_pong(void *arg, WS_PACKET *packet)
{
	return 555;
}

static int proc_protocol(struct mg_connection *nc, struct websocket_message *wm)
{
	int i;
	int ret = 400;
	struct mg_str msg = { (char *)wm->data, wm->size };
	WS_PACKET packet;
	
	json_t *root;
	json_error_t error;

	root = json_loads(msg.p, JSON_DISABLE_EOF_CHECK, &error);
	if(root == NULL) {
		printf("error: json parsing, %dline:%s\n", error.line, error.text);
		ret = 400;
		goto Fail;
	}

	for(i = 0; i < (sizeof(ws_proc) / sizeof(ws_proc[0])); i++) {
		if(json_unpack(root, "{s:o}", ws_proc[i].name, &packet.param) == 0) {
			packet.proc_name = ws_proc[i].name;
			return ws_proc[i].func(nc, &packet);
		}
	}	

	ret = 400;
Fail:
	printf("Unknown Websocket request!!!\n");
	json_decref(root);
	return ret;
}

static int __send_packet(struct mg_connection *nc, unsigned char *data, int len, int packet_size)
{
	int pos = 0;
	int rem = len;
	int unit = packet_size;
	while(rem > 0) {
		if(rem < unit) unit = rem;
		mg_send_websocket_frame(nc, WEBSOCKET_OP_BINARY, data + pos, unit);
		pos += unit;
		rem -= unit;
	}
	return 0;
}

static void send_data(struct mg_connection *nc) 
{
	struct mg_connection_userdata *ud = (struct mg_connection_userdata *)nc->user_data;
	VIDEO_POOL_HEADER_S *frame;

	unsigned char *data = NULL;
	char *text = NULL;
	int data_len = 0;
	if(ud == NULL) {
		return;
	}

	if(ud->send_video) {
	}

	if((data != NULL || text != NULL) && data_len > 0) {
		//__send_packet(nc, data, data_len, 1024);
		if(frame->iframe == 99) {
			mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, text, data_len);
		}
		else {
			mg_send_websocket_frame(nc, WEBSOCKET_OP_BINARY, data, data_len);
		}
	}
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) 
{
	switch (ev) {
		case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST:
			{
			}
			break;
		case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
			{
				struct mg_connection_userdata *ud;
				// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(struct mg_connection_userdata));
				ud = (struct mg_connection_userdata*)malloc(sizeof(struct mg_connection_userdata));
				memset(ud, 0, sizeof(struct mg_connection_userdata));
				nc->user_data = (void *)ud;
			}
			break;
		case MG_EV_WEBSOCKET_FRAME:
			{
				struct websocket_message *wm = (struct websocket_message *)ev_data;
				int ret = proc_protocol(nc, wm);
				if(ret == 555) {
					mg_send_websocket_frame(nc, WEBSOCKET_OP_PONG, NULL, 0);
				}
				else {
					char resp[16];
					sprintf(resp, "%d", ret);
					mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, resp, strlen(resp));
				}
			}
			break;
		case MG_EV_POLL:
			{
				// send_data(nc);
			}
			break;
		case MG_EV_CLOSE:
			{
				if(nc->user_data != NULL) {
					free(nc->user_data);
				}
			}
			break;
		default:
			{
			}
			break;
	}
}
