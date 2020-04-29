// StatBpsHelper.h: interface for the CStatBpsHelper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STATBPSHELPER_H__5E29B9E3_3100_4991_9C18_07BB5478CB89__INCLUDED_)
#define AFX_STATBPSHELPER_H__5E29B9E3_3100_4991_9C18_07BB5478CB89__INCLUDED_

#pragma pack(push,4)

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32
	#include <MyComDef.h>
#endif // _WIN32

class CStatBpsHelper  
{
public:
	int SetStatTimePrecision( int nNewValue );
	BOOL SetStatMethod( BOOL bUseAverage = TRUE );
	void PresetBPS( int nNewValueKB );
	DWORD SetPeroid( DWORD dwNewValue );
	int GetBPS();
	void Reset();
	CStatBpsHelper();
	virtual ~CStatBpsHelper();

	void AddBytes( long nNewValue, BOOL bEnableCalucateBPS = TRUE );
	long CalculateBPS();

	static DWORD GetSysTickCount();

private:
	int		m_nPrecisionUnit;				//	���㾫�ȣ���λ���룬ȱʡΪ 1 MS
	BOOL	m_bUseAsAverage;				//	ƽ���÷�ʽ���м������ʣ�ȱʡΪ TRUE
	DWORD	m_dwTickCount;					//	��ǰʱ��
	DWORD	m_dwTotalBytes;					//	��ǰ�Ѿ����յ����ֽ���
	long	m_nBPSValue;					//	bps
	DWORD	m_dwPeroidTickCount;			//	ͳ�Ƽ��
	DWORD	m_dwActureTickCount;			//	ʵ�ʵ�ʱ����
};

#pragma pack(pop)

#endif // !defined(AFX_STATBPSHELPER_H__5E29B9E3_3100_4991_9C18_07BB5478CB89__INCLUDED_)
