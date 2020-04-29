// MyMapFile.cpp: implementation of the CMyMapFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyMapFile.h"
#include <sys/mman.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyMapFile::CMyMapFile()
{
	m_bReadOnly = TRUE;
	m_pMappedBuffer = NULL;
}

CMyMapFile::~CMyMapFile()
{
	Close();
}

///-----------------------------------------------------------
/// ���ܣ�
///		��ӳ�书��
/// ��ڲ�����
///		lpszFileName			Ŀ���ļ�����ȫ·���������ļ���ΪNULL����""ʱ����ʾʹ���ڴ湲���ļ�����ʱ lpszShareName ����Ϊ 0
///		lpszShareName			�����ļ�����һ���� lpszFileName = NULL ʱ��ʹ��
/// ���ز�����
///		TRUE					�ɹ�
///		FALSE					ʧ��
BOOL CMyMapFile::MapFileForReadOnly(LPCSTR lpszFileName, LPCSTR lpszShareName)
{
	ASSERT( lpszFileName || lpszShareName );		//	���б���һ������Ϊ NULL

	return MapFile( lpszFileName, lpszShareName, MAPFILE_MODE_READONLY );
}

///-------------------------------------------------------
/// CYJ,2003-10-25
/// Function:
///		Open File and map it, with user defined parameter
/// Input parameter:
///		lpszFileName		file name to be mapped
///		lpszShareName		share mapped file name
///		nMode				map mode
///		pbIsExist			output is exist or not, default is NULL
///		dwLowPos			low part 
///		dwHightPos			hight pos
/// Output parameter:
///		None
BOOL CMyMapFile::MapFile(LPCSTR lpszFileName, LPCSTR lpszShareName, int nMode, BOOL * pbIsExist, DWORD dwLowPos, DWORD dwHightPos)
{
	ASSERT( lpszFileName || lpszShareName );		//	���б���һ������Ϊ NULL

	Close();

	DWORD dwOpenFileFlags = 0;
	DWORD dwMapFlags = 0;
	int   nMapProt = 0;
	if( MAPFILE_MODE_READONLY == nMode  )
	{									// read only
		dwOpenFileFlags = CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite;
		m_bReadOnly = TRUE;
		nMapProt = PROT_READ;
		dwMapFlags = MAP_SHARED;
	}
	else if( MAPFILE_MODE_WRITEONLY == nMode )
	{									//	write only
		dwOpenFileFlags = CFile::modeCreate|CFile::modeWrite|CFile::typeBinary|CFile::shareDenyWrite;
		m_bReadOnly = FALSE;
		nMapProt = PROT_WRITE;
		dwMapFlags = MAP_SHARED;
	}
	else
	{									// read write
		dwOpenFileFlags = CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite|CFile::typeBinary|CFile::shareDenyWrite;
		m_bReadOnly = FALSE;
		nMapProt = PROT_WRITE|PROT_READ;
		dwMapFlags = MAP_SHARED;
	}

	if( lpszFileName && strlen(lpszFileName) )
	{
		if( FALSE == m_file.Open( lpszFileName, dwOpenFileFlags ) )
			return FALSE;
		if( 0 == m_file.GetLength() )
		{
			m_file.Abort();
			return FALSE;
		}
	}
	else
		m_file.m_hFile = CFile::hFileNull;
 
	m_pMappedBuffer = (PBYTE) mmap( 0, m_file.GetLength(), nMapProt, dwMapFlags, m_file.m_hFile, 0 );
	if( NULL == m_pMappedBuffer )
	{
		Close();
		return FALSE;
	}	

	return TRUE;
}

///-----------------------------------------------------------
/// ���ܣ�
///		�ж��Ƿ���Ч
/// ��ڲ�����
///		��
/// ���ز�����
///		TRUE					��Ч�����Բ���
///		FAKSE					��Ч
BOOL CMyMapFile::IsValid()
{
	return NULL != m_pMappedBuffer;
}

///-----------------------------------------------------------
/// ���ܣ�
///		��ȡ�ڴ��ַ
/// ��ڲ�����
///		��
/// ���ز�����
///		�ڴ��ַ
PBYTE CMyMapFile::GetBuffer()
{
	ASSERT( IsValid() );
	return m_pMappedBuffer;
}

///-----------------------------------------------------------
/// ���ܣ�
///		�ر�ӳ��
/// ��ڲ�����
///		��
/// ���ز�����
///		��
void CMyMapFile::Close()
{
	if( m_pMappedBuffer )
		munmap( (void*)m_pMappedBuffer, GetFileLen() );
	m_pMappedBuffer = NULL;
	m_file.Abort();
}

///-----------------------------------------------------------
/// ���ܣ�
///		��ȡ�ļ�����
/// ��ڲ�����
///		��
/// ���ز�����
///		�ļ�����
DWORD CMyMapFile::GetFileLen()
{
	ASSERT( IsValid() );
	if( FALSE == IsValid() )
		return 0;
	return m_file.GetLength();
}

