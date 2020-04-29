///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2006-9-21
///
///		��;��
///			IP Over DVB MPE decoder
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӡ��ڲ�ʹ��!	  			     !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	��Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!			 !
///=======================================================

// DVBPSITable_IP_MPE.cpp: implementation of the CDVBPSITable_IP_MPE class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "dvbpsitable_ip_mpe.h"
#include "dvbpsitablesdefine.h"
#include "dvb_crc.h"

#if defined(_DEBUG) && defined(_WIN32)
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef _DEBUG
#define __ENABLE_TRACE__
#endif //_DEBUG


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDVBPSITable_IP_MPE::CDVBPSITable_IP_MPE()
    :CDVBPSITablesBase(MPE_TABLE_MAX_SIZE),
      m_EthFrame( MPE_TABLE_MAX_SIZE )
{
    m_nDataHeaderLen = 20;		// Ŀǰֻ���� IP Э���װ��һ��IPͷ����20�ֽ�
}

CDVBPSITable_IP_MPE::~CDVBPSITable_IP_MPE()
{

}

///-------------------------------------------------------
/// CYJ,2006-9-21
/// ��������:
///		���յ�һ���ӱ�
/// �������:
///		��
/// ���ز���:
///		��
void CDVBPSITable_IP_MPE::OnTableReceived()
{
    int nByteReceived = GetMemoryAllocated();
    ASSERT( nByteReceived );
    PBYTE pSrcBuf = GetHeapBuf();
    ASSERT( pSrcBuf );
    if( NULL == pSrcBuf || nByteReceived <= 12 )	// һ��PAT��������Ҫ12�ֽ�
        return;

    ASSERT( *pSrcBuf == DVBPSI_TBLID_DATABROCAST_MPE );	// 0x3E
    if( *pSrcBuf != DVBPSI_TBLID_DATABROCAST_MPE )
    {
#ifdef __ENABLE_TRACE__
        TRACE("CDVBPSITable_IP_MPE: Not a MPE private data table, tableId=%d != 0\n", *pSrcBuf );
#endif // __ENABLE_TRACE__
        return;						//	���� MPE private data ��
    }
    WORD wSectionLen = ((pSrcBuf[1]&0xF) << 8) | pSrcBuf[2];
    if( nByteReceived < wSectionLen+3 )
    {
#ifdef __ENABLE_TRACE__
        TRACE("CDVBPSITable_IP_MPE: Bytes received %d < %d needed.\n", nByteReceived, wSectionLen );
#endif // __ENABLE_TRACE__
        return;
    }

    if( 0x30 != (pSrcBuf[1] & 0x30 ) )
    {		// reserved, should be '11'
#ifdef __ENABLE_TRACE__
        TRACE("CDVBPSITable_IP_MPE: section_syntax_indicator and next should be 2,but actual=%d\n", (pSrcBuf[1] & 0xC0 ) >> 6 );
#endif // __ENABLE_TRACE__
        return;
    }

    if( ( pSrcBuf[5] & 1 ) == 0 )
    {
#ifdef __ENABLE_TRACE__
        TRACE("CDVBPSITable_IP_MPE, current_next_indicator should be 1.\n");
#endif //__ENABLE_TRACE__
        return;
    }

    unsigned char bSectionSyntaxIndicator = ( pSrcBuf[1] & 0x80 );
    //	unsigned char bPrivateIndicator = ( pSrcBuf[1] & 0x40 );
    //	unsigned char byPayloadScramblingCtrl = ( pSrcBuf[5] >> 4 ) & 3;
    //	unsigned char byAddrScramblingCtrl = (pSrcBuf[5] >> 2 ) & 3;
    unsigned char bLLC_SNAP_Flag = (pSrcBuf[5] & 2);

    if( bLLC_SNAP_Flag )
    {
#ifdef __ENABLE_TRACE__
        TRACE("IP_MPE: This is a LLC_SNAP protocol, abort\n");
#endif  // __ENABLE_TRACE__
        return;
    }

    if( bSectionSyntaxIndicator )
    {
        if( DVB_GetCRC32( pSrcBuf, wSectionLen+3 ) )
        {							//	����CRC32���ڣ������ṹ��С��wSectionLen��3��
#ifdef __ENABLE_TRACE__
            TRACE("CDVBPSITable_IP_MPE: Bad CRC32\n");
#endif // __ENABLE_TRACE__
            return;
        }
    }
    else
    {					// check sum
        // not implement
    }

    m_abyEthAddr[5] = pSrcBuf[3];
    m_abyEthAddr[4] = pSrcBuf[4];
    for(int i=0; i<4; i++)
    {
        m_abyEthAddr[i] = pSrcBuf[11-i];
    }

    PBYTE pDataPtr = pSrcBuf + 12;
    WORD wDataLen = wSectionLen - 9 - 4;	// 4 bytes CRC32 or checksum, but 3 bytes header(table_id and senction len)

    if( 0 == pSrcBuf[6] )		// only one section
        OnEthernetFrame( m_abyEthAddr, pDataPtr, wDataLen );
    else
    {					// multiple section
        // δ�������ԣ�δ֪��ȷ��񣿹ʣ���һ�� ASSERT( FALSE );
        ASSERT( FALSE );
        if( 0 == m_EthFrame.GetMemoryAllocated() )
            SetSectionCount( pSrcBuf[7]+1 );	// ����section����
        SetSectionNoStatus( pSrcBuf[6] );
        if( false == m_EthFrame.Write( pDataPtr, wDataLen ) )
            Reset( true );			// write data failed
        if( m_EthFrame.GetMemoryAllocated() && IsAllSectionReceived() && m_EthFrame.GetHeapBuf() )
            OnEthernetFrame( m_abyEthAddr, m_EthFrame.GetHeapBuf(), m_EthFrame.GetMemoryAllocated() );
    }

    Reset( true );
}

///-------------------------------------------------------
/// CYJ,2006-9-21
/// ��������:
///		���յ�һ��IP����
/// �������:
///		pEthernetAddr					Ŀ����̫����ַ
///		pIPPacket						IP ���ݰ�
///		nIPLen							IP ����
/// ���ز���:
///		��
void CDVBPSITable_IP_MPE::OnEthernetFrame( PBYTE pEthernetAddr, PBYTE pIPPacket, int nIPLen )
{
#ifdef __ENABLE_TRACE__
    TRACE( "IP over DVB, MAC:%02X-%02X-%02X-%02X-%02X-%02X, DstIP=%d.%d.%d.%d, %d Bytes\n",
           pEthernetAddr[0], pEthernetAddr[1], pEthernetAddr[2],
            pEthernetAddr[3], pEthernetAddr[4], pEthernetAddr[5],
            pIPPacket[16], pIPPacket[17], pIPPacket[18], pIPPacket[19],
            nIPLen );
#endif // __ENABLE_TRACE__
}


void CDVBPSITable_IP_MPE::Dump(FILE*fOutput)
{
#ifdef _DEBUG
    if( NULL == fOutput )
        fOutput = stderr;
    PBYTE pEthernetAddr = m_abyEthAddr;
    PBYTE pIPPacket = m_EthFrame.GetHeapBuf() + 12;
    int nIPLen = m_EthFrame.GetMemoryAllocated() - 12 - 4;
    if( NULL == pIPPacket || nIPLen < 20 )
    {
        fprintf( fOutput, "CDVBPSITable_IP_MPE::Dump, error data.\n" );
        return;
    }
    fprintf( fOutput, "IP over DVB, MAC:%02X-%02X-%02X-%02X-%02X-%02X\n",
             pEthernetAddr[0], pEthernetAddr[1], pEthernetAddr[2],
            pEthernetAddr[3], pEthernetAddr[4], pEthernetAddr[5] );
#else
    (void)fOutput;
#endif //_DEBUG
}

///-------------------------------------------------------
/// CYJ,2006-9-21
/// ��������:
///		��ȡ�ϸ���
/// �������:
///		��
/// ���ز���:
///		��
PDVB_PSI_TABLE_BASE CDVBPSITable_IP_MPE::GetTableLastReceived()
{
    if( m_EthFrame.GetMemoryAllocated() <= m_nDataHeaderLen )	// һ��IP�������ٰ���20�ֽڵ�IPͷ
        return NULL;			// û���յ�
    return (PDVB_PSI_TABLE_BASE)m_EthFrame.GetHeapBuf();
}

///-------------------------------------------------------
/// CYJ,2006-9-21
/// ��������:
///		��λ
/// �������:
///		bForce				�Ƿ�ǿ�Ƹ�λ��ȱʡΪtrue
/// ���ز���:
///		��
void CDVBPSITable_IP_MPE::Reset(bool bForce)
{
    CDVBPSITablesBase::Reset(bForce);
    if( bForce )
        m_EthFrame.Reset();
}

///-------------------------------------------------------
/// CYJ,2006-9-21
/// ��������:
///		�Ƿ���Ч
/// �������:
///		��
/// ���ز���:
///		true			��Ч
///		false			ʧ��
bool CDVBPSITable_IP_MPE::IsValid()
{
    if( false == CDVBPSITablesBase::IsValid() )
        return false;
    return m_EthFrame.IsValid();
}

///-------------------------------------------------------
/// CYJ,2006-9-21
/// ��������:
///		���ݰ��Ƿ�����
/// �������:
///		��
/// ���ز���:
///		true			��������
///		false			δ����
bool CDVBPSITable_IP_MPE::IsPSIPacketIntegral()
{
    int nByteReceived = GetMemoryAllocated();
    ASSERT( nByteReceived );
    PBYTE pSrcBuf = GetHeapBuf();
    ASSERT( pSrcBuf );
    if( NULL == pSrcBuf || nByteReceived <= 12+m_nDataHeaderLen )	// һ��MPE��������Ҫ12�ֽڣ��ټ�20�ֽڵ�IPͷ
        return false;
    WORD wSectionLen = ((pSrcBuf[1]&0xf) << 8) | pSrcBuf[2];
    return( nByteReceived >= wSectionLen+3 );
}


