// GetBindIP.cpp : implementation file
//
////////////////////////////////////////////////////
//  2003-9-3	�޸� OnInitDialog��ֻ��һ������Ҳִ�а󶨹���
//	2002.6.29	��Ӻ��� OnDblclkListAdapter��˫����ʾѡ�иÿ�


#include "stdafx.h"
#include "GetBindIP.h"
#include "IPHlpAPI.H"
#include "Winsock2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGetBindIP dialog

//	ִ�а� IP �ĶԻ���
//	��ڲ���
//		lpszBuf				��������ַ
//		nBufLen				����������
//		pnIPAddressCount	��������������
//		pszAdapterName		������������ƣ�����Ϊ0����ʾ���������������
//		nAdapterBufSize		��������������С
//	ע��
//		������Ϊ ERROR_CANCELLED ʱ��pszAdapterName ���������������
extern "C" HRESULT WINAPI DoBindIPDlgExA( LPSTR lpszBuf, int nBufLen, int * pnIPAddressCount, char * pszAdapterName, int nAdapterBufSize )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	ASSERT( lpszBuf && nBufLen );
	if( NULL == lpszBuf || 0 == nBufLen )
		return ERROR_INVALID_DATA;

	if( pnIPAddressCount )
		*pnIPAddressCount = 0;
	if( pszAdapterName )
		*pszAdapterName = 0;

	CGetBindIP dlg;
	dlg.m_strIP = lpszBuf;
	if( IDOK != dlg.DoModal() )
		return ERROR_CANCELLED;			//	cancel

	if( nBufLen < dlg.m_strIP.GetLength() )
		return ERROR_BUFFER_OVERFLOW;

	strcpy( lpszBuf, dlg.m_strIP );
	if( pnIPAddressCount )
		*pnIPAddressCount = dlg.m_nIPAddressCount;

	if( pszAdapterName )
	{
		if( dlg.m_strAdapterName.GetLength() > nAdapterBufSize )
			return ERROR_BUFFER_OVERFLOW;
		strcpy( pszAdapterName, dlg.m_strAdapterName );
	}

	return ERROR_SUCCESS;
}

//	ִ�а� IP �ĶԻ���
//	��ڲ���
//		lpszBuf				��������ַ
//		nBufLen				����������
//		pnIPAddressCount	��������������
extern "C" HRESULT WINAPI DoBindIPDlgA( LPSTR lpszBuf, int nBufLen, int * pnIPAddressCount )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return DoBindIPDlgExA( lpszBuf, nBufLen, pnIPAddressCount, NULL, 0 );
}

///-------------------------------------------------------
/// 2003-1-15
/// ���ܣ�
///		��ȡ��һ��������ַ
/// ��ڲ�����
///		pszIPBuffer			���IP��ַ
///		nBufLen				����������
/// ���ز�����
///		ERROR_SUCCESS			�ɹ�
///		ERROR_DEV_NOT_EXIST		û������
///		ERROR_BUFFER_OVERFLOW	���������
extern "C" HRESULT WINAPI GetFirstIPAddressA( char * pszIPBuffer, int nBufLen )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	ASSERT( pszIPBuffer && nBufLen );
	if(NULL == pszIPBuffer || 0 == nBufLen )
		return ERROR_INVALID_DATA;

	CGetBindIP helper;
	DWORD adwIPAddressList[20];
	int nCount = helper.GetIPAddressList( adwIPAddressList, sizeof(adwIPAddressList)/sizeof(DWORD) );
	if( 0 == nCount )
		return ERROR_DEV_NOT_EXIST;		//	������

	CString strTmp;
	in_addr address;
	address.S_un.S_addr = adwIPAddressList[0];
	strTmp.Format("%d.%d.%d.%d",address.S_un.S_un_b.s_b1,address.S_un.S_un_b.s_b2,\
		address.S_un.S_un_b.s_b3,address.S_un.S_un_b.s_b4);

	if( nBufLen < strTmp.GetLength()+1 )
		return ERROR_BUFFER_OVERFLOW;

	strcpy( pszIPBuffer, strTmp );
	
	return ERROR_SUCCESS;
}

//////////////////////////////////////////////
//	2002.5.23	���
//	����:
//		��ȡ���е����������Ƽ��� IP ��ַ
//	��ڲ���:
//		pBuf				���룬�����ַ
//		nBufSize			���룬�����С
//	���ز���:
//		NULL				ʧ�ܣ��ɸ��� GetLastError ʧ�ܴ���ԭ��
//		����				�ɹ�
//	ע��
//		������������ hostent.h_name ��ӳ������Ϊ NULL ʱ����
//		ֻ�� hostent.h_name, .h_addr_list ��Ч
extern "C" struct hostent * WINAPI GetAllAdaptersAndIPsA( PBYTE pBuf, int nBufSize )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	ASSERT( pBuf && nBufSize );
	if( NULL == pBuf || 0 == nBufSize )
	{
		::SetLastError( ERROR_INVALID_PARAMETER);		//	������̫С
		return NULL;
	}

	ZeroMemory( pBuf, nBufSize );

	HMODULE hDll = ::LoadLibrary( "IPHLPAPI.DLL" );
	if( NULL == hDll )
	{
		::SetLastError(ERROR_FILE_NOT_FOUND);			//	����û���ҵ�
		return NULL;
	}
	
	BYTE abyBuf[20*1024];								//	20K
	DWORD dwBufSize = sizeof(abyBuf);
	ZeroMemory( abyBuf, dwBufSize );

	DWORD (WINAPI*pfnGetAdaptersInfo)( PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen );
	pfnGetAdaptersInfo = \
		(DWORD(WINAPI*)(PIP_ADAPTER_INFO,PULONG))GetProcAddress( hDll, "GetAdaptersInfo" );
	if( NULL == pfnGetAdaptersInfo )
	{
		::SetLastError(ERROR_FILE_NOT_FOUND);			//	����û���ҵ�
		FreeLibrary( hDll );
		return NULL;					//	û�иú���
	}
		
	PIP_ADAPTER_INFO pInfo = (PIP_ADAPTER_INFO)abyBuf;
	DWORD dwRetVal = pfnGetAdaptersInfo( pInfo, &dwBufSize );
	if( ERROR_SUCCESS != dwRetVal )
	{
		::SetLastError(ERROR_DEV_NOT_EXIST);			//	û���ҵ�������
		FreeLibrary( hDll );
		return NULL;
	}

	BOOL bError = FALSE;								//	��ʾ��������	
	DWORD dwByteLeft = nBufSize;
	struct hostent * pRetVal = (struct hostent * )( pBuf + 1 );
	char * pszEmptyString = (char*)pBuf;
	*pszEmptyString = 0;								//	���ַ�
	pBuf ++;
	dwByteLeft --;
	pRetVal[0].h_name = NULL;							//	�ȱ�ʾ�������Ժ����޸�	

	PIP_ADAPTER_INFO pTmpInfo = pInfo;
	int nCount = 0;
	while( pTmpInfo )
	{
		pTmpInfo = pTmpInfo->Next;
		nCount ++;
	}
	nCount ++;											//	һ������ռ�
	if( dwByteLeft < nCount*sizeof(struct hostent) )
	{
		::SetLastError(ERROR_INSUFFICIENT_BUFFER);		//	����������
		FreeLibrary( hDll );
		return NULL;
	}
	pBuf += nCount*sizeof(struct hostent);
	dwByteLeft -= nCount*sizeof(struct hostent);

	nCount = 0;
	while( pInfo )
	{			
		PIP_ADDR_STRING pIPList = &pInfo->IpAddressList;
		if( NULL == pIPList )
		{
			pInfo = pInfo->Next;
			continue;
		}
		pRetVal[nCount].h_name = NULL;		//	�ȱ�ʾ�������Ժ����޸�
		pRetVal[nCount].h_addrtype = 0;
		pRetVal[nCount].h_aliases = 0;
		pRetVal[nCount].h_length = 0;
		pRetVal[nCount].h_addr_list = NULL;
		LPCSTR pszAdapters = pInfo->Description;
		int nAdapterLen = strlen(pszAdapters);
		if( 0 == nAdapterLen )
		{
			pszAdapters = pInfo->AdapterName;			//	û�����ƣ��� UUID ����
			nAdapterLen = strlen( pszAdapters );
		}
		nAdapterLen ++;									//	������β�ַ�
		if( dwByteLeft < (DWORD)nAdapterLen )
		{
			bError = TRUE;
			break;
		}
		strcpy( (char*)pBuf, pszAdapters );
		pRetVal[nCount].h_name = (char*)pBuf;
		pBuf += nAdapterLen;
		dwByteLeft -= nAdapterLen;		

		int nIPCount = 0;								//	���� IP �ĸ�������ȷ�� pszIP �������
		int nTotalIPStringLen = 0;
		while( pIPList )
		{			
			nIPCount ++;			
			nTotalIPStringLen += strlen(pIPList->IpAddress.String) + 1;
			pIPList = pIPList->Next;
		}
		if( dwByteLeft < nTotalIPStringLen + sizeof(char*)*nIPCount )
		{
			bError = TRUE;							//	��������
			break;
		}
		pRetVal[nCount].h_addr_list = (char* *)pBuf;
		int nPtrBuf = sizeof(char*) * ( nIPCount + 1 );
		pBuf += nPtrBuf;
		dwByteLeft -= nPtrBuf;
	
		pIPList = &pInfo->IpAddressList;
		nIPCount = 0;
		while( pIPList )
		{
			pRetVal[nCount].h_addr_list[nIPCount] = (char*)pBuf;
			strcpy( (char*)pBuf, pIPList->IpAddress.String );
			int nLen = strlen( pIPList->IpAddress.String ) + 1;
			pBuf += nLen;
			dwByteLeft -= nLen;
			nIPCount ++;
			pIPList = pIPList->Next;
		}

		pInfo = pInfo->Next;
		nCount ++;
	}

	FreeLibrary( hDll );
	if( bError )
	{
		::SetLastError( ERROR_INSUFFICIENT_BUFFER );
			return NULL;
	}
	return pRetVal;
}


CGetBindIP::CGetBindIP(CWnd* pParent /*=NULL*/)
	: CDialog(CGetBindIP::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGetBindIP)
	m_strIP = _T("");
	//}}AFX_DATA_INIT
	m_nIPAddressCount = 0;
	WSADATA wsaData;
	m_bIsWSAStartupSucc = ( 0 == m_drv.WSAStartup(0x0202,&wsaData) );	
}

CGetBindIP::~CGetBindIP()
{
	if( m_bIsWSAStartupSucc )
		m_drv.WSACleanup();
}

void CGetBindIP::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGetBindIP)
	DDX_Control(pDX, IDC_LIST_ADAPTER, m_list_Adapter);
	DDX_Control(pDX, IDC_CB_IP, m_cb_IP);
	DDX_CBString(pDX, IDC_CB_IP, m_strIP);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGetBindIP, CDialog)
	//{{AFX_MSG_MAP(CGetBindIP)
	ON_LBN_SELCHANGE(IDC_LIST_ADAPTER, OnSelchangeListAdapter)
	ON_CBN_SELCHANGE(IDC_CB_IP, OnSelchangeCbIp)
	ON_LBN_DBLCLK(IDC_LIST_ADAPTER, OnDblclkListAdapter)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGetBindIP message handlers

void CGetBindIP::OnOK() 
{
	UpdateData();

	int nCurSel = m_cb_IP.GetCurSel();
	if( CB_ERR == nCurSel )
		return;
	DWORD dwIP = m_cb_IP.GetItemData( nCurSel );
	int nCount = m_list_Adapter.GetCount();

	m_strAdapterName.Empty();
	for(int i=0; i<nCount; i++)
	{
		if( m_list_Adapter.GetItemData( i ) == dwIP )
		{
			m_list_Adapter.GetText( i, m_strAdapterName  );
			break;
		}
	}

	CDialog::OnOK();
}

//---------------------------------------------------------
//	�޸ļ�¼��
//  2003-9-3	ֻ��һ������Ҳִ�а󶨹���
BOOL CGetBindIP::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if( m_strIP.Find( "default" ) >= 0 )		//  2003-9-3 �޸ģ�ֻ��һ��������Ҳִ����ʾ��
		m_strIP.Empty();

	if( m_bIsWSAStartupSucc )
	{
		EnumIPAndFillIPCB();
		UpdateData( FALSE );		
	}	

	LoadAndFillAdapter();
	UpdateData( FALSE );

	if( m_strIP.IsEmpty() )						//  2003-9-3 ȱʡΪ��һ������
		m_cb_IP.SetCurSel( 0 );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//	��ȡ����������Ϣ
//	���ز���
//		TRUE			�ɹ������ж�� IP(����)
//		FALSE			ֻ��һ������������û�� IPHlpAPI.DLL
BOOL CGetBindIP::LoadAndFillAdapter()
{
	HMODULE hDll = ::LoadLibrary( "IPHLPAPI.DLL" );
	if( NULL == hDll )
		return FALSE;
	
	BYTE abyBuf[20*1024];
	DWORD dwBufSize = sizeof(abyBuf);
	ZeroMemory( abyBuf, dwBufSize );

	DWORD (WINAPI*pfnGetAdaptersInfo)( PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen );
	pfnGetAdaptersInfo = \
		(DWORD(WINAPI*)(PIP_ADAPTER_INFO,PULONG))GetProcAddress( hDll, "GetAdaptersInfo" );
	if( NULL == pfnGetAdaptersInfo )
	{
		FreeLibrary( hDll );
		return FALSE;					//	û�иú���
	}
	
	PIP_ADAPTER_INFO pInfo = (PIP_ADAPTER_INFO)abyBuf;
	DWORD dwRetVal = pfnGetAdaptersInfo( pInfo, &dwBufSize );
	if( ERROR_SUCCESS == dwRetVal )
	{
		while( pInfo )
		{			
			LPCSTR pszAdapters = pInfo->Description;
			if( 0 == strlen(pszAdapters) )
				pszAdapters = pInfo->AdapterName;			//	û�����ƣ��� UUID ����
			int nNo = m_list_Adapter.AddString( pszAdapters );
			PIP_ADDR_STRING pIPList = &pInfo->IpAddressList;

			BOOL bFound = FALSE;
			while( pIPList )
			{				
				for(int i=0; i<m_nIPAddressCount; i++)
				{
					DWORD dwIP = m_cb_IP.GetItemData( i );
					if( m_drv.inet_addr(pIPList->IpAddress.String) == dwIP )
					{
						m_list_Adapter.SetItemData( nNo, dwIP );
						bFound = TRUE;
						break;
					}
				}		
				if( bFound )				//	ƥ�䵽 IP ��ַ��������һ�� IP
					break;

				pIPList = pIPList->Next;
			}

			if( FALSE == bFound )
			{								//	û��ƥ�䵽��Ӧ����һ���µ� IP�����
				pIPList = &pInfo->IpAddressList;
				DWORD dwIP = m_drv.inet_addr( pIPList->IpAddress.String );
				int nIP = m_cb_IP.AddString( pIPList->IpAddress.String );
				m_cb_IP.SetItemData( nIP, dwIP );		// ����µ�IP
				m_list_Adapter.SetItemData( nNo, dwIP);	
			}

			pInfo = pInfo->Next;
		}
	}

	FreeLibrary( hDll );
	return m_nIPAddressCount > 1 ;
}

void CGetBindIP::OnSelchangeListAdapter() 
{
	int nCurSel = m_list_Adapter.GetCurSel();
	if( LB_ERR == nCurSel )
		return;
	DWORD dwIP = m_list_Adapter.GetItemData( nCurSel );
	int nCount = m_cb_IP.GetCount();
	for(int i=0; i<nCount; i++)
	{
		if( m_cb_IP.GetItemData(i) == dwIP )
		{
			m_cb_IP.SetCurSel( i );
			break;
		}
	}	
}

void CGetBindIP::OnSelchangeCbIp() 
{
	int nCurSel = m_cb_IP.GetCurSel();
	if( CB_ERR == nCurSel )
		return;
	DWORD dwIP = m_cb_IP.GetItemData( nCurSel );
	int nCount = m_list_Adapter.GetCount();
	for(int i=0; i<nCount; i++)
	{
		if( m_list_Adapter.GetItemData( i ) == dwIP )
		{
			m_list_Adapter.SetCurSel( i );
			break;
		}
	}
}

//	÷�ٱ���IP��ַ�������IP CB
//	���ز���
//		IP ��ַ����
int CGetBindIP::EnumIPAndFillIPCB()
{
	DWORD adwIPAddressList[20];
	m_nIPAddressCount = GetIPAddressList(adwIPAddressList,sizeof(adwIPAddressList)/sizeof(DWORD) );

	CString strTmp;
	for(int i=0; i<m_nIPAddressCount; i++)
	{
		in_addr address;
		address.S_un.S_addr = adwIPAddressList[i];
		strTmp.Format("%d.%d.%d.%d",address.S_un.S_un_b.s_b1,address.S_un.S_un_b.s_b2,\
			address.S_un.S_un_b.s_b3,address.S_un.S_un_b.s_b4);
		int nNo = m_cb_IP.AddString( strTmp );
		m_cb_IP.SetItemData( nNo, address.S_un.S_addr );
	}
	return m_nIPAddressCount;
}

//	2002.6.29 ��ӣ�˫����ʾѡ��
void CGetBindIP::OnDblclkListAdapter() 
{
	OnSelchangeListAdapter();
	OnOK();		
}

///-------------------------------------------------------
/// 2003-1-15
/// ���ܣ�
///		��ȡ����IP��ַ�б�
/// ��ڲ�����
///		pdwIPList		IP ��ַ����
///		nBufCount		�������
/// ���ز�����
///		��
int CGetBindIP::GetIPAddressList(PDWORD pdwIPList, int nBufCount)
{
	if( FALSE == m_bIsWSAStartupSucc )
		return 0;				//	ʧ��

	char szHostname[256];
	if( m_drv.gethostname(szHostname, sizeof(szHostname) ) )
	{
		  TRACE(_T("Failed in call to gethostname, WSAGetLastError returns %d\n"), m_drv.WSAGetLastError());
		  return 0;
	}

	//get host information from the host name
	HOSTENT* pHostEnt = m_drv.gethostbyname(szHostname);
	if (pHostEnt == NULL)
	{
		TRACE(_T("Failed in call to gethostbyname, WSAGetLastError returns %d\n"), m_drv.WSAGetLastError());
		return 0;
	}

	//check the length of the IP adress
	if (pHostEnt->h_length != 4)
	{
		TRACE(_T("IP address returned is not 32 bits !!\n"));
		return 0;
	}

	//call the virtual callback function in a loop
	int nAdapter = 0;
	CString strTmp;
	while( pHostEnt->h_addr_list[nAdapter] && nBufCount )
	{
		in_addr address;
		ASSERT( pHostEnt->h_length <= sizeof(in_addr) );
		CopyMemory( &address.S_un.S_addr, pHostEnt->h_addr_list[nAdapter], pHostEnt->h_length );

		*pdwIPList = address.S_un.S_addr;
		if( *pdwIPList )
		{						//	��ȥ��ַΪ 0 ��IP
			pdwIPList ++;
			nBufCount --;
		}
	
		nAdapter ++;
	}
	return nAdapter;
}
