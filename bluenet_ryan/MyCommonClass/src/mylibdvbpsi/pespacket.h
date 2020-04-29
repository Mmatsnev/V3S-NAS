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

#if !defined(AFX_PESPACKET_H__7C4FE69C_BE2C_4FA9_88A2_60CC90418602__INCLUDED_)
#define AFX_PESPACKET_H__7C4FE69C_BE2C_4FA9_88A2_60CC90418602__INCLUDED_

#include <myheap.h>
#include <MyArray.h>
#include "tspacket.h"

#pragma pack(push,4)

//-----------------------------------------------
// ISO ��׼�����������
typedef enum
{
	DVBPESSI_PROGRAM_STREAM_MAP = 0xBC,
	DVBPESSI_PRIVATE_STREAM_1 = 0xBD,
	DVBPESSI_PADING_STREAM,
	DVBPESSI_PRIVATE_STREAM_2,
	DVBPESSI_AUDIO_13818_11172_START = 0xC0,
	DVBPESSI_AUDIO_13818_11172_END = 0xDF,
	DVBPESSI_VIDEO_13818_11172_START = 0xE0,
	DVBPESSI_VIDEO_13818_11172_END = 0xEF,
	DVBPESSI_ECM_STREAM = 0xF0,
	DVBPESSI_EMM_STREAM,
	DVBPESSI_DSM_CC_STREAM,
	DVBPESSI_IEC_13522_STREAM,
	DVBPESSI_PROGRAM_STREAM_DIRECTORY = 0xFF,
}DVB_PES_STREAM_ID;

//-------------------------------------------------
// Stream Type
typedef enum
{
	DVBPES_STREAM_TYPE_VIDEO_MPEG1 = 1,
	DVBPES_STREAM_TYPE_VIDEO_MPEG2,
	DVBPES_STREAM_TYPE_AUDIO_MPEG1,
	DVBPES_STREAM_TYPE_AUDIO_MPEG2,
	DVBPES_STREAM_TYPE_H222_ISO13818_PRIVATE_SEGMENT,
	DVBPES_STREAM_TYPE_H222_ISO13818_PEC_PACKET,
	DVBPES_STREAM_TYPE_ISOIEC13522_MHEG,
	DVBPES_STREAM_TYPE_H222_ISO13818_1_DSMCC,
	DVBPES_STREAM_TYPE_H222_ISO13818_1_11172_1,

	DVBPES_STREAM_TYPE_AUDIO_MPEG4 = 0xF,
	DVBPES_STREAM_TYPE_VIDEO_MPEG4 = 0x10,
	DVBPES_STREAM_TYPE_VIDEO_H264 = 0x1B,

}DVB_PES_STREAM_TYPE;

//----------------------------------------------
//	���� PES ������ʽ��Ϊ�����������Ĵ�������
typedef enum
{
	PES_PROCESS_MODE_AV_STREAM_COMPATIBLE = 0,
	PES_PROCESS_MODE_EMM_EMC_COMPATIBLE,
	PES_PROCESS_MODE_PADING,
}SELFDEFINE_PES_PROCESS_MODE;

#ifndef MY_LONG64
	#ifdef _WIN32
		typedef __int64	MY_LONG64;
	#else
		typedef long long MY_LONG64;
	#endif //_WIN32
#endif // MY_LONG64

#pragma pack(push,1)

typedef struct tagDVB_PES_PACKET_HEADER
{
	BYTE	m_abyStartCodePrefix[3];		// 0x00 00 01
	BYTE	m_byStreamID;
	BYTE	m_abyPacketLength[2];
	BYTE	m_abyData[1];
public:
	//	��ȡ PES ����С
	inline WORD	GetPacketLength(){ return (m_abyPacketLength[0]<<8) | m_abyPacketLength[1]; }
	//  ����ǰ3���ֽڣ��ж��Ƿ�Ϊ PES ��
	inline bool	IsPESPacket()
	{ return  m_abyStartCodePrefix[0] == 0 && m_abyStartCodePrefix[1] == 0 && m_abyStartCodePrefix[2] == 1; }
	//  ��ȡPES�Ĵ���ʽ
	SELFDEFINE_PES_PROCESS_MODE GetProcessMode();
	//  ��ȡPESͷ��С
	int GetHeaderLen();
	//  ��ȡ��Ч���ݵ�ַ���䳤��
	PBYTE GetPayloadData( int & nPayloadDataLen );
	//	��ȡ���ſ���
	BYTE  GetScrambling_Control()
	{
		ASSERT(PES_PROCESS_MODE_AV_STREAM_COMPATIBLE == GetProcessMode() );
		return (( m_abyData[0]>>4)&3 );
	};
}__MY_PACKTED__ DVB_PES_PACKET_HEADER,*PDVB_PES_PACKET_HEADER;

#pragma pack(pop)

class CPCR_TSPacketResponser;

//-----------------------------------------------
//	PES ����⸴��
class CPESPacket :
	public CTSPacketResponser,
	public CMyHeap
{
public:
	bool GetPCR(MY_LONG64 & scr, WORD & wExtension );
	void SetPCR(MY_LONG64 & scr, WORD wExtension );
	bool HandlePCR( PDVB_TS_PACKET pTSPacket );
	bool IsRequireCompletePESPacket() const{ return m_bRequireCompletePacket; }
	// ����Ҫ���������������£�����CRC���
	bool DoCRCCheck(bool bDoCheck = true);
	bool IsDoCRCCheck()const {return m_bDoCRCCheck;}
	bool SetRequireCompletePESPacket( bool bComplete = true );
	int GetOutputMethod()const	{ return m_nOutputMethod; }
	int SetOutputMethod( int nMethod );
	CPCR_TSPacketResponser * GetAssociatedPCRResponser();
	CPCR_TSPacketResponser * SetAssociatedPCRResponser( CPCR_TSPacketResponser * pResponser);

	CPESPacket(int nCacheBufSize = 0x10100);
	virtual ~CPESPacket();

	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual void PushOneTSPacket( PDVB_TS_PACKET pPacket );
	//	���յ�PES���飬pData,nDataLen ���ݣ�nOffset ����������PES������е�ƫ�ƣ�nErrorTimes ��PES��������Ĵ���
	virtual void OnPESReceived(PBYTE pData, int nDataLen, int nOffset, int nErrorTimes ) = 0;

	enum{
		RESULT_CONSTRUCT_SUCC = 0,			//	����ɹ�
		RESULT_CONSTRUCT_CANCELD,			//	��ı䣬��û������
	};
	enum{
		OUTPUT_METHOD_AS_ELEMENTARY_STREAM = 0,	//	���Ԫ����
		OUTPUT_METHOD_AS_PES_PACKET,			//	���PES����
		OUTPUT_METHOD_AS_PROGRAM_STREAM,		//	�Խ�Ŀ����ʽ���
	};

	void Dump(FILE*fOutput=NULL);

protected:
	bool	m_bHeaderReceived;				//	��ͷ���յ���
	WORD	m_wErrorCounter;				//	TS packet ��������Ĵ���
	BYTE	m_byExpectTSContinuityCounter;	//	�ϴ� TS packet ��continuity counter
	bool	m_bRequireCompletePacket;		//	Ҫ�����������PES���飬������Ƶ����������������ķ��飬���洦������ƴ��
	DWORD	m_dwPESBytesReceived;			//  PES �����Ѿ����յ����ֽ���

	int		m_nOutputMethod;				//	�����ʽ��ȱʡ��ʽ���ΪԪ����

	BYTE	m_byStreamID;
	WORD	m_wPacketLength;				//	����Ϊ0
	int		m_nPayloadLen;					//	��Ч���ݴ�С���������Ԫ����ʱ�����ã��ҷ���Ƶ���������

	bool	m_bHasSCR;
	WORD	m_wPS_SCR_Base_0_14;			//	system clock reference
	WORD	m_wPS_SCR_Base_15_29;
	WORD	m_wPS_SCR_Base_30_32;
	WORD	m_wPS_SCR_Extension;			// system clock reference extension
	MY_LONG64	m_PCR;						// Program clock reference

	bool	m_bHasPTS;
	bool	m_bHasDTS;
	bool	m_bHasESMuxRate;
	MY_LONG64	m_PTS;						// ����ʱ��
	MY_LONG64	m_DTS;						// ����ʱ��
	DWORD		m_dwProgramMuxRate;

	bool	m_bRandomAccessPoint;			// �Ƿ�Ϊ������ʵ�
	bool	m_bTSPacketDiscontinuity;		// TS ���鲻����

	CPCR_TSPacketResponser	*	m_pPCRResponser;		// �� PCR_PID = ��PES's PID ʱ����Ҫ�øö���ȥ����������PES������

	bool m_bDoCRCCheck;						//  CYJ,2006-3-2, ����CRC��⣬ȱʡΪ���
	BYTE m_byLastHasCRCFlags;				//  �ϴ���CRC����������һ��û��CRC������ΪPES����Ĵ���;bit0 ���һ֡��bit1��һ֡

private:
	PBYTE GetCRC( PBYTE pBuf, int nLen ) const;
	void DecodePESHeader(PBYTE pBuf, int nLen);
	void ConstructPSPackHeader( bool bOutputSystemHeader = false );
	void DecodeTSAdaptionField( PDVB_TS_PACKET pTSPacket );
};


class CPESResponserArray;

//---------------------------------------------------
//	PCR TS �������
class CPCR_TSPacketResponser : public CTSPacketResponser
{
public:
	CPCR_TSPacketResponser();
	virtual ~CPCR_TSPacketResponser();
	virtual void PushOneTSPacket( PDVB_TS_PACKET pPacket );

	bool IsValid();
	void RemoveAll();
	void Remove( CPESPacket * pResponser );
	void Add( CPESPacket * pResponser );

private:
	CPESResponserArray * m_paPESResponser;

private:
	int	Find( CPESPacket * pResponser );
};

#pragma pack(pop)

#endif // !defined(AFX_PESPACKET_H__7C4FE69C_BE2C_4FA9_88A2_60CC90418602__INCLUDED_)
