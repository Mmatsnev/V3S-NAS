#ifndef __IP_DATA_H_20040520__
#define __IP_DATA_H_20040520__

#include "IPRecSvr.h"
#include <BufPacket4C.h>
#include "SrcDataPort.h"
#include "BaseFileCombiner.h"
#include "FileObject.h"
#include "TSDBFileSystem.h"
#include "IPFileMendHelper.h"

#ifndef _WIN32
	#include <MyList.h>
#endif//_WIN32

class CIPData;

typedef struct tagONEASYNCREAD
{
	SDP_HANDLE	m_hSDP;
	CIPData	*	m_pIPData;
}ONEASYNCREAD,*PONEASYNCREAD;

#define IP_MAX_PACKET_SIZE	2048

#pragma pack(push,1)

class COneDataPortItem
{
public:
	COneDataPortItem()
	{
		m_pDataPort = NULL;
		m_pFileObjMgr = NULL;
		m_wPacketBufSize = IP_MAX_PACKET_SIZE;							//	Ĭ�ϰ���СΪ 2K
		m_nPort = 0;
		m_pFileMendHelper = NULL;		
		memset( &m_ReceiveLog, 0, sizeof(m_ReceiveLog) );
	};

	~COneDataPortItem()
	{
		ASSERT( NULL == m_pFileMendHelper );
	}

	typedef struct tagFILERECEIVELOG
	{
		DWORD	m_dwID;					//	�ļ�ID
		DWORD	m_dwFileCount;			//	�ܹ��ֽ���
		DWORD	m_dwTotalLen_16KB;		//	�ܹ�����
		DWORD	m_dwOKFileCount;		//	�ɹ����յ��ļ�����
		DWORD	m_dwOKLen_16KB;			//	�ɹ����յ��ܳ��ȣ���λ 16KB
		DWORD	m_dwOkLen_Below16KB;	//	�������ݣ�����׼ȷ����16K
		float	m_fProgress;			//	����
	}FILERECEIVELOG;

	CSrcDataPort		*	m_pDataPort;					//	����Դ�˿�,һ���� ActiveX ����
#ifdef _WIN32
	CList<ONEASYNCREAD,ONEASYNCREAD&> m_AsyncHandle;		//	�ȴ�����ľ������
#else
	CMyList<ONEASYNCREAD> m_AsyncHandle;		//	�ȴ�����ľ������
#endif //_WIN32
	CBaseFileCombiner	m_FileCombiner;						//	�ļ�ƴ�϶���
	CTSDBFileSystem *	m_pFileObjMgr;						//	�ļ��������<==> ActiveX
	CIPFileMendHelper * m_pFileMendHelper;					//  2003-4-11 ���

	WORD	m_wPacketBufSize;								//	����С
	long	m_nPort;
	CString m_strTargetIP;

	FILERECEIVELOG	m_ReceiveLog;
public:
	//////////////////////////////////////////////
	//����:
	//		��ȡ��һ���첽�������
	//��ڲ���:
	//		��
	//���ز���:
	//		>			�����Ŀ
	//		<0			ʧ��
	SDP_HANDLE	GetHeadHandle()
	{
		if( m_AsyncHandle.IsEmpty() )
			return -1;
		ONEASYNCREAD & oneread = m_AsyncHandle.GetHead();
		return oneread.m_hSDP;
	};
};

class CIPData : public CBufPacket4C<IBufPacket>
{
public:
	CIPData() : CBufPacket4C<IBufPacket>( 0 ){ m_pDataPortItem = NULL; };
	COneDataPortItem * m_pDataPortItem;			//	���Ӳ���	
	void DeleteHeadData( int nHeadLen )
	{
		ASSERT( nHeadLen < (int)GetDataLen() );
		Admin_AccessReservedBytes() += nHeadLen;
		PutDataLen( GetDataLen() - nHeadLen );
	}
    virtual void SafeDelete()
    {
    	delete this;
    };
};

#pragma pack( pop )

#endif // __IP_DATA_H_20040520__
