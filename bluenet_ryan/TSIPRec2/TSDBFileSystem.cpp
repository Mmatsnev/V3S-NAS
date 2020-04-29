///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2002-11-14
///
///=======================================================

// TSDBFileSystem.cpp: implementation of the CTSDBFileSystem class.
//
//////////////////////////////////////////////////////////////////////
//
// 2002.11.14	�޸� ProcessSingleFile �� ProcessHugeFile����ֵ�ಥ IP : Port
// 2002.5.22	��ӣ��Ƿ������ TSDB �ļ�ͨ��
// 2001.9.21	�޸� ProcessHugeFile������������ļ�
// 2001.8.17	�޸� ProcessSingleFile �������ͷ��ڴ�
// 2001.8.6		��Ӻ��� ProcessOneFileInMem�����������ڴ��е��ļ�
// 2001.3.16	��ӳ�Ա����m_dwByteRecevied��������ȡʵ�ʽ��յ�����
// 2000.10.28	�� CTSDBDrv �з������
//		

#include "stdafx.h"
#include "Resource.h"
#include "TSDBFileSystem.h"
#include "TSDB_Rec.h"
#include "UnCmpMgr.h"
#include "FileUpdate.h"
#include "FilePurpose.h"

#ifdef _WIN32
  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif
#endif //_WIN32


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTSDBFileSystem::CTSDBFileSystem()
{
	m_dwByteRecevied = 0;
	m_pOneBaseFile = NULL;
	m_bIsEnableNotTSDBFile = FALSE;
}

CTSDBFileSystem::~CTSDBFileSystem()
{
}

//	�����ڴ��е��ļ�
void CTSDBFileSystem::ProcessOneFile( CMB_OneFile * pOneFile )
{
	ASSERT( pOneFile );
	if( FALSE == m_bIsEnableNotTSDBFile && pOneFile->m_dwByteRead < pOneFile->m_dwFileLen )
		return;										//	2002.5.22 ��ӣ�TSDB ��װ���ļ���һ����Ҫ��ȷ������

	m_pOneBaseFile = pOneFile;
	PBYTE pBuf = pOneFile->GetBuffer();
	if( CTSDBMultiFileHeader::IsMultiFileHeader( pBuf ) )
	{
		if( pOneFile->m_dwFileLen == pOneFile->m_dwByteRead )
			ProcessMultiFile( (PTSDBMULFILEHEAD)pBuf, pOneFile );			//	�Ƕ���ļ�
	}
	else
		ProcessSingleFile( pBuf, pOneFile->GetDataLen(), pOneFile );	//	�������ļ�, ������ͨ�ļ�, Ҳ������ TSDB �ĵ����ļ��ļ�
	m_pOneBaseFile = NULL;
}

//	�������ļ�
void CTSDBFileSystem::ProcessMultiFile(PTSDBMULFILEHEAD pMultiHeader, CMB_OneFile * pOneFile)
{
	ASSERT( CTSDBMultiFileHeader::IsMultiFileHeader( pMultiHeader ) );
	CTSDBMultiFileHeader	hdr( pMultiHeader );
	int nNum = hdr.GetFileNum();
	for(int i=0; i<nNum; i++)
	{
		ProcessSingleFile( (PBYTE) &hdr[i], hdr[i].m_dwFileLen, pOneFile );
	}
}

//	�������ļ�
//	�޸ļ�¼��
//		2002.11.14 ��ֵ�ಥ IP : Port
void CTSDBFileSystem::ProcessSingleFile(PBYTE pBuffer,DWORD dwLen, CMB_OneFile * pBaseFile)
{
PBYTE	pUncmpAutoAllocBuf = NULL;
//	if( CTSDBUnlock::IsLock( pBuffer ) ) 
//	{
//	�ڴ˵��ý���
//		UnlockData();		
//	}
	ASSERT( m_pOneBaseFile );
	CFileObject * pOneFile = AllocatePacket();
	if( NULL == pOneFile )									//	�����ļ�����ʧ��
		return;

	ASSERT( pBaseFile->m_wMC_Port && FALSE == pBaseFile->m_strMC_DstIP.IsEmpty() );
	pOneFile->SetMulticastParameter( pBaseFile->m_strMC_DstIP, \
		pBaseFile->m_wMC_Port, pBaseFile->m_pDataPortItem );	// 2002.11.14 ���
	
	BOOL bIsCompleteReceived = ( pBaseFile->m_dwByteRead == pBaseFile->m_dwFileLen );
#ifdef _DEBUG
	if( FALSE == bIsCompleteReceived )
		TRACE("One file %08X is changed.\n", pBaseFile );
#endif // _DEBUG

	int	nSrcFileLen = dwLen;

	if( CUnCmpMgr::IsCompress( nSrcFileLen, pBuffer ) )
	{														//	�ж��Ƿ� TSDB ��ѹ���ļ�
		if( FALSE == bIsCompleteReceived || m_UnCompressSvr.Attach( nSrcFileLen,pBuffer ) == 0 )
		{													//	��Ȼѹ����û����������һ����ʧ��
			DeAllocate( pOneFile );
			pOneFile->Release();
			return;											//	��ѹʧ��
		}
		ASSERT( m_UnCompressSvr.GetFileNum() == 1 );
		CFileStatus outfStatus;
		if( 0 == m_UnCompressSvr.GetFileInfo( 0, outfStatus ) ||\
			FALSE == pOneFile->SetBufSize( outfStatus.m_size ) )
		{													//	ʧ��
			DeAllocate( pOneFile );
			pOneFile->Release();
			return;
		}
		pBuffer = m_UnCompressSvr.DecodeOneFile( 0,outfStatus, pOneFile->GetBuffer() );
		if( pBuffer == NULL )
		{
			DeAllocate( pOneFile );
			pOneFile->Release();
			return;											//	��ѹʧ��
		}
		pOneFile->PutDataLen( outfStatus.m_size );
		nSrcFileLen = outfStatus.m_size;
	}
	else
	{
		if( FALSE == pOneFile->SetBufSize( nSrcFileLen ) )
		{
			DeAllocate( pOneFile );
			pOneFile->Release();
			return;
		}
		memcpy( pOneFile->GetBuffer(), pBuffer, nSrcFileLen );
		pOneFile->PutDataLen( nSrcFileLen );
	}

	if( CTSDBFileHeader::IsFileHead( pBuffer, pOneFile->GetDataLen() ) )
	{
		if( FALSE == bIsCompleteReceived || FALSE == ProcessTSDBSingleFile( pOneFile ) )		//	�������ļ�
		{						//	û���������գ�һ��ʧ�ܣ�Ҳ������������ԭ�򣬷���������ļ�����Ҫ�ͷ�
			DeAllocate( pOneFile );
			pOneFile->Release();
		}
	}
	else
	{														//	��ͨ�ļ�
		if( m_bIsEnableNotTSDBFile )
		{
			pOneFile->m_strFileName = pBaseFile->m_szFileName;
			NotifyOnFileOKEvent( pOneFile );		
		}
	}
}

//////////////////////////////////////////////
//����:
//		���� TSDB �ĵ����ļ�
//��ڲ���:
//		pOneFile		�ļ����󣬻�������Ϊ�ļ�����
//���ز���:
//		TRUE			���ļ������ѱ��ύ�������� DeAllocate �ͷ�
//		FALSE			����ĳ��ԭ��û�б�������������Ҫ DeAllocate ���ͷŶ���
BOOL CTSDBFileSystem::ProcessTSDBSingleFile( CFileObject * pOneFile )
{
	ASSERT( pOneFile );
	PTSDBFILEHEADER pFileHead = (PTSDBFILEHEADER)pOneFile->GetBuffer();
	CTSDBFileHeader	hdr( pFileHead );
	if( hdr.IsHugeFile() )
		return ProcessHugeFile( pOneFile, hdr );								//	���ļ�

	pOneFile->DoTSDBSingleFile();				//	��ȡ����

	if( hdr.HasFileAttarib() && IsSysReservFile(hdr,pOneFile->m_pAttributeData->m_dwPurpose) )
	{														//	�����ļ�����
		PTSDBFILEATTRIBHEAD		pAttrib = hdr.GetFileAttribHead();
		CFileStatus fsta;
		int nFilePurpose = pAttrib->m_dwPurpose;
		fsta.m_attribute = (BYTE)pAttrib->m_dwAttribute;
		fsta.m_mtime = CTime( pAttrib->m_LastWriteTime );
		strcpy(fsta.m_szFullName, hdr.GetFileName() );
		PBYTE pDataBuf = hdr.GetDataBuf();						//	��ȡ�����ļ�
		int nFileLen = hdr.GetFileLen();
		if( ProcessSysReservFile( hdr, pDataBuf, fsta, nFilePurpose ) )
			return FALSE;										//	����֪ͨӦ�ó���, �ڲ��Ѿ�����
	}
	NotifyOnFileOKEvent( pOneFile );
	return TRUE;
}

//	������������
//	��ڲ���
//		hdr						�ļ�ͷ
//		pBuf					���ݻ�����
//		fsta					�ļ�״̬�ṹ
//		nFilePurpose			��;
//	���ز���
//		TRUE					ϵͳ�������ļ�,������֪ͨӦ�ó���
//		FALSE					�����ļ�
BOOL CTSDBFileSystem::ProcessUpdateFile(CTSDBFileHeader &hdr, PBYTE pBuf, CFileStatus &fsta, DWORD nFilePurpose)
{
BOOL bRetVal = FALSE;
	int nExtLen = hdr.ExtDataLen();
	PBYTE pExtDataBuf = new BYTE[ (nExtLen+4095)&(~4095) ];
	if( pExtDataBuf )								//	�������
	{
		hdr.CopyExtData( pExtDataBuf, nExtLen );
		switch( nFilePurpose )
		{
		case TSDB_FP_UPDATE_SYSFILE:				//	ϵͳ�ļ�������
        	ASSERT( FALSE );
//			CFileUpdate::Update( fsta.m_size, pBuf, pExtDataBuf , nExtLen );
			bRetVal = TRUE;
			break;

		case TSDB_FP_UPDATE_UPDATENOW:				//	���������ĳ���, �����ܳ���
			{
/*				CString strFileName = CFileUpdate::UpdateUnlockProc( fsta.m_size, pBuf, pExtDataBuf , nExtLen );
				if( strFileName.IsEmpty() == FALSE )
					gUnlockSvr.GetProcMgr().AddObj( strFileName );				
*/
				bRetVal = TRUE;
			}
			break;

		case TSDB_FP_UPDATE_APP:					//	OEM ��Ӧ�ó��������
			{
				PSOFTUPDATE pItem = (PSOFTUPDATE) pExtDataBuf;
				CString strTmp;
				strTmp.Format("%s\\%s /%s",pItem->szPath, pItem->szFileName, pItem->szOemName );
				strncpy( fsta.m_szFullName, strTmp, _MAX_PATH );
			}
			break;
		}
		delete pExtDataBuf;
	}
	return bRetVal;
}

//	����ϵͳ�������ļ�
//	��ڲ���
//		hdr						�ļ�ͷ
//		pBuf					���ݻ�����
//		fsta					�ļ�״̬�ṹ
//		nFilePurpose			��;
//	���ز���
//		TRUE					ϵͳ�ڲ������ļ�,������֪ͨӦ�ó���
//		FALSE					�����ļ�
BOOL CTSDBFileSystem::ProcessSysReservFile(CTSDBFileHeader &hdr, PBYTE pBuf, CFileStatus &fsta, DWORD nFilePurpose)
{
	ASSERT( IsSysReservFile( hdr, nFilePurpose ) );

	if( nFilePurpose >= 0x80000000 || nFilePurpose  == 0 )		//	�û��Զ�������
		return FALSE;
	switch( nFilePurpose )
	{
	case TSDB_FP_UPDATE_SYSFILE:
	case TSDB_FP_UPDATE_UPDATENOW:
	case TSDB_FP_UPDATE_APP:
		if( hdr.HasExtData() )
			ProcessUpdateFile( hdr, pBuf, fsta, nFilePurpose );
		else
			return FALSE;						//	���������
		break;

	case TSDB_FP_LICENCE_MSG:					//	��Ȩ�ļ�
//		gUnlockSvr.ProcessLicData( pBuf, fsta.m_size );
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////////
//����:
//		�ж��Ƿ�Ϊϵͳ�������ļ�
//��ڲ���:
//		hdr				�ļ�ͷ
//		nFilePurpose	�ļ�����
//���ز���:
//		TRUE			ϵͳ�������ļ�����Ҫ����
//		FALSE			Ӧ�ó���
BOOL CTSDBFileSystem::IsSysReservFile(CTSDBFileHeader &hdr, DWORD nFilePurpose)
{
	if( nFilePurpose >= 0x80000000 || nFilePurpose  == 0 )		//	�û��Զ�������
		return FALSE;
	switch( nFilePurpose )
	{
	case TSDB_FP_UPDATE_SYSFILE:
	case TSDB_FP_UPDATE_UPDATENOW:
	case TSDB_FP_UPDATE_APP:
		if( hdr.HasExtData() )
			return TRUE;
		else
			return FALSE;						//	���������

	case TSDB_FP_LICENCE_MSG:					//	��Ȩ�ļ�
		return TRUE;

	default:
		return FALSE;
	}
	return FALSE;
}

//	������ļ�
//	��ڲ���
//		FileHead						TSDB �ļ�
//	�޸ļ�¼��
//		2002.11.14 ��ֵ�ಥ IP : Port
BOOL CTSDBFileSystem::ProcessHugeFile( CFileObject * pOneFile,CTSDBFileHeader &FileHead )
{
	PTSDBHUGEFILEHEAD pHugeHead = FileHead.GetHugeFileHead();
	ASSERT( pHugeHead );
	BOOL bIsFileOK = FALSE;	

	BOOL bIsSysReservedFile = FALSE;
	if( FileHead.HasFileAttarib() && IsSysReservFile(FileHead, FileHead.GetFileAttribHead()->m_dwPurpose) )
		bIsSysReservedFile = TRUE;

	BOOL bOneFileIsSummit = FALSE;					//	��ʶ���ļ������Ƿ��ύ����û���ύ�����������Ҫ DeAllocate
	if( FALSE == bIsSysReservedFile )
	{
		pOneFile->DoTSDBSingleFile();				//	��ȡ����
		ASSERT( pOneFile->GetDataLen() == pHugeHead->m_wBlockSize );
		NotifySubFileOKEvent( pOneFile );			//	ֻ�з�ϵͳ�����ļ�����֪ͨ���ļ����յ����ļ�
		bOneFileIsSummit = TRUE;
	}

	return bOneFileIsSummit;
}

//	��������
//	��ڲ���
//		pBuffer				�ļ�������
//		outLen				�������
//		dwSysCodeIndex		���ϵͳ��������
//	���ز���
//		NULL				ʧ��
//		����				������ָ��
PBYTE CTSDBFileSystem::UnlockData()
{	
	ASSERT( FALSE );
	return NULL;
}


