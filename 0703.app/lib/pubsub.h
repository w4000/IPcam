#ifndef __PUBSUB_H__
#define __PUBSUB_H__

void *init_pub(int port);
int publish(void *pub, const char *data, size_t len);
void fini_pub(void *pub);

void *init_sub(int port);
int subscribe(void *sub, char *data, size_t len);
void fini_sub(void *sub);

void term_pubsub();

#endif /*__PUBSUB_H__*/
