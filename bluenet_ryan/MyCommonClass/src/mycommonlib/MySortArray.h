///////////////////////////////////////////////////////////////
//
//	MySortArray.H	
//	������,2002.9.16
//
//	֧��һ������Ľṹ����
//
//		
///////////////////////////////////////////////////////////////
#ifndef __MY_SORTARRAY_INCLUDE_20020916__
#define __MY_SORTARRAY_INCLUDE_20020916__

#include <MyArray.h>

#ifdef _DEBUG
	#include <stdio.h>
	#include <stdlib.h>
#endif //_DEBUG

#pragma pack(push,4)

template < class T, int OFS, class SORTTYPE > 
class CMySortArray : public CMyArray<T>
{
public:
	CMySortArray(int nMaxCount = 0)
	{
		m_nMaxCount = nMaxCount;		//	�� 0����������ж�
		m_bIsBufAttached = FALSE;
	}

	~CMySortArray()
	{
		if( m_bIsBufAttached )			//	�ڴ���ͨ�� Attach �ģ����ܱ�ɾ��
			Detach();
	}

	int Add( T& newElement )
	{
		ASSERT( FALSE == m_bIsBufAttached );
		if( m_bIsBufAttached )
			return -1;

		int nNo = Find( GetSortValue(newElement) );
		if( nNo >= 0 )
		{
			CMyArray<T>::ElementAt(nNo) = newElement;		//	����
			return nNo;							//	�Ѿ�����
		}
		if( m_nMaxCount && CMyArray<T>::GetSize() >= m_nMaxCount )
		{
            throw "Not enough memory";
     		return -1;
		}

		nNo = -nNo-1;
		InsertAt( nNo, newElement );
		return nNo;
	}

	int Find( SORTTYPE ValueToFind );

#ifdef _DEBUG
	void Dump();
	void Test();
#endif // _DEBUG

	//-----------------------------------------------
	//	ͨ�� Attach ���ڴ�Ϊ�̶������ܱ�ɾ���������Զ�д
	void	Attach( T * pBuf, int nCount )
	{
		ASSERT( pBuf && nCount );
		if( NULL == pBuf || 0 == nCount )
			return;
		m_bIsBufAttached = TRUE;
		CMyArray<T>::m_pData = pBuf;
		CMyArray<T>::m_nSize = nCount;
		CMyArray<T>::m_nMaxSize = nCount;
	}

	T * Detach()
	{
		ASSERT( m_bIsBufAttached );
		if( FALSE == m_bIsBufAttached )
			return NULL;
		m_bIsBufAttached = FALSE;
		T * pRetVal = CMyArray<T>::m_pData;
		CMyArray<T>::m_pData = NULL;
		CMyArray<T>::m_nSize = 0;
		CMyArray<T>::m_nMaxSize = 0;
		return pRetVal;
	}

protected:
	inline SORTTYPE & GetSortValue( const T * pSrc )
	{
		PBYTE pBuf = (PBYTE)pSrc;
		return * (SORTTYPE*)(pBuf + OFS);
	}

	inline SORTTYPE & GetSortValue( const T & src )
	{
		PBYTE pBuf = (PBYTE)&src;
		return * (SORTTYPE*)(pBuf + OFS);
	}
private:
	int	m_nMaxCount;					//	��������ֵ����ֹ���⵼�����
	BOOL	m_bIsBufAttached;
};

//--------------------------------------------------------
//	����һ�� DWORD �Ƿ����б���
//	��ڲ���
//		ValueToFind			��ֵ
//	���ز���
//		<0					ʧ��
//		>=0					�ɹ�
//	ע��
//		�����󷵻�ʱ��(-nRetVal) - 1 ��ʾӦ�������λ��
template < class T, int OFS, class SORTTYPE > 
int CMySortArray<T,OFS,SORTTYPE>::Find( SORTTYPE ValueToFind )
{
	int nCount = CMyArray<T>::GetSize();
	if( 0 == nCount )
		return -1;
	const T * pBuf = CMyArray<T>::GetData();
	SORTTYPE * pSortField = &GetSortValue( pBuf );
	if( ValueToFind < *pSortField )
		return -1;									//	�ж���С
	else if( ValueToFind == *pSortField )
		return 0;
	
	pSortField = &GetSortValue( pBuf[nCount-1] );	//	�ж����	
	if( ValueToFind > *pSortField )
		return -nCount-1;
	else if( ValueToFind == *pSortField )
		return nCount -1;

	int nMin = 0;
	int nMax = nCount-1;
	for(int i=0; i<nCount-1; i++)
	{
		int nTmp = ( nMin + nMax + 1 ) / 2;
		ASSERT( nTmp >= nMin && nTmp <= nMax );
		pSortField = &GetSortValue( pBuf[nTmp] );
		if( ValueToFind == *pSortField )
			return nTmp;							//	����
		if( nTmp == nMin || nTmp == nMax )
			break;									//	�˳�ѭ��
		if( ValueToFind < *pSortField )
			nMax = nTmp;
		else
			nMin = nTmp;
	}
	return -nMax-1;
}

#ifdef _DEBUG
template < class T, int OFS, class SORTTYPE >
void CMySortArray<T,OFS,SORTTYPE>::Dump()
{
	int nCount = CMyArray<T>::GetSize();
	fprintf( stderr, "nCount = %d\n", nCount );
	for(int i=0; i<nCount; i++)
	{
		SORTTYPE & value = GetSortValue( CMyArray<T>::ElementAt(i) );
		fprintf( stderr, "Sort Field[%d] = %d\n", i,value );
	}
}
template < class T, int OFS, class SORTTYPE >
void CMySortArray<T,OFS,SORTTYPE>::Test()
{
	int nCount = CMyArray<T>::GetSize();
	for(int i=0; i<nCount; i++)
	{
		SORTTYPE & currentValue = GetSortValue( CMyArray<T>::ElementAt(i) );
		for(int j=0; j<i; j++)
		{
			SORTTYPE & Tmp = GetSortValue( CMyArray<T>::ElementAt(j) );
			ASSERT( Tmp < currentValue );
		}
	}	
}

#endif // _DEBUG

#pragma pack(pop)

#endif // __MY_SORTARRAY_INCLUDE_20020916__

