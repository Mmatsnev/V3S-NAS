// TSDBMultiFileHeader.cpp: implementation of the CTSDBMultiFileHeader class.
//
//////////////////////////////////////////////////////////////////////
// 2002.5.9		�޸� IsFileHead ��������Ӳ��� nBufLen �Է�ֹ m_cbSize ̫�������ڴ�Խ��
//

#include "stdafx.h"
#include "TSDB_Rec.h"
#include "crc.h"
#include <stddef.h>

#ifdef _WIN32
  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif
#endif //_WIN32  

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//	���ļ�ͷ
CTSDBMultiFileHeader::CTSDBMultiFileHeader()
{
	m_pHeader = NULL;
}

CTSDBMultiFileHeader::CTSDBMultiFileHeader(CTSDBMultiFileHeader &RefMulHead)
{
	m_pHeader = RefMulHead.m_pHeader;
}

CTSDBMultiFileHeader::CTSDBMultiFileHeader(PTSDBMULFILEHEAD pRefHead)
{
	ASSERT( pRefHead );
	m_pHeader = pRefHead;
}

CTSDBMultiFileHeader::~CTSDBMultiFileHeader()
{

}

//	�ж��Ƿ�Ϊ���ļ�����ͷ
BOOL CTSDBMultiFileHeader::IsMultiFileHeader()
{
	ASSERT( m_pHeader );
	return IsMultiFileHeader( m_pHeader );
}

//	�ж��Ƿ���ļ�ͷ
//	��ڲ���
//		pBuf						����ͷ������
BOOL CTSDBMultiFileHeader::IsMultiFileHeader(PBYTE pBuf)
{
	ASSERT( pBuf );
	return IsMultiFileHeader( (PTSDBMULFILEHEAD)pBuf );
}

//	�ж��Ƿ���ļ�ͷ
//	��ڲ���
//		pRefHead					����ͷ
BOOL CTSDBMultiFileHeader::IsMultiFileHeader(PTSDBMULFILEHEAD pRefHead)
{
	ASSERT( pRefHead );
	if( !pRefHead )
		return FALSE;
	if( pRefHead->m_CLSID != CLSID_TSDBMULFILEHEAD )		//	GUID ��ͬ
		return FALSE;	
	if( pRefHead->m_cbSize >= MULFILEHEAD_MAXSIZE || pRefHead->m_cbSize < sizeof(TSDBMULFILEHEAD))	
		return FALSE;										//	�ļ�ͷ̫��
	if( CCRC::GetCRC32( pRefHead->m_cbSize - offsetof(TSDBMULFILEHEAD,m_wVersion),\
		(PBYTE)&pRefHead->m_wVersion) != pRefHead->m_dwHeaderCRC32 )
	{
		return FALSE;
	}
	return TRUE;
}


//	ȡ���ļ��еİ������ļ�����
int CTSDBMultiFileHeader::GetFileNum()
{
	ASSERT( m_pHeader && IsMultiFileHeader() );
	return( m_pHeader->m_cFileNum );
}

//	ȡָ����ŵĵ����ļ�ͷ
//	��ڲ���
//		nIndex				�ļ����
//	���ز���
//		�����ļ�ͷ
PTSDBFILEHEADER CTSDBMultiFileHeader::GetFileHeader(int nIndex)
{
	ASSERT( m_pHeader && nIndex < GetFileNum() );
	if( nIndex >= m_pHeader->m_cFileNum )
		return NULL;					//	ʧ��
	return( (PTSDBFILEHEADER)((PBYTE)m_pHeader + GetFileOfs(nIndex)) );
}

//	ȡָ���ļ���ŵ��ļ�ͷ��ƫ��
//	��ڲ���
//		nIndex				�ļ����
//	���ز���
//		�����ļ�ͷ
int CTSDBMultiFileHeader::GetFileOfs(int nIndex)
{
	ASSERT( m_pHeader && nIndex < GetFileNum() );
	return m_pHeader->m_wFileDataOfs[nIndex];
}

//	ȡָ����ŵĵ����ļ�ͷ
//	��ڲ���
//		nIndex				�ļ����
//	���ز���
//		�����ļ�ͷ
TSDBFILEHEADER& CTSDBMultiFileHeader::operator []( int nIndex )
{
	return( *GetFileHeader(nIndex) );
}

CTSDBMultiFileHeader & CTSDBMultiFileHeader::operator=(CTSDBMultiFileHeader& RefMulHeader)
{
	m_pHeader = RefMulHeader.m_pHeader;
	return *this;
}

CTSDBMultiFileHeader & CTSDBMultiFileHeader::operator=(PTSDBMULFILEHEAD pRefMulHead)
{
	ASSERT( pRefMulHead );
	m_pHeader = pRefMulHead;
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// CTSDBFileHeader Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTSDBFileHeader::CTSDBFileHeader()
{
	m_pHeader = NULL;
}

CTSDBFileHeader::~CTSDBFileHeader()
{

}

CTSDBFileHeader::CTSDBFileHeader(CTSDBFileHeader &refHeader)
{
	m_pHeader = refHeader.m_pHeader;
	ASSERT( IsFileHead() );
}

CTSDBFileHeader::CTSDBFileHeader(PTSDBFILEHEADER pRefHeader)
{
	ASSERT( pRefHeader );
	m_pHeader = pRefHeader;
	ASSERT( IsFileHead() );
}

CTSDBFileHeader::CTSDBFileHeader(TSDBFILEHEADER &RefHeader)
{
	m_pHeader = &RefHeader;
	ASSERT( IsFileHead() );
}

//	�ж��Ƿ�Ϊ�ļ�ͷ
BOOL CTSDBFileHeader::IsFileHead()
{
	ASSERT( m_pHeader );
	if( NULL == m_pHeader || m_pHeader->m_cbSize >= 20480 )
		return FALSE;											//	2002.5.9 ��ӣ����� 20K �ֽ�
#ifdef _WIN32
	if( ::IsBadReadPtr( PBYTE(m_pHeader) + m_pHeader->m_cbSize, 1 ) )	//	2002.5.9 ��ӣ����ɶ�
		return FALSE;
#endif//_WIN32        
	return IsFileHead( m_pHeader, m_pHeader->m_cbSize );		//	���ݣ�Ӧ���Ѿ��жϹ�
}

//	�ж��Ƿ�Ϊ�ļ�ͷ
BOOL CTSDBFileHeader::IsFileHead(PBYTE pBuf, int nBufLen )
{
	return IsFileHead( PTSDBFILEHEADER(pBuf), nBufLen );
}

//	�ж��Ƿ�Ϊ�ļ�ͷ
BOOL CTSDBFileHeader::IsFileHead(PTSDBFILEHEADER pHeader, int nBufLen)
{
	ASSERT( pHeader );
	if( pHeader == NULL )
		return FALSE;
	if( pHeader->m_CLSID != CLSID_TSDBFILEHEADER )
		return FALSE;
	if( pHeader->m_cbSize <= sizeof(TSDBFILEHEADER) || pHeader->m_cbSize > nBufLen ) //	2002.5.9 ��� nBufLen �ж�
		return FALSE;
#ifdef _WIN32
	__try
	{
		if( pHeader->m_dwHeaderCRC32 != \
			CCRC::GetCRC32( pHeader->m_cbSize - offsetof(TSDBFILEHEADER,m_wVersion), (PBYTE)&pHeader->m_wVersion ) )
		{
			return FALSE;
		}
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return FALSE;
	}
#else
	if( pHeader->m_dwHeaderCRC32 != \
		CCRC::GetCRC32( pHeader->m_cbSize - offsetof(TSDBFILEHEADER,m_wVersion), (PBYTE)&pHeader->m_wVersion ) )
    {
        return FALSE;
    }
#endif //_WIN32
	return TRUE;
}


//	ȡ�ļ�����
int CTSDBFileHeader::GetFileLen()
{
	ASSERT( m_pHeader );
	return m_pHeader->m_dwFileLen;
}

//	ȡ�ļ���
//	���ļ������� = 0, ���ʾû���ļ���
CString CTSDBFileHeader::GetFileName()
{
	CString	strRetVal;
	ASSERT( m_pHeader );
	int nLen = m_pHeader->m_cbFileNameLenCount;
	if( nLen )
	{							//	���ļ���
		char * pBuf = strRetVal.GetBuffer( nLen+1 );
		memcpy( pBuf, ((PBYTE)m_pHeader) + sizeof(TSDBFILEHEADER), nLen );
		strRetVal.ReleaseBuffer( nLen );
	}
	return strRetVal;
}

//	�ж��Ƿ�Ϊ���ļ�
BOOL CTSDBFileHeader::IsHugeFile()
{
	ASSERT( m_pHeader );
	return m_pHeader->m_bHugeFile;
}

//	�Ƿ����ļ����Բ���
BOOL CTSDBFileHeader::HasFileAttarib()
{
	ASSERT( m_pHeader );
	return m_pHeader->m_bHasAttrib;
}

//	�Ƿ��� Socket ��������
BOOL CTSDBFileHeader::HasSocket()
{
	ASSERT( m_pHeader );
	return m_pHeader->m_bWinSock;
}

//	�Ƿ��� Multicast ��������
BOOL CTSDBFileHeader::HasMulticast()
{
	ASSERT( m_pHeader );
	return m_pHeader->m_bHasMuiticast;
}

//	�Ƿ��и�������
BOOL CTSDBFileHeader::HasExtData()
{
	ASSERT( m_pHeader );
	return ExtDataLen();
}

//	ȡ�������ݳ���
int CTSDBFileHeader::ExtDataLen()
{
	ASSERT( m_pHeader );
	int nLen = GetParamHeader( 32 );
	if( m_pHeader->m_cbSize <= nLen )
		return 0;
	return ( m_pHeader->m_cbSize - nLen );
}

//	������������
//	��ڲ���
//		pDstBuf				���������
//		nBufSize			��������С
//	���ز���
//		ʵ�ʿ��������ݳ���
int CTSDBFileHeader::CopyExtData(PBYTE pDstBuf, int nBufSize)
{
	int nLen = ExtDataLen();
	ASSERT( nLen >= 0 );
	if( nLen >= nBufSize )
		nLen = nBufSize;
	PBYTE pBuf = (PBYTE) m_pHeader;
	pBuf += m_pHeader->m_cbSize - nLen;
	memcpy( pDstBuf, pBuf, nLen );
	return nLen;
}

//	ȡ���ݻ�������ַ
PBYTE CTSDBFileHeader::GetDataBuf()
{
	ASSERT( m_pHeader );
	return ((PBYTE)m_pHeader) + m_pHeader->m_cbSize;
}

//	��������
//	��ڲ���
//		pDstBuf				���������
//		nBufSize			��������С
//	���ز���
//		ʵ�ʿ��������ݳ���
int CTSDBFileHeader::CopyData(PBYTE pDstBuf, int nBufSize)
{
	ASSERT( m_pHeader && pDstBuf );
	if( m_pHeader->m_dwFileLen < (DWORD)nBufSize )
		nBufSize = m_pHeader->m_dwFileLen;
	memcpy( pDstBuf, GetDataBuf(), nBufSize );
	return 0;
}

//	ȡ���ļ�����ͷ
//	���ز���
//		NULL				ʧ��
PTSDBHUGEFILEHEAD CTSDBFileHeader::GetHugeFileHead()
{
	ASSERT( m_pHeader );
	if( m_pHeader->m_bHugeFile == FALSE )
		return NULL;
	int nLen = sizeof( TSDBFILEHEADER ) + m_pHeader->m_cbFileNameLenCount;
	return (PTSDBHUGEFILEHEAD) ( ((PBYTE)m_pHeader) + nLen );
}

//	ȡ�ļ���������ͷ
//	���ز���
//		NULL				ʧ��
PTSDBFILEATTRIBHEAD CTSDBFileHeader::GetFileAttribHead()
{
	ASSERT( m_pHeader );
	if( m_pHeader->m_bHasAttrib == FALSE )
		return FALSE;
	int nLen = GetParamHeader(1);
	return (PTSDBFILEATTRIBHEAD)( ((PBYTE)m_pHeader) + nLen );
}

//	ȡ�ļ� Socket ����ͷ
//	���ز���
//		NULL				ʧ��
PTSDBSOCKETHEAD CTSDBFileHeader::GetSocketHead()
{
	ASSERT( m_pHeader );
	if( m_pHeader->m_bWinSock == FALSE )
		return NULL;
	int nLen = GetParamHeader(2);
	return (PTSDBSOCKETHEAD)(  ((PBYTE)m_pHeader) + nLen );
}

//	ȡ��ֵ��������ͷ
//	���ز���
//		NULL				ʧ��
PTSDBMULTICAST CTSDBFileHeader::GetMulticastHead()
{
	ASSERT( m_pHeader );
	if( m_pHeader->m_bHasMuiticast == FALSE )
		return FALSE;
	int nLen = GetParamHeader(3);
	return (PTSDBMULTICAST)( ((PBYTE)m_pHeader) + nLen );
}

CTSDBFileHeader& CTSDBFileHeader::operator = ( CTSDBFileHeader & RefHead)
{
	m_pHeader = RefHead.m_pHeader;
	return *this;
}

CTSDBFileHeader& CTSDBFileHeader::operator = ( PTSDBFILEHEADER pRefHeader )
{
	ASSERT( pRefHeader );
	m_pHeader = pRefHeader;
	return *this;
}

CTSDBFileHeader& CTSDBFileHeader::operator = ( TSDBFILEHEADER & RefHeader )
{
	m_pHeader = &RefHeader;
	return *this;
}

//	ȡ����������ƫ�Ƶ��ֽ�ƫ��
//	��ڲ���
//		nBitOfs						ƫ��
//	���ز���
//		ƫ���ֽ�
int CTSDBFileHeader::GetParamHeader(int nBitOfs)
{
	int nLen = sizeof( TSDBFILEHEADER ) + m_pHeader->m_cbFileNameLenCount;
	PBYTE pBuf = (PBYTE)m_pHeader;
	pBuf += nLen;
	register DWORD dwFlags = m_pHeader->m_dwFlags;
	for(int i=0; i<nBitOfs; i++)
	{
		if( dwFlags & 1 )
		{
			nLen += * ( (PWORD)pBuf );
			pBuf = nLen + ((PBYTE)m_pHeader);
		}
		dwFlags >>= 1;
	}
	ASSERT( nLen <= m_pHeader->m_cbSize );
	return nLen;
}
