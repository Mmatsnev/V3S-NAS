///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2005-1-12
///
///		��;��
///			PES �������¹���
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӡ��ڲ�ʹ��!	  			     !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	��Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!			 !
///=======================================================

#include "stdafx.h"

#include <stdio.h>

#include "pespacket.h"
#include "bitstream.h"
#include <MyArray.h>
#include "dvb_crc.h"

#ifdef _DEBUG
	#define __ENABLE_TRACE__
#endif // _DEBUG

class CPESResponserArray : public CMyArray<CPESPacket*>
{
public:
	CPESResponserArray(){}
	virtual ~CPESResponserArray(){}
};

CPCR_TSPacketResponser::CPCR_TSPacketResponser()
{
	m_paPESResponser = new CPESResponserArray;
}

CPCR_TSPacketResponser::~CPCR_TSPacketResponser()
{
	if( m_paPESResponser )
		delete m_paPESResponser;
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// ��������:
///		���� PCR TS ����
/// �������:
///		pPacket			TS ����
/// ���ز���:
///		��
void CPCR_TSPacketResponser::PushOneTSPacket( PDVB_TS_PACKET pPacket )
{
	ASSERT( m_paPESResponser );

	if( NULL == m_paPESResponser )
		return;
	int nCount = m_paPESResponser->GetSize();
	if( 0 == nCount )
		return;
	CPESPacket ** ppResponser = m_paPESResponser->GetData();
	if( !(*ppResponser)->HandlePCR( pPacket ) )
		return;				//	û�� PCR

	MY_LONG64 llPCR = 0;
	WORD wExtension = 0;
	(*ppResponser)->GetPCR( llPCR, wExtension );

	for(int i=1; i<nCount; i++)
	{						//	�Ż������������ٽ���PCR�ˡ�
		ppResponser ++;
		(*ppResponser)->SetPCR( llPCR, wExtension );
	}
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// ��������:
///		�Ƿ���Ч
/// �������:
///		��
/// ���ز���:
///		��
bool CPCR_TSPacketResponser::IsValid()
{
	return ( m_paPESResponser != NULL );
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// ��������:
///		ɾ������ PES ������
/// �������:
///		��
/// ���ز���:
///		��
void CPCR_TSPacketResponser::RemoveAll()
{
	if( m_paPESResponser )
		m_paPESResponser->RemoveAll();
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// ��������:
///		ɾ��һ�� PES ������
/// �������:
///		pResponser		��ɾ���� PES ������
/// ���ز���:
///		��
void CPCR_TSPacketResponser::Remove( CPESPacket * pResponser )
{
	int nNo = Find( pResponser );
	if( nNo >= 0 )
		m_paPESResponser->RemoveAt( nNo );
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// ��������:
///		����һ�� PES ������
/// �������:
///		pResponser			���Ҷ���
/// ���ز���:
///		>=0					���
///		<0					ʧ��
int	CPCR_TSPacketResponser::Find( CPESPacket * pResponser )
{
	ASSERT( m_paPESResponser && pResponser );
	if( NULL == m_paPESResponser )
		return -1;
	int nCount = m_paPESResponser->GetSize();
	for( int i=0; i<nCount; i++)
	{
		if( m_paPESResponser->ElementAt( i ) == pResponser )
			return i;
	}
	return -1;
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// ��������:
///		���һ�� PES ������
/// �������:
///		pResponser		����ӵ� PES ������
/// ���ز���:
///		��
void CPCR_TSPacketResponser::Add( CPESPacket * pResponser )
{
	ASSERT( m_paPESResponser && pResponser );
	if( NULL == m_paPESResponser || NULL == pResponser )
		return;
	if( Find( pResponser ) >= 0 )
		return;				//	�Ѿ�����
	m_paPESResponser->Add( pResponser );
}

//---------------------------------------------------------
//	PES Packet data struct
SELFDEFINE_PES_PROCESS_MODE tagDVB_PES_PACKET_HEADER::GetProcessMode()
{
	ASSERT( IsPESPacket() );
	if( DVBPESSI_PADING_STREAM == m_byStreamID )
		return PES_PROCESS_MODE_PADING;
	else if( DVBPESSI_PROGRAM_STREAM_MAP == m_byStreamID
			|| DVBPESSI_PRIVATE_STREAM_2 == m_byStreamID
			|| DVBPESSI_ECM_STREAM == m_byStreamID
			|| DVBPESSI_EMM_STREAM == m_byStreamID
			|| DVBPESSI_PROGRAM_STREAM_DIRECTORY == m_byStreamID )
	{
		return PES_PROCESS_MODE_EMM_EMC_COMPATIBLE;
	}
	return PES_PROCESS_MODE_AV_STREAM_COMPATIBLE;
}

///-------------------------------------------------------
/// CYJ,2005-1-13
/// ��������:
///		��ȡPESͷ��С
/// �������:
///		��
/// ���ز���:
///		PES ͷ��С
///		<0			ʧ��
int tagDVB_PES_PACKET_HEADER::GetHeaderLen()
{
	SELFDEFINE_PES_PROCESS_MODE eProcessMode = GetProcessMode();
	if( PES_PROCESS_MODE_PADING == eProcessMode )
		return -1;

	else if( PES_PROCESS_MODE_EMM_EMC_COMPATIBLE == eProcessMode )
		return 6;

	WORD wOfs;
	if( 0x80 == ( m_abyData[0] & 0xC0 ) )
	{									// �� '10' ��ͷ
		wOfs = 3 + m_abyData[2];		// �����ֽڵı�־λ��1���ֽڵ�ͷ����
	}
	else
	{
		// FIXME
		wOfs = 0;
		while( wOfs < 23 && m_abyData[wOfs] == 0xFF )
		{								// ���� MPEG��1 ������ֽ�
			wOfs ++;
		}
		if( wOfs >= 23 )
			return -1;
	}
	return wOfs + 6;
}

///-------------------------------------------------------
/// CYJ,2005-1-14
/// ��������:
///		��ȡ��Ч���ص�ַ�볤��
/// �������:
///		nPacketLen			������ش�С
/// ���ز���:
///		��Ч���ص�ַ
///		NULL				û������
PBYTE tagDVB_PES_PACKET_HEADER::GetPayloadData( int & nPayloadDataLen )
{
	nPayloadDataLen = 0;
	int nHeaderSize = GetHeaderLen();
	if( nHeaderSize <= 6 )			//	���볬�� 6 �ֽ�
		return NULL;

	nHeaderSize -= 6;				//	�̶���ͷ����Ϊ HeaderLen �����̶�ͷ,�������Ǵӵ������ֽڿ�ʼ���.

	WORD wPackLen = GetPacketLength();
	if( wPackLen )
		nPayloadDataLen = wPackLen - nHeaderSize;
	else
		nPayloadDataLen = 0;			//	δ֪��С

	ASSERT( nPayloadDataLen >= 0 );
	return m_abyData + nHeaderSize;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPESPacket::CPESPacket(int nCacheBufSize) : CMyHeap( nCacheBufSize )
{
	m_wPS_SCR_Base_0_14 = 0;			//	system clock reference
	m_wPS_SCR_Base_15_29 = 0;
	m_wPS_SCR_Base_30_32 = 0;
	m_wPS_SCR_Extension = 0;			// system clock reference extension
	m_bRequireCompletePacket = true;	//	Ҫ�����������PES���飬������Ƶ����������������ķ��飬���洦������ƴ��
	m_nOutputMethod = OUTPUT_METHOD_AS_ELEMENTARY_STREAM;	// ȱʡ���ΪԪ����
	// FIXME,��֪����дʲôֵ
	m_dwProgramMuxRate = 0x71CC;
	m_pPCRResponser = NULL;
	m_nPayloadLen = 0;
	m_bDoCRCCheck = true;
	m_byLastHasCRCFlags = 0;

	Reset( true );
}

CPESPacket::~CPESPacket()
{

}

///-------------------------------------------------------
/// CYJ,2005-1-11
/// ��������:
///		��λ��ṹ������ڳ�ʼ��
/// �������:
///		bForce			�Ƿ�ǿ�Ƹ�λ��ȱʡΪfalse
/// ���ز���:
///		��
void CPESPacket::Reset(bool bForce)
{
	m_bHeaderReceived = false;
	m_wErrorCounter = 0;					// TS packet ��������Ĵ���
	m_byExpectTSContinuityCounter = 0xF0;	// �ϴ� TS packet ��continuity counter
	m_dwPESBytesReceived = 0;

	CMyHeap::Reset();

	m_bHasPTS = false;
	m_bHasDTS = false;
	m_bHasESMuxRate = false;

	m_byStreamID = 0;
	m_wPacketLength = 0;
	m_bHasSCR = false;
	m_nPayloadLen = 0;
	m_bRandomAccessPoint = false;
	m_bTSPacketDiscontinuity = false;

	if( bForce )
	{
		m_bRequireCompletePacket = true;		//	Ҫ�����������PES���飬������Ƶ����������������ķ��飬���洦������ƴ��
		m_nOutputMethod = OUTPUT_METHOD_AS_ELEMENTARY_STREAM;	// ȱʡ���ΪԪ����
		m_pPCRResponser = NULL;
		m_bDoCRCCheck = true;			//  CYJ,2006-3-2 �����Ƿ����CRC���
	}
}

///-------------------------------------------------------
/// CYJ,2005-1-11
/// ��������:
///		�ж�һ�������Ƿ���Ч
/// �������:
///		��
/// ���ز���:
///		true			��Ч
///		false			��Ч
bool CPESPacket::IsValid()
{
	return CMyHeap::IsValid();
}

///--------------------------------------------------------------
///	CYJ, 2005-1-13
///	��������:
///		����һ��TS����
///	�������:
///		pPacket			���յ���TS����
///	���ز���:
///		��
void CPESPacket::PushOneTSPacket( PDVB_TS_PACKET pPacket )
{
	ASSERT( IsValid() );

	bool bIsPayloadStart = pPacket->IsPayloadUnitStart();

	if( !m_bHeaderReceived && !bIsPayloadStart )
		return;					//	û�н��յ�����ͷ���Ҳ���һ����Ŀ�ʼ��

	int nDataLen;
	PBYTE pPayloadData = pPacket->GetPayloadData( nDataLen );
	if( NULL == pPayloadData )
	{
#ifdef __ENABLE_TRACE__
	//	TRACE("No TS payload data.\n");
#endif //__ENABLE_TRACE__
		HandlePCR( pPacket );
		return;					//	û������
	}

	BYTE byContinuityCounter = pPacket->GetContinuityCount();
	bool m_bTSPacketDiscontinuity = (pPacket->m_abyData[3]&0x20) && (pPacket->m_abyData[5] & 0x80);

	//	FIXME, �б�Ҫ�ж�TS�����Ƿ��ظ�
#if 0
	if( !m_bTSPacketDiscontinuity && (m_byExpectTSContinuityCounter == ((byContinuityCounter+1)&0xF)) )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITablesBase::PushOneTSPacket, find one duplicated TS packeted.\n");
#endif // __ENABLE_TRACE__
		return;					//	�ظ���TS����
	}
#endif // 0

	int nByteHasAllocated = GetMemoryAllocated();
	if( bIsPayloadStart )
	{									// ���յ�һ���±�
		if( m_bHeaderReceived && nByteHasAllocated  > 0 )	// �ύԭ��������
		{
			if( m_nPayloadLen > 0 )
			{							//	��Ч�ĸ��ش�С���ж��Ƿ�Խ��
				int nByteLeft = m_nPayloadLen - m_dwPESBytesReceived;	// m_dwPESBytesReceived ��ʾ�Ѿ����͵��ֽ�����
				if( nByteHasAllocated > nByteLeft )
					nByteHasAllocated = nByteLeft;		//	ȥ������Ĳ���
			}
			if( nByteHasAllocated > 0 )				//  CYJ,2006-3-1 ��ֹ����� nByteHasAllocated
			{
				bool bDoSendEvent = true;
				if( m_nPayloadLen && m_bDoCRCCheck && m_bRequireCompletePacket && 0 == m_dwPESBytesReceived )
				{	// ֻ����ȷ���� m_nPayloadLen����Ҫ���� CRC ��⣬ͬʱҪ������������һ�û��ǿ�����ʱ���Ż���CRC���
					// һ������£�ֻ����ES���ʱ������Ҫ��CRC���
					PBYTE pCRC = GetCRC( pPayloadData, nDataLen );
					m_byLastHasCRCFlags <<= 1;
					if( pCRC )
					{								// ����CRC
						m_byLastHasCRCFlags |= 1;	// �����һ����CRC
						int nPESHeaderLen = nByteHasAllocated - m_nPayloadLen;
						if( nPESHeaderLen >= 0 && nByteHasAllocated-nPESHeaderLen > 0 )
						{
							WORD wCRC = DVB_GetCRC16( GetHeapBuf()+nPESHeaderLen, nByteHasAllocated-nPESHeaderLen );
							bDoSendEvent = ( wCRC == ( (pCRC[0]<<8) | pCRC[1] ) );
						}
						else
							bDoSendEvent = false;

					#if 1 // _DEBUG
						if( !bDoSendEvent )
							fprintf( stderr, "One PES\'s CRC error, abort\n" );
					#endif //_DEBUG
					}
					else
					{
						 if( m_byLastHasCRCFlags & 0xF )	// ������4�ζ�û��CRC������Ϊû�У���������Ϊ��PES����Ĵ������´λ�û�У�����Ϊû��
						{
							bDoSendEvent = false;
						#ifdef _DEBUG
							fprintf(stderr,"PESPacket, m_byLastHasCRCFlags=%x, same as CRC error\n", m_byLastHasCRCFlags );
						#endif //_DEBUG
						}
					}
				}
				if( bDoSendEvent )
				{
					int nByteLeftInHeap = GetHeapSize() - nByteHasAllocated;
					if( nByteLeftInHeap > 16 )
						nByteLeftInHeap = 16;
					if( nByteLeftInHeap > 0 )
						memset( GetHeapBuf() + nByteHasAllocated, 0xFF, nByteLeftInHeap );		// ĩ����� 0XFF����ֹ����
					OnPESReceived( GetHeapBuf(), nByteHasAllocated, m_dwPESBytesReceived, m_wErrorCounter );	//	֪ͨ���յ���
				}
			}
			Reset();				//	���¿�ʼ
		}
		PDVB_PES_PACKET_HEADER pPESHeader = (PDVB_PES_PACKET_HEADER)pPayloadData;
		if( !pPESHeader->IsPESPacket() )
		{							//	�����PESͷ
			m_bHeaderReceived = false;
#ifdef __ENABLE_TRACE__
			TRACE("No a PES packet.\n");
#endif //__ENABLE_TRACE__
			return;
		}
		m_bHeaderReceived = true;

		DecodeTSAdaptionField( pPacket );
		DecodePESHeader( pPayloadData, nDataLen );
		HandlePCR( pPacket );		// handle PCR

		PBYTE pPESPayloadData = pPESHeader->GetPayloadData( m_nPayloadLen );
		ASSERT( m_nPayloadLen >= 0 && pPESPayloadData );
		if( m_nPayloadLen < 0 || NULL == pPESPayloadData )
		{			//  CYJ,2005-7-1 ��ֹ��������
#ifdef __ENABLE_TRACE__
			TRACE("No a PES packet. Payload len < 0\n");
#endif //__ENABLE_TRACE__
			return;
		}


		if( OUTPUT_METHOD_AS_ELEMENTARY_STREAM == m_nOutputMethod )
		{		//	���ΪԪ��������Ҫȥ�� PES ͷ
			pPayloadData = pPESPayloadData;
			ASSERT( pPayloadData );

			nDataLen = PBYTE(pPacket) + DVB_TS_PACKET_SIZE - pPayloadData;	//	��Ҫ�Լ�����

			if( NULL == pPayloadData || nDataLen <= 0 )
			{
				m_nPayloadLen = 0;
#ifdef __ENABLE_TRACE__
			//	TRACE("No payload data.\n");
#endif //__ENABLE_TRACE__
				return;								//	û��Ԫ����
			}
		}
		else
		{
			m_nPayloadLen = 0;						//	������ʽ������ Payload data len ����
			if( OUTPUT_METHOD_AS_PROGRAM_STREAM == m_nOutputMethod )
				ConstructPSPackHeader();				//	���PSͷ
		}

		nByteHasAllocated = GetMemoryAllocated();	//	���»�ȡ�Ѿ�д����ֽ���
	}
	else
	{
		ASSERT( m_bHeaderReceived );
		HandlePCR( pPacket );
	}

	if( m_pPCRResponser )				//	��������PES������
		m_pPCRResponser->PushOneTSPacket( pPacket );

	if( m_bTSPacketDiscontinuity )		// �Ѿ�ָ��TS���鲻������
		m_byExpectTSContinuityCounter = byContinuityCounter;

	if( pPacket->IsError() || (false==bIsPayloadStart && m_byExpectTSContinuityCounter != byContinuityCounter) )
	{
	#ifdef __ENABLE_TRACE__
		TRACE("Expect=%d != %d\n", m_byExpectTSContinuityCounter, byContinuityCounter );
	#endif
		m_wErrorCounter ++;		//	������ֻ�Ǳ�ǣ���������
	}

	//  CYJ,2009-6-23 �޸ģ�GetHeapSize() ==> GetHeapMaxSize()
	if( (!m_bRequireCompletePacket) ||\
		( (0==m_wPacketLength) && ( (nByteHasAllocated+nDataLen) >= GetHeapMaxSize() ) ) )
	{		//	��Ҫ�����������PES����������Ϊ0���һ������������
		if( m_nPayloadLen > 0 )
		{							//	��Ч�ĸ��ش�С���ж��Ƿ�Խ��
			int nByteLeft = m_nPayloadLen - m_dwPESBytesReceived;
			if( nByteHasAllocated > nByteLeft )
				nByteHasAllocated = nByteLeft;		//	ȥ������Ĳ���
		}
		if( nByteHasAllocated > 0 )					//  CYJ,2006-3-1 ��ֹ��������
		{
			int nByteLeftInHeap = GetHeapSize() - nByteHasAllocated;
			if( nByteLeftInHeap > 16 )
				nByteLeftInHeap = 16;
			if( nByteLeftInHeap > 0 )
				memset( GetHeapBuf() + nByteHasAllocated, 0xFF, nByteLeftInHeap );		// ĩ����� 0XFF����ֹ����
			OnPESReceived( GetHeapBuf(), nByteHasAllocated, m_dwPESBytesReceived, m_wErrorCounter );	//	֪ͨ���յ���
			m_dwPESBytesReceived += nByteHasAllocated;
		}
		else
			m_dwPESBytesReceived = 0;				// ����

		CMyHeap::Reset();		//	�������
	}

	m_byExpectTSContinuityCounter = (byContinuityCounter+1) & 0xF;
	if( false == Write( pPayloadData, nDataLen ) )
	{
		Reset();						//	���������¿�ʼ
		m_bHeaderReceived = false;		//	д������ʧ�ܣ���������
	}
}

///-------------------------------------------------------
/// CYJ,2005-2-25
/// ��������:
///		����TS������������ֶ�
/// �������:
///		pTSPacket			TS ����
/// ���ز���:
///		��
void CPESPacket::DecodeTSAdaptionField( PDVB_TS_PACKET pTSPacket )
{
	if( !(pTSPacket->m_abyData[3] & 0x20) )
		return;			//	û�� adaption field
	m_bRandomAccessPoint = (pTSPacket->m_abyData[5] & 0x40) ? true : false;
}

///--------------------------------------------------------------
///	CYJ, 2005-1-13
///	��������:
///		��ӡ
///	�������:
///		��
///	���ز���:
///		��
void CPESPacket::Dump(FILE*fOutput)
{
#ifdef _DEBUG
	PDVB_PES_PACKET_HEADER pPES = (PDVB_PES_PACKET_HEADER)GetHeapBuf();
	if( NULL == fOutput )
		fOutput = stderr;
	fprintf( fOutput, "Dumping PES. -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n" );
	fprintf( fOutput, "StartPreFix=%02X,%02X,%02X, StreamID=%2X, PacketLen=%d.\n",
		pPES->m_abyStartCodePrefix[0], pPES->m_abyStartCodePrefix[1],
		pPES->m_abyStartCodePrefix[2], pPES->m_byStreamID,
		pPES->GetPacketLength() );
	int nLen;
	PBYTE pPayloadPtr = pPES->GetPayloadData( nLen );
	fprintf( fOutput, "Payload Buffer=%p, nLen=%d\n", pPayloadPtr, nLen );
	if( pPayloadPtr )
	{
		fprintf( fOutput, "   Payload Data: %02X %02X %02X %02X %02X %02X\n",
			pPayloadPtr[0], pPayloadPtr[1], pPayloadPtr[2], pPayloadPtr[3],
			pPayloadPtr[4], pPayloadPtr[5] );
	}
#else
	(void)fOutput;
#endif // _DEBUG
}

///-------------------------------------------------------
/// CYJ,2005-1-14
/// ��������:
///		�����Ƿ���Program Stream��ʽ���
/// �������:
///		nMethod				�����ʽ
/// ���ز���:
///		ԭ��������
///	˵����
///		1����Program Stream��ʽ��������������ʼ��ʱ����Ҫ���һ��PSͷ������system_header)��PES����
///		   �Ժ󣬵�PES������� ESCR �ֶ�ʱ�����һPSͷ����system_header)��PES����
///		2��ֻ���PES���飬��Ԫ����ǰ�滹��PESͷ
///		3��Ԫ������������PSͷ��Ҳ������PESͷ
int CPESPacket::SetOutputMethod( int nMethod )
{
	int nRetVal = m_nOutputMethod;
	m_nOutputMethod = nMethod;

	Reset();

	if( OUTPUT_METHOD_AS_PROGRAM_STREAM == m_nOutputMethod )
		ConstructPSPackHeader( true );

	return nRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-1-14
/// ��������:
///		�����Ƿ�Ҫ�����������PES����
/// �������:
///		bComplete			ȱʡΪ true
/// ���ز���:
///		ԭ��������
///	˵����
///		��Ҫ�����������PES���飬��ʾ������յ�������PES����󣬲ŵ����¼����ύ���յ���PES����
///		��֮��ֻҪ���յ�TS���飬�͵����¼���������鲿�����ݡ�
bool CPESPacket::SetRequireCompletePESPacket(bool bComplete)
{
	bool bRetVal = m_bRequireCompletePacket;

	m_bRequireCompletePacket = bComplete;
	Reset();

	return bRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-1-14
/// ��������:
///		���� PS Pack header�����������������
/// �������:
///		��
/// ���ز���:
///		��
void CPESPacket::ConstructPSPackHeader(bool bOutputSystemHeader)
{
	PBYTE pBuf = Allocate(14);			// �̶���С
	CMyBitStream OutputBs( pBuf, 14 );

	OutputBs.PutBits32( 0x1BA );		// pack_start_code, 0x1BA
	OutputBs.PutBits( 1, 2 );			// 01
	OutputBs.PutBits( m_wPS_SCR_Base_30_32, 3 );
	OutputBs.PutBit( 1 );				// market

	OutputBs.PutBits( m_wPS_SCR_Base_15_29, 15 );
	OutputBs.PutBit( 1 );				// market bit

	OutputBs.PutBits( m_wPS_SCR_Base_0_14, 15 );
	OutputBs.PutBit( 1 );

	OutputBs.PutBits( m_wPS_SCR_Extension, 9 );
	OutputBs.PutBit( 1 );

	OutputBs.PutBits( m_dwProgramMuxRate, 22 );
	OutputBs.PutBits( 3, 2 );			// 2 market bits
	OutputBs.PutBits( 0, 5 );			// reserved 5
	OutputBs.PutBits( 0, 3 );			// packet stuffing_length

	ASSERT( OutputBs.GetTotalWriteBits() == 14*8 );

	OutputBs.FinishWrite();

	ASSERT( OutputBs.GetTotalWriteBits() == 14*8 );

	if( false == bOutputSystemHeader )
		return;

	static BYTE abySystemHeader[]=
	{
		0x0,	0x0,	0x01,	0xbb,	0x00,	0x0c,	0x80,	0x7F,
		0x5d,	0x04,	0x21,	0x7f,	0xE0,	0xE0,	0xE0,	0xC0,
		0xC0,	0x20,	0x00,	0x00,	0x01,	0xbe,	0x07,	0xDA
	};
	Write( abySystemHeader, sizeof(abySystemHeader) );
}

///-------------------------------------------------------
/// CYJ,2005-1-19
/// ��������:
///		����PESͷ
/// �������:
///		pBuf				��������ַ
///		nLen				���ݳ���
/// ���ز���:
///		��
void CPESPacket::DecodePESHeader(PBYTE pBuf, int nLen)
{
	ASSERT( !m_bHasPTS && !m_bHasDTS && pBuf && nLen > 6 );

	PDVB_PES_PACKET_HEADER pPESHeader = (PDVB_PES_PACKET_HEADER)pBuf;
	if( !pPESHeader->IsPESPacket() )
		return;
	if( pPESHeader->GetProcessMode() != PES_PROCESS_MODE_AV_STREAM_COMPATIBLE || nLen <= 6 )
		return;

	m_byStreamID = pPESHeader->m_byStreamID;
	m_wPacketLength = pPESHeader->GetPacketLength();

	pBuf += 6;				//	���� PES ͷ��6���ֽ�
	CMyBitStream InBs( pBuf, nLen-6 );
	if( InBs.getbits(2) != 2 )
		return;
	InBs.getbits( 6 );		//	���� scrambling_control,priority, data alignment, copyright, original_or_copy
	if( InBs.getbits(1) )
	{						//	������PTS
		m_bHasPTS = true;
		m_bHasDTS = (InBs.getbits(1) == 1);
	}
	else
		InBs.getbits(1);	// ���������һ�����ñ���

	bool bIsSCR = ( InBs.getbits(1) == 1 );
	m_bHasESMuxRate = ( InBs.getbits(1) == 1 );

	InBs.getbits(12);		//	����������12����

	if( m_bHasPTS )
	{
		InBs.getbits(4);
		m_PTS = InBs.getbits(3);
		m_PTS <<= 30;
		InBs.getbits(1);	// marker bit
		m_PTS |= (InBs.getbits(15)<<15);
		InBs.getbits(1);	// marker bit
		m_PTS |= InBs.getbits(15);
		InBs.getbits(1);	// marker bit
	}
	else
		m_PTS = 0;

	if( m_bHasDTS )
	{
		ASSERT( m_bHasPTS );
		InBs.getbits(4);
		m_DTS = InBs.getbits(3);
		m_DTS <<= 30;
		InBs.getbits(1);	// marker bit
		m_DTS |= (InBs.getbits(15)<<15);
		InBs.getbits(1);	// marker bit
		m_DTS |= InBs.getbits(15);
		InBs.getbits(1);	// marker bit
	}
	else
		m_DTS = 0;

	if( bIsSCR )
	{
		m_bHasSCR = true;
		InBs.getbits(2);
		m_wPS_SCR_Base_30_32 = InBs.getbits(3);
		m_PCR = m_wPS_SCR_Base_30_32;
		m_PCR <<= 18;
		InBs.getbits(1);	// marker bit
		m_wPS_SCR_Base_15_29 = InBs.getbits(15);
		m_PCR |= m_wPS_SCR_Base_15_29;
		m_PCR <<= 15;
		InBs.getbits(1);	// marker bit
		m_wPS_SCR_Base_0_14 = InBs.getbits(15);
		m_PCR |= m_wPS_SCR_Base_0_14;
		InBs.getbits(1);	// marker bit
		m_wPS_SCR_Extension = InBs.getbits(9);
		InBs.getbits(1);	// marker bit
	}

	if( m_bHasESMuxRate )
	{
		InBs.getbits(1);	// marker bit
		m_dwProgramMuxRate = InBs.getbits(22);	// marker bit
		InBs.getbits(1);	// marker bit
	}
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// ��������:
///		���� TS ���е�PCR����
/// �������:
///		pTSPacket		�������TS����
/// ���ز���:
///		true			�� PCR
///		false			�� PCR
bool CPESPacket::HandlePCR(PDVB_TS_PACKET pTSPacket)
{
	if( !( (pTSPacket->m_abyData[3] & 0x20) && (pTSPacket->m_abyData[5] & 0x10 )
		&& ( pTSPacket->m_abyData[4] >= 7 ) ) )
	{
		return false;			//	û�� adaption field����û��PCR�ֶ�
	}


	m_PCR = ( (DWORD)(pTSPacket->m_abyData[6]) << 24) |
			( (DWORD)(pTSPacket->m_abyData[7]) << 16) |
			( (DWORD)(pTSPacket->m_abyData[8]) <<  8) |
			( (DWORD)(pTSPacket->m_abyData[9]) );
	m_PCR <<= 1;
	m_PCR |= ( (pTSPacket->m_abyData[10]>>7)&1 );

	m_wPS_SCR_Base_0_14 = WORD(m_PCR >> 30);
	m_wPS_SCR_Base_15_29 = WORD( ( m_PCR >> 15) & 0x7FFF );
	m_wPS_SCR_Base_30_32 = WORD( m_PCR & 0x7FFF );

	m_wPS_SCR_Extension = pTSPacket->m_abyData[11] | ( (pTSPacket->m_abyData[10]&1) << 8 );

	m_bHasSCR = true;

	return true;			//	û�� adaption field
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// ��������:
///		��ȡ PCR
/// �������:
///		scr				���
///		wExtension		��չ���
/// ���ز���:
///		true			�ɹ�
///		false			û�� scr
bool CPESPacket::GetPCR(MY_LONG64 & scr, WORD & wExtension)
{
	if( m_bHasSCR )
	{
		scr = m_PCR;
		wExtension = m_wPS_SCR_Extension;
	}
	return m_bHasSCR;
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// ��������:
///		���� PCR
/// �������:
///		scr					SCR
///		wExtension			��չʱ��
/// ���ز���:
///		��
void CPESPacket::SetPCR(MY_LONG64 & scr, WORD wExtension )
{
	m_bHasSCR = true;
	m_PCR = scr;
	m_wPS_SCR_Extension = wExtension;

	m_wPS_SCR_Base_0_14 = WORD(m_PCR >> 30);
	m_wPS_SCR_Base_15_29 = WORD( ( m_PCR >> 15) & 0x7FFF );
	m_wPS_SCR_Base_30_32 = WORD( m_PCR & 0x7FFF );
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// ��������:
///		��ȡ�������PCR������
/// �������:
///		��
/// ���ز���:
///		PCR������
/// ˵����
///		�� PCR_PID = ��PES's PID ʱ����Ҫ�øö���ȥ����������PES������
CPCR_TSPacketResponser * CPESPacket::GetAssociatedPCRResponser()
{
	return m_pPCRResponser;
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// ��������:
///		���� PCR ����������
/// �������:
///		pResponser		PCR ������
/// ���ز���:
///		ԭ���Ĵ���������ָ��
/// ˵����
///		�� PCR_PID = ��PES's PID ʱ����Ҫ�øö���ȥ����������PES������
CPCR_TSPacketResponser * CPESPacket::SetAssociatedPCRResponser( CPCR_TSPacketResponser * pResponser)
{
	CPCR_TSPacketResponser * pRetVal = m_pPCRResponser;
	m_pPCRResponser = pResponser;

	if( m_pPCRResponser )
		m_pPCRResponser->Remove( this );		//	ɾ���Լ�

	return pRetVal;
}

///-------------------------------------------------------
/// CYJ,2006-3-2
/// ��������:
///		��ȡPESͷ���е�CRC�ֶ�
/// �������:
///		pBuf			PES ͷ��
///		nLen			����
/// ���ز���:
///		NULL				ʧ��
///		����				��16λΪCRC16
PBYTE CPESPacket::GetCRC(PBYTE pBuf, int nLen) const
{
	PDVB_PES_PACKET_HEADER pPESHeader = (PDVB_PES_PACKET_HEADER)pBuf;
	if( !pPESHeader->IsPESPacket() )
		return NULL;
	if( pPESHeader->GetProcessMode() != PES_PROCESS_MODE_AV_STREAM_COMPATIBLE || nLen <= 6 )
		return NULL;
	//	���� PES ͷ��6���ֽ�
	pBuf += 6;
	nLen -= 6;
	if( (pBuf[0] & 0xC0) != 0x80 )
		return NULL;			// not '10'
	if( (pBuf[1] & 2) == 0 )
		return NULL;			// not CRC

	CMyBitStream InBs( pBuf, nLen );
	InBs.getbits(8);

	pBuf += 3;					// ͷ��3���ֽ�

	switch( InBs.getbits(2) )
	{							// PTS_DTS
	case 2:						// '10'
		pBuf += 5;
		break;
	case 3:						// '11'
		pBuf += 10;
		break;
	}

	if( InBs.getbits(1) )		// ESCR
		pBuf += 6;
	if( InBs.getbits(1) )		// ES Rate
		pBuf += 3;
	if( InBs.getbits(1) )		// DSM
		pBuf ++;
	if( InBs.getbits(1) )		// additional copy info
		pBuf ++;
	ASSERT( InBs.getbits(1) );
	return pBuf;
}

///-------------------------------------------------------
/// CYJ,2006-3-2
/// ��������:
///		����CRC���
/// �������:
///		bDoCheck			�Ƿ���м��
/// ���ز���:
///		true				����
///		false				���ܽ���crc��⣬��Ϊû��Ҫ���������
bool CPESPacket::DoCRCCheck(bool bDoCheck)
{
	if( false == m_bRequireCompletePacket )
		return false;
	m_bDoCRCCheck = bDoCheck;
	return true;
}
