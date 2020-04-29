///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2002-11-14
///
///=======================================================

// DVBFileReceiver.cpp : Implementation of CDVBFileReceiver
/////////////////////////////////////////////////////////////////
//  2003-11-18  ���� put_bstrAutoSavePath Ϊ���ַ���
//	2002.11.14	�޸� OnOneFileOK���ͷŴ��ļ������ Detatch �ڴ�
//				�޸� NotifyOnFileOnEvent��NotifyOnSubFileOKEvent��AddPacket ʧ������µĴ���
//				����İ汾 => 1.01.001

#include "stdafx.h"
#include "IPRecSvr.h"
#include "DVBFileReceiver.h"
#include "DecoderThread.h"

#include "UDPDataPortLinux.h"
#include "UDPRecThread.h"
#include "FileWriterThread.h"

#include "DirectroyHelp.h"
#include <time.h>
#include <MyMapFile.h>

CDecoderThread			g_DecoderThread;
CUDPRecThread 			g_UDPRecThread;
CFileWriterThread		g_FileWriterThread;			//  2004-7-31 �Զ������ļ��߳�


#ifdef _WIN32
    #ifdef _DEBUG
    #undef THIS_FILE
    static char THIS_FILE[]=__FILE__;
    #define new DEBUG_NEW
    #endif
#endif //_WIN32

#ifdef _WIN32
	extern "C" IDVBFileReceiver * WINAPI CreateDVBFileReceiver(void)
#else
	extern "C" IDVBFileReceiver * CreateDVBFileReceiver(void)
#endif //_WIN32
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	CDVBFileReceiver * pInstance = new CDVBFileReceiver;
	if( NULL == pInstance )
		return NULL;
	pInstance->AddRef();
	return static_cast<IDVBFileReceiver*>(pInstance);
}


#pragma pack( push, 1 )
typedef union tagEQUFILEATTRIBUTE	//	ͨ���ļ����Ի�ȡ
{
	struct
	{
		DWORD m_dwValue:24;			//	�ļ�����/�ܸ���
		DWORD m_dwID:6;				//	��������ʾ����
		DWORD m_dwIsFileCount:1;	//	�Ƿ�Ϊ�ļ��ܸ�����=0 �ļ����ֽ���
		DWORD m_dwHasExtData:1;		//	ת�Ʊ�ʾ�����ļ����ֽ������ļ�����
	};
	DWORD	m_dwData;
}EQUFILEATTRIBUTE,*PEQUFILEATTRIBUTE;
#pragma pack( pop )

/////////////////////////////////////////////////////////////////////////////
// CDVBFileReceiver

CDVBFileReceiver::CDVBFileReceiver()
{
	m_bSaveFileInBackgound = false;
	PresetVars();
}
CDVBFileReceiver::~CDVBFileReceiver()
{
    ExecDelayDeleteItems();			//	ִ���ӳٵĲ���
    ASSERT( 0 == m_nTimer_2_Second );

	RegisterEventResponser( NULL );
}

DWORD CDVBFileReceiver::QueryInterface( REFIID iid,void ** ppvObject)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( IID_IDVBFileReceiver == iid )
	{
		AddRef();
		*ppvObject = static_cast<IDVBFileReceiver*>(this);
		return 0;	//	S_OK;
	}

	return CMyIUnknownImpl<IDVB_EPG_Receiver>::QueryInterface( iid, ppvObject );
}

long CDVBFileReceiver::GetMaxPacketSize()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_dwMaxPacketSize;
}

//////////////////////////////////////////////
//����:
//		���UDP/TCPһ�����ݶ˿�
//��ڲ���:
//		bstrTargetIP			Ŀ�� IP ��ַ
//		nPort					�˿�
//		varLocalBindIP			���ذ� IP��Ĭ�� �� NULL�����Զ�
//		barIsUDP				�Ƿ� UDP ���ݰ���ȱʡ �� TRUE
//		pVal					����˿ھ��
//								NULL			ʧ��
//								����			�ɹ���ɾ���˿���Ҫ���ø���ֵ
//���ز���:
//		��
HDATAPORT CDVBFileReceiver::CreateDataPort( LPCSTR lpszTargetIP, int nPort, LPCSTR lpszBindIP, BOOL bIsUDP )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	ASSERT( m_bIsOnLine );
	if( FALSE == m_bIsOnLine )
		return NULL;			//	û������

	ASSERT( lpszTargetIP && nPort >= 1024 );
	if( NULL == lpszTargetIP || nPort < 1024 )
		return NULL;
	if( FALSE == bIsUDP )
		return NULL;					//	Ŀǰ��û��ʵ��

	CString strLocalBind = lpszBindIP;

	COneDataPortItem * pDataPortItem = NULL;
	TRY
	{
		CString strIP = lpszTargetIP;
		int nItemNo = FindDataPort( strIP, nPort );	//	�����Ƿ������й�
		ASSERT( nItemNo < m_aDataPortItems.GetSize() );
		if( nItemNo >= 0 )
        	return NULL;				// exist already

        pDataPortItem = NewOneDataPortItem();
        if( NULL == pDataPortItem )
            return NULL;				// allocate memory failed

        pDataPortItem->m_strTargetIP = strIP;
        pDataPortItem->m_nPort = nPort;

		pDataPortItem->m_pDataPort = NULL;		// indicate this dataport is a UDP socket port

		pDataPortItem->m_pFileObjMgr = static_cast< CTSDBFileSystem * >(this);

#ifdef _DEBUG
		printf("To callg_UDPRecThread.AddDataPort\n");
#endif //_DEBUG

		g_UDPRecThread.AddDataPort( strIP, nPort, lpszBindIP, pDataPortItem );

#ifdef _DEBUG
		printf("return from add data port\n");
#endif //_DEBUG

		m_aDataPortItems.Add( pDataPortItem );
	}
	CATCH_ALL( e )
	{
		if( pDataPortItem )
		{
			if( pDataPortItem->m_pFileMendHelper )			//  2003-4-11 ���
				pDataPortItem->m_pFileMendHelper->Release();
			pDataPortItem->m_pFileMendHelper = NULL;

			delete pDataPortItem;
		}
		return NULL;
	}
	END_CATCH_ALL

	return (HDATAPORT)pDataPortItem;
}

//////////////////////////////////////////////
//����:
//		֪ͨ�ļ����� OK
//��ڲ���:
//		pFileObject				��������ļ�����
//���ز���:
//		��
//ע��
//		���ļ�����һ�����ļ�ʱ��pHugeFileObject NULL
//�޸ļ�¼��
//	2002.11.14 ���� AddPacket ʧ�ܵ����
void CDVBFileReceiver::NotifyOnFileOKEvent( CFileObject * pFileObject )
{
	CFileEventObject * pFileEvent = m_FileNotifyEventList.AllocatePacket();
	ASSERT( pFileEvent );
	if( NULL == pFileEvent )
		return;
	pFileEvent->m_pFileObject = pFileObject;
	ASSERT( FALSE == pFileEvent->m_bIsSubFile );
	if( false == m_FileNotifyEventList.AddPacket( pFileEvent ) )
	{
		ASSERT( FALSE );						//	�������󣬷���
		m_FileNotifyEventList.DeAllocate( pFileEvent );
	}

	pFileEvent->Release();
}

//////////////////////////////////////////////
//����:
//		�ύһ�����ļ��ɹ����� OK
//��ڲ���:
//		pFileObject				���ļ�����
//���ز���:
//		��
//�޸ļ�¼��
//	2002.11.14 ���� AddPacket ʧ�ܵ����
void CDVBFileReceiver::NotifySubFileOKEvent( CFileObject * pFileObject )
{
	ASSERT( pFileObject && pFileObject->m_pHugeFileParam );

	CFileEventObject * pFileEvent = m_FileNotifyEventList.AllocatePacket();
	ASSERT( pFileEvent );
	if( NULL == pFileEvent )
		return;
	pFileEvent->m_pFileObject = pFileObject;
	pFileEvent->m_bIsSubFile = TRUE;
	if( false == m_FileNotifyEventList.AddPacket( pFileEvent ) )
	{
		ASSERT( FALSE );				//	��������
		m_FileNotifyEventList.DeAllocate( pFileEvent );
	}

	pFileEvent->Release();
}

//////////////////////////////////////////////
//����:
//		ɾ�����ݶ˿�
//��ڲ���:
//		hDataPort			�������CreateDataPort �� AddDataPort ����ֵ
//		pVal				����ɹ����
//���ز���:
//
BOOL CDVBFileReceiver::DeleteDataPort( HDATAPORT hDataPort )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	ASSERT( m_bIsOnLine );
	if( FALSE == m_bIsOnLine )
		return FALSE;			//	û������

	int nItemNo = FindDataPort( hDataPort );
	if( nItemNo < 0 )
		return FALSE;

	COneDataPortItem * pItem = m_aDataPortItems[nItemNo];
	ASSERT( pItem );

    if( NULL == pItem->m_pDataPort )	// a Socket data port,
	    g_UDPRecThread.DeleteDataPort( pItem->m_strTargetIP, pItem->m_nPort );
    else
    {
    	ASSERT( FALSE );		// deal with MyDataPort
    }

	m_aDataPortItems.RemoveAt( nItemNo );

	if( pItem->m_pFileMendHelper )
	{										//  2003-4-11 ��ӣ�ɾ���޲�����
		pItem->m_pFileMendHelper->Release();
		pItem->m_pFileMendHelper = NULL;
	}

	m_aDelayDeleteItems.Add( pItem );		//	�ӳٲ�����ɾ��

	if( pItem->m_pDataPort )				//	ɾ���˿�
	{
		pItem->m_pDataPort->Release();
		pItem->m_pDataPort = NULL;
	}

	return TRUE;
}

//////////////////////////////////////////////
//����:
//		�����Ƿ������й��ö˿�
//��ڲ���:
//		strIP		Ŀ�� IP
//		nPort		�˿�
//���ز���:
//		-1			ʧ��
//		>=0			���
int CDVBFileReceiver::FindDataPort(CString &strIP, long nPort)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	int nCount = m_aDataPortItems.GetSize();
	for(int i=0; i<nCount; i++)
	{
		COneDataPortItem * pItem = m_aDataPortItems[i];
		ASSERT( pItem );
		if( pItem->m_nPort != nPort )
			continue;
		if( pItem->m_strTargetIP != strIP )
			continue;

		return i;
	}
	return -1;
}

//////////////////////////////////////////////
//����:
//		��ȡ����İ汾
//��ڲ���:
//		pVal			����汾�ַ���
//���ز���:
//
LPCSTR CDVBFileReceiver::GetVersion()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	m_strVersion.Format("%d.%02d.%03d", MAJOR_VERSION,MINOR_VERSION,BUILD_NO  );

	return (LPCSTR)m_strVersion;
}

BOOL CDVBFileReceiver::GetSendSubFileEvent()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_bSendSubFileEvent;

}

void CDVBFileReceiver::PutSendSubFileEvent( BOOL bNewVal )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	m_bSendSubFileEvent = bNewVal;
}

long CDVBFileReceiver::GetIPPacketBPS()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	int nCount = m_aDataPortItems.GetSize();
	DWORD dwBPS = 0;
	for(int i=0; i<nCount; i++)
	{
		dwBPS += m_aDataPortItems[i]->m_FileCombiner.GetIPBPS();
	}

	return dwBPS;
}

long CDVBFileReceiver::GetFileBPS()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	DWORD dwNow = (DWORD)time( NULL );
	DWORD dwDelta = dwNow - m_dwLastTickCount;
	if( dwDelta && (dwDelta >= 10 || 0 == m_dwLastFileBPS) )			//	���̫С��ʹ����һ�ε�����
	{
		m_dwLastFileBPS = ( m_dwByteReceived * 8 ) / dwDelta;		//	125*64 = 8 * 1000
		m_dwLastTickCount = ( m_dwLastTickCount + dwNow ) / 2;
		m_dwByteReceived /= 2;
	}

	return m_dwLastFileBPS;
}

long  CDVBFileReceiver::GetDataPortCount()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_aDataPortItems.GetSize();
}

LPCSTR CDVBFileReceiver::GetAutoSavePath()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return (LPCSTR)m_strAutoSavePath;
}

//  2003-11-18 ����ʹ�ÿ��ַ���
void  CDVBFileReceiver::SetAutoSavePath( LPCSTR lpszNewVal )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( lpszNewVal )
		m_strAutoSavePath = lpszNewVal;
	else
		m_strAutoSavePath = "";
	if( m_strAutoSavePath.IsEmpty() )
	{										//  2003-11-18 ������ַ���
		m_bAutoSave = FALSE;
		return;						//	���Զ�����
	}

	int nLen = m_strAutoSavePath.GetLength();
#ifdef _WIN32
	m_strAutoSavePath.Replace( '/', '\\');
	if( nLen && m_strAutoSavePath[nLen-1] != '\\' )
		m_strAutoSavePath += '\\';					//	��һ���� '\' ��β
#else
	m_strAutoSavePath.Replace( '\\', '/');
	if( nLen && m_strAutoSavePath[nLen-1] != '/' )
		m_strAutoSavePath += '/';					//	��һ���� '\' ��β
#endif //_WIN32

	m_bAutoSave = ( 0 != nLen );
}

///-------------------------------------------------------
/// CYJ,2004-7-31
/// Function:
///		Initialize
/// Input parameter:
///		varMaxPacketSize			max packet size, ignore currently, use 2K instead
///		bSaveFileInBackground		save file data in background thead( CFileWriterThread )
///									default is faluse, in the case, the notification file object's databuf = NULL
/// Output parameter:
///		None
BOOL CDVBFileReceiver::Init(bool bSaveFileInBackground,int varMaxPacketSize)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	ASSERT( FALSE == m_bIsOnLine );

	if( m_bIsOnLine )
		return TRUE;
	PresetVars();

	m_strAutoSavePath = "";				//	�Ȳ��Զ�����

	m_dwMaxPacketSize = varMaxPacketSize;
	if( 0 == m_dwMaxPacketSize )
		m_dwMaxPacketSize = 2048;
	m_bIsOnLine = TRUE;

	if( false == IPRD_Init() )
		return FALSE;

	m_bSaveFileInBackgound = bSaveFileInBackground;
	if( m_bSaveFileInBackgound )			//  2004-7-31 �Զ�д�ļ�����ӵ������߳���
		g_FileWriterThread.Add( this );

	return TRUE;
}

void CDVBFileReceiver::Close()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	RegisterEventResponser( NULL );

	if( FALSE == m_bIsOnLine )
		return;

	if( m_bSaveFileInBackgound )			//  2004-7-31 ��д�ļ��߳���ɾ��
		g_FileWriterThread.Remove( this );

	DeleteAllDataPort();

	m_bIsOnLine = FALSE;

	CleanFileOKQueue();				//	ɾ����û�з��͵��ļ�

	if( m_HugeFile.m_hFile != CFile::hFileNull )
	{
		TRY
		{
			m_HugeFile.Close();
		}
		CATCH( CFileException, e )
		{
			m_HugeFile.Abort();
		}
		END_CATCH
	}

	IPRD_Close();
}

//////////////////////////////////////////////
//����:
//		���һ������Դ�˿�
//��ڲ���:
//		pSrcDataPort		����Դ�˿�
//		pVal				���ݾ��
//	ע��
//		pSrcDataPort		�Ѿ���ʼ���������Խ��ճ���
HDATAPORT CDVBFileReceiver::AddDataPort( void * pDataPort )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32
/*
	CSrcDataPort * pSrcDataPort = (CSrcDataPort *)pDataPort;
	ASSERT( pSrcDataPort );
	if( NULL == pSrcDataPort )
		return 0;

	ASSERT( m_bIsOnLine );
	if( FALSE == m_bIsOnLine )
		return 0;			//	û������

	COneDataPortItem * pDataPortItem = NewOneDataPortItem();
	if( NULL == pDataPortItem )
		return 0;

	CSrcDataPort * pSrcPort = (CSrcDataPort*)pSrcDataPort;

	pDataPortItem->m_strTargetIP = pSrcPort->m_szIPAddress;
	pDataPortItem->m_nPort = pSrcPort->m_wPort;

	pDataPortItem->m_pDataPort = pSrcPort;
	pSrcPort->AddRef();							//	�������ü�����
	pDataPortItem->m_pFileObjMgr = static_cast< CTSDBFileSystem * >(this);

	g_ThreadMgr.m_pReadThread->AddDataPort( pDataPortItem );

	m_aDataPortItems.Add( pDataPortItem );

	return (long)pDataPortItem;
*/
	ASSERT( FALSE );		// do more here
	return NULL;
}

//////////////////////////////////////////////
//����:
//		���Ҷ˿ھ��
//��ڲ���:
//		hDataPort			����Դ�˿�
//���ز���:
//		-1			ʧ��
//		>=0			���
int CDVBFileReceiver::FindDataPort(HDATAPORT hDataPort)
{
	COneDataPortItem * pDataPort = (COneDataPortItem *)hDataPort;

	int nCount = m_aDataPortItems.GetSize();
	for(int i=0; i<nCount; i++)
	{
		COneDataPortItem * pItem = m_aDataPortItems[i];
		ASSERT( pItem );
		if( (HDATAPORT)pItem == hDataPort )
			return i;
	}
	return -1;
}

//////////////////////////////////////////////
//����:
//		ɾ�����е�����Դ�˿ڣ���Ϊ�����˳�
//��ڲ���:
//		��
//���ز���:
//		��
void CDVBFileReceiver::DeleteAllDataPort()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	int nCount = m_aDataPortItems.GetSize();
	for(int i=0; i<nCount; i++)
	{
		COneDataPortItem * pItem = m_aDataPortItems[i];
		ASSERT( pItem );
        if( NULL == pItem->m_pDataPort )	// UDP socket
        	g_UDPRecThread.DeleteDataPort( pItem->m_strTargetIP, pItem->m_nPort );
		else
        {
        	ASSERT(FALSE);		// do more here
		}

		m_aDelayDeleteItems.Add( pItem );		//	�ӳ�ɾ������

		if( pItem->m_pFileMendHelper )
		{										//  2003-4-11 ��ӣ�ɾ���޲�����
			pItem->m_pFileMendHelper->Release();
			pItem->m_pFileMendHelper = NULL;
		}

		if( pItem->m_pDataPort )				//	ɾ���˿�
		{
			pItem->m_pDataPort->Release();
			pItem->m_pDataPort = NULL;
		}
	}
	m_aDataPortItems.RemoveAll();
}

//	�ж��Ƿ��Ѿ�����
//	��ڲ���
//		lpszFileName			�ļ���
//		LastModifyTime			����޸�ʱ��
//		dwFileLen				�ļ�����
//	���ز���
//		TRUE					���£���Ҫ���´��ļ�
//		FALSE					û�и��£����Լ���д����
BOOL CDVBFileReceiver::IsHugeFileChanged(LPCSTR lpszFileName, time_t LastModifyTime,DWORD dwFileLen)
{
	ASSERT( lpszFileName );
#ifdef _WIN32
    ASSERT( lpszFileName[1] == ':' );
#else
	ASSERT( lpszFileName[0] == '/' );
#endif //_WIN32
	if( LastModifyTime == m_HugeFileLastModifyTime && \
		m_dwHugeFileLen == dwFileLen &&\
		0 == m_strHugeFileName.CompareNoCase( lpszFileName ) &&\
		m_HugeFile.m_hFile != CFile::hFileNull )
	{
		return FALSE;
	}
	if( m_HugeFile.m_hFile != CFile::hFileNull  )
	{
		TRY
		{
			m_HugeFile.Close();
		}
		CATCH( CFileException, e )
		{
#if defined(_DEBUG) && defined(_WIN32)
			e->ReportError();
#endif // _DEBUG
			m_HugeFile.Abort();
			return TRUE;
		}
		END_CATCH
	}
	return TRUE;
}

//	׼�����ļ�����
//	��ڲ���
//		lpszFileName			�ļ�����ȫ·��
//		LastModifyTime			������ʱ��
//		dwFileLen				�ļ�����
//		nSubFileCount			���ļ�������2003-8-7 ���
//	���ز���
//		TRUE					�ɹ�
//		FALSE					�����ļ�ʧ��
//	�޸ļ�¼
//		2003-8-7 �����ڲ��� nSubFileCount
BOOL CDVBFileReceiver::PreprareHugeFile(LPCSTR lpszFileName, time_t LastModifyTime,DWORD dwFileLen, int nSubFileCount)
{
	ASSERT( lpszFileName );
#ifdef _WIN32
    ASSERT( lpszFileName[1] == ':' );
#else
	ASSERT( lpszFileName[0] == '/' );
#endif //_WIN32

	ASSERT( m_HugeFile.m_hFile == CFile::hFileNull );
	m_HugeFileLastModifyTime = LastModifyTime;
	m_strHugeFileName = lpszFileName;
	m_dwHugeFileLen = dwFileLen;
	m_dwHugeFileByteReceived = 0;				//	׼����ʼ����

	CDirectroyHelp::Mkdir( lpszFileName );

	m_HugeFile.SetHugeFileParameter( dwFileLen, LastModifyTime, nSubFileCount );

	if( FALSE == m_HugeFile.Open( lpszFileName, CFile::modeNoTruncate|CFile::modeCreate|\
                        CFile::modeWrite|CFile::typeBinary|CFile::shareDenyWrite ) )
	{
		return FALSE;
	}
	if( m_HugeFile.GetLength() > dwFileLen )
		m_HugeFile.SetLength( dwFileLen );


	return TRUE;
}

//	�ƶ��ļ�ָ�뵽ָ��λ�ã����������򴴽�
BOOL CDVBFileReceiver::MoveHugeToOffset(DWORD dwOffset)
{
	BOOL bRetVal = TRUE;
	TRY
	{
		DWORD dwCurPos = m_HugeFile.GetPosition();
		if( dwOffset == dwCurPos )
			return TRUE;
		else if( dwOffset > m_HugeFile.GetLength() )
			m_HugeFile.SetLength( dwOffset );
		else
			m_HugeFile.Seek( dwOffset, CFile::begin );
	}
	CATCH( CFileException,e)
	{
#ifdef _DEBUG_
		e->ReportError();
#endif // _DEBUG
		bRetVal = FALSE;
		m_HugeFile.Abort();
	}
	END_CATCH
	return bRetVal;
}

//////////////////////////////////////////////
//����:
//		������ļ���ֻ���Զ����������²ŵ��øú���
//��ڲ���:
//		pFileObject		(��)�ļ�����
//���ز���:
//		true			�������ļ��ɹ�����
///		false			û�н��ճɹ�
//�޸ļ�¼��
//  2004-5-26		�����ļ��ɹ����գ����� OnFileOK �¼�
bool CDVBFileReceiver::SaveHugeFile(CFileObject *pFileObject)
{
	ASSERT( FALSE == m_strAutoSavePath.IsEmpty() );
	ASSERT( m_bSendSubFileEvent );
#ifdef _WIN32
	ASSERT( m_strAutoSavePath[ m_strAutoSavePath.GetLength()-1 ] == '\\' );
#else
	ASSERT( m_strAutoSavePath[ m_strAutoSavePath.GetLength()-1 ] == '/' );
#endif //_WIN32
	ASSERT( pFileObject->m_pHugeFileParam );

	time_t LastModifyTime = 0;
	if( pFileObject->m_pAttributeData )
		LastModifyTime = pFileObject->m_pAttributeData->m_LastWriteTime;
	DWORD FileLen = pFileObject->m_pHugeFileParam->m_dwFileLen;

	CString strTmp = m_strAutoSavePath;
    strTmp += pFileObject->m_strFileName;
	bool bIncreaseFileCount = false;

	if( IsHugeFileChanged( strTmp, LastModifyTime, FileLen ) )
	{
		int nSubFileCount = pFileObject->m_pHugeFileParam->m_wTotalBlock;
		if( FALSE == PreprareHugeFile( strTmp, LastModifyTime, FileLen, nSubFileCount ) )
			return false;
		bIncreaseFileCount = true;
	}

	if( FALSE == MoveHugeToOffset( pFileObject->m_pHugeFileParam->m_dwFilePosition ) )
		return false;							//	�ƶ�ָ��ʧ��

	bool bRetVal = false;
	TRY
	{
		ASSERT( m_HugeFile.m_hFile != CFile::hFileNull );
		m_HugeFile.Write( pFileObject->GetBuffer(), pFileObject->m_pHugeFileParam->m_wBlockSize );
		bRetVal = m_HugeFile.NotifyOneSubFileOK( pFileObject->m_pHugeFileParam->m_wBlockNo );
	}
	CATCH(CFileException,e)
	{
#if defined(_DEBUG) && defined(_WIN32)
		e->ReportError();
#endif // _DEBUG
		m_HugeFile.Abort();
		return false;
	}
	END_CATCH

	SetDataPortReceiveLog(  pFileObject->m_pDataPortItem, pFileObject->GetAttribute(), \
	pFileObject->GetDataLen(), bIncreaseFileCount, pFileObject->GetFileName() );

	return bRetVal;
}

//////////////////////////////////////////////
//����:
//		ִ���ӳٵ�ɾ������
//��ڲ���:
//		��
//���ز���:
//		��
void CDVBFileReceiver::ExecDelayDeleteItems()
{
	int nCount = m_aDelayDeleteItems.GetSize();
	for(int i=0; i<nCount; i++)
	{
		COneDataPortItem * pItem = m_aDelayDeleteItems[i];
		ASSERT( NULL == pItem->m_pFileMendHelper );	//  2003-4-11 ���
		delete pItem;
	}
	m_aDelayDeleteItems.RemoveAll();
}

//////////////////////////////////////////////
//����:
//		��� OnFileOK �е��ļ�������
//��ڲ���:
//		��
//���ز���:
//		��
void CDVBFileReceiver::CleanFileOKQueue()
{
#ifdef _DEBUG
	BOOL bShown = FALSE;
#endif // _DEBUG
	for(int i=0; i<0xFFFF; i++)								//	��ֹ��ѭ��
	{
		CFileEventObject * pFileEvent = m_FileNotifyEventList.PeekData( 0 );
		if( NULL == pFileEvent )
			return;

#ifdef _DEBUG
		if( FALSE == bShown )
			TRACE("CleanFileOKQueue, last active file is release.\n");
		bShown = TRUE;
#endif // _DEBUG

		DeAllocate( pFileEvent->m_pFileObject );			//	�ͷ��ļ�����
		pFileEvent->m_pFileObject->Release();				//	�ͷ�

		m_FileNotifyEventList.DeAllocate( pFileEvent );		//	�ع����
		pFileEvent->Release();
	}
}

//----------------------------------------------------------
//	�Ƿ񴥷��� TSDB �ļ��¼�
BOOL CDVBFileReceiver::GetSendNotTSDBFileEvent()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_bIsEnableNotTSDBFile;
}

//----------------------------------------------------------
//	�Ƿ񴥷��� TSDB �ļ��¼�
void CDVBFileReceiver::SetSendNotTSDBFileEvent( BOOL bNewVal )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	m_bIsEnableNotTSDBFile = bNewVal;

}

//-----------------------------------------------
//	2002.12.28
//	��Ⲣ�����ļ�OK�¼�
void CDVBFileReceiver::CheckAndSendFileEvent()
{
	for(int i=0; i<1000; i++)				//	2002.7.5��10��̫���ˣ���Լ 1000 �Σ���һ����Ҫ 10 �Σ���ֹ����
	{
		CFileEventObject * pFileEvent = m_FileNotifyEventList.PeekData( 0 );
		if( NULL == pFileEvent )
			return;

		ASSERT( pFileEvent->m_pFileObject );

		if( m_dwByteReceived >= 0x7FFFFFFF )
		{														//	����Խ�磬���봦��
			m_dwLastTickCount = ( m_dwLastTickCount + time(NULL) ) / 2;
			m_dwByteReceived /= 2;
		}

		//	�ڴ˵����¼��������¼��б��봦��������
		if( FALSE == pFileEvent->m_bIsSubFile )
		{
			if( m_bAutoSave )
				pFileEvent->m_pFileObject->SaveTo( m_strAutoSavePath, FALSE, TRUE );	//	�����ļ�
			SetDataPortReceiveLog( pFileEvent->m_pFileObject->m_pDataPortItem,
				pFileEvent->m_pFileObject->GetAttribute(), pFileEvent->m_pFileObject->GetDataLen(),
				true, pFileEvent->m_pFileObject->GetFileName() );

			Fire_OnFileOK( static_cast<IFileObject*>( pFileEvent->m_pFileObject), \
				(HDATAPORT)pFileEvent->m_pFileObject->m_pDataPortItem );
			m_dwByteReceived += pFileEvent->m_pFileObject->GetDataLen();		//	���ļ������ӣ��ɴ��ļ������ļ�����
		}
		else
		{		//	SetDataPortReceiveLog will be called in function SaveHugeFile
			bool bFullHugeFileReceived = false;
			if( m_bAutoSave )
				bFullHugeFileReceived = SaveHugeFile( pFileEvent->m_pFileObject );
			if( m_bSendSubFileEvent )
			{
				Fire_OnSubFileOK( static_cast<IFileObject*>( pFileEvent->m_pFileObject),\
					(HDATAPORT)( pFileEvent->m_pFileObject->m_pDataPortItem ) );
			}
			m_dwByteReceived += pFileEvent->m_pFileObject->GetDataLen();			//	������յ����ֽ���
			if( bFullHugeFileReceived )
			{									//  2004-7-5 huge file is received
				OnHugeFileFullyReceived(  m_strHugeFileName, pFileEvent->m_pFileObject );
			}
		}

		int nFreeItemCount = GetItemCountInFreeList( FALSE );	//	2002.7.5 ��ӣ�������̫��ʱ���ͷ��ڴ�
		if( pFileEvent->m_pFileObject->IsBufAttaced() )
		{														//	2002.11.14 �ͷŴ��ļ��� Attached
			ASSERT( pFileEvent->m_pFileObject->m_pHugeFileParam );	//	һ����˵�����Ǵ��ļ�
			DWORD dwBufLenTmp;
			pFileEvent->m_pFileObject->Detach( dwBufLenTmp );
		}
		pFileEvent->m_pFileObject->PresetVar();				//	2002.5.22 ���
		if( nFreeItemCount >= 30 )
		{														//	�ͷŲ���Ҫ���ڴ档
			pFileEvent->m_pFileObject->SetBufSize( 0 );
			DeAllocateEx( pFileEvent->m_pFileObject, TRUE );			//	�ͷ��ļ�����
			pFileEvent->m_pFileObject->Release();
		}
		else
		{
			DeAllocate( pFileEvent->m_pFileObject );			//	�ͷ��ļ�����
#ifdef _DEBUG
			ASSERT( pFileEvent->m_pFileObject->Release() > 0 );
#else
			pFileEvent->m_pFileObject->Release();
#endif //_DEBUG
		}

		m_FileNotifyEventList.DeAllocate( pFileEvent );		//	�ع����
		pFileEvent->Release();
	}
}


///-------------------------------------------------------
/// 2003-4-10
/// ���ܣ�
///		��ȡ IP File Mend helper
/// ��ڲ�����
///
/// ���ز�����
///
IIPFileMendHelper * CDVBFileReceiver::GetIPFileMendHelper( HDATAPORT hDataPort )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	ASSERT( m_aDataPortItems.GetSize() );

	int nItemNo;
	int nDataItemCount = m_aDataPortItems.GetSize();
	if( 0 == nDataItemCount )
		return NULL;

	if( 1 == nDataItemCount )
		nItemNo = 0;					// ֻ��һ�����ݶ˿ڣ����ý��в�ѯ
	else
	{									//	���ڶ�����ݶ˿ڣ�����
		long nDataPortIndex = (long)hDataPort;
		if( nDataPortIndex < 100 && nDataPortIndex >= 0 && nDataPortIndex < nDataItemCount )
			nItemNo = (int)nDataPortIndex;		//	���������±���ʹ��
		else if( m_aDataPortItems.GetSize() > 1 )
			nItemNo = FindDataPort( hDataPort );
		if( nItemNo < 0 )
			return NULL;
	}

	ASSERT( nItemNo < m_aDataPortItems.GetSize() );
	COneDataPortItem * pDataPortItem = m_aDataPortItems[ nItemNo ];
	ASSERT( pDataPortItem );
#ifdef _WIN32
	ASSERT( FALSE == ::IsBadWritePtr( pDataPortItem, sizeof(COneDataPortItem) ) );
#endif //_WIN32

	if( NULL == pDataPortItem->m_pFileMendHelper )
		return NULL;

	IIPFileMendHelper * pRetVal = NULL;

	if( 0 != pDataPortItem->m_pFileMendHelper->QueryInterface(IID_IIPFileMendHelper, (void**)&pRetVal ) )
		return NULL;
	return pRetVal;
}

///-------------------------------------------------------
/// 2003-4-11
/// ���ܣ�
///		����һ�����ݶ˿ڶ���
/// ��ڲ�����
///		��
/// ���ز�����
///		NULL				ʧ��
///		����				�ɹ�
COneDataPortItem * CDVBFileReceiver::NewOneDataPortItem()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	COneDataPortItem * pDataPortItem = NULL;
	if( m_aDelayDeleteItems.GetSize() )
	{										//	�����ӳ�ɾ���Ķ������ǿ�������ʹ��
		pDataPortItem = m_aDelayDeleteItems[0];
#ifdef _WIN32
		ASSERT( FALSE == ::IsBadWritePtr(pDataPortItem,sizeof(COneDataPortItem) ) );
#endif //_WIN32
		m_aDelayDeleteItems.RemoveAt( 0 );
#ifdef _WIN32
		ConstructElements( pDataPortItem,1 );
#else
		MyConstructElements( pDataPortItem,1 );
#endif //_WIN32
	}
	else
		pDataPortItem = new COneDataPortItem;
	if( NULL == pDataPortItem )
		return NULL;

	CIPFileMendHelper * pInstance = new CIPFileMendHelper;
	if( NULL == pInstance )
			return NULL;
	pInstance->AddRef();
	pDataPortItem->m_pFileMendHelper = pInstance;

	return pDataPortItem;
}

//-----------------------------------------------
// on file oK
void CDVBFileReceiver::Fire_OnFileOK( IFileObject * pObject, HDATAPORT hDataPort )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

#ifdef _DEBUG
	TRACE("Fire_OnFileOK is called. FileName=%s,Len=%d\n", pObject->GetFileName(), pObject->GetDataLen() );
#endif //_DEBUG

	if( NULL == m_pFileEventObject )
		return;

	pObject->AddRef();
	m_pFileEventObject->OnFileOK( pObject, hDataPort );
	pObject->Release();
}

//------------------------------------------------
// on sub-file oK
void CDVBFileReceiver::Fire_OnSubFileOK( IFileObject * pObject, HDATAPORT hDataPort )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

#ifdef _DEBUG
	TRACE("Fire_OnSubFileOK is called. FileName=%s,Len=%d\n", pObject->GetFileName(), pObject->GetDataLen() );
#endif //_DEBUG

	if( NULL == m_pFileEventObject )
		return;

	pObject->AddRef();
	m_pFileEventObject->OnSubFileOK( pObject, hDataPort );
	pObject->Release();
}

//------------------------------------------------
// register event responeser object
void CDVBFileReceiver::RegisterEventResponser( IDVBReceiverEvent * pObject )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( FALSE == m_bSaveFileInBackgound )
	{
		if( m_pFileEventObject )
			m_pFileEventObject->Release();
		m_pFileEventObject = pObject;
		if( pObject )
			pObject->AddRef();
	}
	else
	{
		m_DelayEventDispatcher.SetHandler( pObject );
		if( pObject )
			m_pFileEventObject = static_cast<IDVBReceiverEvent*>( &m_DelayEventDispatcher );
		else
			m_pFileEventObject = NULL;
	}
}

void CDVBFileReceiver::DoMessagePump(void)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( FALSE == m_bSaveFileInBackgound )
		CheckAndSendFileEvent();
	else
		m_DelayEventDispatcher.DispatchEvents();
}

///-------------------------------------------------------
/// CYJ,2004-5-20
/// Function:
///		set ont channel receive status
/// Input parameter:
///		None
/// Output parameter:
///		TRUE				changed
///		false				not changed
bool CDVBFileReceiver::SetDataPortReceiveLog(COneDataPortItem * pDataPortItem, DWORD dwAttrib, DWORD dwFileLen, BOOL bIncFileCount, LPCSTR lpszFileName )
{
	ASSERT( pDataPortItem );
	if( NULL == pDataPortItem )
		return false;
	EQUFILEATTRIBUTE	equLen;
	equLen.m_dwData = (DWORD)dwAttrib;
	if( FALSE == equLen.m_dwHasExtData )
		return false;				//	����������

	COneDataPortItem::FILERECEIVELOG & ReceiveLog = pDataPortItem->m_ReceiveLog;

	bool bIsChanged = FALSE;
	if( ReceiveLog.m_dwID != equLen.m_dwID )				//	����
	{
#if defined(_DEBUG) || defined(_MYDEBUG)
		TRACE("LogID Changed, %X==>%X, NewFileLen=%d,%s=%d\n",\
			ReceiveLog.m_dwID, equLen.m_dwID, dwFileLen, \
			equLen.m_dwIsFileCount ? "Len" : "Count", equLen.m_dwValue );
#endif
		memset( &ReceiveLog, 0, sizeof(ReceiveLog) );
		bIsChanged = TRUE;
	}

	ReceiveLog.m_dwID = equLen.m_dwID;

	if( equLen.m_dwIsFileCount )
		ReceiveLog.m_dwFileCount = equLen.m_dwValue;
	else
		ReceiveLog.m_dwTotalLen_16KB = equLen.m_dwValue;

	ReceiveLog.m_dwOkLen_Below16KB += dwFileLen;
	ReceiveLog.m_dwOKLen_16KB += (ReceiveLog.m_dwOkLen_Below16KB >> 14);
	ReceiveLog.m_dwOkLen_Below16KB &= 0x3FFF;
	float fOldProgress = ReceiveLog.m_fProgress;
	if( ReceiveLog.m_dwTotalLen_16KB )
	{
		ReceiveLog.m_fProgress = float(ReceiveLog.m_dwOKLen_16KB) / ReceiveLog.m_dwTotalLen_16KB ;
		if( ReceiveLog.m_fProgress > 1 )
			ReceiveLog.m_fProgress = 1;
	}
	else
		ReceiveLog.m_fProgress = 0;

	if( bIncFileCount )
	{
		ReceiveLog.m_dwOKFileCount ++;
		if( ReceiveLog.m_dwOKFileCount > ReceiveLog.m_dwFileCount )
		{										//	��ֹ���
			ReceiveLog.m_dwOKFileCount = 0;
			ReceiveLog.m_dwOKLen_16KB = 0;
		}
	}

	if( m_bSendProgressEvent && fOldProgress != ReceiveLog.m_fProgress )
		Fire_OnProgressEvent( pDataPortItem, lpszFileName );

	return bIsChanged;
}

///-------------------------------------------------------
/// CYJ,2004-5-20
/// Function:
///		get one data port information
/// Input parameter:
///		hDataPort			IN		data port handle
///		dwBroLoopCount		out		output broadcast loop count
///		dwFileCount			OUT		total file count
///		dwTotalLen			OUT		total length, KB
///		dwByteReceived		OUT		byte received, KB
///		nCountReceived		OUT		file count received
/// Output parameter:
///		>=0					progress
///		<0					failed
float CDVBFileReceiver::GetProgressInfo( HDATAPORT hDataPort, DWORD & dwBroLoopCount, int & dwFileCount, DWORD & dwTotalLen, DWORD & dwByteReceived, int & nCountReceived )
{
	ASSERT( hDataPort );
	if( NULL == hDataPort )
		return -1;
	COneDataPortItem * pDataPortItem = (COneDataPortItem *) hDataPort;
	COneDataPortItem::FILERECEIVELOG & ReceiveLog = pDataPortItem->m_ReceiveLog;
	dwBroLoopCount = ReceiveLog.m_dwID;
	dwFileCount = ReceiveLog.m_dwFileCount;
	dwTotalLen = (ReceiveLog.m_dwTotalLen_16KB << 4);	// * 16K
	dwByteReceived = (ReceiveLog.m_dwOKLen_16KB << 4) + (ReceiveLog.m_dwOkLen_Below16KB>>10);
	nCountReceived = ReceiveLog.m_dwOKFileCount;

	return ReceiveLog.m_fProgress;
}

///-------------------------------------------------------
/// CYJ,2004-5-20
/// Function:
///		put Send progress event
/// Input parameter:
///		bNewValue			new value
/// Output parameter:
///		None
BOOL CDVBFileReceiver::PutSendProgressEvent( BOOL bNewValue )
{
	BOOL bRetVal = m_bSendProgressEvent;
	m_bSendProgressEvent = bNewValue;
	return bRetVal;
}

///-------------------------------------------------------
/// CYJ,2004-5-20
/// Function:
///		Get send progress event
/// Input parameter:
///		None
/// Output parameter:
///		None
BOOL CDVBFileReceiver::GetSendProgressEvent()
{
	return m_bSendProgressEvent;
}

///-------------------------------------------------------
/// CYJ,2004-5-20
/// Function:
///		call on progress event
/// Input parameter:
///		None
/// Output parameter:
///		None
void CDVBFileReceiver::Fire_OnProgressEvent( COneDataPortItem * pDataPortItem, LPCSTR lpszFileName )
{
	ASSERT( pDataPortItem );
	if( NULL == pDataPortItem || FALSE == m_bSendProgressEvent )
		return;
	if( NULL == m_pFileEventObject )
		return;

	COneDataPortItem::FILERECEIVELOG & ReceiveLog = pDataPortItem->m_ReceiveLog;
	DWORD dwBroLoopCount = ReceiveLog.m_dwID;
	DWORD dwFileCount = ReceiveLog.m_dwFileCount;
	DWORD dwTotalLen = (ReceiveLog.m_dwTotalLen_16KB << 4);	// * 16K
	DWORD dwByteReceived = (ReceiveLog.m_dwOKLen_16KB << 4) + (ReceiveLog.m_dwOkLen_Below16KB>>10);
	DWORD nCountReceived = ReceiveLog.m_dwOKFileCount;
	float fProgress = ReceiveLog.m_fProgress;

	m_pFileEventObject->OnProgress( (HDATAPORT)pDataPortItem, fProgress, dwBroLoopCount,\
		dwFileCount, dwTotalLen, dwByteReceived, nCountReceived, lpszFileName );
}

////-------------------------------------------------------
/// CYJ,2004-5-26
/// Function:
///		On huge file fully received
/// Input parameter:
///		pszFileName					huge file name, full path
///		pFileObject					reference file object
/// Output parameter:
///		None
void CDVBFileReceiver::OnHugeFileFullyReceived(const char * pszFileName, CFileObject *pFileObject)
{
//	TRACE("OnHugeFileFullyReceived is called,%s\n", pszFileName);

	CMyMapFile	mapfile;
	if( m_bMapFileOnFireFileOKEvent && \
		FALSE == mapfile.MapFileForReadOnly( pszFileName ) )
	{
		return;
	}

	pFileObject->AddRef();		//	���ܱ��ͷţ���ΪTmpFileObj�������еĲ���
	CFileObject TmpFileObj;

	TmpFileObj.m_strMC_DstIP = pFileObject->m_strMC_DstIP;					//	2002.11.14 �޸ģ���ಥ��صĲ���
	TmpFileObj.m_nMC_Port = pFileObject->m_nMC_Port;						//	�ಥ�˿�
	TmpFileObj.m_pDataPortItem = pFileObject->m_pDataPortItem;	//  2004-5-20 data port item

	TmpFileObj.m_pHugeFileParam = pFileObject->m_pHugeFileParam;	//	���ļ�����
	TmpFileObj.m_pExtData = pFileObject->m_pExtData;			//	���Ӳ���
	TmpFileObj.m_pAttributeData = pFileObject->m_pAttributeData;	//	���Բ���
	TmpFileObj.m_pFileHeader = pFileObject->m_pFileHeader;		//	TSDB �ļ�ͷ
	TmpFileObj.m_strFileName = pFileObject->m_strFileName;		//	�ļ���
	TmpFileObj.m_PacketTime = pFileObject->m_PacketTime ;		//	�ļ��������ʱ��
	TmpFileObj.m_FileHeadBuf.Copy( pFileObject->m_FileHeadBuf );		//	�ļ�ͷ������

	TmpFileObj.AddRef();			//	��ֹ���ͷ�
	DWORD dwFileLen = 0;
	if( m_bMapFileOnFireFileOKEvent )
	{
		dwFileLen = mapfile.GetFileLen();
		TmpFileObj.Attach( mapfile.GetBuffer(), dwFileLen );
		TmpFileObj.PutDataLen( dwFileLen );
	}

	Fire_OnFileOK( static_cast<IFileObject*>(&TmpFileObj), (HDATAPORT)pFileObject->m_pDataPortItem );

	if( m_bMapFileOnFireFileOKEvent )
		TmpFileObj.Detach( dwFileLen );

	pFileObject->Release();
}

///-------------------------------------------------------
/// CYJ,2004-5-26
/// Function:
///		Preset all vars
/// Input parameter:
///		None
/// Output parameter:
///		None
void CDVBFileReceiver::PresetVars()
{
	m_dwMaxPacketSize = 2048;		// 2K bytes
    m_bSendSubFileEvent = TRUE;
    m_bFileOKEventDone = TRUE;
    m_bIsOnLine = FALSE;
    m_bAutoSave = FALSE;			//	�Ƿ��Զ�����

    m_dwByteReceived = 0;						//	�Ѿ����յ����ļ��ֽ���
    m_dwLastTickCount = time(NULL);			//	�ϴ�ͳ�Ƶ�ʱ��, second
    m_dwLastFileBPS = 0;						//	�ϴ�ͳ�Ƶ�����
    m_nTimer_2_Second = 0;						//	ÿ 2 ����һ���¼�������
    m_pFileEventObject = NULL;
	m_bSendProgressEvent = FALSE;

	m_bMapFileOnFireFileOKEvent = TRUE;
}

///-------------------------------------------------------
/// CYJ,2004-7-31
/// Function:
///		Write thread ����ź��¼������浽�ļ���
/// Input parameter:
///		None
/// Output parameter:
///		None
void CDVBFileReceiver::WriteThread_DoCheckAndSaveFiles()
{
	CheckAndSendFileEvent();
	m_DelayEventDispatcher.FlushAddCatch();
}

///-------------------------------------------------------
/// CYJ,2004-7-6
/// Function:
///		Get not map file on file OK event
/// Input parameter:
///		None
/// Output parameter:
///		None
BOOL CDVBFileReceiver::GetDotMapFileOnFileOKEvent()
{
	return m_bMapFileOnFireFileOKEvent;
}

///-------------------------------------------------------
/// CYJ,2004-7-6
/// Function:
///		set not map file on file ok event
/// Input parameter:
///		None
/// Output parameter:
///		None
void CDVBFileReceiver::SetDoMapFileOnfileOKEvent( BOOL bNewValue )
{
	m_bMapFileOnFireFileOKEvent = bNewValue;
}
