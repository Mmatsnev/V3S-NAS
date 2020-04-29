// MB_OneFile.h: interface for the CMB_OneFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MB_ONEFILE_H__AE83B1CC_79BC_444A_861A_B368DFF9BE3E__INCLUDED_)
#define AFX_MB_ONEFILE_H__AE83B1CC_79BC_444A_861A_B368DFF9BE3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IPRecSvr.h"
#include <BufPacket4C.h>
#include "Tsdb.h"

class COneDataPortItem;


#pragma pack(push,1)						//	��һ���ֽڶ������

typedef struct tagMYMEMALLOCFLAG
{
	DWORD	m_bAllocUseHeadAlloc:1;		//	�Ƿ�� PagedHeap ���䣬��֮�� new
	DWORD	m_bAutoDelete:1;			//	��Ҫ�Զ�ɾ������PagedHeapɾ������֮��delete
	DWORD	m_dwRes:30;					//	���������룽0
}MYMEMALLOCFLAG,*PMYMEMALLOCFLAG;

#pragma pack(pop)						//	��һ���ֽڶ������

//	һ���ļ�
class CMB_OneFile : public CBufPacket4C<IBufPacket>
{
public:
	CMB_OneFile();		
	~CMB_OneFile();

	int AddOnePage( PBYTE pBuf, DWORD dwLen );
	BOOL IsFileChanged( time_t t,DWORD dwFileLen);
	BOOL Initialize( TSDBCHANNEL chFile, LPCSTR lpszFileName, DWORD dwLen, time_t FileTime );
	BOOL CollectDataUseXorChksum();

    virtual void SafeDelete();

public:
	enum {  PRS_MAX_PAGENUM = 256,			//	����¼��ҳ״̬

			MBROF_DATA_ERR = -2,			//	���ݴ���
			MBROF_FILE_CHANGED = -1,		//	�ļ��ı�
			MBROF_DATAOK_FILENOTOK = 0,		//	����OK�����ļ���û�����
			MBROF_FILE_OK = 1,				//	�ļ��ɹ�����
	};
//	������
public:		
	void SetMulticastParameter( LPCSTR lpszIP, WORD wPort, COneDataPortItem * pDataPortItem );
	CString m_strMC_DstIP;					//	2002.11.14 ��ӣ��ಥIP��ַ
	WORD	m_wMC_Port;						//	2002.11.14 ��ӣ��ಥ�˿�
	COneDataPortItem *	m_pDataPortItem;	//  2004-5-20 data port item

	time_t	m_Time;							//	�ļ�ʱ��
	TSDBCHANNEL	m_chFile;
	char	m_szFileName[13];				//	�ļ���
	DWORD	m_dwFileLen;					//	�ļ�����
	DWORD	m_dwByteRead;					//	�Ѿ���ȡ���ֽڳ���
	DWORD	m_dwSysCodeIndex;				//	ϵͳ��������
	DWORD	m_adwPageRecFlags[PRS_MAX_PAGENUM/32];		//	ֻ��ǰ8*32=32*8=256ҳ��ÿ���ر�ʾһҳ�Ƿ����
	DWORD	m_adwPageErrFlags[PRS_MAX_PAGENUM/32];		//	ֻ��ǰ8*32=32*8=256ҳ��ÿ���ر�ʾ��ҳ�Ƿ����
	union
	{
		struct
		{
			DWORD	m_bIsReceived:1;		//	�Ƿ��У����֣�����
			DWORD	m_bIsFileErr:1;			//	�Ƿ�����
			DWORD	m_dwRes:30;				//	��������0
		};
		DWORD	m_dwData;
	}m_dwResultFlags;

private:
	void SetPageReceived( int nPageNo, BOOL bIsErr );	
	enum { XORCHKSUMBUFLEN = 2048 };
	BYTE	m_abyXorChkSum[XORCHKSUMBUFLEN];	//	XOR CHK SUM
	int		m_nXorChkSumDataLen;				//	��0����У�飻��֮��û��У��
};

#endif // !defined(AFX_MB_ONEFILE_H__AE83B1CC_79BC_444A_861A_B368DFF9BE3E__INCLUDED_)
