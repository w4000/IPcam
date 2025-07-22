#include "common.h"
#include "lib/threadsafe.h"

ST_MSG_QUEUE	* g_msgq [64] = {NULL,};

static inline ST_MESSAGE* get_msg_pool()
{
	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof (ST_MESSAGE));
	return _mem_malloc2 (sizeof (ST_MESSAGE));
}

inline void free_msg_pool(ST_MESSAGE* msg)
{
	_mem_free(msg);
}

void init_msg_queue (ST_MSG_QUEUE	* msgq, unsigned idx) {
	assert (msgq);
	assert (idx < 64);
	assert (g_msgq [idx] == NULL);

	init_clist(&msgq->head);
	pthread_mutex_init (& msgq->mutex, NULL);
	msgq->nummsg = 0;

	g_msgq [idx] = msgq;

	// printf("! init_msg_queue >>> idx? %d \n", idx);
}

void flush_msg_queue(struct st_msg_queue *msgq) {
	struct clist *tmp = NULL;
	struct st_message *pmsg = NULL;

	assert (msgq);

	pthread_mutex_lock(&msgq->mutex);
	while (!clist_empty(&msgq->head)) {
		tmp = (struct clist *)msgq->head.next;
		pmsg = clist_entry(tmp, struct st_message, list);
		del_clist(tmp);
		free_msg_pool(pmsg);
	}
	msgq->nummsg = 0;
	pthread_mutex_unlock(&msgq->mutex);
	pthread_mutex_destroy(&msgq->mutex);
}



int send_msg (unsigned from, unsigned to, int msgid, int parm1, int parm2, int parm3)
{
	ST_MESSAGE *msg = NULL;

	assert (to < 64);

	if ((msg = get_msg_pool()) == NULL)
		return -1;


	init_clist(&msg->list);
	msg->from = from;
	msg->msgid = msgid;
	msg->parm1 = parm1;
	msg->parm2 = parm2;
	msg->parm3 = parm3;

	if (! g_msgq [to]) {
		free_msg_pool(msg);
		printf ("Message Queue (g_msgq[%d]) is not ready yet. msgid=%d\n", to, msgid);
		return -1;
	}

	addto_clist_tail(& g_msgq [to]->head, &msg->list);
	g_msgq [to]->nummsg ++;

	return 0;
}


int get_msg(struct st_msg_queue *msgq, struct st_message *msg) {
	struct clist *tmp = NULL;
	struct st_message *pmsg = NULL;

	assert (msgq);
	assert (msg);

	if (clist_empty(&msgq->head)) {
		pthread_mutex_unlock(&msgq->mutex);
		return 0;
	}

	
	tmp = (struct clist *)msgq->head.next;
	pmsg = clist_entry(tmp, struct st_message, list);
	del_clist(tmp);
	msgq->nummsg--;

	*msg = *pmsg;

	free_msg_pool(pmsg);

	return 1;
}
