///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2002-11-14
///
///=======================================================

// MB_OneFile.cpp: implementation of the CMB_OneFile class.
//
//////////////////////////////////////////////////////////////////////
//	2002.11.14	��� SetMulticastParameter����ӳ�Ա���� m_strMC_DstIP �� m_wMC_Port��
//				������ʾ��֮��صĶಥ����
//  2002.4.30	�޸� CollectDataUseXorChksum��������ҳ��������Χʱ����������

#ifndef _WIN32
#include <stdio.h>
#endif //_WWIN32

#include "stdafx.h"
#include "MB_OneFile.h"
#include "TSDVBBROPROTOCOL.H"
#include <stdio.h>
#include "IPData.h"

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

CMB_OneFile::CMB_OneFile() :
	CBufPacket4C<IBufPacket>( 0, 4096 )			//	�� 4K Ϊ��λ���з���
{
	m_nXorChkSumDataLen = 0;					//	û��У������
#ifdef _WIN32
	RtlZeroMemory( m_adwPageRecFlags, sizeof(m_adwPageRecFlags) );
	RtlZeroMemory( m_adwPageErrFlags, sizeof(m_adwPageErrFlags) );
#else
	bzero( m_adwPageRecFlags, sizeof(m_adwPageRecFlags) );
	bzero( m_adwPageErrFlags, sizeof(m_adwPageErrFlags) );
#endif //_WIN32
	m_wMC_Port = 0;						//	2002.11.14 ��ӣ��ಥ�˿�
}

CMB_OneFile::~CMB_OneFile()
{
}

void CMB_OneFile::SafeDelete()
{
	delete this;
}

//	��ʼ��
//	��ڲ���
//		chFile				ͨ����
//		lpszFileName		�ļ���
//		dwLen				����
//		FileTime			�ļ�ʱ��
//	���ز���
//		TRUE				�ɹ�
//		FALSE				ʧ��
BOOL CMB_OneFile::Initialize(TSDBCHANNEL chFile, LPCSTR lpszFileName, DWORD dwLen, time_t FileTime)
{
	ASSERT( lpszFileName && dwLen );
	ASSERT( dwLen > 0 && dwLen < 1024*512 );		//	< 512K

	m_dwFileLen = 0;
	m_dwByteRead = 0;
	m_nXorChkSumDataLen = 0;

#ifdef _WIN32
	RtlZeroMemory( m_adwPageRecFlags, sizeof(m_adwPageRecFlags) );
	RtlZeroMemory( m_adwPageErrFlags, sizeof(m_adwPageErrFlags) );
#else
	bzero( m_adwPageRecFlags, sizeof(m_adwPageRecFlags) );
	bzero( m_adwPageErrFlags, sizeof(m_adwPageErrFlags) );
#endif //_WIN32
	if( FALSE == SetBufSize( dwLen ) )
		return FALSE;
	PutDataLen( dwLen );

	m_dwFileLen = dwLen;
	m_Time = FileTime;	

	m_chFile.m_dwData = chFile.m_dwData;
	if( lpszFileName[0] )
		strncpy( m_szFileName, lpszFileName, 13 );
	else
		m_szFileName[0] = 0;

	m_strMC_DstIP = "";					//	2002.11.14 ��ӣ��ಥIP��ַ
	m_wMC_Port = 0;						//	2002.11.14 ��ӣ��ಥ�˿�

	return TRUE;
}

//	�ж��ļ��Ƿ�ı�
BOOL CMB_OneFile::IsFileChanged(time_t t,DWORD dwFileLen)
{
	ASSERT( GetBuffer() );
	if( dwFileLen != m_dwFileLen || m_Time != t )
		return TRUE;
	return FALSE;
}

//	���һҳ
//	��ڲ���
//		pBuf				��������ַ���μ� PTSDVBMULTICASTPACKET0
//		dwLen				����������
//	���ز���
//		MBROF_DATA_ERR		�����д���
//		MBROF_FILE_CHANGED	�ļ��ı�
//		MBROF_DATAOK_FILENOTOK	�ɹ������ļ�û������
//		MBROF_FILE_OK		�����ļ����� OK
int CMB_OneFile::AddOnePage(PBYTE pBuf, DWORD dwLen)
{
	ASSERT( pBuf && dwLen && GetBuffer() );
	PTSDVBMULTICASTPACKET0 pHeader = (PTSDVBMULTICASTPACKET0)pBuf;
#ifdef _DEBUG
	ASSERT( pHeader->m_cbSize < dwLen );
	if( CCRC::GetCRC32(pHeader->m_cbSize-offsetof(TSDVBMULTICASTPACKET0,m_PacketTime),\
		(PBYTE)&pHeader->m_PacketTime ) != pHeader->m_dwHeaderCRC32 )
	{
		ASSERT( FALSE );
		return MBROF_DATA_ERR;
	}
	if( IsFileChanged( pHeader->m_PacketTime, pHeader->m_dwFileLen ) )
	{
		ASSERT( FALSE );
		return MBROF_FILE_CHANGED;
	}
#endif // _DEBUG	

	ASSERT( pHeader->m_dwFileLen == m_dwFileLen );

	PBYTE pPageBuf = pBuf + pHeader->m_cbSize;
	BOOL bIsPageOK = CCRC::GetCRC32( pHeader->m_wPageLen, pPageBuf ) == pHeader->m_dwPageCRC32;
	
	DWORD dwOfsInFile = pHeader->GetPageOfsInFile();
	if( dwOfsInFile >= pHeader->GetFileLen() )
	{											//	XOR ����У��
		ASSERT( 0 == m_nXorChkSumDataLen );
		m_nXorChkSumDataLen = pHeader->GetPageLen();
		if( m_nXorChkSumDataLen > XORCHKSUMBUFLEN )
			m_nXorChkSumDataLen = 0;			//	̫�󣬷���
		else
			memcpy( m_abyXorChkSum, pPageBuf, m_nXorChkSumDataLen );
		ASSERT( 0 == m_dwByteRead );			//	һ���һҳ��У������, ????
		return MBROF_DATAOK_FILENOTOK;
	}

	memcpy( GetBuffer() + dwOfsInFile, pPageBuf, pHeader->m_wPageLen );
	if( bIsPageOK )
		m_dwByteRead += pHeader->m_wPageLen;
#ifdef _DEBUG
	else
		TRACE("Receive one page error \n");
#endif // _DEBUG
	ASSERT( m_dwByteRead <= m_dwFileLen );

	m_dwResultFlags.m_bIsReceived = TRUE;
	m_dwResultFlags.m_bIsFileErr |= bIsPageOK;
	WORD wPageLen = m_nXorChkSumDataLen;		//	����0����һ����һҳ�ĳ���
	if( 0 == wPageLen )
		wPageLen = pHeader->GetPageLen();
	ASSERT( wPageLen );
	SetPageReceived( dwOfsInFile/wPageLen, bIsPageOK );
	
	if( m_dwByteRead >= m_dwFileLen )
		return MBROF_FILE_OK;
	else
		return MBROF_DATAOK_FILENOTOK;			//	��û�����ȫ������
}

void CMB_OneFile::SetPageReceived(int nPageNo, BOOL bIsErr)
{
	if( nPageNo >= PRS_MAX_PAGENUM )
		return;
	int nOfs = nPageNo >> 5;				//	32
	int dwMask = 1L << ( nPageNo&0x1f );	//	%32
	BOOL bRetVal = m_adwPageRecFlags[nOfs] & dwMask;		//	��ȡԭ����״̬

	m_adwPageRecFlags[nOfs] &= ~dwMask;						//	���״̬
	m_adwPageErrFlags[nOfs] &= ~dwMask;

	m_adwPageRecFlags[nOfs] |= dwMask;						//	���ý��ձ��
	if( bIsErr )
		m_adwPageErrFlags[nOfs] |= dwMask;					//	���ô����־
}

//	2001.10.12	���
//	�� XOR У�����
//	���ز���
//		TRUE				�ɹ�����
//		FALSE				û�о���
BOOL CMB_OneFile::CollectDataUseXorChksum()
{
	ASSERT( m_dwByteRead < m_dwFileLen && GetBuffer() );
	TRACE("One File is changed, %d/%d=   ",m_dwByteRead,m_dwFileLen  );
	if( (m_dwFileLen-m_dwByteRead) > (DWORD)m_nXorChkSumDataLen )	//	̫����󣬻�û��У���
	{
		TRACE("Too many data lost %d/%d, can not collect ????\n",m_dwFileLen-m_dwByteRead, m_nXorChkSumDataLen );
		return FALSE;	
	}
	TRACE("Collect succ. +++++++++\n");

	ASSERT( m_nXorChkSumDataLen );							//	������һҳ�Ĵ�С
	int nTotalPacket = (m_dwFileLen+m_nXorChkSumDataLen-1) / m_nXorChkSumDataLen;
	if( nTotalPacket > PRS_MAX_PAGENUM )
		nTotalPacket = PRS_MAX_PAGENUM;						//	����¼��ҳ��
	int nPTmp = (nTotalPacket+7)/8;
	PBYTE pRecFlag = (PBYTE)m_adwPageRecFlags;				//	�жϴ����λ��
	PBYTE pbyDstBuf = NULL;
	int nErrPacketNo;
    int i;
	for(i=0; i<nPTmp; i++, pRecFlag++ )
	{
		BYTE byFlag = *pRecFlag;
		if( 0xFF == byFlag )					
			continue;				//	��ȷ
		BYTE byMask = 1;
        int j;
		for(j=0; j<8; j++)
		{
			if( 0 == (byMask&byFlag) )
				break;
			byMask <<= 1;
		}
		ASSERT( j < 8 );
		nErrPacketNo = i*8+j;
		if( nErrPacketNo >= nTotalPacket )
			break;							//	2002.4.30��ӣ���������Χ
		ASSERT( nErrPacketNo*m_nXorChkSumDataLen < (int)m_dwFileLen );
		pbyDstBuf = GetBuffer() + nErrPacketNo*m_nXorChkSumDataLen;
		break;
	}
	if( NULL == pbyDstBuf )
		return FALSE;	

	for(i=0; i<nTotalPacket; i++)
	{
		if( i == nErrPacketNo )				//	�Ѿ����������������
			continue;
		PDWORD pdwSrc = (PDWORD)( GetBuffer() + i*m_nXorChkSumDataLen );
		PDWORD pdwDst = (PDWORD)m_abyXorChkSum;
		int nByteToXor;
		if( i==(nTotalPacket-1) ) 
		{
			nByteToXor = m_dwFileLen%m_nXorChkSumDataLen;
			if( 0 == nByteToXor )
				nByteToXor = m_nXorChkSumDataLen;
		}
		else
			nByteToXor = m_nXorChkSumDataLen;

		int nXorCount = nByteToXor >> 2;
        int j;
		for(j=0; j<nXorCount; j++)
		{
			*pdwDst ^= *pdwSrc;
			pdwDst ++;
			pdwSrc ++;
		}
		nXorCount = nByteToXor & 3;
		PBYTE pbySrc = (PBYTE)pdwSrc;
		PBYTE pbyDst = (PBYTE)pdwDst;
		for(j=0; j<nXorCount; j++)
		{
			*pbyDst ^= *pbySrc;
			pbyDst ++;
			pbySrc ++;
		}
	}

	int nByteToCopy = (nErrPacketNo==(nTotalPacket-1)) ? m_dwFileLen%m_nXorChkSumDataLen : m_nXorChkSumDataLen;
	if( 0 == nByteToCopy )
		nByteToCopy = m_nXorChkSumDataLen;
	memcpy( pbyDstBuf, m_abyXorChkSum, nByteToCopy );

	m_dwByteRead += nByteToCopy;	
	ASSERT( m_dwByteRead == m_dwFileLen );
	m_dwResultFlags.m_bIsReceived = TRUE;
	m_dwResultFlags.m_bIsFileErr = FALSE;			//	�Ѿ�������
	SetPageReceived( nErrPacketNo, TRUE );
	return TRUE;
}

///-------------------------------------------------------
/// 2002-11-14
/// ���ܣ�
///		������֮��ص�IP��ַ�Ͷ˿�
/// ��ڲ�����
///		lpszIP				lpszIP ��ַ
///		wPort				�˿�
///		pDataPortItem		data port item
/// ���ز�����
///		��
void CMB_OneFile::SetMulticastParameter(LPCSTR lpszIP, WORD wPort, COneDataPortItem * pDataPortItem )
{
	ASSERT( lpszIP && wPort && pDataPortItem );
	if( lpszIP )
		m_strMC_DstIP = lpszIP;
	else
		m_strMC_DstIP = "";
	m_wMC_Port = wPort;
	m_pDataPortItem = pDataPortItem;
}
