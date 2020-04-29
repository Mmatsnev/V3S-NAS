///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///
///=======================================================

// DecoderThread.cpp: implementation of the CDecoderThread class.
//
//////////////////////////////////////////////////////////////////////
//	2002.12.11	Lookahead packet count = 8k��ԭ��Ϊ 2048 ������
//	2002.11.15	�����̵߳ļ���

#include "stdafx.h"
#include "DecoderThread.h"

#ifdef _WIN32
    #ifdef _DEBUG
	    #undef THIS_FILE
 	   static char THIS_FILE[]=__FILE__;
  	  #define new DEBUG_NEW
    #endif
#else
	#include <unistd.h>    
	#include <pthread.h>
#endif //_WIN32    

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDecoderThread::CDecoderThread() : CLookaheadPacketMgr< CIPData >( 10240, 512 )	// 10K
{

}

CDecoderThread::~CDecoderThread()
{

}

int CDecoderThread::ExitInstance()					//	���ز���
{
	return 0;
}

#ifdef _WIN32
BOOL CDecoderThread::InitInstance()					//	�Ƿ�ɹ�
#else
bool CDecoderThread::InitInstance()					//	�Ƿ�ɹ�
#endif //_WIN32
{
	return TRUE;
}

void CDecoderThread::Run()							//	��������
{
	m_bIsRequestQuit = FALSE;

#ifdef _WIN32
//	SetThreadPriority( m_hThread, THREAD_PRIORITY_ABOVE_NORMAL );	//	2002.11.15, �ı䵽�ϸ߼���
#else
	nice( -3 );			// I dont known how to change priority
   #ifdef _DEBUG
	TRACE("CDecoderThread run is called\n");
   #endif //_DEBUG
#endif //_WIN32    

	while( FALSE == m_bIsRequestQuit )
	{
#ifdef _DEBUG_
	printf("CDecoderThread::running, enter peek data\n");
#endif //_DEBUG

#ifdef _WIN32
		CIPData * pIPData = PeekData( 20 );
#else
		CIPData * pIPData = PeekData( 0 );
#endif	//_WIN32

#ifdef _DEBUG_
	printf("CDecoderThread::running, return from peek data\n");
#endif //_DEBUG

		if( NULL == pIPData )
		{
#ifdef __USE_GNU
			pthread_yield();
#endif //__USE_GNU
			Sleep( 20 );	// no data, sleep 20 ms
			continue;								//	��ʱû������
		}

#ifdef __PRINT_DEBUG_MSG_RELEASE__
//		printf("CDecoderThread::Run.\n");
#endif // __PRINT_DEBUG_MSG_RELEASE__

		ASSERT( pIPData->m_pDataPortItem );
        if( pIPData->m_pDataPortItem )
        {
			pIPData->m_pDataPortItem->m_FileCombiner.DoInputOnePage( pIPData->m_pDataPortItem,\
				pIPData->GetBuffer(), pIPData->GetDataLen() );
        }

		DeAllocate( pIPData );
		pIPData->Release();
	}
}

void CDecoderThread::Delete()						//	ɾ���Լ�
{
	// variable on data segment, need not delete
}
