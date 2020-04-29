// FileObject.h: interface for the CFileObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEOBJECT_H__BD65D9B8_912D_4151_BFCC_0D6D615F0443__INCLUDED_)
#define AFX_FILEOBJECT_H__BD65D9B8_912D_4151_BFCC_0D6D615F0443__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IPRecSvr.h"
#include "Tsdb.h"
#include <BufPacket4C.h>
class COneDataPortItem;

class CFileObject : public CBufPacket4C<IFileObject>
{
public:
	virtual PBYTE GetAttributeExtData( PDWORD pdwLen=NULL);
	virtual PBYTE GetExtData( PDWORD pdwLen=NULL );
	virtual PBYTE GetHugeFileParam( PDWORD pdwLen=NULL );
	virtual LPCSTR GetIPAddress();
	virtual int GetPort();
	virtual BOOL SaveTo( LPCSTR lpszPath, BOOL bIgnoreSubDirectory=FALSE, BOOL bRestoreTimes=TRUE);
	virtual DWORD GetFilePurpose();
	virtual time_t GetPacketTime();    
	virtual time_t GetLastModifyTime();
	virtual time_t GetCreatTime();
	virtual time_t GetLastAccessTime();
	CFileObject();
	virtual ~CFileObject();
	virtual LPCSTR GetFileName();
	virtual DWORD GetAttribute();

    virtual void SafeDelete();

//	IUnknwon
	virtual DWORD QueryInterface( REFIID iid,void ** ppvObject)
	{
		if( IID_IFileObject == iid )
		{
			AddRef();
			*ppvObject = static_cast<IFileObject*>(this);
			return 0;		// S_OK == 0
		}
		return CBufPacket4C<IFileObject>::QueryInterface( iid, ppvObject );
	}

/*
BEGIN_COM_MAP(CFileObject)
	COM_INTERFACE_ENTRY(IFileObject)
END_COM_MAP()
*/
public:
	void SetMulticastParameter( LPCSTR lpszIP, WORD wPort, COneDataPortItem * pDataPortItem );
	void PresetVar();
	BOOL SetHugeFileFileHeader( PTSDBFILEHEADER pFileHead );
	void DoTSDBSingleFile();	

	CString m_strMC_DstIP;					//	2002.11.14 �޸ģ���ಥ��صĲ���
	int		m_nMC_Port;						//	�ಥ�˿�
	COneDataPortItem *	m_pDataPortItem;	//  2004-5-20 data port item

	PTSDBHUGEFILEHEAD	m_pHugeFileParam;	//	���ļ�����
	PBYTE				m_pExtData;			//	���Ӳ���
	PTSDBFILEATTRIBHEAD	m_pAttributeData;	//	���Բ���
	PTSDBFILEHEADER		m_pFileHeader;		//	TSDB �ļ�ͷ
	CString				m_strFileName;		//	�ļ���
	time_t				m_PacketTime;		//	�ļ��������ʱ��
	CByteArray			m_FileHeadBuf;		//	�ļ�ͷ������
};

#endif // !defined(AFX_FILEOBJECT_H__BD65D9B8_912D_4151_BFCC_0D6D615F0443__INCLUDED_)
