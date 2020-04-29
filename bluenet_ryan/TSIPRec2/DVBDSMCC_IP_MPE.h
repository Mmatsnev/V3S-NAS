// DVBDSMCC_IP_MPE.h: interface for the CDVBDSMCC_IP_MPE class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DVBDSMCC_IP_MPE_H__BA035F86_8FFA_4CE9_853C_716F36892C21__INCLUDED_)
#define AFX_DVBDSMCC_IP_MPE_H__BA035F86_8FFA_4CE9_853C_716F36892C21__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MyDVBPSI/tspacket.h>

const DWORD TS_MPE_MAX_ETHERNET_LEN = 8192;			// ��̫֡��󳤶ȣ�����֧��ǧ�׿���Ҫ�õ� 10KB

#pragma pack(push,1)

typedef BYTE ETHERNET_ADDRESS [6];
typedef struct ETHERNET_FRAME 
{
	ETHERNET_ADDRESS	Destination;
	ETHERNET_ADDRESS	Source;
	WORD				FrameType;			// in host-order
} ETHERNET_FRAME;

typedef struct IP_HEADER {
    BYTE    x;
    BYTE    tos;
    WORD    length;
    WORD    identifier;
#define IP_MF 0x2000
    WORD    fragment;
    BYTE    ttl;
    BYTE    protocol;
    WORD    cksum;
    DWORD   src;
    DWORD   dest;
} IP_HEADER;
typedef IP_HEADER * LPIPHEADER;

#define	IP_HEADER_MINIMUM_LEN	20

typedef struct UDP_HEADER {
	WORD	src_port;
	WORD	dest_port;
	WORD	length;			// including this header
	WORD	checksum;
} UDP_HEADER;

#define	UDP_HEADER_LEN			8

#define	ETHERNET_FRAME_TYPE_IP		0x0800


class CUDPDataPort;

class CDVBDSMCC_IP_MPE  
{
public:
	CDVBDSMCC_IP_MPE( CUDPDataPort * pDataPort, WORD wPID );
	virtual ~CDVBDSMCC_IP_MPE();

public:
	void PushOneTSPacket(PDVB_TS_PACKET pPacket);
	BOOL OnTableReceived();
	inline void OnEthernetFrame( PBYTE pEthernetAddr, int nFrameLen );	
	void AppendItem( CDVBDSMCC_IP_MPE * pItem );
	CDVBDSMCC_IP_MPE * GetPrevItem() { return m_pPreItem; }
	CDVBDSMCC_IP_MPE * GetNextItem() { return m_pNextItem; }	//  CYJ,2008-9-9 �޸ģ�ԭ�����ش���

private:
	BOOL Write( PBYTE pBuf, int nLen );
	void Reset();
	bool IsPSIPacketIntegral();	

public:
	WORD m_wPID;
	
private:
	int		m_nDataHeaderLen;		// IP ��ͷ����		
	BYTE	m_abyDataBuf[TS_MPE_MAX_ETHERNET_LEN+sizeof(ETHERNET_FRAME)];	// �ñ�����ֱ������
	PBYTE	m_pInputDataBuf;				// �������ݻ�����
	int		m_nDataLen;						// m_pInputDataBuf

	BYTE	m_abyEthAddr[ sizeof(ETHERNET_ADDRESS) ];

	bool	m_bHeaderReceived;				//	��ͷ���յ���
	WORD	m_wErrorCounter;				//	TS packet ��������Ĵ���
	BYTE	m_byExpectTSContinuityCounter;	//	�ϴ� TS packet ��continuity counter	

	CUDPDataPort * m_pDataPort;

	// ˫������
	CDVBDSMCC_IP_MPE * m_pPreItem;			// ��һ��
	CDVBDSMCC_IP_MPE * m_pNextItem;			// ��һ��
};

#pragma pack(pop)

#endif // !defined(AFX_DVBDSMCC_IP_MPE_H__BA035F86_8FFA_4CE9_853C_716F36892C21__INCLUDED_)
