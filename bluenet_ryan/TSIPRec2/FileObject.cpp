///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2002-11-14
///
///=======================================================

// FileObject.cpp: implementation of the CFileObject class.
//
//////////////////////////////////////////////////////////////////////
//	2002.11.14 �޸� IP �� Port �ı�ʾ��ʽ����Ӻ��� MulticastParameter

#include "stdafx.h"
#include "FileObject.h"
#include "TSDB_Rec.h"
#include "DirectroyHelp.h"
#include "IPData.h"

#ifdef _WIN32
  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif
#else
  #include <utime.h>
#endif //_WIN32  

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
   
CFileObject::CFileObject() :
	CBufPacket4C<IFileObject>( 0, 4096 )			//	��4KΪ��λ���з���
{
	m_nMC_Port = 0;						//	�ಥ�˿�
	m_pHugeFileParam = NULL;	//	���ļ�����
	m_pExtData = NULL;			//	���Ӳ���
	m_pAttributeData = NULL;	//	���Ը��Ӳ���
	m_pFileHeader = NULL;		//	�ļ�ͷ
	m_PacketTime = 0;
	m_pDataPortItem = NULL;		//  2004-5-20 add
}

CFileObject::~CFileObject()
{

}

void CFileObject::SafeDelete()
{
	delete this;
}

//////////////////////////////////////////////
///����:
///			��ȡ�ļ���
///��ڲ���:
///			pVal		ִ�� BSTR ���͵ػ�����
///���ز���:
///			S_OK �ɹ�
LPCSTR CFileObject::GetFileName()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_strFileName;
}

//////////////////////////////////////////////
///����:
///			��ȡ�ļ�����
///��ڲ���:
///			pVal		�������
///���ز���:
///			
///ע��
//		pVal Ӧ�� DWORD ����
DWORD CFileObject::GetAttribute()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( m_pAttributeData )
		return m_pAttributeData->m_dwAttribute;
	return 0;
}

//////////////////////////////////////////////
///����:
///			��ȡ����޸�ʱ��
time_t CFileObject::GetLastModifyTime()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( m_pAttributeData )
		return m_pAttributeData->m_LastWriteTime;
	return 0;
}

//////////////////////////////////////////////
///����:
///		��������ʱ��
///��ڲ���:
///		
///���ز���:
///		
///ע��
///		
time_t CFileObject::GetCreatTime()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( m_pAttributeData )
		return m_pAttributeData->m_CreateTime;
	return 0;
}

//////////////////////////////////////////////
///����:
///		����������ʱ��
///��ڲ���:
///		
///���ز���:
///		
///ע��
///	
time_t CFileObject::GetLastAccessTime()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( m_pAttributeData )
		return m_pAttributeData->m_LastAccessTime;
	return 0;
}


//////////////////////////////////////////////
///����:
///			��ȡ�ļ���;����
///��ڲ���:
///		
///���ز���:
///		
DWORD CFileObject::GetFilePurpose()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( m_pAttributeData )
		return m_pAttributeData->m_dwPurpose;
	else
		return 0;
}

//////////////////////////////////////////////
///����:
///			��ȡ����ʱ�ش��ʱ��
///��ڲ���:
///		
///���ز���:
///		
time_t CFileObject::GetPacketTime()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_PacketTime;
}

//////////////////////////////////////////////
///����:
///		��ȡ�ļ����Ը�������
///��ڲ���:
///		pdwLen		output ExtData Len, default is NULL
///���ز���:
///		NULL		there is no extern data
PBYTE CFileObject::GetAttributeExtData( PDWORD pdwLen)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32
	if( pdwLen )
	{
		if( m_pAttributeData )
			*pdwLen = m_pAttributeData->m_cbSize - sizeof(TSDBFILEATTRIBHEAD) + 1;
		else
			*pdwLen = 0;
	}
	return (PBYTE)m_pAttributeData;
}

//////////////////////////////////////////////
///����:
///			��ȡ�ļ���������
///��ڲ���:
///		
///���ز���:
///		
PBYTE CFileObject::GetExtData( PDWORD pdwLen )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( pdwLen )
	{
		if( m_pExtData )
		{
			ASSERT( m_pFileHeader );
			CTSDBFileHeader Helper( m_pFileHeader );
			*pdwLen = Helper.ExtDataLen();
		}
		else
			*pdwLen = 0;
	}

	return m_pExtData;
}

//////////////////////////////////////////////
///����:
///		��ȡ���ļ�����
///��ڲ���:
///		
///���ز���:
///		
PBYTE CFileObject::GetHugeFileParam( PDWORD pdwLen )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32    

	if( pdwLen )
	{
		if( m_pHugeFileParam )
			*pdwLen = m_pHugeFileParam->m_cbSize;
		else
			*pdwLen = 0;
	}
	return PBYTE( m_pHugeFileParam );
}

//////////////////////////////////////////////
///����:
///			��ȡ IP Address
///��ڲ���:
///		
///���ز���:
///		
LPCSTR CFileObject::GetIPAddress()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32
	return (LPCSTR)m_strMC_DstIP;
}

//////////////////////////////////////////////
///����:
///			��ȡ�˿�
///��ڲ���:
///		
///���ز���:
///		
int CFileObject::GetPort()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif//_WIN32
	return m_nMC_Port;
}

//////////////////////////////////////////////
//����:
//		�����ļ�
//��ڲ���:
//		lpszPath				��Ŀ¼
//		bIgnorSubDirectroy		�Ƿ������Ŀ¼
//		bRestoreTime			�Ƿ�ԭʱ��
//���ز���:
//		
BOOL CFileObject::SaveTo(LPCSTR lpszPath, BOOL bIgnoreSubDirectory, BOOL bRestoreTimes)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	ASSERT( lpszPath && *lpszPath );
	if( NULL == lpszPath || 0 == *lpszPath )
		return FALSE;

	CString strFileName = lpszPath;
	int nLen = strFileName.GetLength();
#ifdef _WIN32
	if( strFileName[nLen-1] != '\\' )
		strFileName += '\\';				//	��һ��
#else
	if( strFileName[nLen-1] != '/' )
		strFileName += '/';				//	��һ��
#endif //_WIN32

	if( bIgnoreSubDirectory )
	{
		const char * pszFileName = strrchr( m_strFileName, '\\' );		//	��������
		if( NULL == pszFileName )
			pszFileName = strrchr( m_strFileName, '/' );				//	��������

		if( NULL == pszFileName )
			pszFileName = m_strFileName;
		else
			pszFileName ++;					//	���� '\\' or '/'
		strFileName += pszFileName;
	}
	else
		strFileName += m_strFileName;

	CDirectroyHelp::Mkdir( strFileName );

	UINT uCreateFlags = CFile::modeCreate|CFile::modeWrite|CFile::typeBinary|CFile::shareDenyWrite;

	BOOL bIsHugeSubFile = FALSE;	
	if( m_pFileHeader && m_pHugeFileParam && GetDataLen() != m_pHugeFileParam->m_dwFileLen )
	{										//	���ļ����������ļ�������Ҫ��λ
		bIsHugeSubFile = TRUE;				//	����һ�������Ĵ��ļ��������ļ�
		uCreateFlags |= CFile::modeNoTruncate;		//	����
	}

	CFile f;
	if( FALSE == f.Open( strFileName, uCreateFlags ) )
		return FALSE;
	TRY
	{
		if( bIsHugeSubFile )
		{
			ASSERT( m_pHugeFileParam );
			f.SetLength( m_pHugeFileParam->m_dwFileLen );
			f.Seek( m_pHugeFileParam->m_dwFilePosition, CFile::begin );
		}
		f.Write( GetBuffer(), GetDataLen() );
		
		if( bRestoreTimes && m_pAttributeData )
		{													//	��ԭʱ��
#ifdef _WIN32
			FILETIME fileTime[3];
			time_t attribtime[3] = { m_pAttributeData->m_CreateTime, m_pAttributeData->m_LastAccessTime, m_pAttributeData->m_LastWriteTime };
			FILETIME * pFileTime[3];
			SYSTEMTIME sysTmpTime;

			for(int i=0; i<3; i++)
			{
				CTime t = attribtime[i];
				if( t.GetAsSystemTime( sysTmpTime) )
				{
					pFileTime[i] = fileTime + i;
					::SystemTimeToFileTime( &sysTmpTime, pFileTime[i] );
				}
				else
					pFileTime[i] = NULL;
			}

			::SetFileTime( (HANDLE) f.m_hFile, pFileTime[0], pFileTime[1], pFileTime[2] );
            f.Close();
#else
			f.Close();
			struct utimbuf filet;
            filet.actime = m_pAttributeData->m_LastAccessTime;
            filet.modtime = m_pAttributeData->m_LastWriteTime;
			utime( strFileName, &filet );
#endif //_WIN32
		}
		else
			f.Close();
	}
	CATCH_ALL( e )
	{
#if	defined(_DEBUG) && defined(_WIN32)
		e->ReportError();
#endif // _DEBUG
		f.Abort();
	}
	END_CATCH_ALL

	return TRUE;
}

//////////////////////////////////////////////
//����:
//		�ӻ������з��� TSDB ��װ�Ĳ���
//��ڲ���:
//		��
//���ز���:
//		��
//ע��
//		һ���ڵ��øú����󣬽����� OnFileOK �¼�
void CFileObject::DoTSDBSingleFile()
{
	m_pHugeFileParam = NULL;	//	���ļ�����
	m_pExtData = NULL;			//	���Ӳ���
	m_pAttributeData = NULL;	//	���Բ���

	m_pFileHeader = (PTSDBFILEHEADER)GetBuffer();
	CTSDBFileHeader	hdr( m_pFileHeader );
	ASSERT( hdr.IsFileHead() );

	ASSERT( (m_pFileHeader->m_dwFileLen + m_pFileHeader->m_cbSize) == GetDataLen() );
	PutDataLen( m_pFileHeader->m_dwFileLen );
	Admin_AccessReservedBytes() += m_pFileHeader->m_cbSize;		//	��������������

	if( hdr.HasFileAttarib() )
		m_pAttributeData = hdr.GetFileAttribHead();
	if( hdr.HasExtData() )
		m_pExtData = GetBuffer() + hdr.ExtDataLen();
	if( hdr.IsHugeFile() )
		m_pHugeFileParam = hdr.GetHugeFileHead();

	m_strFileName = hdr.GetFileName();
#ifdef _WIN32
	m_strFileName.Replace( '/', '\\' );
#else
	m_strFileName.Replace( '\\', '/' );
#endif //_WIN32
}

//////////////////////////////////////////////
//����:
//		���ļ����ճɹ�ʱ���ر������ļ�ͷ����
//��ڲ���:
//		pFileHead			�ļ�ͷ������
//���ز���:
//		TRUE				�ɹ�
//		FALSE				ʧ��
BOOL CFileObject::SetHugeFileFileHeader(PTSDBFILEHEADER pFileHead)
{
	ASSERT( pFileHead );

	m_pHugeFileParam = NULL;	//	���ļ�����
	m_pExtData = NULL;			//	���Ӳ���
	m_pAttributeData = NULL;	//	���Բ���

	TRY
	{
		m_FileHeadBuf.SetSize( pFileHead->m_cbSize );
	}
	CATCH( CMemoryException, e )
	{
#if defined(_DEBUG) && defined(_WIN32)
		e->ReportError();
#endif // _DEBUG
		m_pFileHeader = NULL;
		return FALSE;
	}
	END_CATCH
#ifdef _WIN32
	RtlCopyMemory( m_FileHeadBuf.GetData(), pFileHead, pFileHead->m_cbSize );
#else
	memcpy( m_FileHeadBuf.GetData(), pFileHead, pFileHead->m_cbSize );
#endif //_WIN32
	m_pFileHeader = (PTSDBFILEHEADER) m_FileHeadBuf.GetData();

	CTSDBFileHeader	hdr( m_pFileHeader );
	ASSERT( hdr.IsFileHead() );

	if( hdr.HasFileAttarib() )
		m_pAttributeData = hdr.GetFileAttribHead();
	if( hdr.HasExtData() )
		m_pExtData = m_FileHeadBuf.GetData() + m_pFileHeader->m_cbSize - hdr.ExtDataLen();
	if( hdr.IsHugeFile() )
		m_pHugeFileParam = hdr.GetHugeFileHead();

	m_strFileName = hdr.GetFileName();
	return TRUE;
}

//////////////////////////////////////////////
// 2002.5.22 ���
// ����:
//		Ԥ�ò���ֵ
// ��ڲ���:
//		��
// ���ز���:
//		��
void CFileObject::PresetVar()
{
	m_pHugeFileParam = NULL;	//	���ļ�����
	m_pExtData = NULL;			//	���Ӳ���
	m_pAttributeData = NULL;	//	���Բ���
	m_pFileHeader = NULL;
	m_strMC_DstIP = "";			//	2002.11.14 �޸ģ���ಥ��صĲ���
	m_nMC_Port = 0;				//	�ಥ�˿� 
	m_pDataPortItem = NULL;
}

///-------------------------------------------------------
/// 2002-11-14
/// ���ܣ�
///		���öಥ IP �Ͷ˿�
/// ��ڲ�����
///		lpszIP				�ಥ IP ��ַ
///		wPort				�˿�
/// ���ز�����
///		��
void CFileObject::SetMulticastParameter(LPCSTR lpszIP, WORD wPort, COneDataPortItem * pDataPortItem)
{
	ASSERT( lpszIP && wPort && pDataPortItem );
	if( lpszIP )
		m_strMC_DstIP = lpszIP;
	else
		m_strMC_DstIP = "";
	m_nMC_Port = wPort;
	m_pDataPortItem = pDataPortItem;
}
