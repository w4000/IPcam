/*
 * misc.h
 *
 *   Purpose:
 *	- Provide commonly used data structures and algorithms.
 *	- circular list, tree, hash, sorting etc.
 **************************************************************/

#ifndef __MISC_H
#define __MISC_H


#define PAGESZ	getpagesize()

#define TBLSZ(tbl)		(sizeof (tbl) / sizeof ((tbl) [0]))

/*****************************************************************
 * Circular lists.
 ****************************************************************/
typedef struct clist {
 	volatile struct clist *next;
 	volatile struct clist *prev;
} ST_CLIST;

/* Get the struct containing this clist.
 *  ptr - ptr to the clist.
 *  type - the type of the struct this clist is embedded in.
 *  member - the name of the clist within the struct.
 */

#define _my_offset(_type, _member) \
	(unsigned long)(&(((_type *)0)->_member))

#define  _my_entry(_ptr, _type, _member) \
 	((_type *)((char *)(_ptr) - _my_offset(_type, _member)))

#if 0
#define clist_entry(_ptr, _type, _member) \
		_my_entry (_ptr, _type, _member)
#endif

#define clist_entry(_ptr, _type, _member) ((_type *)(_ptr))


/*
 * head : ptr to the head of list to iterate.
 * tmp  : temporary pointer to struct clist.
 */
#define foreach_clist_entry(head,tmp) \
	for((tmp)=(head)->next; (tmp) != (head); (tmp)=(tmp)->next)

#if 0
extern void init_clist(struct clist *);
extern void addto_clist(struct clist *, struct clist *);
extern void addto_clist_tail(struct clist *, struct clist volatile *);
extern void del_clist(struct clist *);
extern int clist_empty(struct clist *);
#else
/* Initialize list head. */
static inline void init_clist(struct clist *head){
	((struct clist *)head)->next=head;
	((struct clist *)head)->prev=head;
}

/* Internal use only.
 */
static inline void __addto_clist(struct clist volatile *prev, struct clist volatile *newnode, struct clist volatile *next){
	prev->next = newnode;
	newnode->next = next;
	newnode->prev = prev;
	next->prev = newnode;
}

/* Add the new entry just after the head.
 * It's for stack implementation.
 */
static inline void addto_clist(struct clist *head, struct clist *newnode) {
	__addto_clist(head, newnode, head->next);
}

/* Add the new entry just before the head.
 * It's for Queue implementation.
 */
static inline void addto_clist_tail(struct clist *head, struct clist volatile *newnode) {
	__addto_clist(head->prev, newnode, head);
}

/* Internal use only. Remove an entry by directly linking two nodes.
 */
static inline void __del_clist(struct clist volatile *prev, struct clist volatile *next){
 	prev->next = next;
	next->prev = prev;
 }

/* Remove a node by directly linking two nodes.
 */
#if 0
static inline void del_clist(struct clist *entry){
 	__del_clist(entry->prev, entry->next);
}
#else
static inline void del_clist(struct clist *entry){
   if (entry != NULL)
   {
       __del_clist(entry->prev, entry->next);
   }
   else
   {
#if 0   // test not code !
      struct clist *prev = NULL;
      struct clist *next = NULL;
      const int net_cli_max = 32;
      int cnt =0;

      while(!entry->prev && cnt++ != net_cli_max)
      {
         prev = entry->prev;
      }

      cnt =0;
      while(!entry->next && cnt++ != net_cli_max)
      {
         next = entry->next;
      }

      if (prev->next != NULL && prev->prev != NULL)
      {
         prev->next = next;
         next->prev = prev;
      }
      else
      {
      }

#endif
   }
}
#endif

/* test if a list is empty.
 */
static inline int clist_empty(struct clist *head) {
	return (head->next==head);
}
#endif

#if 0
#define RBUF_MAX			128		// Maximum number of items in ring.

typedef struct rbuf {

	pthread_mutex_t	mutex;

	int ridx;		// Read index in ring buffer.
	int widx;		// Write index in ring buffer.

	int nr_item;	// Max number of items in ring buffer.
	int nr_avail;	// Number of items available.
	void* item [RBUF_MAX];
} ST_RBUF;
#else
// 현재 RBUF는 사용되지 않는다..
// 만약 사용하게 되면 void* item[RBUF_MAX] 부분을 char item[RBUF_MAX][64]로 고쳐 memory alloc을 없애도록 한다.

//------------------------------ Ring buffer. --------------------------------
#define RBUF_MAX			128		// Maximum number of items in ring.

typedef struct rbuf {

	pthread_mutex_t	mutex;

	int ridx;		// Read index in ring buffer.
	int widx;		// Write index in ring buffer.

	int nr_item;	// Max number of items in ring buffer.
	int nr_avail;	// Number of items available.
	void* item [RBUF_MAX];
} ST_RBUF;

#define RBUF_NEXT(_idx, _bufmax)			(((_idx) + 1) % (_bufmax))
#define RBUF_FULL(_widx, _ridx, _bufmax)	(RBUF_NEXT((_widx), (_bufmax)) == (_ridx))
#define RBUF_EMPTY(_widx, _ridx)			((_widx) == (_ridx))

#define rbuf_full(_rbuf) 	RBUF_FULL ((_rbuf)->widx, (_rbuf)->ridx, (_rbuf)->nr_item)
#define rbuf_empty(_rbuf) 	RBUF_EMPTY ((_rbuf)->widx, (_rbuf)->ridx)

#define rbuf_release(_rbuf) \
do {\
	int i;\
	for (i = 0; i < RBUF_MAX; i ++) {\
		if ((_rbuf)->item [i]) {\
			_mem_free ((_rbuf)->item [i]);\
			(_rbuf)->item [i] = NULL;\
		}\
	}\
	\
	(_rbuf)->ridx = (_rbuf)->widx = 0;\
	(_rbuf)->nr_item = 0;\
	(_rbuf)->nr_avail = 0;\
	pthread_mutex_destroy (& (_rbuf)->mutex);\
} while (0)


// Get item to read.
#define rbuf_item_read_lock(_rbuf) \
({\
	void * ret;\
	pthread_mutex_lock (& (_rbuf)->mutex);\
	if (rbuf_empty (_rbuf)) {\
		pthread_mutex_unlock (& (_rbuf)->mutex);\
		ret = NULL;\
	}\
	else\
		ret = (_rbuf)->item [(_rbuf)->ridx];\
	ret;\
})

#define rbuf_read_done_unlock(_rbuf) \
do {\
	(_rbuf)->ridx = RBUF_NEXT ((_rbuf)->ridx, (_rbuf)->nr_item);\
	(_rbuf)->nr_avail --;\
	pthread_mutex_unlock (& (_rbuf)->mutex);\
} while (0)

// Get item to write.
#define rbuf_item_write_lock(_rbuf) \
({\
	void * _ret; \
	pthread_mutex_lock (& (_rbuf)->mutex);\
	if (rbuf_full (_rbuf)) {\
		pthread_mutex_unlock (& (_rbuf)->mutex);\
		_ret = NULL;\
	}\
	else\
		_ret = (_rbuf)->item [(_rbuf)->widx];\
	_ret;\
})

#define rbuf_write_done_unlock(_rbuf) \
do {\
	(_rbuf)->widx = RBUF_NEXT ((_rbuf)->widx, (_rbuf)->nr_item);\
	(_rbuf)->nr_avail ++;\
	pthread_mutex_unlock (& (_rbuf)->mutex);\
} while (0)
#endif

//---------------- Architecture specific align & endian conversion. -----------------

#if __BYTE_ORDER==__BIG_ENDIAN
#define LE16_TO_HOST(val) bswap_16(val)		/* Convert one word little endian to host's endian */
#define LE32_TO_HOST(val) bswap_32(val)		/* Convert 2 words little endian to host's endian */
#define HOST_TO_LE16(val) bswap_16(val)		/* Convert one word to little endian.*/
#define HOST_TO_LE32(val) bswap_32(val)		/* Convert 2 words to little endian.*/
#define BE16_TO_HOST(val)	(val)
#define BE32_TO_HOST(val) 	(val)
#define HOST_TO_BE16(val) 	(val)
#define HOST_TO_BE32(val) 	(val)
#else
#define BE16_TO_HOST(val) bswap_16(val)
#define BE32_TO_HOST(val) bswap_32(val)
#define HOST_TO_BE16(val) bswap_16(val)
#define HOST_TO_BE32(val) bswap_32(val)
#define LE16_TO_HOST(val)	(val)
#define LE32_TO_HOST(val) 	(val)
#define HOST_TO_LE16(val) 	(val)
#define HOST_TO_LE32(val) 	(val)
#endif

#if 1
#define __ARCH_BADALGN__(addr, type)  	(__alignof__ (type) - 1) & addr)

#define __ARCH_ALIGN__(addr, type)	(typeof (addr)) ((__u32)((void *)(addr) + (__alignof__(type) - 1)) \
							& (__u32)(~(__alignof__(type) - 1)))

#define __ARCH_ASSIGN__(field, val)	({\
						__u8 __tmp__ [sizeof (field)] __attribute__ ((aligned (4)));\
						int __i__;\
						* (typeof (field) *) __tmp__ = (val);\
						for (__i__ = 0; __i__ < sizeof (field); __i__ ++)\
							* (__u8 *) ((void *) (& field) + __i__) = __tmp__ [__i__];\
					})

#define __ARCH_READ__(field)		({\
						__u8 __tmp__ [sizeof (field)] __attribute__ ((aligned (4)));\
						int __i__;\
						for (__i__ = 0; __i__ < sizeof (field); __i__ ++)\
							__tmp__ [__i__] = * (__u8 *) ((void *) (& field) + __i__);\
						* (typeof (field) *) __tmp__;\
					})

#else
#define __ARCH_BADALGN__(addr, type)  	0
#define __ARCH_ALIGN__(addr, type)	(addr)
#define __ARCH_ASSIGN__(field, val)	({field = val;})
#define __ARCH_READ__(field)		(field)
#endif

// Consider alignment + endian.
#define __ARCH_READ_BE__(field)	({\
						typeof (field) ret = __ARCH_READ__(field);\
						(sizeof (field) == 4) ? BE32_TO_HOST (ret) :\
						(sizeof (field) == 2) ? BE16_TO_HOST (ret) :\
						ret;\
					})

#define __ARCH_ASSIGN_BE__(field, val)	\
						__ARCH_ASSIGN__ (field, \
											(sizeof (field) == 4) ? HOST_TO_BE32 (val) :\
											(sizeof (field) == 2) ? HOST_TO_BE16 (val) :\
											val\
											)

#define __ARCH_READ_LE__(field)	({\
						typeof (field) ret = __ARCH_READ__(field);\
						(sizeof (field) == 4) ? LE32_TO_HOST (ret) :\
						(sizeof (field) == 2) ? LE16_TO_HOST (ret) :\
						ret;\
					})

#define __ARCH_ASSIGN_LE__(field, val)	\
						__ARCH_ASSIGN__ (field, \
											(sizeof (field) == 4) ? HOST_TO_LE32 (val) :\
											(sizeof (field) == 2) ? HOST_TO_LE16 (val) :\
											val\
											)


#define __ARCH_READ_ENDIAN__(field)		__ARCH_READ_LE__(field)
#define __ARCH_ASSIGN_ENDIAN__(field, val)	__ARCH_ASSIGN_LE__(field, val)
#define __ARCH_READ_NET__(field)			__ARCH_READ_BE__(field)
#define __ARCH_ASSIGN_NET__(field, val)		__ARCH_ASSIGN_BE__(field, val)

/***********************************************************************
 * Bitwise operation.
 ***********************************************************************/
#define bitset(var, i)			({var |= (1 << (i)); })
#define bitunset(var, i)		({var &= ~(1 << (i)); })
#define bitisset(var, i)		({(var) & (1 << (i)); })


/***********************************************************************
 * Calculating time elapsion.
 ***********************************************************************/

// Check if given milli seconds has elapsed since last call.
// @msec : variable holding milliseconds left until expire.
//
#define usec_elapsed(msec, clkold) \
	({\
		clock_t clkcur, elapsed;	\
		\
		clkcur = times (NULL);		\
		elapsed = clkcur - clkold;	\
		clkold = clkcur;				\
		msec -= (TIME_MSEC_CLKTCK * elapsed);\
		((msec > 1000) || (msec <= 0)) ? 1 : 0;		\
	})

/***********************************************************************
 * Other commonly used defines.
 ************************************************************************/

// Concatenate formatted string.
#define	STRCAT_BUF(buf, bufsz, arg...)	\
	do {\
		int	__n__ = strlen (buf);\
		if (__n__ < bufsz) \
			snprintf (buf + __n__, bufsz - __n__, ##arg);\
	} while (0)

/* atoi is implemented in stdlib.h, but no func
 * something like itoa.
 */
#define itoa(i, buf) \
		({sprintf((buf), "%d", (i)); buf;})

#define min(a,b)	({	typeof (a) _a = a;		\
						typeof (b) _b = b;		\
						(_a < _b) ? _a : _b;	\
					})

#define max(a,b)	({	typeof (a) _a = a;		\
						typeof (b) _b = b;		\
						(_a > _b) ? _a : _b;	\
					})

/* bitfield defines */
#define MASK_31                 0x80000000
#define MASK_30                 0x40000000
#define MASK_29                 0x20000000
#define MASK_28                 0x10000000
#define MASK_27                 0x08000000
#define MASK_26                 0x04000000
#define MASK_25                 0x02000000
#define MASK_24                 0x01000000
#define MASK_23                 0x00800000
#define MASK_22                 0x00400000
#define MASK_21                 0x00200000
#define MASK_20                 0x00100000
#define MASK_19                 0x00080000
#define MASK_18                 0x00040000
#define MASK_17                 0x00020000
#define MASK_16                 0x00010000
#define MASK_15                 0x00008000
#define MASK_14                 0x00004000
#define MASK_13                 0x00002000
#define MASK_12                 0x00001000
#define MASK_11                 0x00000800
#define MASK_10                 0x00000400
#define MASK_09                 0x00000200
#define MASK_08                 0x00000100
#define MASK_07                 0x00000080
#define MASK_06                 0x00000040
#define MASK_05                 0x00000020
#define MASK_04                 0x00000010
#define MASK_03                 0x00000008
#define MASK_02                 0x00000004
#define MASK_01                 0x00000002
#define MASK_00                 0x00000001
#define MASK_B0                 0x000000ff
#define MASK_B1                 0x0000ff00
#define MASK_B2                 0x00ff0000
#define MASK_B3                 0xff000000
#define MASK_W0                 0x0000ffff
#define MASK_W1                 0xffff0000
#define MASK_NONE               0x00000000

extern char * trimstr (char *, char);
extern void remove_specialstr(char *, char *);
extern int maxval(int [], size_t);
extern int maxidx(int [], size_t);
extern int minval(int [], size_t);
extern int minidx(int [], size_t);
extern int sum(int [], size_t);
extern int compare(const void *a, const void *b);
extern void swap_byte(char *, char *);

extern int ipstr2no (char *);
extern void no2ipstr (int, char *);

extern void set_signal_handler(void (*)(int), int);
// extern inline void sig_block(int);
// extern inline void sig_unblock(int);

//extern int get_cpuload(unsigned long *, unsigned long *);
//extern int get_freememory();
extern int path_prefix(char *, char *);

extern int random_seed (int size, void * buf);

extern char *str_getword (char *buf, int bufsz, char *p);
extern char *str_replace_needle (char *dest, int destsz, char * src, char *needle, char * new_needle);

extern int iconv_codepage_conv(char* out_buf, int out_bufsz, char* in_page, char* out_codepage, char* in_codepage);

#define strsafecat(dest, src, size) do { strncat((dest), (src), (size) - strlen(dest)); (dest)[(size) - 1] = 0; } while(0)

#endif

