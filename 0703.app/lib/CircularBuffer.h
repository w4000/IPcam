// CirculatBuffer.h: interface for the CirculatBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CIRCULARBUFFER_H_INCLUDED_)
#define AFX_CIRCULARBUFFER_H_INCLUDED_
// C implementation from CCircularBuffer class

typedef struct _st_CCircularBuffer
{
	int	m_nAllocSize;
	int	m_nEnteredCount;
	int	m_nFront;
	int	m_nBack;

	unsigned char*	m_pBuffer;

} CCircularBuffer_t;

int CCircularBuffer_IsEmpty(CCircularBuffer_t *);
int CCircularBuffer_IsFull(CCircularBuffer_t *);
int CCircularBuffer_HasSpace(CCircularBuffer_t *, int nCount);
int	CCircularBuffer_GetSize(CCircularBuffer_t *);
int	CCircularBuffer_GetCount(CCircularBuffer_t *);

int CCircularBuffer_Create(CCircularBuffer_t *, int nAllocSize);
int	CCircularBuffer_Peek(CCircularBuffer_t *, unsigned char* pBuffer, int nCount);
int	CCircularBuffer_Pop(CCircularBuffer_t *, unsigned char* pBuffer, int nCount);
int	CCircularBuffer_Push(CCircularBuffer_t *, unsigned char* const pBuffer, int nCount);
void CCircularBuffer_Clear(CCircularBuffer_t *);
void CCircularBuffer_Delete(CCircularBuffer_t *);

#endif
