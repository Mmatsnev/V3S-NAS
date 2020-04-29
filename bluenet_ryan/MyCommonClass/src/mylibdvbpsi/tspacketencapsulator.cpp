///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2005-2-7
///
///		��;��
///			��װ�ԣӷ���
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӡ��ڲ�ʹ��!	  			     !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	��Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!			 !
///=======================================================

#include "stdafx.h"
#include "tspacketencapsulator.h"
#include "bitstream.h"

///////////////////////////////////////////////////////////////////////
//	CTSAdaptionFieldEncapsulate
///////////////////////////////////////////////////////////////////////
CTSAdaptionFieldEncapsulate::CTSAdaptionFieldEncapsulate()
{
    Preset();
    memset(m_abyDataBuf, 0, sizeof(m_abyDataBuf) );
}

///-------------------------------------------------------
/// CYJ,2005-2-23
/// ��������:
///		����PCR
/// �������:
///		llPCR			PCR
///		wExtension		��չ
/// ���ز���:
///		��
void CTSAdaptionFieldEncapsulate::SetPCR( LONGLONG llPCR, WORD wExtension )
{
    m_bPCR_Flag = true;
    m_llPCR = llPCR;
    m_wPCR_Extension = wExtension;
}

///-------------------------------------------------------
/// CYJ,2005-2-23
/// ��������:
///		����OPCR
/// �������:
///		llOPCR			OPCR
///		wExtension		��չ
/// ���ز���:
///		��
void CTSAdaptionFieldEncapsulate::SetOPCR( LONGLONG llPCR, WORD wExtension )
{
    m_bOPCR_Flag = true;
    m_llOPCR = llPCR;
    m_wOPCR_Extension = wExtension;
}

///-------------------------------------------------------
/// CYJ,2005-2-23
/// ��������:
///		���ý�ϵ�
/// �������:
///		byValue			��ֵ
/// ���ز���:
///		��
void CTSAdaptionFieldEncapsulate::SetSplicingPoint( BYTE byValue )
{
    m_bSplicingPointFlag = true;
    m_bySplicingPoint = byValue;
}

///-------------------------------------------------------
/// CYJ,2005-2-23
/// ��������:
///		Ԥ�Ʋ���
/// �������:
///		��
/// ���ز���:
///		��
void CTSAdaptionFieldEncapsulate::Preset()
{
    m_bDiscontinuity = false;
    m_bRandomAccess = false;
    m_bElementStreamPrority = false;

    m_bPCR_Flag = false;
    m_llPCR = 0;
    m_wPCR_Extension = 0;

    m_bOPCR_Flag = false;
    m_llOPCR = 0;
    m_wOPCR_Extension = 0;

    m_bSplicingPointFlag = false;
    m_bySplicingPoint = 0;
}

///-------------------------------------------------------
/// CYJ,2005-2-23
/// ��������:
///		�������
/// �������:
///		nLen			�������
/// ���ز���:
///		��NULL			������
///		NULL			ʧ��
PBYTE CTSAdaptionFieldEncapsulate::Build( int & nLen )
{
    CMyBitStream	OutStream( m_abyDataBuf, sizeof(m_abyDataBuf) );
    OutStream.PutBit( m_bDiscontinuity );
    OutStream.PutBit( m_bRandomAccess );
    OutStream.PutBit( m_bElementStreamPrority );
    OutStream.PutBit( m_bPCR_Flag );
    OutStream.PutBit( m_bOPCR_Flag );
    OutStream.PutBit( m_bSplicingPointFlag );
    OutStream.PutBits( 0, 2 );			//	������λΪ0

    if( m_bPCR_Flag )
    {
        OutStream.PutBits32( DWORD(m_llPCR >> 1) );
        OutStream.PutBit( BYTE(m_llPCR & 1) );
        OutStream.PutBits( 0, 6 );
        OutStream.PutBits( m_wPCR_Extension, 9 );
    }

    if( m_bOPCR_Flag )
    {
        OutStream.PutBits32( DWORD(m_llOPCR >> 1) );
        OutStream.PutBit( BYTE(m_llOPCR & 1) );
        OutStream.PutBits( 0, 6 );
        OutStream.PutBits( m_wOPCR_Extension, 9 );
    }

    if( m_bSplicingPointFlag )
        OutStream.PutBits8( m_bySplicingPoint );

    OutStream.FinishWrite();

    nLen = OutStream.GetTotalWriteBits()/8;
    return m_abyDataBuf;
}

///////////////////////////////////////////////////////////////////////
//	CTSPacketEncapsulator
///////////////////////////////////////////////////////////////////////
CTSPacketEncapsulator::CTSPacketEncapsulator()
{ 
    Preset();
}

///-------------------------------------------------------
/// CYJ,2005-2-7
/// ��������:
///		��ʼ��TSͷ4���ֽ�
/// �������:
///		��
/// ���ز���:
///		��
void CTSPacketEncapsulator::Preset()
{
    m_abyData[0] = 0x47;		// ͬ����
    m_abyData[1] = 0x1F;		// ��Ч��PID
    m_abyData[2] = 0xFF;
    m_abyData[3] = 0x10;		// ���Ϊ0����������Ч����
}
///-------------------------------------------------------
/// CYJ,2005-2-7
/// ��������:
///		����PID
/// �������:
///		wPID		PID ��ֵ
/// ���ز���:
///		��	
void CTSPacketEncapsulator::SetPID( WORD wPID )
{
    m_abyData[1] &= 0xE0;
    m_abyData[1] |= BYTE( (wPID>>8)&0x1F );
    m_abyData[2] = BYTE(wPID);
}
///-------------------------------------------------------
/// CYJ,2005-2-7
/// ��������:
///		���ø�����ʼ��־
/// �������:
///		bStart			�Ƿ���ʼ��ȱʡΪ true
/// ���ز���:
///		��
void CTSPacketEncapsulator::SetPayloadStartIndicator( BOOL bStart )
{
    m_abyData[1] &= (~0x40);	// 1011 1111
    if( bStart )
        m_abyData[1] |= 0x40;
}	

///-------------------------------------------------------
/// CYJ,2005-2-7
/// ��������:
///
/// �������:
///		byValue			��������ֵ�� 1 �� 3
///		byLen			�����򳤶ȣ� 0 �� 183
///		pBuf			�������ݣ�ȱʡΪNULL
/// ���ز���:
///		��	
void CTSPacketEncapsulator::SetAdaptionField( BYTE byValue, BYTE byLen, PBYTE pBuf )
{
    m_abyData[3] &= 0xCF;		// 1100 1111
    byValue &= 3;
    byValue <<= 4;
    m_abyData[3] |= byValue;

    if( byValue & 0x20 )
    {							//	������������
        m_abyData[4] = byLen;
        if( pBuf )
            memcpy( m_abyData+5, pBuf, byLen );
        else
            memset( m_abyData+5, 0, byLen );			//  CYJ,2006-9-6 ���
    }
}

///-------------------------------------------------------
/// CYJ,2005-2-7
/// ��������:
///		����������ָ��
/// �������:
///		byValue			������ָ��
/// ���ز���:
///		��
void CTSPacketEncapsulator::SetContinuity( BYTE byValue )
{
    byValue &= 0xF;
    m_abyData[3] &= 0xF0;
    m_abyData[3] |= byValue;
}

///////////////////////////////////////////////////////////////
CDVBPacketEncapsulatorBase::CDVBPacketEncapsulatorBase()
{
    m_wPID = INVALID_PID;
    m_byTSContinuity = 0;
}

///-------------------------------------------------------
/// CYJ,2005-2-7
/// ��������:
///		����TS�����PID
/// �������:
///		wPID			�����PID
/// ���ز���:
///		ԭ����PID
WORD CDVBPacketEncapsulatorBase::SetPID(WORD wPID)
{
    WORD wRetVal = m_wPID;
    m_wPID = wPID & 0x1FFF;
    return wRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-2-7
/// ��������:
///		����һ��TS���飬��������Ӧ���ֶ�
/// �������:
///		��
/// ���ز���:
///		NULL			ʧ��
///		����			TS����
///	˵����
///		�÷�������AllocateTSPacketBuffer�������Ի��TS����
CTSPacketEncapsulator * CDVBPacketEncapsulatorBase::GetTSPacket()
{
    PDVB_TS_PACKET pPacket = AllocateTSPacketBuffer();
    if( NULL == pPacket )
        return NULL;
    CTSPacketEncapsulator *pRetVal = (CTSPacketEncapsulator *)pPacket;

    pRetVal->Preset();
    pRetVal->SetPID( m_wPID );
    pRetVal->SetContinuity( m_byTSContinuity & 0xF );
    m_byTSContinuity ++;

    return pRetVal;
}
