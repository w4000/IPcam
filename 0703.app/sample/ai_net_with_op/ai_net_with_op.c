/**
	@brief Source file of vendor ai net sample code.

	@file ai_net_with_op.c

	@ingroup ai_net_sample

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2020.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Including Files                                                             */
/*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "hdal.h"
#include "hd_debug.h"
#include "vendor_ai.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"
#include "vendor_ai_cpu_postproc.h"
#if defined(_BSP_NA51068_) || defined(_BSP_NA51090_)
#include "vendor_common.h"
#endif

// platform dependent
#if defined(__LINUX)
#include <pthread.h>			//for pthread API
#define MAIN(argc, argv) 		int main(int argc, char** argv)
#define GETCHAR()				getchar()
#else
#include <FreeRTOS_POSIX.h>	
#include <FreeRTOS_POSIX/pthread.h> //for pthread API
#include <kwrap/util.h>		//for sleep API
#define sleep(x)    			vos_util_delay_ms(1000*(x))
#define msleep(x)    			vos_util_delay_ms(x)
#define usleep(x)   			vos_util_delay_us(x)
#include <kwrap/examsys.h> 	//for MAIN(), GETCHAR() API
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(ai_net_with_op, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define DEBUG_MENU 		1

#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)

///////////////////////////////////////////////////////////////////////////////

#define NET_PATH_ID		UINT32

#define VENDOR_AI_CFG  				0x000f0000  //vendor ai config

#define AI_RGB_BUFSIZE(w, h)		(ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(HD_VIDEO_PXLFMT_RGB888_PLANAR) / 8) * (h))

#define NET_VDO_SIZE_W	1920 //max for net
#define NET_VDO_SIZE_H	1080 //max for net

#define SV_LENGTH 		10240
#define SV_FEA_LENGTH 	256

#define USE_NET 		TRUE

#define SCALE_DIM_W 384
#define SCALE_DIM_H 282

#define ALLOC_WORKBUF_BY_USER 1

typedef enum _AI_OP {
    AI_OP_FC                             = 0, 
	AI_OP_PREPROC_YUV2RGB                = 1,  
	AI_OP_PREPROC_YUV2RGB_SCALE          = 2,
	AI_OP_PREPROC_YUV2RGB_MEANSUB_PLANE  = 3,
	AI_OP_PREPROC_YUV2RGB_MEANSUB_DC     = 4,
	AI_OP_PREPROC_Y2Y_UV2UV				 = 5,
	AI_OP_FC_LL_MODE                     = 6,
	ENUM_DUMMY4WORD(AI_OP)
} AI_OP;
///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------*/
/* Type Definitions                                                            */
/*-----------------------------------------------------------------------------*/

typedef struct _MEM_PARM {
	uintptr_t pa;
	uintptr_t va;
	UINT32 size;
	uintptr_t blk;
} MEM_PARM;


/*-----------------------------------------------------------------------------*/
/* Global Functions                                                             */
/*-----------------------------------------------------------------------------*/

#if defined(_BSP_NA51068_) || defined(_BSP_NA51090_)
static HD_RESULT mem_get(MEM_PARM *mem_parm, UINT32 size, UINT32 id)
{
	HD_RESULT ret = HD_OK;

	if (size == 0) {
		printf("mem_alloc fail, size = 0\r\n");
		ret = HD_ERR_NG;
		goto exit;
	}

	mem_parm->size = size;
	mem_parm->blk = hd_common_mem_get_block(HD_COMMON_MEM_CNN_POOL, mem_parm->size, DDR_ID0);
	if (HD_COMMON_MEM_VB_INVALID_BLK == mem_parm->blk) {
		printf("hd_common_mem_get_block fail\r\n");
		ret = HD_ERR_NG;
		goto exit;
	}
	mem_parm->pa = hd_common_mem_blk2pa(mem_parm->blk);
	if (mem_parm->pa == 0) {
		printf("hd_common_mem_blk2pa fail, blk = %#lx\r\n", mem_parm->blk);
		hd_common_mem_release_block(mem_parm->blk);
		ret = HD_ERR_NG;
		goto exit;
	}
	/* Must use "HD_COMMON_MEM_MEM_TYPE_CACHE", or it will cause the cpu layer to perform inefficiently */
	mem_parm->va = (UINTPTR)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, mem_parm->pa, mem_parm->size);
	if (mem_parm->va == 0) {
		ret = HD_ERR_NG;
		goto exit;
	}

exit:
	return ret;
}

static HD_RESULT mem_rel(MEM_PARM *mem_parm)
{
	HD_RESULT ret = HD_OK;

	if (mem_parm->va) {
		ret = hd_common_mem_munmap((void *)mem_parm->va, mem_parm->size);
		if (ret != HD_OK) {
			printf("hd_common_mem_munmap fail\n");
		}
	}
	ret = hd_common_mem_release_block(mem_parm->blk);
	if (ret != HD_OK) {
		printf("hd_common_mem_release_block fail\n");
	}

	return ret;
}
static HD_RESULT mem_alloc(MEM_PARM *mem_parm, CHAR* name, UINT32 size)
{
	return mem_get(mem_parm, size, 0);
}

static HD_RESULT mem_free(MEM_PARM *mem_parm)
{
	return mem_rel(mem_parm);
}
#else
#if (USE_NET == TRUE)
static HD_RESULT mem_get(MEM_PARM *mem_parm, UINT32 size, UINT32 id)
{
	HD_RESULT ret = HD_OK;
	UINT32 pa   = 0;
	void  *va   = NULL;
	HD_COMMON_MEM_VB_BLK      blk;

	blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_DEFINIED_POOL + id, size, DDR_ID0);
	if (HD_COMMON_MEM_VB_INVALID_BLK == blk) {
		printf("hd_common_mem_get_block fail\r\n");
		return HD_ERR_NG;
	}
	pa = hd_common_mem_blk2pa(blk);
	if (pa == 0) {
		printf("not get buffer, pa=%08x\r\n", (int)pa);
		return HD_ERR_NOMEM;
	}
	va = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, size);

	/* Release buffer */
	if (va == 0) {
		ret = hd_common_mem_munmap(va, size);
		if (ret != HD_OK) {
			printf("mem unmap fail\r\n");
			return ret;
		}
	}

	mem_parm->pa = pa;
	mem_parm->va = (UINT32)va;
	mem_parm->size = size;
	mem_parm->blk = blk;

	return HD_OK;
}

static HD_RESULT mem_rel(MEM_PARM *mem_parm)
{
	HD_RESULT ret = HD_OK;

	/* Release in buffer */
	if (mem_parm->va) {
		ret = hd_common_mem_munmap((void *)mem_parm->va, mem_parm->size);
		if (ret != HD_OK) {
			printf("mem_uninit : (g_mem.va)hd_common_mem_munmap fail.\r\n");
			return ret;
		}
	}
	//ret = hd_common_mem_release_block((HD_COMMON_MEM_VB_BLK)g_mem.pa);
	ret = hd_common_mem_release_block(mem_parm->blk);
	if (ret != HD_OK) {
		printf("mem_uninit : (g_mem.pa)hd_common_mem_release_block fail.\r\n");
		return ret;
	}

	mem_parm->pa = 0;
	mem_parm->va = 0;
	mem_parm->size = 0;
	mem_parm->blk = (UINT32)-1;
	return HD_OK;
}
#endif
static HD_RESULT mem_alloc(MEM_PARM *mem_parm, CHAR* name, UINT32 size)
{
	HD_RESULT ret = HD_OK;
	UINT32 pa   = 0;
	void  *va   = NULL;

	//alloc private pool
	ret = hd_common_mem_alloc(name, &pa, (void**)&va, size, DDR_ID0);
	if (ret!= HD_OK) {
		return ret;
	}

	mem_parm->pa   = pa;
	mem_parm->va   = (UINT32)va;
	mem_parm->size = size;
	mem_parm->blk  = (UINT32)-1;

	return HD_OK;
}

static HD_RESULT mem_free(MEM_PARM *mem_parm)
{
	HD_RESULT ret = HD_OK;
	
	//free private pool
	ret =  hd_common_mem_free(mem_parm->pa, (void *)mem_parm->va);
	if (ret!= HD_OK) {
		return ret;
	}
	
	mem_parm->pa = 0;
	mem_parm->va = 0;
	mem_parm->size = 0;
	mem_parm->blk = (UINT32)-1;

	return HD_OK;
}
#endif

static INT32 mem_save(MEM_PARM *mem_parm, const CHAR *filename)
{
	FILE *fd;
	UINT32 size = 0;

	fd = fopen(filename, "wb");

	if (!fd) {
		printf("ERR: cannot open %s for write!\r\n", filename);
		return -1;
	}

	size = (INT32)fwrite((VOID *)mem_parm->va, 1, mem_parm->size, fd);
	if (size != mem_parm->size) {
		printf("ERR: write %s with size %ld < wanted %ld?\r\n", filename, size, mem_parm->size);
	} else {
		printf("write %s with %ld bytes.\r\n", filename, mem_parm->size);
	}

	if (fd) {
		fclose(fd);
	}

	return size;
}

static VOID mem_fill(MEM_PARM *mem_parm, int mode)
{
    UINT32 i = 0;

	if (mode == 0) {
		// clear
		memset((VOID *)mem_parm->va, 1, mem_parm->size); 
	} else {
	    // struct timeval time_temp;
	    // gettimeofday(&time_temp, NULL);
	    // srand((time_temp.tv_sec - time_temp.tv_sec) * 1000000 + (time_temp.tv_usec - time_temp.tv_usec));
	    // for(i = 0; i < mem_parm->size; i++)
	    // {
	    //     ((UINT8 *)mem_parm->va)[i] = rand() & 0xff;
	    // }
	    for(i = 0; i < mem_parm->size; i++) {
	        ((INT8 *)mem_parm->va)[i] = (i & 0x07);
	    }
	}
}

static INT32 mem_load(MEM_PARM *mem_parm, const CHAR *filename)
{
	FILE *fd;
	INT32 size = 0;

	fd = fopen(filename, "rb");

	if (!fd) {
		printf("cannot read %s\r\n", filename);
		size = -1;
		goto exit;
	}

	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	// check "ai_in_buf" enough or not
	if (mem_parm->size < (UINT32)size) {
		printf("ERROR: ai_in_buf(%u) is not enough, input file(%u)\r\n", mem_parm->size, (UINT32)size);
		size = -1;
		goto exit;
	}
	
	if (size < 0) {
		printf("getting %s size failed\r\n", filename);
		goto exit;
	} else if ((INT32)fread((VOID *)mem_parm->va, 1, size, fd) != size) {
		printf("read size < %ld\r\n", size);
		size = -1;
		goto exit;
	}
	mem_parm->size = size;

	// we use cpu to read memory, which needs to deal cache flush.
	if(hd_common_mem_flush_cache((VOID *)mem_parm->va, mem_parm->size) != HD_OK) {
        printf("flush cache failed.\r\n");
    }

exit:
	if (fd) {
		fclose(fd);
	}

	return size;
}

/*-----------------------------------------------------------------------------*/
/* Input Functions                                                             */
/*-----------------------------------------------------------------------------*/

#if (USE_NET == TRUE)
///////////////////////////////////////////////////////////////////////////////

typedef struct _NET_IN_CONFIG {

	CHAR input_filename[256];
	UINT32 w;
	UINT32 h;
	UINT32 c;
	UINT32 loff;
	UINT32 fmt;
	
} NET_IN_CONFIG;

typedef struct _NET_IN {

	NET_IN_CONFIG in_cfg;
	MEM_PARM input_mem;
	UINT32 in_id;
	VENDOR_AI_BUF src_img;
		
} NET_IN;

static NET_IN g_in[16] = {0};

static HD_RESULT _load_buf(MEM_PARM *mem_parm, CHAR *filename, VENDOR_AI_BUF *p_buf, UINT32 w, UINT32 h, UINT32 c, UINT32 loff, UINT32 fmt)
{
	INT32 file_len;
#if 0
	UINT32 key = 0;
#endif

	file_len = mem_load(mem_parm, filename);
	if (file_len < 0) {
		printf("load buf(%s) fail\r\n", filename);
		return HD_ERR_NG;
	}
	printf("load buf(%s) ok\r\n", filename);
	p_buf->width = w;
	p_buf->height = h;
	p_buf->channel = c;
	p_buf->line_ofs = loff;
	p_buf->fmt = fmt;
	p_buf->pa   = mem_parm->pa;
	p_buf->va   = mem_parm->va;
	p_buf->sign = MAKEFOURCC('A','B','U','F');
	p_buf->size = loff * h * 3 / 2;
	return HD_OK;
}

static HD_RESULT input_init(void)
{
	HD_RESULT ret = HD_OK;
	int  i;
	
	for (i = 0; i < 16; i++) {
		NET_IN* p_net = g_in + i;
		p_net->in_id = i;
	}
	return ret;
}

static HD_RESULT input_uninit(void)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

INT32 input_mem_config(NET_PATH_ID net_path, HD_COMMON_MEM_INIT_CONFIG* p_mem_cfg, void* p_cfg, INT32 i)
{
	return i;
}

static HD_RESULT input_set_config(NET_PATH_ID net_path, NET_IN_CONFIG* p_in_cfg)
{
	HD_RESULT ret = HD_OK;
	NET_IN* p_in = g_in + net_path;
	UINT32 proc_id = p_in->in_id;
	
	memcpy((void*)&p_in->in_cfg, (void*)p_in_cfg, sizeof(NET_IN_CONFIG));
	printf("proc_id(%u) set in_cfg: file(%s), buf=(%u,%u,%u,%u,%08x)\r\n", 
		proc_id,
		p_in->in_cfg.input_filename,
		p_in->in_cfg.w,
		p_in->in_cfg.h,
		p_in->in_cfg.c,
		p_in->in_cfg.loff,
		p_in->in_cfg.fmt);
	
	return ret;
}

static HD_RESULT input_open(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_IN* p_net = g_in + net_path;
	UINT32 proc_id = p_net->in_id;

	ret = mem_alloc(&p_net->input_mem, "ai_in_buf", AI_RGB_BUFSIZE(p_net->in_cfg.w, p_net->in_cfg.h));
	if (ret != HD_OK) {
		printf("proc_id(%lu) alloc ai_in_buf fail\r\n", proc_id);
		return HD_ERR_FAIL;
	}

	ret = _load_buf(&p_net->input_mem, 
		p_net->in_cfg.input_filename, 
		&p_net->src_img,
		p_net->in_cfg.w,
		p_net->in_cfg.h,
		p_net->in_cfg.c,
		p_net->in_cfg.loff,
		p_net->in_cfg.fmt);
	if (ret != HD_OK) {
		printf("proc_id(%u) input_open fail=%d\n", proc_id, ret);
	}

	return ret;
}

static HD_RESULT input_close(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_IN* p_net = g_in + net_path;

	mem_free(&p_net->input_mem);
	
	return ret;
}

static HD_RESULT input_start(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

static HD_RESULT input_stop(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

static HD_RESULT input_pull_buf(NET_PATH_ID net_path, VENDOR_AI_BUF *p_in, INT32 wait_ms)
{
	HD_RESULT ret = HD_OK;
	NET_IN* p_net = g_in + net_path;
	
	memcpy((void*)p_in, (void*)&(p_net->src_img), sizeof(VENDOR_AI_BUF));
	return ret;
}
///////////////////////////////////////////////////////////////////////////////
#endif

/*-----------------------------------------------------------------------------*/
/* Network Functions                                                             */
/*-----------------------------------------------------------------------------*/

#if (USE_NET == TRUE)
typedef struct _NET_PROC_CONFIG {

	CHAR model_filename[256];
	INT32 binsize;
	int job_method;
	int job_wait_ms;
	int buf_method;
	void *p_share_model;

	CHAR label_filename[256];

} NET_PROC_CONFIG;

typedef struct _NET_PROC {

	NET_PROC_CONFIG net_cfg;
	MEM_PARM proc_mem;
	UINT32 proc_id;
	
	CHAR out_class_labels[MAX_CLASS_NUM * VENDOR_AIS_LBL_LEN];
	MEM_PARM rslt_mem;
	MEM_PARM io_mem;
		
} NET_PROC;

static NET_PROC g_net[16] = {0};

static INT32 _getsize_model(char* filename)
{
	FILE *bin_fd;
	UINT32 bin_size = 0;

	bin_fd = fopen(filename, "rb");
	if (!bin_fd) {
		printf("get bin(%s) size fail\n", filename);
		return (-1);
	}

	fseek(bin_fd, 0, SEEK_END);
	bin_size = ftell(bin_fd);
	fseek(bin_fd, 0, SEEK_SET);
	fclose(bin_fd);

	return bin_size;
}

static UINT32 _load_model(CHAR *filename, UINT32 va)
{
	FILE  *fd;
	UINT32 file_size = 0, read_size = 0;
	const UINT32 model_addr = va;
	//DBG_DUMP("model addr = %08x\r\n", (int)model_addr);

	fd = fopen(filename, "rb");
	if (!fd) {
		printf("load model(%s) fail\r\n", filename);
		return 0;
	}

	fseek ( fd, 0, SEEK_END );
	file_size = ALIGN_CEIL_4( ftell(fd) );
	fseek ( fd, 0, SEEK_SET );

	read_size = fread ((void *)model_addr, 1, file_size, fd);
	if (read_size != file_size) {
		printf("size mismatch, real = %d, idea = %d\r\n", (int)read_size, (int)file_size);
	}
	fclose(fd);
	
	printf("load model(%s) ok\r\n", filename);
	return read_size;
}

static HD_RESULT _load_label(UINT32 addr, UINT32 line_len, const CHAR *filename)
{
	FILE *fd;
	CHAR *p_line = (CHAR *)addr;

	fd = fopen(filename, "r");
	if (!fd) {
		printf("load label(%s) fail\r\n", filename);
		return HD_ERR_NG;
	}

	while (fgets(p_line, line_len, fd) != NULL) {
		p_line[strlen(p_line) - 1] = '\0'; // remove newline character
		p_line += line_len;
	}

	if (fd) {
		fclose(fd);
	}

	printf("load label(%s) ok\r\n", filename);
	return HD_OK;
}
#endif

static HD_RESULT network_init(void)
{
	HD_RESULT ret = HD_OK;
	
	ret = hd_videoproc_init();
	if (ret != HD_OK) {
		printf("hd_videoproc_init fail=%d\n", ret);
		return ret;
	}
	
	// config extend engine plugin, process scheduler
	{
		UINT32 schd = VENDOR_AI_PROC_SCHD_FAIR;
		vendor_ai_cfg_set(VENDOR_AI_CFG_PLUGIN_ENGINE, vendor_ai_cpu1_get_engine());
		vendor_ai_cfg_set(VENDOR_AI_CFG_PROC_SCHD, &schd);
	}

	ret = vendor_ai_init();
	if (ret != HD_OK) {
		printf("vendor_ai_init fail=%d\n", ret);
		return ret;
	}

#if (USE_NET == TRUE)
	{
		int i;
		for (i = 0; i < 16; i++) {
			NET_PROC* p_net = g_net + i;
			p_net->proc_id = i;
		}
	}
#endif
	return ret;
}

static HD_RESULT network_uninit(void)
{
	HD_RESULT ret = HD_OK;
	
	ret = hd_videoproc_uninit();
	if (ret != HD_OK) {
		printf("hd_videoproc_uninit fail=%d\n", ret);
	}
	ret = vendor_ai_uninit();
	if (ret != HD_OK) {
		printf("vendor_ai_uninit fail=%d\n", ret);
	}
	
	return ret;
}

#if (USE_NET == TRUE)
INT32 network_mem_config(NET_PATH_ID net_path, HD_COMMON_MEM_INIT_CONFIG* p_mem_cfg, void* p_cfg, INT32 i)
{
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id;
	NET_PROC_CONFIG* p_proc_cfg = (NET_PROC_CONFIG*)p_cfg;
	p_net->proc_id = net_path;
	proc_id = p_net->proc_id;
	
	memcpy((void*)&p_net->net_cfg, (void*)p_proc_cfg, sizeof(NET_PROC_CONFIG));
	if (strlen(p_net->net_cfg.model_filename) == 0) {
		printf("proc_id(%u) input model is null\r\n", proc_id);
		return HD_ERR_NG;
	}

	p_net->net_cfg.binsize = _getsize_model(p_net->net_cfg.model_filename);
	if (p_net->net_cfg.binsize <= 0) {
		printf("proc_id(%u) input model is not exist?\r\n", proc_id);
		return HD_ERR_NG;
	}
	
	printf("proc_id(%u) set net_mem_cfg: model-file(%s), binsize=%d\r\n", 
		proc_id,
		p_net->net_cfg.model_filename,
		p_net->net_cfg.binsize);

	printf("proc_id(%u) set net_mem_cfg: label-file(%s)\r\n", 
		proc_id,
		p_net->net_cfg.label_filename);
	
	// config common pool (in)
	p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_USER_DEFINIED_POOL + proc_id;
	p_mem_cfg->pool_info[i].blk_size = p_net->net_cfg.binsize;
	p_mem_cfg->pool_info[i].blk_cnt = 1;
	p_mem_cfg->pool_info[i].ddr_id = DDR_ID0;
	i++;
	return i;
}

static HD_RESULT network_set_config(NET_PATH_ID net_path, NET_PROC_CONFIG* p_proc_cfg)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 binsize = p_net->net_cfg.binsize;
	UINT32 proc_id;
	p_net->proc_id = net_path;
	proc_id = p_net->proc_id;

	memcpy((void*)&p_net->net_cfg, (void*)p_proc_cfg, sizeof(NET_PROC_CONFIG));
	p_net->net_cfg.binsize = binsize;
	printf("proc_id(%u) set net_cfg: job-opt=(%u,%d), buf-opt(%u)\r\n", 
		proc_id,
		p_net->net_cfg.job_method,
		(int)p_net->net_cfg.job_wait_ms,
		p_net->net_cfg.buf_method);
	
	// set buf opt
	{
		VENDOR_AI_NET_CFG_BUF_OPT cfg_buf_opt = {0};
		cfg_buf_opt.method = p_net->net_cfg.buf_method;
		cfg_buf_opt.ddr_id = DDR_ID0;
		vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_BUF_OPT, &cfg_buf_opt);
	}

	// set job opt
	{
		VENDOR_AI_NET_CFG_JOB_OPT cfg_job_opt = {0};
		cfg_job_opt.method = p_net->net_cfg.job_method;
		cfg_job_opt.wait_ms = p_net->net_cfg.job_wait_ms;
		cfg_job_opt.schd_parm = VENDOR_AI_FAIR_CORE_ALL; //FAIR dispatch to ALL core
		vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_JOB_OPT, &cfg_job_opt);
	}

	return ret;
}

static HD_RESULT network_alloc_io_buf(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id = p_net->proc_id;
	VENDOR_AI_NET_CFG_WORKBUF wbuf = {0};

	ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_CFG_WORKBUF, &wbuf);
	if (ret != HD_OK) {
		printf("proc_id(%lu) get VENDOR_AI_NET_PARAM_CFG_WORKBUF fail\r\n", proc_id);
		return HD_ERR_FAIL;
	}
	ret = mem_alloc(&p_net->io_mem, "ai_io_buf", wbuf.size);
	if (ret != HD_OK) {
		printf("proc_id(%lu) alloc ai_io_buf fail\r\n", proc_id);
		return HD_ERR_FAIL;
	}

	wbuf.pa = p_net->io_mem.pa;
	wbuf.va = p_net->io_mem.va;
	wbuf.size = p_net->io_mem.size;
	ret = vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_WORKBUF, &wbuf);
	if (ret != HD_OK) {
		printf("proc_id(%lu) set VENDOR_AI_NET_PARAM_CFG_WORKBUF fail\r\n", proc_id);
		return HD_ERR_FAIL;
	}

	printf("alloc_io_buf: work buf, pa = %#lx, va = %#lx, size = %lu\r\n", wbuf.pa, wbuf.va, wbuf.size);

	return ret;
}

static HD_RESULT network_free_io_buf(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;

	if (p_net->io_mem.pa && p_net->io_mem.va) {
		mem_free(&p_net->io_mem);
	}
	return ret;
}

static HD_RESULT network_open(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id;
	p_net->proc_id = net_path;
	proc_id = p_net->proc_id;

	UINT32 loadsize = 0;

	if (strlen(p_net->net_cfg.model_filename) == 0) {
		printf("proc_id(%u) input model is null\r\n", proc_id);
		return 0;
	}

	mem_get(&p_net->proc_mem, p_net->net_cfg.binsize, proc_id);
	//load file
	loadsize = _load_model(p_net->net_cfg.model_filename, p_net->proc_mem.va);

	if (loadsize <= 0) {
		printf("proc_id(%u) input model load fail: %s\r\n", proc_id, p_net->net_cfg.model_filename);
		return 0;
	}

	// load label
	ret = _load_label((UINT32)p_net->out_class_labels, VENDOR_AIS_LBL_LEN, p_net->net_cfg.label_filename);
	if (ret != HD_OK) {
		printf("proc_id(%u) load_label fail=%d\n", proc_id, ret);
		return HD_ERR_FAIL;
	}

	// set model
	vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_MODEL, (VENDOR_AI_NET_CFG_MODEL*)&p_net->proc_mem);

	// open
	vendor_ai_net_open(proc_id);

	if ((ret = network_alloc_io_buf(net_path)) != HD_OK)
		return ret;
	
	return ret;
}

static HD_RESULT network_close(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id = p_net->proc_id;

	if ((ret = network_free_io_buf(net_path)) != HD_OK)
		return ret;
	
	// close
	ret = vendor_ai_net_close(proc_id);
	
	mem_rel(&p_net->proc_mem);

	return ret;
}

static HD_RESULT network_dump_out_buf(NET_PATH_ID net_path, void *p_out)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id = p_net->proc_id;
	VENDOR_AI_POSTPROC_RESULT_INFO *p_rslt_info = (VENDOR_AI_POSTPROC_RESULT_INFO *)(p_out);
	UINT32 i, j;
	
	printf("proc_id(%u) classification results:\r\n", proc_id);
	if (p_rslt_info) {
		for (i = 0; i < p_rslt_info->result_num; i++) {
			VENDOR_AI_POSTPROC_RESULT *p_rslt = &p_rslt_info->p_result[i];
			for (j = 0; j < NN_POSTPROC_TOP_N; j++) {
				printf("%ld. no=%ld, label=%s, score=%f\r\n",
					j + 1,
					p_rslt->no[j],
					&p_net->out_class_labels[p_rslt->no[j] * VENDOR_AIS_LBL_LEN],
					p_rslt->score[j]);
			}
		}
	}

	return ret;
}

#endif

///////////////////////////////////////////////////////////////////////////////
typedef struct _OP_PROC {

	UINT32 proc_id;
    int op_opt;
	MEM_PARM input_mem;
	MEM_PARM weight_mem;
	MEM_PARM output_mem;
#if ALLOC_WORKBUF_BY_USER
	MEM_PARM work_mem;
#endif
		
} OP_PROC;

static OP_PROC g_op[16] = {0};

static HD_RESULT op_init(void)
{
	HD_RESULT ret = HD_OK;
	int  i;	
	for (i = 0; i < 16; i++) {
		OP_PROC* p_op = g_op + i;
		p_op->proc_id = i;
	}
	return ret;
}

static HD_RESULT op_uninit(void)
{
	HD_RESULT ret = HD_OK;
	return ret;
}

static HD_RESULT operator_set_config(NET_PATH_ID net_path, int in_op_opt)
{
	HD_RESULT ret = HD_OK;
	OP_PROC* p_op = g_op + net_path;
    p_op->op_opt = in_op_opt;
	return ret;
}

static HD_RESULT operator_alloc_out_buf(NET_PATH_ID op_path, NET_PATH_ID in_path)
{
	HD_RESULT ret = HD_OK;
	OP_PROC* p_op = g_op + op_path;
	NET_IN* p_in = g_in + in_path;
	UINT32 proc_id = p_op->proc_id;
	
	// alloc result buff
    switch (p_op->op_opt) {
	    case AI_OP_FC: //VENDOR_AI_OP_FC
	    case AI_OP_FC_LL_MODE:
        {
	        ret = mem_alloc(&p_op->output_mem, "user_out_buf", SV_LENGTH*4);
	        if (ret != HD_OK) {
	       	    printf("proc_id(%u) alloc out_buf fail\r\n", proc_id);
	      	    return HD_ERR_FAIL;
	        }
            else {
                printf("proc_id(%u) alloc out_buf OK, size = %d\r\n", proc_id, SV_LENGTH*4);
            }
    	    mem_fill(&p_op->output_mem, 1); 
	        mem_save(&p_op->output_mem, "./user_out_ori.bin");
    	}
        break;
		case AI_OP_PREPROC_YUV2RGB_SCALE:
		{
			ret = mem_alloc(&p_op->output_mem, "user_out_buf", AI_RGB_BUFSIZE(SCALE_DIM_W, SCALE_DIM_H));
			if (ret != HD_OK) {
	        	printf("proc_id(%lu) alloc out_buf fail\r\n", proc_id);
	        	return HD_ERR_FAIL;
	        }
            else {
                printf("proc_id(%lu) alloc out_buf OK, size = %d\r\n", proc_id, AI_RGB_BUFSIZE(SCALE_DIM_W, SCALE_DIM_H));
            }
		}
		break;
		case AI_OP_PREPROC_YUV2RGB:
		case AI_OP_PREPROC_YUV2RGB_MEANSUB_PLANE:
		case AI_OP_PREPROC_YUV2RGB_MEANSUB_DC:
		case AI_OP_PREPROC_Y2Y_UV2UV:
        {
			if(p_op->op_opt == AI_OP_PREPROC_YUV2RGB_SCALE) {
            	ret = mem_alloc(&p_op->output_mem, "user_out_buf", AI_RGB_BUFSIZE(SCALE_DIM_W, SCALE_DIM_H));
				if (ret != HD_OK) {
	        		printf("proc_id(%u) alloc out_buf fail\r\n", proc_id);
	        		return HD_ERR_FAIL;
	        	}
            	else {
                	printf("proc_id(%u) alloc out_buf OK, size = %d\r\n", proc_id, AI_RGB_BUFSIZE(SCALE_DIM_W, SCALE_DIM_H));
            	}
			}
			else {
				ret = mem_alloc(&p_op->output_mem, "user_out_buf", AI_RGB_BUFSIZE(p_in->in_cfg.w, p_in->in_cfg.h));
				if (ret != HD_OK) {
	        		printf("proc_id(%u) alloc out_buf fail\r\n", proc_id);
	        		return HD_ERR_FAIL;
	        	}
				else {
                	printf("proc_id(%u) alloc out_buf OK, size = %d\r\n", proc_id, AI_RGB_BUFSIZE(p_in->in_cfg.w, p_in->in_cfg.h));
            	}
			}
        }
        break;
        default:
        break;
    }
	return ret;
}

#if ALLOC_WORKBUF_BY_USER
static HD_RESULT operator_alloc_work_buf(NET_PATH_ID op_path)
{
	HD_RESULT ret = HD_OK;
	OP_PROC* p_op = g_op + op_path;
	UINT32 proc_id = p_op->proc_id;
	VENDOR_AI_OP_CFG_MAX wmax;

	switch (p_op->op_opt) {
		case AI_OP_FC: 
        {
			wmax.op = VENDOR_AI_OP_FC;
			ret = vendor_ai_op_get(proc_id, VENDOR_AI_OP_PARAM_CFG_MAX, &wmax);
			if (ret != HD_OK) {
		   	    printf("proc_id(%u) get work_buf fail\r\n", proc_id);
		  	    return HD_ERR_FAIL;
		    }
			else {
				printf("proc_id(%u) work_buf size = %lu\r\n", proc_id, wmax.size);
			}
		}
		break;
		case AI_OP_PREPROC_YUV2RGB:
		case AI_OP_PREPROC_YUV2RGB_SCALE:
		case AI_OP_PREPROC_YUV2RGB_MEANSUB_PLANE:
		case AI_OP_PREPROC_YUV2RGB_MEANSUB_DC:
		case AI_OP_PREPROC_Y2Y_UV2UV:
		{
			wmax.op = VENDOR_AI_OP_PREPROC;
			ret = vendor_ai_op_get(proc_id, VENDOR_AI_OP_PARAM_CFG_MAX, &wmax);
			if (ret != HD_OK) {
		   	    printf("proc_id(%u) get work_buf fail\r\n", proc_id);
		  	    return HD_ERR_FAIL;
		    }
			else {
				printf("proc_id(%u) work_buf size = %lu\r\n", proc_id, wmax.size);
			}
		}
		break;
		case AI_OP_FC_LL_MODE:
		{
			wmax.op = VENDOR_AI_OP_LIST;
			wmax.max_param[0] = SV_LENGTH;
			ret = vendor_ai_op_get(proc_id, VENDOR_AI_OP_PARAM_CFG_MAX, &wmax);
			if (ret != HD_OK) {
		   	    printf("proc_id(%u) get work_buf fail\r\n", proc_id);
		  	    return HD_ERR_FAIL;
		    }
			else {
				printf("proc_id(%u) work_buf size = %lu\r\n", proc_id, wmax.size);
			}
		}
		break;
	}
	
	// alloc work buff
    ret = mem_alloc(&p_op->work_mem, "op_work_buf", wmax.size);
    if (ret != HD_OK) {
   	    printf("proc_id(%u) alloc work_buf fail\r\n", proc_id);
  	    return HD_ERR_FAIL;
    }
            
	return ret;
}

static HD_RESULT operator_free_work_buf(NET_PATH_ID op_path)
{
	HD_RESULT ret = HD_OK;
	OP_PROC* p_op = g_op + op_path;
	
	// free work buff
	mem_free(&p_op->work_mem);
	
	return ret;
}

#endif

static HD_RESULT operator_free_out_buf(NET_PATH_ID op_path)
{
	HD_RESULT ret = HD_OK;
	OP_PROC* p_op = g_op + op_path;
	
	// free result buff
	mem_free(&p_op->output_mem);
	
	return ret;
}

static HD_RESULT operator_open(NET_PATH_ID op_path, NET_PATH_ID in_path)
{
	HD_RESULT ret = HD_OK;
	OP_PROC* p_op = g_op + op_path;
	NET_IN* p_in = g_in + in_path;
	UINT32 proc_id = p_op->proc_id;
	VENDOR_AI_OP_CFG_WORKBUF wbuf = {0};

	// alloc buffer
	switch (p_op->op_opt) {
		case AI_OP_FC: 
		case AI_OP_FC_LL_MODE:
        {
	        ret = mem_alloc(&p_op->input_mem, "user_in_buf", SV_FEA_LENGTH);
	        if (ret != HD_OK) {
	        	printf("proc_id(%u) alloc in_buf fail\r\n", proc_id);
	        	return HD_ERR_FAIL;
	        }
	        ret = mem_alloc(&p_op->weight_mem, "user_weight_buf", SV_LENGTH*SV_FEA_LENGTH);
	        if (ret != HD_OK) {
	        	printf("proc_id(%u) alloc weight_buf fail\r\n", proc_id);
	        	return HD_ERR_FAIL;
	        }

	        // fill buffer
	        mem_fill(&p_op->input_mem, 1); 
	        mem_fill(&p_op->weight_mem, 1); 
	        // save buffer
	        mem_save(&p_op->input_mem, "./user_in.bin"); 
	        mem_save(&p_op->weight_mem, "./user_weight.bin");
		}
        break;
		case AI_OP_PREPROC_YUV2RGB:
		case AI_OP_PREPROC_YUV2RGB_SCALE:
		case AI_OP_PREPROC_YUV2RGB_MEANSUB_PLANE:
		case AI_OP_PREPROC_YUV2RGB_MEANSUB_DC:
		case AI_OP_PREPROC_Y2Y_UV2UV:
        {
			if(p_op->op_opt == AI_OP_PREPROC_YUV2RGB_MEANSUB_PLANE)
            	ret = mem_alloc(&p_op->input_mem, "user_in_buf", 2*AI_RGB_BUFSIZE(p_in->in_cfg.w, p_in->in_cfg.h));
			else
				ret = mem_alloc(&p_op->input_mem, "user_in_buf", AI_RGB_BUFSIZE(p_in->in_cfg.w, p_in->in_cfg.h));
			if (ret != HD_OK) {
	    	    printf("proc_id(%lu) alloc in_buf fail\r\n", proc_id);
	    	    return HD_ERR_FAIL;
	        }
            
            INT32 file_len;
            file_len = mem_load(&p_op->input_mem, p_in->in_cfg.input_filename);
	        if (file_len < 0) {
		        printf("load buf(%s) fail\r\n", p_in->in_cfg.input_filename);
		        return HD_ERR_NG;
	        }
        	printf("load buf(%s) ok, size = %d\r\n", p_in->in_cfg.input_filename, file_len);
        }
        break;
        default:
        {
            printf("Unknown op_opt");
	        return HD_ERR_LIMIT;
        }
        break;
	}

	// open
	ret = vendor_ai_op_open(op_path);
	
#if ALLOC_WORKBUF_BY_USER
	//alloc work buffer
	ret = operator_alloc_work_buf(op_path);

	//set work buffer
	wbuf.pa = (&p_op->work_mem)->pa;
	wbuf.va = (&p_op->work_mem)->va;
	wbuf.size = (&p_op->work_mem)->size;
	switch (p_op->op_opt) {
		case AI_OP_FC: 
        {
			wbuf.op = VENDOR_AI_OP_FC;
		}
		break;
		case AI_OP_PREPROC_YUV2RGB:
		case AI_OP_PREPROC_YUV2RGB_SCALE:
		case AI_OP_PREPROC_YUV2RGB_MEANSUB_PLANE:
		case AI_OP_PREPROC_YUV2RGB_MEANSUB_DC:
		case AI_OP_PREPROC_Y2Y_UV2UV:
        {
			wbuf.op = VENDOR_AI_OP_PREPROC;
		}
		break;
		case AI_OP_FC_LL_MODE:
        {
			wbuf.op = VENDOR_AI_OP_LIST;
		}
		break;
	}
	ret = vendor_ai_op_set(proc_id, VENDOR_AI_OP_PARAM_CFG_WORKBUF, &wbuf);
#endif
	
	//start
	ret = vendor_ai_op_start(op_path);

	return ret;
}

static HD_RESULT operator_close(NET_PATH_ID op_path)
{
	HD_RESULT ret = HD_OK;
	OP_PROC* p_op = g_op + op_path;

	//stop
	ret = vendor_ai_op_stop(op_path);

#if ALLOC_WORKBUF_BY_USER
	//free work buf
	ret = operator_free_work_buf(op_path);
#endif

	// close	
	ret = vendor_ai_op_close(op_path);
    // free buffer
	switch (p_op->op_opt) {
		case AI_OP_FC: //VENDOR_AI_OP_FC
		case AI_OP_FC_LL_MODE:
        {
			mem_free(&p_op->input_mem);
			mem_free(&p_op->weight_mem);
		}
        break;
		case AI_OP_PREPROC_YUV2RGB:
		case AI_OP_PREPROC_YUV2RGB_SCALE:
		case AI_OP_PREPROC_YUV2RGB_MEANSUB_PLANE:
		case AI_OP_PREPROC_YUV2RGB_MEANSUB_DC:
		case AI_OP_PREPROC_Y2Y_UV2UV:
        {
            mem_free(&p_op->input_mem);
        }
        break;
        default:
        break;
	}
	return ret;
}


///////////////////////////////////////////////////////////////////////////////

typedef struct _VIDEO_LIVEVIEW {

	// (1) input 
	NET_IN_CONFIG net_in_cfg;
	NET_PATH_ID in_path;

#if (USE_NET == TRUE)
	// (2) network 
	NET_PROC_CONFIG net_proc_cfg;
	NET_PATH_ID net_path;
	pthread_t  proc_thread_id;
	UINT32 proc_start;
	UINT32 proc_exit;
	UINT32 proc_oneshot;
#endif

	// (3) operator
	NET_PATH_ID op_path;
	int net_op_opt;
	pthread_t  op_thread_id;
	UINT32 op_start;
	UINT32 op_exit;
	UINT32 op_oneshot;

} VIDEO_LIVEVIEW;

static HD_RESULT init_module(void)
{
	HD_RESULT ret;
	if ((ret = op_init()) != HD_OK)
		return ret;
#if (USE_NET == TRUE)
	if ((ret = input_init()) != HD_OK)
		return ret;
#endif
	if ((ret = network_init()) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT open_module(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;
#if (USE_NET == TRUE)
	if ((ret = input_open(p_stream->in_path)) != HD_OK)
		return ret;
	if ((ret = network_open(p_stream->net_path)) != HD_OK)
		return ret;
#endif
	if ((ret = operator_open(p_stream->op_path, p_stream->in_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT close_module(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;
#if (USE_NET == TRUE)
	if ((ret = input_close(p_stream->in_path)) != HD_OK)
		return ret;
	if ((ret = network_close(p_stream->net_path)) != HD_OK)
		return ret;
#endif
	if ((ret = operator_close(p_stream->op_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT exit_module(void)
{
	HD_RESULT ret;
	if ((ret = op_uninit()) != HD_OK)
		return ret;
#if (USE_NET == TRUE)
	if ((ret = input_uninit()) != HD_OK)
		return ret;
#endif
	if ((ret = network_uninit()) != HD_OK)
		return ret;
	return HD_OK;
}
///////////////////////////////////////////////////////////////////////////////

#if (USE_NET == TRUE)
static VOID *network_user_thread(VOID *arg);

static HD_RESULT network_user_start(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;
	p_stream->proc_start = 0;
	p_stream->proc_exit = 0;
	p_stream->proc_oneshot = 0;
	
	ret = vendor_ai_net_start(p_stream->net_path);
	if (HD_OK != ret) {
		printf("proc_id(%u) start fail !!\n", p_stream->net_path);
	}
	
	ret = pthread_create(&p_stream->proc_thread_id, NULL, network_user_thread, (VOID*)(p_stream));
	if (ret < 0) {
		return HD_ERR_FAIL;
	}

	p_stream->proc_start = 1;
	p_stream->proc_exit = 0;
	p_stream->proc_oneshot = 0;
	return ret;
}

static HD_RESULT network_user_oneshot(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;
	p_stream->proc_oneshot = 1;
	return ret;
}

static HD_RESULT network_user_stop(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;
	p_stream->proc_exit = 1;
	
	if (p_stream->proc_thread_id) {
		pthread_join(p_stream->proc_thread_id, NULL);
	}

	//stop: should be call after last time proc
	ret = vendor_ai_net_stop(p_stream->net_path);
	if (HD_OK != ret) {
		printf("proc_id(%u) stop fail !!\n", p_stream->net_path);
	}
	
	return ret;
}

static VOID *network_user_thread(VOID *arg)
{
	HD_RESULT ret = HD_OK;
	
	VIDEO_LIVEVIEW *p_stream = (VIDEO_LIVEVIEW*)arg;

	printf("\r\n");
	while (p_stream->proc_start == 0) sleep(1);

	printf("\r\n");
	while (p_stream->proc_exit == 0) {

		if (p_stream->proc_oneshot) {
			
			VENDOR_AI_BUF	in_buf = {0};
			//VENDOR_AI_BUF	out_buf = {0};
			VENDOR_AI_POSTPROC_RESULT_INFO out_buf = {0};

			p_stream->proc_oneshot = 0;
			
			ret = input_pull_buf(p_stream->in_path, &in_buf, 0);
			if (HD_OK != ret) {
				printf("proc_id(%u) pull input fail !!\n", p_stream->in_path);
				goto skip;
			}

			printf("proc_id(%u) oneshot ...\n", p_stream->net_path);
			// set input image
			ret = vendor_ai_net_set(p_stream->net_path, VENDOR_AI_NET_PARAM_IN(0, 0), &in_buf);
			if (HD_OK != ret) {
				printf("proc_id(%u)push input fail !!\n", p_stream->net_path);
				goto skip;
			}

			// do net proc
			ret = vendor_ai_net_proc(p_stream->net_path);
			if (HD_OK != ret) {
				printf("proc_id(%u) proc fail !!\n", p_stream->net_path);
				goto skip;
			}

			// get output result
			ret = vendor_ai_net_get(p_stream->net_path, VENDOR_AI_NET_PARAM_OUT(VENDOR_AI_MAXLAYER, 0), &out_buf);
			if (HD_OK != ret) {
				printf("proc_id(%u) output get fail !!\n", p_stream->net_path);
				goto skip;
			}
			printf("proc_id(%u) oneshot done!\n", p_stream->net_path);
			ret = network_dump_out_buf(p_stream->net_path, &out_buf);
			if (HD_OK != ret) {
				printf("proc_id(%u) output dump fail !!\n", p_stream->net_path);
				goto skip;
			}
		}
		usleep(100);
	}
	
skip:

	return 0;
}
#endif


///////////////////////////////////////////////////////////////////////////////

static VOID *operator_user_thread(VOID *arg);

static HD_RESULT operator_user_start(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;

	p_stream->op_start = 0;
	p_stream->op_exit = 0;
	p_stream->op_oneshot = 0;
	
	ret = pthread_create(&p_stream->op_thread_id, NULL, operator_user_thread, (VOID*)(p_stream));
	if (ret < 0) {
		return HD_ERR_FAIL;
	}

	p_stream->op_start = 1;
	p_stream->op_exit = 0;
	p_stream->op_oneshot = 0;
	
	return ret;
}

static HD_RESULT operator_user_oneshot(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;
	p_stream->op_oneshot = 1;
	return ret;
}

static HD_RESULT operator_user_stop(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;
	p_stream->op_exit = 1;
	
	pthread_join(p_stream->op_thread_id, NULL);

	return ret;
}

static VOID *operator_user_thread(VOID *arg)
{
	HD_RESULT ret = HD_OK;
	
	VIDEO_LIVEVIEW *p_stream = (VIDEO_LIVEVIEW*)arg;
	OP_PROC* p_op = g_op + p_stream->op_path;
	NET_IN* p_in = g_in + p_stream->in_path;

	printf("\r\n");
	while (p_stream->op_start == 0) sleep(1);

	printf("\r\n");
	ret = operator_alloc_out_buf(p_stream->op_path, p_stream->in_path);
	if (HD_OK != ret) {
		printf("proc_id(%u) alloc output fail !!\n", p_stream->op_path);
		goto skip;
	}
	
	printf("\r\n");
    
    switch (p_op->op_opt) {
		case AI_OP_FC: 
        {
	        while (p_stream->op_exit == 0) {

	        	if (p_stream->op_oneshot) {
            

	                // 2. flush input
	        		ret = hd_common_mem_flush_cache((VOID *)(&p_op->input_mem)->va, (&p_op->input_mem)->size);
                    if(HD_OK != ret) {
                        printf("flush cache failed.\n");
                    }
	        		ret = hd_common_mem_flush_cache((VOID *)(&p_op->weight_mem)->va, (&p_op->weight_mem)->size);
                    if(HD_OK != ret) {
                        printf("flush cache failed.\n");
                    }
	        		ret = hd_common_mem_flush_cache((VOID *)(&p_op->output_mem)->va, (&p_op->output_mem)->size);
                    if(HD_OK != ret) {
                        printf("flush cache failed.\n");
                    }
		        	// 3. run OP
		        	{
		        		/*
		        		The code below is the flow of using FC 
		        		suppose the input feature size is 256 bytes (defined as SV_FEA_LENGTH)
		        		and the desired output length is 10240 (defined as SV_LENGTH)
		        		the following sample will transpose the input 256 bytes feature (1 byte per element) into 10240*4 bytes feature (4 bytes per element)
		        		
		        		fc_init_param is for setting parameter of FC
			        	user should set input/output/weight address
			        	*/
			        	VENDOR_AI_BUF src[2] = {0};
			        	VENDOR_AI_BUF dest[1] = {0};
			        	MEM_PARM* in_buf = &p_op->input_mem;
			        	MEM_PARM* out_buf = &p_op->output_mem;
			        	MEM_PARM* weight_buf = &p_op->weight_mem;
		        		//pprintf("input addr pa = 0x%08X\n", (unsigned int)(in_buf->pa));
			        	//pprintf("output addr pa = 0x%08X\n", (unsigned int)(out_buf->pa));
			        	//pprintf("weight addr pa = 0x%08X\n", (unsigned int)(weight_buf->pa));
				        
		          		//set src1 as 1d tensor
			        	src[0].sign = MAKEFOURCC('A','B','U','F');
			        	src[0].ddr_id = 0;
			        	src[0].va = in_buf->va; //< input address	 (size = SV_FEA_LENGTH bytes)
			        	src[0].pa = in_buf->pa;
			        	src[0].size = SV_FEA_LENGTH;
			        	src[0].fmt = HD_VIDEO_PXLFMT_AI_SFIXED8(0); //fixpoint s7.0
			        	src[0].width = SV_FEA_LENGTH;
			        	src[0].height = 1;
			        	src[0].channel = 1;
			        	src[0].batch_num = 1;
			        	
		        		//set src2 as 2d tensor
		        		src[1].sign = MAKEFOURCC('A','B','U','F');
		        		src[1].ddr_id = 0;
		        		src[1].va = weight_buf->va; //< sv weight address (size = SV_LENGTH*SV_FEA_LENGTH bytes)
		        		src[1].pa = weight_buf->pa;
		        		src[1].size = SV_FEA_LENGTH*SV_LENGTH;
		        		src[1].fmt = HD_VIDEO_PXLFMT_AI_SFIXED8(0); //fixpoint s7.0
			        	src[1].width = SV_FEA_LENGTH;
			        	src[1].height = SV_LENGTH;
			        	src[1].channel = 1;
			        	src[1].batch_num = 1;
		        		
		        		//set dest as 1d tensor
			        	dest[0].sign = MAKEFOURCC('A','B','U','F');
		        		dest[0].ddr_id = 0;
			        	dest[0].va = out_buf->va; //< output address	 (size = SV_LENGTH*4 bytes)
			        	dest[0].pa = out_buf->pa;
			        	dest[0].size = SV_LENGTH*4;
			        	dest[0].fmt = HD_VIDEO_PXLFMT_AI_SFIXED32(0); //fixpoint s31.0
			        	dest[0].width = SV_LENGTH;
			        	dest[0].height = 1;
			        	dest[0].channel = 1;
			        	dest[0].batch_num = 1;

			        	ret = vendor_ai_op_proc(p_stream->op_path, VENDOR_AI_OP_FC, NULL, 2, src, 1, dest);
		        	}
		        	if (ret != 0) {
		        		printf("op inference fail\n");
		        		return 0;
		        	}
        
		        	p_stream->op_oneshot = FALSE;
		                	
		        	printf("inference done!\n");
		        	ret = hd_common_mem_flush_cache((VOID *)(&p_op->output_mem)->va, (&p_op->output_mem)->size);
                  if(HD_OK != ret) {
                        printf("flush cache failed.\n");
                    }
        
		        	mem_save(&p_op->output_mem, "./op_user_out.bin");
        
		        }
		        usleep(100);
	        }
		}
        break;
		case AI_OP_FC_LL_MODE: 
        {
	        while (p_stream->op_exit == 0) {

	        	if (p_stream->op_oneshot) {
            

	                // 2. flush input
	        		ret = hd_common_mem_flush_cache((VOID *)(&p_op->input_mem)->va, (&p_op->input_mem)->size);
                    if(HD_OK != ret) {
                        printf("flush cache failed.\n");
                    }
	        		ret = hd_common_mem_flush_cache((VOID *)(&p_op->weight_mem)->va, (&p_op->weight_mem)->size);
                    if(HD_OK != ret) {
                        printf("flush cache failed.\n");
                    }
	        		ret = hd_common_mem_flush_cache((VOID *)(&p_op->output_mem)->va, (&p_op->output_mem)->size);
                    if(HD_OK != ret) {
                        printf("flush cache failed.\n");
                    }
		        	// 3. run OP
		        	{
		        		/*
		        		The code below is the flow of using FC 
		        		suppose the input feature size is 256 bytes (defined as SV_FEA_LENGTH)
		        		and the desired output length is 10240 (defined as SV_LENGTH)
		        		the following sample will transpose the input 256 bytes feature (1 byte per element) into 10240*4 bytes feature (4 bytes per element)
		        		
		        		fc_init_param is for setting parameter of FC
			        	user should set input/output/weight address
			        	*/
			        	VENDOR_AI_BUF src[2] = {0};
			        	VENDOR_AI_BUF dest[1] = {0};
			        	MEM_PARM* in_buf = &p_op->input_mem;
			        	MEM_PARM* out_buf = &p_op->output_mem;
			        	MEM_PARM* weight_buf = &p_op->weight_mem;
		        		//pprintf("input addr pa = 0x%08X\n", (unsigned int)(in_buf->pa));
			        	//pprintf("output addr pa = 0x%08X\n", (unsigned int)(out_buf->pa));
			        	//pprintf("weight addr pa = 0x%08X\n", (unsigned int)(weight_buf->pa));
				        
		          		//set src1 as 1d tensor
			        	src[0].sign = MAKEFOURCC('A','B','U','F');
			        	src[0].ddr_id = 0;
			        	src[0].va = in_buf->va; //< input address	 (size = SV_FEA_LENGTH bytes)
			        	src[0].pa = in_buf->pa; //must 4 bytes align!
			        	src[0].size = SV_FEA_LENGTH;
			        	src[0].fmt = HD_VIDEO_PXLFMT_AI_SFIXED8(0); //fixpoint s7.0
			        	src[0].width = SV_FEA_LENGTH;
			        	src[0].height = 1;
			        	src[0].channel = 1;
			        	src[0].batch_num = 1;
			        	
		        		//set src2 as 2d tensor
		        		src[1].sign = MAKEFOURCC('A','B','U','F');
		        		src[1].ddr_id = 0;
		        		src[1].va = weight_buf->va; //< sv weight address (size = SV_LENGTH*SV_FEA_LENGTH bytes)
		        		src[1].pa = weight_buf->pa; //must 4 bytes align!
		        		src[1].size = SV_FEA_LENGTH*SV_LENGTH;
		        		src[1].fmt = HD_VIDEO_PXLFMT_AI_SFIXED8(0); //fixpoint s7.0
			        	src[1].width = SV_FEA_LENGTH;
			        	src[1].height = SV_LENGTH;
			        	src[1].channel = 1;
			        	src[1].batch_num = 1;
		        		
		        		//set dest as 1d tensor
			        	dest[0].sign = MAKEFOURCC('A','B','U','F');
		        		dest[0].ddr_id = 0;
			        	dest[0].va = out_buf->va; //< output address	 (size = SV_LENGTH*4 bytes)
			        	dest[0].pa = out_buf->pa; //must 4 bytes align!
			        	dest[0].size = SV_LENGTH*4;
			        	dest[0].fmt = HD_VIDEO_PXLFMT_AI_SFIXED32(0); //fixpoint s31.0
			        	dest[0].width = SV_LENGTH;
			        	dest[0].height = 1;
			        	dest[0].channel = 1;
			        	dest[0].batch_num = 1;

						ret = vendor_ai_op_add(p_stream->op_path, VENDOR_AI_OP_LIST, NULL, 2, src, 1, dest);
						if (ret != HD_OK) {
					   	    printf("proc_id(%u) vendor_ai_op_add fail\r\n", p_stream->op_path);
					    }
						
						ret = hd_common_mem_flush_cache((VOID *)(&p_op->work_mem)->va, (&p_op->work_mem)->size);
                    	if(HD_OK != ret) {
                        	printf("flush cache failed.\n");
                    	}
						
			        	ret = vendor_ai_op_proc(p_stream->op_path, VENDOR_AI_OP_LIST, NULL, 0, NULL, 0, NULL);
						if (ret != HD_OK) {
					   	    printf("proc_id(%u) vendor_ai_op_proc for run fc ll fail\r\n", p_stream->op_path);
    					}
						
		        	}
        
		        	p_stream->op_oneshot = FALSE;
		                	
		        	printf("inference done!\n");
		        	ret = hd_common_mem_flush_cache((VOID *)(&p_op->output_mem)->va, (&p_op->output_mem)->size);
                  if(HD_OK != ret) {
                        printf("flush cache failed.\n");
                    }
        
		        	mem_save(&p_op->output_mem, "./op_user_out.bin"); 
        
		        }
		        usleep(100);
	        }		
		}
        break;
		/*current support preproc function
		[format]
		Y -> Y
		UV -> UV
		RGB -> RGB
		YUV -> RGB
		[param]
		meansub plane mode: (using p_out_sub in VENDOR_AI_OP_PREPROC_PARAM) 
		meansub dc mode: (using p_out_sub in VENDOR_AI_OP_PREPROC_PARAM)
		Bilinear Scaling down: (using scale_dim in VENDOR_AI_OP_PREPROC_PARAM)
		*/
        case AI_OP_PREPROC_YUV2RGB:
		case AI_OP_PREPROC_YUV2RGB_SCALE:
		case AI_OP_PREPROC_YUV2RGB_MEANSUB_PLANE:
		case AI_OP_PREPROC_YUV2RGB_MEANSUB_DC: 
        {
            while (p_stream->op_exit == 0) {

	        	if (p_stream->op_oneshot) {
                    ret = hd_common_mem_flush_cache((VOID *)(&p_op->input_mem)->va, (&p_op->input_mem)->size);
                    if(HD_OK != ret) {
                        printf("flush cache failed.\n");
                    }
                    ret = hd_common_mem_flush_cache((VOID *)(&p_op->output_mem)->va, (&p_op->output_mem)->size);
                    if(HD_OK != ret) {
                        printf("flush cache failed.\n");
                    }
                    // 3. run OP
		        	{
                        VENDOR_AI_BUF src[2] = {0};
			        	VENDOR_AI_BUF dest[3] = {0};
			        	MEM_PARM* in_buf = &p_op->input_mem;
			        	MEM_PARM* out_buf = &p_op->output_mem;
                        NET_IN_CONFIG in_cfg = p_in->in_cfg;
                        VENDOR_AI_OP_PREPROC_PARAM p_parm = {0};

                        //set src1 as 1d tensor
						src[0].sign = MAKEFOURCC('A','B','U','F');
						src[0].ddr_id = 0;
						src[0].va = in_buf->va; //< input address	 
						src[0].pa = in_buf->pa; //must 2 bytes align!
						src[0].size = in_cfg.loff * in_cfg.h;
						src[0].fmt = HD_VIDEO_PXLFMT_Y8;
						src[0].width = in_cfg.w;
						src[0].height = in_cfg.h;
						src[0].line_ofs = in_cfg.loff;
						src[0].channel = 1;
						src[0].batch_num = 1;

                        //set src2 as 1d tensor
						src[1].sign = MAKEFOURCC('A','B','U','F');
						src[1].ddr_id = 0;
						src[1].va = in_buf->va + src[0].size; //< input address	 
						src[1].pa = in_buf->pa + src[0].size; //must 2 bytes align!
						src[1].size = in_cfg.loff * in_cfg.h;
						src[1].fmt = HD_VIDEO_PXLFMT_UV; 
						src[1].width = in_cfg.w;
						src[1].height = in_cfg.h;
						src[1].line_ofs = in_cfg.loff;
						src[1].channel = 1;
						src[1].batch_num = 1;

						if(p_op->op_opt == AI_OP_PREPROC_YUV2RGB_SCALE) {
							//set dest1 as 1d tensor
							dest[0].sign = MAKEFOURCC('A','B','U','F');
							dest[0].ddr_id = 0;
							dest[0].va = out_buf->va; //< output address	 
							dest[0].pa = out_buf->pa;
							dest[0].size = SCALE_DIM_W * SCALE_DIM_H;
							dest[0].fmt = HD_VIDEO_PXLFMT_R8;
							dest[0].width = SCALE_DIM_W;
							dest[0].height = SCALE_DIM_H;
							dest[0].line_ofs = SCALE_DIM_W;
							dest[0].channel = 1;
							dest[0].batch_num = 1;

							//set dest2 as 1d tensor
							dest[1].sign = MAKEFOURCC('A','B','U','F');
							dest[1].ddr_id = 0;
							dest[1].va = out_buf->va + dest[0].size; //< output address	 
							dest[1].pa = out_buf->pa + dest[0].size;
							dest[1].size = SCALE_DIM_W * SCALE_DIM_H;
							dest[1].fmt = HD_VIDEO_PXLFMT_G8;
							dest[1].width = SCALE_DIM_W;
							dest[1].height = SCALE_DIM_H;
							dest[1].line_ofs = SCALE_DIM_W;
							dest[1].channel = 1;
							dest[1].batch_num = 1;

							//set dest3 as 1d tensor
							dest[2].sign = MAKEFOURCC('A','B','U','F');
							dest[2].ddr_id = 0;
							dest[2].va = out_buf->va + 2*dest[0].size; //< output address		 
							dest[2].pa = out_buf->pa + 2*dest[0].size;
							dest[2].size = SCALE_DIM_W * SCALE_DIM_H;
							dest[2].fmt = HD_VIDEO_PXLFMT_B8;
							dest[2].width = SCALE_DIM_W;
							dest[2].height = SCALE_DIM_H;
							dest[2].line_ofs = SCALE_DIM_W;
							dest[2].channel = 1;
							dest[2].batch_num = 1;
						}
						else {
							//set dest1 as 1d tensor
							dest[0].sign = MAKEFOURCC('A','B','U','F');
							dest[0].ddr_id = 0;
							dest[0].va = out_buf->va; //< output address	 
							dest[0].pa = out_buf->pa;
							dest[0].size = in_cfg.loff * in_cfg.h;
							dest[0].fmt = HD_VIDEO_PXLFMT_R8;
							dest[0].width = in_cfg.w;
							dest[0].height = in_cfg.h;
							dest[0].line_ofs = in_cfg.w;
							dest[0].channel = 1;
							dest[0].batch_num = 1;

							//set dest2 as 1d tensor
							dest[1].sign = MAKEFOURCC('A','B','U','F');
							dest[1].ddr_id = 0;
							dest[1].va = out_buf->va + dest[0].size; //< output address	 
							dest[1].pa = out_buf->pa + dest[0].size;
							dest[1].size = in_cfg.loff * in_cfg.h;
							dest[1].fmt = HD_VIDEO_PXLFMT_G8;
							dest[1].width = in_cfg.w;
							dest[1].height = in_cfg.h;
							dest[1].line_ofs = in_cfg.w;
							dest[1].channel = 1;
							dest[1].batch_num = 1;

							//set dest3 as 1d tensor
							dest[2].sign = MAKEFOURCC('A','B','U','F');
							dest[2].ddr_id = 0;
							dest[2].va = out_buf->va + 2*dest[0].size; //< output address		 
							dest[2].pa = out_buf->pa + 2*dest[0].size;
							dest[2].size = in_cfg.loff * in_cfg.h;
							dest[2].fmt = HD_VIDEO_PXLFMT_B8;
							dest[2].width = in_cfg.w;
							dest[2].height = in_cfg.h;
							dest[2].line_ofs = in_cfg.w;
							dest[2].channel = 1;
							dest[2].batch_num = 1;
						}

                        // set func parameter

						//scale
						if (p_op->op_opt == AI_OP_PREPROC_YUV2RGB_SCALE) {
							p_parm.scale_dim.w = SCALE_DIM_W;
							p_parm.scale_dim.h = SCALE_DIM_H;
						}				
						
                        // plane mode
                        if (p_op->op_opt == AI_OP_PREPROC_YUV2RGB_MEANSUB_PLANE) {
							memset((VOID *)(in_buf->va + src[0].size*3), 0x80808080, src[0].size*3);   //clear buffer for sub
							ret = hd_common_mem_flush_cache((VOID *)(in_buf->va + src[0].size*3), src[0].size*3);
							if(HD_OK != ret) {
								printf("flush cache failed.\n");
							}
							p_parm.p_out_sub.pa = in_buf->pa + 3*src[0].size;
							p_parm.p_out_sub.va = in_buf->va + 3*src[0].size;                       
							p_parm.p_out_sub.width = in_cfg.w;
							p_parm.p_out_sub.height = in_cfg.h;
							p_parm.p_out_sub.line_ofs = in_cfg.w*3;
                        }

						// dc mode
						if (p_op->op_opt == AI_OP_PREPROC_YUV2RGB_MEANSUB_DC) {                 
							p_parm.out_sub_color[0] = 128;
							p_parm.out_sub_color[1] = 127;
							p_parm.out_sub_color[2] = 126;
						}
         
                        ret = vendor_ai_op_proc(p_stream->op_path, VENDOR_AI_OP_PREPROC, &p_parm, 2, src, 3, dest);

                    }
                    if (ret != 0) {
		        		printf("op inference fail\n");
		        		return 0;
		        	}
        
		        	p_stream->op_oneshot = FALSE;
		                	
		        	printf("inference done!\n");
		        	ret = hd_common_mem_flush_cache((VOID *)(&p_op->output_mem)->va, (&p_op->output_mem)->size);
                    if(HD_OK != ret) {
                        printf("flush cache failed.\n");
                    }

                    mem_save(&p_op->output_mem, "./op_user_out.bin");
					

	        	}
                usleep(100);
            }
        }
        break;
		case AI_OP_PREPROC_Y2Y_UV2UV: 
        {
            while (p_stream->op_exit == 0) {

	        	if (p_stream->op_oneshot) {
                    ret = hd_common_mem_flush_cache((VOID *)(&p_op->input_mem)->va, (&p_op->input_mem)->size);
                    if(HD_OK != ret) {
                        printf("flush cache failed.\n");
                    }
                    ret = hd_common_mem_flush_cache((VOID *)(&p_op->output_mem)->va, (&p_op->output_mem)->size);
                    if(HD_OK != ret) {
                        printf("flush cache failed.\n");
                    }
                    // 3. run OP
		        	
                        VENDOR_AI_BUF src[2] = {0};
			        	VENDOR_AI_BUF dest[2] = {0};
			        	MEM_PARM* in_buf = &p_op->input_mem;
			        	MEM_PARM* out_buf = &p_op->output_mem;
                        NET_IN_CONFIG in_cfg = p_in->in_cfg;
                        VENDOR_AI_OP_PREPROC_PARAM p_parm = {0};

                        //set src1 as 1d tensor
						src[0].sign = MAKEFOURCC('A','B','U','F');
						src[0].ddr_id = 0;
						src[0].va = in_buf->va; //< input address	 
						src[0].pa = in_buf->pa;
						src[0].size = in_cfg.loff * in_cfg.h;
						src[0].fmt = HD_VIDEO_PXLFMT_Y8;
						src[0].width = in_cfg.w;
						src[0].height = in_cfg.h;
						src[0].line_ofs = in_cfg.loff;
						src[0].channel = 1;
						src[0].batch_num = 1;

                        //set src2 as 1d tensor
						src[1].sign = MAKEFOURCC('A','B','U','F');
						src[1].ddr_id = 0;
						src[1].va = in_buf->va + src[0].size; //< input address	 
						src[1].pa = in_buf->pa + src[0].size; //must 2 bytes align!
						src[1].size = in_cfg.loff * in_cfg.h;
						src[1].fmt = HD_VIDEO_PXLFMT_UV; 
						src[1].width = in_cfg.w/2;
						src[1].height = in_cfg.h/2;
						src[1].line_ofs = in_cfg.w;
						src[1].channel = 1;
						src[1].batch_num = 1;

						//set dest1 as 1d tensor
						dest[0].sign = MAKEFOURCC('A','B','U','F');
						dest[0].ddr_id = 0;
						dest[0].va = out_buf->va; //< output address	 
						dest[0].pa = out_buf->pa;
						dest[0].size = SCALE_DIM_W * SCALE_DIM_H;
						dest[0].fmt = HD_VIDEO_PXLFMT_Y8;
						dest[0].width = SCALE_DIM_W;
						dest[0].height = SCALE_DIM_H;
						dest[0].line_ofs = SCALE_DIM_W;
						dest[0].channel = 1;
						dest[0].batch_num = 1;

						//set dest2 as 1d tensor
						dest[1].sign = MAKEFOURCC('A','B','U','F');
						dest[1].ddr_id = 0;
						dest[1].va = out_buf->va + dest[0].size; //< output address	 
						dest[1].pa = out_buf->pa + dest[0].size;
						dest[1].size = SCALE_DIM_W * SCALE_DIM_H / 2;
						dest[1].fmt = HD_VIDEO_PXLFMT_UV;
						dest[1].width = SCALE_DIM_W/2;
						dest[1].height = SCALE_DIM_H/2;
						dest[1].line_ofs = SCALE_DIM_W;
						dest[1].channel = 1;
						dest[1].batch_num = 1;

                        // set func parameter
                        
						//scale
						p_parm.scale_dim.w = SCALE_DIM_W;
						p_parm.scale_dim.h = SCALE_DIM_H;	 
         
                        ret = vendor_ai_op_proc(p_stream->op_path, VENDOR_AI_OP_PREPROC, &p_parm, 1, src, 1, dest);

						p_parm.scale_dim.w = SCALE_DIM_W/2;
						p_parm.scale_dim.h = SCALE_DIM_H/2;
						
						ret = vendor_ai_op_proc(p_stream->op_path, VENDOR_AI_OP_PREPROC, &p_parm, 1, src+1, 1, dest+1);

                    
                    if (ret != 0) {
		        		printf("op inference fail\n");
		        		return 0;
		        	}
        
		        	p_stream->op_oneshot = FALSE;
		                	
		        	printf("inference done!\n");
		        	ret = hd_common_mem_flush_cache((VOID *)(&p_op->output_mem)->va, (&p_op->output_mem)->size);
                    if(HD_OK != ret) {
                        printf("flush cache failed.\n");
                    }

                    mem_save(&p_op->output_mem, "./op_user_out.bin");

	        	}
                usleep(100);
            }	
		}
		break;
        default:
        break;
    }
	
	ret = operator_free_out_buf(p_stream->op_path);
	if (HD_OK != ret) {
		printf("proc_id(%u) free output fail !!\n", p_stream->op_path);
		goto skip;
	}
skip:
	
	return 0;
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	VIDEO_LIVEVIEW stream[2] = {0}; //0: net proc, 1: op path
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
#if (USE_NET == TRUE)
	INT32 idx;
#endif
	HD_RESULT ret;
	INT key;

#if (USE_NET == TRUE)
	//net_in
	NET_IN_CONFIG in_cfg = {
		.input_filename = "jpg/YUV420_SP_W512H376.bin",
		.w = 512,
		.h = 376,
		.c = 2,
		.loff = 512,
		.fmt = HD_VIDEO_PXLFMT_YUV420
	};
	
	//net proc
	NET_PROC_CONFIG net_cfg = {
		.model_filename = "para/nvt_model.bin",
		.label_filename = "accuracy/labels.txt"
	};

	if(argc < 5){
		printf("usage : ai_net_with_op (job_opt) (job_sync) (buf_opt) (op-opt)\n"
			   "op-opt:\n"
			   "0 FC\n"
			   "1 PREPROC (YUV2RGB)\n"
			   "2 PREPROC (YUV2RGB & scale)\n"
			   "3 PREPROC (YUV2RGB & meansub_plane)\n"
			   "4 PREPROC (YUV2RGB & meansub_dc)\n"
			   "5 PREPROC (Y2Y_UV2UV)\n"
			   "6 FC (LL MODE)\n");
		return -1;
	}

	idx = 1;

	// parse network config
	if (argc > idx) {
		sscanf(argv[idx++], "%d", &net_cfg.job_method);
	}
	if (argc > idx) {
		sscanf(argv[idx++], "%d", &net_cfg.job_wait_ms);
	}
	if (argc > idx) {
		sscanf(argv[idx++], "%d", &net_cfg.buf_method);
	}
	if (argc > idx) {
		sscanf(argv[idx++], "%d", &stream[0].net_op_opt);
	}
	
	printf("\r\n\r\n");

	stream[0].in_path = 0;
	stream[0].net_path = 0;
#endif
	stream[0].op_path = 1;

	if (stream[0].net_op_opt == AI_OP_FC)
		printf("Run FC!\r\n");
	else if (stream[0].net_op_opt == AI_OP_PREPROC_YUV2RGB)
		printf("Run PREPROC (YUV2RGB)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_PREPROC_YUV2RGB_SCALE)
		printf("Run PREPROC (YUV2RGB & scale)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_PREPROC_YUV2RGB_MEANSUB_PLANE)
		printf("Run PREPROC (YUV2RGB & meansub_plane)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_PREPROC_YUV2RGB_MEANSUB_DC)
		printf("Run PREPROC (YUV2RGB & meansub_dc)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_PREPROC_Y2Y_UV2UV)
		printf("Run PREPROC (Y2Y_UV2UV)!\r\n");
	else if (stream[0].net_op_opt == AI_OP_FC_LL_MODE)
		printf("Run FC (LL MODE)!\r\n");
	else {
		printf("Unknown op-opt = %d",stream[0].net_op_opt);
		return -1;
	}
	
	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("hd_common_init fail=%d\n", ret);
		goto exit;
	}
    
#if defined(_BSP_NA51055_) || defined(_BSP_NA51089_)
	// set project config for AI
	hd_common_sysconfig(0, (1<<16), 0, VENDOR_AI_CFG); //enable AI engine
#else
	ret = vendor_common_clear_pool_blk(HD_COMMON_MEM_CNN_POOL, 0);
	if (ret != HD_OK) {
		printf("vendor_common_clear_pool_blk fail=%d\n", ret);
		goto exit;
	}
#endif

#if (USE_NET == TRUE)
	// init mem
	{
		INT32 idx = 0; // mempool index
		input_mem_config(stream[0].in_path, &mem_cfg, 0, idx);

		// config common pool
		network_mem_config(stream[0].net_path, &mem_cfg, &net_cfg, idx);
	}
#endif
#if defined(_BSP_NA51055_) || defined(_BSP_NA51089_)
	ret = hd_common_mem_init(&mem_cfg);
	if (HD_OK != ret) {
		printf("hd_common_mem_init err: %d\r\n", ret);
		goto exit;
	}
#endif
	
	// init all modules
	ret = init_module();
	if (ret != HD_OK) {
		printf("init fail=%d\n", ret);
		goto exit;
	}

#if (USE_NET == TRUE)
	// set open config
	ret = input_set_config(stream[0].in_path, &in_cfg);
	if (HD_OK != ret) {
		printf("proc_id(%u) input_set_config fail=%d\n", stream[0].in_path, ret);
		goto exit;
	}
	ret = network_set_config(stream[0].net_path, &net_cfg);
	if (HD_OK != ret) {
		printf("proc_id(%u) network_set_config fail=%d\n", stream[0].net_path, ret);
		goto exit;
	}
#endif

    // set operator config
    ret = operator_set_config(stream[0].op_path, stream[0].net_op_opt);
    if (HD_OK != ret) {
	    printf("proc_id(%u) operator_set_config fail=%d\n", stream[0].in_path, ret);
		goto exit;
	}

	// open video_liveview modules
	ret = open_module(&stream[0]);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}

// start
#if (USE_NET == TRUE)
	input_start(stream[0].in_path);
	network_user_start(&stream[0]);
#endif
	operator_user_start(&stream[0]);

	printf("Enter q to quit\n");
	printf("Enter r to run once\n");
	do {
		key = getchar();
		if (key == 'r') {
		
#if (USE_NET == TRUE)
			// run once
			network_user_oneshot(&stream[0]);
#endif
			operator_user_oneshot(&stream[0]);
			continue;
		}
		if (key == 'q' || key == 0x3) {

			break;
		}
	} while(1);

	// stop
#if (USE_NET == TRUE)
	input_stop(stream[0].in_path);
	network_user_stop(&stream[0]);
#endif
	operator_user_stop(&stream[0]);

exit:
	// close video_liveview modules
	ret = close_module(&stream[0]);
	if (ret != HD_OK) {
		printf("close fail=%d\n", ret);
	}

	// uninit all modules
	ret = exit_module();
	if (ret != HD_OK) {
		printf("exit fail=%d\n", ret);
	}


#if defined(_BSP_NA51055_) || defined(_BSP_NA51089_)
	// uninit memory
	ret = hd_common_mem_uninit();
	if (ret != HD_OK) {
		printf("mem fail=%d\n", ret);
	}
#endif

	// uninit hdal
	ret = hd_common_uninit();
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
	}

	return ret;
}
