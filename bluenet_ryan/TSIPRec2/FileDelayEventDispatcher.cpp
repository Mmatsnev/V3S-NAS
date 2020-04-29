// FileDelayEventDispatcher.cpp: implementation of the CFileDelayEventDispatcher class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileDelayEventDispatcher.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CDelayEvnet_FileObject :: GetParamFromFileObj( CFileObject & FileObj )
{
	m_strMC_DstIP = FileObj.m_strMC_DstIP;					//	2002.11.14 �޸ģ���ಥ��صĲ���
	m_nMC_Port = FileObj.m_nMC_Port;						//	�ಥ�˿�
	m_pDataPortItem = NULL;									//  2004-5-20 data port item
	m_strFileName = FileObj.m_strFileName;	//	�ļ���
	m_PacketTime = FileObj.m_PacketTime;					//	�ļ��������ʱ��

	m_dwFileLen = FileObj.GetDataLen();

	if( m_pAttributeData )
		memcpy( &m_AttribDataBuf, m_pAttributeData, sizeof(m_AttribDataBuf) );
	else
		memset( &m_AttribDataBuf, 0, sizeof(m_AttribDataBuf) );

	m_pAttributeData = &m_AttribDataBuf;	//	���Բ���

	m_pFileHeader = NULL;					//	TSDB �ļ�ͷ
}

///-------------------------------------------------------
/// CYJ,2004-7-31
/// Function:
///		û�и�������
/// Input parameter:
///		None
/// Output parameter:
///		None
DWORD CDelayEvnet_FileObject :: GetBufSize()
{
	return 0;
}

DWORD CDelayEvnet_FileObject :: GetDataLen()
{
	return m_dwFileLen;
}

BOOL CDelayEvnet_FileObject :: SetBufSize( DWORD dwNewValue )
{
	return FALSE;
}

PBYTE CDelayEvnet_FileObject :: GetBuffer()
{
	return NULL;
}

void CDelayEvnet_FileObject :: SafeDelete()
{
	//	���ᱻɾ��
}

CDelayEventItem::CDelayEventItem()
{
	m_nRef = 0;
}

long CDelayEventItem::AddRef()
{
	return ::InterlockedIncrement( &m_nRef );
}

long CDelayEventItem::Release()
{
	::InterlockedDecrement( &m_nRef );
	if( m_nRef )
		return m_nRef;
	delete this;
	return 0;
}

void CDelayEventItem::Preset()
{
	m_fProgress = 0;
	m_dwBroLoopCount = 0;
	m_dwFileCount = 0;
	m_dwTotalLen = 0;
	m_dwByteReceived = 0;
	m_nCountReceived = 0;

	m_nEventType = 0;
	m_hDataPort = NULL;
}

///////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////

CFileDelayEventDispatcher::CFileDelayEventDispatcher()
: m_DelayEventQueue( 1024, 10 )
{
	m_pRealHandler = NULL;
}

CFileDelayEventDispatcher::~CFileDelayEventDispatcher()
{
	if( m_pRealHandler )
		m_pRealHandler->Release();
}

void CFileDelayEventDispatcher::OnFileOK( IFileObject * pObj, HDATAPORT hDataPort )
{
	if( NULL == pObj || NULL == m_pRealHandler )
		return;			//	û��ԭ���ģ�����������

	CDelayEventItem * pEvnetObj = m_DelayEventQueue.AllocatePacket();
	if( NULL == pEvnetObj )
		return;

	pEvnetObj->m_nEventType = CDelayEventItem::EVENT_TYPE_FILE_OK;
	pEvnetObj->m_hDataPort = hDataPort;
	CFileObject * pFileObj = static_cast<CFileObject*>(pObj);
	pEvnetObj->m_FileObject.GetParamFromFileObj( *pFileObj );

	m_DelayEventQueue.AddPacket( pEvnetObj );
	pEvnetObj->Release();
}

void CFileDelayEventDispatcher::OnSubFileOK( IFileObject * pObj, HDATAPORT hDataPort )
{
	//	���ᴫ�� OnSubFileOK �¼�
}

void CFileDelayEventDispatcher::OnProgress( HDATAPORT hDataPort, float fProgress, DWORD dwBroLoopCount, int dwFileCount, DWORD dwTotalLen, DWORD dwByteReceived, int nCountReceived, LPCSTR lpszFileName )
{
	if( NULL == m_pRealHandler )
		return;				//	û��ԭ���ģ�����������

	CDelayEventItem * pEvnetObj = m_DelayEventQueue.AllocatePacket();
	if( NULL == pEvnetObj )
		return;

	pEvnetObj->m_nEventType = CDelayEventItem::EVENT_TYPE_PROGRESS;
	pEvnetObj->m_hDataPort = hDataPort;
	pEvnetObj->m_fProgress = fProgress;
	pEvnetObj->m_dwBroLoopCount = dwBroLoopCount;
	pEvnetObj->m_dwFileCount = dwFileCount;
	pEvnetObj->m_dwTotalLen = dwTotalLen;
	pEvnetObj->m_dwByteReceived = dwByteReceived;
	pEvnetObj->m_nCountReceived = nCountReceived;
	pEvnetObj->m_FileObject.m_strFileName = lpszFileName;	//	����

	m_DelayEventQueue.AddPacket( pEvnetObj );
	pEvnetObj->Release();
}

///-------------------------------------------------------
/// CYJ,2004-7-31
/// Function:
///		set handler
/// Input parameter:
///		None
/// Output parameter:
///		None
void CFileDelayEventDispatcher::SetHandler( IDVBReceiverEvent * pHandler )
{
	if( m_pRealHandler )
		m_pRealHandler->Release();

	m_pRealHandler = pHandler;

	if( m_pRealHandler )
		m_pRealHandler->AddRef();
}

///-------------------------------------------------------
/// CYJ,2004-7-31
/// Function:
///		ת����Ϣ
/// Input parameter:
///		None
/// Output parameter:
///		None
void CFileDelayEventDispatcher::DispatchEvents()
{
	if( NULL == m_pRealHandler )
		return;

	CDelayEventItem * pEventObj = NULL;
	while( pEventObj = m_DelayEventQueue.PeekData(0) )
	{
		m_pRealHandler->AddRef();

		if( CDelayEventItem::EVENT_TYPE_FILE_OK == pEventObj->m_nEventType )
			m_pRealHandler->OnFileOK( static_cast<IFileObject*>(&pEventObj->m_FileObject), pEventObj->m_hDataPort );
		else if( CDelayEventItem::EVENT_TYPE_PROGRESS == pEventObj->m_nEventType )
		{
			m_pRealHandler->OnProgress( pEventObj->m_hDataPort, pEventObj->m_fProgress,\
				pEventObj->m_dwBroLoopCount, pEventObj->m_dwFileCount, \
				pEventObj->m_dwTotalLen, pEventObj->m_dwByteReceived, pEventObj->m_nCountReceived,\
				pEventObj->m_FileObject.m_strFileName );
		}
		m_pRealHandler->Release();

		m_DelayEventQueue.DeAllocate( pEventObj );
		pEventObj->Release();
	}
}


void CFileDelayEventDispatcher::SafeDelete()
{
	// ����ɾ��
}


void CFileDelayEventDispatcher::FlushAddCatch()
{
	m_DelayEventQueue.FlushAddCache();
}

