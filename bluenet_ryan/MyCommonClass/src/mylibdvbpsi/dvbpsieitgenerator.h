///=======================================================
///    
///     ����: ������
///    
///     niniryuhappy@gmail.com
///    
///     ����: 2006-4-17
///     �ļ�: OneProgram_EITGenerator.h
///     �汾: 
///     ˵��: 
///    
///========================================================
// OneProgram_EITGenerator.h: interface for the COneProgram_EITGenerator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ONEPROGRAM_EITGENERATOR_H__D90BA713_5534_4D88_B2D0_45BFE2A8E63C__INCLUDED_)
#define AFX_ONEPROGRAM_EITGENERATOR_H__D90BA713_5534_4D88_B2D0_45BFE2A8E63C__INCLUDED_

#include <MyString.h>
#include <MyArray.h>
#include "bitstream.h"
#include "psitablegenerator.h"

#pragma pack(push,4)
//////////////////////////////////////////////////////////////////////////
// COneProgram_EITGenerator ������ĳһ�߼�Ƶ����EIT
//////////////////////////////////////////////////////////////////////////
class COneProgram_EITGenerator  
{
public:
	
	//EIT_BUFFER������ɵ�EIT Section
	typedef struct tagEIT_Buffer
	{
		BYTE m_abyEITBuf[4000];
		WORD m_wBufSize;
	}EIT_BUFFER,*PEIT_BUFFER;

	//�¼�����
	typedef struct tagOne_Event_Item
	{
		WORD		m_wEventID;
		time_t		m_tStartUTCTime;		//Universal Coordinated time
		DWORD		m_dwDuration;			// in seconds
		BYTE		m_byRunningStatus;		// 0-undefine,1-not running,2-start in a few second,3-pausing,4-running
		BYTE		m_bFreeCAMode;			//0-scrambled, 1-not scrambled
		BYTE		m_aISOLanguage[3];		// "chi"-Chinese, "eng"-English
		CMyString   m_strEventName;			//event name
		CMyString	m_strShortDescription;	//short event description, event_name+ short_event_description <= 250 bytes
		CMyString	m_strDetailDescription;	//detail event description , <= 2000bytes,
											//at the most 240 bytes in one extend descriptor, so the max count of descriptor is 9 	
	}ONE_EVENT_ITEM, *PONE_EVENT_ITEM;

	//ĳһ�߼�Ƶ��EIT��������
	typedef struct tagOne_ProgramEIT_Base
	{
		WORD m_wSID;
		WORD m_wTSID;
		WORD m_wOriginalNetworkID;
		BYTE m_bCurrentNextIndicator;
		BYTE m_byEITScheduleVersion;
		BYTE m_byEITPFVersion;
	}ONE_PROGRAMEIT_BASE,*PONE_PROGRAMEIT_BASE;
	
	enum{
		ENABLE_SID  = 0x1,
		ENABLE_TSID = 0x02,
		ENABLE_CURRENT_NEXT_INDICATOR = 0x04,
		ENABLE_ORIGINAL_NETWORK_ID = 0x08,
		ENABLE_SCHEDULE_VERSION = 0x10,
		ENABLE_PF_VERSION = 0x20,
		ENABLE_ALL_FLAG = 0xFF
	};
	
	//EIT Segment:ÿ����Сʱ�¼�Ŀ���һ��Segment��������8��section��
	typedef struct tagOne_Segment_Item
	{
		WORD m_wFirstEventIndex;   //��ʼ�¼���m_aScheduleEvent�е�����
		WORD m_wEventCount;        //��segment�а������¼�����
		WORD m_wFirstSectionIndex; //��ʼsection��m_aScheduleSection�е�����
		WORD m_wSectionCount;      //section����
	}ONE_SEGMENT_ITEM,*PONE_SEGMENT_ITEM;

	//EIT section ����
	typedef struct tagOne_Section_Cfg
	{
		BYTE  m_byTableID;          //EIT Table ID
		BYTE  m_byLastTableID;      //���߼�Ƶ���¼��ƻ���ʹ�õ����һ��EIT Table ID
		BYTE  m_bySection_Num;     //���߼�Ƶ���ĵ�ǰTable�����һ��Section���
		BYTE  m_byLastSection_Num; //���߼�Ƶ���ĵ�ǰTable�����һ��Section���
		BYTE  m_bySegment_LastSection_Num; //����segment�����һ��section���
	}ONE_SECTION_CFG, *PONE_SECTION_CFG;

public:	
	COneProgram_EITGenerator();
	virtual ~COneProgram_EITGenerator();
	void Preset();
public:
	void RemovePFEvent();
	void SetCurrentDate(time_t  tUTCmidnight);
	void RemoveAllSchedule(); 
	
	long Release();
	long AddRef();
	void RemoveEvent(WORD wEventID);

	int AddScheduleEvent(ONE_EVENT_ITEM& OneEvent);	
	void SetBaseCfg(ONE_PROGRAMEIT_BASE& Basecf, DWORD dwFlags = 0xffffffff);
	void SetFollowingEvent(ONE_EVENT_ITEM &OneEvent);
	void SetPresentEvent(ONE_EVENT_ITEM& OneEvent);
	void Build();

	CMyArray<EIT_BUFFER>	  m_aScheduleEITBuf;
	CMyArray<EIT_BUFFER>	  m_aPFEITBuf;
	//���߼�Ƶ��EIT��������
	ONE_PROGRAMEIT_BASE m_BaseCfg;
protected:
	long m_nRefCount;
	int FindOneEvent(WORD wEventID);
	WORD SplitSegmentEdge();
	void SplitScheduleSectionEdge(ONE_SEGMENT_ITEM& segmentItem);
	
	void BuildScheduleTable();
	void BuildScheduleOneSection(ONE_SECTION_CFG& SecCfg, int nNo);
	void BuildPFEITSection();
	WORD BuildOneEvent(CMyBitStream* pStream,ONE_EVENT_ITEM& pEvent);
	WORD BuildExtendDescriptor(CMyBitStream *pStream, CMyString& strDescriptor, DWORD dwISOlanguage);
	WORD BuildShortDescriptor(CMyBitStream *pStream, CMyString strEventName,CMyString strDescriptor, DWORD dwISOlanguage);
	void UTCTimetoMJD(time_t UTCtime, PBYTE pBuf);
	
	CMyArray<ONE_SEGMENT_ITEM> m_aScheduleSegment;//
	CMyArray<DWORD>			   m_aScheduleSection;// higher 16 bit : event count, lower 16 bit :start event index;
	CMyArray<ONE_EVENT_ITEM>   m_aScheduleEvent;//����ʱ��˳������(�ӵ���UTC��ҹ��ʼ)��eventid��������
	
	ONE_EVENT_ITEM		m_aPFEventBuffer[2];	//  CYJ,2006-12-31 ���ڵ�ǰ/��һ���¼�����������������������/�ͷ��ڴ�	
	//��ǰ�¼�
	PONE_EVENT_ITEM		m_pPresentEvent;   
	//��һ���¼�
	PONE_EVENT_ITEM     m_pFollowingEvent;
	

	//ScheduleEIT  ��ʼʱ��, һ��ΪUTC���
	time_t	m_tScheduleStartTime;
	bool	m_bEventPFModified;
	bool	m_bEventScheduleModified;
};

//////////////////////////////////////////////////////////////////////////
// CEITGenerator : �����߼�Ƶ����EIT���������װ��TS��
//////////////////////////////////////////////////////////////////////////
class CDVBPSI_EITGenerator : public CDVBPSI_TableGeneratorBase
{
public:
	enum
	{
		BUILD_FLAG_PF_SECTION = 1,
		BUILD_FLAG_SCHEDULE = 2,

		BUILD_FLAG_ALL = 0xFFFFFFFF,
	};

	void Build( DWORD dwBuildMask = BUILD_FLAG_ALL );
	void RemoveAll();
	void DeregisterProgramEIT(WORD wSID);
	void RegisterOneProgramEIT(WORD wSID, COneProgram_EITGenerator* pProgramEIT);
	virtual void EncapsulateOneSection(PBYTE pBuf, int nLen );
	CDVBPSI_EITGenerator();
	virtual ~CDVBPSI_EITGenerator();
	
protected:
	CMyMap<WORD, WORD, COneProgram_EITGenerator*, COneProgram_EITGenerator*> m_MapSID_ProgarmEIT;
};

#pragma pack(pop)

#endif // !defined(AFX_ONEPROGRAM_EITGENERATOR_H__D90BA713_5534_4D88_B2D0_45BFE2A8E63C__INCLUDED_)
