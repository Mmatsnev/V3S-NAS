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

// DVBPSITable_IP_MPE.h: interface for the CDVBPSITable_IP_MPE class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DVBPSITABLE_IP_MPE_H__284625B0_07C2_411A_A11D_7AEAA33CC8B4__INCLUDED_)
#define AFX_DVBPSITABLE_IP_MPE_H__284625B0_07C2_411A_A11D_7AEAA33CC8B4__INCLUDED_

#include "dvbpsitables.h"

#pragma pack(push,4)

class CDVBPSITable_IP_MPE : public CDVBPSITablesBase
{
public:
	CDVBPSITable_IP_MPE();
	virtual ~CDVBPSITable_IP_MPE();

	enum{
		MPE_TABLE_MAX_SIZE = 2*1024,		// 2K, Ŀǰֻ���� IPv4 ����̫֡���Ժ��ٿ���������
	};

	virtual void OnTableReceived();			//	���յ�����
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	��ȡ�ϴν��յ��ı�
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual bool IsPSIPacketIntegral();

	virtual void OnEthernetFrame( PBYTE pEthernetAddr, PBYTE pIPPacket, int nIPLen );

	virtual void Dump(FILE*fOutput=NULL);

private:
	CMyHeap	m_EthFrame;
	int m_nDataHeaderLen;		// IP ��ͷ����
	BYTE	m_abyEthAddr[6];	// 6 bytes ethernet addr
};


#pragma pack(pop)

#endif // !defined(AFX_DVBPSITABLE_IP_MPE_H__284625B0_07C2_411A_A11D_7AEAA33CC8B4__INCLUDED_)
