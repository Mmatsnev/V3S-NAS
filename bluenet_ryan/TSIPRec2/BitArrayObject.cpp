///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2003-8-7
///
///		��;��
///			����ͳ��
///=======================================================

// BitArrayObject.cpp: implementation of the CBitArrayObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BitArrayObject.h"

#ifdef _WIN32
  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif
#endif //_WIN32

// ���ض�Ӧ�� 1 �ĸ���
static BYTE g_BIT_1_COUNT_TBL[16]={ 0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4 };


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBitArrayObject::CBitArrayObject()
{
	m_BitmapArray.SetSize( 0, 4096 );		//	ÿ�η���ͷ���4096�ֽ�
	m_nTotalBitCount = 0;
	m_nBit_1_Count = 0;
	m_nNextFileIDPtr = 0;
}

CBitArrayObject::~CBitArrayObject()
{

}


///-------------------------------------------------------
/// 2003-4-4
/// ���ܣ�
///		��ȡ�Ѿ��ɹ����յ����ļ�����
/// ��ڲ�����
///		pVal				�������
/// ���ز�����
///
long CBitArrayObject::GetSubFileHasReceived()
{
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
long CBitArrayObject::GetTotalSubFileCount()
{
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
BOOL CBitArrayObject::IsSubFileOK(long nIndex)
{
	int nByteOfs = nIndex / 8;
	int nBitOfs = nIndex % 8;

	ASSERT( nByteOfs < m_BitmapArray.GetSize() );
	if( nByteOfs >= m_BitmapArray.GetSize() )
		return FALSE;

	int nRetVal = 0;
	BYTE byMask = 1 << nBitOfs;
	if( m_BitmapArray[nByteOfs] & byMask )
		return TRUE;
	else
		return FALSE;
}

///-------------------------------------------------------
/// 2003-4-4
/// ���ܣ�
///		��ȡDataBuffer for VC Only
/// ��ڲ�����
///
/// ���ز�����
///
PBYTE CBitArrayObject::GetDataBuffer()
{
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
BOOL CBitArrayObject::SetTotalSubFileCount(long nNewValue)
{
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
		return FALSE;
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
/// ���ز�����
///		�޸ĺ���Ѿ��ɹ����յ����ļ�����
long CBitArrayObject::SetBitValue(int nIndex, int nBitValue )
{
	if( nIndex >= m_nTotalBitCount )
	{								//	̫���ˣ����·����ڴ�
		if( FALSE == SetTotalSubFileCount( nIndex+1 ) )
			return 0;
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
void CBitArrayObject::Prepare()
{
	m_nNextFileIDPtr = 0;
}

///-------------------------------------------------------
/// 2003-4-4
/// ���ܣ�
///		��ȡ�¸�������������ֵ
/// ��ڲ�����
///		nBitValue			������ֵ
/// ���ز�����
///		����������. -1 ��ʾ����
long CBitArrayObject::GetNextFileID(int nBitValue )
{
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
///		��
/// ���ز�����
///		����Ѿ��ɹ����յ��ļ�����
long CBitArrayObject::ReStat()
{
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
/// ���ز�����
///		����Ѿ��ɹ����յĸ���
long CBitArrayObject::Combine(CBitArrayObject * pSrcObj)
{
	long nRetVal = 0;
	PBYTE pSrcBuf = NULL;
	long nSrcCount = 0;
	TRY
	{
		pSrcBuf = pSrcObj->GetDataBuffer();
		if( NULL == pSrcBuf )
			return 0;
		nSrcCount = pSrcObj->GetTotalSubFileCount();
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
		nRetVal = ReStat();
	}
	CATCH_ALL( e )
	{
		return 0;
	}
	END_CATCH_ALL

	return nRetVal;
}

///-------------------------------------------------------
/// 2003-4-4
/// ���ܣ�
///		���ļ���ȡ����
/// ��ڲ�����
///		bstrFileName		�ļ���
/// ���ز�����
///		�����Ѿ��ɹ���ȡ�����ļ�����
long CBitArrayObject::LoadFromFile( LPCSTR lpszFileName )
{
	long nRetVal = 0;
	m_nTotalBitCount = 0;
	m_nBit_1_Count = 0;
	m_BitmapArray.RemoveAll();

	CString strFileName = lpszFileName;
	CFile f;
	if( FALSE == f.Open( strFileName, CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite ) )
		return 0;	

	TRY
	{
		int nLen = f.GetLength();
		if( nLen < 4 )					//	�ļ����Ȳ���
			return 0;

		if( sizeof(int) != f.Read( &m_nTotalBitCount, sizeof(int) ) || m_nTotalBitCount <= 0 )
		{
			m_nTotalBitCount = 0;
			return 0;				//	����
		}

		int nByteCount = (m_nTotalBitCount + 7 )/8;
		m_BitmapArray.SetSize( nByteCount, 4096 );
		if( nByteCount != (int) f.Read( m_BitmapArray.GetData(), nByteCount ) )
		{
			m_nTotalBitCount = 0;
			m_BitmapArray.RemoveAll();
			return 0;
		}

		nRetVal = ReStat();						//	����ͳ��
		ASSERT( nRetVal == m_nBit_1_Count );

		TRY
		{
			int nCount = 0;
			if( f.Read( &nCount, sizeof(int) ) == sizeof(int) )
			{
				m_adwUserDefData.SetSize( nCount );
				if( nCount )
				{
					f.Read( m_adwUserDefData.GetData(), nCount*sizeof(DWORD) );
				}
			}
		}
		CATCH( CFileException, e )
		{
			m_adwUserDefData.RemoveAll();
		}
		END_CATCH

		f.Close();
	}
	CATCH_ALL( e )
	{
		m_nTotalBitCount = 0;
		m_BitmapArray.RemoveAll();
		return 0;
	}
	END_CATCH_ALL

	return nRetVal;
}

///-------------------------------------------------------
/// 2003-4-4
/// ���ܣ�
///		
/// ��ڲ�����
///
/// ���ز�����
///
BOOL CBitArrayObject::SaveToFile( LPCSTR lpszFileName )
{
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

		int nCount = m_adwUserDefData.GetSize();		//  2003-8-8 �����û��Զ�������
		f.Write( &nCount, sizeof(int) );
		f.Write( m_adwUserDefData.GetData(), nCount*sizeof(DWORD) );

		f.Close();
	}
	CATCH_ALL( e )
	{
		return FALSE;
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
CBitArrayObject & CBitArrayObject::operator =( CBitArrayObject & src )
{
	m_BitmapArray.RemoveAll();
	m_BitmapArray.Copy( src.m_BitmapArray );
	m_nBit_1_Count = src.m_nBit_1_Count;
	m_nTotalBitCount = src.m_nTotalBitCount;

	return *this;
}

///-------------------------------------------------------
/// CYJ,2003-8-7
/// Function:
///		Preset all variant
/// Input parameter:
///		None
/// Output parameter:
///		None
void CBitArrayObject::Reset()
{
	m_BitmapArray.SetSize( 0, 4096 );		//	ÿ�η���ͷ���4096�ֽ�	
	m_nTotalBitCount = 0;
	m_nBit_1_Count = 0;
	m_nNextFileIDPtr = 0;
	m_adwUserDefData.RemoveAll();			//	��� 0
}

///-------------------------------------------------------
/// CYJ,2003-8-8
/// Function:
///		Get User define data array
/// Input parameter:
///		None
/// Output parameter:
///		None
CDWordArray & CBitArrayObject::GetUserDefData()
{
	return m_adwUserDefData;
}

///-------------------------------------------------------
/// CYJ,2003-8-8
/// Function:
///		Clean all data to zero
/// Input parameter:
///		None
/// Output parameter:
///		None
void CBitArrayObject::CleanDataOnly()
{
	m_nBit_1_Count = 0;
	PBYTE pBuf = m_BitmapArray.GetData();
	if( pBuf )
		memset( pBuf, 0, m_BitmapArray.GetSize() );	
}
