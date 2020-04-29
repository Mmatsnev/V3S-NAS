///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2007-1-17
///
///		��;��
///			ͨ�� VOD �㲥Զ�̿��Ʊ�
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӡ��ڲ�ʹ��!	  			     !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	��Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!			 !
///=======================================================

// TSVODPSI_RemoteCtrlTblGenerator.cpp: implementation of the CTSVODPSI_RemoteCtrlTblGenerator class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tsvodpsi_remotectrltblgenerator.h"
#include "bitstream.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTSVODPSI_RemoteCtrlTblGenerator::CTSVODPSI_RemoteCtrlTblGenerator(BYTE byTableID, bool bDoCompress)
  :CTSVODPSI_TableGeneratorBase( byTableID, bDoCompress )
{
	m_wSID = 0;						// ��� != 0 �����ʾ�ñ������һ��SID�ģ���֮�����һ��STB ID
	m_llSTBID = 0;
	m_llEndSTBID = 0;
	
	m_pDataBuf = NULL;
	m_nBufSize = 0;
	m_nDataLen = 0;

	m_nInstructionCount = 0;
	m_dwEncryptParameter = 0;
}

CTSVODPSI_RemoteCtrlTblGenerator::~CTSVODPSI_RemoteCtrlTblGenerator()
{
	if( m_pDataBuf )
		free( m_pDataBuf );
	m_pDataBuf = NULL;
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// ��������:
///		��ȡ˽������
/// �������:
///		��
/// ���ز���:
///		��
PBYTE CTSVODPSI_RemoteCtrlTblGenerator::GetPrivateData( int & nOutLen )
{
	int nHeaderLen = 2;						// �̶��ı���
	if( m_wSID )
		nHeaderLen += 2;					// SID ����
	else
	{
		nHeaderLen += 8;					// �ض���STB ID
		if( m_llEndSTBID )
			nHeaderLen += 8;				// ��Χ STB ID
	}
	if( m_dwEncryptParameter )
		nHeaderLen += 2;					// ���м���
	nHeaderLen ++;							// ָ���

#ifdef _DEBUG
	DWORD dwValueBak = *(PDWORD)(m_pDataBuf + BUFSIZE_FOR_TBL_HEADER);
#endif //_DEBUG

	PBYTE pInstructionCompiledBufPtr = m_pDataBuf + BUFSIZE_FOR_TBL_HEADER - nHeaderLen;
	int nInstructionCompiledLen = m_nDataLen - BUFSIZE_FOR_TBL_HEADER + nHeaderLen;

	CMyBitStream bs( pInstructionCompiledBufPtr, nHeaderLen );
	bs.PutBit( m_wSID ? 1 : 0 );
	bs.PutBit( m_llEndSTBID ? 1 : 0 );
	bs.PutBit( m_dwEncryptParameter ? 1 : 0 );
	bs.PutBits( 0, 13 );

	if( m_wSID )
		bs.PutBits16( m_wSID );
	else 
	{
		bs.PutBits32( (DWORD)( m_llSTBID >> 32 ) );
		bs.PutBits32( (DWORD)m_llSTBID );
		if( m_llEndSTBID )
		{
			bs.PutBits32( (DWORD)( m_llEndSTBID >> 32 ) );
			bs.PutBits32( (DWORD)m_llEndSTBID );
		}
	}

	if( m_dwEncryptParameter )
		bs.PutBits16( (WORD)m_dwEncryptParameter );
	bs.PutBits8( m_nInstructionCount );
	bs.FinishWrite();
	ASSERT( bs.GetTotalWriteBits()/8 == nHeaderLen );

#ifdef _DEBUG
	// ��֤�Ƿ��ƻ�
	ASSERT( dwValueBak == *(PDWORD)(m_pDataBuf + BUFSIZE_FOR_TBL_HEADER) );
#endif //_DEBUG

	nOutLen = nInstructionCompiledLen;
	return pInstructionCompiledBufPtr;
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// ��������:
///		��ʼ��
/// �������:
///		��
/// ���ز���:
///		��
void CTSVODPSI_RemoteCtrlTblGenerator::Initialize()
{
	m_nDataLen = 0;
	m_wSID = 0;										// ��� != 0 �����ʾ�ñ������һ��SID�ģ���֮�����һ��STB ID
	m_llSTBID = m_llEndSTBID = 0;
	m_nInstructionCount = 0; 

	SetModifyFlag( true );

	m_dwEncryptParameter = 0;
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// ��������:
///		���ø����������һ����Ŀ��
/// �������:
///		wSID				��Ŀ��
/// ���ز���:
///		��
void CTSVODPSI_RemoteCtrlTblGenerator::SetSID( WORD wSID )
{
	m_llSTBID = 0;
	m_wSID = wSID;
	
	SetModifyFlag( true );
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// ��������:
///		���ø��������һ�������ն�
/// �������:
///		llSTBID					������ID
/// ���ز���:
///		true					�ɹ�
///		false					ʧ�ܣ��Ѿ����ù�SID��Ҫ�ȵ��� Initialize
void CTSVODPSI_RemoteCtrlTblGenerator::SetSTBID( MY_LONG64 llSTBID, MY_LONG64 llEndSTBID )
{
	m_wSID = 0;

	m_llSTBID = llSTBID;
	m_llEndSTBID = llEndSTBID;

	SetModifyFlag( true );
}

///-------------------------------------------------------
/// CYJ,2007-1-18
/// ��������:
///		���� STB ID
/// �������:
///		��
/// ���ز���:
///		��
void CTSVODPSI_RemoteCtrlTblGenerator::SetSTBID( BYTE abySTBID[] )
{
	m_wSID = 0;

	m_llSTBID = 0;
	for(int i=0; i<8; i++ )
	{
		m_llSTBID <<= 8;
		m_llSTBID += abySTBID[i];
	}

	m_llEndSTBID = 0;

	SetModifyFlag( true );
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// ��������:
///		���ü��ܲ���
/// �������:
///		��
/// ���ز���:
///		��
void CTSVODPSI_RemoteCtrlTblGenerator::SetEncryptParameter( WORD wParameter )
{
	// FIXME
	m_dwEncryptParameter = (1<<16) + wParameter;		// �� 16 ���ر�ʾ�Ƿ���м���
	SetModifyFlag( true );

	ASSERT( FALSE );
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// ��������:
///		�������ָ��
/// �������:
///		��
/// ���ز���:
///		��
void CTSVODPSI_RemoteCtrlTblGenerator::CleanInstructions()
{
	m_nDataLen = BUFSIZE_FOR_TBL_HEADER;	// �����ռ�
	m_nInstructionCount = 0;

	SetModifyFlag( true );
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// ��������:
///		����л�Ƶ����ָ��
/// �������:
///		byChNo				�����л���Ƶ����
/// ���ز���:
///		ָ����ܳ���
int CTSVODPSI_RemoteCtrlTblGenerator::AddIns_SwitchChannel( BYTE byChNo )
{
	m_nInstructionCount ++;
	
	if( false == AcquireMem(3) )
		return -1;

	ASSERT( m_nBufSize >= m_nDataLen+3 );

	CMyBitStream bs( m_pDataBuf+m_nDataLen, m_nBufSize-m_nDataLen);
	bs.PutBits16( RCMDID_SWITCH_CHANNEL );
	bs.PutBits8( byChNo );
	bs.FinishWrite();

	ASSERT( bs.GetTotalWriteBits()/8 == 3 );

	m_nDataLen += (bs.GetTotalWriteBits()/8);

	SetModifyFlag( true );

	return m_nDataLen;
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// ��������:
///		��ӽ���ָ����Ŀ������
/// �������:
///		byPhysNo			����Ƶ����
///		wPMT_PID			PMT_PID
/// ���ز���:
///		ָ����ܳ���
int CTSVODPSI_RemoteCtrlTblGenerator::AddIns_ReceiveProgram( BYTE byPhysNo, WORD wPMT_PID )
{
	m_nInstructionCount ++;
	
	if( false == AcquireMem(5) )
		return -1;

	ASSERT( m_nBufSize >= m_nDataLen+5 );

	CMyBitStream bs( m_pDataBuf+m_nDataLen, m_nBufSize-m_nDataLen);
	bs.PutBits16( RCMDID_RECEIVE_PROGRAM );
	bs.PutBits8( byPhysNo );
	bs.PutBits16( wPMT_PID );
	bs.FinishWrite();

	ASSERT( bs.GetTotalWriteBits()/8 == 5 );

	m_nDataLen += (bs.GetTotalWriteBits()/8);

	SetModifyFlag( true );

	return m_nDataLen;
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// ��������:
///		��ӣ��㲥�����ֵ
/// �������:
///		pBuf				����ֵ
///		byDataLen			���ݳ���
/// ���ز���:
///		ָ����ܳ���
int CTSVODPSI_RemoteCtrlTblGenerator::AddIns_VODOperatorResponse( PBYTE pBuf, BYTE byDataLen )
{
	ASSERT( pBuf && byDataLen );
	if( NULL == pBuf || 0 == byDataLen )
		return -1;

	m_nInstructionCount ++;
	
	if( false == AcquireMem(byDataLen+3) )
		return -1;

	ASSERT( m_nBufSize >= m_nDataLen+byDataLen+3 );

	CMyBitStream bs( m_pDataBuf+m_nDataLen, m_nBufSize-m_nDataLen);
	bs.PutBits16( RCMDID_VOD_OPERATION_RESPONSE );
	bs.PutBits8( byDataLen );
	for(int i=0; i<byDataLen; i++ )
	{
		bs.PutBits8( pBuf[i] );
	}
	bs.FinishWrite();

	ASSERT( bs.GetTotalWriteBits()/8 == byDataLen+3 );

	m_nDataLen += (bs.GetTotalWriteBits()/8);

	SetModifyFlag( true );

	return m_nDataLen;
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// ��������:
///		���ͨ������
/// �������:
///		wCommand			����
///		pBuf				������
///		nLen				����
/// ���ز���:
///		ָ����ܳ���
int CTSVODPSI_RemoteCtrlTblGenerator::AddIns_PrecompiledInstructions( WORD wCommand, PBYTE pBuf, int nLen )
{
	ASSERT( pBuf && nLen );
	if( NULL == pBuf || 0 == nLen )
		return -1;

	m_nInstructionCount ++;
	
	if( false == AcquireMem(nLen+2) )
		return -1;

	ASSERT( m_nBufSize >= m_nDataLen+nLen+2 );

	CMyBitStream bs( m_pDataBuf+m_nDataLen, m_nBufSize-m_nDataLen);
	bs.PutBits16( wCommand );
	for(int i=0; i<nLen; i++ )
	{
		bs.PutBits8( pBuf[i] );
	}
	bs.FinishWrite();

	ASSERT( bs.GetTotalWriteBits()/8 == nLen+2 );

	m_nDataLen += (bs.GetTotalWriteBits()/8);

	SetModifyFlag( true );

	return m_nDataLen;
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// ��������:
///		�����ڴ�
/// �������:
///		nIncBytes			Ҫ�������ӵ��ڴ�
/// ���ز���:
///		true				succ
///		false				no memory
bool CTSVODPSI_RemoteCtrlTblGenerator::AcquireMem( int nIncBytes )
{
	ASSERT( nIncBytes > 0 );
	if( m_nDataLen + nIncBytes < m_nBufSize )
		return true;

	int nNewBufSize;
	if( NULL == m_pDataBuf )
	{
		nNewBufSize = ( BUFSIZE_FOR_TBL_HEADER + nIncBytes + 4095) & (~4095);	// ��4K����
		m_pDataBuf = (PBYTE)malloc( nNewBufSize );
		if( m_pDataBuf )
			m_nDataLen = BUFSIZE_FOR_TBL_HEADER;			// ��һ�η��䣬��Ҫ�����ռ����ͷ
	}
	else
	{
		nNewBufSize = ( m_nDataLen + nIncBytes + 4095) & (~4095);	// ��4K����
		m_pDataBuf = (PBYTE)realloc( m_pDataBuf, nNewBufSize );
	}

	if( m_pDataBuf )
		m_nBufSize = nNewBufSize;
	else
		m_nBufSize = 0;

	return ( NULL != m_pDataBuf );
}

