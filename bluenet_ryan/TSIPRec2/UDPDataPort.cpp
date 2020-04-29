// UDPDataPort.cpp: implementation of the CUDPDataPort class.
//
//////////////////////////////////////////////////////////////////////
//  2003-11-21  �޸� Initialize������ʧ�ܣ����°�
//	2002.12.11	ԭ���첽��ȡΪ 256, �ָĳ� 128 ��
//	2002.6.17	�޸� Initialize��Windows 98 �£����ܽ� UDP ����������û��ṩ�Ļ�������

#include "stdafx.h"
#include "resource.h"
#include "UDPDataPort.h"
#include "MyThread.h"

#ifdef _WIN32
  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif
#endif //_WIN32  


/////////////////////////////////////////////////////////////////////
//	���� UDP ���ն˿�
//	���ز���
//		NULL			ʧ��
//		����			�ɹ�
extern "C" CSrcDataPort * WINAPI CreateUDPDataPort()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CUDPDataPort * pUDPPort = new CUDPDataPort;
	if( NULL == pUDPPort )
		return NULL;
	pUDPPort->AddRef();

	return static_cast<CSrcDataPort*>(pUDPPort);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUDPDataPort::CUDPDataPort()
{
	m_hSocket = INVALID_SOCKET;
	m_nItemCount = 0;					//	�첽�����ĸ���
	m_nCurReadItemCount = 0;			//	��ǰ��ȡ�ļ�¼��
	m_bNeedToCallCleanUp = FALSE;		//	������� CleanUp
	RtlZeroMemory( &m_mrMReq, sizeof(m_mrMReq) );
}

CUDPDataPort::~CUDPDataPort()
{
	Invalidate();
}

void CUDPDataPort::SafeDelete()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	delete this;
}

///////////////////////////////////////////
//	��ʼ��
//	��ڲ���
//		lpszIP			�����յ� IP ��ַ
//		nPort			���յĶ˿�
//		lpszLocalBind	���ذ󶨵�����
//		nCount			ͬʱ�첽��ȡ��˳��ȱʡ��1���Զ�ȷ��
//	���ز���
//		TRUE			�ɹ�
//		FALSE			ʧ��
//	����ֵ��:
//		
//	�޸ļ�¼��
//		2003-11-21 ����ʧ�ܣ������°�
BOOL	CUDPDataPort::Initialize( LPCSTR lpszIP, UINT nPort, LPCSTR lpszLocalBind, int nCount )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

#ifdef __USE_FOR_LINUX__			// compile for linux
	printf("CUDPDataPort::Initialize, IP=%s, PORT=%d, LOCALIP=%s\n", lpszIP, nPort, lpszLocalBind );
#endif //__USE_FOR_LINUX__

	ASSERT( lpszIP && *lpszIP && nPort >= 1024 );
	if( nCount < 0 )
		nCount = DEFAULT_ASYN_COUNT;		//	�Զ�ȷ��

	m_nItemCount = 0;
	m_nLastError = 0;

	WSADATA wsaData;
	m_nLastError = m_drv.WSAStartup(0x0202,&wsaData);
	if( m_nLastError )
	{
#ifdef __USE_FOR_LINUX__			// compile for linux
		printf("WSAStartup failed with error %d\n", m_nLastError );
#endif // __USE_FOR_LINUX__
		return FALSE;
	}
	m_bNeedToCallCleanUp = TRUE;

	m_hSocket = m_drv.WSASocket( AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
	if( m_hSocket == INVALID_SOCKET )
	{
		m_nLastError = m_drv.WSAGetLastError();
#ifdef __USE_FOR_LINUX__			// compile for linux
		printf("Failed to get a socket %d\n", m_nLastError );
#endif // __USE_FOR_LINUX__
		return FALSE;
	}

	BOOL bMultipleApps = TRUE;		/* allow reuse of local port if needed */
	m_drv.setsockopt( m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&bMultipleApps, sizeof(BOOL) );

	struct sockaddr_in  local;
	ZeroMemory( &local, sizeof(local) );
	local.sin_family      = AF_INET;
	local.sin_port        = m_drv.htons( nPort );

#ifdef __USE_FOR_LINUX__			// compile for linux
	local.sin_addr.s_addr = m_drv.inet_addr( lpszIP );	
#else
	if( lpszLocalBind && lpszLocalBind[0] )
		local.sin_addr.s_addr = m_drv.inet_addr( lpszLocalBind );
	else
		local.sin_addr.s_addr = m_drv.htonl( INADDR_ANY );
#endif // __USE_FOR_LINUX__	

	if( m_drv.bind( m_hSocket, (SOCKADDR *)&local, sizeof(local) ) != 0 )
	{													//  2003-11-21 add, rebind if bind failed
		local.sin_addr.s_addr = m_drv.htonl( INADDR_ANY );
		m_drv.bind( m_hSocket, (SOCKADDR *)&local, sizeof(local) );
#ifndef __USE_FOR_LINUX__			// compile for linux
		lpszLocalBind = NULL;
#endif // __USE_FOR_LINUX__
	}

	ZeroMemory( &m_mrMReq, sizeof(m_mrMReq) );
	m_mrMReq.imr_multiaddr.s_addr = m_drv.inet_addr(lpszIP);
	if( lpszLocalBind && lpszLocalBind[0] )
		m_mrMReq.imr_interface.s_addr = m_drv.inet_addr( lpszLocalBind );
	else
		m_mrMReq.imr_interface.s_addr = m_drv.htonl( INADDR_ANY );

	if(m_drv.setsockopt(m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char FAR *)&m_mrMReq, sizeof(m_mrMReq)) < 0)
	{
		m_nLastError = m_drv.WSAGetLastError();
#ifdef __PRINT_DEBUG_MSG_RELEASE__
		printf("Join Multicast failed. Error code=%d\n", m_nLastError );
#endif // __PRINT_DEBUG_MSG_RELEASE__
		return FALSE;
	}

	// Join the multicast group.  Note that sockM is not used 
    // to send or receive data. It is used when you want to 
    // leave the multicast group. You simply call closesocket() 
    // on it.

	strncpy( m_szIPAddress, lpszIP, sizeof(m_szIPAddress)-1 );
	m_wPort = nPort;

	struct sockaddr_in		remote;
	remote.sin_family      = AF_INET;
    remote.sin_port        = m_drv.htons( nPort );
    remote.sin_addr.s_addr = m_drv.inet_addr( lpszIP );

#ifndef __USE_FOR_LINUX__			// compile for linux
	if( CMyThread::IsWinNT() )			//	2002.6.17 �޸���ӣ�ֻ�� Windows NT ��ʹ���û��ṩ�Ļ�����
	{									
		int nRcvBuf = 0;				//	��ȫ�����ṩ�Ļ�����
		m_drv.setsockopt( m_hSocket, SOL_SOCKET , SO_RCVBUF, (char*)&nRcvBuf, sizeof(int) );
	}
	else
#endif // __USE_FOR_LINUX__
	{									//	2002.6.17��Windows 98 ����Ĳ�֧���û��ṩ�Ļ�������ֻ��ʹ��ϵͳ�ṩ�Ļ�������kao
		int nRcvBuf = 1536*96;			//	1.5K
		for(int i=0; i<10; i++)
		{
			m_drv.setsockopt( m_hSocket, SOL_SOCKET , SO_RCVBUF, (char*)&nRcvBuf, sizeof(int) );

			int nBufSize = sizeof(int);
			int nBufRet = 0;
			int nRetVal = m_drv.getsockopt( m_hSocket, SOL_SOCKET , SO_RCVBUF, (char*)&nBufRet, &nBufSize );
			if( 0 == nRetVal && nBufRet == nRcvBuf )
				break;
	#ifdef _DEBUG
			else
			{
				TRACE("getsockopt, last error = %d\n",m_drv.WSAGetLastError());
			}	
	#endif // _DEBUG
			nRcvBuf -= 1536*5;
		}
	}

	TRY
	{
		m_asynobjs.SetSize( nCount );
	}
	CATCH( CMemoryException, e )
	{
		m_asynobjs.RemoveAll();
		m_nLastError = ERROR_NOT_ENOUGH_MEMORY;
		return FALSE;
	}
	END_CATCH

	RtlZeroMemory( m_asynobjs.GetData(), sizeof(UDPOVERLAP)*nCount );

	for( int i=0; i<nCount; i++ )
	{
		UDPOVERLAP & obj = m_asynobjs[i];
		obj.m_hEvent = m_drv.WSACreateEvent();
		if( WSA_INVALID_EVENT == obj.m_hEvent )
		{
			m_nLastError = m_drv.WSAGetLastError();
			Invalidate();
			return FALSE;				//	ʧ��
		}
		obj.m_overlapped.hEvent = obj.m_hEvent;
	}
	m_nItemCount = nCount;

#ifdef __PRINT_DEBUG_MSG_RELEASE__
	printf("init UDP receiver socket succ, %s : %d\n", lpszIP, nPort );
#endif // __PRINT_DEBUG_MSG_RELEASE__

	return TRUE;
}

//////////////////////////////////////////////
///����:
///		��ȡ��ǰ������Ŀ��
///��ڲ���:
///		��
///���ز���:
///		��ǰʹ�õ׵�Ԫ��
int		CUDPDataPort::GetItemCount()
{
	return m_nItemCount;
}

//////////////////////////////////////////////
//����:
//		ʹ֮ʧЧ���ͷ�ȫ����Դ
//��ڲ���:
//		��
//���ز���:
//		��
void	CUDPDataPort::Invalidate()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if( FALSE == m_bNeedToCallCleanUp )
		return;
	m_bNeedToCallCleanUp = FALSE;

	m_nLastError = 0;	
	int nCount = m_asynobjs.GetSize();
	for(int i=0; i<nCount; i++)
	{
		UDPOVERLAP & obj = m_asynobjs[i];
		if( WSA_INVALID_EVENT == obj.m_hEvent )
			continue;
		m_drv.WSACloseEvent( obj.m_hEvent );
	}

	if(m_drv.setsockopt (m_hSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char FAR *)&m_mrMReq, sizeof(m_mrMReq)) < 0)
		return;

	if( INVALID_SOCKET != m_hSocket )
		m_drv.closesocket( m_hSocket );
	m_hSocket = INVALID_SOCKET;

	m_drv.WSACleanup();

	m_asynobjs.RemoveAll();
	m_nItemCount = 0;
}

//////////////////////////////////////////////
//����:
//		��ȡ��ǰ���¼�����������ȴ�
//��ڲ���:
//		hNo				������� ReadAsyn ����
//���ز���:
//		�¼����
//		INVALID_HANDLE_VALUE ʧ�ܣ�һ�㲻���֣����� Initialize ʧ�ܺ󻹵��øú���
HANDLE 	CUDPDataPort::GetEventHandle(SDP_HANDLE hNo)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	ASSERT( hNo > 0 && hNo <= m_nItemCount );
	if( hNo <= 0 || hNo > m_nItemCount )
		return INVALID_HANDLE_VALUE;

	ASSERT( m_asynobjs.GetSize() == m_nItemCount );
	UDPOVERLAP & obj = m_asynobjs[ hNo-1 ];
	return obj.m_hEvent;
}

//////////////////////////////////////////////
//����:
//		�첽��ȡ����
//��ڲ���:
//		pBuf			��������ַ
//		dwBufSize		��������С
//		pdwByteRead		ʵ�ʶ�ȡ���ֽ���,����Ϊ NULL
//���ز���:
//		0				�ɹ���pdwByteRead ���ʵ�ʵ��ֽ���
//		> 0				�ӳٲ�������Ҫ���� GetOverlappedResult
//		< 0				δ֪���󣬲��� m_nLastError
SDP_HANDLE	CUDPDataPort::ReadAsyn(	PBYTE pBuf, DWORD dwBufSize,PDWORD pdwByteRead )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ASSERT( m_nCurReadItemCount >= 0 && m_nCurReadItemCount < m_nItemCount );
	ASSERT( pBuf && dwBufSize && pdwByteRead );
	
	UDPOVERLAP & obj = m_asynobjs[ m_nCurReadItemCount ];
	if( obj.m_bIsPending )
	{
		m_nLastError = ERROR_NO_SPOOL_SPACE;
		return -1;			
	}

	obj.m_dwByteRead = 0;
	obj.m_dwFlags = 0;
	obj.m_wsaDataBuf.buf = (char*)pBuf;
	obj.m_wsaDataBuf.len = dwBufSize;
	obj.m_bIsPending = FALSE;
	m_drv.WSAResetEvent( obj.m_hEvent );

	m_nLastError = m_drv.WSARecv( m_hSocket, &obj.m_wsaDataBuf, 1, &obj.m_dwByteRead, &obj.m_dwFlags, &obj.m_overlapped, NULL );
	if( 0 == m_nLastError )
	{									//	�ɹ�
		if( pdwByteRead )
			*pdwByteRead = obj.m_dwByteRead;
#ifdef _DEBUG
		obj.m_wsaDataBuf.buf = NULL;
#endif // _DEBUG
		return 0;						//	�ɹ���û���ӳٲ���
	}
	
	m_nLastError = m_drv.WSAGetLastError();
	ASSERT( WSA_IO_PENDING == m_nLastError );
	if( WSA_IO_PENDING == m_nLastError )
	{									//	�ӳٲ���
		obj.m_bIsPending = TRUE;
		m_nCurReadItemCount ++;
		int nRetVal = m_nCurReadItemCount;
		if( m_nCurReadItemCount >= m_nItemCount )
			m_nCurReadItemCount = 0;
		return nRetVal;
	}
	else
		obj.m_dwByteRead = 0;			//	�������󣬷���

	return -1;
}

//////////////////////////////////////////////
///����:
///		ͬ����ȡ����
///��ڲ���:
//		pBuf			��������ַ
//		dwBufSize		��������С
//		pdwByteRead		ʵ�ʶ�ȡ���ֽ���,����Ϊ NULL
///���ز���:
///		TRUE			�ɹ�
///		FALSE			ʧ��
BOOL	CUDPDataPort::ReadSync( PBYTE pBuf, DWORD dwBufSize, PDWORD pdwByteRead )
{
	ASSERT( pBuf && dwBufSize );
	if( NULL == pBuf || 0 == dwBufSize )
	{
		m_nLastError = ERROR_NOT_ENOUGH_MEMORY;
		return FALSE;
	}
	WSABUF	wsBuf = { dwBufSize, (char*)pBuf };
	DWORD dwFlags = 0;
	DWORD dwByteRead = 0;
	if( NULL == pdwByteRead )
		pdwByteRead = &dwByteRead;

	m_nLastError = m_drv.WSARecv( m_hSocket, &wsBuf, 1, pdwByteRead, &dwFlags, NULL, NULL );
	if( 0 == m_nLastError )
		return TRUE;
	m_nLastError = m_drv.WSAGetLastError();
	return FALSE;
}

//////////////////////////////////////////////
//����:
//		��ȡ�ӳٲ�����״̬
//��ڲ���:
//		hReadNo			���������ͬ ReadAsyn ���ز���
//		pdwByteRead		���ʵ�ʵĶ�ȡ���ֽ���������Ϊ NULL
//		pWait			�Ƿ�ȴ���������
//���ز���:
//		TRUE			�ɹ�
//		FALSE			ʧ��,�����ǻ�û�����
BOOL	CUDPDataPort::GetOverlappedResult( SDP_HANDLE hReadNo, PDWORD pdwByteRead, BOOL bWait )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ASSERT( hReadNo > 0 && hReadNo <= m_nItemCount );
	if( hReadNo <= 0 || hReadNo > m_nItemCount )
		return FALSE;					//	ʧ��

	UDPOVERLAP & obj = m_asynobjs[ hReadNo-1 ];
	m_nLastError = 0;
	ASSERT( obj.m_bIsPending );

	if( m_drv.WSAGetOverlappedResult( m_hSocket, &obj.m_overlapped, &obj.m_dwByteRead, bWait, &obj.m_dwFlags ) )
	{
		if( pdwByteRead )
			*pdwByteRead = obj.m_dwByteRead;
		obj.m_bIsPending = FALSE;				//	��Ǹõ�Ԫ�Ѿ��ɹ���ȡ
		return TRUE;
	}

	m_nLastError = m_drv.WSAGetLastError();
	if( WSA_IO_INCOMPLETE == m_nLastError )
		return FALSE;							//	��û�����

	obj.m_bIsPending = FALSE;					//	ʧ�ܣ�����
	return FALSE;
}

//////////////////////////////////////////////
//����:
//		ȡ����ǰ���еĲ���
//��ڲ���:
//		hReadNo			������ȡ�ľ��
//���ز���:
//		��
//ע��
//		ȡ�����еĲ�����ζ�ţ������ͷ����е��ڴ���Դ
void	CUDPDataPort::CancelAsynRead(SDP_HANDLE hReadNo)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ASSERT( hReadNo > 0 && hReadNo <= m_nItemCount );
	if( hReadNo <= 0 || hReadNo > m_nItemCount )
		return;
	UDPOVERLAP & obj = m_asynobjs[ hReadNo-1 ];
	obj.m_bIsPending = FALSE;
}

//////////////////////////////////////////////
//����:
//		����ִ���첽������
//��ڲ���:
//		��
//���ز���:
//		TRUE			���пռ䣬����
//		FALSE			����
BOOL	CUDPDataPort::CanIDoReadAsync()
{
	UDPOVERLAP & obj = m_asynobjs[ m_nCurReadItemCount ];
	return FALSE == obj.m_bIsPending;
}