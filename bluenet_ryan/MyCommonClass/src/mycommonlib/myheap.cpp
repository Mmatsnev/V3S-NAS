///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2005-1-10
///
///		��;��
///			���ѡ��ڴ����
///		
///			���ѹ�������ֻ�ܰ�ȡ����һ�εķ��䣬�������ͷ��κη�����ڴ档
///			���������Ŀ����Ϊ�˸����ڴ��еĶ�̬���ݣ����ݽṹ��
///
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӹ�˾���ڲ�ʹ��!				 !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	����Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!		 !
///=======================================================

#include "stdafx.h"
#include "myheap.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyHeap::CMyHeap(int nHeapSize)
{
	m_bIsAttached = false;
	m_pHeapBuf = NULL;
	m_nHeapSize = 0;
	m_nLastAllocatedSize = 0;
	m_nTotalBytesAllocaed = 0;
	m_nHeapMaxSize = nHeapSize;

	if( nHeapSize )
		CreateHeap( nHeapSize );
}

CMyHeap::CMyHeap(PBYTE pBuf, int nHeapSize)
{
	m_bIsAttached = false;
	m_pHeapBuf = NULL;
	m_nHeapSize = 0;
	m_nLastAllocatedSize = 0;
	m_nTotalBytesAllocaed = 0;
	ASSERT( pBuf && nHeapSize );

	if( pBuf && nHeapSize )
		Attach( pBuf, nHeapSize );
}

CMyHeap::~CMyHeap()
{
	DestroyHeap();
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		Initialize and create heap
/// Input parameter:
///		nHeapSize		heap size
/// Output parameter:
///		true			succ
///		false			failed
bool CMyHeap::CreateHeap(int nHeapSize)
{
	ASSERT( nHeapSize );
	if( 0 == nHeapSize )
		return false;
	DestroyHeap();

	// �ȼ�¼�ѵ����ߴ�
	m_nHeapMaxSize = nHeapSize;

	if( nHeapSize > 8192 )		// ���� 8 KB��ת��Ϊ�Զ�������Ҫ����
		nHeapSize = 8192;

	m_nHeapSize = 0;
	m_pHeapBuf = (PBYTE)malloc( nHeapSize );
	if( NULL == m_pHeapBuf )
		return false;
	m_nHeapSize = nHeapSize;

	Reset();

	return true;	
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		invalidate and free the memory that allocated for heap
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyHeap::DestroyHeap()
{
	if( false == IsValid() )
		return;
	if( m_bIsAttached )
		return;

	free( m_pHeapBuf );

	m_pHeapBuf = NULL;
	m_nHeapSize = 0;
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		attach one allocaed memory to the object
/// Input parameter:
///		pBuf				allocated memory
///		nSize				heap size
/// Output parameter:
///		true				succ
///		false				failed
bool CMyHeap::Attach(PBYTE pBuf, int nSize)
{
	ASSERT( false == IsValid() );
	if( IsValid() )
		return false;

	ASSERT( pBuf && nSize );
	if( NULL == pBuf || 0 == nSize )
		return false;

	m_pHeapBuf = pBuf;
	m_nHeapSize = nSize;
	
	m_bIsAttached = true;		//  CYJ,2005-12-12 ��ӣ���ǰ����

	Reset();

	return true;
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		Detach
/// Input parameter:
///		pnHeapSize			output heap size
/// Output parameter:
///		memory address
PBYTE CMyHeap::Detach(int *pnHeapSize)
{
	if( pnHeapSize )
		*pnHeapSize = m_nHeapSize;

	PBYTE pRetVal = m_pHeapBuf;

	m_pHeapBuf = NULL;
	m_nHeapSize = 0;
	Reset();

	return pRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		reset
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyHeap::Reset()
{
	m_nLastAllocatedSize = 0;
	m_nTotalBytesAllocaed = 0;
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		Allocate memory
/// Input parameter:
///		nSize			size in bytes
/// Output parameter:
///		NULL			failed
///		else			succ, memory address
PBYTE CMyHeap::Allocate(int nSize)
{
	if( nSize + m_nTotalBytesAllocaed > m_nHeapSize )
	{			// ����������
		if( m_bIsAttached || m_nHeapSize >= m_nHeapMaxSize )
			return NULL;			// �����ڷ�����
		int nHeapSize = m_nHeapSize;
		int nBufNeed = nSize + m_nTotalBytesAllocaed;
		while( nHeapSize < nBufNeed )
		{
			nHeapSize += 8192;
			if( nHeapSize > m_nHeapMaxSize )
			{
				nHeapSize = m_nHeapMaxSize;
				break;
			}
		}
		if( nBufNeed > nHeapSize )
			return NULL;			// �����ڷ�����
		m_nHeapSize = 0;
		if( m_pHeapBuf )
			m_pHeapBuf = (PBYTE)realloc( m_pHeapBuf, nHeapSize );
		else
			m_pHeapBuf = (PBYTE)malloc( nHeapSize );
		if( NULL == m_pHeapBuf )
			return NULL;
		m_nHeapSize = nHeapSize;
	}

	m_nLastAllocatedSize = nSize;

	PBYTE pRetVal = m_pHeapBuf + m_nTotalBytesAllocaed;

	m_nTotalBytesAllocaed += nSize;

	return pRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		Cancel last allocate
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyHeap::CancelLastAllocate()
{
	m_nTotalBytesAllocaed -= m_nLastAllocatedSize;
	m_nLastAllocatedSize = 0;

	ASSERT( m_nTotalBytesAllocaed >= 0 && m_nTotalBytesAllocaed <= m_nHeapSize );
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		Is heap object valid
/// Input parameter:
///		None
/// Output parameter:
///		true				��Ч
///		false				��Ч
bool CMyHeap::IsValid()
{
	return m_nHeapSize && m_pHeapBuf;
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// Function:
///		Get memory
/// Input parameter:
///		None
/// Output parameter:
///		��ȡ�Ѿ�������ֽ����ݣ�Ҳ����˵���Ѿ�д����ֽ���
int CMyHeap::GetMemoryAllocated() const
{
	return m_nTotalBytesAllocaed;
}

///-------------------------------------------------------
/// CYJ,2005-1-11
/// ��������:
///		д�����ݣ��ȷ����ڴ棬Ȼ����д������
/// �������:
///		pBuf				������
///		nCount				�ֽڴ�С
/// ���ز���:
///		true				�ɹ�
///		false				ʧ��
bool CMyHeap::Write(void *pBuf, int nCount)
{
	PBYTE pDstBuf = Allocate( nCount );
	if( NULL == pDstBuf )
		return false;
	memcpy( pDstBuf, pBuf, nCount );
	return true;
}
