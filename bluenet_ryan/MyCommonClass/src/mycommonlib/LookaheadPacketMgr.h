///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2001-1-1
///
///		��;��
///			LookAhead ģʽ�������ݰ�
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӡ��ڲ�ʹ��!	  			     !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	��Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!			 !
///=======================================================

///////////////////////////////////////////////////////////
//  2005-1-28		���Ӷ� __LOOKAHEAD_CALL_CONSTRUCTURE_FOR_STRUCT__ ��֧�֣�ʹ��һ���ṹ�ܹ�����ʼ�����൱��һ����Ĺ��캯��
//	2003.8.21		Add catch function.
//	2002.7.5		��� GetItemCountInFreeList, GetInDataItemCount, DeAllocateEx
//	2002.1.3.22		��ԭ���� m_SynObj �ĳ� m_InDataSynObj �� m_FreeDataSynObj������Դ�������ͷ�
//					�������Ч��

#ifndef __LOOKAHEAD_PACKET_MGR_INCLUDE_20020124__
#define __LOOKAHEAD_PACKET_MGR_INCLUDE_20020124__

#ifndef _WIN32
	#include <unistd.h>
#endif //_WIN32

#pragma pack(push,4)

//ifdefine __LOOKAHEAD_DISABLE_CATCH_FUNCTION__ then all catch function is disabled
//#ifdefine __LOOKAHEAD_CALL_CONSTRUCTURE_FOR_STRUCT__	then call the OnConstructure function of the class/struct

//	T �����ҪҪ�з��� AddRef �� Release
//	T ������ṩ Preset ��������Ԥ�����ݣ�!!! Preset �����У����ܶ����ô��������޸ġ�
//	T ������ṩһ������ OnConstructure ��ʵ����Ĺ��캯����������������T����һ���ṹ����ʵ����Ĺ��캯��
template <class T> class CLookaheadPacketMgr
{
public:
	CLookaheadPacketMgr( int nMaxItem = 2048, unsigned int nCatchCount=0 )	//	Ĭ�������Է���2K����Ԫ
	{
		m_nMaxItemCount = nMaxItem;				//	�����Է�������ݰ�����
		m_nPacketAllocated = 0;					//	�Ѿ���������ݸ���
	#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
        m_nCatchItemCount = nCatchCount;
        if( m_nCatchItemCount > (nMaxItem/4) )	//	can not too large
			m_nCatchItemCount = (nMaxItem/4);
	#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
	};

	~CLookaheadPacketMgr()
	{											//	�����������ͷſռ�
		Reset();
	};

	//	�����������
	void Clean(BOOL bDoLock = TRUE)
	{
		CSingleLock insynobj( &m_InDataSynObj, bDoLock );
		CSingleLock freesynobj( &m_FreeDataSynObj, bDoLock );
		if( !m_InDataList.IsEmpty() )
		{
			m_FreePacketList.AddTail( &m_InDataList );
			m_InDataList.RemoveAll();
        }

	#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
		//	release all catched data
        if( !m_DeAllocateCatchList.IsEmpty() )
        {
	        m_FreePacketList.AddTail( &m_DeAllocateCatchList );
    	    m_DeAllocateCatchList.RemoveAll();
        }

		if( !m_AddPacketCatchList.IsEmpty() )
        {
	        m_FreePacketList.AddTail( &m_AddPacketCatchList );
    	    m_AddPacketCatchList.RemoveAll();
        }

		if( !m_PeekDataCatchList.IsEmpty() )
        {
	        m_FreePacketList.AddTail( &m_PeekDataCatchList );
    	    m_PeekDataCatchList.RemoveAll();
        }
	#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
	};

	//	�ͷ��������ݰ�
	void Reset(BOOL bDoLock = TRUE )
	{
		CSingleLock synobj( &m_FreeDataSynObj, bDoLock );
		Clean( FALSE );
		int nCount = m_FreePacketList.GetCount();
		for(int i=0; i<nCount; i++)
		{
			T * pPacket = m_FreePacketList.RemoveHead();
			ASSERT( pPacket );
			if( pPacket )
				pPacket->Release();
		}
		m_nPacketAllocated = 0;					//	�Ѿ���������ݸ���

	#ifdef _WIN32
		m_FreePacketEvent.ResetEvent();			//	�������ͷţ�����������
		m_AddDataEvent.ResetEvent();			//	�������
	#endif //_WIN32  // I do not known, should I reset the event under Linux ?
	};

	//	������ݰ�
	BOOL	AddPacket( T * pPacket, BOOL bDoLock = TRUE, BOOL bFlush = FALSE )
	{
		ASSERT( pPacket );
		if( NULL == pPacket )
			return FALSE;

		pPacket->AddRef();

	#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
		if( m_nCatchItemCount > 1 )
        {							//	support deallocate
    	    if( m_AddPacketCatchList.AddTail( pPacket ) )
            {
				if( FALSE == bFlush && m_AddPacketCatchList.GetCount() < m_nCatchItemCount )
                	return TRUE;
             	CSingleLock synobj( &m_InDataSynObj, bDoLock );
                m_InDataList.AddTail( &m_AddPacketCatchList );
                m_AddPacketCatchList.RemoveAll();

			#ifdef _WIN32
                SetInDataEvent();
			#endif // #ifdef _WIN32
				return TRUE;
            }
		}
	#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__

		CSingleLock synobj( &m_InDataSynObj, bDoLock );

		if( NULL == m_InDataList.AddTail( pPacket ) )
        {
        	pPacket->Release();
        	return FALSE;
        }
	#ifdef _WIN32
		SetInDataEvent();
	#endif //_WIN32
		return TRUE;
	};

	///----------------------------------------------------
	///	ˢ�� AddPacket ������
	void FlushAddCache( BOOL bDoLock = TRUE )
	{
	#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
		CSingleLock synobj( &m_InDataSynObj, bDoLock );
		if( FALSE == m_AddPacketCatchList.IsEmpty() )	//  CYJ,2006-2-26 ��������жϣ���ֹû������ʱҲ�����ź�
		{
			m_InDataList.AddTail( &m_AddPacketCatchList );
			m_AddPacketCatchList.RemoveAll();

		#ifdef _WIN32
			SetInDataEvent();
		#endif // #ifdef _WIN32
		}
	#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
	}

	//	�������ݰ�
	//	��ڲ���
	//		dwTimeOut		��ʱʱ�䣬Ĭ��һֱ�ȴ�
	//	���ز���
	//		NULL			ʧ��
	T * AllocatePacket( DWORD dwTimeOut = -1, BOOL bDoLock = TRUE )
	{
		CSingleLock synobj( &m_FreeDataSynObj );
		if( bDoLock && FALSE == synobj.Lock( dwTimeOut ) )
			return NULL;						//	��ʱ

		if( 0 == m_FreePacketList.GetCount() )
		{										//	���ж�����û�����ݣ��Ӷ��з���
			if( m_nPacketAllocated >= m_nMaxItemCount )
			{									//	�����ٴӶ��з��䣬ֻ�õȵȿ�����û���ͷŻ���
				if( 0 == dwTimeOut )
					return NULL;				//	���ȴ���ֱ�ӷ���
				if( bDoLock )
					synobj.Unlock();

			#ifdef _WIN32
				if( FALSE == WaitForFreePacketEvent( dwTimeOut ) )
					return NULL;				//	��ʱ����û�����ݰ��ͷ�
			#else
				// linux
				while( 1 )
				{
					if( GetItemCountInFreeList( bDoLock ) )
						break;
					if( 0xFFFFFFFF != dwTimeOut )
					{
						if( 1 == dwTimeOut )
							return NULL;
						dwTimeOut --;
					}
					usleep( 1000 );
				}
			#endif //_WIN32

				if( bDoLock && FALSE == synobj.Lock( dwTimeOut ) )
					return NULL;

				if( 0 == m_FreePacketList.GetCount() )
					return NULL;				//	�ٴ��ж�
			}
			else
			{									//	�����Է���
				T * pPacket = new T;
				if( NULL == pPacket )
					return NULL;
			#ifdef __LOOKAHEAD_CALL_CONSTRUCTURE_FOR_STRUCT__
				pPacket->OnConstructure();		//  CYJ,2005-1-28 ģ��ʵ��һ����Ĺ��캯����
			#endif // __LOOKAHEAD_CALL_CONSTRUCTURE_FOR_STRUCT__
				m_nPacketAllocated ++;
				pPacket->AddRef();
			#ifdef _DEBUG
				m_pLastAllocObject = pPacket;
			#endif // _DEBUG
				pPacket->Preset();
				return pPacket;
			}
		}
		ASSERT( m_nPacketAllocated <= m_nMaxItemCount );
		ASSERT( m_FreePacketList.GetCount() );
		T * pPacket = m_FreePacketList.RemoveHead();
		ASSERT( pPacket );						//	��Ϊ DeAllocate �Ѿ����� AddRef;
	#ifdef _DEBUG
		m_pLastAllocObject = pPacket;
	#endif // _DEBUG
		pPacket->Preset();
		return pPacket;
	};

	//	�ͷ����ݰ�
	void DeAllocate( T * pPacket, BOOL bDoLock = TRUE  )
	{
		ASSERT( pPacket );
		pPacket->AddRef();

	#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
		if( m_nCatchItemCount > 1 )
        {							//	support deallocate
	        if( m_DeAllocateCatchList.AddTail( pPacket ) )
            {						//	If add tail fail, try another way
                if( m_DeAllocateCatchList.GetCount() < m_nCatchItemCount )
                    return;

                CSingleLock synobj( &m_FreeDataSynObj, bDoLock );
                m_FreePacketList.AddTail( &m_DeAllocateCatchList );
                m_DeAllocateCatchList.RemoveAll();
			#ifdef _WIN32
                SetFreePacketEvent();
			#endif //_WIN32
                return;
            }
        }
	#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__

		CSingleLock synobj( &m_FreeDataSynObj, bDoLock );
		if( m_FreePacketList.AddTail( pPacket ) )
		{
		#ifdef _WIN32
			SetFreePacketEvent();
		#endif // #ifdef _WIN32
		}
        else
        {
         	m_nPacketAllocated --;			//	add to free list fail, so release it directly
            pPacket->Release();
        }
	};

	//	2002.7.5 ���
	//	�ͷ����ݰ�
	void DeAllocateEx( T * pPacket, BOOL bDiscard = FALSE, BOOL bDoLock = TRUE  )
	{
		ASSERT( pPacket );
		if( bDiscard )
		{
			CSingleLock synobj( &m_FreeDataSynObj, bDoLock );
			m_nPacketAllocated --;			//	�����ˣ�����δ������ö���
		}
		else
			DeAllocate( pPacket, bDoLock );
	};

#ifdef _WIN32
	//	�ȴ����������¼�
	BOOL WaitForInDataEvent(DWORD dwTimeOut = -1)
	{
		ASSERT( dwTimeOut );
		CSingleLock synobj( &m_AddDataEvent );
		return synobj.Lock( dwTimeOut );
	};
#endif // #ifdef _WIN32

#ifdef _WIN32
	//	�ȴ��ͷ������¼�
	BOOL WaitForFreePacketEvent( DWORD dwTimeOut = -1 )
	{
		ASSERT( dwTimeOut );
		CSingleLock synobj( &m_FreePacketEvent );
		return synobj.Lock( dwTimeOut );
	};
#endif // #ifdef _WIN32

	//	��ȡ����
	//	��ڲ���
	//		dwTimeOut			0	��ʾ���ȴ�
	//							-1  һֱ�ȴ�
	T * PeekData( DWORD dwTimeOut = -1, BOOL bDoLock = TRUE  )
	{
	    T * pPacket;

	#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
	    if( FALSE == m_PeekDataCatchList.IsEmpty() )
        {
			pPacket = m_PeekDataCatchList.RemoveHead();
			return pPacket;							//	���õ��� AddRef ��Ϊ AddPacket �Ѿ����� AddRef
        }
	#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__

		CSingleLock synobj( &m_InDataSynObj );
		if( bDoLock && FALSE == synobj.Lock( dwTimeOut ) )
			return NULL;

		if( FALSE == m_InDataList.IsEmpty() )
		{
        	pPacket = m_InDataList.RemoveHead();

	#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
			if( FALSE == m_InDataList.IsEmpty() )
            {
		    	m_PeekDataCatchList.AddTail( &m_InDataList );
    	        m_InDataList.RemoveAll();
            }
	#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__

			return pPacket;							//	���õ��� AddRef ��Ϊ AddPacket �Ѿ����� AddRef
		}
		if( 0 == dwTimeOut )
			return NULL;			//	���ȴ�����û������������

		if( bDoLock )
			synobj.Unlock();		//	�ͷţ��ȴ��µ�����

	#ifdef _WIN32
		if( FALSE == WaitForInDataEvent( dwTimeOut ) )
			return NULL;

		ASSERT( FALSE == m_InDataList.IsEmpty() );
	#else	// Linux, wait myself
		while( 1 )
		{
			if( GetInDataItemCount( bDoLock ) )
				break;

			if( 0xFFFFFFFF != dwTimeOut )
			{
				if( 1 == dwTimeOut )
					return NULL;			// timeout
				dwTimeOut --;
			}
			usleep( 1000 );	// wait 1 ms
		}
	#endif // #ifdef _WIN32

		if( bDoLock && FALSE == synobj.Lock( dwTimeOut ) )
			return NULL;

		return m_InDataList.RemoveHead();			//	���õ��� AddRef ��Ϊ AddPacket �Ѿ����� AddRef
	};

#ifdef _WIN32
	//	����������
	void	SetInDataEvent()
	{
		m_AddDataEvent.PulseEvent();
	};
#endif // #ifdef _WIN32

#ifdef _WIN32
	//	���ݰ��ͷ��¼�
	void	SetFreePacketEvent()
	{
		m_FreePacketEvent.PulseEvent();
	};
#endif // #ifdef _WIN32

	////////////////////////////////////////////////////
	//	2002.7.5 ���
	//	��ȡ�����б��е���Ŀ��
	int		GetItemCountInFreeList(BOOL bDoLock = TRUE)
	{
		CSingleLock synobj( &m_FreeDataSynObj, bDoLock );
		return m_FreePacketList.GetCount();
	}

	////////////////////////////////////////////////////
	//	2002.7.5 ���
	//	��ȡ�����б��е���Ŀ��
	int		GetInDataItemCount(BOOL bDoLock = TRUE)
	{
		CSingleLock synobj( &m_InDataSynObj, bDoLock );

	#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
		return m_InDataList.GetCount() + m_PeekDataCatchList.GetCount();
	#else
		return m_InDataList.GetCount();
	#endif //__LOOKAHEAD_DISABLE_CATCH_FUNCTION__
	}

public:
	CCriticalSection	m_InDataSynObj;		//	InData,�߳̿���Ȩͬ������
	CCriticalSection	m_FreeDataSynObj;	//	FreeData,�߳̿���Ȩͬ������

protected:
#ifdef _WIN32
	CEvent	m_FreePacketEvent;				//	�������ͷţ�����������
	CEvent	m_AddDataEvent;					//	�������
#endif // #ifdef _WIN32

#ifdef _WIN32
	CList< T *, T* > m_FreePacketList;    	//	��ǰ�������õ����ݰ�
	CList< T *, T* > m_InDataList;	    	//	�����ݵ����ݰ�
#else // LInux
    CMyList< T * > m_FreePacketList;    	//	��ǰ�������õ����ݰ�
	CMyList< T * > m_InDataList;	    	//	�����ݵ����ݰ�
#endif // _WIN32

	int	m_nMaxItemCount;					//	�����Է�������ݰ�����
	int	m_nPacketAllocated;					//	�Ѿ���������ݸ���

#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
  	int	m_nCatchItemCount;					//	Catch Item count, if 0, do not catch any item
#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__

#ifdef _DEBUG
	T * m_pLastAllocObject;
#endif // _DEBUG

private:
#ifndef __LOOKAHEAD_DISABLE_CATCH_FUNCTION__

  #ifdef _WIN32									//	2003.8.21 add, catch list
	CList< T *, T* > m_DeAllocateCatchList;		//	catch for Deallocate function
    CList< T *, T* > m_AddPacketCatchList;  	//	catch for AddPacket function
    CList< T *, T* > m_PeekDataCatchList;  		//	catch for Peek Data function
  #else	//Linux
	CMyList< T * > m_DeAllocateCatchList;  		//	catch for allocate function
    CMyList< T * > m_AddPacketCatchList;  		//	catch for AddPacket function
    CMyList< T *>  m_PeekDataCatchList;  			//	catch for Peek Data function
  #endif // _WIN32

#endif // __LOOKAHEAD_DISABLE_CATCH_FUNCTION__
};

#pragma pack(pop)

#endif // __LOOKAHEAD_PACKET_MGR_INCLUDE_20020124__

