// CircularBuffer.cpp: implementation of the CCircularBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "common.h"

//#include <string.h>
//#include <stdio.h>
//#include <stdlib.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//

int CCircularBuffer_Create(CCircularBuffer_t *cb, int nAllocSize)
{
	cb->m_nAllocSize = 0;
	cb->m_nEnteredCount = 0;
	cb->m_pBuffer	= 0x00;
	cb->m_nFront	= 0;
	cb->m_nBack		= 0;

	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, nAllocSize);
	cb->m_pBuffer = (unsigned char*)malloc(nAllocSize);
	cb->m_nAllocSize = nAllocSize;
	CCircularBuffer_Clear(cb);

	if (cb->m_pBuffer == 0x00)
	{
		printf( "\n CCircularBuffer Buffer Alloc Error\n " );

		return 0;
	}

	return 1;
}

void CCircularBuffer_Delete(CCircularBuffer_t *cb)
{
	CCircularBuffer_Clear(cb);
	if (cb->m_pBuffer)
	{
		__free(cb->m_pBuffer);
	}
	cb->m_pBuffer = 0x00;
}

int CCircularBuffer_IsEmpty(CCircularBuffer_t *cb) {
	if (!cb->m_pBuffer)
		return 1;
	if (cb->m_nEnteredCount < 1)
		return 1;
	return 0;
}

int CCircularBuffer_IsFull(CCircularBuffer_t *cb) {
	if (cb->m_nEnteredCount >= cb->m_nAllocSize)
		return 1;
	return 0;
}

int CCircularBuffer_HasSpace(CCircularBuffer_t *cb, int nCount)
{
	if (cb->m_nEnteredCount + nCount > cb->m_nAllocSize)
		return 0;
	else
		return 1;
}

int	CCircularBuffer_GetSize(CCircularBuffer_t *cb)
{
	return cb->m_nAllocSize;
}

int	CCircularBuffer_GetCount(CCircularBuffer_t *cb)
{
	return cb->m_nEnteredCount;
}

int	CCircularBuffer_Peek(CCircularBuffer_t *cb, unsigned char* pBuffer, int nCount)
{
	int	nBack = cb->m_nBack;
	if (CCircularBuffer_IsEmpty(cb))
		return 0;

	if (nCount > cb->m_nEnteredCount)
		nCount = cb->m_nEnteredCount;

	unsigned char* pBack = &cb->m_pBuffer[cb->m_nBack];
	int nCopySize;
	int bCanCopyDirect;

	if ((cb->m_nAllocSize - cb->m_nBack) >= nCount)
	{
		nCopySize = nCount;
		bCanCopyDirect = 1;
	}
	else
	{
		nCopySize = cb->m_nAllocSize - cb->m_nBack;
		bCanCopyDirect = 0;
	}

	if (pBuffer)
	{
		memcpy(pBuffer, pBack, sizeof(unsigned char) * nCopySize);
	}
	nBack+=nCopySize;

	if (nBack >= cb->m_nAllocSize)
		nBack = 0;

	if (!bCanCopyDirect)
	{
		unsigned char* pTemp = &pBuffer[nCopySize];
		pBack = cb->m_pBuffer;
		nCopySize = nCount - nCopySize;
		if (pBuffer)
		{
			memcpy(pTemp, pBack, sizeof(unsigned char) * nCopySize);
		}
	}

	return nCount;
}

int	CCircularBuffer_Pop(CCircularBuffer_t *cb, unsigned char* pBuffer, int nCount)
{
	if (nCount < 1)
		return 0;

	if (CCircularBuffer_IsEmpty(cb))
		return -1;

	if (nCount > cb->m_nEnteredCount)
		nCount = cb->m_nEnteredCount;

	unsigned char* pBack = &cb->m_pBuffer[cb->m_nBack];
	int nCopySize;
	int bCanCopyDirect;

	if ((cb->m_nAllocSize - cb->m_nBack) >= nCount)
	{
		nCopySize = nCount;
		bCanCopyDirect = 1;
	}
	else
	{
		nCopySize = cb->m_nAllocSize - cb->m_nBack;
		bCanCopyDirect = 0;
	}

	if (pBuffer)
	{
		memcpy(pBuffer, pBack, sizeof(unsigned char) * nCopySize);
	}
	cb->m_nBack+=nCopySize;

	if (cb->m_nBack >= cb->m_nAllocSize)
		cb->m_nBack = 0;

	if (!bCanCopyDirect)
	{
		unsigned char* pTemp = &pBuffer[nCopySize];
		pBack = cb->m_pBuffer;
		nCopySize = nCount - nCopySize;
		if (nCopySize < 1)
			return nCount;

		if (pBuffer)
		{
			memcpy(pTemp, pBack, sizeof(unsigned char) * nCopySize);
		}
		cb->m_nBack += nCopySize;
	}

	cb->m_nEnteredCount -= nCount;
	return nCount;
};

int	CCircularBuffer_Push(CCircularBuffer_t* cb, unsigned char* const pBuffer, int nCount)
{
	if (nCount < 1)
		return 0;

	if (CCircularBuffer_IsFull(cb))
		return -1;

	int nRamnant = cb->m_nAllocSize - cb->m_nEnteredCount;

	if (nRamnant < nCount)
	{
		nCount = nRamnant;
	}

	unsigned char *pFront = &cb->m_pBuffer[cb->m_nFront];
	int nCopySize;
	int bCanCopyDirect;

	if ((cb->m_nAllocSize - cb->m_nFront) >= nCount)
	{
		nCopySize = nCount;
		bCanCopyDirect = 1;
	}
	else
	{
		nCopySize = cb->m_nAllocSize - cb->m_nFront;
		bCanCopyDirect = 0;
	}

	memcpy(pFront, pBuffer, sizeof(unsigned char) * nCopySize);
	cb->m_nFront+=nCopySize;

	if (cb->m_nFront >= cb->m_nAllocSize)
		cb->m_nFront = 0;

	if (!bCanCopyDirect)
	{
		unsigned char* pTemp = &pBuffer[nCopySize];
		pFront = cb->m_pBuffer;
		nCopySize = nCount - nCopySize;
		if (nCopySize < 1)
			return nCount;
		memcpy(pFront, pTemp, sizeof(unsigned char) * nCopySize);
		pFront += nCopySize;
		cb->m_nFront = nCopySize;
	}

	cb->m_nEnteredCount+= nCount;

	return nCount;
}

void CCircularBuffer_Clear(CCircularBuffer_t*cb)
{
	cb->m_nBack = cb->m_nFront = 0;
	cb->m_nEnteredCount = 0;
}
