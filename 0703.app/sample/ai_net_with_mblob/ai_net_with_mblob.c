/**
	@brief Source file of vendor ai net sample code.

	@file ai_net_with_mblob.c

	@ingroup ai_net_sample

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2021.  All rights reserved.
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
#include "vendor_ai_util.h"
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
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(ai_net, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif

#define DEBUG_MENU 		1

///////////////////////////////////////////////////////////////////////////////

#define NET_PATH_ID		UINT32

#define VENDOR_AI_CFG  				0x000f0000  //vendor ai config

#define AI_RGB_BUFSIZE(w, h)		(ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(HD_VIDEO_PXLFMT_RGB888_PLANAR) / 8) * (h))

#define FLOAT_FEATURE_IN  0

#define MULTI_BATCH_IN    1


///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------*/
/* Type Definitions                                                            */
/*-----------------------------------------------------------------------------*/

typedef struct _MEM_PARM {
	UINTPTR pa;
	UINTPTR va;
	UINT32 size;
	UINTPTR blk;
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

///////////////////////////////////////////////////////////////////////////////

typedef struct _NET_IN_CONFIG {

	CHAR input_filename[256];
	UINT32 w;
	UINT32 h;
	UINT32 c;
	UINT32 b;
	UINT32 bitdepth;
	UINT32 loff;
	UINT32 fmt;
	UINT32 is_comb_img;  // 1: image image (or feature-in) is a combination image (or feature-in).
	
} NET_IN_CONFIG;

typedef struct _NET_IN {

	NET_IN_CONFIG in_cfg;
	MEM_PARM input_mem;
	UINT32 in_id;
	VENDOR_AI_BUF src_img;
		
} NET_IN;

static NET_IN g_in[16] = {0};

UINT32 _calc_ai_buf_size(UINT32 loff, UINT32 h, UINT32 c, UINT32 b, UINT32 bitdepth, UINT32 fmt)
{
	UINT size = 0;

	switch (fmt) {
	case HD_VIDEO_PXLFMT_YUV420:
		{
			size = loff * h * 3 / 2;
		}
		break;
	case HD_VIDEO_PXLFMT_RGB888_PLANAR:
		{
			size = AI_RGB_BUFSIZE(loff, h);
		}
		break;
	case HD_VIDEO_PXLFMT_BGR888_PLANAR:
		{
			size = AI_RGB_BUFSIZE(loff, h);
		}
		break;
#if FLOAT_FEATURE_IN
	case HD_VIDEO_PXLFMT_AI_FLOAT32:
		{
			size = loff * h * c * sizeof(float);
		}
		break;
#endif
	default: // feature-in
		{
			size = loff * h * c * (bitdepth/8);
		}
		break;
	}

	if (!size) {
		printf("ERROR!! ai_buf size = 0\n");
	}
	return size;
}

static HD_RESULT _load_buf(MEM_PARM *mem_parm, CHAR *filename, VENDOR_AI_BUF *p_buf, UINT32 w, UINT32 h, UINT32 c, UINT32 b, UINT32 bitdepth, UINT32 loff, UINT32 fmt)
{
	INT32 file_len;

	file_len = mem_load(mem_parm, filename);
	if (file_len < 0) {
		printf("load buf(%s) fail\r\n", filename);
		return HD_ERR_NG;
	}
	printf("load buf(%s) ok\r\n", filename);
	p_buf->width = w;
	p_buf->height = h;
	p_buf->channel = c;
	p_buf->batch_num = b;
	p_buf->line_ofs = loff;
	p_buf->fmt = fmt;
	p_buf->pa   = mem_parm->pa;
	p_buf->va   = mem_parm->va;
	p_buf->sign = MAKEFOURCC('A','B','U','F');
	p_buf->size = _calc_ai_buf_size(loff, h, c, b, bitdepth, fmt);
	if (p_buf->size == 0) {
		printf("load buf(%s) fail, p_buf->size = 0\r\n", filename);
		return HD_ERR_NG;
	}

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
	NET_IN* p_net = g_in + net_path;
	UINT32 proc_id = p_net->in_id;
	
	memcpy((void*)&p_net->in_cfg, (void*)p_in_cfg, sizeof(NET_IN_CONFIG));
	printf("proc_id(%u) set in_cfg: file(%s), buf=(%u,%u,%u,%u,%u,%u,%08x)\r\n", 
		proc_id,
		p_net->in_cfg.input_filename,
		p_net->in_cfg.w,
		p_net->in_cfg.h,
		p_net->in_cfg.c,
		p_net->in_cfg.b,
		p_net->in_cfg.bitdepth,
		p_net->in_cfg.loff,
		p_net->in_cfg.fmt);
	
	return ret;
}

static HD_RESULT input_open(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_IN* p_net = g_in + net_path;
	UINT32 proc_id = p_net->in_id;
	UINT32 buf_size = 0;

	// calculate in buf size
	buf_size = _calc_ai_buf_size(p_net->in_cfg.loff, p_net->in_cfg.h, p_net->in_cfg.c, p_net->in_cfg.b, p_net->in_cfg.bitdepth, p_net->in_cfg.fmt);
	if (buf_size == 0) {
		printf("proc_id(%lu) calc buf size 0.\r\n", proc_id);
		return HD_ERR_FAIL;
	}

	// allocate in buf
	ret = mem_alloc(&p_net->input_mem, "ai_in_buf", buf_size * p_net->in_cfg.b);
	if (ret != HD_OK) {
		printf("proc_id(%lu) alloc ai_in_buf fail\r\n", proc_id);
		return HD_ERR_FAIL;
	}
	printf("alloc_in_buf: pa = 0x%lx, va = 0x%lx, size = %u\n", p_net->input_mem.pa, p_net->input_mem.va, p_net->input_mem.size);

	// load in buf
	ret = _load_buf(&p_net->input_mem, 
		p_net->in_cfg.input_filename, 
		&p_net->src_img,
		p_net->in_cfg.w,
		p_net->in_cfg.h,
		p_net->in_cfg.c,
		p_net->in_cfg.b,
		p_net->in_cfg.bitdepth,
		p_net->in_cfg.loff,
		p_net->in_cfg.fmt);
	if (ret != HD_OK) {
		printf("proc_id(%u) input_open fail=%d\n", proc_id, ret);
	}

	return ret;
}

#if FLOAT_FEATURE_IN
static HD_RESULT input_float2fix(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_IN* p_net = g_in + net_path;
	UINT32 proc_id = p_net->in_id;
	VENDOR_AI_BUF layer0_in = {0};
	UINT32 layer_id = 0;
	UINT32 fixed_size = 0;
	VOID *fixed_buf = NULL;

	// get layer0 in buf
	layer_id = 0;
	ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_IN(layer_id, 0), &layer0_in);
	if (HD_OK != ret) {
		printf("proc_id(%u) get layer0 inbuf fail !!\n", proc_id);
		goto exit;
	}
	printf("#### layer0_in.fmt = 0x%x ###\n", layer0_in.fmt);


	// alloc fixed buf size
	fixed_size = p_net->in_cfg.w * p_net->in_cfg.h *p_net-> in_cfg.c * sizeof(short);
	fixed_buf = (VOID*)malloc(fixed_size);

	// transfer input from float to fixed
	vendor_ai_cpu_util_float2fixed((FLOAT*)p_net->input_mem.va, 1.0f, fixed_buf,  layer0_in.fmt, p_net->in_cfg.h * p_net->in_cfg.w * p_net->in_cfg.c);

	// overwrite with fixed buf
	if (p_net->input_mem.size < fixed_size) {
		printf("proc_id(%u) invalid size !! input_mem.size(%u) < fixed_size(%u)\n", proc_id, p_net->input_mem.size, fixed_size);
		ret = HD_ERR_FAIL;
		goto exit;
	}
	memcpy((VOID*)p_net->input_mem.va, fixed_buf, fixed_size);
	hd_common_mem_flush_cache((VOID*)p_net->input_mem.va, fixed_size);

exit:
	if (fixed_buf)
		free(fixed_buf);

	return ret;
}
#endif

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

/*-----------------------------------------------------------------------------*/
/* Network Functions                                                             */
/*-----------------------------------------------------------------------------*/

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

static UINT32 _load_model(CHAR *filename, UINTPTR va)
{
	FILE  *fd;
	UINT32 file_size = 0, read_size = 0;
	const UINTPTR model_addr = va;
	//DBG_DUMP("model addr = %#lx\r\n", model_addr);

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

static HD_RESULT _load_label(UINTPTR addr, UINT32 line_len, const CHAR *filename)
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

static HD_RESULT network_init(void)
{
	HD_RESULT ret = HD_OK;
	int i;
	
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

	for (i = 0; i < 16; i++) {
		NET_PROC* p_net = g_net + i;
		p_net->proc_id = i;
	}
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
	ret = _load_label((UINTPTR)p_net->out_class_labels, VENDOR_AIS_LBL_LEN, p_net->net_cfg.label_filename);
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
	if (ret != HD_OK) {
		printf("proc_id(%u) vendor_ai_net_close fail=%d\n", proc_id, ret);
		return HD_ERR_FAIL;
	}
	mem_rel(&p_net->proc_mem);

	return ret;
}

static HD_RESULT network_dump_ai_buf(NET_PATH_ID net_path, VENDOR_AI_BUF *p_outbuf)
{
	HD_RESULT ret = HD_OK;

	printf("  sign(0x%x) ddr(%u) pa(0x%lx) va(0x%lx) sz(%u) w(%u) h(%u) ch(%u) batch_num(%u)\n",
		p_outbuf->sign, p_outbuf->ddr_id, p_outbuf->pa, p_outbuf->va, p_outbuf->size, p_outbuf->width, 
		p_outbuf->height, p_outbuf->channel, p_outbuf->batch_num);
	printf("  l_ofs(%u) c_ofs(%u) b_ofs(%u) t_ofs(%u) layout(%s) name(%s) op_name(%s) scale_ratio(%.6f)\n",
		p_outbuf->line_ofs, p_outbuf->channel_ofs, p_outbuf->batch_ofs, p_outbuf->time_ofs, p_outbuf->layout,
		p_outbuf->name, p_outbuf->op_name, p_outbuf->scale_ratio);

	// parsing pixel format
	switch (AI_PXLFMT_TYPE(p_outbuf->fmt)) {
	case HD_VIDEO_PXLFMT_AI_UINT8:
	case HD_VIDEO_PXLFMT_AI_SINT8:
	case HD_VIDEO_PXLFMT_AI_UINT16:
	case HD_VIDEO_PXLFMT_AI_SINT16:
	case HD_VIDEO_PXLFMT_AI_UINT32:
	case HD_VIDEO_PXLFMT_AI_SINT32:
	case HD_VIDEO_PXLFMT_AI_FLOAT32:
		{
			INT8 bitdepth = HD_VIDEO_PXLFMT_BITS(p_outbuf->fmt);
			INT8 int_bits = HD_VIDEO_PXLFMT_INT(p_outbuf->fmt);
			INT8 frac_bits = HD_VIDEO_PXLFMT_FRAC(p_outbuf->fmt);
			printf("  fmt(0x%lx) bits(%u) int(%u) frac(%u)\n", p_outbuf->fmt, bitdepth, int_bits, frac_bits);
		}
		break;
	default:
		switch ((UINT32)p_outbuf->fmt) {
		case HD_VIDEO_PXLFMT_BGR888_PLANAR:
			{
				printf("  fmt(0x%lx), BGR888_PLANAR\n", p_outbuf->fmt);
			}
			break;
		case HD_VIDEO_PXLFMT_YUV420:
			{
				printf("  fmt(0x%lx), YUV420\n", p_outbuf->fmt);
			}
			break;
		case HD_VIDEO_PXLFMT_Y8:
			{
				printf("  fmt(0x%lx), Y8 only\n", p_outbuf->fmt);
			}
			break;
		case HD_VIDEO_PXLFMT_UV:
			{
				printf("  fmt(0x%lx), UV only\n", p_outbuf->fmt);
			}
			break;
		case 0:
			{
				printf("  fmt(0x%lx), AI BUF\n", p_outbuf->fmt);
			}
			break;
		default:
			printf("unknown pxlfmt(0x%lx)\n", p_outbuf->fmt);
			break;
		}
	}
	printf("\n");

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

typedef struct _VIDEO_LIVEVIEW {

	// (1) input 
	NET_IN_CONFIG net_in_cfg;
	NET_PATH_ID in_path;
	NET_PATH_ID in_path2;
#if FLOAT_FEATURE_IN
	UINT32 is_float_in;
#endif

	// (2) network 
	NET_PROC_CONFIG net_proc_cfg;
	NET_PATH_ID net_path;
	pthread_t  proc_thread_id;
	UINT32 proc_start;
	UINT32 proc_exit;
	UINT32 proc_oneshot;
	UINT32 input_blob_num;

} VIDEO_LIVEVIEW;

static HD_RESULT init_module(void)
{
	HD_RESULT ret;
	if ((ret = input_init()) != HD_OK)
		return ret;
	if ((ret = network_init()) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT open_module(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;
	//if ((ret = input_open(p_stream->in_path)) != HD_OK)
	//	return ret;
	if ((ret = network_open(p_stream->net_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT close_module(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret;
	if ((ret = input_close(p_stream->in_path)) != HD_OK)
		return ret;
	if ((ret = network_close(p_stream->net_path)) != HD_OK)
		return ret;
	return HD_OK;
}

static HD_RESULT exit_module(void)
{
	HD_RESULT ret;
	if ((ret = input_uninit()) != HD_OK)
		return ret;
	if ((ret = network_uninit()) != HD_OK)
		return ret;
	return HD_OK;
}
///////////////////////////////////////////////////////////////////////////////

static VOID *network_user_thread(VOID *arg);

static HD_RESULT set_buf_by_in_path_list(VIDEO_LIVEVIEW *p_stream)
{
	HD_RESULT ret = HD_OK;
	VENDOR_AI_NET_INFO net_info = {0};
	VENDOR_AI_BUF in_buf = {0};
	VENDOR_AI_BUF tmp_buf = {0};
	UINT32 proc_id = p_stream->net_path;
	UINT32 i = 0, j = 0, idx = 0;
	UINT32 in_buf_cnt = 0, src_img_size = 0;
	UINT32 tmp_src_va = 0, tmp_src_pa = 0;
	UINT32 *in_path_list = NULL;

	/* get input process number */
	ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_INFO, &net_info);
	if (HD_OK != ret) {
		printf("proc_id(%u) get VENDOR_AI_NET_PARAM_INFO fail(%d) !!\n", proc_id, ret);
		goto exit;
	}
	in_buf_cnt = net_info.in_buf_cnt;
	printf("==============< %s, in_buf_cnt(%u) >==============\n", __func__, in_buf_cnt);

	/* get in path list */
	in_path_list = (UINT32 *)malloc(sizeof(UINT32) * in_buf_cnt);
	ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_IN_PATH_LIST, in_path_list);
	if (HD_OK != ret) {
		printf("proc_id(%u) get VENDOR_AI_NET_PARAM_IN_PATH_LIST fail(%d) !!\n", proc_id, ret);
		goto exit;
	}

	for (i = 0; i < in_buf_cnt; i++) {
		/* get in buf (by in path list) */
		ret = vendor_ai_net_get(proc_id, in_path_list[i], &tmp_buf);
		if (HD_OK != ret) {
			printf("proc_id(%u) get in buf fail, i(%d), in_path(0x%lx)\n", proc_id, i, in_path_list[i]);
			goto exit;
		}

		// dump in buf
		printf("dump_in_buf: path_id: 0x%lx\n", in_path_list[i]);
		ret = network_dump_ai_buf(proc_id, &tmp_buf);
		if (HD_OK != ret) {
			printf("proc_id(%u) dump in buf fail !!\n", proc_id);
			goto exit;
		}
	}

	for (i = 0; i < p_stream->input_blob_num; i++) {
		NET_IN* p_in = g_in + i;

		/* load input bin */
		ret = input_pull_buf((p_stream->in_path + i), &in_buf, 0);
		if (HD_OK != ret) {
			printf("in_path(%u) pull input fail !!\n", (p_stream->in_path + i));
			goto exit;
		}

		src_img_size = in_buf.size;
		tmp_src_va = in_buf.va; // backup in_buf addr
		tmp_src_pa = in_buf.pa;

		/* fill va, pa, then set input */
		printf("set_buf: batch_num = %u\n", in_buf.batch_num);
		for(j = 0; j < in_buf.batch_num; j++) {
			if (j > 0) {
				
				if (p_in->in_cfg.is_comb_img == 0) { // no combine image, need to use the same image as input.
					UINTPTR dst_va = in_buf.va + src_img_size;
					memcpy((VOID*)dst_va, (VOID*)in_buf.va, src_img_size);
					hd_common_mem_flush_cache((VOID *)dst_va, src_img_size);
					in_buf.pa += src_img_size;
					in_buf.va += src_img_size;
				} else {
					in_buf.pa += src_img_size;
					in_buf.va += src_img_size;
				}
			}

			printf(" path_%d(0x%x) pa(0x%lx) va(0x%lx) size(%u)\n", idx, in_path_list[idx], in_buf.pa, in_buf.va, in_buf.size);
			ret = vendor_ai_net_set(proc_id, in_path_list[idx], &in_buf);
			if (HD_OK != ret) {
				printf("proc_id(%u)push input fail !! i(%lu)\n", proc_id, i);
				goto exit;
			}
			idx++;
		}
		in_buf.va = tmp_src_va; // reduction in_buf addr
		in_buf.pa = tmp_src_pa;
	}

exit:
	/* free in_path_list */
	if (in_path_list)
		free(in_path_list);

	return ret;
}

static HD_RESULT get_buf_by_out_path_list(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id = p_net->proc_id;
	UINT32 i, out_buf_cnt;
	UINT32 *out_path_list = NULL;
	VENDOR_AI_NET_INFO net_info = {0};
	VENDOR_AI_BUF ai_buf = {0};

	/* get out_buf_cnt */
	ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_INFO, &net_info);
	if (HD_OK != ret) {
		printf("proc_id(%lu) get info fail !!\n", proc_id);
		goto exit;
	}
	out_buf_cnt = net_info.out_buf_cnt;
	printf("==============< %s, out_buf_cnt(%u) >==============\n", __func__, out_buf_cnt);

	/* get out path list */
	out_path_list = (UINT32 *)malloc(sizeof(UINT32) * out_buf_cnt);
	ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_OUT_PATH_LIST, out_path_list);
	if (HD_OK != ret) {
		printf("proc_id(%u) get out_path_list fail !!\n", proc_id);
		goto exit;
	}

	/* get out buf */
	for (i = 0; i < out_buf_cnt; i++) {
		// get out buf (by out path list)
		ret = vendor_ai_net_get(proc_id, out_path_list[i], &ai_buf);
		if (HD_OK != ret) {
			printf("proc_id(%u) get out buf fail, i(%d), out_path(0x%lx)\n", proc_id, i, out_path_list[i]);
			goto exit;
		}

		// dump out buf
		printf("dump_out_buf: path_id: 0x%lx\n", out_path_list[i]);
		ret = network_dump_ai_buf(proc_id, &ai_buf);
		if (HD_OK != ret) {
			printf("proc_id(%u) dump out buf fail !!\n", proc_id);
			goto exit;
		}
	}

exit:
	if (out_path_list)
		free(out_path_list);

	return ret;
}

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
			p_stream->proc_oneshot = 0;

			// set buf by in_path_list
			ret = set_buf_by_in_path_list(p_stream);
			if (HD_OK != ret) {
				printf("proc_id(%u) set in_buf fail(%d) !!\n", p_stream->net_path, ret);
				goto skip;
			}

			// do net proc
			ret = vendor_ai_net_proc(p_stream->net_path);
			if (HD_OK != ret) {
				printf("proc_id(%u) proc fail(%d) !!\n", p_stream->net_path, ret);
				goto skip;
			}

			printf("proc_id(%u) oneshot done!\n\n", p_stream->net_path);

			// get buf by out_path_list
			ret = get_buf_by_out_path_list(p_stream->net_path);
			if (HD_OK != ret) {
				printf("proc_id(%u) get out_buf fail(%d) !!\n", p_stream->net_path, ret);
				goto skip;
			}
		}
		usleep(100);
	}
	
skip:
	return 0;
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	VIDEO_LIVEVIEW stream[1] = {0};
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	INT32 idx;
	HD_RESULT ret;
	INT key;
	UINT32 j;

#if FLOAT_FEATURE_IN
    /* float feature input */
	stream[0].input_blob_num = 1;
	stream[0].is_float_in = 1;

	//net_in
	NET_IN_CONFIG in_cfg = {
		.input_filename = "jpg/float32.bin",
		.w = 256,
		.h = 1,
		.c = 1,
		.b = 1,
		.bitdepth = 32,
		.loff = 256,
		.fmt = HD_VIDEO_PXLFMT_AI_SFIXED16(8),
		.is_comb_img = 0,
	};

	NET_IN_CONFIG in_cfg2 = {
		.input_filename = "NULL",
		.w = 0,
		.h = 0,
		.c = 0,
		.b = 0,
		.bitdepth = 0,
		.loff = 0,
		.fmt = 0,
		.is_comb_img = 0,
	};

	//net proc
	NET_PROC_CONFIG net_cfg = {
		.model_filename = "para/nvt_model_float32.bin",
		.label_filename = "accuracy/labels.txt"
	};
#else
#if MULTI_BATCH_IN
	/* multi-blob with multi-batch */
	stream[0].input_blob_num = 2;

	//net_in
	NET_IN_CONFIG in_cfg = {
		.input_filename = "jpg/mblob_mbatch_0.yuv",
		.w = 12,
		.h = 12,
		.c = 2,
		.b = 8,
		.bitdepth = 8,
		.loff = 12,
		.fmt = HD_VIDEO_PXLFMT_YUV420,
		.is_comb_img = 0,
	};

	NET_IN_CONFIG in_cfg2 = {
		.input_filename = "jpg/mblob_mbatch_1.bin",
		.w = 1,
		.h = 1,
		.c = 5,
		.b = 8,
		.bitdepth = 16,
		.loff = 1,
		.fmt = HD_VIDEO_PXLFMT_AI_UFIXED16(0),
		.is_comb_img = 1,
	};

	//net proc
	NET_PROC_CONFIG net_cfg = {
		.model_filename = "para/nvt_model_mblob_mbatch.bin",
		.label_filename = "accuracy/labels.txt"
	};
#else
	/* multi-blob without batch */
	stream[0].input_blob_num = 2;

	//net_in
	NET_IN_CONFIG in_cfg = {
		.input_filename = "jpg/mblob.bin",
		.w = 224,
		.h = 224,
		.c = 3,
		.b = 1,
		.bitdepth = 8,
		.loff = 224,
		.fmt = HD_VIDEO_PXLFMT_AI_UFIXED8(0),
		.is_comb_img = 0,
	};

	NET_IN_CONFIG in_cfg2 = {
		.input_filename = "jpg/mblob.bin",
		.w = 224,
		.h = 224,
		.c = 3,
		.b = 1,
		.bitdepth = 8,
		.loff = 224,
		.fmt = HD_VIDEO_PXLFMT_AI_UFIXED8(0),
		.is_comb_img = 0,
	};

	//net proc
	NET_PROC_CONFIG net_cfg = {
		.model_filename = "para/nvt_model_mblob.bin",
		.label_filename = "accuracy/labels.txt"
	};
#endif // End of #if MULTI_BATCH_IN
#endif // End of #if FLOAT_FEATURE_IN

	if(argc < 3){
		printf("usage : ai_net (job_opt) (job_sync) (buf_opt)\n");
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
	
	printf("\r\n\r\n");

	stream[0].in_path = 0;
	stream[0].net_path = 0;
	
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

	// init mem
	{
		INT32 idx = 0; // mempool index
		input_mem_config(stream[0].in_path, &mem_cfg, 0, idx);

		// config common pool
		ret = network_mem_config(stream[0].net_path, &mem_cfg, &net_cfg, idx);
		if (ret < 0) {
			printf("network_mem_config err: %d\r\n", ret);
			goto exit;
		}
	}
#if defined(_BSP_NA51055_)|| defined(_BSP_NA51089_)
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

	// set open config
	for (j=0; j < stream[0].input_blob_num; j++) {
		if (j == 0) {
			ret = input_set_config((stream[0].in_path + j), &in_cfg);
		} else {
			ret = input_set_config((stream[0].in_path + j), &in_cfg2);
		}
		if (HD_OK != ret) {
			printf("in_path(%u) input_set_config fail=%d\n", (stream[0].in_path + j), ret);
			goto exit;
		}
	}
	ret = network_set_config(stream[0].net_path, &net_cfg);
	if (HD_OK != ret) {
		printf("proc_id(%u) network_set_config fail=%d\n", stream[0].net_path, ret);
		goto exit;
	}

	// open video_liveview modules
	for(j=0; j < stream[0].input_blob_num; j++) {
		ret = input_open((stream[0].in_path + j));
		if (ret != HD_OK) {
			printf("in_path(%u) input open fail=%d\n", (stream[0].in_path + j), ret);
			goto exit;
		}
	}
	ret = open_module(&stream[0]);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}

	// start
	input_start(stream[0].in_path);
	network_user_start(&stream[0]);

#if FLOAT_FEATURE_IN
	// input float2fix
	if (stream[0].is_float_in) {
		ret = input_float2fix(stream[0].net_path); // Getting layer0 info needs to call vendor_ai_net_start before it used.
		if (ret != HD_OK) {
			printf("input float-to-fixed fail=%d\n", ret);
			goto exit;
		}
	}
#endif

	do {
		printf("Enter q to exit, r to run once\n");
		key = getchar();
		if (key == 'r') {
			// run once
			network_user_oneshot(&stream[0]);
			continue;
		}
		if (key == 'q' || key == 0x3) {

			break;
		}
	} while(1);

	// stop
	input_stop(stream[0].in_path);
	network_user_stop(&stream[0]);

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
