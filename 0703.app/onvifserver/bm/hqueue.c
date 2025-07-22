

#include "sys_inc.h"
#include "hqueue.h"

/***************************************************************************************/
HT_API HQUEUE * hqCreate(uint32 unit_num, uint32 unit_size, uint32 queue_mode)
{
	uint32 q_len = unit_num * unit_size + sizeof(HQUEUE);

	HQUEUE *phq = (HQUEUE *)malloc(q_len);
	if (phq == NULL)
	{
		return NULL;
	}
	
	phq->queue_mode = queue_mode;
	phq->unit_size = unit_size;
	phq->unit_num = unit_num;
	phq->front = 0;
	phq->rear = 0;
	phq->count_put_full = 0;
	phq->queue_buffer = sizeof(HQUEUE);

	if (queue_mode & HQ_NO_EVENT)
	{
		phq->queue_nnulEvent = NULL;
		phq->queue_nfulEvent = NULL;
		phq->queue_putMutex = NULL;
	}
	else
	{
		phq->queue_nnulEvent = sys_os_create_sig();
		phq->queue_nfulEvent = sys_os_create_sig();
		phq->queue_putMutex = sys_os_create_mutex();
	}

	return phq;
}

HT_API void hqDelete(HQUEUE * phq)
{
	if (phq == NULL)
	{
		return;
	}
	
	if (phq->queue_mode & HQ_NO_EVENT)
	{
	}
	else
	{
		sys_os_destroy_sig_mutex(phq->queue_nnulEvent);
		sys_os_destroy_sig_mutex(phq->queue_nfulEvent);
		sys_os_destroy_sig_mutex(phq->queue_putMutex);
	}

	free(phq);
}

/***************************************************************************************/
void hqPutMutexEnter(HQUEUE * phq)
{
	if ((phq->queue_mode & HQ_NO_EVENT) == 0)
	{
		sys_os_mutex_enter(phq->queue_putMutex);
	}
}

void hqPutMutexLeave(HQUEUE * phq)
{
	if ((phq->queue_mode & HQ_NO_EVENT) == 0)
	{
		sys_os_mutex_leave(phq->queue_putMutex);
	}
}

HT_API BOOL hqBufPut(HQUEUE * phq, char * buf)
{
	uint32 real_rear, queue_count;
	char * ptr;

	if (phq == NULL || buf == NULL)
	{
		return FALSE;
	}

	hqPutMutexEnter(phq);

hqBufPut_start:

	queue_count = phq->rear - phq->front;

	if (queue_count == (phq->unit_num - 1))
	{
		if ((phq->queue_mode & HQ_NO_EVENT) == 0)
		{
			if (phq->queue_mode & HQ_PUT_WAIT)
			{
				sys_os_sig_wait(phq->queue_nfulEvent);
				goto hqBufPut_start;
			}
			else 
			{
				phq->count_put_full++;
				hqPutMutexLeave(phq);
				
				return FALSE;
			}
		}
		else
		{
			hqPutMutexLeave(phq);
            return FALSE;
		}
	}

	real_rear = phq->rear % phq->unit_num;
	ptr = ((char *)phq) + phq->queue_buffer + real_rear*phq->unit_size;
	memcpy((char *)ptr, buf, phq->unit_size);
	phq->rear++;

	if ((phq->queue_mode & HQ_NO_EVENT) == 0)
	{
		sys_os_sig_sign(phq->queue_nnulEvent);
	}

	hqPutMutexLeave(phq);

	return TRUE;
}

HT_API BOOL hqBufGet(HQUEUE * phq, char * buf)
{
	uint32 real_front;

	if (phq == NULL || buf == NULL)
	{
		return FALSE;
	}
	
hqBufGet_start:

	if (phq->front == phq->rear)
	{
		if ((phq->queue_mode & HQ_NO_EVENT) == 0)
		{
			if (phq->queue_mode & HQ_GET_WAIT)
			{
				sys_os_sig_wait(phq->queue_nnulEvent);
				goto hqBufGet_start;
			}
			else 
			{
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}	
	}

	real_front = phq->front % phq->unit_num;
	memcpy(buf, ((char *)phq) + phq->queue_buffer + real_front*phq->unit_size, phq->unit_size);
	phq->front++;

	if ((phq->queue_mode & HQ_NO_EVENT) == 0)
	{
		sys_os_sig_sign(phq->queue_nfulEvent);
	}

	return TRUE;
}

HT_API BOOL hqBufIsEmpty(HQUEUE * phq)
{
	if (phq == NULL)
	{
		return TRUE;
	}
	
	if (phq->front == phq->rear)
	{
		return TRUE;
	}
	
	return FALSE;
}

HT_API BOOL hqBufIsFull(HQUEUE * phq)
{
	uint32 queue_count;
	
	queue_count = phq->rear - phq->front;

	if (queue_count == (phq->unit_num - 1))
	{
		return TRUE;
	}

	return FALSE;
}

HT_API char * hqBufGetWait(HQUEUE * phq)
{
	uint32 real_front;
	char * ptr = NULL;

	if (phq == NULL)
	{
		return NULL;
	}
	
hqBufGet_start:

	if (phq->front == phq->rear)
	{
		if ((phq->queue_mode & HQ_NO_EVENT) == 0)
		{
			if (phq->queue_mode & HQ_GET_WAIT)
			{
				sys_os_sig_wait(phq->queue_nnulEvent);
				goto hqBufGet_start;
			}
			else 
			{
				return NULL;
			}
		}
		else
		{
			return NULL;
		}	
	}

	real_front = phq->front % phq->unit_num;

	ptr = ((char *)phq) + phq->queue_buffer + real_front*phq->unit_size;
	return ptr;
}

HT_API void hqBufGetWaitPost(HQUEUE * phq)
{
	if (phq == NULL)
	{
		return;
	}
	
	phq->front++;

	if ((phq->queue_mode & HQ_NO_EVENT) == 0)
	{
		sys_os_sig_sign(phq->queue_nfulEvent);
	}
}

HT_API char * hqBufPutPtrWait(HQUEUE * phq)
{
	uint32 real_rear,queue_count;
	char * ptr;

	if (phq == NULL)
	{
		return NULL;
	}
	
	hqPutMutexEnter(phq);

hqBufPutPtr_start:

	queue_count = phq->rear - phq->front;

	if (queue_count == (phq->unit_num - 1))
	{
		if ((phq->queue_mode & HQ_NO_EVENT) == 0)
		{
			if (phq->queue_mode & HQ_PUT_WAIT)
			{
				sys_os_sig_wait(phq->queue_nfulEvent);
				goto hqBufPutPtr_start;
			}
			else 
			{
				phq->count_put_full++;
				hqPutMutexLeave(phq);
				return FALSE;
			}
		}
		else
		{
			hqPutMutexLeave(phq);
            return FALSE;
		}
	}

	real_rear = phq->rear % phq->unit_num;
	ptr = ((char *)phq) + phq->queue_buffer + real_rear*phq->unit_size;

	return ptr;
}

HT_API void hqBufPutPtrWaitPost(HQUEUE * phq, BOOL bPutFinish)
{
	if (phq == NULL)
	{
		return;
	}
	
	if (bPutFinish)
	{
		phq->rear++;
	}
	
	if ((phq->queue_mode & HQ_NO_EVENT) == 0)
	{
		sys_os_sig_sign(phq->queue_nnulEvent);
	}

	hqPutMutexLeave(phq);
}

HT_API BOOL hqBufPeek(HQUEUE * phq, char * buf)
{
	uint32 real_front;

	if (phq == NULL || buf == NULL)
	{
		return FALSE;
	}
	
hqBufPeek_start:

	if (phq->front == phq->rear)
	{
		if ((phq->queue_mode & HQ_NO_EVENT) == 0)
		{
			if (phq->queue_mode & HQ_GET_WAIT)
			{
				sys_os_sig_wait(phq->queue_nnulEvent);
				goto hqBufPeek_start;
			}
			else 
			{
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}	
	}

	real_front = phq->front % phq->unit_num;
	memcpy(buf, ((char *)phq) + phq->queue_buffer + real_front*phq->unit_size, phq->unit_size);

	return TRUE;
}




