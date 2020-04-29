// StatBpsHelper.cpp: implementation of the CStatBpsHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdio.h>
#include "StatBpsHelper.h"

#ifndef _WIN32
	#include <MySyncObj.h>
    #include <sys/time.h>
#endif //_WIN32

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStatBpsHelper::CStatBpsHelper()
{
	Reset();
}

CStatBpsHelper::~CStatBpsHelper()
{

}

///-------------------------------------------------------
/// 2003-2-22
/// ���ܣ�
///		���³�ʼ��
/// ��ڲ�����
///		��
/// ���ز�����
///		��
void CStatBpsHelper::Reset()
{	
	m_dwActureTickCount = GetSysTickCount();
	m_dwTickCount = m_dwActureTickCount;
	m_dwTotalBytes = 0;
	m_nBPSValue = 0;
	m_dwPeroidTickCount = 4000;
	m_bUseAsAverage = TRUE;
	m_nPrecisionUnit = 1;			//	ȱʡΪ 1 MS
}

///-------------------------------------------------------
/// 2003-2-22
/// ���ܣ�
///		��ȡ����ֵ
/// ��ڲ�����
///		��
/// ���ز�����
///		��ǰ��BPS����λ bps
int CStatBpsHelper::GetBPS()
{
	return ::InterlockedExchangeAdd( &m_nBPSValue, 0 );
}

///-------------------------------------------------------
/// 2003-2-22
/// ���ܣ�
///		���ü������
/// ��ڲ�����
///		dwNewValue			�µļ������
/// ���ز�����
///		ԭ���ļ������
DWORD CStatBpsHelper::SetPeroid(DWORD dwNewValue)
{
#ifdef _DEBUG
	ASSERT( 0 == (m_dwPeroidTickCount%m_nPrecisionUnit) );
	if( (m_dwPeroidTickCount%m_nPrecisionUnit) )
		TRACE("It's recommanded the PeroidTickCount should be multiple of Precision.\n");
#endif // _DEBUG
	DWORD dwOldValue = m_dwPeroidTickCount;
	if( dwNewValue < 50 )				// ����̫С
		dwNewValue = 2000;
	m_dwPeroidTickCount = dwNewValue;
	return dwOldValue;
}

///-------------------------------------------------------
/// 2003-2-23
/// ���ܣ�
///		Ԥ�����ʣ���λ kbps
/// ��ڲ�����
///		nNewValueKB		Ԥ�Ƶ�����
/// ���ز�����
///		��
void CStatBpsHelper::PresetBPS(int nNewValueKB)
{
	m_nBPSValue = nNewValueKB;
	m_dwTickCount = GetSysTickCount() - 2000;	//	�뵽��2��ǰ��ʱ��
	m_dwTotalBytes = nNewValueKB/4;
}

///-------------------------------------------------------
/// 2003-2-28
/// ���ܣ�
///		�������
/// ��ڲ�����
///		nNewValue				�ֽ���
///		bEnableCalucateBPS		����������ʣ�ȱʡΪ TRUE
/// ���ز�����
///
void CStatBpsHelper::AddBytes( long nNewValue, BOOL bEnableCalucateBPS )
{
	m_dwTotalBytes += nNewValue;
	if( bEnableCalucateBPS )
		CalculateBPS();
}

///-------------------------------------------------------
/// 2003-2-23
/// ���ܣ�
///		�������ʣ������ص�ǰ����
/// ��ڲ�����
///		��
/// ���ز�����
///		��ǰ����
long CStatBpsHelper::CalculateBPS()
{
	DWORD dwActualTickCount = GetSysTickCount();
	DWORD dwTickCount = dwActualTickCount;
	if( m_nPrecisionUnit > 1 )
		dwTickCount = dwTickCount - (dwTickCount%m_nPrecisionUnit);	//	ȡ��
	ASSERT( m_dwPeroidTickCount >= 8 );
	if( DWORD(dwTickCount - m_dwTickCount) < m_dwPeroidTickCount ||\
		DWORD(dwActualTickCount-m_dwActureTickCount) < m_dwPeroidTickCount*4/5 )
	{
		return m_nBPSValue;
	}
	m_dwActureTickCount = dwActualTickCount;
	DWORD dwDelta = dwTickCount-m_dwTickCount;
	float fBPS = float(m_dwTotalBytes) * 8000.0f;
	long nBPS = long( fBPS/dwDelta + 0.5f);		//	��������
	::InterlockedExchange( &m_nBPSValue, nBPS );

//	TRACE("This=%08X, %d Bytes / %d ms = %d bps.\n", this, m_dwTotalBytes, dwDelta, nBPS );

	if( m_bUseAsAverage )
	{
		m_dwTickCount = m_dwTickCount/2 + dwTickCount/2;
		m_dwTotalBytes /= 2;
	}
	else
	{
		m_dwTickCount = dwTickCount;
		m_dwTotalBytes = 0;
	}

	return nBPS;
}

///-------------------------------------------------------
/// 2003-3-1
/// ���ܣ�
///		ʹ��ƽ���÷�ʽ���м���
/// ��ڲ�����
///		bUseAverage				= TRUE ǰ������ƽ��
/// ���ز�����
///		ԭ��������
BOOL CStatBpsHelper::SetStatMethod(BOOL bUseAverage)
{
	BOOL bRetVal = m_bUseAsAverage;
	m_bUseAsAverage = bUseAverage;
	return bRetVal;
}

///-------------------------------------------------------
/// 2003-3-1
/// ���ܣ�
///		����ͳ��ʱ�侫��
/// ��ڲ�����
///		nNewValue		ʱ�侫�ȣ���λ MS
/// ���ز�����
///		ԭ��������ֵ
int CStatBpsHelper::SetStatTimePrecision(int nNewValue)
{
	ASSERT( nNewValue >= 1 );
	if( nNewValue < 1 )
		nNewValue = 1;
	int nRetVal = m_nPrecisionUnit;
	m_nPrecisionUnit = nNewValue;
	return nRetVal;
}

///-----------------------------------------------
///	Get System tick count
DWORD CStatBpsHelper::GetSysTickCount()
{
#ifdef _WIN32
	return ::GetTickCount();
#else  // Linux
	struct timeval now;
    gettimeofday( &now, NULL );
    DWORD dwRetVal = now.tv_sec * 1000 ;
    dwRetVal += (now.tv_usec/1000);
    return dwRetVal;
#endif //	_WIN32
}
