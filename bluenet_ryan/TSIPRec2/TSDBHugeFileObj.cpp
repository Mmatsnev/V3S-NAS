// TSDBHugeFileObj.cpp: implementation of the CTSDBHugeFileObj class.
//
//						�޸ļ�¼
//	�޸�ʱ��			����
//////////////////////////////////////////////////////////////////////
//	2002.3.29			��� AddRef �� Release ����
//	2001.4.5			tagFLAGFILE ��������� m_bIsOpen
//						~CTSDBHugeFileObj �رմ��ļ�
//						SetOwnerHandle �����жϴ��ļ��Ƿ������ر�
//						IsFileOK ��Ӷ��ļ�CRC32���ж�
//	1999.12.19			�����մ��ļ���ʱ����Ŀ¼


#include "stdafx.h"
#include "resource.h"
#include "TSDBHugeFileObj.h"
#include "DirectroyHelp.h"

#ifdef _WIN32
  #include "MyRegKey.h"

  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif
#endif //_WIN32

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTSDBHugeFileObj::CTSDBHugeFileObj()
{
	m_nRef = 0;
	Init();
}

//	��ڲ���
//		pHeader						����ͷ
CTSDBHugeFileObj::CTSDBHugeFileObj(PTSDBHUGEFILEHEAD pHeader)
{
	Attach( pHeader );
}

//	�ر�ӳ���ļ�
//	�ر��ļ�
CTSDBHugeFileObj::~CTSDBHugeFileObj()
{
	if( m_bIsOwner )							//	ԭ��������, �ͷ�ʹ��Ȩ
	{
		m_pFlagBuf->m_bHasOwner = FALSE;			
		m_pFlagBuf->m_bCloseErr = FALSE;		//	2001.4.5 ��ǹر�
	}
	if( m_pFlagBuf )
		::UnmapViewOfFile(m_pFlagBuf);
	if( m_pDataBuf )
		::UnmapViewOfFile( m_pDataBuf );
	if( m_hmapFlagFile )
		::CloseHandle( m_hmapFlagFile );
	if( m_hmapDataFile )
		::CloseHandle( m_hmapDataFile );
	if( m_DataFile.m_hFile != CFile::hFileNull  )
		m_DataFile.Close();
	if( m_FlagFile.m_hFile != CFile::hFileNull  )
		m_FlagFile.Close();
}

//	��ʼ������
void CTSDBHugeFileObj::Init()
{
	m_LastAccessTime = 0;
	m_bMsgSended = FALSE;						//	δ�����͹���Ϣ
	m_bIsOwner = FALSE;
	m_pFlagBuf = NULL;							//	����ļ�������
	m_pDataBuf = NULL;							//	�����ļ�������
	m_hmapFlagFile = NULL;						//	��־�ļ�ӳ����
	m_hmapDataFile = NULL;						//	�����ļ�ӳ����
}

//	������ͷ���ŵ�����
//	��ڲ���
//		pHeader					����ͷ
//	���ز���
//		TRUE					�ɹ�
//		FALSE					ʧ��
BOOL CTSDBHugeFileObj::Attach(PTSDBHUGEFILEHEAD pHeader)
{
	ASSERT( pHeader );
	Init();
	if( !pHeader || pHeader->m_dwFileLen==0 || pHeader->m_szTmpFileName[0]==0 )
		return FALSE;					//	�Ƿ�ָ�� �� �ļ�����=0 �� û����ʱ�ļ���
	int nFlagLen = (pHeader->m_wTotalBlock+7) / 8;		//	�������ļ���¼��ǻ�������С
	nFlagLen += sizeof( FLAGFILE );

	CMyRegKey regkey(HKEY_LOCAL_MACHINE,"Software");
	CString	strWorkPath =  regkey.GetProfileString( "Tongshi","mainPath","C:\\TS" );
	strWorkPath += "\\TEMP\\HUGEFILE\\";
	CDirectroyHelp::Mkdir( strWorkPath );				//	����Ŀ¼

	m_pDataBuf = CreateAndMapFile(strWorkPath + pHeader->m_szTmpFileName,\
		pHeader->m_dwFileLen, m_DataFile, m_hmapDataFile );
	strWorkPath += pHeader->m_szTmpFileName;
	strWorkPath += ".REC";								//	���
	m_pFlagBuf = (PFLAGFILE) CreateAndMapFile(strWorkPath,nFlagLen, m_FlagFile, m_hmapFlagFile );
	if( m_pDataBuf == NULL || m_pFlagBuf == NULL )
		return FALSE;
	memcpy( &m_pFlagBuf->m_Head, pHeader, sizeof(TSDBHUGEFILEHEAD) );
	if( m_pFlagBuf->m_bHasOwner == FALSE || m_pFlagBuf->m_bCloseErr )
	{
		m_pFlagBuf->m_bHasOwner = FALSE;
		m_bIsOwner = SetOwnerHandle();
	}
	return TRUE;
}

//	������ӳ��
//	��ڲ���
//		pszFileName				�ļ���
//		dwFileLen				�ļ�����
//		file					CFile	����
//		hOut					��� CreateFileMapping �ľ��
//	�������
//		NULL					ʧ��
PBYTE CTSDBHugeFileObj::CreateAndMapFile(LPCSTR pszFileName, DWORD dwFileLen, CFile &file, HANDLE &hOut)
{
BOOL	bIsCreate = FALSE;
	hOut = NULL;
	ASSERT( pszFileName && dwFileLen );
	if( file.Open(pszFileName,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite|CFile::typeBinary|CFile::shareDenyNone) == FALSE )
		return NULL;
	for(int i=0; i<2; i++)
	{
		try
		{
			if( file.GetLength() != dwFileLen )
			{
				file.SetLength( dwFileLen );
				bIsCreate = TRUE;
			}
		}
		catch( CFileException * e )
		{
			e->Delete();
			ClearHugeFileTmpBuf();			//	û�пռ�,��Ҫ����ռ�
			if( i )
			{
				file.Close();
				return NULL;				//	�Ѿ����Թ���,ֻ�÷���
			}
		}
	}
	hOut = ::CreateFileMapping( (HANDLE)file.m_hFile,NULL,PAGE_READWRITE,0,0,NULL );
	if( hOut == NULL )
		return NULL;
	PBYTE pRetVal = (PBYTE)::MapViewOfFile(hOut,FILE_MAP_ALL_ACCESS,0,0,0 );
	if( pRetVal && bIsCreate )
		memset( pRetVal,0,dwFileLen );						//	���������
	return pRetVal;
}

//	���� Owner ����
BOOL CTSDBHugeFileObj::SetOwnerHandle()
{
	ASSERT(m_pFlagBuf);
	ASSERT( m_pFlagBuf->m_bHasOwner == FALSE );

	CString	strTmp = "Mutex_TS_HUGEFILE_";
	strTmp += m_pFlagBuf->m_Head.m_szTmpFileName;
	HANDLE hMutex = CreateMutex( NULL,FALSE, strTmp );
	if( hMutex == NULL )
		return FALSE;									//	����ʧ��
	BOOL bIsOwner = ( GetLastError() != ERROR_ALREADY_EXISTS );
	if( bIsOwner )
	{
		if( m_pFlagBuf->m_bCloseErr )					//	2001.4.5 ���
		{												//	˵���ϴ�û����ȷ�ػ�
			m_pFlagBuf->m_dwBlockReceived = 0;
			int nFlagBytes = (m_pFlagBuf->m_Head.m_wTotalBlock+7) / 8;
			memset( m_pFlagBuf->m_abyFlags, 0, nFlagBytes );
		}
		m_pFlagBuf->m_bHasOwner = TRUE;					//	���Ϊ���˵�λ
		m_pFlagBuf->m_bCloseErr = TRUE;					//	2001.4.5  ����Ѿ���
	}
	::CloseHandle( hMutex );
	return bIsOwner;
}

//	���ļ��Ƿ���� OK
BOOL CTSDBHugeFileObj::IsFileOK()
{
	ASSERT( m_pFlagBuf && m_pFlagBuf );
	if( NULL == m_pFlagBuf || NULL == m_pFlagBuf)
		return FALSE;
	ASSERT( m_pFlagBuf->m_dwBlockReceived <= m_pFlagBuf->m_Head.m_wTotalBlock );
	if( m_pFlagBuf->m_dwBlockReceived < m_pFlagBuf->m_Head.m_wTotalBlock )
		return FALSE;
													//	2001.4.5 ��Ӷ��ļ�CRC32���ж�
	if( CCRC::GetCRC32( m_pFlagBuf->m_Head.m_dwFileLen, GetDataBuf() ) == m_pFlagBuf->m_Head.m_dwFileCRC32 )
		return TRUE;
	if( m_bIsOwner )
	{												//	2001.4.5 �������ˣ��޸Ĵ���
		m_pFlagBuf->m_dwBlockReceived = 0;
		int nFlagBytes = (m_pFlagBuf->m_Head.m_wTotalBlock+7) / 8;
		memset( m_pFlagBuf->m_abyFlags, 0, nFlagBytes );
	}
	return FALSE;	
}

//	ȡ���ļ��Ľ��հٷֱ�
float CTSDBHugeFileObj::GetPercentage()
{
	ASSERT( m_pFlagBuf );
	if( !m_pFlagBuf || m_pFlagBuf->m_Head.m_wTotalBlock == 0 )
		return 0.0f;
	float f0 = (float)m_pFlagBuf->m_dwBlockReceived;
	f0 /= m_pFlagBuf->m_Head.m_wTotalBlock;
	return f0;
}

//	�����ļ��Ƿ����
//	��ڲ���
//		nBlockNo				���ļ����
//	���ز���
//		TRUE					�ɹ�
//		FALSE					ʧ��
BOOL CTSDBHugeFileObj::IsBlockOK(int nBlockNo)
{
	ASSERT( m_pFlagBuf );
	int nOffset = nBlockNo / 8;
	BYTE byMask = 1 << (nBlockNo & 7);
	return ( m_pFlagBuf->m_abyFlags[ nOffset ] & byMask );
}

//	����ָ���ļ�
//	��ڲ���
//		nBlockNo				���ļ����
//	���ز���
//		�ļ��Ƿ���ճɹ�
BOOL CTSDBHugeFileObj::SetBlockNo(int nBlockNo)
{
	ASSERT( m_pFlagBuf && m_bIsOwner);
	ASSERT( nBlockNo < m_pFlagBuf->m_Head.m_wTotalBlock );
	int nOffset = nBlockNo / 8;
	BYTE byMask = 1 << (nBlockNo & 7);
	if( (m_pFlagBuf->m_abyFlags[ nOffset ] & byMask ) == 0 )
	{												//	δ�����յ�
		m_pFlagBuf->m_abyFlags[ nOffset ] |= byMask;
		m_pFlagBuf->m_dwBlockReceived ++;
	}
	ASSERT( m_pFlagBuf->m_dwBlockReceived <= m_pFlagBuf->m_Head.m_wTotalBlock );
	return IsFileOK();								//	2001.4.5 �޸�
}

//	����һ�����ļ�
//	��ڲ���
//		pHead					����ͷ
//		pDataBuf				���ݻ�����
//	���ز���
//		�ļ��Ƿ���ճɹ�
BOOL CTSDBHugeFileObj::SaveBlock(PTSDBHUGEFILEHEAD pHead, PBYTE pDataBuf)
{
	ASSERT( pHead && pDataBuf );
	ASSERT( m_pDataBuf );
	ASSERT( pHead->m_dwFileCRC32 == m_pFlagBuf->m_Head.m_dwFileCRC32 );
	if( IsFileOK() )
		return TRUE;
	m_LastAccessTime = CTime::GetCurrentTime().GetTime();			//	���·���ʱ��
	if( m_bIsOwner == FALSE && m_pFlagBuf->m_bHasOwner == FALSE )	//	ԭ���������˳�, Ҫ�½�������
		m_bIsOwner = SetOwnerHandle();
	if( m_bIsOwner )
	{								//	������, ��Ȩ�޸�������
		memcpy( m_pDataBuf + pHead->m_dwFilePosition, pDataBuf, pHead->m_wBlockSize );
		return SetBlockNo( pHead->m_wBlockNo );
	}
	else
		return IsFileOK();			//	������, ֻ�в�ѯ�ķ���
}

//	ȡ�ļ�������
//	�� m_pDataBuf != NULL ��ʾ�����ɹ�
PBYTE CTSDBHugeFileObj::GetDataBuf()
{
	return m_pDataBuf;
}

//	ȡ�ļ�����
DWORD CTSDBHugeFileObj::GetFileLen()
{
	ASSERT( m_pFlagBuf );
	return m_pFlagBuf->m_Head.m_dwFileLen;
}

//	�Ƿ�ͬһ�����ļ�
//	��ڲ���
//		pHeader					����ͷ
BOOL CTSDBHugeFileObj::IsSameObj(PTSDBHUGEFILEHEAD pHeader)
{
	ASSERT( pHeader && m_pFlagBuf );
	if( pHeader->m_dwFileCRC32 != m_pFlagBuf->m_Head.m_dwFileCRC32 )
		return FALSE;
	return( stricmp( pHeader->m_szTmpFileName, m_pFlagBuf->m_Head.m_szTmpFileName ) == 0 );
}

//	��մ��ļ���ʱ����Ŀ¼
void CTSDBHugeFileObj::ClearHugeFileTmpBuf()
{
	CFileFind	finder;
	CMyRegKey	regkey( HKEY_LOCAL_MACHINE, "Software" );
	CString strPath = regkey.GetProfileString( "Tongshi","mainPath","C:\\TS" );
	strPath += "\\Temp\\HugeFile";
	if( finder.FindFile( strPath+"\\*.*" ) == FALSE )
		return;
	BOOL bFindNext = TRUE;
	CString strTmp;
	do
	{
		bFindNext = finder.FindNextFile();
		if( finder.IsDots() == FALSE )
		{
			strTmp = finder.GetFilePath();
			if( strTmp.IsEmpty() == FALSE )
				remove( strTmp );						//	ɾ���ļ�
		}
	}while( bFindNext );
}

//////////////////////////////////////////////
// 2002.3.29	���
//����:
//		�������ü�����
//��ڲ���:
//		��
//���ز���:
//		���ô���
long CTSDBHugeFileObj::AddRef()
{
	return ::InterlockedIncrement( &m_nRef );
}

//////////////////////////////////////////////
// 2002.3.29	���
//����:
//		�������ü������������� 0 ��ʱ��ɾ���Լ�
//��ڲ���:
//		��
//���ز���:
//		��ǰ���ü�����
long CTSDBHugeFileObj::Release()
{
	if( ::InterlockedDecrement( &m_nRef ) )
		return m_nRef;
	delete this;
	return 0;
}
