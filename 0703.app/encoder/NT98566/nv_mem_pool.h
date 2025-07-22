#ifndef __NV_MEM_POOL_H__
#define __NV_MEM_POOL_H__

// int nv_mem_pool_init(unsigned int width, unsigned int height, unsigned int stamp_size, unsigned int ai_buff_size);
int nv_mem_pool_init(unsigned int width, unsigned int height, unsigned int stamp_size);
int nv_mem_pool_exit();
unsigned int nv_mem_alloc(UINT32 stamp_size, UINT32 *stamp_blk, UINT32 *stamp_pa);
unsigned int nv_mem_release(UINT32 stamp_blk);
int nv_share_memory_init(NV_MEM_RANGE *p_share_mem);
int nv_share_memory_exit(NV_MEM_RANGE *p_share_mem);
int nv_query_osg_buf_size(unsigned int width, unsigned int height);

#endif /*__NV_MEM_POOL_H__*/
