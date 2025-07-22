#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pool.h"

#define     STREAM_PRINTF   printf

typedef struct _cindex_t
{
	unsigned int frame_pos; 
	unsigned int frame_end;      
	unsigned int circle_count;      
} cindex_t;

int pool_handle_init(pool_t  *pool, pool_rpos_t *rpos)
{
	assert((NULL != pool) && (NULL != rpos));
	int index = 0;

	if (pool->index_vaild_pos >= 0)
	{
		index = pool->index_vaild_pos;
	}
	else
	{
		index = 0;
	}

	memset(rpos, 0, sizeof(pool_rpos_t));
	rpos->data_start_pos = rpos->data_end_pos = ((cindex_t *)(pool->pindex))[index].frame_pos;
	rpos->index_cur_pos  = index;
	rpos->circle_count = ((cindex_t *)(pool->pindex))[index].circle_count;
	rpos->P = pool;
	pool->user_num++;
	return 0;
}

int pool_handle_deinit(pool_rpos_t *rpos)
{
	if((NULL != rpos->P) && (NULL != (pool_t *)rpos->P))
	{
		pool_t    *pool;
		pool = (pool_t *)rpos->P;
		pool->user_num--;
		rpos->P = NULL;    
	}

	return 0;
}

int pool_init(pool_t  *pool, pool_config_t pool_config)
{
	assert((NULL != pool) && (pool_config.pool_size > 0) && (pool_config.min_frame_size > 0) && (pool_config.max_frame_size > 0));
	memset(pool, 0, sizeof(pool_t));
	pool->total_space = pool_config.pool_size;
	pool->index_num = pool->total_space / pool_config.min_frame_size + 1;
	pool->max_frame_size = pool_config.max_frame_size;
	pool->block_safe_diff = pool->max_frame_size;
	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, pool->total_space + sizeof(cindex_t) * (pool->index_num));
	pool->pindex = (char *)malloc(pool->total_space + sizeof(cindex_t) * (pool->index_num));
	assert(NULL != pool->pindex);
	memset(pool->pindex, 0, sizeof(cindex_t) * (pool->index_num) + pool->total_space);
	pool->pbuf = (char *)(pool->pindex) + sizeof(cindex_t) * (pool->index_num);
	pool->index_vaild_pos = -1;
	return 0;
}

int pool_deinit(pool_t  *pool)
{
	assert((NULL != pool) && (NULL != pool->pindex));

	if (pool->pindex != NULL)
	{
		free(pool->pindex);
	}

	return 0;
}

int pool_send(pool_t  *pool, vbuf_t vbuf)
{
	assert((NULL != pool) && (NULL != pool->pindex));
	int cur_frame_len;
	int i = 0;
	int write_offset = 0;
	cur_frame_len = 0;

	if(pool->user_num<=0)
	{
		return 0;
	}

	for (i = 0; i < vbuf.valid_num; i++)
	{
		if ((vbuf.data_len[i] > 0) && (NULL != vbuf.pdata[i]))
		{
			cur_frame_len += vbuf.data_len[i];
		}
	}

	if (cur_frame_len >= pool->max_frame_size)
	{
		return -1;
	}

	if ((pool->total_space - pool->data_next_pos) < cur_frame_len)
	{
		pool->circle_count++;
		pool->data_next_pos = 0;
	}

	write_offset = pool->data_next_pos;
	((cindex_t *)pool->pindex)[pool->index_next_pos].frame_pos = write_offset;
	((cindex_t *)pool->pindex)[pool->index_next_pos].frame_end = write_offset + cur_frame_len;
	((cindex_t *)pool->pindex)[pool->index_next_pos].circle_count = pool->circle_count;

	for (i = 0; i < vbuf.valid_num; i++)
	{
		if ((vbuf.data_len[i] > 0)&&(NULL != vbuf.pdata[i]))
		{
			memcpy(&pool->pbuf[write_offset], vbuf.pdata[i], vbuf.data_len[i]);
			write_offset += vbuf.data_len[i];
		}
	}

	pool->data_next_pos += cur_frame_len;
	pool->index_vaild_pos = pool->index_next_pos;
	pool->index_next_pos = (pool->index_next_pos + 1) % pool->index_num;

	return 0;
}
