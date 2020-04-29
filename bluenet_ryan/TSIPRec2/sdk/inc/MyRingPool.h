///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2003-1-17
///
///		���λ�����ģ��
///
///=======================================================

#ifndef __MY_RING_POOL_TEMPLATE_20030116__
#define __MY_RING_POOL_TEMPLATE_20030116__

#pragma pack(push,4)

template <class T>
class CMyRingPool
{
public:
	CMyRingPool( int nInitSize = 0);
	~CMyRingPool();

	///-------------------------------------------------------
	/// 2003-1-17
	/// ���ܣ�
	///		��ʼ��������
	/// ��ڲ�����
	///		nInitSize			��ʼ���Ĵ�С
	/// ���ز�����
	///		TRUE				�ɹ�
	///		FALSE				ʧ��
	BOOL Initialize( int nInitSize )
	{
		Invalidate();
		ASSERT( nInitSize );
		if( 0 == nInitSize )
			return FALSE;

		m_pBuf = new T[nInitSize];
		if( NULL == m_pBuf )
			return FALSE;
		m_nTotalUnitCount = nInitSize;
		ASSERT( m_nHeadPtr == 0 );				//	ͷָ��
		ASSERT( m_nTailPtr == 0 );				//	βָ��

		return TRUE;
	}

	///-------------------------------------------------------
	/// 2003-1-17
	/// ���ܣ�
	///		�Ƿ���Ч
	/// ��ڲ�����
	///		��
	/// ���ز�����
	///		TRUE					��Ч
	///		FALSE					��Ч
	BOOL IsValid()
	{
		return NULL != m_pBuf;
	}

	///-------------------------------------------------------
	/// 2003-1-17
	/// ���ܣ�
	///		ʹ֮��Ч
	/// ��ڲ�����
	///		��
	/// ���ز�����
	///		��
	void Invalidate()
	{
		if( m_pBuf )
			delete m_pBuf;
		m_pBuf = NULL;
		m_nTotalUnitCount = 0;		//	�ܵ�Ԫ��
		m_nHeadPtr = 0;				//	ͷָ��
		m_nTailPtr = 0;				//	βָ��
	}

	///-------------------------------------------------------
	/// 2003-1-17
	/// ���ܣ�
	///		��ȡ����
	/// ��ڲ�����
	///		pBuf				������ݵĻ�����
	///		nCount				��������Ԫ�����������ֽ���
	/// ���ز�����
	///		ʵ�ʶ�ȡ�ĵ�Ԫ����
	int Read( T * pBuf, int nCount )
	{
		ASSERT( pBuf && nCount );
		if( NULL == pBuf || 0 == nCount )
			return 0;
		int nUnitCountHasData = AvailableUnit();
		if( 0 == nUnitCountHasData )
			return 0;
		if( nCount > nUnitCountHasData )
			nCount = nUnitCountHasData;
		T * pSrcBuf = m_pBuf + m_nTailPtr;
		if( m_nHeadPtr >= m_nTailPtr )
		{							//	ͷָ�� >= βָ��
			memcpy( pBuf, pSrcBuf, nCount*sizeof(T) );
			m_nTailPtr += nCount;
			ASSERT( m_nTailPtr <= m_nHeadPtr );
			ASSERT( m_nTailPtr < m_nTotalUnitCount );
			return nCount;
		}
		int nCount1 = m_nTotalUnitCount - m_nTailPtr;
		if( nCount1 > nCount )
			nCount1 = nCount;
		memcpy( pBuf, pSrcBuf, nCount1*sizeof(T) );
		m_nTailPtr += nCount1;
		pBuf += nCount1;

		ASSERT( m_nTailPtr <= m_nTotalUnitCount );
		if( m_nTailPtr < m_nTotalUnitCount )
		{
			ASSERT( nCount == nCount1 );
			return nCount;
		}
		nCount -= nCount1;
		if( nCount > m_nHeadPtr )
			nCount = m_nHeadPtr;
		pSrcBuf = m_pBuf;
		if( nCount )
			memcpy( pBuf, pSrcBuf, nCount*sizeof(T) );
		m_nTailPtr = nCount;
		return nCount + nCount1;
	}

	///-------------------------------------------------------
	/// 2003-1-17
	/// ���ܣ�
	///		д������
	/// ��ڲ�����
	///		pBuf				������
	///		nCount				��¼��
	/// ���ز�����
	///		ʵ��д��ļ�¼��
	int Write( T * pBuf, int nCount )
	{
		int nFreeUnitCount = FreeUnitCount();
		if( 0 == nFreeUnitCount )
			return 0;
		if( nCount > nFreeUnitCount )
			nCount = nFreeUnitCount;
		ASSERT( nCount < m_nTotalUnitCount );
		T * pDstBuf = m_pBuf + m_nHeadPtr;
		if( m_nHeadPtr < m_nTailPtr )
		{
			ASSERT( nFreeUnitCount == (m_nTailPtr - m_nHeadPtr - 1) );
			memcpy( pDstBuf, pBuf, nCount*sizeof(T) );
			m_nHeadPtr += nCount;
			ASSERT( m_nHeadPtr < m_nTotalUnitCount );
			ASSERT( m_nHeadPtr < m_nTailPtr );
			ASSERT( m_nTailPtr < m_nTotalUnitCount );
			return nCount;
		}
		ASSERT( nFreeUnitCount == ( m_nTotalUnitCount - m_nHeadPtr + m_nTailPtr - 1 ) );

		int nCount1 = m_nTotalUnitCount - m_nHeadPtr;
		if( 0 == m_nTailPtr )
			nCount1 --;
		if( nCount1 > nCount )
			nCount1 = nCount;
		memcpy( pDstBuf, pBuf, nCount1*sizeof(T) );
		pBuf += nCount1;
		nCount -= nCount1;
		m_nHeadPtr += nCount1;
		if( m_nHeadPtr < m_nTotalUnitCount )
			return nCount1;

		ASSERT( m_nHeadPtr == m_nTotalUnitCount );
		ASSERT( nCount < m_nTotalUnitCount );
		ASSERT( nCount <= FreeUnitCount() );

		pDstBuf = m_pBuf;
		if( nCount )
			memcpy( pDstBuf, pBuf, nCount*sizeof(T) );
		m_nHeadPtr = nCount;

		return nCount1 + nCount;
	}

	///-------------------------------------------------------
	/// CYJ,2005-3-10
	/// ��������:
	///		��ȡ�´�Ҫд�����ݵ�Ԫ
	/// �������:
	///		��
	/// ���ز���:
	///		NULL			ʧ�ܣ�û������
	///		����			���ݵ�Ԫ
	T * GetNextWriteUnit()
	{
		if( FreeUnitCount() == 0 )
			return NULL;
		return m_pBuf + m_nHeadPtr;
	}

	///-------------------------------------------------------
	/// CYJ,2005-3-10
	/// ��������:
	///		����дָ�룬�ú���ͨ���� GetNextWriteUnit ���ʹ��
	/// �������:
	///		��
	/// ���ز���:
	///		��
	void IncreaseWritePtr()
	{
		ASSERT( FreeUnitCount() );
		m_nHeadPtr ++;
		if( m_nHeadPtr >= m_nTotalUnitCount )
			m_nHeadPtr = 0;
	}

	///-------------------------------------------------------
	/// CYJ,2005-3-10
	/// ��������:
	///		��ȡ��ǰ�ɶ������ݵ�Ԫ��ͨ���� Skip ���ʹ��
	/// �������:
	///		��
	/// ���ز���:
	///		NULL				û�����ݿɶ�
	///		����				����
	T * GetCurrentReadUnit()
	{
		if( AvailableUnit() == 0 )
			return NULL;
		return m_pBuf + m_nTailPtr;
	}

	///-------------------------------------------------------
	/// 2003-1-17
	/// ���ܣ�
	///		���� n ����¼
	/// ��ڲ�����
	///		nCount			�����ļ�¼��
	/// ���ز�����
	///		��
	void Skip( int nCount = 1 )
	{
		int nCountInBuf = AvailableUnit();
		ASSERT( nCountInBuf < m_nTotalUnitCount );
		if( nCount > nCountInBuf )
			nCount = nCountInBuf;
		m_nTailPtr += nCount;
		m_nTailPtr %= m_nTotalUnitCount;
	}

	///-------------------------------------------------------
	/// 2003-1-17
	/// ���ܣ�
	///		�������
	/// ��ڲ�����
	///		��
	/// ���ز�����
	///		��
	void Empty()
	{
		m_nHeadPtr = 0;				//	ͷָ��
		m_nTailPtr = 0;				//	βָ��
	}

	///-------------------------------------------------------
	/// 2003-1-17
	/// ���ܣ�
	///		�����ݵĵ�Ԫ����
	/// ��ڲ�����
	///		��
	/// ���ز�����
	///		��
	int AvailableUnit()
	{
		if( m_nHeadPtr >= m_nTailPtr )
			return m_nHeadPtr - m_nTailPtr;
		return m_nHeadPtr + m_nTotalUnitCount - m_nTailPtr;
	}

	///-------------------------------------------------------
	/// 2003-1-17
	/// ���ܣ�
	///		���л���������
	/// ��ڲ�����
	///		��
	/// ���ز�����
	///		���еĻ���������
	///	ע��
	///		����һ����ԪΪ������Ԫ����Ϊ������
	int FreeUnitCount()
	{
		return m_nTotalUnitCount - AvailableUnit() - 1;
	}

#ifdef _DEBUG
	void Dump()
	{
		TRACE("HeadPtr=%d, TailPtr=%d, TotalCount=%d,", m_nHeadPtr, m_nTailPtr, m_nTotalUnitCount );
		TRACE("DataInBuf=%d, FreeUnit=%d\n", AvailableUnit(), FreeUnitCount() );
	}
#endif // _DEBUG

protected:
	T * m_pBuf;					//	������
	int m_nTotalUnitCount;		//	�ܵ�Ԫ��
	int m_nHeadPtr;				//	ͷָ��
	int m_nTailPtr;				//	βָ��
};

template <class T>
CMyRingPool<T>::CMyRingPool( int nInitSize )
{
	m_pBuf = NULL;
	m_nTotalUnitCount = 0;		//	�ܵ�Ԫ��
	m_nHeadPtr = 0;				//	ͷָ��
	m_nTailPtr = 0;				//	βָ��
	if( nInitSize )
		Initialize( nInitSize );
}

template <class T>
CMyRingPool<T>::~CMyRingPool()
{
	Invalidate();
}

#pragma pack(pop)

#endif // __MY_RING_POOL_TEMPLATE_20030116__
