// BaseFileCombiner.cpp: implementation of the CBaseFileCombiner class.
//
//////////////////////////////////////////////////////////////////////
//	2002.11.14  �޸� OnFileOK���� pOneFile �� IP:Port ֵ
//	2002.6.29	�޸ģ�GetIPBPS �� deDelta ����Ϊ 0
//	2002.6.16	�޸� GetIPBPS��ͳ�Ʒ��������ܻᵼ��ʱ�����

#include "stdafx.h"
#include "BaseFileCombiner.h"
#include "crc.h"
#include "IPData.h"
#include <time.h>

#ifdef _WIN32
  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif
#endif //_WIN32

class COneDataPortItem;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBaseFileCombiner::CBaseFileCombiner()
{
	m_dwByteReceived = 0;						//	�ܹ����յ����ֽ���
	m_dwLastTickCount = time(NULL);					//	�ϴμ����ʱ��
	m_dwLastBPS = 0;
}

CBaseFileCombiner::~CBaseFileCombiner()
{
	FreeActiveOne_FileObject();
}
//////////////////////////////////////////////
//����:
//		���������һҳ����
//��ڲ���:
//		pDPItem			���ݶ˿ڲ���
//		pBuf			��������ַ
//		dwLen			����
//���ز���:
//		��
void CBaseFileCombiner::DoInputOnePage(COneDataPortItem *pDPItem, PBYTE pBuf, DWORD dwLen)
{
	ASSERT( pDPItem && pBuf && dwLen );
	ASSERT( &pDPItem->m_FileCombiner == this );

	m_dwByteReceived += dwLen;					//	���ֽ�Ϊ��λ��ͳ�� IP�� ����
	if( m_dwByteReceived >= 0x7FFFFFFF )
	{											//	���� 2G �ֽڣ����п���
		m_dwLastTickCount = ( m_dwLastTickCount + time(NULL) ) / 2;
		m_dwByteReceived /= 2;
	}

	PTSDVBMULTICASTPACKET0 pHeader = (PTSDVBMULTICASTPACKET0)pBuf;

	if( pHeader->m_cbSize >= dwLen || pHeader->m_cbSize < offsetof(TSDVBMULTICASTPACKET0,m_PacketTime) )
		return;									//	���������ͷ
	if( CCRC::GetCRC32(pHeader->m_cbSize-offsetof(TSDVBMULTICASTPACKET0,m_PacketTime),\
		(PBYTE)&pHeader->m_PacketTime ) != pHeader->m_dwHeaderCRC32 )
	{
		TRACE("CBaseFileCombiner::DoInputOnePage, Header CRC32 error.\n");
		return;									//	����ͷ CRC ����
	}
#ifdef _DEBUG_
	TRACE("Receive One page %d.%02d %s %d\n",pHeader->m_chFile.m_nMag, pHeader->m_chFile.m_nPageNo,\
		pHeader->m_cFileName, pHeader->GetPageOfsInFile()/pHeader->GetPageLen() );
#endif // _DEBUG

	CMB_OneFile * pOneFile=NULL;
	if( FALSE == m_OneFileMgr.Lookup( pHeader->m_chFile.m_dwData, pOneFile ) )
	{											//	û���ҵ�
		pOneFile = AllocOneFile( pHeader );
		if( NULL == pOneFile )					//	�����ڴ�ʧ��
			return;
	}
	else
	{
		if( pOneFile->IsFileChanged(pHeader->m_PacketTime,pHeader->m_dwFileLen) )
		{
			pOneFile->CollectDataUseXorChksum();	//	2001.10.12 ��ͼ��������
			if( pOneFile->m_dwByteRead )
			{
				TRACE("File changed.\n");
				pOneFile->m_Time = RestoreBroTime( pOneFile->m_Time );
				OnFileOK( pDPItem, pOneFile );				//	pOneFile ������Ҫ���� Release ����
			}
			else
			{
				TRACE("File changed and none page received.\n");
				DeAllocate( pOneFile, FALSE );		//	�ͷ��ļ�����
				pOneFile->Release();
			}
			pOneFile = AllocOneFile( pHeader );
			if( NULL == pOneFile )
			{
				m_OneFileMgr.RemoveKey( pHeader->m_chFile.m_dwData );
				return;								//	����ʧ��
			}
		}
	}

	int nRetVal = pOneFile->AddOnePage( pBuf, dwLen );
	ASSERT( nRetVal != CMB_OneFile::MBROF_FILE_CHANGED );
	if( nRetVal == CMB_OneFile::MBROF_FILE_OK )
	{
		ASSERT( pOneFile->m_dwByteRead );
		pOneFile->m_Time = RestoreBroTime( pOneFile->m_Time );
		OnOneSubFileOK( pDPItem, pBuf, dwLen );					//  2003-4-11 ��¼���ļ�����OK
		OnFileOK( pDPItem, pOneFile );							//	pOneFile ������Ҫ���� Release ����
		m_OneFileMgr.RemoveKey( pHeader->m_chFile.m_dwData );	//	ɾ��
	}
}

//////////////////////////////////////////////
//����:
//		һ�������ļ����ճɹ�
//��ڲ���:
//		pDPItem			����Դ����
//		pOneFile		�����ļ�
//���ز���:
//		��
//�޸ļ�¼��
//		�� pOneFile �� IP:Port ֵ
void CBaseFileCombiner::OnFileOK(COneDataPortItem *pDPItem, CMB_OneFile *pOneFile)
{
	ASSERT( pOneFile );
	ASSERT( pDPItem && pDPItem->m_pFileObjMgr);

#ifdef _DEBUG_
	TRACE("One base file %08X is received, FileLen=%d, Byte received=%d, PacketTime=%d, Ch=%X\n",
		pOneFile, pOneFile->GetDataLen(), pOneFile->m_dwByteRead,
		pOneFile->m_Time, pOneFile->m_chFile.m_dwData );
#endif // _DEBUG_

	ASSERT( pDPItem->m_nPort );			//	2002.11.14 ���
	pOneFile->SetMulticastParameter( pDPItem->m_strTargetIP, (WORD)pDPItem->m_nPort, pDPItem );

	pDPItem->m_pFileObjMgr->ProcessOneFile( pOneFile );

	DeAllocate( pOneFile, FALSE );
	pOneFile->Release();
}

//	���䲢��ʼ��һ������
//	��ڲ���
//		pHeader				����ͷ
//	���ز���
//		NULL				ʧ��
CMB_OneFile * CBaseFileCombiner::AllocOneFile(PTSDVBMULTICASTPACKET0 pHeader)
{
	CMB_OneFile * pOneFile = AllocatePacket(0,FALSE);
	if( NULL == pOneFile )
		return NULL;

	if( FALSE == pOneFile->Initialize( pHeader->m_chFile, pHeader->m_cFileName, \
		pHeader->m_dwFileLen, pHeader->m_PacketTime ) )
	{										//	��ʼ��ʧ��
		DeAllocate( pOneFile, FALSE );
		pOneFile->Release();
		return NULL;
	}
	m_OneFileMgr.SetAt( pHeader->m_chFile.m_dwData, pOneFile );
	return pOneFile;
}

//	��ԭʱ��
//	��ڲ���
//		BroTime						������Ĳ���ʱ��
//	���ز���
//		UTC	ʱ��
time_t CBaseFileCombiner::RestoreBroTime(time_t BroTime)
{
	if( BroTime & (1L<<31) )
	{
		BroTime >>= 8;
		BroTime &= 0x7FFFFF;
		BroTime += Y2KSECOND;
	}
	else
	{
		if( BroTime < 24*3600*10L )
		{							//	�ϵĴ��ʱ��,��һ���� 2000.1.1  00:00:00
			BroTime /= 10;
			BroTime += Y2KSECOND;
		}
	}
	return BroTime;
}

//////////////////////////////////////////////
//����:
//		�ͷ����ڽ��յ��ļ�����
//��ڲ���:
//		��
//���ز���:
//		��
void CBaseFileCombiner::FreeActiveOne_FileObject()
{
	POSITION pos = m_OneFileMgr.GetStartPosition();
#ifdef _DEBUG
	if( pos )
		TRACE("CBaseFileCombiner::FreeActiveOne_FileObject,  Free Active OneFile Object.\n");
#endif // _DEBUG
	while( pos )
	{
		DWORD dwKey;
		CMB_OneFile * pOneFile;
		m_OneFileMgr.GetNextAssoc( pos, dwKey, pOneFile );
		ASSERT( pOneFile );
		pOneFile->Release();
	}
}

//////////////////////////////////////////////
//����:
//		��ȡ IP ��·�������
//��ڲ���:
//		��
//���ز���:
//		��·������
DWORD CBaseFileCombiner::GetIPBPS()
{
	DWORD dwNow = time(NULL);
	DWORD dwDelta = dwNow - m_dwLastTickCount;
	if( dwDelta >= 10 || 0 == m_dwLastBPS )			//	���̫С��ʹ����һ�ε����ʣ�ÿ10��ͳ��һ��
	{													//	�Ժ�ÿ5��ͳ��һ��
		if( 0 == dwDelta )								//	2002.6.29����ӣ�dwDelta ����Ϊ 0
			return 0;
		m_dwLastBPS = ( m_dwByteReceived * 8) / dwDelta;		//	125*64 = 8 * 1000
		m_dwLastTickCount = m_dwLastTickCount + dwDelta / 2;
		m_dwByteReceived /= 2;
	}
	return m_dwLastBPS;
}

///-------------------------------------------------------
/// 2003-4-11
/// ���ܣ�
///		һ�����ļ����ճɹ�
/// ��ڲ�����
///		pDPItem				���ݶ˿ڶ���
///		pBuf				������
///		dwLen				����������
/// ���ز�����
///		��
void CBaseFileCombiner::OnOneSubFileOK(COneDataPortItem *pDPItem, PBYTE pBuf, DWORD dwLen)
{
	ASSERT( pDPItem && pBuf && dwLen );
	if( NULL == pDPItem->m_pFileMendHelper )
		return;

	PTSDVBMULTICASTPACKET0 pPacket = (PTSDVBMULTICASTPACKET0)pBuf;
	if( pPacket->m_ExtParam.m_byReserved_0 )
		return;				//	������չ���ʽ
	BYTE byXorValue = 0;
	for(int i=2; i<13; i++)
	{
		byXorValue ^= (BYTE)pPacket->m_cFileName[i];
	}
	if( byXorValue != pPacket->m_ExtParam.m_byXorChkSum )
		return;				//	����У�����
	if( EPFI_SUBFILE_ID != pPacket->m_ExtParam.m_byFunctionIndex )
		return;
	int nSubFileID = pPacket->m_ExtParam.m_SubFileID.m_dwSubFileID;
	int nTotalFileCount = pPacket->m_ExtParam.m_SubFileID.m_dwTotalSubFileCount;

	if( nTotalFileCount )
		pDPItem->m_pFileMendHelper->SetTotalSubFileCount( nTotalFileCount );

	long nTotalBit1Count = pDPItem->m_pFileMendHelper->SetBitValue( nSubFileID, 1 );

#ifdef _DEBUG
	CString strTmp;
	strTmp.Format("SubFile %d/%d is OK.\n", nSubFileID, nTotalFileCount );
#endif // _DEBUG

}
