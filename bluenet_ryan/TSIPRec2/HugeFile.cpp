///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2003-8-7
///
///		��;��
///			������ļ�
///=======================================================
//  2003-9-20	�޸� NotifyOneSubFileOK�������ļ��Ѿ���������رո��ļ�

// HugeFile.cpp: implementation of the CHugeFile class.
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "IPRecSvr.h"
#include "HugeFile.h"

#ifdef _WIN32
  #include <io.h>
  #include <stdio.h>
  #include <stdlib.h>

  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif

#else
	#include <unistd.h>
    #include <utime.h>
    #define _access	access
    #define _unlink unlink
#endif //_WIN32  

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHugeFile::CHugeFile()
{
	Preset();
}

CHugeFile::~CHugeFile()
{
	OnHugeFileClose();
}

void CHugeFile::Close()
{
	if( m_hFile != hFileNull )
		CFile::Close();

	if( m_IsHugeFileAlreadOK )			//	has ready succ, and do not open the file
		return;							//	and this make the file shareable possible
	OnHugeFileClose();	
}

void CHugeFile::Abort()
{
	CFile::Abort();

	if( m_IsHugeFileAlreadOK )			//	has ready succ, and do not open the file
		return;							//	and this make the file shareable possible

	OnHugeFileClose();
}

#ifdef _WIN32
  BOOL CHugeFile::Open( LPCSTR lpszFileName, UINT nOpenFlags, CFileException* pError )
#else
  bool CHugeFile::Open( const char * lpszFileName, unsigned int nOpenFlags )
#endif //_WINew
{	
	m_IsHugeFileAlreadOK = FALSE;
	m_strFileName = lpszFileName;
	CString strFlagsFile = GetBitFlagsFileName();
    if( _access( lpszFileName, 0 ) != -1 )
	{									//	file exist, �����Ѿ��ɹ����գ������������жϷ�����
		if( _access( strFlagsFile, 0 ) != -1 )
			LoadRecFlags( strFlagsFile );	// exist
		else
			m_IsHugeFileAlreadOK = !IsHugeFileChanged();
	}

	if( !m_IsHugeFileAlreadOK )
	{
		TRY
		{
			CDWordArray & params = m_RecFlags.GetUserDefData();
			params.SetSize( 2 );
			PUSERDEFPARAMETER pParam = (PUSERDEFPARAMETER)params.GetData();
			pParam->m_dwFileLen = m_dwHugeFileLen;
			pParam->m_LastModifyTime = m_HugeFileLastModifyTime;
		}
		CATCH_ALL( e )
		{
			m_RecFlags.GetUserDefData().RemoveAll();
		}
		END_CATCH_ALL

		m_RecFlags.SaveToFile( strFlagsFile );		//	��¼��ʾ���ڽ���
	}

	if( m_IsHugeFileAlreadOK )						//	�Ѿ��ɹ����գ����ٴ���
		nOpenFlags = CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone;

#ifdef _WIN32
	return CFile::Open( m_strFileName, nOpenFlags, pError );
#else
	return CFile::Open( m_strFileName, nOpenFlags );
#endif //_WIN32
}

#ifdef _WIN32
  void CHugeFile::Write( const void* lpBuf, UINT nCount )
  {
	if( m_IsHugeFileAlreadOK )
		return;							//	�Ѿ�������
	CFile::Write( lpBuf, nCount );
  }
#else
  unsigned int CHugeFile::Write( const void* lpBuf, UINT nCount )
  {
	if( m_IsHugeFileAlreadOK )
		return nCount;							//	�Ѿ�������
	return CFile::Write( lpBuf, nCount );
  }
#endif //_WIN32

void CHugeFile::Preset()
{
	m_IsHugeFileAlreadOK = FALSE;
	m_HugeFileLastModifyTime = 0;		//	������ʱ�䣬�ж��Ƿ����
	m_dwHugeFileLen = 0;				//	���ļ��ļ�����
	m_strFileName = "";
	m_nTotalSubFileCount = 0;
	m_RecFlags.Reset();
}

///-------------------------------------------------------
/// CYJ,2003-8-7
/// Function:
///		Test whether current Huge file is changed
/// Input parameter:
///		None
/// Output parameter:
///		TRUE				changed
///		FALSE				same
BOOL CHugeFile::IsHugeFileChanged()
{
	CFileStatus fsta;
	if( FALSE == GetStatus( m_strFileName, fsta ) )
		return TRUE;				//	�����ڣ����Կ�����Ϊ�ı���

	if( fsta.m_mtime.GetTime() == m_HugeFileLastModifyTime && m_dwHugeFileLen == (DWORD)fsta.m_size  )
		return FALSE;				// same, not changed

	return TRUE;
}

///-------------------------------------------------------
/// CYJ,2003-8-7
/// Function:
///		��ȡ��¼�ļ������γɷ�����ԭ�ļ�����".$$$"�����Ѿ��ɹ����գ���ɾ���ü�¼�ļ�
/// Input parameter:
///		None
/// Output parameter:
///		None
CString CHugeFile::GetBitFlagsFileName()
{
	CString strRetVal = m_strFileName;
    strRetVal += ".$HF$$$";
	return strRetVal;
}

///-------------------------------------------------------
/// CYJ,2003-8-7
/// Function:
///		Huge file will be closed, check if the file is receive ok, 
///		if OK, then delete the record flags file and set the file last modify time to the original time of broadcast side
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHugeFile::OnHugeFileClose()
{
	if( m_IsHugeFileAlreadOK )
		return;
	
	CString strFlagFileName = GetBitFlagsFileName();

//	ASSERT( m_RecFlags.GetTotalSubFileCount() );
	if( 0 == m_RecFlags.GetTotalSubFileCount() )
		return;

	if( m_RecFlags.GetSubFileHasReceived() < m_RecFlags.GetTotalSubFileCount() )
	{								// Not received OK		
		m_RecFlags.SaveToFile( strFlagFileName );
		Preset();
		return;
	}

	m_IsHugeFileAlreadOK = TRUE;
	_unlink( strFlagFileName );		//	�Ѿ��ɹ�����

#ifdef _WIN32
	CFileStatus fstat;
	GetStatus( m_strFileName, fstat );
	fstat.m_mtime = m_HugeFileLastModifyTime;
	fstat.m_atime = m_HugeFileLastModifyTime;
	ASSERT( fstat.m_size == (long)m_dwHugeFileLen );
	SetStatus( m_strFileName, fstat );			//	��������޸�ʱ��
#else
	struct utimbuf filet;
    filet.actime = m_HugeFileLastModifyTime;
    filet.modtime = m_HugeFileLastModifyTime;
	utime( m_strFileName, &filet );
#endif //_WIN32

	Preset();
}

///-------------------------------------------------------
/// CYJ,2003-8-7
/// Function:
///		���ý��ղ���
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHugeFile::SetHugeFileParameter(DWORD dwFileLen, time_t LastModifyTime, int nSubFileCount)
{
	Preset();

	ASSERT( dwFileLen && nSubFileCount >= 0 );
	m_dwHugeFileLen = dwFileLen;

	m_HugeFileLastModifyTime = LastModifyTime;
	m_nTotalSubFileCount = nSubFileCount;
	m_RecFlags.SetTotalSubFileCount( nSubFileCount );
}

///-------------------------------------------------------
/// CYJ,2003-8-7
/// Function:
///		��¼���ļ����ճɹ�
/// Input parameter:
///		nSubFileNo				���ļ����
/// Output parameter:
///		true					full huge file received
///		false					not fully received
///	Modify log
//		2004-7-5, return true if huge file is fully recieved
//		2003-9-20, If the huge file is received succ, close the file, and the CDVBFileReceiver will reopen it if needed
bool CHugeFile::NotifyOneSubFileOK(int nSubFileNo)
{
	ASSERT( nSubFileNo >= 0 && nSubFileNo < m_RecFlags.GetTotalSubFileCount() );
	if( nSubFileNo >= 0 && nSubFileNo < m_RecFlags.GetTotalSubFileCount() )
	{
		m_RecFlags.SetBitValue( nSubFileNo, 1 );
		int nTotalSubFileCount = m_RecFlags.GetTotalSubFileCount();
		if( !m_IsHugeFileAlreadOK && nTotalSubFileCount && m_RecFlags.GetSubFileHasReceived() >= nTotalSubFileCount )
		{
			Close();				//  2003-9-20 �Ѿ����ճɹ����ر��ļ�		
			return true;
		}
	}
	return false;
}

///-------------------------------------------------------
/// CYJ,2003-8-8
/// Function:
///		Load Rec Flags data from file
/// Input parameter:
///		strFlagsFile		rec falgs data file name
/// Output parameter:
///		None
void CHugeFile::LoadRecFlags(CString &strFlagsFile)
{
	m_RecFlags.LoadFromFile( strFlagsFile );		//	����
	m_RecFlags.SetTotalSubFileCount( m_nTotalSubFileCount );
	CDWordArray & params = m_RecFlags.GetUserDefData();
	if( params.GetSize() >= 2 )
	{
		PUSERDEFPARAMETER pParam = (PUSERDEFPARAMETER)params.GetData();
		if( pParam->m_dwFileLen == m_dwHugeFileLen &&\
			pParam->m_LastModifyTime == m_HugeFileLastModifyTime )
		{											//	��ͬ���ļ�
			return;
		}
	}

	TRY
	{	
		m_RecFlags.CleanDataOnly();			//	������� 0 

		params.SetSize( 2 );
		PUSERDEFPARAMETER pParam = (PUSERDEFPARAMETER)params.GetData();
		pParam->m_dwFileLen = m_dwHugeFileLen;
		pParam->m_LastModifyTime = m_HugeFileLastModifyTime;
	}
	CATCH_ALL( e )
	{
		params.RemoveAll();
	}
	END_CATCH_ALL
}

#ifdef _WIN32
 void CHugeFile::SetLength( DWORD dwNewLen )
 {
	if( m_IsHugeFileAlreadOK )
		return;

	CFile::SetLength( dwNewLen );
 }
#else
  bool CHugeFile::SetLength( DWORD dwNewLen )
  {
	if( m_IsHugeFileAlreadOK )
		return true;

	return CFile::SetLength( dwNewLen );
}
#endif //_WIN32

