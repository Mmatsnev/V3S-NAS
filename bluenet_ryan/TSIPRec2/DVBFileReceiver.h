// DVBFileReceiver.h : Declaration of the CDVBFileReceiver

#ifndef __DVBFILERECEIVER_H_
#define __DVBFILERECEIVER_H_

#include "Resource.h"       // main symbols
#include "IPData.h"
#include "TSDBFileSystem.h"
#include "HugeFile.h"
#include "MyIUnknownImpl.h"
#include "FileDelayEventDispatcher.h"

#pragma pack(push,8)

class CFileEventObject
{
public:
	CFileEventObject(){  m_nRef = 0;  };

public:
	CFileObject *	m_pFileObject;
	BOOL	m_bIsSubFile;					//	�Ƿ�Ϊ���ļ�֮���ļ�

public:
	long AddRef()
	{
		return ::InterlockedIncrement( &m_nRef );
	};

	long Release()
	{
		if( ::InterlockedDecrement( &m_nRef ) )
			return m_nRef;
		delete this;
		return 0;
	};

	void Preset()
	{
		m_pFileObject = NULL;
		m_bIsSubFile = FALSE;
	};

private:
	long	m_nRef;
};

/////////////////////////////////////////////////////////////////////////////
// CDVBFileReceiver
class CDVBFileReceiver : public CMyIUnknownImpl<IDVB_EPG_Receiver>,	public CTSDBFileSystem
{
public:
	CDVBFileReceiver();
	virtual ~CDVBFileReceiver();

	enum {
		MAJOR_VERSION = 1,				//	���汾
		MINOR_VERSION = 1,				//	�ΰ汾
		BUILD_NO = 13,					//	�����
	};

	void WriteThread_DoCheckAndSaveFiles();


	virtual void  NotifyOnFileOKEvent( CFileObject * pFileObject );
	virtual void  NotifySubFileOKEvent( CFileObject * pFileObject );

	virtual void SafeDelete()
	{
		Close();
		delete this;
	}

// IDVBFileReceiver
public:
	bool SetDataPortReceiveLog(COneDataPortItem * pDataPortItem, DWORD dwAttrib, DWORD dwFileLen, BOOL bIncFileCount, LPCSTR lpszFileName);
	virtual IIPFileMendHelper * GetIPFileMendHelper( HDATAPORT hDataPort );
	virtual BOOL GetSendNotTSDBFileEvent();
	virtual void SetSendNotTSDBFileEvent( BOOL bNewVal );
	virtual HDATAPORT AddDataPort( void * pDataPort );
	virtual void Close();
	virtual BOOL Init(bool bSaveFileInBackground=false,int varMaxPacketSize=2048);
	virtual LPCSTR GetAutoSavePath();
	virtual void SetAutoSavePath( LPCSTR lpszNewVal );
	virtual long GetDataPortCount();
	virtual long GetFileBPS();
	virtual long GetIPPacketBPS();
	virtual BOOL GetSendSubFileEvent();
    virtual void PutSendSubFileEvent( BOOL bNewVal );
	virtual LPCSTR GetVersion();
	virtual BOOL DeleteDataPort( HDATAPORT hDataPort );
	virtual HDATAPORT CreateDataPort( LPCSTR lpszTargetIP, int nPort, LPCSTR lpszBindIP=NULL, BOOL bIsUDP = TRUE );
	virtual long GetMaxPacketSize();
	virtual void RegisterEventResponser( IDVBReceiverEvent * pObject );
	virtual void DoMessagePump(void);
	virtual float GetProgressInfo( HDATAPORT hDataPort, DWORD & dwBroLoopCount, int & dwFileCount, DWORD & dwTotalLen, DWORD & dwByteReceived, int & nCountReceived );
	virtual BOOL PutSendProgressEvent( BOOL bNewValue );
	virtual BOOL GetSendProgressEvent();
	virtual BOOL GetDotMapFileOnFileOKEvent();
	virtual void SetDoMapFileOnfileOKEvent( BOOL bNewValue );

//	IUnknwon
	virtual DWORD QueryInterface( REFIID iid,void ** ppvObject);

// EPG Recevier
	virtual bool GetEnableOldEPG(){ return false;};
	virtual void SetEnableOldEPG( bool bNewValue ){ bNewValue = false; };

private:
	BOOL m_bMapFileOnFireFileOKEvent;				//	����Hugefile On File OK event ʱ���Ƿ�ӳ���ļ���ȱʡΪ TRUE
	BOOL m_bSendProgressEvent;
	BOOL m_bAutoSave;										//	�Ƿ��Զ�����
	BOOL m_bIsOnLine;
	long m_bFileOKEventDone;								//	File OK �¼�֪ͨ�Ƿ��Ѿ�����
	CString m_strAutoSavePath;
	BOOL m_bSendSubFileEvent;
	DWORD m_dwMaxPacketSize;
#ifdef _WIN32
	CArray<COneDataPortItem*,COneDataPortItem*> m_aDataPortItems;		//	��¼��ǰ���еĶ˿�
	CArray<COneDataPortItem*,COneDataPortItem*> m_aDelayDeleteItems;	//	�ӳ�ɾ���� DataPortItem �ڴ�
#else
	CMyArray<COneDataPortItem*> m_aDataPortItems;		//	��¼��ǰ���еĶ˿�
	CMyArray<COneDataPortItem*> m_aDelayDeleteItems;	//	�ӳ�ɾ���� DataPortItem �ڴ�
#endif //_WIN32
	CLookaheadPacketMgr<CFileEventObject> m_FileNotifyEventList;		//	�ļ����ճɹ����б�

///////////////////////////////////////////////////////////
//	���ļ�
private:
	CHugeFile m_HugeFile;					//	�ļ�����
	time_t	m_HugeFileLastModifyTime;		//	������ʱ�䣬�ж��Ƿ����
	CString	m_strHugeFileName;				//	�ļ���
	DWORD	m_dwHugeFileLen;				//	���ļ��ļ�����
	int		m_nSavePathLen;					//	��¼����·���ĳ���
	DWORD	m_dwHugeFileByteReceived;		//	�Ѿ��ɹ����յ��ֽ���

//////////////////////////////////////////////////////////////
//	ͳ������
private:
	DWORD	m_dwByteReceived;				//	�Ѿ����յ����ļ��ֽ���
	DWORD	m_dwLastTickCount;				//	�ϴ�ͳ�Ƶ�ʱ��
	DWORD	m_dwLastFileBPS;				//	�ϴ�ͳ�Ƶ�����

protected:
	IDVBReceiverEvent * m_pFileEventObject;			// file event

protected:
	virtual void Fire_OnFileOK( IFileObject * pObject, HDATAPORT hDataPort );
	virtual void Fire_OnSubFileOK( IFileObject * pObject, HDATAPORT hDataPort );
	virtual void Fire_OnProgressEvent( COneDataPortItem * pDataPortItem , LPCSTR lpszFileName);

private:
	CString m_strVersion;
	int m_nTimer_2_Second;
	bool m_bSaveFileInBackgound;		//  2004-7-31 �Ƿ�ʹ���ļ������̱߳�������
	CFileDelayEventDispatcher	m_DelayEventDispatcher;

private:
	void PresetVars();
	void OnHugeFileFullyReceived(const char * pszFileName, CFileObject *pFileObject);
	COneDataPortItem * NewOneDataPortItem();
	void CheckAndSendFileEvent();
	bool SaveHugeFile( CFileObject * pFileObject );
	BOOL IsHugeFileChanged(LPCSTR lpszFileName, time_t LastModifyTime,DWORD dwFileLen);
	BOOL PreprareHugeFile(LPCSTR lpszFileName, time_t LastModifyTime,DWORD dwFileLen, int nSubFileCount);
	BOOL MoveHugeToOffset(DWORD dwOffset);

	void DeleteAllDataPort();
	void CleanFileOKQueue();
	void ExecDelayDeleteItems();
	int FindDataPort( HDATAPORT hDataPort );
	int FindDataPort( CString & strIP, long nPort );
};

#pragma pack(pop)

#endif //__DVBFILERECEIVER_H_
