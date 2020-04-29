///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2002-11-9
///
///=======================================================
//  2003-6-9 �޸ĺ��� SetFilePathParameters �У�ǿ�ƽ� m_nLogFileNo = -1

// LogFileHelper.cpp: implementation of the CLogFileHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LogFileHelper.h"

#ifdef _WIN32
	#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
	#endif
#else
	#include <sys/stat.h>
    #include <sys/types.h>
#endif //_WIN32

#define _MAX_PATH	260    

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///-------------------------------------------------------
/// 2002-11-9
/// ���ܣ�
///		
/// ��ڲ�����
///		lpszFullPath			ȫ·��������log�ļ��������ܰ�����չ����
///								��չ��Ϊ .LOG
///								ȱʡΪ NULL����Ĭ��Ϊ��ǰĿ¼�� log ��Ŀ¼��
///		nMaxCount				�ļ�����ȱʡΪ 10
///		nLenGate				�л��ļ����ż����ȣ�ȱʡΪ 1M
/// ���ز�����
///
CLogFileHelper::CLogFileHelper(LPCSTR lpszFullPath, int nMaxCount, int nLenGate)
{
	m_nLogFileNo = -1;
	SetFilePathParamters( lpszFullPath, nMaxCount, nLenGate );
}

CLogFileHelper::~CLogFileHelper()
{

}

///-------------------------------------------------------
/// 2002-11-9
/// ���ܣ�
///		
/// ��ڲ�����
///		lpszFullPath			ȫ·��������log�ļ��������ܰ�����չ����
///								��չ��Ϊ .LOG
///								ȱʡΪ NULL����Ĭ��Ϊ��ǰĿ¼�� log ��Ŀ¼��
///		nMaxCount				�ļ�����ȱʡΪ 10
///		nLenGate				�л��ļ����ż����ȣ�ȱʡΪ 1M
/// ���ز�����
///		��
void CLogFileHelper::SetFilePathParamters(LPCSTR lpszFullPath, int nMaxCount, int nLenGate)
{
	ASSERT( nMaxCount && nLenGate );

	m_nLogFileNo = -1;			//  2003-6-9 ��ͷ��ʼ��¼

	if( lpszFullPath && strlen(lpszFullPath) )
	{
		m_strFullPath = lpszFullPath;
		int nLen = m_strFullPath.GetLength();
#ifdef _WIN32
		if( nLen && m_strFullPath[nLen-1] == '\\' )
			m_strFullPath.ReleaseBuffer( nLen-1 );		//	�������'\\'��β
#else
		if( nLen && m_strFullPath[nLen-1] == '/' )
			m_strFullPath.ReleaseBuffer( nLen-1 );		//	�������'\\'��β
#endif //_WIN32
	}
	else
	{
		char szDir[_MAX_PATH];
		char szPath[_MAX_PATH];
		char szFileName[_MAX_PATH];
#ifdef _WIN32
		_splitpath( __argv[0], szDir, szPath, szFileName, NULL );
		m_strFullPath.Format( "%s%sLog\\%s", szDir, szPath, szFileName );

        CString strTmp;
		strTmp.Format("%s%sLog", szDir, szPath );
		::CreateDirectory( strTmp, NULL );
#else
		m_strFullPath = "Log/LogData";
        mkdir( "Log", 0 );
#endif //_WIN32
	}	

	if( nLenGate > 10*1024 )
		m_nLenGate = nLenGate;
	else
		m_nLenGate = 10*1024;

	if( nMaxCount > 1 )
		m_nMaxCount = nMaxCount;
	else
		m_nMaxCount = 10;

	Abort();							//	�رյ�ǰ�ļ�
}

///-------------------------------------------------------
/// 2002-10-22
/// ���ܣ�
///		����־�ļ�
/// ��ڲ�����
///		��
/// ���ز�����
///		TRUE				�ɹ�
///		FALSE				ʧ��
BOOL CLogFileHelper::OpenLogFile()
{
#ifdef _WIN32
	TRY
	{
#endif //_WIN32
		if( m_hFile != CFile::hFileNull )
			Close();
#ifdef _WIN32
	}
	CATCH( CFileException, e )
	{
		Abort();
	}
	END_CATCH
#endif //_WIN32    

	m_nLogFileNo ++;
	if( m_nLogFileNo >= m_nMaxCount || m_nLogFileNo < 0 )
		m_nLogFileNo = 0;

	CString strTmp = GetCurrentLogFileName();
	return Open( strTmp, CFile::modeCreate|CFile::modeWrite|CFile::typeText|CFile::shareDenyWrite );
}

///-------------------------------------------------------
/// 2002-11-9
/// ���ܣ�
///		д�� Log �ļ���
/// ��ڲ�����
///		strLog			��д����ַ���
/// ���ز�����
///		��
///	ע��
///		strLog ����Ҫ���뻻�лس�
void CLogFileHelper::WriteToLogFile(LPCSTR lpszLog)
{
	if( m_hFile == CFile::hFileNull )
	{
		if( FALSE == OpenLogFile() )
			return;
	}
#ifdef _WIN32
	TRY
	{
#endif //_WIN32
		WriteString( lpszLog );
		if( GetLength() >= 1024*1024 )	//	1M
			OpenLogFile();			//	�رյ�ǰLog�ļ��������µ�Log�ļ�
#ifdef _WIN32
	}
	CATCH( CFileException, e)
	{
		return;
	}
	END_CATCH
#endif //_WIN32    
}

///-------------------------------------------------------
/// 2002-11-9
/// ���ܣ�
///		��ȡ��ǰlog�ļ���
/// ��ڲ�����
///		��
/// ���ز�����
///		��ǰ�ļ���
CString CLogFileHelper::GetCurrentLogFileName()
{
	CString strTmp;
	strTmp.Format("%s%d.LOG", (LPCSTR)m_strFullPath, m_nLogFileNo );
	return strTmp;
}
