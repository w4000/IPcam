

#include "sys_inc.h"
#include "sys_buf.h"


/***************************************************************************************/
PPSN_CTX * net_buf_fl = NULL;

PPSN_CTX * hdrv_buf_fl = NULL;


/***************************************************************************************/
HT_API BOOL net_buf_init(int num, int size)
{
	net_buf_fl = pps_ctx_fl_init(num, size, TRUE);
	if (net_buf_fl == NULL)
	{
		return FALSE;
	}
	

	return TRUE;
}

HT_API char * net_buf_get_idle()
{
	return (char *)pps_fl_pop(net_buf_fl);
}

HT_API void net_buf_free(char * rbuf)
{
	if (rbuf == NULL)
	{
		return;
	}
	
	if (pps_safe_node(net_buf_fl, rbuf))
	{
		pps_fl_push_tail(net_buf_fl, rbuf);
	}	
	else
	{
		free(rbuf);
	}	
}

HT_API uint32 net_buf_get_size()
{
	if (net_buf_fl == NULL)
	{
		return 0;
	}
	
	return (net_buf_fl->unit_size - sizeof(PPSN));
}

HT_API uint32 net_buf_idle_num()
{
	if (net_buf_fl == NULL)
	{
		return 0;
	}
	
	return net_buf_fl->node_num;
}

HT_API void net_buf_deinit()
{
	if (net_buf_fl)
	{
		pps_fl_free(net_buf_fl);
		net_buf_fl = NULL;
	}
}

HT_API BOOL hdrv_buf_init(int num)
{
	hdrv_buf_fl = pps_ctx_fl_init(num, sizeof(HDRV), TRUE);
	if (hdrv_buf_fl == NULL)
	{
		return FALSE;
	}
	

	return TRUE;
}

HT_API void hdrv_buf_deinit()
{
	if (hdrv_buf_fl)
	{
		pps_fl_free(hdrv_buf_fl);
		hdrv_buf_fl = NULL;
	}
}

HT_API HDRV * hdrv_buf_get_idle()
{
	HDRV * p_ret = (HDRV *)pps_fl_pop(hdrv_buf_fl);

	return p_ret;
}

HT_API void hdrv_buf_free(HDRV * pHdrv)
{
	if (pHdrv == NULL)
	{
		return;
	}
	
	pHdrv->header[0] = '\0';
	pHdrv->value_string = NULL;
	
	pps_fl_push(hdrv_buf_fl, pHdrv);
}

HT_API uint32 hdrv_buf_idle_num()
{
	if (NULL == hdrv_buf_fl)
	{
		return 0;
	}
	
	return hdrv_buf_fl->node_num;
}

HT_API void hdrv_ctx_ul_init(PPSN_CTX * ul_ctx)
{
	pps_ctx_ul_init_nm(hdrv_buf_fl, ul_ctx);
}

HT_API void hdrv_ctx_free(PPSN_CTX * p_ctx)
{
	HDRV * p_free;

	if (p_ctx == NULL)
	{
		return;
	}
	
	p_free = (HDRV *)pps_lookup_start(p_ctx);
	while (p_free != NULL) 
	{
		HDRV * p_next = (HDRV *)pps_lookup_next(p_ctx, p_free);

		pps_ctx_ul_del(p_ctx, p_free);
		hdrv_buf_free(p_free);

		p_free = p_next;		
	}
	pps_lookup_end(p_ctx);
}

#define NET_BUF_SIZE (4096) //2048
HT_API BOOL sys_buf_init(int nums)
{
	if (net_buf_init(nums, NET_BUF_SIZE) == FALSE)
	{
		return FALSE;
	}

	if (hdrv_buf_init(8*nums) == FALSE)
	{
		return FALSE;
	}
    
	return TRUE;
}

HT_API void sys_buf_deinit()
{
	net_buf_deinit();
	hdrv_buf_deinit();
}


