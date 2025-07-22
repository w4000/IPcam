/**
	@brief Source file of vendor ai net sample code.

	@file ai_net_with_dim.c

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
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(ai_net_with_dim, argc, argv)
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

#define MULTI_INPUT		0
///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------*/
/* Type Definitions                                                            */
/*-----------------------------------------------------------------------------*/

typedef struct _MEM_PARM {
	UINT32 pa;
	UINT32 va;
	UINT32 size;
	UINT32 blk;
} MEM_PARM;


/*-----------------------------------------------------------------------------*/
/* Global Functions                                                             */
/*-----------------------------------------------------------------------------*/
char dump_dir[256];
int dump_idx = 0;
extern HD_RESULT _vendor_ai_net_debug_layer(UINT32 proc_id, UINT32 layer_opt, CHAR *filedir);
static int ai_test_debug_dumpout(void)
{
	snprintf(dump_dir, 256 - 1, "/mnt/sd/output_idx%d", dump_idx);
	_vendor_ai_net_debug_layer(0, 1, dump_dir);
	printf("!!## dump_layer done ##!!\n");
	dump_idx++;
	return 1;
}

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
	NET_IN* p_net = g_in + net_path;
	UINT32 proc_id = p_net->in_id;
	
	memcpy((void*)&p_net->in_cfg, (void*)p_in_cfg, sizeof(NET_IN_CONFIG));
	printf("proc_id(%u) set in_cfg: file(%s), buf=(%u,%u,%u,%u,%08x)\r\n", 
		proc_id,
		p_net->in_cfg.input_filename,
		p_net->in_cfg.w,
		p_net->in_cfg.h,
		p_net->in_cfg.c,
		p_net->in_cfg.loff,
		p_net->in_cfg.fmt);
	
	return ret;
}

static INT32 get_file_size(const CHAR *filename)
{
	FILE *fd;
	INT32 size = 0;
	fd = fopen(filename, "rb");
	if (!fd) {
		printf("cannot read %s\r\n", filename);
		size = -1;
		return 0;
	}
	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	return size;
}
static HD_RESULT input_open(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_IN* p_net = g_in + net_path;
	UINT32 proc_id = p_net->in_id;
	INT32 buf_size = AI_RGB_BUFSIZE(p_net->in_cfg.w, p_net->in_cfg.h);
	INT32 file_size = 0;

	buf_size = AI_RGB_BUFSIZE(p_net->in_cfg.w, p_net->in_cfg.h);
	file_size = get_file_size(p_net->in_cfg.input_filename);
	if (file_size == 0) {
		printf("proc_id(%lu) get_file_size = 0\r\n", proc_id);
		return HD_ERR_FAIL;
	}
	if (buf_size < file_size) {
		buf_size = file_size;
	}
	ret = mem_alloc(&p_net->input_mem, "ai_in_buf", buf_size);
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
	CHAR diff_filename[256];
	INT32 diff_binsize;

} NET_PROC_CONFIG;

typedef struct _NET_PROC {

	NET_PROC_CONFIG net_cfg;
	MEM_PARM proc_mem;
	MEM_PARM diff_mem;
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
	
	if (i < 0) {
		printf("proc_id(%u) index < 0\r\n", proc_id);
		return HD_ERR_NG;
	}

	memcpy((void*)&p_net->net_cfg, (void*)p_proc_cfg, sizeof(NET_PROC_CONFIG));
	if (strlen(p_net->net_cfg.model_filename) == 0) {
		printf("proc_id(%u) input model is null\r\n", proc_id);
		return HD_ERR_NG;
	}

	/* config nvt_model.bin
	 * use common user pool */
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

	/* config nvt_stripe_model.bin
	 * use common user pool */
	p_net->net_cfg.diff_binsize = _getsize_model(p_net->net_cfg.diff_filename);
	if (p_net->net_cfg.diff_binsize <= 0) {
		printf("proc_id(%lu) diff model is not exist?\r\n", proc_id);
		return HD_ERR_NG;
	}

	printf("proc_id(%lu) set net_mem_cfg: diff-file(%s), binsize=%ld\r\n",
		proc_id,
		p_net->net_cfg.diff_filename,
		p_net->net_cfg.diff_binsize);

	// config common pool (in)
	p_mem_cfg->pool_info[i].type = HD_COMMON_MEM_USER_DEFINIED_POOL + proc_id;
	p_mem_cfg->pool_info[i].blk_size = p_net->net_cfg.diff_binsize;
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
	UINT32 diffsize = p_net->net_cfg.diff_binsize;
	UINT32 proc_id;
	p_net->proc_id = net_path;
	proc_id = p_net->proc_id;

	memcpy((void*)&p_net->net_cfg, (void*)p_proc_cfg, sizeof(NET_PROC_CONFIG));
	p_net->net_cfg.binsize = binsize;
	p_net->net_cfg.diff_binsize = diffsize;
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
		cfg_job_opt.schd_parm = VENDOR_AI_FAIR_CORE(0); //VENDOR_AI_FAIR_CORE_ALL; //FAIR dispatch to ALL core
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

	ret = mem_get(&p_net->proc_mem, p_net->net_cfg.binsize, proc_id);
	if (ret != HD_OK) {
		printf("proc_id(%lu) alloc model binsize fail\r\n", proc_id);
		return HD_ERR_FAIL;
	}
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

	/* load nvt_stripe_model.bin and set before open */
	mem_get(&p_net->diff_mem, p_net->net_cfg.diff_binsize, proc_id);

	printf("diff_mem va(%lu) diff_mem pa(%lu) diff_binsize(%ld)\n", p_net->diff_mem.va, p_net->diff_mem.pa, p_net->net_cfg.diff_binsize);
	//load file
	loadsize = _load_model(p_net->net_cfg.diff_filename, p_net->diff_mem.va);
	if (loadsize <= 0) {
		printf("proc_id(%lu) diff model load fail: %s\r\n", proc_id, p_net->net_cfg.diff_filename);
		return 0;
	}
	vendor_ai_net_set(proc_id, VENDOR_AI_NET_PARAM_CFG_MODEL_RESINFO, (VENDOR_AI_NET_CFG_MODEL*)&p_net->diff_mem);

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
static HD_RESULT network_dump_ai_outbuf(NET_PATH_ID net_path, VENDOR_AI_BUF *p_outbuf)
{
	HD_RESULT ret = HD_OK;
	printf("dump ai_outbuf:\n");
	printf("   sign(0x%x)\n", p_outbuf->sign);
	printf("   ddr_id(%lu)\n", p_outbuf->ddr_id);
	printf("   pa(0x%lx)\n", p_outbuf->pa);
	printf("   va(0x%lx)\n", p_outbuf->va);
	printf("   size(%lu)\n", p_outbuf->size);
	printf("   width(%lu)\n", p_outbuf->width);
	printf("   height(%lu)\n", p_outbuf->height);
	printf("   channel(%lu)\n", p_outbuf->channel);
	printf("   batch_num(%lu)\n", p_outbuf->batch_num);
	printf("   line_ofs(%lu)\n", p_outbuf->line_ofs);
	printf("   channel_ofs(%lu)\n", p_outbuf->channel_ofs);
	printf("   batch_ofs(%lu)\n", p_outbuf->batch_ofs);
	printf("   time_ofs(%lu)\n", p_outbuf->time_ofs);
	printf("   layout(%s)\n", p_outbuf->layout);
	printf("   name(%s)\n", p_outbuf->name);
	printf("   op_name(%s)\n", p_outbuf->op_name);
	printf("   scale_ratio(%.6f)\n", p_outbuf->scale_ratio);
	printf("\n");

	// parsing pixel format
	printf("parsing pixel format:\n");
	printf("   fmt(0x%lx)\n", p_outbuf->fmt);
	switch (HD_VIDEO_PXLFMT_TYPE(p_outbuf->fmt))) {
	case HD_VIDEO_PXLFMT_AI_UINT8:
	case HD_VIDEO_PXLFMT_AI_SINT8:
	case HD_VIDEO_PXLFMT_AI_UINT16:
	case HD_VIDEO_PXLFMT_AI_SINT16:
	case HD_VIDEO_PXLFMT_AI_UINT32:
	case HD_VIDEO_PXLFMT_AI_SINT32:
		{
			INT8 int_bits = HD_VIDEO_PXLFMT_INT(p_outbuf->fmt);
			INT8 frac_bits = HD_VIDEO_PXLFMT_FRAC(p_outbuf->fmt);
			printf("   int_bits(0x%x)\n", int_bits);
			printf("   frac_bits(0x%x)\n", frac_bits);
		}
		break;
	case HD_VIDEO_PXLFMT_AI_FLOAT32:
		{
		}
		break;
	default:
		switch ((UINT32)p_outbuf->fmt) {
		case HD_VIDEO_PXLFMT_BGR888_PLANAR:
			{
				printf("   -> BGR888_PLANAR\n");
			}
			break;
		case HD_VIDEO_PXLFMT_UV:
			{
				printf("   -> UV only\n");
			}
			break;
		case 0:
			{
				printf("   -> AI BUF\n");
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
static HD_RESULT get_buf_by_name(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id = p_net->proc_id;
	VENDOR_AI_BUF_NAME buf_name = {0};
	VENDOR_AI_BUF ai_buf = {0};
	sprintf(buf_name.name, "post_proc.out0");

	// get out_path by name
	ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_OUT_PATH_BY_NAME, &buf_name);
	if (HD_OK != ret) {
		printf("proc_id(%u) get OUT_PATH_BY_NAME fail !!\n", proc_id);
		goto exit;
	}
	printf("get out_path_id(%lu) by name(%s)\n\n", buf_name.path_id, buf_name.name);

	// get out buf by path_id
	ret = vendor_ai_net_get(proc_id, buf_name.path_id, &ai_buf);
	if (HD_OK != ret) {
		printf("proc_id(%u) get AI_OUTBUF fail !!\n", proc_id);
		goto exit;
	}
	printf("==============< %s >==============\n", __func__);
	ret = network_dump_ai_outbuf(proc_id, &ai_buf);
	if (HD_OK != ret) {
		printf("proc_id(%u) dump ai_buf fail !!\n", proc_id);
		goto exit;
	}
exit:
	return ret;
}
static HD_RESULT get_buf_by_out_path_list(NET_PATH_ID net_path)
{
	HD_RESULT ret = HD_OK;
	NET_PROC* p_net = g_net + net_path;
	UINT32 proc_id = p_net->proc_id;
	UINT32 i, out_buf_cnt;
	UINT32 *path_list = NULL;
	VENDOR_AI_NET_INFO net_info = {0};
	VENDOR_AI_BUF ai_buf = {0};

	// get out_buf_cnt
	ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_INFO, &net_info);
	if (HD_OK != ret) {
		printf("proc_id(%lu) get info fail !!\n", proc_id);
		goto exit;
	}
	out_buf_cnt = net_info.out_buf_cnt;
	printf("out_buf_cnt(%lu)\n", out_buf_cnt);

	// alloc for path_list
	path_list = (UINT32 *)malloc(sizeof(UINT32) * out_buf_cnt);

	// get path_list
	ret = vendor_ai_net_get(proc_id, VENDOR_AI_NET_PARAM_OUT_PATH_LIST, path_list);
	if (HD_OK != ret) {
		printf("proc_id(%u) get out_path_list fail !!\n", proc_id);
		goto exit;
	}

	// get out_buf by path_list
	printf("==============< %s >==============\n", __func__);
	for (i = 0; i < out_buf_cnt; i++) {
		ret = vendor_ai_net_get(proc_id, path_list[i], &ai_buf);
		if (HD_OK != ret) {
			printf("proc_id(%u) output get fail, i(%d), path_list(0x%lx)\n", proc_id, i, path_list[i]);
			goto exit;
		}
		printf("[%lu] path_id: 0x%lx\n", i, path_list[i]);
		ret = network_dump_ai_outbuf(proc_id, &ai_buf);
		if (HD_OK != ret) {
			printf("proc_id(%u) dump ai_buf fail !!\n", proc_id);
			goto exit;
		}
	}
exit:
	if (path_list)
		free(path_list);
	return ret;
}

///////////////////////////////////////////////////////////////////////////////

typedef struct _VIDEO_LIVEVIEW {

	// (1) input 
	NET_IN_CONFIG net_in_cfg;
	NET_PATH_ID in_path;

	// (2) network 
	NET_PROC_CONFIG net_proc_cfg;
	NET_PATH_ID net_path;
	pthread_t  proc_thread_id;
	UINT32 proc_start;
	UINT32 proc_exit;
	UINT32 proc_oneshot;

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
	if ((ret = input_open(p_stream->in_path)) != HD_OK)
		return ret;
#if MULTI_INPUT
	if ((ret = input_open(p_stream->in_path + 1)) != HD_OK)
		return ret;
#endif
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

	while (p_stream->proc_start == 0) sleep(1);

	while (p_stream->proc_exit == 0) {

		if (p_stream->proc_oneshot) {
			
			VENDOR_AI_BUF	in_buf = {0};
			VENDOR_AI_POSTPROC_RESULT_INFO out_buf = {0};

			p_stream->proc_oneshot = 0;
			
			ret = input_pull_buf(p_stream->in_path, &in_buf, 0);
			if (HD_OK != ret) {
				printf("proc_id(%u) pull input fail !!\n", p_stream->in_path);
				goto skip;
			}

			printf("proc_id(%u) oneshot ... wh(%d,%d)\n", p_stream->net_path, in_buf.width, in_buf.height);
			// set input image
			ret = vendor_ai_net_set(p_stream->net_path, VENDOR_AI_NET_PARAM_IN(0, 0), &in_buf);
			if (HD_OK != ret) {
				printf("proc_id(%u)push input fail !!\n", p_stream->net_path);
				goto skip;
			}
#if MULTI_INPUT
			// do net proc
			ret = input_pull_buf(p_stream->in_path + 1, &in_buf, 0);
			if (HD_OK != ret) {
				printf("proc_id(%u) pull input fail !!\n", p_stream->in_path);
				goto skip;
			}
			ret = vendor_ai_net_set(p_stream->net_path, VENDOR_AI_NET_PARAM_IN(1, 0), &in_buf);
			if (HD_OK != ret) {
				printf("proc_id(%u)push input fail !!\n", p_stream->net_path);
				goto skip;
			}
#endif
			// do net proc
			ret = vendor_ai_net_proc(p_stream->net_path);
			if (HD_OK != ret) {
				printf("proc_id(%u) proc fail !!\n", p_stream->net_path);
				goto skip;
			}

			// get output result
			vendor_ai_net_get(p_stream->net_path, VENDOR_AI_NET_PARAM_OUT(VENDOR_AI_MAXLAYER, 0), &out_buf);
			printf("proc_id(%u) oneshot done!\n", p_stream->net_path);

			// get buf by name
			get_buf_by_name(p_stream->net_path);

			// get buf by out_path_list
			get_buf_by_out_path_list(p_stream->net_path);
			printf("ready to stop, press 't' !!\n");
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
	VIDEO_LIVEVIEW stream[1] = {0}; //0: net proc
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	INT32 idx;
	HD_RESULT ret;
	INT key;
	NET_IN* p_net = g_in;

	//net_in
#if MULTI_INPUT
	NET_IN_CONFIG in_cfg = {
		.input_filename = "jpg/YUV420_SP_W512H376_33x.bin",
		.w = 512,
		.h = 376,
		.c = 2,
		.loff = 512,
		.fmt = HD_VIDEO_PXLFMT_YUV420
	};
	NET_IN_CONFIG in_cfg2 = {
		.input_filename = "jpg/YUV420_SP_W512H376_33x.bin",
		.w = 512,
		.h = 376,
		.c = 2,
		.loff = 512,
		.fmt = HD_VIDEO_PXLFMT_YUV420
	};
#else
	NET_IN_CONFIG in_cfg = {
		.input_filename = "YUV420_SP_Huang_W1920H1080.bin",
		.w = 1920,
		.h = 1080,
		.c = 3,
		.loff = 1920,
		.fmt = 0x51080400,
	};
	
#endif
	//net proc
	NET_PROC_CONFIG net_cfg = {
		.model_filename = "para/multi_scale/nvt_model.bin",
		.label_filename = "accuracy/labels.txt",
		.diff_filename = "para/multi_scale/nvt_stripe_model.bin"
	};

	if(argc < 3){
		printf("usage : ai_net_with_dim (job_opt) (job_sync) (buf_opt)\n");
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
		strncpy(in_cfg.input_filename, argv[idx++], 256-1);
		in_cfg.input_filename[256-1] = '\0';
	}
	if (argc > idx) {
		sscanf(argv[idx++], "%lu", &in_cfg.w);
	}
	if (argc > idx) {
		sscanf(argv[idx++], "%lu", &in_cfg.h);
	}
	if (argc > idx) {
		sscanf(argv[idx++], "%lu", &in_cfg.c);
	}
	if (argc > idx) {
		sscanf(argv[idx++], "%lu", &in_cfg.loff);
	}
	if (argc > idx) {
		sscanf(argv[idx++], "%x", (unsigned int *)&in_cfg.fmt);
	}
	if (argc > idx) {
		strncpy(net_cfg.model_filename, argv[idx++], 256-1);
		net_cfg.model_filename[256-1] = '\0';
	}
	if (argc > idx) {
		strncpy(net_cfg.diff_filename, argv[idx++], 256-1);
		net_cfg.diff_filename[256-1] = '\0';
	}

	// parse network config
	if (argc > idx) {
		strncpy(net_cfg.model_filename, argv[idx++], 256-1);
		net_cfg.model_filename[256-1] = '\0';
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
		if (network_mem_config(stream[0].net_path, &mem_cfg, &net_cfg, idx) < 0) {
			printf("network_mem_config fail\n");
			goto exit;
	}
	}
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

	// set open config
	ret = input_set_config(stream[0].in_path, &in_cfg);
	if (HD_OK != ret) {
		printf("proc_id(%u) input_set_config fail=%d\n", stream[0].in_path, ret);
		goto exit;
	}
#if MULTI_INPUT
	ret = input_set_config(stream[0].in_path + 1, &in_cfg2);
	if (HD_OK != ret) {
		printf("proc_id(%u) input_set_config fail=%d\n", stream[0].in_path + 1, ret);
		goto exit;
	}
#endif
	ret = network_set_config(stream[0].net_path, &net_cfg);
	if (HD_OK != ret) {
		printf("proc_id(%u) network_set_config fail=%d\n", stream[0].net_path, ret);
		goto exit;
	}

	// open video_liveview modules
	ret = open_module(&stream[0]);
	if (ret != HD_OK) {
		printf("open fail=%d\n", ret);
		goto exit;
	}

	printf("Enter q to quit\n");
	printf("Enter u to update by dim\n");
	printf("Enter v to update by index\n");
	printf("Enter s to start\n");
	printf("Enter t to stop\n");
	printf("Enter r to run once\n");
	do {
		key = getchar();
		if (key == 'r') {
			printf("run once\n");
			// run once
			network_user_oneshot(&stream[0]);
			continue;
		}
		if (key == 's') {
			printf("fix start\n");
			input_start(stream[0].in_path);
			network_user_start(&stream[0]);
			printf("ready to trigger, press 'r' !!\n");
			continue;
		}
		if (key == 't') {
			printf("stop\n");
			input_stop(stream[0].in_path);
			network_user_stop(&stream[0]);
			continue;
		}

		/* dynamic set DIM (change resoluiton scale), set before start (after stop) */
		if (key == 'u') {
			HD_DIM dim = {0};
			printf("Input width:\n");
			scanf("%ld", &dim.w);
			printf("Input height:\n");
			scanf("%ld", &dim.h);
			vendor_ai_net_set(0, VENDOR_AI_NET_PARAM_RES_DIM, &dim);
			printf("set VENDOR_AI_NET_PARAM_RES_DIM dim(%ldx%ld)\n", dim.w, dim.h);
			p_net->src_img.width = dim.w;
			p_net->src_img.height = dim.h;
			printf("update start (by dim)\n");
			input_start(stream[0].in_path);
			network_user_start(&stream[0]);
			printf("ready to trigger, press 'r' !!\n");
			continue;
		}
		/* dynamic set DIM (change resoluiton scale), set only after start */
		if (key == 'm') {
			HD_DIM dim = {0};
			printf("Input width:\n");
			scanf("%ld", &dim.w);
			printf("Input height:\n");
			scanf("%ld", &dim.h);
			printf(">>> set dim\n");
			if (HD_OK != vendor_ai_net_set(0, VENDOR_AI_NET_PARAM_RES_DIM_IMM, &dim))
				continue;
			p_net->src_img.width = dim.w;
			p_net->src_img.height = dim.h;
			printf("update start imm (by dim)\n");
			printf("ready to trigger, press 'r' !!\n");
			continue;
		}
		/* dynamic set ID (change resoluiton ID), set before start (after stop) */
		if (key == 'v') {
			UINT32 idx = 2;
			printf("Input index:\n");
			scanf("%ld", &idx);
			vendor_ai_net_set(0, VENDOR_AI_NET_PARAM_RES_ID, &idx);
			printf("set VENDOR_AI_NET_PARAM_RES_ID idx(%ld)\n", idx);
			printf("update start (by idx)\n");
			input_start(stream[0].in_path);
			network_user_start(&stream[0]);
			continue;
		}
		/* dynamic set ID (change resoluiton ID), set only after start */
		if (key == 'n') {
			UINT32 idx = 2;
			printf("Input index:\n");
			scanf("%ld", &idx);
			if (HD_OK != vendor_ai_net_set(0, VENDOR_AI_NET_PARAM_RES_ID_IMM, &idx))
				continue;
			printf("set VENDOR_AI_NET_PARAM_RES_ID_IMM idx(%ld)\n", idx);
			printf("update start imm (by idx)\n");
			printf("ready to trigger, press 'r' !!\n");
			continue;
		}
		if (key == 'd') {
			printf("dump\n");
			ai_test_debug_dumpout();
			continue;
		}
		if (key == 'q' || key == 0x3) {
			printf("quit\n");
			break;
		}
	} while(1);

	// stop

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
