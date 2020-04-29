// TSDBHugeFileObj.h: interface for the CTSDBHugeFileObj class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSDBHUGEFILEOBJ_H__7B40D441_74C2_11D3_B1F1_005004868EAA__INCLUDED_)
#define AFX_TSDBHUGEFILEOBJ_H__7B40D441_74C2_11D3_B1F1_005004868EAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Tsdb.h"

class CTSDBHugeFileObj
{
public:
	BOOL IsSameObj( PTSDBHUGEFILEHEAD pHeader );
	DWORD GetFileLen();
	PBYTE GetDataBuf();
	BOOL Attach( PTSDBHUGEFILEHEAD pHeader );
	BOOL SaveBlock( PTSDBHUGEFILEHEAD pHead, PBYTE pDataBuf );
	float GetPercentage();
	BOOL IsFileOK();
	CTSDBHugeFileObj( PTSDBHUGEFILEHEAD pHeader );
	CTSDBHugeFileObj();
	virtual ~CTSDBHugeFileObj();

public:	
	long Release();
	long AddRef();
	static void ClearHugeFileTmpBuf();
	time_t	m_LastAccessTime;					//	�ϴη���ʱ��
	BOOL m_bMsgSended;							//	�Ѿ����͹���Ϣ

private:
	BOOL IsBlockOK( int nBlockNo );
	BOOL SetBlockNo( int nBlockNo );
	void Init();
	BOOL SetOwnerHandle();
	static PBYTE CreateAndMapFile( LPCSTR pszFileName, DWORD dwFileLen, CFile & file, HANDLE & hOut );

private:
	typedef struct tagFLAGFILE
	{
		BOOL				m_bHasOwner;		//	����������д����,����λ = FASLE, ������, �����ͻ�����ֻ�ܲ�ѯ
		TSDBHUGEFILEHEAD	m_Head;				//	���ļ�����ͷ
		DWORD				m_dwBlockReceived;	//	�ɹ����յ������ļ���
		BOOL				m_bCloseErr;		//	���ļ��Ƿ������ر�
		BYTE				m_abyFlags[1];		//	���ļ����ձ��, ��Ӧλ = 1 ��ʾ���ճɹ�,ÿһ���ض�Ӧ1�����ļ�
	}FLAGFILE, *PFLAGFILE;

private:
	long m_nRef;
	BOOL	m_bIsOwner;							//	�Ƿ�Ϊ����
	PFLAGFILE	m_pFlagBuf;						//	����ļ�������
	PBYTE	m_pDataBuf;							//	�����ļ�������
	HANDLE m_hmapFlagFile;						//	��־�ļ�ӳ����
	HANDLE m_hmapDataFile;						//	�����ļ�ӳ����
	CFile m_FlagFile;							//	����ļ�
	CFile m_DataFile;							//	�����ļ�
};

#endif // !defined(AFX_TSDBHUGEFILEOBJ_H__7B40D441_74C2_11D3_B1F1_005004868EAA__INCLUDED_)
