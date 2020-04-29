// IPFileMendHelper.h : Declaration of the CIPFileMendHelper

#ifndef __IPFILEMENDHELPER_H_
#define __IPFILEMENDHELPER_H_

#include "IPRecSvr.h"
#include "MyIUnknownImpl.h"

/////////////////////////////////////////////////////////////////////////////
// CIPFileMendHelper
class CIPFileMendHelper : public CMyIUnknownImpl<IIPFileMendHelper>
{
public:
	CIPFileMendHelper()
	{
		m_BitmapArray.SetSize( 0, 4096 );		//	ÿ�η���ͷ���4096�ֽ�
		m_nTotalBitCount = 0;
		m_nBit_1_Count = 0;
		m_nNextFileIDPtr = 0;
	}	

// IIPFileMendHelper
public:
	virtual IIPFileMendHelper * Clone();
	virtual BOOL SaveToFile( LPCSTR lpszFileName );
	virtual BOOL LoadFromFile( LPCSTR lpszFileName );
	virtual long Combine( IIPFileMendHelper *pSrcObj );
	virtual long ReStat();
	virtual long GetNextFileID( int nBitValue );
	virtual void Prepare( void ) ;
	virtual long SetBitValue( int nIndex,int nBitValue );
	virtual BOOL SetTotalSubFileCount(long nNewValue);
	virtual PBYTE GetDataBufferVC();
	virtual BOOL GetIsSubFileOK( long nIndex );
    virtual long GetSubFileHasReceived();
    virtual long GetTotalSubFileCount();


	virtual void SafeDelete()
	{
		delete this;
	}

private:
	CByteArray	m_BitmapArray;
	int			m_nBit_1_Count;			//	����Ϊ 1 �ĸ���
	int			m_nTotalBitCount;		//	��������
	int			m_nNextFileIDPtr;		//  ��һ���ļ��ı�����
};

#endif //__IPFILEMENDHELPER_H_
