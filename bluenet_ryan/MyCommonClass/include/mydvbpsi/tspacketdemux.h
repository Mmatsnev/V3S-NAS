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


#if !defined(AFX_TSPACKETDEMUX_H__4496255B_82FA_4F32_A767_D45D6AD82BFD__INCLUDED_)
#define AFX_TSPACKETDEMUX_H__4496255B_82FA_4F32_A767_D45D6AD82BFD__INCLUDED_

#include "tspacket.h"

#pragma pack(push,4)

class CPID_ResponserMapper;

class CTSPacketDemux
{
public:
	void DeregisterResponser( WORD wPID );
	void RegisterResponser( WORD wPID, CTSPacketResponser * pResponser );	
	void PushOneTSPacket( PDVB_TS_PACKET pPacket );
	bool IsValid(){ return m_pMapper != NULL; }
	void Reset();

	CTSPacketDemux();
	virtual ~CTSPacketDemux();

private:
	CPID_ResponserMapper * m_pMapper;
};

#pragma pack(pop)

#endif // !defined(AFX_TSPACKETDEMUX_H__4496255B_82FA_4F32_A767_D45D6AD82BFD__INCLUDED_)
