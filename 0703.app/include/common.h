
#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <locale.h>
//#include <libintl.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <dirent.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <time.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/vfs.h>
//#include <sys/timeb.h>
#include <sys/times.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <linux/fb.h>
#include <linux/netlink.h>
#include <mtd/mtd-user.h>
#include <net/if.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <termios.h>
#include <byteswap.h>
#include <ctype.h>
#include <libgen.h>
#include <sys/swap.h>
#include <asm/types.h>

#ifdef USE_DF2_SUPPORT
#include "oem_xml_define.h"
#include "oem_tool.h"
#else
#include "_config/model_spec.h"
#endif

#include "utils/utils.h"

#include "oem.h"
#include "dvr.h"
#include "err.h"




#include "openssl/opensslconf.h"
#include "openssl/crypto.h"
#include "openssl/blowfish.h"
#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/rsa.h"
#include "openssl/evp.h"
#include "openssl/objects.h"
#include "openssl/aes.h"
#include "lib/blowfish_av.h"


#include "lib/circular_queue.h"
#include "lib/CircularBuffer.h"

#include "lib/sha256.h"
#include "lib/cipher.h"
#include "lib/datetime.h"
#include "lib/blowfish.h"
#include "lib/filelib.h"
#include "lib/misc.h"
#include "lib/verify_user.h"
#include "lib/font.h"
#include "lib/pubsub.h"
#include "ipc/ipc_status.h"

#include "thread/thread.h"
#include "netsvr/netsvr.h"
#include "file/file.h"
#include "timer/timer.h"
#include "keybd/keybd.h"
#include "encoder/encoder.h"
#include "encoder/video_default.h"


#include "pos/poslib.h"
#include "utils/debug.h"
#include "timer/watchdog.h"


#include "main.h"

#endif /* COMMON_H_ */
