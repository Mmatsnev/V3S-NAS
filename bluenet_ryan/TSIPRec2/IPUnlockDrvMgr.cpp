///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2002-11-28
///
///=======================================================

// IPUnlockDrvMgr.cpp: implementation of the CIPUnlockDrvMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "iprecsvr.h"
#include "IPUnlockDrvMgr.h"
#include "IPEncryptDataStruct.h"
#include "CRC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIPUnlockDrvMgr::CIPUnlockDrvMgr()
{

}

CIPUnlockDrvMgr::~CIPUnlockDrvMgr()
{
	POSITION pos = m_DrvBuf.GetStartPosition();
	while( pos )
	{
		int nKey;
		CIPUnlockDrvWrapper * pDrv = NULL;
		m_DrvBuf.GetNextAssoc( pos, nKey, pDrv );
		ASSERT( pDrv );
		if( pDrv )
			delete pDrv;
	}
}

///-------------------------------------------------------
/// 2002-11-28
/// ���ܣ�
///		��ȡ��������
/// ��ڲ�����
///		nDrvSN				�������
/// ���ز�����
///		NULL				ʧ��
///		����				�ɹ�
CIPUnlockDrvWrapper * CIPUnlockDrvMgr::GetDrv(int nDrvSN)
{
	CIPUnlockDrvWrapper * pRetVal = NULL;
	if( m_DrvBuf.Lookup( nDrvSN, pRetVal ) )
	{
		ASSERT( pRetVal );
		return pRetVal;
	}

	if( FALSE == m_KeyMgr.m_bIsRcvIDReady )
		return NULL;						//	û������ RcvID�����ܽ��н���

	pRetVal = new CIPUnlockDrvWrapper;
	if( NULL == pRetVal )
		return NULL;
	if( FALSE == pRetVal->LoadDrv( nDrvSN ) )
	{
		delete pRetVal;
		return NULL;
	}

	if( FALSE == pRetVal->InitDrv( &m_KeyMgr ) )
	{
		delete pRetVal;
		return NULL;
	}

	CSingleLock	SynLock( &m_SynbObj, TRUE );	//	ͬ�����Է�ֹͬʱ����

	TRY
	{
		m_DrvBuf.SetAt( nDrvSN, pRetVal );
	}
	CATCH_ALL( e )
	{
		delete pRetVal;
		return NULL;
	}
	END_CATCH_ALL

	return pRetVal;
}

///-------------------------------------------------------
/// 2002-11-28
/// ���ܣ�
///		��ȡ���������
/// ��ڲ�����
///		��
/// ���ز�����
///		���������
CIPEncryptKeyMgrImpl & CIPUnlockDrvMgr::GetKeyMgr()
{
	return m_KeyMgr;
}

///-------------------------------------------------------
/// 2002-11-28
/// ���ܣ�
///		��������
/// ��ڲ�����
///		pBuf				�����ܵ����ݻ�����
///		dwBufLen			���������ȣ�
///							���룺��������С
///							��������ܺ���Ч���ݵĴ�С
/// ���ز�����
///		NULL				ʧ��
///		������				��Ч���ݵĿ�ʼ	
PBYTE CIPUnlockDrvMgr::UnlockData( PBYTE pBuf, DWORD & dwBufLen )
{	
	if( FALSE == IsDataTongShiModeLocked( pBuf, dwBufLen ) )
	{							//	û�м���
		ASSERT( FALSE );
		return pBuf;
	}

	PTS_IP_ENCRYPT_STRUCT pIPEncryptHeader = (PTS_IP_ENCRYPT_STRUCT)pBuf;
	pBuf += sizeof(TS_IP_ENCRYPT_STRUCT);
	ASSERT( dwBufLen > sizeof(TS_IP_ENCRYPT_STRUCT) );
	dwBufLen -= sizeof(TS_IP_ENCRYPT_STRUCT);
	ASSERT( dwBufLen == pIPEncryptHeader->m_wSrcDataLen );
	if( dwBufLen < pIPEncryptHeader->m_wSrcDataLen )
		return NULL;			//	���ȴ���

	int nDrvSN = pIPEncryptHeader->m_dwDrvSN;	

	CIPUnlockDrvWrapper * pDrv = GetDrv( nDrvSN );
	if( NULL == pDrv )
		return NULL;
	
	int nRetVal = pDrv->UnlockData( pBuf, dwBufLen, pIPEncryptHeader->m_dwSysCodeIndex );
	if( nRetVal <= 0 )
		return NULL;			//	ʧ����

	if( pIPEncryptHeader->m_dwSrcDataCRC32 != CCRC::GetCRC32( pIPEncryptHeader->m_wSrcDataLen, pBuf ) )
		return NULL;			//	���ݴ���

	dwBufLen = pIPEncryptHeader->m_wSrcDataLen;	
	return pBuf;
}

///-------------------------------------------------------
/// 2002-11-28
/// ���ܣ�
///		�ж������Ƿ�ͨ�ӷ�ʽ����
/// ��ڲ�����
///		pBuf				���ݻ�����
///		dwBufLen			��������С
/// ���ز�����
///		TRUE				����
///		FALSE				û�м���
BOOL CIPUnlockDrvMgr::IsDataTongShiModeLocked(PBYTE pBuf, DWORD dwBufLen)
{
	ASSERT( pBuf && dwBufLen );
	if( NULL == pBuf || dwBufLen < sizeof(TS_IP_ENCRYPT_STRUCT) )
		return FALSE;
	PTS_IP_ENCRYPT_STRUCT pIPEncryptHeader = (PTS_IP_ENCRYPT_STRUCT)pBuf;
	if( TS_IP_ENCRYPT_DATA_TAG != pIPEncryptHeader->m_dwTag )
		return FALSE;							//	��Ǵ���
	if( pIPEncryptHeader->m_dwHeadCRC32 != \
		CCRC::GetCRC32( sizeof(TS_IP_ENCRYPT_STRUCT)-offsetof(TS_IP_ENCRYPT_STRUCT,m_wVersion),\
		PBYTE(&pIPEncryptHeader->m_wVersion) ) )
	{											//	����ͷ CRC ����
		return FALSE;
	}
	if( dwBufLen < pIPEncryptHeader->m_wSrcDataLen )
		return FALSE;							//	���ݳ�����Ч
	
	return TRUE;
}
