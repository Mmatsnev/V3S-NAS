///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2005-2-8
///
///		��;��
///			DVB PSI ��������
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӡ��ڲ�ʹ��!	  			     !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	��Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!			 !
///=======================================================

// PATGenerator.cpp: implementation of the CPATGenerator class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "psitablegenerator.h"
#include "dvb_crc.h"
#include "bitstream.h"
#include "dvbdescriptorsdefine.h"
#include "dvbpsitablesdefine.h"

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		�����ݷ�װ��TS����
/// �������:
///		pBuf				table ������
///		nLen				table ��С
/// ���ز���:
///		>0					����ĸ���
///		<0					ʧ��
int CDVBPSI_TableGeneratorBase::Encapsulate( PBYTE pBuf, int nLen )
{
    ASSERT( pBuf && nLen > 0 );
    if( !pBuf || nLen <=0 )
        return -1;

    int nPointerField = 1;
    int nRetVal = 0;
    while( nLen > 0 )
    {
        CTSPacketEncapsulator * pPacket = GetTSPacket();
        if( NULL == pPacket )
            return -1;
        PBYTE pDstBuf = pPacket->m_abyData + 4;
        int nPayloadLen = 184;
        if( nPointerField )
        {							//	��һ�����飬��Ҫ�ṩһ��
            pPacket->SetPayloadStartIndicator();
            *pDstBuf = 0;			//  pointer field
            pDstBuf ++;
            nPayloadLen --;
            nPointerField = 0;
        }
        if( nPayloadLen > nLen )
        {
            memset(pDstBuf+nLen,0xFF, nPayloadLen-nLen);
            nPayloadLen = nLen;
        }
        memcpy( pDstBuf, pBuf, nPayloadLen );
        nLen -= nPayloadLen;
        pBuf += nPayloadLen;

        OnTSPacketReady( static_cast<PDVB_TS_PACKET>(pPacket) );
        nRetVal ++;
    }
    ASSERT( 0 == nLen );

    return nRetVal;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CDVBPSI_PATGenerator::CDVBPSI_PATGenerator()
{
    Preset();
}

CDVBPSI_PATGenerator::~CDVBPSI_PATGenerator()
{
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		Ԥ�Ʋ���
/// �������:
///		��
/// ���ز���:
///		��
void CDVBPSI_PATGenerator::Preset()
{	
    m_mapSID_PID.RemoveAll();
    m_wNetworkID = INVALID_PID;
    memset( m_abyPATHeader, 0, 16 );	//	��ͷ��8���ֽ�
    m_bModified = true;
    m_abyPATHeader[0] = 0;		// TABLE ID
    m_abyPATHeader[1] = 0x80;	// Ŀǰ�趨 Section length = 0
    m_abyPATHeader[3] = 0xFF;	// TS ID
    m_abyPATHeader[4] = 0xFF;
    m_wSectionLen = 4;			//	������4�ֽڵ�CRC32
    SetPID( 0 );
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		����PID
/// �������:
///		wSID		����PID
///		wPMTPID		��Ӧ��PMT_PID
/// ���ز���:
///		��
void CDVBPSI_PATGenerator::SetSID_PMT( WORD wSID, WORD wPMTPID )
{
    try
    {
        m_mapSID_PID[wSID] = wPMTPID;
        m_bModified = true;
    }
    catch( ... )
    {
    }
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		ɾ��һ����Ŀ
/// �������:
///		��
/// ���ز���:
///		��
void CDVBPSI_PATGenerator::RemoveSID_PMT( WORD wSID )
{
    m_mapSID_PID.RemoveKey( wSID );
    m_bModified = true;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		ɾ��ȫ����Ŀ
/// �������:
///		��
/// ���ز���:
///		��
void CDVBPSI_PATGenerator::RemoveAllSID_PMT()
{
    m_bModified = true;
    m_mapSID_PID.RemoveAll();
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		�������
/// �������:
///		bCurrentValid		�Ƿ�ǰ��Ч��ȱʡΪ true
/// ���ز���:
///		��
///	˵����
///		������TS���飬ͨ�� OnTSPacketReady �������
void CDVBPSI_PATGenerator::Build( bool bCurrentValid )
{
    BYTE byIndicator = bCurrentValid ? 1 : 0;
    if( (m_abyPATHeader[5]&1) ^ byIndicator )
        m_bModified = true;		//	current_next_indicator �ı�
    if( m_bModified )
    {							//	�����ı䣬��������
        m_wSectionLen = 5;		//	�̶�����
        PBYTE pBuf = m_abyPATHeader + 8;
        if( m_wNetworkID != INVALID_PID )
        {
            pBuf[0] = pBuf[1] = 0;		// SID = 0 ==> NetworkID
            pBuf[2] = BYTE(m_wNetworkID>>8);
            pBuf[3] = BYTE(m_wNetworkID);
            m_wSectionLen += 4;
            pBuf += 4;
        }
        if( !m_mapSID_PID.IsEmpty() )
        {
            POSITION pos = m_mapSID_PID.GetStartPosition();
            while( pos )
            {
                WORD wSID,wPMT_PID;
                m_mapSID_PID.GetNextAssoc( pos, wSID, wPMT_PID );
                pBuf[0] = BYTE(wSID>>8);
                pBuf[1] = BYTE(wSID);
                wPMT_PID &= 0x1FFF;
                pBuf[2] = BYTE(wPMT_PID>>8);
                pBuf[3] = BYTE(wPMT_PID);

                m_wSectionLen += 4;
                pBuf += 4;
            }
        }
        m_wSectionLen += 4;				// CRC32
        ASSERT( m_wSectionLen < PAT_BUFFER_SIZE-3 );
        m_wSectionLen &= 0x3FF;			//	ֻ�е�10������Ч
        m_abyPATHeader[1] &= 0xF0;
        m_abyPATHeader[1] |= BYTE(m_wSectionLen>>8);	//	���ô�С
        m_abyPATHeader[2] = BYTE(m_wSectionLen);

        BYTE byVersionAndIndicator = (GetVersionNumber()+1) & 0x1F;
        byVersionAndIndicator <<= 1;
        byIndicator &= 1;
        byVersionAndIndicator |= byIndicator;
        m_abyPATHeader[5] &= 0xC0;
        m_abyPATHeader[5] |= byVersionAndIndicator;

        DWORD dwCRC32 = DVB_GetCRC32( m_abyPATHeader, m_wSectionLen-4+3 );
        pBuf = m_abyPATHeader + m_wSectionLen + 3 -1; // ����3�ֽ�ͷ�������һ���ֽڿ�ʼ
        for(int i=0; i<4; i++ )
        {
            *pBuf -- = BYTE(dwCRC32);
            dwCRC32 >>= 8;
        }

        m_bModified = false;
    }

#ifdef _DEBUG
    WORD wLenTmp = 0;
    if( m_wNetworkID != INVALID_PID )
        wLenTmp += 4;
    wLenTmp += m_mapSID_PID.GetCount()*4;
    wLenTmp += 4;					// CRC32
    ASSERT( m_wSectionLen == 5 + wLenTmp );		//	�̶����� 5 �ֽ�
#endif //_DEBUG	

    Encapsulate( m_abyPATHeader, m_wSectionLen + 3 );
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		���ô�����ID
/// �������:
///		wTSID
/// ���ز���:
///		ԭ���Ĵ�����ID
WORD CDVBPSI_PATGenerator::SetTSID(WORD wTSID)
{
    m_bModified = true;
    WORD wRetVal = m_abyPATHeader[3] * 0x100 + m_abyPATHeader[4];

    m_abyPATHeader[3] = BYTE(wTSID >> 8);
    m_abyPATHeader[4] = BYTE(wTSID);

    return wRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		���õ�ǰ�����
/// �������:
///		��
/// ���ز���:
///		ԭ�������
BYTE CDVBPSI_PATGenerator::SetSectionNumber( BYTE bySectionNumber )
{
    m_bModified = true;
    BYTE byRetVal = m_abyPATHeader[6];
    m_abyPATHeader[6] = bySectionNumber;
    return byRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		��������ction_number
/// �������:
///		bySectionNumber			�������
/// ���ز���:
///		ԭ�������
BYTE CDVBPSI_PATGenerator::SetLastSectionNumber( BYTE bySectionNumber )
{
    m_bModified = true;
    BYTE byRetVal = m_abyPATHeader[7];
    m_abyPATHeader[7] = bySectionNumber;
    return byRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		��ȡ�汾��
/// �������:
///		��
/// ���ز���:
///		�汾��
BYTE CDVBPSI_PATGenerator::GetVersionNumber() const
{
    return (m_abyPATHeader[5]>>1) & 0x1F;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		���ð汾��
/// �������:
///		byNumber			�汾���
/// ���ز���:
///		ԭ���İ汾���
BYTE CDVBPSI_PATGenerator::SetVersionNumber( BYTE byNumber )
{
    m_bModified = true;
    BYTE byRetVal = m_abyPATHeader[5];
    byRetVal >>= 1;
    byRetVal &= 0x1F;

    m_abyPATHeader[5] &= 0xC1;		// 11000001

    byNumber --;					// �����ر���ʱ�����Զ���1���Ӷ��ﵽ�����İ汾
    byNumber &= 0x1F;
    byNumber <<= 1;
    m_abyPATHeader[5] |= byNumber;

    return byRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		��������ID
/// �������:
///		wNetworkdID			�µ�����ID
/// ���ز���:
///		ԭ��������ID
WORD CDVBPSI_PATGenerator::SetNetworkID( WORD wNetworkID )
{
    m_bModified = true;
    WORD wRetVal = m_wNetworkID;
    m_wNetworkID = wNetworkID & 0x1FFF;
    return wRetVal;
}

//////////////////////////////////////////////////////////////////////////
// PMT Generator
CDVBPSI_PMTGenerator::CDVBPSI_PMTGenerator()
{
    Preset();
}

CDVBPSI_PMTGenerator::~CDVBPSI_PMTGenerator()
{
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		������Ŀ
/// �������:
///		byStreamID			��ID
///		wPID				PES ��Ӧ��PID
///		pDescriptor			������
///		nLen				�����Ӵ�С
/// ���ز���:
///		��
void CDVBPSI_PMTGenerator::CreateStream( BYTE byStreamID, WORD wPID, PBYTE pDescriptor, int nLen )
{
    ONE_STREAM_INFO StreamInfo;
    StreamInfo.m_byStreamID = byStreamID;
    StreamInfo.m_wPID = wPID;
    try
    {
        StreamInfo.m_abyDescriptor.SetSize( nLen );
        memcpy( StreamInfo.m_abyDescriptor.GetData(), pDescriptor, nLen );
    }
    catch(...)
    {
        return;
    }

    int nNo = FindStream( byStreamID );
    if( nNo >= 0 )
        m_aStreams[nNo] = StreamInfo;
    else
        m_aStreams.Add( StreamInfo );
    m_bModified = true;
}

///-------------------------------------------------------
/// CYJ,2005-2-9
/// ��������:
///		������
/// �������:
///		byStreamID			�����ҵ���ID
/// ���ز���:
///		>=0					���
///		<0					ʧ��
int CDVBPSI_PMTGenerator::FindStream( BYTE byStreamID )
{
    int nCount = m_aStreams.GetSize();
    for(int i=0; i<nCount; i++ )
    {
        if( m_aStreams[i].m_byStreamID == byStreamID )
            return i;
    }
    return -1;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		������Ŀ������
/// �������:
///		pDescriptor				������
///		nLen					�����Ӵ�С
/// ���ز���:
///		��
void CDVBPSI_PMTGenerator::CreateProgramInfo(PBYTE pDescriptor, int nLen)
{
    try
    {
        m_ProgramInfo.SetSize( nLen );
        memcpy( m_ProgramInfo.GetData(), pDescriptor, nLen );
    }
    catch(...)
    {
        m_ProgramInfo.RemoveAll();
    }
    m_bModified = true;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		����ͨ��MPEG4����
/// �������:
///		wPID					������PID
///		Video					��Ƶ����
///		pAudio					��Ƶ������һ�㲻�ᣬ������ΪNULL
/// ���ز���:
///		��
///	˵����
///		ͬʱ������ͨ��������Ŀ������
void CDVBPSI_PMTGenerator::CreateTongshiMPEG4Bro( WORD wPID, PTSMPEG4_VIDEO_PARAM pVideo, PTSMPEG4_AUDIO_PARAM pAudio)
{
    m_bModified = true;

    static BYTE abyTS_ProgramCharacterDr[]={ 200, 6, 'T', 'S', 'V', 'B', 0, 0 };
    CreateProgramInfo( abyTS_ProgramCharacterDr, sizeof(abyTS_ProgramCharacterDr) );

    TSMPEG4_VIDEO_PARAM	VideoTmp;
    TSMPEG4_AUDIO_PARAM AudioTmp;

    int nLenNeed = 0;
    if( pVideo )
    {
        memcpy( &VideoTmp, pVideo, sizeof(VideoTmp) );
        VideoTmp.SwapByteOrder();
        pVideo = &VideoTmp;
        nLenNeed = 2 + sizeof(VideoTmp);		// 2 �ֽڷֱ�Ϊ tagID + len
    }
    if( pAudio )
    {
        memcpy( &AudioTmp, pAudio, sizeof(AudioTmp));
        AudioTmp.SwapByteOrder();
        pAudio = &AudioTmp;
        nLenNeed += 2 + sizeof(AudioTmp);	// 2 �ֽڷֱ�Ϊ tagID + len
    }
    AudioTmp.m_byExtersionLen = 0;
    AudioTmp.m_byExtersionLen = 0;

    try
    {
        m_aStreams.SetSize( 1 );
    }
    catch(...)
    {
        return;
    }

    ONE_STREAM_INFO & StreamInfo = m_aStreams[0];
    StreamInfo.m_byStreamID = 0xA0;				// ʹ����Ƶ����ID
    StreamInfo.m_wPID = wPID;
    try
    {
        StreamInfo.m_abyDescriptor.SetSize( nLenNeed );
    }
    catch(...)
    {
        RemoveAllStream();
        return;
    }

    PBYTE pBuf = StreamInfo.m_abyDescriptor.GetData();
    if( pVideo )
    {
        *pBuf++ = 201;			// Video
        *pBuf++ = sizeof(VideoTmp);
        memcpy( pBuf, pVideo, sizeof(VideoTmp));
        pBuf += sizeof(VideoTmp);
    }
    if( pAudio )
    {
        *pBuf ++ = 202;
        *pBuf ++ = sizeof(AudioTmp);
        memcpy( pBuf, pAudio, sizeof(AudioTmp) );
        pBuf += sizeof(AudioTmp);
    }
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		ɾ��һ����
/// �������:
///		byStreamID		��ɾ������ID
/// ���ز���:
///		��
void CDVBPSI_PMTGenerator::DeleteStream( BYTE byStreamID )
{
    int nNo = FindStream( byStreamID );
    if( nNo < 0 )
        return;
    m_aStreams.RemoveAt( nNo );
    m_bModified = true;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		ɾ��������
/// �������:
///		��
/// ���ز���:
///		��
void CDVBPSI_PMTGenerator::RemoveAllStream()
{
    m_aStreams.RemoveAll();
    m_bModified = true;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		ɾ����Ŀ��Ϣ����
/// �������:
///		��
/// ���ز���:
///		��
void CDVBPSI_PMTGenerator::RemoveProgramInfo()
{
    m_ProgramInfo.RemoveAll();
    m_bModified = true;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		Ԥ�Ʊ���
/// �������:
///		��
/// ���ز���:
///		��
void CDVBPSI_PMTGenerator::Preset()
{
    m_aStreams.RemoveAll();
    m_ProgramInfo.RemoveAll();
    memset( m_abyPMTBuf, 0, 15 );
    m_wSectionLen = 0;
    m_bModified = true;

    m_abyPMTBuf[0] = 2;
    m_abyPMTBuf[1] = 0x80;	// Ŀǰ�趨 Section length = 0
    m_abyPMTBuf[2] = 0;
    m_abyPMTBuf[3] = 0xFF;	// Ŀǰ�趨 Section length = 0
    m_abyPMTBuf[4] = 0xFF;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		���õ�ǰ�ĻỰ���
/// �������:
///		
/// ���ز���:
///		��
BYTE CDVBPSI_PMTGenerator::SetSectionNumber( BYTE bySectionNumber )
{	
    m_bModified = true;
    BYTE byRetVal = m_abyPMTBuf[6];
    m_abyPMTBuf[6] = bySectionNumber;
    return byRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		�������ĻỰ���
/// �������:
///		bySectionNumber			�������
/// ���ز���:
///		ԭ����������
BYTE CDVBPSI_PMTGenerator::SetLastSectionNumber( BYTE bySectionNumber )
{
    m_bModified = true;
    BYTE byRetVal = m_abyPMTBuf[7];
    m_abyPMTBuf[7] = bySectionNumber;
    return byRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		��ȡ�汾��
/// �������:
///		��
/// ���ز���:
///		��ǰ�İ汾��
BYTE CDVBPSI_PMTGenerator::GetVersionNumber() const
{
    return (m_abyPMTBuf[5]>>1) & 0x1F;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		���ð汾��
/// �������:
///		byNumber			�汾��0��31
/// ���ز���:
///		ԭ���İ汾��
BYTE CDVBPSI_PMTGenerator::SetVersionNumber( BYTE byNumber )
{
    m_bModified = true;
    BYTE byRetVal = m_abyPMTBuf[5];
    byRetVal >>= 1;
    byRetVal &= 0x1F;

    m_abyPMTBuf[5] &= 0xC1;		// 11000001

    byNumber --;					// �����ر���ʱ�����Զ���1���Ӷ��ﵽ�����İ汾
    byNumber &= 0x1F;
    byNumber <<= 1;
    m_abyPMTBuf[5] |= byNumber;

    return byRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		�����µ�SID
/// �������:
///		wSID			�µĽ�Ŀ��
/// ���ز���:
///		ԭ����SID
WORD CDVBPSI_PMTGenerator::SetSID(WORD wSID )
{
    m_bModified = true;
    WORD wRetVal = m_abyPMTBuf[3] * 0x100 + m_abyPMTBuf[4];

    m_abyPMTBuf[3] = BYTE(wSID >> 8);
    m_abyPMTBuf[4] = BYTE(wSID);

    return wRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		��ȡ��ǰ��SID
/// �������:
///		��
/// ���ز���:
///		��ǰ��SID
WORD CDVBPSI_PMTGenerator::GetSID() const
{
    return m_abyPMTBuf[3] * 0x100 + m_abyPMTBuf[4];
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		����PID
/// �������:
///		wPID			�µ�PID
/// ���ز���:
///		ԭ����PID
WORD CDVBPSI_PMTGenerator::SetPCR_PID( WORD wPID )
{
    wPID &= 0x1FFF;
    WORD wRetVal = (m_abyPMTBuf[8]&0x1F) * 0x100 + m_abyPMTBuf[9];

    m_abyPMTBuf[8] &= 0xE0;
    m_abyPMTBuf[8] |= BYTE(wPID>>8);
    m_abyPMTBuf[9] = BYTE(wPID);

    m_bModified = true;

    return wRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		����PCR��PID
/// �������:
///		��
/// ���ز���:
///		��ǰ��PID
WORD CDVBPSI_PMTGenerator::GetPCR_PID()const
{
    return (m_abyPMTBuf[8]&0x1F) * 0x100 + m_abyPMTBuf[9];
}	

///-------------------------------------------------------
/// CYJ,2005-2-8
/// ��������:
///		�������
/// �������:
///		bCurrentValid		�Ƿ�ǰ��Ч��ȱʡΪ true
/// ���ز���:
///		��
///	˵����
///		������TS���飬ͨ�� OnTSPacketReady �������
void CDVBPSI_PMTGenerator::Build( bool bCurrentValid )
{
    int i;
    BYTE byIndicator = bCurrentValid ? 1 : 0;
    if( (m_abyPMTBuf[5]&1) ^ byIndicator )
        m_bModified = true;		//	current_next_indicator �ı�
    if( m_bModified )
    {							// �����ı䣬��������
        m_wSectionLen = 9;		// �̶��򳤶� 9 �ֽ�
        PBYTE pBuf = m_abyPMTBuf + 12;
        int nLen = m_ProgramInfo.GetSize();
        nLen &= 0x3FF;
        m_abyPMTBuf[10] = BYTE(nLen>>10) & 3;
        m_abyPMTBuf[11] = BYTE(nLen);
        if( nLen )
        {					// program info
            memcpy( pBuf, m_ProgramInfo.GetData(), nLen );
            m_wSectionLen += nLen;
            pBuf += nLen;
        }
        int nCount = m_aStreams.GetSize();
        for(i=0; i<nCount; i++ )
        {
            ONE_STREAM_INFO & OneStream = m_aStreams[i];
            pBuf[0] = OneStream.m_byStreamID;
            OneStream.m_wPID &= 0x1FFF;
            pBuf[1] = BYTE( OneStream.m_wPID >> 8 );
            pBuf[2] = BYTE( OneStream.m_wPID );
            nLen = OneStream.m_abyDescriptor.GetSize();
            pBuf[3] = BYTE( nLen >> 8 ) & 3;
            pBuf[4] = BYTE( nLen );
            m_wSectionLen += nLen + 5;		// �̶��� 5 ���ֽ�
            memcpy( pBuf+5, OneStream.m_abyDescriptor.GetData(), nLen );
            pBuf += 5 + nLen;
        }
        m_wSectionLen += 4;				// CRC32
        ASSERT( m_wSectionLen < PMT_BUFFER_SIZE-3 );
        m_wSectionLen &= 0x3FF;			//	ֻ�е�10������Ч
        m_abyPMTBuf[1] &= 0xF0;
        m_abyPMTBuf[1] |= BYTE(m_wSectionLen>>8);	//	���ô�С
        m_abyPMTBuf[2] = BYTE(m_wSectionLen);

        BYTE byVersionAndIndicator = (GetVersionNumber()+1) & 0x1F;
        byVersionAndIndicator <<= 1;
        byIndicator &= 1;
        byVersionAndIndicator |= byIndicator;
        m_abyPMTBuf[5] &= 0xC0;
        m_abyPMTBuf[5] |= byVersionAndIndicator;

        DWORD dwCRC32 = DVB_GetCRC32( m_abyPMTBuf, m_wSectionLen-4+3 );
        pBuf = m_abyPMTBuf + m_wSectionLen + 3 -1; // ����3�ֽ�ͷ�������һ���ֽڿ�ʼ
        for(i=0; i<4; i++ )
        {
            *pBuf -- = BYTE(dwCRC32);
            dwCRC32 >>= 8;
        }

        m_bModified = false;
    }

#ifdef _DEBUG
    WORD wLenTmp = 0;
    wLenTmp += m_ProgramInfo.GetSize();
    int nCount = m_aStreams.GetSize();
    for(i=0; i<nCount; i++ )
    {
        wLenTmp += m_aStreams[i].m_abyDescriptor.GetSize() + 5;	// �̶��� 5 ���ֽ�
    }
    wLenTmp += 4;		// CRC32

    ASSERT( m_wSectionLen == 9 + wLenTmp );			// �̶��򳤶� 9 �ֽ�
#endif //_DEBUG	

    Encapsulate( m_abyPMTBuf, m_wSectionLen + 3 );	// ���� 3 �ֽڵİ�ͷ
}


///////////////////////////////////////////////////////////////////////
/// SDT, Service description table
CDVBPSI_SDTGenerator::CDVBPSI_SDTGenerator()
{	
    Preset();
}

CDVBPSI_SDTGenerator::~CDVBPSI_SDTGenerator()
{
}

///-------------------------------------------------------
/// CYJ,2005-4-18
/// ��������:
///		ɾ�����н�Ŀ
/// �������:
///		��
/// ���ز���:
///		��
void CDVBPSI_SDTGenerator::RemoveAll()
{
    m_aPrograms.RemoveAll();
    m_bModified = true;
}

///-------------------------------------------------------
/// CYJ,2005-4-18
/// ��������:
///		ɾ��һ����Ŀ
/// �������:
///		wSID		��ɾ����Ŀ��wSID
/// ���ز���:
///		��
void CDVBPSI_SDTGenerator::Remove( WORD wSID )
{
    int nNo = FindProgram( wSID );
    if( nNo >= 0 )
    {
        m_aPrograms.RemoveAt( nNo );
        m_bModified = true;
    }
}

///-------------------------------------------------------
/// CYJ,2005-4-18
/// ��������:
///		���ҽ�Ŀ
/// �������:
///		wSID			�����ҵ���Ŀ
/// ���ز���:
///		>=0				�����±�
///		<0				û�з���
int CDVBPSI_SDTGenerator::FindProgram( WORD wSID )
{
    int nCount = m_aPrograms.GetSize();
    for(int i=0; i<nCount; i++ )
    {
        if( m_aPrograms[i].m_wSID == wSID )
            return i;
    }
    return -1;
}

///-------------------------------------------------------
/// CYJ,2005-4-18
/// ��������:
///		���һ����Ŀ
/// �������:
///		Program			����ӵĽ�Ŀ
/// ���ز���:
///		��
void CDVBPSI_SDTGenerator::Add( ONE_SDT_ITEM & Program )
{
    m_bModified = true;
    int nNo = FindProgram( Program.m_wSID );
    if( nNo >= 0 )
    {
        m_aPrograms[nNo] = Program;
        return;
    }
    m_aPrograms.Add( Program );
}

///-------------------------------------------------------
/// CYJ,2005-4-18
/// ��������:
///		���û�������
/// �������:
///		Cfg				����������Ϣ
///		dwFlags			�ֶ���Ч��־���μ� ENABLE_XXXX
/// ���ز���:
///		��
void CDVBPSI_SDTGenerator::SetBaseCfg( SDT_BASE_CFG & Cfg, DWORD dwFlags )
{
    if( 0 == dwFlags )
        return;

    if( dwFlags & ENABLE_TSID )
        m_BaseCfg.m_wTSID = Cfg.m_wTSID;

    if( dwFlags & ENABLE_VERSION )
    {
        m_BaseCfg.m_byVersionNumber = Cfg.m_byVersionNumber - 1;
        m_BaseCfg.m_byVersionNumber &= 0x1F;
    }

    if( dwFlags & ENABLE_CURRENT_NEXT_INDICATOR )
        m_BaseCfg.m_bCurrentNextIndicator = Cfg.m_bCurrentNextIndicator;

    if( dwFlags & ENABLE_ORIGINAL_NETWORK_ID )
        m_BaseCfg.m_wOriginalNetworkID = Cfg.m_wOriginalNetworkID;

    m_bModified = true;
}

///-------------------------------------------------------
/// CYJ,2005-4-18
/// ��������:
///		�������
/// �������:
///		��
/// ���ز���:
///		��
///	ע��
///		������TS���飬ͨ�� OnTSPacketReady �������
void CDVBPSI_SDTGenerator::Build()
{
    int nCacheCount = m_aSDTBuf.GetSize();
    if( m_bModified || 0 == nCacheCount )
    {
        if( SplitSectionEdge() <= 0 )
            return;
        m_BaseCfg.m_byVersionNumber ++;
        m_BaseCfg.m_byVersionNumber &= 0x1F;

        m_byLastSectionNumber = m_aSectionEdge.GetSize() - 1;
        m_aSDTBuf.SetSize( m_aSectionEdge.GetSize() );

        //	���½��б���
        m_bModified = false;
        for( m_bySectionNumber=0; m_bySectionNumber<=m_byLastSectionNumber; m_bySectionNumber++ )
        {
            BuildOneSection( m_bySectionNumber );
        }

        nCacheCount = m_aSDTBuf.GetSize();
    }

    for(int i=0; i<nCacheCount; i++ )
    {
        SDT_BUFFER & BufItem = m_aSDTBuf[i];
        Encapsulate( BufItem.m_abyBuf, BufItem.m_wBufSize );
    }
}

///-------------------------------------------------------
/// CYJ,2005-4-18
/// ��������:
///		����һ����
/// �������:
///		nIndex					���
/// ���ز���:
///		��
void CDVBPSI_SDTGenerator::BuildOneSection( int nIndex )
{	
    SDT_BUFFER & CacheItem = m_aSDTBuf[nIndex];

    CMyBitStream outBS( CacheItem.m_abyBuf, sizeof(CacheItem.m_abyBuf) );

    outBS.PutBits8( DVBPSI_TBLID_SDT_ACTUAL );
    outBS.PutBit( 1 );			// syntax, always 1
    outBS.PutBits( 0, 3 );		// reserved_future_used and reserved

    outBS.PutBits( 0, 12 );		// section length, keep space
    outBS.PutBits16( m_BaseCfg.m_wTSID );
    outBS.PutBits( 0, 2 );		// reserved
    outBS.PutBits( m_BaseCfg.m_byVersionNumber, 5 );
    outBS.PutBit( m_BaseCfg.m_bCurrentNextIndicator );
    outBS.PutBits8( m_bySectionNumber );
    outBS.PutBits8( m_byLastSectionNumber );
    outBS.PutBits16( m_BaseCfg.m_wOriginalNetworkID );
    outBS.PutBits8( 0 );		// reserved future use

    DWORD dwStartNo = m_aSectionEdge[ nIndex ] & 0xFFFF;
    int nCount = m_aSectionEdge[ nIndex ]>>16;
    for(int i=0; i<nCount; i ++ )
    {
        ASSERT( (outBS.GetTotalWriteBits() & 7) == 0 );
        BuildOneProgram( &outBS, dwStartNo );
        dwStartNo ++;
    }
    ASSERT( 0 == (outBS.GetTotalWriteBits() & 7) );
    outBS.FinishWrite();			// д������
    ASSERT( 0 == (outBS.GetTotalWriteBits() & 7) );
    CacheItem.m_wBufSize = (WORD)outBS.GetTotalWriteBits() / 8;

    int nSectionLen = CacheItem.m_wBufSize + 4 - 3;			// ǰ��3�ֽڲ������ڣ�������4�ֽڵ�CRC32
    CacheItem.m_abyBuf[2] = nSectionLen & 0xFF;
    CacheItem.m_abyBuf[1] |= ( nSectionLen>>8 ) & 3;		// ֻ����10 bit���ȣ���1K

    DWORD dwCRC32 = DVB_GetCRC32( CacheItem.m_abyBuf, CacheItem.m_wBufSize );
    outBS.PutBits32( dwCRC32 );
    CacheItem.m_wBufSize += 4;

    outBS.FinishWrite();			// д������
    ASSERT( 0 == (outBS.GetTotalWriteBits() & 7) );
    ASSERT( CacheItem.m_wBufSize == outBS.GetTotalWriteBits() / 8 );
}

///-------------------------------------------------------
/// CYJ,2005-4-18
/// ��������:
///		ȷ��������ı߽�����
/// �������:
///		��
/// ���ز���:
///		��ĸ���
int  CDVBPSI_SDTGenerator::SplitSectionEdge()
{
    m_aSectionEdge.RemoveAll();
    int nCount = m_aPrograms.GetSize();
    int nByteUsed = 15;				// �̶���֧

    int nStartNo = 0;
    int nProgramCount = 0;
    for(int i=0; i<nCount; i ++ )
    {
        int nCurItemBytes = 5 + 5;	//	�̶���֧
        ONE_SDT_ITEM & Item = m_aPrograms[i];
        nCurItemBytes += Item.m_strProviderName.GetLength();
        nCurItemBytes += Item.m_strName.GetLength();

        nByteUsed += nCurItemBytes;

        if( nByteUsed > 1000 )			// һ�� SDT ���ܳ��� 1000 �ֽ�
        {								// �л�����һ���߽�����
            DWORD dwValue = nProgramCount*0x10000 + nStartNo;
            m_aSectionEdge.Add( dwValue );

            nStartNo = i;
            nProgramCount = 0;
            nByteUsed = 15 + nCurItemBytes;
        }
        nProgramCount ++;
    }

    if( nProgramCount )
    {
        DWORD dwValue = DWORD(nProgramCount*0x10000 + nStartNo);
        m_aSectionEdge.Add( dwValue );
    }

#ifdef _DEBUG_
    TRACE("SDT Encoder, %d Sectioins:\n", m_aSectionEdge.GetSize() );
    nCount = 0;
    for(i=0; i<m_aSectionEdge.GetSize(); i++ )
    {
        TRACE("StartNo=%d, %d Programs\n", m_aSectionEdge[i]&0xFFFF, m_aSectionEdge[i]>>16 );
        nCount += (m_aSectionEdge[i]>>16);
    }
    ASSERT( nCount == m_aPrograms.GetSize() );
#endif //_DEBUG

    return m_aSectionEdge.GetSize();
}

///-------------------------------------------------------
/// CYJ,2005-4-18
/// ��������:
///		�������һ����Ŀ
/// �������:
///		pOutBS			�����
///		nNo				��Ŀ���
/// ���ز���:
///		��
void CDVBPSI_SDTGenerator::BuildOneProgram( CMyBitStream * pOutBS, int nNo )
{
    ASSERT( pOutBS && nNo >= 0 && nNo < m_aPrograms.GetSize() );
    ONE_SDT_ITEM & Item = m_aPrograms[nNo];
    pOutBS->PutBits16( Item.m_wSID );
    pOutBS->PutBits( 0, 6 );		// reserved for future using
    pOutBS->PutBit( Item.m_bEITScheduleFlag );
    pOutBS->PutBit( Item.m_bEITPresentFollowingFlag );

    pOutBS->PutBits( Item.m_byRunningStatus, 3 );
    pOutBS->PutBit( Item.m_bFreeCAMode );

    // Output total len
    WORD wProviderLen = Item.m_strProviderName.GetLength();
    WORD wNameLen = Item.m_strName.GetLength();
    pOutBS->PutBits( wProviderLen + wNameLen + 5, 12 );

    // Output service description
    pOutBS->PutBits8( DVBPSI_DTID_SERVICE_DESCRIPTOR );
    pOutBS->PutBits8( 3 + wProviderLen + wNameLen );
    pOutBS->PutBits8( 1 );	// 1 digitial television service
    pOutBS->PutBits8( (BYTE)wProviderLen );
    int i;
    for( i=0; i<wProviderLen; i++ )
    {
        pOutBS->PutBits8( BYTE(Item.m_strProviderName[i]) );
    }
    pOutBS->PutBits8( (BYTE)wNameLen );
    for(i=0; i<wNameLen; i++)
    {
        pOutBS->PutBits8( BYTE(Item.m_strName[i]) );
    }
}

///-------------------------------------------------------
/// CYJ,2005-4-18
/// ��������:
///		Ԥ�����в���
/// �������:
///		��
/// ���ز���:
///		��
void CDVBPSI_SDTGenerator::Preset()
{
    memset( &m_BaseCfg, 0, sizeof(m_BaseCfg) );
    m_bModified = false;
    m_aPrograms.RemoveAll();
    m_aSDTBuf.RemoveAll();
    m_aSectionEdge.RemoveAll();
    m_bySectionNumber = 0;
    m_byLastSectionNumber = 0;

    SetPID( 0x11 );			// PID for SDT and BAT
}

