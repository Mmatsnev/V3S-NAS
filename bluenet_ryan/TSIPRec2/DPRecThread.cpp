///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///
///=======================================================

// DPRecThread.cpp: implementation of the CDPRecThread class.
//
//////////////////////////////////////////////////////////////////////
// 2003-3-5		�޸�DoReadSyncDataPort()������ͬ���˿ڷ�ʽ��ÿ�ζ�ȡ 100 �Σ���û������Ϊֹ
//	2002.12.30	��� DoReadSyncDataPort��֧�����ݶ˿�ģʽ
//	2002.11.28	��� g_UnlockDrvMgr���Լ����� GetIPEncryptKeyMgrObj
//				�޸� OnDataOK�����Ӷ����ݽ��ܵ�֧��
//	2002.11.14  �޸� OnDataOK����� AddPacket ʧ�ܵ�����µĴ���
//	2002.6.17	�޸� Run �������Ż��㷨����Ҫ���ͷ�ͬ������Ŀ���Ȩ��

#include "stdafx.h"
#include "resource.h"
#include "DPRecThread.h"
#include "IPUnlockDrvMgr.h"
#include "IPEncryptDataStruct.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//--------------------------------------------
//	��������������
CIPUnlockDrvMgr	g_UnlockDrvMgr;

///-------------------------------------------------------
/// 2002-11-28
/// ���ܣ�
///		��ȡ������������
/// ��ڲ�����
///		��
/// ���ز�����
///		������������
extern "C" CIPEncryptKeyMgr2 * WINAPI GetIPEncryptKeyMgrObj()
{
	CIPEncryptKeyMgrImpl & KeyMgr = g_UnlockDrvMgr.GetKeyMgr();
	return static_cast<CIPEncryptKeyMgr2*>( &KeyMgr );
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDPRecThread::CDPRecThread()
{
	m_pDecoderThread = NULL;
}

CDPRecThread::~CDPRecThread()
{

}

int CDPRecThread::ExitInstance()
{
	CleanUpIPData();
	return 0;
}

BOOL CDPRecThread::InitInstance()
{
	return TRUE;
}

//---------------------------------------
//	�޸ļ�¼��
//	2002.12.30	��� DoReadSyncDataPort
void CDPRecThread::Run()
{
	ASSERT( m_pDecoderThread && m_pDecoderThread->m_hThread );

	m_bIsRequestQuit = FALSE;
	SetThreadPriority( m_hThread, THREAD_PRIORITY_TIME_CRITICAL );	//	�ı䵽��󼶱�

	HANDLE anEventObjs[ MAX_DATA_PORT_COUNT ];						//	�����ܳ������¼�����
	COneDataPortItem * anDataPort[ MAX_DATA_PORT_COUNT ];			//	ʵ�ʲ���ȴ��Ķ���
	while( FALSE == m_bIsRequestQuit )
	{	
		DoReadAsync();												//	��ȡ����
		DoReadSyncDataPort();										//	��ȡͬ����ʽ�����ݶ˿�7

		CSingleLock symobj( &m_SynObj, TRUE );				

		int nCount = m_anPorts.GetSize();		
		int nActualCount = 0;
		for(int i=0; i<nCount; i++)
		{															//	��ȡ��������Ķ˿�ͬ������
			ASSERT( m_anPorts[i]->m_pDataPort );
			SDP_HANDLE hAsync = m_anPorts[i]->GetHeadHandle();
			if( hAsync < 0 )
				continue;
			anEventObjs[nActualCount] = m_anPorts[i]->m_pDataPort->GetEventHandle( hAsync );
			anDataPort[nActualCount] = m_anPorts[i];
			nActualCount ++;
		}	
		if( 0 == nActualCount )				//	û���ӳٲ���
		{
			symobj.Unlock();				//	2002.6.17 ��ӣ����ͷţ�Ȼ��������
			Sleep( 20 );					//	û�����ݶ˿ڣ���Ϣ 20 ms
			continue;
		}

		DWORD dwRetVal = ::WaitForMultipleObjects( nActualCount,anEventObjs,FALSE, 10 );	//	ÿ 10 ������һ��
		if( WAIT_TIMEOUT == dwRetVal )
		{															// 2002.6.17 ����޸ģ�ԭ���� WaitForMultipleObjects �޸� 20 ms���ָĳ� 10ms��Ȼ���������10ms
			symobj.Unlock();										//	����֮ǰ�����ͷŶ���Ȼ��������
			Sleep( 5 );												//	��ʱû�����ݣ����� 5 ms���������̹߳���	
			continue;												//	��ʱ
		}
		
		if( dwRetVal >= WAIT_ABANDONED_0 && dwRetVal < WAIT_ABANDONED_0 + nActualCount )
		{															//	�ж��󱻷���/����, ????
			TRACE("One event is abandoned.\n");
			int nNo = dwRetVal - WAIT_ABANDONED_0;
			ASSERT( FALSE == anDataPort[nNo]->m_AsyncHandle.IsEmpty() );
			ONEASYNCREAD oneread = anDataPort[nNo]->m_AsyncHandle.RemoveHead();

			anDataPort[nNo]->m_pDataPort->CancelAsynRead( oneread.m_hSDP );	//	�Ƿ���Ҫ���������� ???
				
			symobj.Unlock();

			m_pDecoderThread->DeAllocate( oneread.m_pIPData );		//	�ͷ��ڴ�
			oneread.m_pIPData->Release();
			continue;
		}
		int nNo = dwRetVal - WAIT_OBJECT_0;							//	�ɹ�
		ASSERT( nNo >= 0 && nNo < nActualCount );

		COneDataPortItem * pPort = anDataPort[nNo];		
		ONEASYNCREAD & oneread = anDataPort[nNo]->m_AsyncHandle.GetHead();
		CIPData * pIPData = oneread.m_pIPData;
		DWORD dwByteRead = 0;										//	ʵ�ʶ�ȡ���ֽ���
		if( FALSE == pPort->m_pDataPort->GetOverlappedResult( oneread.m_hSDP, &dwByteRead, FALSE ) )
		{											//	ʧ�ܣ������ǻ�û����ɣ�����˵��̫���ܣ���Ϊ���Ѿ����ȴ�
			if( WSA_IO_INCOMPLETE == pPort->m_pDataPort->m_nLastError )
			{
				TRACE("CDPRecThread, This condition can not be happen.\n");
				ASSERT( FALSE );
				continue;
			}

			pPort->m_pDataPort->CancelAsynRead( oneread.m_hSDP );	//	�Ƿ���Ҫ���������� ???
			anDataPort[nNo]->m_AsyncHandle.RemoveHead();

			symobj.Unlock();			

			m_pDecoderThread->DeAllocate( pIPData );	//	�ͷ��ڴ�
			pIPData->Release();
			continue;
		}

//		TRACE("One Packet is received.\n");

		anDataPort[nNo]->m_AsyncHandle.RemoveHead();
		symobj.Unlock();

		OnDataOK( pPort, pIPData, dwByteRead );
		pIPData->Release();
	}
}

//////////////////////////////////////////////
//����:
//		ִ�ж�ȡ����
//��ڲ���:
//		Ԥ����д���е��첽������
//���ز���:
//		��
void CDPRecThread::DoReadAsync()
{
	CSingleLock symobj( &m_SynObj, TRUE );

	int nCount = m_anPorts.GetSize();
	for(int i=0; i<nCount; i++)
	{
		COneDataPortItem * pPort = m_anPorts[i];
		while( pPort->m_pDataPort->CanIDoReadAsync() )
		{								//	�ж��Ƿ����첽�����ļ�¼�ռ�
			CIPData * pIPData = m_pDecoderThread->AllocatePacket( 0 );
			if( NULL == pIPData )
				return;					//	û���ˣ�ֻ�÷���
			DWORD dwDataRead = 0;
			if( FALSE == pIPData->SetBufSize( pPort->m_wPacketBufSize ) )
			{
				symobj.Unlock();		//	�����ڴ�ʧ�ܣ�����
				m_pDecoderThread->DeAllocate( pIPData );
				pIPData->Release();
				return;
			}
			SDP_HANDLE hRead = pPort->m_pDataPort->ReadAsyn( pIPData->GetBuffer(), pPort->m_wPacketBufSize, &dwDataRead );
//			TRACE("........ To receive One packet. return value=%d\n",hRead);

			if( hRead > 0 )
			{						//	�ӳٲ���
				ONEASYNCREAD oneread;
				oneread.m_hSDP = hRead;
				oneread.m_pIPData = pIPData;
				pPort->m_AsyncHandle.AddTail( oneread );
				continue;
			}
			symobj.Unlock();
			if( 0 == hRead )			//	�ɹ���ȡ
				OnDataOK( pPort, pIPData, dwDataRead );
			else							//	ʧ��
				m_pDecoderThread->DeAllocate( pIPData );
			pIPData->Release();
			symobj.Lock();

			nCount = m_anPorts.GetSize();	//	���ܱ�ɾ��
			if( NULL== pPort->m_pDataPort )
				break;
		}
	}
}

//////////////////////////////////////////////
//����:
//		���յ�����
//��ڲ���:
//		pDataPortItem			���ݶ˿�
//		pIPData					IP ���ݰ�
//		dwByteCount				�ɹ���ȡ���ֽ���
//���ز���:
//		��
//�޸ļ�¼��
//	2002.11.28	��������
//	2002.11.14	��� AddPacket ʧ�ܵ�����µĴ���
void CDPRecThread::OnDataOK(COneDataPortItem *pDataPortItem, CIPData *pIPData, DWORD dwByteCount)
{
	ASSERT( pDataPortItem && dwByteCount );
	pIPData->m_pDataPortItem = pDataPortItem;
	pIPData->PutDataLen( dwByteCount );

//--------------- Begin 2002.11.28 Added for Unlock TongShi mode Data Encrypt -----------------
	if( g_UnlockDrvMgr.IsDataTongShiModeLocked( pIPData->GetBuffer(), dwByteCount ) )
	{							//	���ݲ���ͨ��ר�÷�ʽ���ܣ���Ҫ���н���
		PBYTE pRetVal = g_UnlockDrvMgr.UnlockData( pIPData->GetBuffer(), dwByteCount );
		if( NULL == pRetVal )
		{						//	����ʧ��
			TRACE0("Unlock Data Failed.\n");
			m_pDecoderThread->DeAllocate( pIPData );			//	2002.11.14 ���
			return;
		}
#ifdef _DEBUG
		ASSERT( int(pRetVal-pIPData->GetBuffer()) == int(pIPData->GetDataLen() - dwByteCount) );
		ASSERT( (pIPData->GetDataLen() - dwByteCount) == sizeof(TS_IP_ENCRYPT_STRUCT) );
#endif // _DEBUG

		pIPData->DeleteHeadData( sizeof(TS_IP_ENCRYPT_STRUCT) );

		ASSERT( pIPData->GetDataLen() == dwByteCount );
	}
//------------ End 2002.11.28 -----------------------------------------------

	if( FALSE == m_pDecoderThread->AddPacket( pIPData ) )	//	�ύ����
		m_pDecoderThread->DeAllocate( pIPData );			//	2002.11.14 ���
}

//////////////////////////////////////////////
///����:
///		��ȫ��ɾ���Լ�
///��ڲ���:
///		��
///���ز���:
///		��
void CDPRecThread::Delete()
{
	if( m_bAutoDelete )
		delete this;
}

//////////////////////////////////////////////
///����:
///		���������߳�
///��ڲ���:
///		pDecoderThread			�����߳�
///���ز���:
///		TRUE					�ɹ�
///		FALSE					ʧ��
///ע��
///		pDecoderThread			�����̱߳����ȴ���
BOOL CDPRecThread::Init(CDecoderThread *pDecoderThread)
{
	ASSERT( pDecoderThread && pDecoderThread->m_hThread );
	m_pDecoderThread = pDecoderThread;
	return CreateThread();
}

//////////////////////////////////////////////
///����:
///		�ر��߳�
///��ڲ���:
///		��
///���ز���:
///		��
void CDPRecThread::Close()
{
	if( NULL != m_hThread )
		StopThread(-1);						//	ֹͣ����
	else if( m_bAutoDelete )
		delete this;						//	��û�д����̣߳�����ֻ���Լ�ɾ������
}

//////////////////////////////////////////////
///����:
///		������ݶ˿ڶ���
///��ڲ���:
///		pDataPortItem			����ӵض˿�
///���ز���:
///		>0						�ɹ���ӣ��ҷ��ص�ǰ���еض˿���Ŀ
///		<0						ʧ��
int CDPRecThread::AddDataPort(COneDataPortItem *pDataPortItem)
{
	ASSERT( pDataPortItem && pDataPortItem->m_pDataPort && pDataPortItem->m_pFileObjMgr );
	if( NULL == pDataPortItem || NULL == pDataPortItem->m_pDataPort || NULL == pDataPortItem->m_pFileObjMgr )
		return -1;

	CSingleLock symobj( &m_SynObj, TRUE );

	int nNo = FindDataPortItem( pDataPortItem );
	if( nNo >= 0 )
	{								//	�Ѿ�����
		TRACE("CDPRecThread, this data port has been add already.\n");
		ASSERT( FALSE );
		return -1;					//	�����ٴ����
	}
	
	if( m_anPorts.GetSize() >= MAX_DATA_PORT_COUNT )
	{
		TRACE("Too many data ports.\n");
		return -1;
	}

	m_anPorts.Add( pDataPortItem );

	return m_anPorts.GetSize();
}

//////////////////////////////////////////////
///����:
///		ɾ��һ�����ݶ˿�
///��ڲ���:
///		pDataPortItem			��ɾ���Ķ˿ڣ������ݱ�������ǰ������ӹ��Ķ˿�
///���ز���:
///		��
void CDPRecThread::DeleteDataPort(COneDataPortItem *pDataPortItem)
{
	ASSERT( pDataPortItem );
	CSingleLock symobj( &m_SynObj, TRUE );

	int nNo = FindDataPortItem( pDataPortItem );
	if( nNo < 0 )
	{
		TRACE("CDPRecThread, this data port is not add ever.\n");
		return;
	}
	TRACE("CDPRecThread::DeleteDataPort, delete one data port=%08X\n",pDataPortItem );
	m_anPorts.RemoveAt( nNo );	

	symobj.Unlock();
	
	ASSERT( m_pDecoderThread );
	while( FALSE == pDataPortItem->m_AsyncHandle.IsEmpty() )
	{
		ONEASYNCREAD oneread = pDataPortItem->m_AsyncHandle.RemoveHead();
		m_pDecoderThread->DeAllocate( oneread.m_pIPData );	//	�ͷ��ڴ�
		oneread.m_pIPData->Release();
	}
}

//////////////////////////////////////////////
///����:
///		���ݸ�����ָ����Ҷ���
///��ڲ���:
///		pDataPortItem			�����ҵ����ݶ˿�
///���ز���:
///		<0						û���ҵ�
///		>=0						���
int CDPRecThread::FindDataPortItem(COneDataPortItem *pDataPortItem)
{
	int nCount = m_anPorts.GetSize();
	for(int i=0; i<nCount; i++)
	{
		COneDataPortItem * pItem = m_anPorts[i];
		ASSERT( pItem );
		if( pItem == pDataPortItem )
			return i;
	}
	return -1;
}

//////////////////////////////////////////////
//����:
//		�ͷ��ڲ��������� IPDATA ����
//��ڲ���:
//		��
//���ز���:
//		��
void CDPRecThread::CleanUpIPData()
{
	CSingleLock symobj( &m_SynObj, TRUE );	
	int nCount = m_anPorts.GetSize();
	CArray< CIPData *, CIPData * > aIPDataToFree;
	TRY
	{
		for(int i=0; i<nCount; i++)
		{
			COneDataPortItem * pPort = m_anPorts[i];
			if( pPort->m_AsyncHandle.IsEmpty() )
				continue;

			ONEASYNCREAD oneread = pPort->m_AsyncHandle.RemoveHead();
			aIPDataToFree.Add( oneread.m_pIPData );		
		}
	}
	CATCH( CMemoryException, e )
	{
#ifdef _DEBUG
		e->ReportError();
#endif // _DEBUG
	}
	END_CATCH

	symobj.Unlock();

	nCount = aIPDataToFree.GetSize();
	for( int i=0; i<nCount; i++)
	{
		m_pDecoderThread->DeAllocate( aIPDataToFree[i] );
		aIPDataToFree[i]->Release();
	}
}

///-------------------------------------------------------------------
/// 2002-12-31
/// ���ܣ�
///		��ȡͬ�����ݶ˿�
/// ��ڲ�����
///		��
/// ���ڲ�����
///		��
//	�޸ļ�¼��
// 2003-3-5 ����ͬ���˿ڷ�ʽ��ÿ�ζ�ȡ 100 �Σ���û������Ϊֹ
void CDPRecThread::DoReadSyncDataPort()
{
	CSingleLock symobj( &m_SynObj, TRUE );

	int nCount = m_anPorts.GetSize();
	CIPData * pIPData = NULL;
	for(int i=0; i<nCount; i++)
	{
		COneDataPortItem * pPort = m_anPorts[i];
		ASSERT( pPort );
		if( pPort->m_pDataPort->GetItemCount() )
			continue;				//	Ϊ�첽��ʽ�����ø÷�ʽ��ȡ����

		for(int j=0; j<100; j++)
		{
			if( NULL == pIPData )
			{							//	��Ҫ���з���
				pIPData = m_pDecoderThread->AllocatePacket( 0 );
				if( NULL == pIPData )
					return;				//	û���ˣ�ֻ�÷���
				if( FALSE == pIPData->SetBufSize( pPort->m_wPacketBufSize ) )
				{
					symobj.Unlock();		//	�����ڴ�ʧ�ܣ�����
					m_pDecoderThread->DeAllocate( pIPData );
					pIPData->Release();
					return;
				}
			}
			DWORD dwByteRead = 0;
			if( FALSE == pPort->m_pDataPort->ReadSync( pIPData->GetBuffer(), pPort->m_wPacketBufSize, &dwByteRead ) ||\
				0 == dwByteRead ) 
			{
				break;			// 2003-3-5 �Ķ˿�û�����ݣ��ֵ��¸��˿�
			}

			symobj.Unlock();
			OnDataOK( pPort, pIPData, dwByteRead );
			pIPData->Release();
			pIPData = NULL;
			symobj.Lock();					
		}
	}

	symobj.Unlock();		//	�����ڴ�ʧ�ܣ�����
	if( pIPData )
	{
		m_pDecoderThread->DeAllocate( pIPData );
		pIPData->Release();
	}
}
