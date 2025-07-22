/**
	@brief Source file of custom layers.

	@file net_cust_layer.c

	@ingroup custnn

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

/*-----------------------------------------------------------------------------*/
/* Include Files                                                               */
/*-----------------------------------------------------------------------------*/
#include "hd_type.h"
#include "vendor_ai_util.h"

#include "vendor_ai_cpu_custnn.h"         // NN_CUSTOMER
#include "vendor_ai_cpu_custnn_sample.h"
#include "nvtnn_lib.h"
#include <stdio.h>
#include <string.h>
#if NN_USE_DSP
#include "custnn_dsp.h"
#endif
#include "hd_common.h"

/*-----------------------------------------------------------------------------*/
/* Macro Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/
#define CUST_PRINT_PARM     FALSE

typedef struct _CUSTNN_POOL_PROTO_PARM {
	INT32 pool_type;	// 0:MAX; 1:average
	INT32 kerl_sz;
	INT32 stride;
	INT32 pad;
} CUSTNN_POOL_PROTO_PARM;

typedef struct _CUSTNN_SLICE_PROTO_PARM {
	INT32 axis;
	INT32 slice_point_cnt;
	INT32 slice_point[256];
} CUSTNN_SLICE_PROTO_PARM;
/*-----------------------------------------------------------------------------*/
/* Interface Functions                                                         */
/*-----------------------------------------------------------------------------*/
static INT32 get_hashcode(CHAR *p_str)
{
	INT32 hash = 0, i;
	INT32 len = strlen(p_str);
	if (len == 0) {
		return hash;
	}
	for (i = 0; i < len; i++) {
		hash = (31 * hash) + (INT32)p_str[i];
	}
	return hash;
}
UINT32 vendor_ai_cpu_cust_get_layer_id(UINT32 parm_addr)
{
	NN_CUSTOM_PARM *p_head = (NN_CUSTOM_PARM *)parm_addr;
#if CUST_SUPPORT_MULTI_IO
	UINT32 input_num = p_head->input_num;
	UINT32 output_num = p_head->output_num;
	UINT32 model_num = p_head->model_num;
	UINT32 dim_addr = (UINT32)(parm_addr + sizeof(NN_CUSTOM_PARM) + (input_num+output_num+model_num)*sizeof(NN_DATA));
	UINT32 *p_layer_type_id = (UINT32 *)(dim_addr + sizeof(NN_CUSTOM_DIM)*(input_num+output_num));
#else
	UINT32 *p_layer_type_id = (UINT32 *)(p_head + 1);
#endif

	return p_layer_type_id[0];
}

HD_RESULT vendor_ai_cpu_cust_init(UINT32 parm_addr, UINT32 net_id)
{
	/*
		custom parm format:
		1. custom_layer_type_id(UINT32)
		2. weight_num(UINT32), weight_size_1(UINT32), weight_size_2(UINT32), ...
		   (weights at p_head->model.va)
		3. prototxt_parm_len(UINT32),
		4. isf_len(INT32), isf_mul_1(INT32), isf_shift_1(INT32), isf_mul_2(INT32), isf_shift_2(INT32), ...
		5. osf_len(INT32), osf_mul_1(INT32), osf_shift_1(INT32), osf_mul_2(INT32), osf_shift_2(INT32), ...
		6. bin_parm_len(UINT32), p_prototxt_parm, p_bin_parm
	*/

	return HD_OK;
}

HD_RESULT vendor_ai_cpu_cust_uninit(UINT32 parm_addr, UINT32 net_id)
{
	/*
		custom parm format:
		1. custom_layer_type_id(UINT32)
		2. weight_num(UINT32), weight_size_1(UINT32), weight_size_2(UINT32), ...
		   (weights at p_head->model.va)
		3. prototxt_parm_len(UINT32),
		4. isf_len(INT32), isf_mul_1(INT32), isf_shift_1(INT32), isf_mul_2(INT32), isf_shift_2(INT32), ...
		5. osf_len(INT32), osf_mul_1(INT32), osf_shift_1(INT32), osf_mul_2(INT32), osf_shift_2(INT32), ...
		6. bin_parm_len(UINT32), p_prototxt_parm, p_bin_parm
	*/

	return HD_OK;
}

HD_RESULT vendor_ai_cpu_cust_set_tmp_buf(NN_DATA tmp_buf, UINT32 parm_addr)
{
	/*
		custom parm format:
		1. custom_layer_type_id(UINT32)
		2. weight_num(UINT32), weight_size_1(UINT32), weight_size_2(UINT32), ...
		   (weights at p_head->model.va)
		3. prototxt_parm_len(UINT32),
		4. isf_len(INT32), isf_mul_1(INT32), isf_shift_1(INT32), isf_mul_2(INT32), isf_shift_2(INT32), ...
		5. osf_len(INT32), osf_mul_1(INT32), osf_shift_1(INT32), osf_mul_2(INT32), osf_shift_2(INT32), ...
		6. bin_parm_len(UINT32), p_prototxt_parm, p_bin_parm
	*/
#if 0
	NN_CUSTOM_PARM *p_head = (NN_CUSTOM_PARM *)(parm_addr);
	UINT32 *p_layer_type_id = (UINT32 *)(p_head + 1);
	UINT32 *p_weight_num = (UINT32 *)(p_layer_type_id + 1);
	UINT32 weight_num = *p_weight_num;
	UINT32 *p_layer_parm_len = (UINT32 *)(p_weight_num + 1 + weight_num);

	UINT32 *p_isf_len = (UINT32 *)(p_layer_parm_len + 1);
	UINT32 isf_num = (*p_isf_len) / 8;
	INT32 *p_isf_parm = (INT32 *)(p_isf_len + 1);
	UINT32 *p_osf_len = (UINT32 *)(p_isf_parm + isf_num*2);
	UINT32 osf_num = (*p_osf_len) / 8;
	INT32 *p_osf_parm = (INT32 *)(p_osf_len + 1);
	UINT32 *p_bin_parm_len = (UINT32 *)(p_osf_parm + osf_num*2);
	UINT32 bin_parm_len = p_bin_parm_len[0];
	NN_DATA* p_tmp_buf = NULL;

	if (tmp_buf.size > 0 && tmp_buf.va > 0) {
		p_tmp_buf = (NN_DATA*)(parm_addr + sizeof(NN_CUSTOM_PARM) + bin_parm_len);
		memcpy(p_tmp_buf, &tmp_buf, sizeof(NN_DATA));
	}
#endif

	return HD_OK;
}

HD_RESULT vendor_ai_cpu_cust(UINT32 parm_addr, UINT32 net_id, UINT32 layer_id, VOID* usr_info)
{
	/*
		custom parm format:
		1. custom_layer_type_id(UINT32)
		2. weight_num(UINT32), weight_size_1(UINT32), weight_size_2(UINT32), ...
		   (weights at p_head->model.va)
		3. prototxt_parm_len(UINT32),
		4. isf_len(INT32), isf_mul_1(INT32), isf_shift_1(INT32), isf_mul_2(INT32), isf_shift_2(INT32), ...
		5. osf_len(INT32), osf_mul_1(INT32), osf_shift_1(INT32), osf_mul_2(INT32), osf_shift_2(INT32), ...
		6. bin_parm_len(UINT32), p_prototxt_parm, p_bin_parm
		7. tmp_buf addr(NN_DATA)
	*/
	NN_CUSTOM_PARM *p_head = (NN_CUSTOM_PARM *)(parm_addr);
#if CUST_SUPPORT_MULTI_IO
	UINT32 input_num = p_head->input_num;
	UINT32 output_num = p_head->output_num;
	UINT32 model_num = p_head->model_num;
	NN_DATA* input_info = (NN_DATA*)(parm_addr + sizeof(NN_CUSTOM_PARM));
	NN_DATA* output_info = (NN_DATA*)(parm_addr + sizeof(NN_CUSTOM_PARM) + input_num*sizeof(NN_DATA));
	UINT32 dim_addr = (UINT32)(parm_addr + sizeof(NN_CUSTOM_PARM) + (input_num+output_num+model_num)*sizeof(NN_DATA));
	NN_CUSTOM_DIM* input_dim = (NN_CUSTOM_DIM*)(dim_addr);
	NN_CUSTOM_DIM* output_dim = (NN_CUSTOM_DIM*)(dim_addr + sizeof(NN_CUSTOM_DIM)*input_num);
	UINT32 *p_layer_type_id = (UINT32 *)(dim_addr + sizeof(NN_CUSTOM_DIM)*(input_num+output_num));
#else
	UINT32 *p_layer_type_id = (UINT32 *)(p_head + 1);
#endif
	INT32 layer_type_id = (INT32)(*p_layer_type_id);
	UINT32 *p_weight_num = (UINT32 *)(p_layer_type_id + 1);
	UINT32 weight_num = *p_weight_num;
	UINT32 *p_prototxt_parm_len = (UINT32 *)(p_weight_num + 1 + weight_num);

	UINT32 *p_isf_len = (UINT32 *)(p_prototxt_parm_len + 1);
	UINT32 isf_num = (*p_isf_len) / 8;
	INT32 *p_isf_parm = (INT32 *)(p_isf_len + 1);
	UINT32 *p_osf_len = (UINT32 *)(p_isf_parm + isf_num*2);
	UINT32 osf_num = (*p_osf_len) / 8;
	INT32 *p_osf_parm = (INT32 *)(p_osf_len + 1);
#if 0
	UINT32 *p_bin_parm_len = (UINT32 *)(p_osf_parm + osf_num*2);
	CUSTNN_ELT_PARM *p_elt_parm = (CUSTNN_ELT_PARM *)(p_bin_parm_len + 1);
	CUSTNN_POOL_PARM *p_pool_parm = (CUSTNN_POOL_PARM *)(p_bin_parm_len + 1);
#if CUST_SUPPORT_MULTI_IO
	CUSTNN_CONCAT_PARM *p_concat_parm = (CUSTNN_CONCAT_PARM *)(p_bin_parm_len + 1);
	CUSTNN_SLICE_PARM *p_slice_parm = (CUSTNN_SLICE_PARM *)(p_bin_parm_len + 1);
#endif
#else
	UINT32 *p_bin_parm_len = (UINT32 *)(p_osf_parm + osf_num * 2);
	CHAR *p_prototxt_parm = (CHAR *)(p_bin_parm_len + 1);
	CHAR *p_bin_parm = (CHAR *)(p_prototxt_parm + *p_prototxt_parm_len);
	UINT32 bin_length = (UINT32)p_bin_parm - (UINT32)p_layer_type_id - *p_bin_parm_len;
	
	CUSTNN_ELT_PARM *p_elt_parm = (CUSTNN_ELT_PARM *)(p_bin_parm);
	CUSTNN_POOL_PARM *p_pool_parm = (CUSTNN_POOL_PARM *)(p_bin_parm);
#if CUST_SUPPORT_MULTI_IO
	CUSTNN_CONCAT_PARM *p_concat_parm = (CUSTNN_CONCAT_PARM *)(p_bin_parm);
	CUSTNN_SLICE_PARM *p_slice_parm = (CUSTNN_SLICE_PARM *)(p_bin_parm);
#endif
#endif
	CUSTNN_ELTWISE_PROTO_PARM *p_proto_eltwise_parm = (CUSTNN_ELTWISE_PROTO_PARM*)p_prototxt_parm;
	
#if 0
	CUSTNN_POOL_PROTO_PARM *p_proto_pool_parm = (CUSTNN_POOL_PROTO_PARM*)p_prototxt_parm;
	UINT32 i = 0;
	//CUSTNN_SLICE_PROTO_PARM *p_proto_slice_parm = (CUSTNN_SLICE_PROTO_PARM*)p_prototxt_parm;
	printf("pool_type: %d\n", (int)p_proto_pool_parm->pool_type);
	printf("kerl_sz: %d\n", (int)p_proto_pool_parm->kerl_sz);
	printf("stride: %d\n", (int)p_proto_pool_parm->stride);
	printf("pad: %d\n", (int)p_proto_pool_parm->pad);

	printf("temp buf size = %d\n", (unsigned int)p_head->temp_buf_size);

	printf("input num = %d\n", (int)input_num);
	for (i = 0; i < input_num; i++) {
		printf("input %d size = %d\n", (int)i, (unsigned int)input_info[i].size);
		printf("         fmt (frac/int/sign) = %d %d %d\n", (int)input_info[i].fmt.frac_bits, (int)input_info[i].fmt.int_bits, (int)input_info[i].fmt.sign_bits);
		printf("         dim = %d %d %d %d %d\n", (unsigned int)input_dim[i].dim[0], (unsigned int)input_dim[i].dim[1],
		(unsigned int)input_dim[i].dim[2], (unsigned int)input_dim[i].dim[3], (unsigned int)input_dim[i].dim[4] );
		printf("         ofs = %d %d %d %d\n", (unsigned int)input_dim[i].ofs[0], (unsigned int)input_dim[i].ofs[1],
		(unsigned int)input_dim[i].ofs[2], (unsigned int)input_dim[i].ofs[3]);
	}
	
	printf("output num = %d\n", (int)output_num);
	for (i = 0; i < output_num; i++) {
		printf("output %d size = %d\n", (int)i, (unsigned int)output_info[i].size);
		printf("         fmt (frac/int/sign) = %d %d %d\n", (int)output_info[i].fmt.frac_bits, (int)output_info[i].fmt.int_bits, (int)output_info[i].fmt.sign_bits);
		printf("         dim = %d %d %d %d %d\n", (unsigned int)output_dim[i].dim[0], (unsigned int)output_dim[i].dim[1],
		(unsigned int)output_dim[i].dim[2], (unsigned int)output_dim[i].dim[3], (unsigned int)output_dim[i].dim[4] );
		printf("         ofs = %d %d %d %d\n", (unsigned int)output_dim[i].ofs[0], (unsigned int)output_dim[i].ofs[1],
		(unsigned int)output_dim[i].ofs[2], (unsigned int)output_dim[i].ofs[3]);
	}
#endif

#if NN_USE_DSP
	UINT32 in_pa, out_pa;
	NN_POOL_PARM parm = {0};
#endif

#if CUST_PRINT_PARM
#if CUST_SUPPORT_MULTI_IO
	UINT32 i = 0;
	NN_DATA* model_info = (NN_DATA*)(parm_addr + sizeof(NN_CUSTOM_PARM) + (input_num+output_num)*sizeof(NN_DATA));
	for (i = 0; i < input_num; i++) {
		DBGH(input_info[i].pa);
		DBGH(input_info[i].va);
		DBGH(input_info[i].size);
		DBGH(*(UINT32 *)&input_info[i].fmt);
	}
	for (i = 0; i < output_num; i++) {
		DBGH(output_info[i].pa);
		DBGH(output_info[i].va);
		DBGH(output_info[i].size);
		DBGH(*(UINT32 *)&output_info[i].fmt);
	}
	for (i = 0; i < model_num; i++) {
		DBGH(model_info[i].pa);
		DBGH(model_info[i].va);
		DBGH(model_info[i].size);
		DBGH(*(UINT32 *)&model_info[i].fmt);
	}
#else
	DBGH(p_head->input.pa);
	DBGH(p_head->input.va);
	DBGH(p_head->input.size);
	DBGH(*(UINT32 *)&p_head->input.fmt);
	DBGH(p_head->output.pa);
	DBGH(p_head->output.va);
	DBGH(p_head->output.size);
	DBGH(*(UINT32 *)&p_head->output.fmt);
	DBGH(p_head->model.pa);
	DBGH(p_head->model.va);
	DBGH(p_head->model.size);
	DBGH(*(UINT32 *)&p_head->model.fmt);
#endif
	DBGD(p_head->parm_size);

	DBGD(*p_layer_type_id);
	DBGD(weight_num);
	//DBGD(layer_parm_len);
	DBGD(*p_bin_parm_len);
	
	//DBG_DUMP("layer_parm=%s\r\n", p_layer_parm);
#endif

	if (layer_type_id == get_hashcode("Pooling")) {
		// pooling
#if CUST_SUPPORT_MULTI_IO
		p_pool_parm->batch_num = input_dim[0].dim[3];
		custnn_cpu_scaleshift_process(input_info[0].va, p_head->temp_buf_addr, input_info[0].size, p_pool_parm->in_type, p_isf_parm[0], p_isf_parm[1]);
		custnn_cpu_pool_process(p_pool_parm, p_head->temp_buf_addr, output_info[0].va);
		custnn_cpu_scaleshift_process(output_info[0].va, output_info[0].va, output_info[0].size, p_pool_parm->out_type, p_osf_parm[0], p_osf_parm[1]);
#else
		custnn_cpu_scaleshift_process(p_head->input.va, p_head->input.va, p_head->input.size, p_pool_parm->in_type, p_isf_parm[0], p_isf_parm[1]);
		custnn_cpu_pool_process(p_pool_parm, p_head->input.va, p_head->output.va);
		custnn_cpu_scaleshift_process(p_head->output.va, p_head->output.va, p_head->output.size, p_pool_parm->out_type, p_osf_parm[0], p_osf_parm[1]);
#endif
} else if (layer_type_id == get_hashcode("Eltwise")) {
		// eltwise
#if CUST_SUPPORT_MULTI_IO
	if (bin_length == 0) { //prototxt mode				
		NN_ELTWISE_PARM nn_eltwise_parm;				
		nn_eltwise_parm.coeff0 = p_proto_eltwise_parm->elt_coeff0;				
		nn_eltwise_parm.coeff1 = p_proto_eltwise_parm->elt_coeff1;				
		nn_eltwise_parm.op = p_proto_eltwise_parm->operation;				
		nn_eltwise_parm.in_addr0 = input_info[0].va;				
		nn_eltwise_parm.in_addr1 = input_info[1].va;				
		nn_eltwise_parm.in_fmt0 = input_info[0].fmt;				
		nn_eltwise_parm.in_fmt1 = input_info[1].fmt;				
		nn_eltwise_parm.in_ofs0.line_ofs = input_dim[0].ofs[0];				
		nn_eltwise_parm.in_ofs0.channel_ofs = input_dim[0].ofs[1];				
		nn_eltwise_parm.in_ofs0.batch_ofs = input_dim[0].ofs[2];				
		nn_eltwise_parm.in_ofs1.line_ofs = input_dim[1].ofs[0];				
		nn_eltwise_parm.in_ofs1.channel_ofs = input_dim[1].ofs[1];				
		nn_eltwise_parm.in_ofs1.batch_ofs = input_dim[1].ofs[2];				
		nn_eltwise_parm.isf0.mul = 1;				
		nn_eltwise_parm.isf0.shf = 0;				
		nn_eltwise_parm.isf1.mul = 1;				
		nn_eltwise_parm.isf1.shf = 0;				
		nn_eltwise_parm.osf.mul = 1;				
		nn_eltwise_parm.osf.shf = 0;				
		nn_eltwise_parm.out_addr = output_info[0].va;				
		nn_eltwise_parm.out_fmt = output_info[0].fmt;				
		nn_eltwise_parm.out_ofs.line_ofs = output_dim[0].ofs[0];				
		nn_eltwise_parm.out_ofs.channel_ofs = output_dim[0].ofs[1];				
		nn_eltwise_parm.out_ofs.batch_ofs = output_dim[0].ofs[2];				
		nn_eltwise_parm.shape.batch_num = output_dim[0].dim[3];				
		nn_eltwise_parm.shape.channel = output_dim[0].dim[2];				
		nn_eltwise_parm.shape.height = output_dim[0].dim[1];				
		nn_eltwise_parm.shape.width = output_dim[0].dim[0];				
		nn_eltwise_parm.tmp_addr = p_head->temp_buf_addr;				
		nvtnn_eltwise_process(&nn_eltwise_parm);			
	}
	else {//bin mode
		UINT32 input0_addr = p_head->temp_buf_addr;
		UINT32 input1_addr = p_head->temp_buf_addr + input_info[0].size;
		UINT32 output_addr = output_info[0].va;
		UINT32 input0_size = input_info[0].size;
		UINT32 input1_size = input_info[1].size;
		UINT32 output_size = output_info[0].size;

		custnn_cpu_scaleshift_process(input_info[0].va, input0_addr, input0_size, p_elt_parm->in0_type, p_isf_parm[0], p_isf_parm[1]);
		custnn_cpu_scaleshift_process(input_info[1].va, input1_addr, input1_size, p_elt_parm->in1_type, p_isf_parm[2], p_isf_parm[3]);
		custnn_cpu_elt_process(p_elt_parm, input0_addr, input1_addr, output_addr);
		custnn_cpu_scaleshift_process(output_addr, output_addr, output_size, p_elt_parm->out_type, p_osf_parm[0], p_osf_parm[1]);
	}
#else
		custnn_cpu_scaleshift_process(p_head->input.va, p_head->input.va, p_head->input.size, p_elt_parm->in0_type, p_isf_parm[0], p_isf_parm[1]);
		custnn_cpu_scaleshift_process(p_head->model.va, p_head->model.va, p_head->model.size, p_elt_parm->in1_type, p_isf_parm[2], p_isf_parm[3]);
		custnn_cpu_elt_process(p_elt_parm, p_head->input.va, p_head->model.va, p_head->output.va);
		custnn_cpu_scaleshift_process(p_head->output.va, p_head->output.va, p_head->output.size, p_elt_parm->out_type, p_osf_parm[0], p_osf_parm[1]);
#endif
#if CUST_SUPPORT_MULTI_IO
	} else if (layer_type_id == get_hashcode("Concat")) {
		// concat
		custnn_cpu_concat_process(p_concat_parm, p_head->temp_buf_addr, input_info, input_dim, p_isf_parm, input_num, output_info, output_dim, p_osf_parm, output_num);
	} else if (layer_type_id == get_hashcode("Slice")) {
		// slice
		custnn_cpu_slice_process(p_slice_parm, p_head->temp_buf_addr, input_info, input_dim, p_isf_parm, input_num, output_info, output_dim, p_osf_parm, output_num);
#endif
	} else {
		printf("custnn: unknown customer layer type\n");
	}


#if 0  //sample code: update info of out dim

	// update dim of layer[2].out[0]
	if(layer_id == 2){
		UINT32 out_id = 0;
		output_dim[out_id].dim[0] = 10;
		output_dim[out_id].dim[1] = 10;
		output_dim[out_id].dim[2] = 64;
		output_dim[out_id].dim[3] = 1;
		output_dim[out_id].ofs[0] = 10;
		output_dim[out_id].ofs[1] = 100;
		output_dim[out_id].ofs[2] = 6400;
		output_dim[out_id].ofs[3] = 6400;
		if (vendor_ai_cpu_cust_set_out_dim(net_id, layer_id, out_id, &output_dim[out_id]) != HD_OK) {
			printf("custnn: vendor_ai_cpu_cust_set_out_dim fail!\n");
		}
	}
	
	// update dim of layer[3].out[0]
	if(layer_id == 3){
		UINT32 out_id = 0;
		output_dim[out_id].dim[0] = 56;
		output_dim[out_id].dim[1] = 64;
		output_dim[out_id].dim[2] = 56;
		output_dim[out_id].dim[3] = 1;
		output_dim[out_id].ofs[0] = 56;
		output_dim[out_id].ofs[1] = 3584;
		output_dim[out_id].ofs[2] = 200704;
		output_dim[out_id].ofs[3] = 200704;
		if (vendor_ai_cpu_cust_set_out_dim(net_id, layer_id, out_id, &output_dim[out_id]) != HD_OK) {
			printf("custnn: vendor_ai_cpu_cust_set_out_dim fail!\n");
		}
	}
#endif

	return HD_OK;
}

