///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2005-1-11
///
///		��;��
///			DVB PSI Tables ����
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӡ��ڲ�ʹ��!	  			     !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	��Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!			 !
///=======================================================

#if !defined(AFX_DVBPSITABLES_H__FD159F36_C595_4D3F_AC09_15406E7B5F8E__INCLUDED_)
#define AFX_DVBPSITABLES_H__FD159F36_C595_4D3F_AC09_15406E7B5F8E__INCLUDED_

#include <myheap.h>
#include "tspacket.h"
#include "dvbpsitablesdefine.h"


#pragma pack(push,4)
#define INVALID_ID	0xFFFF

class CDVBSectionReceivingLog
{
public:
	CDVBSectionReceivingLog();
	~CDVBSectionReceivingLog();

	void ClearSectionLog();					// ���section��¼״̬
	void SetSectionNoStatus(int nSectionNo, bool bValue=true);	// ���ý���״̬
	bool IsSectionReceived( int nSectionNo );	// �ж��ͷŽ��յ�
	bool IsAllSectionReceived();			// �ж�ȫ������
	void SetSectionCount(int nSectionCount);	// ����section����

	void Reset();
	int GetSectionCount()const{return m_nSectionCount;}

protected:
	DWORD	m_adwSectionRecLog[8];			// 8*4*8 = 256�����ڼ�¼Section�Ƿ���յ�
	int		m_nSectionCount;				// section ���������256
};

class CDVBPSITablesBase :
	public CTSPacketResponser,
	public CMyHeap,
	public CDVBSectionReceivingLog
{
public:
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual void PushOneTSPacket( PDVB_TS_PACKET pPacket );
	CDVBPSITablesBase(int nMaxTableSize=0);
	virtual ~CDVBPSITablesBase();

	enum{
		RESULT_CONSTRUCT_SUCC = 0,			//	����ɹ�
		RESULT_CONSTRUCT_CANCELD,			//	��ı䣬��û������
		RESULT_TABLE_NOT_EXIST,				//	������
	};

	virtual void OnTableReceived() = 0;			//	���յ�����
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived() = 0;		//	��ȡ�ϴν��յ��ı�
	virtual bool IsPSIPacketIntegral()=0;		//	�ж�һ�������Ƿ������ȫ

	virtual void Dump(FILE*fOutput=NULL) = 0;

protected:
	bool	m_bHeaderReceived;				//	��ͷ���յ���
	WORD	m_wErrorCounter;				//	TS packet ��������Ĵ���
	BYTE	m_byExpectTSContinuityCounter;	//	�ϴ� TS packet ��continuity counter
};

//------------------------------------------------
//	PAT ��
class CDVBPSITable_PAT : public CDVBPSITablesBase
{
public:
	CDVBPSITable_PAT();
	virtual ~CDVBPSITable_PAT();
	enum{
		PAT_TABLE_MAX_SIZE = 1024+256,
		PAT_TABLE_DECODED_MAX_SIZE = 4096,	// �������У�1000����Ŀ
		PAT_TABLE_MAX_PROGRAM = 1000,
	};

	virtual void OnTableReceived();			//	���յ�����
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	��ȡ�ϴν��յ��ı�
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual void OnPATReceived( const PDVB_PSI_TABLE_PAT pTable );
	virtual bool IsPSIPacketIntegral();

	virtual void Dump(FILE*fOutput=NULL);

private:
	CMyHeap	m_PatTable;
};

//------------------------------------------------
//	CAT ��
class CDVBPSITable_CAT : public CDVBPSITablesBase
{
public:
	CDVBPSITable_CAT();
	virtual ~CDVBPSITable_CAT();

	enum
	{
		CAT_TABLE_MAX_SIZE = 1024+256,
		CAT_TABLE_DECODED_MAX_SIZE = 4096,	// �������У�1000����Ŀ
	};

	virtual void OnTableReceived();			//	���յ�����
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	��ȡ�ϴν��յ��ı�
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual void OnCATReceived( const PDVB_PSI_TABLE_CAT pTable );
	virtual bool IsPSIPacketIntegral();

	virtual void Dump(FILE*fOutput=NULL);

private:
	CMyHeap	m_CatTable;
};

//------------------------------------------------
//	PMT ��
class CDVBPSITable_PMT : public CDVBPSITablesBase
{
public:
	CDVBPSITable_PMT();
	virtual ~CDVBPSITable_PMT();
	enum{
		PMT_TABLE_MAX_SIZE = 1024+512,
		PMT_TABLE_DECODED_MAX_SIZE = 4096,	//  CYJ,2007-3-21 ���ӵ�4KB
	};

	virtual void OnTableReceived();			//	���յ�����
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	��ȡ�ϴν��յ��ı�
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual void OnPMTReceived( const PDVB_PSI_TABLE_PMT pTable );
	void SetSID( WORD wSID ){ m_wDstSID = wSID; }
	WORD GetSID() const{ return m_wDstSID;  }
	virtual bool IsPSIPacketIntegral();

	virtual void Dump(FILE*fOutput=NULL);

private:
	int DecodeStreamInfo( PBYTE pBuf, int nLen );
	int GetStreamCount( PBYTE pBuf, int nLen );

private:
	CMyHeap	m_PmtTable;
	WORD	m_wDstSID;				//	Ŀ��SID����Ϊ 0x1FFF����ȫ��ͨ��
};

//------------------------------------------------
// SDT, BAT
class CDVBPSITable_SDT_BAT : public CDVBPSITablesBase
{
public:
	CDVBPSITable_SDT_BAT();
	virtual ~CDVBPSITable_SDT_BAT();
	enum{
		TABLE_MAX_SIZE = 1024 + 256,
		DECODED_SDT_MAX_SIZE = 2048,
		DECODED_ST_MAX_SIZE = 2048,
		DECODED_BAT_MAX_SIZE = 2048,
	};
	virtual void OnTableReceived();			//	���յ�����
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	��ȡ�ϴν��յ��ı�
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual bool IsPSIPacketIntegral();

	virtual void OnSDTReceived( PDVB_PSI_TABLE_SDT pSDT );
	virtual void OnBATReceived( PDVB_PSI_TABLE_BAT pBAT );

	PDVB_PSI_TABLE_SDT	GetLastReceivedSDT();
	PDVB_PSI_TABLE_BAT	GetLastReceivedBAT();

	virtual void Dump(FILE*fOutput=NULL);

private:
	void DecodeSDT();
	void DecodeBAT();
	WORD SDT_CalculateProgramCoumt(PBYTE pBuf, int nLen);
	int  BAT_GetBatItemsCount( PBYTE pBuf, int nLen );
	int BAT_DecodeBATItems( PBYTE pBuf, int nLen );

private:
	CMyHeap	m_SDT;
	CMyHeap	m_BAT;
};

//////////////////////////////////////////////////////////
/// EIT
class CDVBPSITable_EIT : public CDVBPSITablesBase
{
public:
	CDVBPSITable_EIT();
	virtual ~CDVBPSITable_EIT();
	enum{
		TABLE_MAX_SIZE = 4096 + 256,			// ����256�ֽ���Ϊ���ܹ��������һ��TS����
		DECODED_TABLE_MAX_SIZE = 2048+4096,
	};
	virtual void OnTableReceived();							//	���յ�����
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	��ȡ�ϴν��յ��ı�
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual bool IsPSIPacketIntegral();
	virtual void Dump(FILE*fOutput=NULL);

	WORD	GetDstSID()const { return m_wDstSID; }
	WORD	SetDstSID( WORD wSID );

	virtual void OnEITReceived( PDVB_PSI_TABLE_EIT pEIT );

private:
	WORD GetEventsCount( PBYTE pBuf, int nLen );
	CMyHeap	m_EIT;
	WORD	m_wDstSID;
};

//////////////////////////////////////////////////////////
/// NIT
class CDVBPSITable_NIT : public CDVBPSITablesBase
{
public:
	CDVBPSITable_NIT();
	virtual ~CDVBPSITable_NIT();
	enum{
		TABLE_MAX_SIZE = 1024 + 256,
		DECODED_TABLE_MAX_SIZE = 2048,
	};
	virtual void OnTableReceived();							//	���յ�����
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	��ȡ�ϴν��յ��ı�
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual bool IsPSIPacketIntegral();
	virtual void Dump(FILE*fOutput=NULL);

	virtual void OnNITReceived( PDVB_PSI_TABLE_NIT pNIT );
	void SetDstNID( WORD wNID ){ m_wDstNID = wNID; }

private:
	CMyHeap	m_NIT;
	WORD	m_wDstNID;

private:
	int GetTSItemDescriptorCount( PBYTE pBuf, int nLen );
};

//////////////////////////////////////////////////////////////////////////
//	TDT		Time and Date Table
//	TOT		Time Offset Table
//	ST		Stuffing Table
class CDVBPSITable_TDT_TOT_ST :  public CDVBPSITablesBase
{
public:
	CDVBPSITable_TDT_TOT_ST();
	virtual ~CDVBPSITable_TDT_TOT_ST();

	enum{
		TABLE_MAX_SIZE = 1024 + 256,			// 1KB
		DECODED_TABLE_MAX_SIZE = 2048,
	};
	virtual void OnTableReceived();							//	���յ�����
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	��ȡ�ϴν��յ��ı�
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual bool IsPSIPacketIntegral();
	virtual void Dump(FILE*fOutput=NULL);

	virtual void OnDateTimeReceived( time_t tUTCDateTime, const PDVBPSI_TABLE_TOT pTOTTable ) = 0;		// ���յ� TDT
	virtual void OnTimeOffsetTableReceived( const PDVBPSI_TABLE_TOT pTOTTable ) = 0;		// ���յ� TDT

protected:
	void DecodeTDT();
	void DecodeTOT();

protected:
	CMyHeap	m_TOT;
};

///////////////////////////////////////////////////////////////////////
// ECM/EMM table
class CDVBPSITable_ECM_EMM_Message : public CDVBPSITablesBase
{
public:
	CDVBPSITable_ECM_EMM_Message();
	virtual ~CDVBPSITable_ECM_EMM_Message();

	enum{
		TABLE_MAX_SIZE = 4096,			// 4 KB
	};
	virtual void OnTableReceived();							//	���յ�����
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	��ȡ�ϴν��յ��ı�
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual bool IsPSIPacketIntegral();

	virtual void OnECM_EMMMessageReceived( BYTE byTableID, PBYTE pSectionData, int nSectionLen ) = 0;

	virtual void Dump(FILE*fOutput=NULL);
};

///////////////////////////////////////////////////////////////////////
// Tongshi VOD UniversalTable
class CDVBPSI_TSVOD_UniversalTable : public CDVBPSITablesBase
{
public:
	CDVBPSI_TSVOD_UniversalTable();
	CDVBPSI_TSVOD_UniversalTable(PBYTE pRawBuf,int nRawBufLen, PBYTE pDstBuf, int nDstBufLen);
	virtual ~CDVBPSI_TSVOD_UniversalTable();

	enum{
		TABLE_MAX_SIZE = ( 512*1024 + 256 ),
		DECODED_TABLE_MAX_SIZE = ( TABLE_MAX_SIZE * 2 + 2048 ),
	};

	virtual void OnPrivateDataReceived( PBYTE pBuf, int nLen, BYTE byBuildCounter, BYTE byTableID );

	virtual void OnTableReceived();							//	���յ�����
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	��ȡ�ϴν��յ��ı�
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual bool IsPSIPacketIntegral();

	virtual void Dump(FILE*fOutput=NULL);

private:
	CMyHeap	m_Tbl;
	BYTE	m_byBuildCounter;
};

#pragma pack(pop)

#endif // !defined(AFX_DVBPSITABLES_H__FD159F36_C595_4D3F_AC09_15406E7B5F8E__INCLUDED_)
