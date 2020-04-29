// DecoderThread.h: interface for the CDecoderThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DECODERTHREAD_H__D458920C_6D5E_4603_9349_97FE254B2498__INCLUDED_)
#define AFX_DECODERTHREAD_H__D458920C_6D5E_4603_9349_97FE254B2498__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MyThread.h>

#ifndef _WIN32
	#include <MySyncObj.h>
    #include <MyArray.h>
    #include <MyList.h>
    #include <MyMap.h>
#endif //_WIN32

#include <LookaheadPacketMgr.h>
#include "IPData.h"

class CDecoderThread : 	public CMyThread ,
	public CLookaheadPacketMgr< CIPData >
{
public:
	CDecoderThread();
	virtual ~CDecoderThread();

	virtual int ExitInstance();					//	���ز���
#ifdef _WIN32
	virtual BOOL InitInstance();				//	�Ƿ�ɹ�
#else
	virtual bool InitInstance();				//	�Ƿ�ɹ�
#endif //_WIN32
	virtual void Run();							//	��������
	virtual void Delete();						//	ɾ���Լ�

};

#endif // !defined(AFX_DECODERTHREAD_H__D458920C_6D5E_4603_9349_97FE254B2498__INCLUDED_)
