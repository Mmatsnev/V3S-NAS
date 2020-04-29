///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2005-2-7
///
///		��;��
///			PES ��װ
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӡ��ڲ�ʹ��!	  			     !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	��Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!			 !
///=======================================================

// PESPacketEncapsulator.cpp: implementation of the PESPacketEncapsulator class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pespacketencapsulator.h"
#include "bitstream.h"
#include "dvb_crc.h"

#pragma pack(push,1)
typedef struct tagDVB_SIMPLE_PES_PACKET
{
	BYTE	m_abyStartCodePreFix[3];	// 0,0,1
	BYTE	m_byStreamID;
	BYTE	m_abyPESPacketLen[2];		// Big Endian
public:
	void Preset()
	{
		memset( this, 0, sizeof(struct tagDVB_SIMPLE_PES_PACKET) );
		m_abyStartCodePreFix[0] = m_abyStartCodePreFix[1] = 0;
		m_abyStartCodePreFix[2] = 1;		
	}
	void SetPESPacketLen( WORD wPESPacketLen )
	{
		m_abyPESPacketLen[0] = BYTE(wPESPacketLen>>8);
		m_abyPESPacketLen[1] = BYTE(wPESPacketLen);
	}
}DVB_SIMPLE_PES_PACKET,*PDVB_SIMPLE_PES_PACKET;
#pragma pack(pop)

///////////////////////////////////////////////////////////////////////
// CPESPacketFlagsEncapsulator
///////////////////////////////////////////////////////////////////////
CPESPacketFlagsEncapsulator::CPESPacketFlagsEncapsulator()
{
	Preset();
	memset( m_abyData, 0, sizeof(m_abyData) );
	m_llPTS = 0;
	m_llDTS = 0;
	m_llESCR = 0;
	m_wESCRExtension = 0;
	m_dwESRate = 0;
	m_wLastPESCRC = 0;
	m_bHasCRC = false;
}

///-------------------------------------------------------
/// CYJ,2005-2-24
/// ��������:
///		Ԥ�Ʋ���
/// �������:
///		��
/// ���ز���:
///		��
void CPESPacketFlagsEncapsulator::Preset()
{
	m_byPESScramblingControl = 0;			// 2 ������Ч��ȱʡΪ0
	m_bPES_Priority = false;				// ȱʡΪ 0
	m_bDataAlignmentIndicator = false;		// ȱʡΪ 0
	m_bCopyRight = false;					// ȱʡΪ 0
	m_bOriginalOrCopy = true;				// ȱʡΪ 1
	m_byPTS_DTS_Flag = 0;					// ȱʡΪ 0
	m_bESCRFlag = false;					// ȱʡΪ 0
	m_bESRateFlag = false;					// ȱʡΪ 0
	m_bHasCRC = false;
	m_wLastPESCRC = 0;
}

///-------------------------------------------------------
/// CYJ,2005-2-24
/// ��������:
///		����
/// �������:
///		nLen			��������ĳ���
/// ���ز���:
///		NULL			ʧ��
///		����			�ɹ�����Ϊ��ַ
PBYTE CPESPacketFlagsEncapsulator::Build( int & nLen )
{
	CMyBitStream OutBs( m_abyData, sizeof(m_abyData) );

	OutBs.PutBits( 2, 2 );		// '10'
	OutBs.PutBits( m_byPESScramblingControl, 2 );
	OutBs.PutBit( m_bPES_Priority );
	OutBs.PutBit( m_bDataAlignmentIndicator );
	OutBs.PutBit( m_bCopyRight );
	OutBs.PutBit( m_bOriginalOrCopy );

	OutBs.PutBits( m_byPTS_DTS_Flag, 2 );
	OutBs.PutBit( m_bESCRFlag );
	OutBs.PutBit( m_bESRateFlag );
	OutBs.PutBits(0, 2);
	OutBs.PutBit( m_bHasCRC ? 1 : 0 );
	OutBs.PutBit(0);
	OutBs.PutBits8( 0 );			// PES_Header_data_length; �ȱ����ռ䣬Ȼ�����޸�

	m_byPTS_DTS_Flag &= 3;
	if( m_byPTS_DTS_Flag & 2 )
		OutputPTS_DTS( &OutBs );
	if( m_bESCRFlag )
		OutputESCR( &OutBs );
	if( m_bESRateFlag )
	{
		OutBs.PutBit( 1 );
		OutBs.PutBits(m_dwESRate, 22);
		OutBs.PutBit( 1 );
	}
	ASSERT( (OutBs.GetTotalWriteBits() % 8) == 0 );
	if( m_bHasCRC )					//  CYJ,2006-3-2 �������CRC
		OutBs.PutBits16( m_wLastPESCRC );

	OutBs.FinishWrite();
	nLen = OutBs.GetTotalWriteBits()/8;
	m_abyData[2] = nLen - 3;	 // �޸� PES_Header_data_length
	
	return m_abyData;
}

///-------------------------------------------------------
/// CYJ,2005-2-24
/// ��������:
///		���PTS DTS
/// �������:
///		pOutBs			���������
/// ���ز���:
///		��
void CPESPacketFlagsEncapsulator::OutputPTS_DTS( CMyBitStream * pOutBs )
{
	m_byPTS_DTS_Flag &= 3;
	pOutBs->PutBits( m_byPTS_DTS_Flag, 4 );

	pOutBs->PutBits( DWORD(m_llPTS>>30), 3 );
	pOutBs->PutBit(1);	// Market bit
	pOutBs->PutBits( DWORD(m_llPTS>>15), 15 );
	pOutBs->PutBit(1);	// Market bit
	pOutBs->PutBits( DWORD(m_llPTS), 15 );
	pOutBs->PutBit(1);	// Market bit

	if( m_byPTS_DTS_Flag != 3 )
		return;
	pOutBs->PutBits( 1, 4 );

	pOutBs->PutBits( DWORD(m_llDTS>>30), 3 );
	pOutBs->PutBit(1);	// Market bit
	pOutBs->PutBits( DWORD(m_llDTS>>15), 15 );
	pOutBs->PutBit(1);	// Market bit
	pOutBs->PutBits( DWORD(m_llDTS), 15 );
	pOutBs->PutBit(1);	// Market bit
}

///-------------------------------------------------------
/// CYJ,2005-2-24
/// ��������:
///		����ESCR
/// �������:
///		pOutBs			���������
/// ���ز���:
///		��
void CPESPacketFlagsEncapsulator::OutputESCR( CMyBitStream * pOutBs )
{
	pOutBs->PutBits( 3, 2 );		// ����
	pOutBs->PutBits( DWORD(m_llESCR>>30), 3 );
	pOutBs->PutBit(1);	// Market bit
	pOutBs->PutBits( DWORD(m_llESCR>>15), 15 );
	pOutBs->PutBit(1);	// Market bit
	pOutBs->PutBits( DWORD(m_llESCR), 15 );
	pOutBs->PutBit(1);	// Market bit
	pOutBs->PutBits(m_wESCRExtension, 9 );
	pOutBs->PutBit(1);	// Market bit
}

///-------------------------------------------------------
/// CYJ,2005-2-24
/// ��������:
///		����PTS
/// �������:
///		llPTS		PTS
/// ���ز���:
///		��
void CPESPacketFlagsEncapsulator::SetPTS( LONGLONG llPTS)
{
	m_byPTS_DTS_Flag = 2;
	m_llPTS = llPTS;
}

///-------------------------------------------------------
/// CYJ,2005-2-24
/// ��������:
///		����PTS & DTS
/// �������:
///		llPTS		PTS
///		llDTS		DTS
/// ���ز���:
///		��
void CPESPacketFlagsEncapsulator::SetPTSDTS( LONGLONG llPTS, LONGLONG llDTS )
{
	m_byPTS_DTS_Flag = 3;
	m_llPTS = llPTS;
	m_llDTS = llDTS;
}

///-------------------------------------------------------
/// CYJ,2005-2-24
/// ��������:
///		���� ESCR
/// �������:
///		llESCR			ESCR
///		wExtension		��չ
/// ���ز���:
///		��
void CPESPacketFlagsEncapsulator::SetESCR( LONGLONG llESCR, WORD wExtension )
{
	m_bESCRFlag = true;
	m_llESCR = llESCR;
	m_wESCRExtension = wExtension;
}

///-------------------------------------------------------
/// CYJ,2005-2-24
/// ��������:
///		���� ES Rate
/// �������:
///		dwESRate
/// ���ز���:
///		��
void CPESPacketFlagsEncapsulator::SetESRate(DWORD dwESRate)
{
	m_bESRateFlag = true;
	m_dwESRate = dwESRate;
}

///-------------------------------------------------------
/// CYJ,2006-3-2
/// ��������:
///		�����ϸ�PES֡��CRC
/// �������:
///		wLastPESCRC
/// ���ز���:
///		��
void CPESPacketFlagsEncapsulator::SetCRC( WORD wLastPESCRC )
{
	m_bHasCRC = true;
	m_wLastPESCRC = wLastPESCRC;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPESPacketEncapsulator::CPESPacketEncapsulator()
{
	ASSERT( sizeof(DVB_SIMPLE_PES_PACKET) == 6 );
	m_byTSContinuity = 0;
	m_wPID = INVALID_PID;

	m_bHasAdationField = false;
	m_bCalculateCRC = true;
	m_wLastPESCRC = 0;
}

CPESPacketEncapsulator::~CPESPacketEncapsulator()
{

}

///-------------------------------------------------------
/// CYJ,2005-2-7
/// ��������:
///		��װһ��Ԫ����
/// �������:
///		pESData				Ԫ��������
///		nLen				Ԫ��������
///		byStreamID			Ԫ��������
/// ���ز���:
///		>=0					��װ���TS�������
///		<0					ʧ��
int CPESPacketEncapsulator::Encapsulate(PBYTE pESData, int nLen, BYTE byStreamID )
{
	ASSERT( pESData && nLen > 0 );
	if( NULL == pESData || nLen <= 0 )
	{
		m_bHasAdationField = false;
		m_PESPacketFlagsEncapsulator.Preset();
		m_AdaptionEncapsulator.Preset();
		return -1;
	}

	//	��һ��PES����
	int nRetVal = 0;
	int nByteDone = EncapsulateFirstPESPacket( pESData, nLen, byStreamID );
	m_bHasAdationField = false;
	m_PESPacketFlagsEncapsulator.Preset();
	m_AdaptionEncapsulator.Preset();
	if( nByteDone < 0 )
		return -1;

	if( m_bCalculateCRC )			//  CYJ,2006-3-2 ֧�ּ���CRC
		m_wLastPESCRC = DVB_GetCRC16( pESData, nLen );

	ASSERT( !m_bHasAdationField );
	
	nRetVal ++;
	nLen -= nByteDone;
	pESData += nByteDone;

	//	�м�ķ���
	while( nLen >= (DVB_TS_PACKET_SIZE-4) )
	{
		CTSPacketEncapsulator * pTSPacket = GetTSPacket();
		ASSERT( pTSPacket );
		if( NULL == pTSPacket )
			return -1;

		memcpy( pTSPacket->m_abyData+4, pESData, (DVB_TS_PACKET_SIZE-4) );
		OnTSPacketReady( static_cast<PDVB_TS_PACKET>(pTSPacket) );

		pESData += (DVB_TS_PACKET_SIZE-4);
		nLen -= (DVB_TS_PACKET_SIZE-4);
		nRetVal ++;
	}
	ASSERT( nLen >= 0 );

	// ���һ�����飬����184������������Ҫͨ�� Adaption Field ��������ݣ�ʹ֮�ﵽ1���ֽ�Ҳ����
	if( nLen > 0 )
	{
		if( nLen == 183 )
		{					//	��Ϊ���ɶ��ķ�װ�����󣬵�����183�ֽڵ�ʱ�򣬰ѵ�һ�����ݵ��ɱ�־λ������
							//  ����ֻ����183�ֽ�ʱ��Ӳ�Էֳ�2��TS����
			EncapsulateLastPESPacket( pESData, 90 );
			nRetVal ++;
			pESData += 90;
			nLen -= 90;
		}

		EncapsulateLastPESPacket( pESData, nLen );
		nRetVal ++;
	}

	return nRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-3-7
/// ��������:
///		��װ���һ��TS����
/// �������:
///		pPESData			��������
///		nLen				����
/// ���ز���:
///		-1					ʧ��
int CPESPacketEncapsulator::EncapsulateLastPESPacket(PBYTE pESData, int nLen)
{
	ASSERT( nLen < (DVB_TS_PACKET_SIZE-4) );
	CTSPacketEncapsulator * pTSPacket = GetTSPacket();
	ASSERT( pTSPacket );
	if( NULL == pTSPacket )
		return -1;

	pTSPacket->SetAdaptionField( 3, (DVB_TS_PACKET_SIZE-5-nLen) );
	PBYTE pBufDst = pTSPacket->m_abyData + DVB_TS_PACKET_SIZE - nLen;
	memset( pTSPacket->m_abyData+5, 0, DVB_TS_PACKET_SIZE-5-nLen );

#ifdef _DEBUG
	int nPayloadLen;		
	ASSERT( pBufDst == pTSPacket->GetPayloadData( nPayloadLen ) );
	ASSERT( nPayloadLen == nLen );
#endif // _DEBUG

	memcpy( pBufDst, pESData, nLen );
	OnTSPacketReady( static_cast<PDVB_TS_PACKET>(pTSPacket) );

#ifdef _DEBUG
	nLen -= nPayloadLen;
#endif //_DEBUG

	return 0;
}

///-------------------------------------------------------
/// CYJ,2005-2-7
/// ��������:
///		��װ��һ��PES��
/// �������:
///		pESData				Ԫ��������
///		nLen				Ԫ��������
///		byStreamID			Ԫ��������
/// ���ز���:
///		ʵ�ʷ�װ��ES�����ֽ���
///		-1					ʧ��
int CPESPacketEncapsulator::EncapsulateFirstPESPacket(PBYTE pESData, int nLen, BYTE byStreamID )
{	
	CTSPacketEncapsulator * pTSPacket = GetTSPacket();
	ASSERT( pTSPacket );
	if( NULL == pTSPacket )
		return -1;

	pTSPacket->SetPayloadStartIndicator();
	int nAdaptionFieldLen = 0;
	if( m_bHasAdationField )
	{
		int nLen = 0;
		PBYTE pBufAdaptionField = m_AdaptionEncapsulator.Build( nLen );
		ASSERT( pBufAdaptionField );
		if( pBufAdaptionField )
		{
			pTSPacket->SetAdaptionField( 3, nLen, pBufAdaptionField );
			nAdaptionFieldLen = nLen + 1;
		}
	}

	if( m_bCalculateCRC )
		m_PESPacketFlagsEncapsulator.SetCRC( m_wLastPESCRC );

	int nPESFlagsBufLen = 0;		// PES ��־λ����
	PBYTE pPESFlagsBuf = m_PESPacketFlagsEncapsulator.Build( nPESFlagsBufLen );
	ASSERT( pPESFlagsBuf && nPESFlagsBufLen >= 3 );

	int nPESPacketLen = nLen + nPESFlagsBufLen;		// PES_Packet_Len ������ǰ���6���ֽ�
	if( nPESPacketLen > 0xFFFF )
		nPESPacketLen = 0;

	PBYTE pBuf = pTSPacket->m_abyData + 4 + nAdaptionFieldLen;
	PDVB_SIMPLE_PES_PACKET pPESHeader = (PDVB_SIMPLE_PES_PACKET)pBuf;
	pPESHeader->Preset();
	pPESHeader->SetPESPacketLen( nPESPacketLen );

	pBuf += sizeof(DVB_SIMPLE_PES_PACKET);
	memcpy( pBuf, pPESFlagsBuf, nPESFlagsBufLen );		// ����PES��־λ��Ϣ
	pBuf += nPESFlagsBufLen;

	int nPayloadLen;
	nPayloadLen = 184 - nAdaptionFieldLen - sizeof(DVB_SIMPLE_PES_PACKET) - nPESFlagsBufLen;

#ifdef _DEBUG
	int nTSPayloadLen;
	ASSERT( pTSPacket->GetPayloadData(nTSPayloadLen));
	ASSERT( nTSPayloadLen == int(nPayloadLen+sizeof(DVB_SIMPLE_PES_PACKET)+nPESFlagsBufLen) );
#endif //_DEBUG

	if( nPayloadLen > nLen )
		nPayloadLen = nLen;

	pPESHeader->m_byStreamID = byStreamID;

	memcpy( pBuf, pESData, nPayloadLen );

	OnTSPacketReady( static_cast<PDVB_TS_PACKET>(pTSPacket) );

	return nPayloadLen;
}

///-------------------------------------------------------
/// CYJ,2005-2-23
/// ��������:
///		����PCR
/// �������:
///		llPCR			PCR
///		wExternsion		��չʱ��
/// ���ز���:
///		��
void CPESPacketEncapsulator::SetPCR( LONGLONG llPCR, WORD wExtension )
{
	m_bHasAdationField = true;
	m_AdaptionEncapsulator.SetPCR( llPCR, wExtension );
}

///-------------------------------------------------------
/// CYJ,2005-2-24
/// ��������:
///		����PTS
/// �������:
///		llPTS			PTS
/// ���ز���:
///		��
void CPESPacketEncapsulator::SetPTS( LONGLONG llPTS )
{
	m_PESPacketFlagsEncapsulator.SetPTS( llPTS );
}

///-------------------------------------------------------
/// CYJ,2005-2-24
/// ��������:
///		���� PTS & DTS
/// �������:
///		llPTS			PTS
///		llDTS			DTS
/// ���ز���:
///		��
void CPESPacketEncapsulator::SetPTSDTS(LONGLONG llPTS, LONGLONG llDTS)
{
	m_PESPacketFlagsEncapsulator.SetPTSDTS( llPTS, llDTS );
}

///-------------------------------------------------------
/// CYJ,2005-2-24
/// ��������:
///		���� ESCR
/// �������:
///		llESCR			ESCR
///		wExtension		extension
/// ���ز���:
///		��
void CPESPacketEncapsulator::SetESCR( LONGLONG llESCR, WORD wExtension )
{
	m_PESPacketFlagsEncapsulator.SetESCR( llESCR, wExtension );
}

///-------------------------------------------------------
/// CYJ,2005-2-25
/// ��������:
///		��������ɷ��ʵ�
/// �������:
///		bValue			�Ƿ�����ɷ���
/// ���ز���:
///		��
void CPESPacketEncapsulator::SetRandomAccess( bool bValue )
{ 
	m_bHasAdationField = true;
	m_AdaptionEncapsulator.SetRandomAccess(bValue); 
}
