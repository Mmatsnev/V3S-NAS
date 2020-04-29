// UDPDataPort.h: interface for the CUDPDataPort class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UDPDATAPORT_H__6AD7167C_11A4_474A_AAD2_B9A7F6FF4AF3__INCLUDED_)
#define AFX_UDPDATAPORT_H__6AD7167C_11A4_474A_AAD2_B9A7F6FF4AF3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SrcDataPort.H"
#include <afxmt.h>
#include "MyWS2_32.h"	// Added by ClassView
#include <afxtempl.h>
#include <WS2TCPIP.H>

typedef struct tagUDPOVERLAP
{
	WSAEVENT		m_hEvent;					//	ͬ������
	BOOL			m_bIsPending;				//	�Ƿ��ӳٲ���
	WSABUF			m_wsaDataBuf;				//	Socket �õĻ�����
	WSAOVERLAPPED	m_overlapped;				//	�ص�����
	DWORD			m_dwByteRead;				//	�ɹ���ȡ���ֽ���
	DWORD			m_dwFlags;					//	��־
}UDPOVERLAP,*PUDPOVERLAP;

class CUDPDataPort : public CSrcDataPort  
{
public:
	CUDPDataPort();
	virtual ~CUDPDataPort();

public:	
	virtual BOOL	Initialize( LPCSTR lpszIP, UINT nPort, LPCSTR lpszLocalBind = NULL, int nCount = -1 );
	virtual void	Invalidate();
	virtual HANDLE 	GetEventHandle(SDP_HANDLE hNo);
	virtual SDP_HANDLE	ReadAsyn(	PBYTE pBuf, DWORD dwBufSize,PDWORD pdwByteRead );
	virtual BOOL	ReadSync( PBYTE pBuf, DWORD dwBufSize, PDWORD pdwByteRead );
	virtual BOOL	GetOverlappedResult( SDP_HANDLE hReadNo, PDWORD pdwByteRead, BOOL bWait );
	virtual void	CancelAsynRead(SDP_HANDLE hReadNo);
	virtual int		GetItemCount();
	virtual void	SafeDelete();
	virtual	BOOL	CanIDoReadAsync();

	enum { DEFAULT_ASYN_COUNT = 128 };

private:
	int m_nItemCount;			//	�첽�����ĸ���
	int	m_nCurReadItemCount;	//	��ǰ��ȡ�ļ�¼��

	SOCKET m_hSocket;
	struct ip_mreq m_mrMReq;			// Contains IP and interface of the host group

private:
	BOOL m_bNeedToCallCleanUp;
	CArray<UDPOVERLAP,UDPOVERLAP&> m_asynobjs;
	CMyWS2_32 m_drv;
};

#endif // !defined(AFX_UDPDATAPORT_H__6AD7167C_11A4_474A_AAD2_B9A7F6FF4AF3__INCLUDED_)
