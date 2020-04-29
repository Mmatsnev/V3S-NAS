///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2003-4-4
///
///		��;��
///			λͼ����������
///=======================================================

// IPFileMendHelper.cpp : Implementation of CIPFileMendHelper
#include "stdafx.h"
#include "IPRecSvr.h"
#include "IPFileMendHelper.h"

// ���ض�Ӧ�� 1 �ĸ���
static BYTE g_BIT_1_COUNT_TBL[16]={ 0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4 };

/////////////////////////////////////////////////////////////////////////////
// CIPFileMendHelper

///-------------------------------------------------------
/// 2003-4-4
/// ���ܣ�
///		��ȡ�Ѿ��ɹ����յ����ļ�����
/// ��ڲ�����
///		pVal				�������
/// ���ز�����
///
long CIPFileMendHelper::GetSubFileHasReceived()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_nBit_1_Count;
}

///-------------------------------------------------------
/// 2003-4-4
/// ���ܣ�
///		��ȡ���ļ�����
/// ��ڲ�����
///
/// ���ز�����
///
long CIPFileMendHelper::GetTotalSubFileCount()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_nTotalBitCount;
}

///-------------------------------------------------------
/// 2003-4-4
/// ���ܣ�
///		�ж�һ�����ļ��Ƿ�ɹ����գ��� 1 �� �ɹ����գ�0 �� ʧ�ܻ�δ������
/// ��ڲ�����
///
/// ���ز�����
///
BOOL CIPFileMendHelper::GetIsSubFileOK( long nIndex )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	int nByteOfs = nIndex / 8;
	int nBitOfs = nIndex % 8;

	ASSERT( nByteOfs < m_BitmapArray.GetSize() );
	if( nByteOfs >= m_BitmapArray.GetSize() )
		return FALSE;

	BYTE byMask = 1 << nBitOfs;
	return ( m_BitmapArray[nByteOfs] & byMask );
}

///-------------------------------------------------------
/// 2003-4-4
/// ���ܣ�
///		��ȡDataBuffer for VC Only
/// ��ڲ�����
///
/// ���ز�����
///
PBYTE CIPFileMendHelper::GetDataBufferVC()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_BitmapArray.GetData();
}

///-------------------------------------------------------
/// 2003-4-4
/// ���ܣ�
///		�������ļ�����
/// ��ڲ�����
///		nNewValue			�µ��ļ�����
/// ���ز�����
///
BOOL CIPFileMendHelper::SetTotalSubFileCount(long nNewValue)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	m_nTotalBitCount = nNewValue;
	nNewValue = ( nNewValue + 7 ) / 8;
	TRY
	{
		if( nNewValue != m_BitmapArray.GetSize() )
			m_BitmapArray.SetSize( nNewValue, 4096 );		//	���� 4096 �ֽ�
	}
	CATCH_ALL( e )
	{
		m_BitmapArray.RemoveAll();
		m_nTotalBitCount = 0;
		return  FALSE;
	}
	END_CATCH_ALL

	return TRUE;
}

///-------------------------------------------------------
/// 2003-4-4
/// ���ܣ�
///		����һ����ֵ
/// ��ڲ�����
///		nIndex					�������
///		nBitValue				�µ�ֵ
///		pRetVal					�޸ĺ���Ѿ��ɹ����յ����ļ�����
/// ���ز�����
///		<0						�ǲ�
long CIPFileMendHelper::SetBitValue( int nIndex,int nBitValue )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( nIndex >= m_nTotalBitCount )
	{								//	̫���ˣ����·����ڴ�
		if( FALSE == SetTotalSubFileCount( nIndex+1 ) )
			return -1;
	}

	int nByteOfs = nIndex / 8;
	int nBitOfs = nIndex % 8;
	ASSERT( nByteOfs < m_BitmapArray.GetSize() );
	BYTE byMask = ( 1 << nBitOfs );

	BYTE & byValue = m_BitmapArray[nByteOfs];

	if( nBitValue )
	{
		if( 0 == (byValue&byMask) )
			m_nBit_1_Count ++;			//	�����ӵ�����
		ASSERT( m_nBit_1_Count <= m_nTotalBitCount );
		byValue |= byMask;
	}
	else
	{
		if( byValue&byMask )
			m_nBit_1_Count --;			//	�����ӵ�����
		ASSERT( m_nBit_1_Count >= 0 );
		byValue &= (~byMask);
	}

	return m_nBit_1_Count;
}

///-------------------------------------------------------
/// 2003-4-4
/// ���ܣ�
///		Ϊ GetNextFile ��׼��
/// ��ڲ�����
///
/// ���ز�����
///
void CIPFileMendHelper::Prepare()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	m_nNextFileIDPtr = 0;
}

///-------------------------------------------------------
/// 2003-4-4
/// ���ܣ�
///		��ȡ�¸�������������ֵ
/// ��ڲ�����
///		nBitValue			������ֵ
///		pRetVal				����������
/// ���ز�����
///
long CIPFileMendHelper::GetNextFileID( int nBitValue )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	while( m_nNextFileIDPtr < m_nTotalBitCount )
	{
		int nByteOfs = m_nNextFileIDPtr / 8;
		int nBitOfs = m_nNextFileIDPtr % 8;
		m_nNextFileIDPtr ++;

		BYTE byMask = (1 << nBitOfs );
		BYTE byResult = m_BitmapArray[nByteOfs] & byMask;
		if( (nBitValue&&byResult) || (0==nBitValue && 0 ==byResult) )
			return m_nNextFileIDPtr-1;
	}

	return -1;
}

///-------------------------------------------------------
/// 2003-4-4
/// ���ܣ�
///		����ͳ���Ѿ��ɹ����յ����ļ�����
/// ��ڲ�����
///		pRetVal				����Ѿ��ɹ����յ��ļ�����
/// ���ز�����
///
long CIPFileMendHelper::ReStat()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	m_nBit_1_Count = 0;

	int nCount = m_BitmapArray.GetSize();
	PBYTE pBuf = m_BitmapArray.GetData();
	for(int i=0; i<nCount; i++)
	{
		m_nBit_1_Count += g_BIT_1_COUNT_TBL[ (*pBuf) & 0xF ];
		m_nBit_1_Count += g_BIT_1_COUNT_TBL[ (*pBuf) >> 4 ];
		pBuf ++;
	}

	return m_nBit_1_Count;
}

///-------------------------------------------------------
/// 2003-4-4
/// ���ܣ�
///		�ϲ�
/// ��ڲ�����
///		pSrcObj				�ϲ�����λͼ������ OR ����
///		pRetVal				����Ѿ��ɹ����յĸ���
/// ���ز�����
///		<0					failed
long CIPFileMendHelper::Combine( IIPFileMendHelper *pSrcObj )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	PBYTE pSrcBuf = NULL;
	long nSrcCount = 0;
	TRY
	{
		pSrcBuf = pSrcObj->GetDataBufferVC();
		if( NULL == pSrcBuf )
			return -1;
		nSrcCount = pSrcObj->GetTotalSubFileCount();
		if( nSrcCount < 0 )
			return -1;
		int nCount = m_nTotalBitCount;
		if( nCount > nSrcCount )
			nCount = nSrcCount;
		int nByteCount = (nCount+7) / 8;
		PBYTE pDstBuf = m_BitmapArray.GetData();
		for(int i=0; i<nByteCount; i++)
		{
			*pDstBuf |= *pSrcBuf;
			pDstBuf ++;
			pSrcBuf ++;
		}
		return ReStat( );
	}
	CATCH_ALL( e )
	{
		return -1;
	}
	END_CATCH_ALL

	return -1;
}

///-------------------------------------------------------
/// 2003-4-4
/// ���ܣ�
///		���ļ���ȡ����
/// ��ڲ�����
///		bstrFileName		�ļ���
///		pRetVal				�����Ѿ��ɹ���ȡ�����ļ�����
/// ���ز�����
///		<0					failed
BOOL CIPFileMendHelper::LoadFromFile( LPCSTR lpszFileName )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	m_nTotalBitCount = 0;
	m_nBit_1_Count = 0;
	m_BitmapArray.RemoveAll();

	CString strFileName = lpszFileName;
	CFile f;
	if( FALSE == f.Open( strFileName, CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite ) )
		return FALSE;

	TRY
	{
		int nLen = f.GetLength();
		if( nLen < 4 )					//	�ļ����Ȳ���
			return FALSE;

		if( sizeof(int) != f.Read( &m_nTotalBitCount, sizeof(int) ) || m_nTotalBitCount <= 0 )
		{
			m_nTotalBitCount = 0;
			return FALSE;				//	����
		}

		int nByteCount = (m_nTotalBitCount + 7 )/8;
		m_BitmapArray.SetSize( nByteCount, 4096 );
		if( nByteCount != (int) f.Read( m_BitmapArray.GetData(), nByteCount ) )
		{
			m_nTotalBitCount = 0;
			m_BitmapArray.RemoveAll();
			return FALSE;
		}

		long nTotalCount = ReStat();						//	����ͳ��
		ASSERT( nTotalCount == m_nBit_1_Count );
		return TRUE;
	}
	CATCH_ALL( e )
	{
		m_nTotalBitCount = 0;
		m_BitmapArray.RemoveAll();
	}
	END_CATCH_ALL

	return FALSE;
}

///-------------------------------------------------------
/// 2003-4-4
/// ���ܣ�
///
/// ��ڲ�����
///
/// ���ز�����
///
BOOL CIPFileMendHelper::SaveToFile( LPCSTR lpszFileName )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	CString strFileName = lpszFileName;
	CFile f;
	if( FALSE == f.Open( strFileName, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary|CFile::shareDenyWrite ) )
		return FALSE;

	TRY
	{
		f.Write( &m_nTotalBitCount, sizeof(int) );
		if( m_nTotalBitCount )
		{
			ASSERT( m_BitmapArray.GetSize() == ((m_nTotalBitCount+7)/8) );
			f.Write( m_BitmapArray.GetData(), m_BitmapArray.GetSize() );
		}
		f.Close();
	}
	CATCH_ALL( e )
	{
	}
	END_CATCH_ALL

	return TRUE;
}

///-------------------------------------------------------
/// 2003-4-5
/// ���ܣ�
///		����һ���µĶ���
/// ��ڲ�����
///
/// ���ز�����
///
IIPFileMendHelper * CIPFileMendHelper::Clone()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	CIPFileMendHelper * pInstance = new CIPFileMendHelper;
	if( NULL == pInstance )
		return NULL;
	pInstance->AddRef();

	pInstance->m_BitmapArray.RemoveAll();
	pInstance->m_BitmapArray.Copy( m_BitmapArray );
	pInstance->m_nBit_1_Count = m_nBit_1_Count;
	pInstance->m_nTotalBitCount = m_nTotalBitCount;

	return static_cast<IIPFileMendHelper*>(pInstance);

}
