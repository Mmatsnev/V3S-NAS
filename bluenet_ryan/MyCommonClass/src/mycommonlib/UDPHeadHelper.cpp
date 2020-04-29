///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2002-11-20
///
///=======================================================

// UDPHeadHelper.cpp: implementation of the CUDPHeadHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "UDPHeadHelper.h"

#define IPPROTO_UDP	17


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUDPHeadHelper::CUDPHeadHelper()
{
	ASSERT( sizeof(UDPFAKEHEADER) == 20 );
}

CUDPHeadHelper::~CUDPHeadHelper()
{

}

// 
// Function: checksum
//
// Description:
//    This function calculates the 16-bit one's complement sum
//    for the supplied buffer
//
WORD CUDPHeadHelper::CheckSum(WORD *buffer, int size)
{
    unsigned long cksum=0;

    while (size > 1)
    {
        cksum += *buffer++;
        size  -= sizeof(WORD);   
    }
    if (size)
    {
        cksum += *(BYTE*)buffer;   
    }
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >>16); 

    return (WORD)(~cksum); 
}

///-------------------------------------------------------
/// 2002-11-20
/// ���ܣ�
///		���� UDP У���
/// ��ڲ�����
///		pUDPHeader				һ��UDP����pUDPHeader->length �Ѿ���ֵ
///		dwSrcIP					Դ IP�������ֽ�˳��
///		dwDstIP					Ŀ��IP�������ֽ�˳��
/// ���ز�����
///		У���
WORD CUDPHeadHelper::CheckSum(UDP_HEADER *pUDPHeader, DWORD dwSrcIP, DWORD dwDstIP)
{
	ASSERT( sizeof(UDPFAKEHEADER) == 20 );
	ASSERT( pUDPHeader && pUDPHeader->length );
	if( NULL == pUDPHeader )
		return 0;
	UDPFAKEHEADER FakeHeader;
	FakeHeader.m_dwSrcIP = dwSrcIP;
	FakeHeader.m_dwDstIP = dwDstIP;
	FakeHeader.m_byProtocol = IPPROTO_UDP;
	FakeHeader.m_byReserved_0 = 0;
	FakeHeader.m_wUDPLen = pUDPHeader->length;

	DWORD dwChkSum = 0;
	PWORD pwBufTmp = (PWORD)&FakeHeader;				//	�ȼ���αͷ��һ����
	for(int i=0; i<6; i++)
	{
		dwChkSum += pwBufTmp[i];
	}

	pUDPHeader->checksum = 0;							//	�� 0
	int nLen = XCHG(pUDPHeader->length);
	pwBufTmp = (PWORD) pUDPHeader;
	while( nLen > 1 )
	{
		dwChkSum += *pwBufTmp;
		pwBufTmp ++;
		nLen -= sizeof(WORD);
	}
	if( nLen )
		dwChkSum += *(BYTE*)pwBufTmp; 

	dwChkSum = (dwChkSum >> 16) + (dwChkSum & 0xffff);
    dwChkSum += (dwChkSum >>16);

    return (WORD)(~dwChkSum);
}

///-------------------------------------------------------
/// 2002-11-20
/// ���ܣ�
///		���� IP ͷ
/// ��ڲ�����
///		��
/// ���ز�����
///		IP ͷ��У���
WORD CUDPHeadHelper::CheckSum(LPIPHEADER lpIPHeader)
{
	ASSERT( lpIPHeader );
	int nIPHeadLen = (lpIPHeader->x & 0xF)*sizeof(DWORD);
	lpIPHeader->cksum = 0;
	return CheckSum( PWORD(lpIPHeader), nIPHeadLen );
}
