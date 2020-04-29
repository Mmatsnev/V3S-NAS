// UnCompressObj.h: interface for the CUnCompressObj class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNCOMPRESSOBJ_H__73F868A1_736F_11D3_B1F1_005004868EAA__INCLUDED_)
#define AFX_UNCOMPRESSOBJ_H__73F868A1_736F_11D3_B1F1_005004868EAA__INCLUDED_

#include "Tsdb.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32			// linux
#include <MyFile.h>
#endif //_WIN32

class CUnCompressObj  
{
public:
	virtual void FreeMemory() =0;
	void Write( PBYTE pBuf, int nCount=1);
	void OutputOneByte( BYTE byData );
	int ReadOneByte();
	static BOOL IsCompress( int nLen, PBYTE pBuffer );
	PBYTE ReleaseDstBuffer(long & outfLen );
	void SetDstBuffer(int nDstBufLen,PBYTE pBuffer );
	PBYTE GetDataBuf();
	PTSDBCOMPRESSHEAD GetHeader();
	virtual void Detach();
	virtual PBYTE DecodeOneFile(int nFileNo,CFileStatus & outfStatus,PBYTE pDstBuf=NULL);
	virtual int GetFileInfo(int nFileNo,CFileStatus & outfStatus);
	virtual int GetFileNum();
	virtual int Attach(int nFileLen,PBYTE pBuf);
	virtual DWORD GetCompressMethod();
	virtual int GetDecoderVersion();

	CUnCompressObj();
	virtual ~CUnCompressObj();

public:	
	PTSDBCOMPRESSHEAD m_pTSDBCmpHead;					//	����ͷ
	int		m_nSrcDataLen;								//	ԭʼ����ͷ��С
	PBYTE	m_pSrcDataBuf;								//	��������ַ
	int		m_nDataRead;								//	�Ѿ���ȡ�������ֽ���
	PBYTE	m_pDstDataBuf;								//	Ŀ������
	PBYTE	m_pOutDataBufPtr;							//	��������ݻ�����ָ��
	int		m_nOutDataLen;								//	ʵ�������������С
	int		m_nDstBufSize;								//	�����������С
};

#endif // !defined(AFX_UNCOMPRESSOBJ_H__73F868A1_736F_11D3_B1F1_005004868EAA__INCLUDED_)
