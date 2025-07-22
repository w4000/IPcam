
#ifndef __KMSG_H
#define __KMSG_H

#define KMSG_FILE				"/proc/kmsg"
#define ETHERNET_LINKUP0		": Link is Up"
#define ETHERNET_LINKDOWN0		": Link is Down"

extern pthread_t kmsg_tid;
extern void *kmsgproc(void *);

extern int is_net_link();

#endif
