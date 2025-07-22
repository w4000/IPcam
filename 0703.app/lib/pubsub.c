#include <stdio.h>

#include "zmq.h"

void *context = NULL;

void *init_pub(int port)
{
	char url[64];
	void *publisher = NULL;
	
	if(context == NULL) {
		context = zmq_init(1);
	}
	
	if(context == NULL) {
		return NULL;
	}

	publisher = zmq_socket(context, ZMQ_PUB);
	if(publisher == NULL) {
		return NULL;
	}

	unsigned long long hwm = 1;
    zmq_setsockopt (publisher, ZMQ_HWM, &hwm, sizeof (hwm));

	sprintf(url, "tcp://*:%d", port);
	if(zmq_bind(publisher, url) != 0) {
		zmq_close(publisher);
		return NULL;
	}

	return publisher;
}

int publish(void *pub, const char *data, size_t len)
{
	zmq_msg_t msg;

	if(pub == NULL) {
		return -1;
	}

	if(zmq_msg_init_size(&msg, len) != 0) {
		return -1;
	}

	memcpy(zmq_msg_data(&msg), data, len);

	return zmq_send(pub, &msg, ZMQ_NOBLOCK);
}

void fini_pub(void *pub)
{
	zmq_close(pub);
}

void *init_sub(int port)
{
	char url[64];
	void *subscriber = NULL;
	
	if(context == NULL) {
		context = zmq_init(1);
	}

	subscriber = zmq_socket(context, ZMQ_SUB);
	if(subscriber == NULL) {
		return NULL;
	}

	sprintf(url, "tcp://127.0.0.1:%d", port);
	if(zmq_connect(subscriber, url) != 0) {
		zmq_close(subscriber);
		subscriber = NULL;
		return NULL;
	}

	if(zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0) != 0) {
		zmq_close(subscriber);
		subscriber = NULL;
		return NULL;
	}

	return subscriber;
}

int subscribe(void *sub, char *data, size_t len)
{
	zmq_msg_t msg;
	
	if(sub == NULL) {
		return -1;
	}

	if(zmq_msg_init(&msg) != 0) {
		return -1;
	}

#if 0
	if(zmq_recv(sub, &msg, ZMQ_NOBLOCK) < 0) {
		return -4;
	}

	len = zmq_msg_size(&msg);
	memcpy(data, zmq_msg_data(&msg), len);
#else
	while(zmq_recv(sub, &msg, ZMQ_NOBLOCK) >= 0) {
		len = zmq_msg_size(&msg);
		memcpy(data, zmq_msg_data(&msg), len);
	}

	if(len <= 0) {
		return -4;
	}
#endif

	zmq_msg_close(&msg);

	return len;
}

void fini_sub(void *sub)
{
	zmq_close(sub);
}

void term_pubsub()
{
	if(context != NULL) {
		zmq_term(context);
	}
}
