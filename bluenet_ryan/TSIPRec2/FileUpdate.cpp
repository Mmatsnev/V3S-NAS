// FileUpdate.cpp: implementation of the CFileUpdate class.
//
//				�����ļ�
//	����:	ͨ�� �޸� WinInit.Ini ������, ������ Windows ʱ�����ļ�
//			������¼�� <TS>\Receive\Ŀ¼�µ� Update.Log �ļ���
//	
//////////////////////////////////////////////////////////////////////
//	2001.6.14	�޸� SetupInstall �е� FORCE_IN_USE ��־
//	2001.4.6	�޸� SetInstall �����ļ�����ɾ��Դ�ļ�
//	2000.11.9	�޸� Update �������޸� SetupInstallFile ��ڲ���

#include "stdafx.h"
#include "resource.h"
#include "FileUpdate.h"
#include "MyRegKey.h"
#include "DirectroyHelp.h"
#include "io.h"
#include "setupapi.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileUpdate::CFileUpdate()
{

}

CFileUpdate::~CFileUpdate()
{

}

//	�����ļ�
//	��ڲ���
//		dwFileLen					�ļ�����
//		pBuf						���ݻ�����
//		pExtDataBuf					�������ݻ�������ַ
//		dwExtLen					�������ݳ���
void CFileUpdate::Update( DWORD dwFileLen, PBYTE pBuf, void *pExtDataBuf, DWORD dwExtLen )
{
CString strSrcFileName;
	ASSERT( dwFileLen && pBuf && pExtDataBuf && dwExtLen && dwExtLen == sizeof(SOFTUPDATE) );
	PSOFTUPDATE pItem = (PSOFTUPDATE) pExtDataBuf;
	CString strDstFile = GetRealPath( pItem->szPath ) + pItem->szFileName;

	strSrcFileName = strDstFile + ".1";
	CDirectroyHelp::Mkdir( strDstFile );			//	������ʱĿ¼

	if( _access( strSrcFileName,0 ) == 0 )
		return;										//	�Ѿ�����
	else
	{
		CFile f;
		try
		{
			if( f.Open( strSrcFileName, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary ) == FALSE )
				return;
			f.Write( pBuf, dwFileLen );
			f.Close();
		}
		catch(CFileException * e)
		{
			TRACE("MSG CFile::Update, file IO error, filename=%s\n",strSrcFileName);
			e->Delete();
			return;								
		}
	}
	BOOL bFileInUse = IsUsing( strDstFile );
	if( bFileInUse )
		AddToUpdateTaskList( strSrcFileName, strDstFile );		//	����ʹ����,�ҳ��ļ���
	else
	{
		if( SetupInstall( strSrcFileName, strDstFile ) )		//	����ʹ����, ��������
			AddToDelList( strSrcFileName );						//	��ӵ�ɾ�����ļ��б���
		else
			AddToUpdateTaskList( strSrcFileName, strDstFile );		//	����ʹ����,�ҳ��ļ���
	}

	strSrcFileName = strDstFile;
	strSrcFileName += "  ---------  ";
	strSrcFileName += pItem->szHelp;
	CTime now = CTime::GetCurrentTime();
	strSrcFileName.Insert(0, now.Format("%Y.%m.%d %H:%M:%S ���յ������ļ� ==> ") );
	AddToUpdateLog( strSrcFileName );
}

//	���ܳ��������
//	��ڲ���
//		dwFileLen						�ļ�����
//		pBuf							������
//		pExtDataBuf						��չ��������ַ
//		dwExtLen						��չ��������С
//	���ز���
//		���� = 0						ʧ��
//		��ʱ�ļ����ļ���
CString CFileUpdate::UpdateUnlockProc(DWORD dwFileLen, PBYTE pBuf, void *pExtDataBuf, DWORD dwExtLen)
{
CString strSrcFileName;
CString strRetVal;
	ASSERT( dwFileLen && pBuf && pExtDataBuf && dwExtLen == sizeof(SOFTUPDATE) );
	PSOFTUPDATE pItem = (PSOFTUPDATE) pExtDataBuf;
	ASSERT( stricmp(pItem->szPath,"<WINSYSDIR>" ) == 0 );
	CString strDstFile = GetRealPath( "<WINSYSDIR>" ) + pItem->szFileName;

	strSrcFileName = strDstFile + ".Dll";
	strRetVal = strSrcFileName;									//	���ص��ļ���
	if( _access( strSrcFileName,0 ) == 0 )
		return strRetVal;										//	�Ѿ�����
	else
	{
		try	
		{
			CFile f;
			if( f.Open( strSrcFileName, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary ) == FALSE )
			{
				strRetVal.Empty();
				return strRetVal;									//	ʧ��
			}
			f.Write( pBuf, dwFileLen );
			f.Close();
		}
		catch( CFileException * e)
		{
			strRetVal.Empty();
			e->Delete();
			TRACE("MSG CFileUpdate::UpdateUnlockProc,  File IO failed,filename=%s\n",strSrcFileName);
			return strRetVal;
		}
	}
	BOOL bFileInUse = IsUsing( strDstFile );
	if( bFileInUse )
		AddToUpdateTaskList( strSrcFileName, strDstFile );		//	����ʹ����,�ҳ��ļ���
	else
	{
		if( SetupInstall( strSrcFileName, strDstFile ) )				//	����ʹ����, ��������
			AddToDelList( strSrcFileName );							//	��ӵ�ɾ�����ļ��б���
		else
			AddToUpdateTaskList( strSrcFileName, strDstFile );		//	����ʹ����,�ҳ��ļ���
		strRetVal = strDstFile;									//	�����ɹ�, ��Ŀ���ļ���
	}

	strSrcFileName = strDstFile;
	strSrcFileName += "  ---------  ";
	strSrcFileName += pItem->szHelp;
	CTime now = CTime::GetCurrentTime();
	strSrcFileName.Insert(0, now.Format("%Y.%m.%d %H:%M:%S ���յ������������� ==> ") );
	AddToUpdateLog( strSrcFileName );
	return strRetVal;
}

//	���ð�װ����,ֱ�ӿ�������
//	��ڲ���
//		pszSrcFileName				����Դ�ļ���
//		pszDstFileName				Ŀ���ļ���
BOOL CFileUpdate::SetupInstall(LPCSTR pszSrcFileName, LPCSTR pszDstFileName)
{
	ASSERT( pszSrcFileName && pszDstFileName );

	HINSTANCE hDll = ::LoadLibrary( "SetupAPI.DLL" );
	if( hDll == NULL )
		return FALSE;

	BOOL ( WINAPI* pFunc)(HINF,PINFCONTEXT,LPCSTR,LPCSTR,LPCSTR,DWORD,PSP_FILE_CALLBACK ,PVOID);

	pFunc = \
		(BOOL(WINAPI*)(HINF,PINFCONTEXT,LPCSTR,LPCSTR,LPCSTR,DWORD,PSP_FILE_CALLBACK ,PVOID))::GetProcAddress( hDll,"SetupInstallFileA" );
	if( pFunc )
	{																		//	2001.6.14 ȡ�� FORCE_IN_USE ��־
		pFunc( NULL, NULL, pszSrcFileName, NULL, pszDstFileName,\
			SP_COPY_SOURCE_ABSOLUTE|SP_COPY_NEWER_OR_SAME, NULL, NULL );	// ���а汾���

	}
	::FreeLibrary( hDll );
	return pFunc ? TRUE : FALSE;
}

//	��ӵ�����Ƶİ�װ����������
//	��ڲ���
//		pszSrcFileName				����Դ�ļ���
//		pszDstFileName				Ŀ���ļ���
//	ע:
//		�����ļ����б��� TS\UPDATE Ŀ¼�µ� UPDATE.INI
//		[Rename]
//		ITEMNUM = ����
//		ItemSrc1 = Դ�ļ���
//		ItemDst1 = Ŀ���ļ���
void CFileUpdate::AddToUpdateTaskList(LPCSTR pszSrcFileName, LPCSTR pszDstFileName)
{
char * pszSection =  "Rename";			//	�����ļ�
	ASSERT( pszSrcFileName && pszDstFileName );
	CString strFileName = GetTSMainPath() + "\\UPDATE\\UPDATE.INI";
	int nItem = ::GetPrivateProfileInt( pszSection, "ItemNum",0,strFileName );

	CString strTmp;
	strTmp.Format("%d",nItem+1);
	::WritePrivateProfileString( pszSection,"ItemNum",strTmp,strFileName );

	CString strKey;
	strKey.Format("ItemSrc%d",nItem);
	::WritePrivateProfileString( pszSection, strKey, pszSrcFileName, strFileName );
	strKey.Format("ItemDst%d",nItem);
	::WritePrivateProfileString( pszSection, strKey, pszDstFileName, strFileName );
}

//	����ļ��������� WinInit.Ini
//	��ڲ���
//		pszSrcFileName				����Դ�ļ���
//		pszDstFileName				Ŀ���ļ���
//	���ز���
//		FALSE						�Ѿ�������������
//		TRUE						�ɹ�
//	ע:
//		������������ֻ�ʺ��� 8.3 ��ʽ�Ķ��ļ���
BOOL CFileUpdate::AddFileNameToWinInit(LPCSTR pszSrcFileName, LPCSTR pszDstFileName)
{
	ASSERT( pszSrcFileName && pszDstFileName );
	CString strDst = pszDstFileName;
	strDst.MakeUpper();
	CString strTmp;
	::GetPrivateProfileSection( "rename", strTmp.GetBuffer( 32000 ), 32000, "WinInit.Ini" );
	strTmp.ReleaseBuffer();
	if( strTmp.Find( strDst + "=" ) >= 0 )
		return FALSE;				//	�Ѿ�����
	::WritePrivateProfileString( "rename",strDst,pszSrcFileName,"WinInit.Ini" );
	return TRUE;
}

//	��ȡ������¼
CString CFileUpdate::GetUpdateLog()
{
	CString strFileName = GetTSMainPath() + "\\Update\\Update.Log";
	CString strLog;
	try
	{
		CFile f;
		if( f.Open( strFileName, CFile::modeRead|CFile::typeBinary ) == FALSE )
			return strLog;
		int nLen = f.GetLength();
		f.Read( strLog.GetBuffer( nLen + 10 ), nLen );
		strLog.ReleaseBuffer( nLen );
		f.Close();
	}
	catch( CFileException * e)
	{
		strLog.Empty();
		TRACE("MSG CFileUpdate::GetUpdateLog, file IO failed, filename=%s\n",strFileName);
		e->Delete();
	}
	return strLog;
}

//	���һ��������¼��������¼�ļ���
void CFileUpdate::AddToUpdateLog(LPCSTR pszLog)
{
	CString strFileName = GetTSMainPath() + "\\Update\\Update.Log";
	CString strLog;
	FILE * fp ;
	fp = fopen( strFileName, "r+t" );
	if( fp == NULL )
		fp = fopen( strFileName, "wt" );
	if( fp )
	{
		fseek( fp,0,SEEK_END );
		fprintf( fp, "%s\n", pszLog );
		fclose( fp );
	}
}

//	��ӵ�ɾ�����ļ��б���
//	��ڲ���
//		pszFileName				�ļ���
//	ע:
//		�� Update\Update.Ini �е� [Delete] ����
//		ItemNum = ��������
//		Item0 = ��ɾ�����ļ���
void CFileUpdate::AddToDelList(LPCSTR pszFileName)
{
	ASSERT( pszFileName );
	const char * pszSection = "Delete";
	CString strFileName = GetTSMainPath() + "\\UPDATE\\UPDATE.INI";
	int nItem = ::GetPrivateProfileInt( pszSection, "ItemNum",0,strFileName );

	CString strTmp;
	strTmp.Format("%d",nItem+1);
	::WritePrivateProfileString( pszSection,"ItemNum",strTmp,strFileName );

	CString strKey;
	strKey.Format("Item%d",nItem);
	::WritePrivateProfileString( pszSection, strKey, pszFileName, strFileName );
}

//	��ȡͨ�ӵĹ�����Ŀ¼ 
CString CFileUpdate::GetTSMainPath()
{
	CMyRegKey	regkey( HKEY_LOCAL_MACHINE, "Software" );
	return regkey.GetProfileString( "Tongshi","mainPath","C:\\TS" );
}

//	��ȡ������Ŀ¼
CString CFileUpdate::GetRealPath(LPCSTR pszPath)
{
	CString strRet;
	int nSkipLen = 0;
	if( strnicmp( pszPath, "<TS>", 4 ) == 0 )
	{
		nSkipLen = 4;
		strRet.Format("%s%s", GetTSMainPath(), pszPath+nSkipLen );
	}
	else if( strnicmp( pszPath, "<WINDIR>", 8 ) == 0 )
	{
		nSkipLen = 8;
		GetWindowsDirectory( strRet.GetBuffer(_MAX_PATH),_MAX_PATH );
		strRet.ReleaseBuffer();
		strRet += (pszPath+nSkipLen);
	}
	else if( strnicmp( pszPath, "<WINSYSDIR>", 11 ) == 0 )
	{
		nSkipLen = 11;
		GetSystemDirectory( strRet.GetBuffer(_MAX_PATH),_MAX_PATH );
		strRet.ReleaseBuffer();
		strRet += (pszPath+nSkipLen);
	}
	else
	{
		strRet = pszPath;
		if( strRet.IsEmpty() )
			strRet = GetTSMainPath();
	}
	int nLen = strRet.GetLength();
	if( nLen && strRet[nLen-1] != '\\' )
		strRet += '\\';
	return strRet;
}

//	�Ƿ�
BOOL CFileUpdate::IsAShortFileName(LPCSTR pszFileName)
{
char szName[_MAX_PATH*4];		//  CYJ,2005-8-1 �޸Ļ���������
char szExtName[_MAX_PATH];
	_splitpath( pszFileName, NULL, NULL, szName, szExtName );
	if( strlen( szName ) > 8 )			//	���ļ���
		return FALSE;
	if( strstr( szName, "." ) )			//	���ļ���
		return FALSE;
	if( strlen( szExtName ) > 4 )
		return FALSE;
	strcat( szName, szExtName );
	if( strlen(szName) >= 13 )
		return FALSE;
	if( strstr( szName," " ) )
		return FALSE;
	return TRUE;
}

//	�ж�һ���ļ��Ƿ�ʹ����
//	��ڲ���
//		pszFileName						�ļ���
//	���ز���
//		TRUE							ʹ����
//		FALSE							��������
BOOL CFileUpdate::IsUsing(LPCSTR pszFileName)
{
	CFile f;
	if( _access( pszFileName,0 ) != 0 )
	{
		if( f.Open( pszFileName, CFile::modeCreate|CFile::shareExclusive|CFile::modeWrite|CFile::typeBinary ) )
			f.Close();							//	����
		return FALSE;							//	������, һ������ʹ����
	}
	if( f.Open( pszFileName, CFile::modeCreate|CFile::modeNoTruncate|CFile::shareExclusive|CFile::modeWrite|CFile::typeBinary ) == FALSE )
		return TRUE;
	f.Close();
	return FALSE;
}
