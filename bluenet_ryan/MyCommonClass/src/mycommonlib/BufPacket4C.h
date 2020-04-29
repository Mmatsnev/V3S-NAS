// IpBroPacket.h: interface for the CIpBroPacket class.
//
//////////////////////////////////////////////////////////////////////
//	2002.11.15	��Ӻ�����IsBufAttached
//	2002.7.5	�޸� SetBufSize��������� m_dwAlignment������ڴ���䷽ʽ

#if !defined(AFX_IPBROPACKET_H__4BE7CB66_4619_446C_9902_66795779B44B__INCLUDED_)
#define AFX_IPBROPACKET_H__4BE7CB66_4619_446C_9902_66795779B44B__INCLUDED_

#pragma pack(push,4)

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdlib.h>
#include "IPRecDrvInterface.h"

template < class TBase >
class CBufPacket4C : public TBase
{
//	interface of IUnknown
public:
	virtual long AddRef(void)
	{
		return InterlockedIncrement( &m_nRef );
	}

	virtual long Release(void)
	{
		if( 0 == InterlockedDecrement( &m_nRef ) )
		{
			ASSERT( FALSE == m_bIsAttached );
			if( m_pBuf )
				free( m_pBuf );
			m_pBuf = NULL;
			SafeDelete();
			return 0;
		}
		return m_nRef;
	}

	virtual void SafeDelete() = 0;

	virtual DWORD QueryInterface( REFIID iid, void **ppvObject)
	{
		if( IID_IMyUnknown == iid || IID_IMyBufPacket == iid )
		{
			AddRef();
			*ppvObject = static_cast<IMyUnknown*>(this);
			return 0;         // S_OK
		}
		return 0x80004002;    //  E_NOINTERFACE;
	}

	void Preset();

//	interface of IpBroPacket
public:		
	//	����ʹ���ڴ棬nHeadLen ������ڴ��ֽ��������ص�ַ��ΪNULL��ʾʧ��
	virtual PBYTE	AcquireHeadBuf( int nHeadLen )
	{
		ASSERT( m_dwReservedBytes <= m_dwBufSize );		//	�����ֽڲ����ܳ����ܵĴ�С
		ASSERT( (m_dwDataLen+m_dwReservedBytes) <= m_dwBufSize );
		ASSERT( DWORD(nHeadLen) <= m_dwReservedBytes );

		if( nHeadLen > long(m_dwReservedBytes) )
			return NULL;

		m_dwReservedBytes -= nHeadLen;
		m_dwDataLen += nHeadLen;
		return ( m_pBuf + m_dwReservedBytes );
	}


	virtual BOOL	SetBufSize( DWORD dwNewValue )
	{
		ASSERT( FALSE == m_bIsAttached );
		if( m_bIsAttached )
			return FALSE;

		ASSERT( 0 == m_dwBufSize || m_dwReservedBytes <= m_dwBufSize );		//	�����ֽڲ����ܳ����ܵĴ�С
		ASSERT( 0 == m_dwBufSize || (m_dwDataLen+m_dwReservedBytes) <= m_dwBufSize );

		Preset();

		DWORD dwMemSize = dwNewValue + m_dwDefReservedBytes;
														//  2003-4-1 ���һ������ m_pBuf = NULL����Ҳ���·���
		if( dwMemSize > m_dwBufSize || 0 == dwMemSize || NULL == m_pBuf ||\
			( 2*dwMemSize < m_dwBufSize && m_dwBufSize > 20*1024 ) )	//	����20K���ͷ��ڴ�
		{												//	��Ҫ���·���
			m_dwBufSize = 0;
			if( 0 == dwMemSize )
            {
                if( m_pBuf )
                    free( m_pBuf );
                m_pBuf = NULL;
				return TRUE;
            }

			dwMemSize += m_dwMemAlignment;				//	�����ڴ�ķ���飬m_dwMemAlignment �Ѿ���һ
			dwMemSize &= (~m_dwMemAlignment);
            m_pBuf = (PBYTE)realloc( m_pBuf, dwMemSize );
			if( NULL == m_pBuf )
				return FALSE;							//	û���ڴ�		
			m_dwBufSize = dwMemSize;
		}

		ASSERT( m_pBuf && m_dwBufSize );
		
		return TRUE;
	}

	virtual PBYTE	GetBuffer()
	{
		ASSERT( m_dwReservedBytes <= m_dwBufSize );		//	�����ֽڲ����ܳ����ܵĴ�С
		ASSERT( (m_dwDataLen+m_dwReservedBytes) <= m_dwBufSize );

		return ( m_pBuf + m_dwReservedBytes );
	}

	//	��ȡ�û��Զ�������
	virtual DWORD	GetUserData()
	{
		ASSERT( m_dwReservedBytes <= m_dwBufSize );		//	�����ֽڲ����ܳ����ܵĴ�С
		ASSERT( (m_dwDataLen+m_dwReservedBytes) <= m_dwBufSize );		

		return m_nUserData;
	}

	//	�����µ��û����ݣ�ͬʱ����ԭ�����û�����
	virtual DWORD	PutUserData( DWORD dwNewValue )
	{
		DWORD dwOldUserData = m_nUserData;

		m_nUserData = dwNewValue;

		return dwOldUserData;
	}

	//	��ȡ����������ͷ����
	virtual DWORD	GetReservedBytes()
	{
		ASSERT( m_dwReservedBytes <= m_dwBufSize );		//	�����ֽڲ����ܳ����ܵĴ�С
		ASSERT( (m_dwDataLen+m_dwReservedBytes) <= m_dwBufSize );

		return m_dwReservedBytes;
	}

	//	��ȡ���ݴ�С
	virtual DWORD GetDataLen()
	{
		ASSERT( m_dwReservedBytes <= m_dwBufSize );		//	�����ֽڲ����ܳ����ܵĴ�С
		ASSERT( (m_dwDataLen+m_dwReservedBytes) <= m_dwBufSize );		

		return m_dwDataLen;
	}

	//	�������ݴ�С
	virtual void  PutDataLen( DWORD dwNewValue )
	{
		ASSERT( m_dwReservedBytes <= m_dwBufSize );		//	�����ֽڲ����ܳ����ܵĴ�С
		ASSERT( (m_dwDataLen+m_dwReservedBytes) <= m_dwBufSize );
		ASSERT( dwNewValue <= m_dwBufSize );
		ASSERT( (dwNewValue+m_dwReservedBytes) <= m_dwBufSize );

		m_dwDataLen = dwNewValue;
	}

	virtual DWORD GetBufSize()
	{
		ASSERT( m_dwReservedBytes <= m_dwBufSize );		//	�����ֽڲ����ܳ����ܵĴ�С
		ASSERT( (m_dwDataLen+m_dwReservedBytes) <= m_dwBufSize );
		return m_dwBufSize - m_dwReservedBytes;
	}

	CBufPacket4C(DWORD dwDefReservedByte = 4096, DWORD dwAlignment = 64 );
	virtual ~CBufPacket4C();

	//////////////////////////////////////////////
	//����:
	//		�ⲿ�趨�ڴ�
	//��ڲ���:
	//		pBuf				��������ַ
	//		dwBufSize			��������С
	//���ز���:
	//		��
	void	Attach( PBYTE pBuf, DWORD dwBufSize )
	{
		ASSERT( pBuf && dwBufSize );
		if( m_pBuf )
			free( m_pBuf );
		m_pBuf = pBuf;		
		m_dwBufSize = dwBufSize;
		Preset();
		m_bIsAttached = TRUE;
	}

	//////////////////////////////////////////////
	//����:
	//		ȡ���ڴ�
	//��ڲ���:
	//		dwBufSize			�����������С
	//���ز���:
	//		��������ַ������Ϊ 0
	PBYTE Detach( DWORD & dwBufSize )
	{
		PBYTE pBuf = m_pBuf;
		dwBufSize = m_dwBufSize;
		m_pBuf = NULL;
		m_dwBufSize = 0;		//  2003-4-1 ɾ���ڴ�ʱ��ͬʱ���ڴ��СΪ 0
		Preset();
		m_bIsAttached = FALSE;
		return pBuf;
	}

	///-------------------------------------------------------
	/// 2002-11-14
	/// ���ܣ�
	///		��ȡ�������Ƿ��� Attached
	/// ��ڲ�����
	///		�� 
	/// ���ز�����
	///		TRUE			ͨ�� Attached
	///		FALSE			���� Attached
	BOOL IsBufAttaced()
	{
		return m_bIsAttached;
	}

protected:
	long m_nRef;	
	//	ֻ�������������ʹ�øú������з���/�޸� m_dwReservedBytes
	DWORD & Admin_AccessReservedBytes()
	{
		return m_dwReservedBytes;
	}

private:
	long m_nUserData;
	PBYTE m_pBuf;		
	DWORD	m_dwReservedBytes;			//	�������ֽ���
	DWORD	m_dwBufSize;				//	��������С
	DWORD	m_dwDataLen;				//	���ݳ���
	DWORD	m_dwDefReservedBytes;		//	Ĭ�ϵı����ֽڳ���
	BOOL	m_bIsAttached;				//	�Ƿ��� Attach �����ڴ�
	DWORD	m_dwMemAlignment;			//	2002.7.5 ��ӣ��ڴ���䷽ʽ,ȱʡΪ 1 �ֽ�
};

template <class TBase>
CBufPacket4C<TBase>::CBufPacket4C(DWORD dwDefReservedByte/*=4096*/,DWORD dwAlignment /*= 64*/ )
 : m_nRef( long(0) ),
   m_pBuf( NULL )
{
	ASSERT( dwDefReservedByte <= 0xFFFF );
	ASSERT( dwAlignment > 0 && dwAlignment < 128*1024 );		//	���費����128K
	m_dwDefReservedBytes = dwDefReservedByte;		//	Ĭ�ϵı����ֽ�

	m_dwReservedBytes = 0;			//	�������ֽ���
	m_dwBufSize = 0;				//	��������С
	m_dwDataLen = 0;				//	���ݳ���
	m_nUserData = 0;	
	m_bIsAttached = FALSE;				//	Attach ��ʽ���õ��ڴ棬������������
	m_dwMemAlignment = dwAlignment-1;	//	1 �ֽڶ��룬��û��Ҫ��
}

template <class TBase>
CBufPacket4C<TBase>::~CBufPacket4C()
{
	ASSERT( FALSE == m_bIsAttached );
    if( m_pBuf )
        free( m_pBuf );
    m_pBuf = NULL;
};

//	���ó�ʼֵ
//	1�������ֽ�Ϊ 4K
template <class TBase>
void CBufPacket4C<TBase>::Preset()
{
	m_dwReservedBytes = m_dwDefReservedBytes;
	m_dwDataLen = 0;				//	û������
};

#pragma pack(pop)

#endif // !defined(AFX_IPBROPACKET_H__4BE7CB66_4619_446C_9902_66795779B44B__INCLUDED_)
