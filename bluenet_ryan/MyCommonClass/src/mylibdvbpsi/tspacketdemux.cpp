///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2005-1-12
///
///		��;��
///			TS ���⸴��
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӡ��ڲ�ʹ��!	  			     !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	��Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!			 !
///=======================================================

#include "stdafx.h"
#include "tspacketdemux.h"
#include <MyMap.h>

#if defined(_DEBUG) && defined(_WIN32)
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

class CPID_ResponserMapper : public CMyMap<WORD,WORD,CTSPacketResponser*,CTSPacketResponser*>
{
public:
	CPID_ResponserMapper(){}
	virtual ~CPID_ResponserMapper(){}
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTSPacketDemux::CTSPacketDemux()
{
	m_pMapper = new CPID_ResponserMapper;
}

CTSPacketDemux::~CTSPacketDemux()
{
	if( m_pMapper )
		delete m_pMapper;
	m_pMapper = NULL;
}

///-------------------------------------------------------
/// CYJ,2005-1-12
/// ��������:
///		���յ�һ��TS���飬�ҳ����еĴ�����󣬲�����֮
/// �������:
///		pPacket			TS ����
/// ���ز���:
///		��
void CTSPacketDemux::PushOneTSPacket(PDVB_TS_PACKET pPacket)
{
	ASSERT( pPacket && m_pMapper );
	if( NULL == pPacket )
		return;
	WORD wPID = pPacket->GetPID();
	if( INVALID_PID == wPID )
		return;					//	��Ч����

	CTSPacketResponser * pResponser = NULL;
	if( FALSE == m_pMapper->Lookup( wPID, pResponser ) || NULL == pResponser )
		return;					//	û�д�����
	pResponser->PushOneTSPacket( pPacket );
}

///-------------------------------------------------------
/// CYJ,2005-1-12
/// ��������:
///		ע��PID������
/// �������:
///		wPID				�������PID
///		pResponser			�������
/// ���ز���:
///		��
void CTSPacketDemux::RegisterResponser(WORD wPID, CTSPacketResponser *pResponser)
{
	ASSERT( wPID != INVALID_PID && pResponser && m_pMapper );
	if( wPID == INVALID_PID || NULL == pResponser || NULL == m_pMapper )
		return;
	m_pMapper->SetAt( wPID, pResponser );
}

///-------------------------------------------------------
/// CYJ,2005-1-12
/// ��������:
///		ע��TS������Ӧ��
/// �������:
///		��
/// ���ز���:
///		��
void CTSPacketDemux::DeregisterResponser(WORD wPID)
{
	ASSERT( m_pMapper );
	if( m_pMapper )
		m_pMapper->RemoveKey( wPID );
}

///-------------------------------------------------------
/// CYJ,2005-1-12
/// ��������:
///		ɾ�����ж�����Ӧ����
/// �������:
///		��
/// ���ز���:
///		��
void CTSPacketDemux::Reset()
{
	ASSERT( m_pMapper );
	if( m_pMapper )
		m_pMapper->RemoveAll();
}
