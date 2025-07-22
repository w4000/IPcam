#ifndef __POOL_H__
#define __POOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>

#define MAX_BUF_NUM   (10)

typedef struct _vbuf_t
{
	unsigned int valid_num;            
	unsigned int data_len[MAX_BUF_NUM];
	void *pdata[MAX_BUF_NUM];          
} vbuf_t;

typedef struct
{
	unsigned int max_frame_size; 
	unsigned int min_frame_size;
	unsigned int pool_size;
} pool_config_t;

typedef struct
{
	int index_cur_pos;

	int data_start_pos;
	int data_end_pos;

	int circle_count;
	int trace_enable;
	void *P;
} pool_rpos_t;

typedef struct _pool_t
{
	int index_vaild_pos;
	int index_next_pos;

	int data_next_pos;
	int circle_count;

	int max_frame_size;

	int block_safe_diff;

	int total_space;
	int index_num;
	int user_num;

	char *pindex;
	char *pbuf;
} pool_t;

int pool_handle_init(pool_t  *pool, pool_rpos_t *rpos);
int pool_handle_deinit(pool_rpos_t *rpos);

int pool_init(pool_t  *pool, pool_config_t pool_config);
int pool_deinit(pool_t  *phead_pool);

int pool_send(pool_t  *pool, vbuf_t vbuf);


#endif
