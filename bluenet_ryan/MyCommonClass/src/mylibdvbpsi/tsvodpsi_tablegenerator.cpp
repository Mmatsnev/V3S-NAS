// TSVODPSI_TableGenerator.cpp: implementation of the CTSVODPSI_TableGenerator class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tsvodpsi_tablegenerator.h"
#ifdef _WIN32
  #include <zlib/zlib.h>
#else
  #include <zlib.h>
#endif //_WIN32
#include "bitstream.h"
#include "dvb_crc.h"
#include "dvbpsitablesdefine.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTSVODPSI_TableGeneratorBase::CTSVODPSI_TableGeneratorBase(BYTE byTableID, bool bDoCompress )
{
	m_byBuildCounter = 0;
	m_bDoCompressed = bDoCompress;
	m_byTableID = byTableID;
	m_pbyOutBuf = NULL;
	Preset();
}

CTSVODPSI_TableGeneratorBase::~CTSVODPSI_TableGeneratorBase()
{
	if( m_pbyOutBuf )
		delete m_pbyOutBuf;
}

///-------------------------------------------------------
/// CYJ,2005-4-19
/// ��������:
///		Ԥ��
/// �������:
///		��
/// ���ز���:
///		��
void CTSVODPSI_TableGeneratorBase::Preset()
{
	if( m_pbyOutBuf )
		delete m_pbyOutBuf;
	m_pbyOutBuf = NULL;
	m_nOutBufLen = 0;
	m_dwOutBufSize = 0;
	m_bIsModified = false;
	SetPID( 0x20 );
}

///-------------------------------------------------------
/// CYJ,2005-4-19
/// ��������:
///		�Ƿ��޸�
/// �������:
///		bModified			�µı�־
/// ���ز���:
///		ԭ���ı�־
bool CTSVODPSI_TableGeneratorBase::SetModifyFlag( bool bModified )
{
	bool bRetVal = m_bIsModified;
	m_bIsModified = bModified;
	return bRetVal;
}


///-------------------------------------------------------
/// CYJ,2005-4-19
/// ��������:
///		�������
/// �������:
///		��
/// ���ز���:
///		��
bool CTSVODPSI_TableGeneratorBase::Build()
{
	if( NULL == m_pbyOutBuf || m_bIsModified )
	{	
		SetModifyFlag(false);			//  CYJ,2005-12-20 ���ܻᵼ�±����������
		m_byBuildCounter ++;

		int nPriDataLen = 0;
		PBYTE pbyPriData = GetPrivateData( nPriDataLen );
		if( NULL == pbyPriData || nPriDataLen <= 0 )
			return false;

		if( m_pbyOutBuf && m_dwOutBufSize < DWORD(nPriDataLen+1024) )
		{			// �л��������һ�����������
			delete m_pbyOutBuf;
			m_pbyOutBuf = NULL;
			m_dwOutBufSize = 0;
		}
		if( NULL == m_pbyOutBuf )
		{
			m_dwOutBufSize = nPriDataLen+1024;		// ���ڱ�������
			m_pbyOutBuf = new BYTE[m_dwOutBufSize];
			if( NULL == m_pbyOutBuf )
				return false;
		}
		
		int nOriginalPriDataLen = nPriDataLen;
		PBYTE pDstPriData = m_pbyOutBuf + 6;
		if( m_bDoCompressed )
		{
			pDstPriData += 3;			// keep space for original length
			// 2015.11.23 CYJ Modify, using nPriBufLen instead of nPriDataLen since linux64, long => 8 bytes
			uLongf nPriBufLen = m_dwOutBufSize - 100;
			if( Z_OK != compress( pDstPriData, &nPriBufLen, pbyPriData, nOriginalPriDataLen ) )
				return false;
			nPriDataLen = nPriBufLen;
		}
		else
			memcpy( pDstPriData, pbyPriData, nPriDataLen );

#ifdef _DEBUG
		BYTE byDstFirstByte = *pDstPriData;
#endif //_DEBUG

		int nSectionLen = 2 + nPriDataLen + 4;			// ��������4�ֽ�CRC32
		if( m_bDoCompressed )
			nSectionLen += 3;

		CMyBitStream outBS( m_pbyOutBuf, m_dwOutBufSize );
		outBS.PutBits8( m_byTableID );
		outBS.PutBits( 8,4 );
		outBS.PutBits( nSectionLen, 20 );
		outBS.PutBits8( m_byBuildCounter );
		outBS.PutBit( m_bDoCompressed ? 1 : 0 );
		outBS.PutBits( 0, 7 );
		if( m_bDoCompressed )
			outBS.PutBits( nOriginalPriDataLen, 24 );
#ifdef _DEBUG
		ASSERT( (outBS.GetTotalWriteBits() & 7) == 0 );
		if( m_bDoCompressed )
			ASSERT( outBS.GetTotalWriteBits() == 9 * 8 );
		else
			ASSERT( outBS.GetTotalWriteBits() == 6 * 8 );
#endif //_DEBUG

		outBS.FinishWrite();

#ifdef _DEBUG
		ASSERT( *pDstPriData == byDstFirstByte );
#endif
		
		DWORD dwCRC32 = DVB_GetCRC32( m_pbyOutBuf, nSectionLen-4+4 );	// ���Ȳ�����CRC����������ͷ��4�ֽ�		
		*(PDWORD)( m_pbyOutBuf + nSectionLen-4+4 ) = SWAP_DWORD( dwCRC32 );
		m_nOutBufLen = nSectionLen + 4;		//	������ǰ���4�ֽ�
	}

	if( NULL == m_pbyOutBuf || 0 == m_nOutBufLen )
		return false;

	Encapsulate( m_pbyOutBuf, m_nOutBufLen );
	return true;
}

///////////////////////////////////////////////////////////////////////
//	class CTSVODPSI_VOD_IdentityTableGenerator
CTSVODPSI_VOD_IdentityTableGenerator::CTSVODPSI_VOD_IdentityTableGenerator()
	: CTSVODPSI_TableGeneratorBase(DVBPSI_TBLID_TONGSHI_VOD_IDENTITY, false )
{
	Preset();
}

///-------------------------------------------------------
/// CYJ,2005-4-20
/// ��������:
///		Ԥ����Ϣ
/// �������:
///		��
/// ���ز���:
///		��
void CTSVODPSI_VOD_IdentityTableGenerator::Preset()
{
	CTSVODPSI_TableGeneratorBase::Preset();

	memset( &m_PrivateData, 0, sizeof(m_PrivateData) );
	memcpy( m_PrivateData.m_acIdentity, "TongshiVOD", 10 );
	m_PrivateData.m_dwReserved_1 = 1;		//  CYJ,2007-3-17 �޸�Ϊ 1
	m_nPrivateDataLen = 0;
	SetModifyFlag( true );
}

///-------------------------------------------------------
/// CYJ,2005-4-19
/// ��������:
///		��ȡ˽�б�����
/// �������:
///		nOutLen				����ֽ���
/// ���ز���:
///		��
PBYTE CTSVODPSI_VOD_IdentityTableGenerator::GetPrivateData( int & nOutLen )
{
	// 10  => sizeof(m_acIdentity[10]);
	// 8 => sizeof(m_dwRootChFreq_kHZ) + sizeof(m_dwReserved_1)
	// 2 => m_strIPAndPort's Len + m_strTelephoneNo.Len
	m_nPrivateDataLen = 10+8+2+m_strIPAndPort.GetLength()+m_strTelephoneNo.GetLength();
	CMyBitStream bs( m_PrivateData.m_abyData, sizeof(m_PrivateData.m_abyData) );

	int nIPLen = m_strIPAndPort.GetLength();
	int i;
	bs.PutBits8( nIPLen );
	for( i=0; i<nIPLen; i++ )
	{
		bs.PutBits8( m_strIPAndPort[i] );
	}
	int nPhoneLen = m_strTelephoneNo.GetLength();
	bs.PutBits8( nPhoneLen );
	for( i=0; i<nPhoneLen; i++ )
	{
		bs.PutBits8( m_strTelephoneNo[i] );
	}	
	bs.FinishWrite();
	ASSERT( m_nPrivateDataLen == 18 + bs.GetTotalWriteBits()/8 );
	
	nOutLen = m_nPrivateDataLen;
	return (PBYTE)&m_PrivateData;
}
	
///-------------------------------------------------------
/// CYJ,2005-4-19
/// ��������:
///		���ø�Ƶ����Ƶ��
/// �������:
///		dwFreqInKHz			Ƶ�ʣ���λ kHz
/// ���ز���:
///		ԭ����Ƶ��
DWORD	CTSVODPSI_VOD_IdentityTableGenerator::SetRootChFreq( DWORD dwFreqInKHz )
{
	DWORD dwRetVal = m_PrivateData.m_dwRootChFreq_kHZ;

	m_PrivateData.m_dwRootChFreq_kHZ = SWAP_DWORD( dwFreqInKHz );
	SetModifyFlag( true );

	return dwRetVal;
}

///-------------------------------------------------------
/// CYJ,2007-3-17
/// ��������:
///		���õ㲥���� IP ��ַ�Ͷ˿�
/// �������:
///		lpszIP				IP ��ַ,����Ϊ���ַ���
///		nPort				�˿ڣ�������� 1024
/// ���ز���:
///		true				�ɹ�
///		false				ʧ��
bool CTSVODPSI_VOD_IdentityTableGenerator::SetServiceIPAndPort( LPCSTR lpszIP, int nPort )
{
	m_strIPAndPort = "";
	if( NULL == lpszIP || 0 == *lpszIP || nPort < 1024  )
		return false;
	m_strIPAndPort.Format("%s : %d", lpszIP, nPort );
	if( m_strIPAndPort.GetLength() >= 250 )
		m_strIPAndPort.ReleaseBuffer( 250 );
	SetModifyFlag( true );
	return true;
}

///-------------------------------------------------------
/// CYJ,2007-3-17
/// ��������:
///		���ý���绰
/// �������:
///		lpszTelephoneNo		�绰
/// ���ز���:
///		��
void CTSVODPSI_VOD_IdentityTableGenerator::SetServiceTelephoneNo( LPCSTR lpszTelephoneNo )
{
	m_strTelephoneNo = lpszTelephoneNo;
	if( m_strTelephoneNo.GetLength() > 250 )	
		m_strTelephoneNo.ReleaseBuffer( 250 );
	SetModifyFlag( true );
}

