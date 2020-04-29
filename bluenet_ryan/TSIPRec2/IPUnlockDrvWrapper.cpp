///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2002-11-28
///
///=======================================================

// IPUnlockDrvWrapper.cpp: implementation of the CIPUnlockDrvWrapper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "iprecsvr.h"
#include "IPUnlockDrvWrapper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIPUnlockDrvWrapper::CIPUnlockDrvWrapper()
{
	Preset();
}

CIPUnlockDrvWrapper::~CIPUnlockDrvWrapper()
{
	if( m_hDll )
		::FreeLibrary( m_hDll );
}

///-------------------------------------------------------
/// 2002-11-28
/// ���ܣ�
///		��ȡ������ SN
/// ��ڲ�����
///		��
/// ���ز�����
///		������SN
DWORD	CIPUnlockDrvWrapper::GetDrvSN()
{
	ASSERT( m_pfnGetDrvSN );
	if( NULL == m_pfnGetDrvSN )
		return 0;
	return m_pfnGetDrvSN();
}

///-------------------------------------------------------
/// 2002-11-28
/// ���ܣ�
///		��ʼ������
/// ��ڲ�����
///		pKeyMgr			�������
/// ���ز�����
///		TRUE			�ɹ�
///		FALSE			ʧ��
BOOL	CIPUnlockDrvWrapper::InitDrv( CIPEncryptKeyMgr * pKeyMgr )
{
	ASSERT( pKeyMgr && m_pfnInitDrv );
	if( NULL == pKeyMgr || NULL == m_pfnInitDrv )
		return FALSE;
	return m_pfnInitDrv( pKeyMgr );
}

///-------------------------------------------------------
/// 2002-11-28
/// ���ܣ�
///		��������
/// ��ڲ�����
///		pBuf					������
///		dwBufLen				��������С
///		dwSysCodeIndex			ϵͳ��������
///		dwOfsInFile				�������ļ��е�ƫ�ƣ�ȱʡΪ 0
/// ���ز�����
///		>0						�ɹ�ִ�н��ܣ������ݲ�һ����ȷ
///		=0						ʧ�ܣ�û�н�������
///		<0						����δ֪����
int		CIPUnlockDrvWrapper::UnlockData( PBYTE pBuf, DWORD dwBufLen, DWORD dwSysCodeIndex, DWORD dwOfsInFile )
{
	ASSERT( m_pfnUnlockData && pBuf && dwBufLen && dwSysCodeIndex );
	if( NULL == m_pfnUnlockData || NULL == pBuf || 0 == dwBufLen || 0 == dwSysCodeIndex )
		return -1;
	return m_pfnUnlockData( pBuf, dwBufLen, dwSysCodeIndex, dwOfsInFile );
}

///-------------------------------------------------------
/// 2002-11-28
/// ���ܣ�
///		��������
/// ��ڲ�����
///		nDrvSN					������ţ������ļ���Ϊ<WinSysDir>\CODUnlockDrvXXXXXX.DLL
/// ���ز�����
///		TRUE					�ɹ�
///		FALSE					ʧ��
BOOL CIPUnlockDrvWrapper::LoadDrv(int nDrvSN)
{
	CString strFileName;
	char szSysDir[_MAX_PATH];
	::GetSystemDirectory( szSysDir, sizeof(szSysDir) );
	strFileName.Format("%s\\CODUnlockDrv%06X.DLL", szSysDir, nDrvSN );
	return LoadDrv( strFileName );
}

///-------------------------------------------------------
/// 2002-11-28
/// ���ܣ�
///		��������
/// ��ڲ�����
///		lpszDrvFileName			�ļ���
/// ���ز�����
///		TRUE					�ɹ�
///		FALSE					ʧ��
BOOL CIPUnlockDrvWrapper::LoadDrv(LPCSTR lpszDrvFileName)
{
	ASSERT( lpszDrvFileName );

	if( m_hDll )
		FreeLibrary( m_hDll );
	Preset();

	m_hDll = ::LoadLibrary( lpszDrvFileName );
	if( NULL == m_hDll )
		return FALSE;

	m_pfnGetDrvSN = (DWORD(WINAPI*)())::GetProcAddress( m_hDll, "GetDrvSN" );
	m_pfnInitDrv = (BOOL(WINAPI*)(CIPEncryptKeyMgr*))::GetProcAddress( m_hDll, "InitDrv" );
	m_pfnUnlockData = (int(WINAPI*)(PBYTE,DWORD,DWORD,DWORD))::GetProcAddress( m_hDll, "UnlockData" );
	return (m_pfnGetDrvSN && m_pfnInitDrv && m_pfnUnlockData);
}

///-------------------------------------------------------
/// 2002-11-28
/// ���ܣ�
///		Ԥ�ò���
/// ��ڲ�����
///		��
/// ���ز�����
///		��
void CIPUnlockDrvWrapper::Preset()
{
	m_hDll = NULL;
	m_pfnGetDrvSN = NULL;
	m_pfnInitDrv = NULL;
	m_pfnUnlockData = NULL;
}
