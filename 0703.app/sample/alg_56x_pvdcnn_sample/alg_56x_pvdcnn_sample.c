/*-----------------------------------------------------------------------------*/
/* Including Files                                                             */
/*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "hdal.h"
#include "hd_type.h"
#include "hd_debug.h"
#include "vendor_ai.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"
#include "vendor_ai_cpu_postproc.h"
#include <arm_neon.h>
#include "pvdcnn_lib.h"
#include <sys/time.h>


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
#define MAIN(argc, argv) 		EXAMFUNC_ENTRY(ai_net_with_buf, argc, argv)
#define GETCHAR()				NVT_EXAMSYS_GETCHAR()
#endif
#define AI_POST_PROC	1

#define CHKPNT			printf("\033[37mCHK: %s, %s: %d\033[0m\r\n",__FILE__,__func__,__LINE__)
#define DBGH(x)			printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#define DBGD(x)			printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)

///////////////////////////////////////////////////////////////////////////////

#define NET_PATH_ID		UINT32

#define VENDOR_AI_CFG  				0x000f0000  //vendor ai config

#define SAVE_SCALE		DISABLE
#define PROF			ENABLE

#if PROF
	static struct timeval tstart, tend;
	#define PROF_START()    gettimeofday(&tstart, NULL);
	#define PROF_END(msg)   gettimeofday(&tend, NULL);  \
			printf("%s time (us): %lu\r\n", msg,         \
					(tend.tv_sec - tstart.tv_sec) * 1000000 + (tend.tv_usec - tstart.tv_usec));
#else
	#define PROF_START()
	#define PROF_END(msg)
#endif
#define PDCNN_PROC				1
#define VDCNN_PROC				1
#define PD_DISTANCE_MODE		0
#define VD_DISTANCE_MODE		0
static CHAR model_name[3][256]	= { {"/mnt/sd/CNNLib/para/pvdcnn/nvtpd.bin"},
									{"/mnt/sd/CNNLib/para/pvdcnn/nvtvd.bin"},
									{"/mnt/sd/CNNLib/para/pvdcnn/nvtrfp.bin"}
	                              };
static VENDOR_AIS_FLOW_MEM_PARM g_mem       = {0};
///////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------*/
/* Type Definitions                                                            */
/*-----------------------------------------------------------------------------*/
typedef struct _PVD_THREAD_PARM {
	VENDOR_AIS_FLOW_MEM_PARM pvd_mem;
	VENDOR_AIS_FLOW_MEM_PARM srcimg_mem;
} PVD_THREAD_PARM;

typedef struct _NET_IN {
	CHAR input_filename[256];
	UINT32 w;
	UINT32 h;
	UINT32 c;
	UINT32 loff;
	UINT32 fmt;
	VENDOR_AI_BUF src_img;
} NET_IN;
static HD_COMMON_MEM_VB_BLK g_blk = 0;
/*-----------------------------------------------------------------------------*/
/* Global Functions                                                             */
/*-----------------------------------------------------------------------------*/

static HD_RESULT mem_get(VOID)
{
	HD_RESULT ret = HD_OK;
	UINT32 pa   = 0;
	void  *va   = NULL;
	HD_COMMON_MEM_VB_BLK      blk;
	
	blk = hd_common_mem_get_block(HD_COMMON_MEM_USER_DEFINIED_POOL, PVD_MAX_MEM_SIZE, DDR_ID0);
	if (HD_COMMON_MEM_VB_INVALID_BLK == blk) {
		printf("hd_common_mem_get_block fail\r\n");
		return HD_ERR_NG;
	}
	pa = hd_common_mem_blk2pa(blk);
	if (pa == 0) {
		printf("not get buffer, pa=%08x\r\n", (int)pa);
		return HD_ERR_NOMEM;
	}
	va = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, PVD_MAX_MEM_SIZE);

	/* Release buffer */
	if (va == 0) {
		ret = hd_common_mem_munmap(va, PVD_MAX_MEM_SIZE);
		if (ret != HD_OK) {
			printf("mem unmap fail\r\n");
			return ret;
		}
	}

	g_mem.pa = pa;
	g_mem.va = (UINT32)va;
	g_mem.size = PVD_MAX_MEM_SIZE;
	g_blk = blk;
	
	return HD_OK;
}



static INT32 mem_load(VENDOR_AIS_FLOW_MEM_PARM *mem_parm, const CHAR *filename)
{
	FILE *fd;
	INT32 size = 0;

	fd = fopen(filename, "rb");

	if (!fd) {
		printf("cannot read %s\r\n", filename);
		return -1;
	}

	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	
	if (size < 0) {
		printf("getting %s size failed\r\n", filename);
	} else if ((INT32)fread((VOID *)mem_parm->va, 1, size, fd) != size) {
		printf("read size < %ld\r\n", size);
		size = -1;
	}
	//mem_parm->size = size;

	if (fd) {
		fclose(fd);
	}

	return size;
}

/*-----------------------------------------------------------------------------*/
/* Input Functions                                                             */
/*-----------------------------------------------------------------------------*/

///////////////////////////////////////////////////////////////////////////////


static HD_RESULT input_open(NET_IN *p_nn_in, VENDOR_AIS_FLOW_MEM_PARM *srcimg_buf)
{
	HD_RESULT ret = HD_OK;

	UINT32 file_len = mem_load(srcimg_buf, p_nn_in->input_filename);
	if (file_len < 0) {
		printf("load buf(%s) fail\r\n", p_nn_in->input_filename);
		return HD_ERR_NG;
	}
	printf("load buf(%s) ok\r\n", p_nn_in->input_filename);
	hd_common_mem_flush_cache((VOID *)srcimg_buf->va, file_len);

	p_nn_in->src_img.width 		= p_nn_in->w;
	p_nn_in->src_img.height 	= p_nn_in->h;
	p_nn_in->src_img.channel 	= p_nn_in->c;
	p_nn_in->src_img.line_ofs 	= p_nn_in->loff;
	p_nn_in->src_img.fmt      	= p_nn_in->fmt;
	p_nn_in->src_img.pa      	= srcimg_buf->pa;
	p_nn_in->src_img.va   		= srcimg_buf->va;
	p_nn_in->src_img.sign 		= MAKEFOURCC('A','B','U','F');
	p_nn_in->src_img.size 		= p_nn_in->loff * p_nn_in->h * 3 / 2;

	return ret;
}


/*-----------------------------------------------------------------------------*/
/* Network Functions                                                             */
/*-----------------------------------------------------------------------------*/


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

HD_RESULT load_model(CHAR *filename, UINT32 va)
{
	FILE *fd;
	INT32 size = 0;
	fd = fopen(filename, "rb");
	if (!fd) {
		printf("cannot read %s\r\n", filename);
		return HD_ERR_NOT_OPEN;
	}

	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	if (size < 0) {
		printf("getting %s size failed\r\n", filename);
	} else if ((INT32)fread((VOID *)va, 1, size, fd) != size) {
		printf("read size < %ld\r\n", size);
		size = -1;
	};
	//mem_parm->size = size;

	if (fd) {
		fclose(fd);
	};
	printf("model buf size: %d\r\n", size);
	return HD_OK;
}

static HD_RESULT mem_rel(VOID)
{
	HD_RESULT ret = HD_OK;
	/* Release in buffer */
	if (g_mem.va > 0) {
		ret = hd_common_mem_munmap((void *)g_mem.va, g_mem.size);
		if (ret != HD_OK) {
			printf("mem_uninit : hd_common_mem_munmap fail (%ld).\r\n", ret);
			return ret;
		}
	}

	ret = hd_common_mem_release_block(g_blk);
	if (ret != HD_OK) {
		printf("mem_uninit : hd_common_mem_release_block fail (%ld).\r\n", ret);
		return ret;
	}

	hd_common_mem_uninit();
	return HD_OK;
}

HD_RESULT load_pvdcnn_model(PVDCNN_MEM *pvdcnn_mem, VENDOR_AIS_FLOW_MEM_PARM *buf)
{
	HD_RESULT ret = HD_OK;
	INT32 size = 0;

#if PDCNN_PROC
	size = _getsize_model(model_name[0]);
	ret = get_mem(buf, &pvdcnn_mem->pdbin_mem, size, 32);
	if(ret != HD_OK){
		printf("Get pdcnn bin mem fail (%d)!\r\n", ret);
		return ret;
	}
	ret = load_model(model_name[0], pvdcnn_mem->pdbin_mem.va);
	if(ret != HD_OK){
		printf("Load pdcnn nvt model fail (%d)!\n", ret);
		return ret;
	}
#endif
#if VDCNN_PROC
	size = _getsize_model(model_name[1]);
	ret = get_mem(buf, &pvdcnn_mem->vdbin_mem, size, 32);
	if(ret != HD_OK){
		printf("Get vdcnn bin mem fail (%d)!\r\n", ret);
		return ret;
	}
	ret = load_model(model_name[1], pvdcnn_mem->vdbin_mem.va);
	if(ret != HD_OK){
		printf("Load vdcnn nvt model fail (%d)!\n", ret);
		return ret;
	}
#endif

#if REDUCE_FP
	size = _getsize_model(model_name[2]);
	ret = get_mem(buf, &pvdcnn_mem->rfp_buf.bin_mem, size, 32);
	if(ret != HD_OK){
		printf("Get rfp mem bin fail (%d)!\r\n", ret);
		return ret;
	}
	ret = load_model(model_name[2], pvdcnn_mem->rfp_buf.bin_mem.va);
	if(ret != HD_OK){
		printf("Load rfp bin fail (%d)!\n", ret);
		return ret;
	}
#endif


	return ret;
}

VOID assign_ai_buf(PVD_THREAD_PARM *parm)
{
	parm->srcimg_mem.pa = g_mem.pa;
	parm->srcimg_mem.va = g_mem.va;
	parm->srcimg_mem.size = PVD_SRCIMG_MEM_SIZE;
	
	parm->pvd_mem.pa = parm->srcimg_mem.pa + parm->srcimg_mem.size;
	parm->pvd_mem.va = parm->srcimg_mem.va + parm->srcimg_mem.size;
	parm->pvd_mem.size = g_mem.size - parm->srcimg_mem.size;
}

static VOID *nn_thread_api(VOID *arg)
{
	HD_RESULT ret;
	PVD_THREAD_PARM* pvd_thread_parm = (PVD_THREAD_PARM*)arg;
	VENDOR_AIS_FLOW_MEM_PARM pvd_buf = pvd_thread_parm->pvd_mem;
	VENDOR_AIS_FLOW_MEM_PARM srcimg_buf = pvd_thread_parm->srcimg_mem;
	HD_GFX_IMG_BUF gfx_img = {0};
	VENDOR_AI_BUF	p_src_img = {0};
	UINT32 inimg_size = AI_VDO_W * AI_VDO_H * 3 / 2; 

	PVDCNN_PARAM pvdcnn_param = {0};
	PVDCNN_MEM pvdcnn_mem = {0};
	pvdcnn_param.proposal_param.pd_conf_thr = 0.45;
	pvdcnn_param.proposal_param.vd_conf_thr = 0.45;
	pvdcnn_param.proposal_param.nms_thr = 0.2;
	pvdcnn_param.pdnet_id = 0;
	pvdcnn_param.vdnet_id = 1;
	pvdcnn_param.pdcnn_proc = PDCNN_PROC;
	pvdcnn_param.vdcnn_proc = VDCNN_PROC;
	pvdcnn_param.rfp_param.rfpnet_id = 2;
	pvdcnn_param.rfp_param.rfp_proc = REDUCE_FP;
	pvdcnn_param.rfp_param.thr = 0.7;
	pvdcnn_param.pd_limit_param.limit_module = 2;
	pvdcnn_param.vd_limit_param.limit_module = 1;
	pvdcnn_param.pd_limit_param.sm_thr_num = 2;
	pvdcnn_param.vd_limit_param.sm_thr_num = 2;
	pvdcnn_param.pd_distance_mode = 0;
	pvdcnn_param.vd_distance_mode = 0;
	
	
	ret = load_pvdcnn_model(&pvdcnn_mem, &pvd_buf);
	if(ret != HD_OK){
		printf("Load pvdcnn model fail (%d)!\n", ret);
		goto exit_thread;
	}

	ret = pvdcnn_init(&pvdcnn_param, &pvdcnn_mem, &pvd_buf);
	if(ret != HD_OK){
		printf("PVDCNN init process fail (%d)!\n", ret);
		goto exit_thread;
	}

	ret = get_mem(&pvd_buf, &pvdcnn_mem.input_mem, inimg_size, 32);
	if(ret != HD_OK){
		printf("Get pvdcnn input mem fail (%d)!\n", ret);
		goto exit_thread;
	}

#if 1
	UINT32 ai_bufsize = calculate_pvdcnn_memsize(&pvdcnn_mem, &pvdcnn_param);
	printf("The total buf required by PVDCNN = %ld.\r\n", ai_bufsize);
#endif
	
	//image parameter
	NET_IN nn_in;
	nn_in.c = 2,
	nn_in.fmt = HD_VIDEO_PXLFMT_YUV420;

	FLOAT ratio_w = 0.0;
	FLOAT ratio_h = 0.0;


	UINT32 all_time = 0;
	INT32 img_num = 0;
#if SAVE_SCALE
	CHAR BMP_FILE[256];
#endif
	CHAR IMG_PATH[256];
	CHAR SAVE_TXT[256];
	CHAR IMG_LIST[256];
	CHAR list_infor[256];
	CHAR *line_infor;
	BOOL INPUT_STATE = TRUE;

	sprintf(IMG_LIST, "/mnt/sd/jpg/pvd_image_list.txt");
	sprintf(IMG_PATH, "/mnt/sd/jpg/PVD");

	FILE *fs, *fr;

    sprintf(SAVE_TXT, "/mnt/sd/det_results/pvd_test_results.txt");
	
	fr = fopen(IMG_LIST, "r");
	fs = fopen(SAVE_TXT, "w+");

	INT32 len = 0;
	CHAR img_name[256]={0};
	CHAR *token;
	INT32 sl = 0;

	if(NULL == fr)
	{
		printf("Failed to open img_list!\r\n");	
	} 
	while(fgets(list_infor, 256, fr) != NULL){
		len = strlen(list_infor);
		list_infor[len - 1] = '\0';
		sl = 0;
		line_infor = list_infor;

		while((token = strtok(line_infor, " ")) != NULL)
		{
			if(sl > 2){
				break;
			}
			if (sl == 0){
				strcpy(img_name, token);
				sprintf(nn_in.input_filename, "%s/%s", IMG_PATH, token);
				printf("%s ", token);
			}
			if (sl == 1){
				nn_in.w = atoi(token);
				nn_in.loff = ALIGN_CEIL_4(nn_in.w);
				printf("%s ", token);
			}
			if (sl == 2){
				nn_in.h = atoi(token);
				printf("%s\r\n", token);
			}
			line_infor = NULL;
			sl++;
		}

		if ((ret = input_open(&nn_in, &srcimg_buf)) != HD_OK) {
			printf("ERR: pvdcnn image open fail !!\n");
			goto exit_thread;
		}
		
		ratio_w = (FLOAT)nn_in.w / (FLOAT)AI_VDO_W;
		ratio_h = (FLOAT)nn_in.h / (FLOAT)AI_VDO_H;
		
		gfx_img.dim.w = AI_VDO_W;
		gfx_img.dim.h = AI_VDO_H;
		gfx_img.format = nn_in.src_img.fmt;
		gfx_img.lineoffset[0] = ALIGN_CEIL_4(AI_VDO_W);
		gfx_img.lineoffset[1] = ALIGN_CEIL_4(AI_VDO_H);
		gfx_img.p_phy_addr[0] = pvdcnn_mem.input_mem.pa;
		gfx_img.p_phy_addr[1] = pvdcnn_mem.input_mem.pa + AI_VDO_W * AI_VDO_H;

		PVD_IRECT roi = {0, 0, nn_in.src_img.width, nn_in.src_img.height};
		ret = pvdcnn_crop_img(&gfx_img, &(nn_in.src_img), HD_GFX_SCALE_QUALITY_NULL, &roi);
		if (ret != HD_OK) {
			printf("pvdcnn_crop_img fail=%d\n", ret);
		}
		pvd_gfx_img_to_vendor(&p_src_img, &gfx_img, (UINT32)pvdcnn_mem.input_mem.va);

		pvdcnn_param.rfp_param.ratiow = (FLOAT)p_src_img.width / (FLOAT)AI_VDO_W;
		pvdcnn_param.rfp_param.ratioh = (FLOAT)p_src_img.height / (FLOAT)AI_VDO_H;
#if SAVE_SCALE
		FILE *fb;
		sprintf(BMP_FILE, "/mnt/sd/save_bmp/%s_resize.bin", img_name);
		fb = fopen(BMP_FILE, "wb+");
		fwrite((UINT32 *)pvdcnn_mem.input_mem.va, sizeof(UINT32), (gfx_img.dim.h * gfx_img.dim.w + gfx_img.dim.h * gfx_img.dim.w / 2), fb);
		fclose(fb);
#endif

		PROF_START();

		ret = pvdcnn_process(&pvdcnn_param, &pvdcnn_mem, &p_src_img);
		if(ret != HD_OK){
			printf("ERR: pvdcnn_process fail (%d)!\r\n", ret);
			goto exit_thread;
		}

		PROF_END("PVDCNN");
		all_time += (tend.tv_sec - tstart.tv_sec) * 1000000 + (tend.tv_usec - tstart.tv_usec); 

		PVDCNN_RESULT *final_out_results = (PVDCNN_RESULT *)pvdcnn_mem.final_rslt.va;
		for(INT32 num = 0; num < pvdcnn_mem.out_num; num++){
			//printf("before: %f %f %f %f %f.\r\n", final_out_results[num].score, final_out_results[num].x1, final_out_results[num].y1, final_out_results[num].x2 - final_out_results[num].x1, final_out_results[num].y2 - final_out_results[num].y1);
			FLOAT xmin = final_out_results[num].x1 * ratio_w;
			FLOAT ymin = final_out_results[num].y1 * ratio_h;
			FLOAT width = final_out_results[num].x2 * ratio_w - xmin;
			FLOAT height = final_out_results[num].y2 * ratio_h - ymin;
			FLOAT score = final_out_results[num].score;
			INT32 category = final_out_results[num].category;
			
			printf("obj information is: (category: %d score: %f [%f %f %f %f])\r\n", category, score, xmin, ymin, width, height);
			fprintf(fs, "%s %d %f %f %f %f %f\r\n", img_name, category, score, xmin, ymin, width, height);
		}
		
		img_num++;

	}
	if (INPUT_STATE == TRUE){
    	printf("all test img number is: %d\r\n", img_num);
	}
	fclose(fs);
	fclose(fr);
	//printf("mean_time: %d\r\n", all_time / img_num);
	printf("mean_time: %d\r\n", all_time / img_num);
	
exit_thread:

	ret = pvdcnn_uninit(&pvdcnn_param);
	if(ret != HD_OK){
		printf("ERR: pvdcnn_uninit fail (%d)!!\r\n", ret);
	}
	
	return 0;
}

/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	HD_RESULT ret;
	pthread_t nn_thread_id;
	PVD_THREAD_PARM pvd_thread_parm;

	// init hdal
	ret = hd_common_init(0);
	if (ret != HD_OK) {
		printf("hd_common_init fail=%d\n", ret);
		goto exit;
	}
	// set project config for AI
	hd_common_sysconfig(0, (1<<16), 0, VENDOR_AI_CFG); //enable AI engine

	ret = hd_gfx_init();
	if (ret != HD_OK) {
		printf("hd_gfx_init fail\r\n");
		goto exit;
	}

	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};
	mem_cfg.pool_info[0].type = HD_COMMON_MEM_USER_DEFINIED_POOL;
	mem_cfg.pool_info[0].blk_size = PVD_MAX_MEM_SIZE;
	mem_cfg.pool_info[0].blk_cnt = 1;
	mem_cfg.pool_info[0].ddr_id = DDR_ID0;
	ret = hd_common_mem_init(&mem_cfg);
	if (HD_OK != ret) {
		printf("hd_common_mem_init err: %d\r\n", ret);
		goto exit;
	}
	
	ret = mem_get();
	if(ret != HD_OK){
		printf("pdcnn all mem get fail (%d)!!\r\n", ret);
		goto exit;
	}

	//ai cfg set
	UINT32 schd = VENDOR_AI_PROC_SCHD_FAIR;
	vendor_ai_cfg_set(VENDOR_AI_CFG_PLUGIN_ENGINE, vendor_ai_cpu1_get_engine());
	vendor_ai_cfg_set(VENDOR_AI_CFG_PROC_SCHD, &schd);
	ret = vendor_ai_init();
	if (ret != HD_OK) {
		printf("vendor_ai_init fail=%d\n", ret);
		goto exit;
	}
	//printf("before NN g_mem: pa=(%#x), va=(%#x), size=(%ld)\r\n", g_mem.pa, g_mem.va, g_mem.size);
	assign_ai_buf(&pvd_thread_parm);

	ret = pthread_create(&nn_thread_id, NULL, nn_thread_api, (VOID *)(&pvd_thread_parm));
	if (ret < 0) {
		printf("create encode thread failed");
		goto exit;
	}
	
	pthread_join(nn_thread_id, NULL);

exit:
	ret = hd_gfx_uninit();
	if (ret != HD_OK) {
		printf("hd_gfx_uninit fail\r\n");
	}

    ret = vendor_ai_uninit();
    if (ret != HD_OK) {
        printf("vendor_ai_uninit fail=%d\n", ret);
    }
	
	ret = mem_rel();
	if(ret != HD_OK){
		printf("release pdcnn buf fail!!\r\n");
	}
			
	// uninit hdal
	ret = hd_common_uninit();
	if (ret != HD_OK) {
		printf("common fail=%d\n", ret);
	}
	printf("test finish!!!\r\n");
	return ret;
}
