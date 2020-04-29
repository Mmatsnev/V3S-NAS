// LicDecode.h: interface for the CLicDecode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LICDECODE_H__D8598D01_935F_11D3_BD17_005004868EAA__INCLUDED_)
#define AFX_LICDECODE_H__D8598D01_935F_11D3_BD17_005004868EAA__INCLUDED_

#include "CodecFormat.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CODUNLOCKDRVAPI.H"

class CLicDecode  
{
public:
	WORD GetVersion();
	BOOL IsToDelete();
	void Init( DVB_USER_ID & UserID);
	LICOP_METHOD GetLicMethod();
	void Detach();
	DWORD GetSysCodeIndex();
	PDWORD GetLicData();
	BOOL IsInRange();
	BOOL Attach( PBYTE pLicBuf );
	CLicDecode();
	virtual ~CLicDecode();

//	������
#ifdef __CYJ_TEST_LICCODE__
	int ListIDCode( PDWORD pBuf );
#endif // #ifdef __CYJ_TEST_LICCODE__

private:
	DWORD UnlockIDCode( DWORD dwIDCode );
	void LockIDCode( );

	DVB_USER_ID m_UserID;					//  2004-3-11 �û�����
	DWORD m_dwIDCodeReal;					//	ʵ�ʵĿ���
	PBYTE m_pDataBuf;						//	���ݻ�����
	PBLKHEADER m_pHeader;					//	����ͷ
	DWORD m_dwIDCode;						//	���ƺ�Ŀ���,��ʵ�ʱ���Ŀ���
};

#endif // !defined(AFX_LICDECODE_H__D8598D01_935F_11D3_BD17_005004868EAA__INCLUDED_)
