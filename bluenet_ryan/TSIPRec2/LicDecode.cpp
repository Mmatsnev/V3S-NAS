// LicDecode.cpp: implementation of the CLicDecode class.
//
//////////////////////////////////////////////////////////////////////
//	2000.9.5	��ӳ�Ա���� GetVersion, ��ȡ���ݸ�ʽ�İ汾��

#include "stdafx.h"
#include "LicDecode.h"
#include "Crc.h"

#ifdef _WIN32
  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif
#endif //_WIN32  

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//	�����ܵĿ���
CLicDecode::CLicDecode()
{
	m_pDataBuf = NULL;
	m_pHeader = NULL;
	m_dwIDCodeReal = 0;
#ifdef _WIN32
	RtlZeroMemory( &m_UserID, sizeof(m_UserID) );
#else
	bzero( &m_UserID, sizeof(m_UserID) );
#endif //_WIN32
}

CLicDecode::~CLicDecode()
{
}

void CLicDecode::Init(DVB_USER_ID & UserID)
{
	m_UserID = UserID;
	m_dwIDCodeReal = UserID.m_RcvID.m_adwID[0];
	LockIDCode();
	m_pDataBuf = NULL;
	m_pHeader = NULL;
}

//	���ƿ���
void CLicDecode::LockIDCode()
{	
	DWORD dwIDCode = m_dwIDCodeReal;	// V1.00, V1.01 ֻʹ�õ�4�ֽ�������Ȩ
	_asm{
		mov eax,dwIDCode
		ror eax,3
		mov dl,al
		mov dh,al
		shl edx,16
		mov dh,al
		xor eax,edx
		mov dwIDCode,eax
	}
	m_dwIDCode = dwIDCode;
}

//	���ƿ���
//	��ڲ���
//		dwIDCode					����
DWORD CLicDecode::UnlockIDCode(DWORD dwIDCode)
{
	_asm{
		mov eax,dwIDCode
		mov dl,al
		mov dh,dl
		shl edx,16
		mov dh,al
		xor eax,edx
		rol eax,3
		mov dwIDCode,eax
	}
	return dwIDCode;
}

//	��������
//	��ڲ���
//		pLicBuf					��Ȩ���ݻ�����
//	���ز���
//		TRUE					CRC У��ɹ�
//		FALSE					CRC32 У��ʧ��
BOOL CLicDecode::Attach(PBYTE pLicBuf)
{
	ASSERT( pLicBuf );
	m_pDataBuf = pLicBuf;
	m_pHeader = (PBLKHEADER)pLicBuf;
	if( m_pHeader->m_wVersion >= 0x102 )			//	Ŀǰֻ�ܽ��� 1.00 �� 1.01 �汾�ĸ�ʽ
		return FALSE;
	if( CCRC::GetCRC32( TSLOCKCODECLEN-sizeof(DWORD), pLicBuf+sizeof(DWORD) ) != m_pHeader->m_dwCRC32 )
	{
		Detach();
		return FALSE;
	}
	return TRUE;
}

//	�ͷ�����
void CLicDecode::Detach()
{
	m_pDataBuf = NULL;
	m_pHeader = NULL;
}

//	�Ƿ��иÿ��ŵ���Ȩ����
//	���ز���
//		FALSE					�϶�û������
//		TRUE					����������
BOOL CLicDecode::IsInRange()
{
	ASSERT( m_pHeader );
	DWORD dwIDCode = UnlockIDCode( m_pHeader->m_dwMinIDCode );
	if( dwIDCode > m_dwIDCodeReal )
		return FALSE;
	dwIDCode = UnlockIDCode( m_pHeader->m_dwMaxIDCode );
	if( dwIDCode < m_dwIDCodeReal )
		return FALSE;
	return TRUE;
}

//	��ȡϵͳ��������
DWORD CLicDecode::GetSysCodeIndex()
{
	ASSERT( m_pHeader );
	return m_pHeader->m_dwSysCodeIndex;
}

//	ȡ��һ����Ȩ����
//	���ز���
//		NULL					û����Ȩ����
//		���ݺŵ�ָ��
PDWORD CLicDecode::GetLicData()
{
register DWORD dwIDCode;
	ASSERT( m_pHeader );
	ASSERT( GetLicMethod() == LICOP_LIC );
	int nPos = sizeof(BLKHEADER);
	for(register int i=0; i<m_pHeader->m_wBlkNum && nPos<TSLOCKCODECLEN; i++)
	{									//	��ѭ��
		PSUBBLKHEAD pSubHeader = (PSUBBLKHEAD)(m_pDataBuf+nPos);
		if( pSubHeader->m_dwIDCode == m_dwIDCode )
			return & pSubHeader->m_dwLicCode;					//	�ҵ�
		nPos += sizeof(SUBBLKHEAD);
		int nUserNum = pSubHeader->m_byUserNum - 1;
		DWORD dwBaseIDCode = UnlockIDCode( pSubHeader->m_dwIDCode );
		switch( pSubHeader->m_bySubType )
		{
		case LICTYPE_CONTINUE:
			{
				dwIDCode = dwBaseIDCode;
				PSUBBLK_CONTINUE	pSC = (PSUBBLK_CONTINUE)( m_pDataBuf+nPos);
				for(register int j=0; j<nUserNum; j++)
				{
					dwIDCode ++;
					if( dwIDCode == m_dwIDCodeReal )
						return &pSC->m_dwLicCode;				//	�ҵ�
					nPos += sizeof(SUBBLK_CONTINUE);
					ASSERT( nPos<TSLOCKCODECLEN );
					pSC ++;
				}
			}
			break;

		case LICTYPE_LESS100:
			{
				PSUBBLK_LESS100		pSL1 = (PSUBBLK_LESS100)( m_pDataBuf+nPos);
				for(register int j=0; j<nUserNum; j++)
				{
					dwIDCode = pSL1->m_byIDCode + dwBaseIDCode;
					if( dwIDCode == m_dwIDCodeReal )
						return &pSL1->m_dwLicCode;
					nPos += sizeof(SUBBLK_LESS100);
					ASSERT( nPos<TSLOCKCODECLEN );
					pSL1 ++;
				}
			}
			break;

		case LICTYPE_LESS10000:
			{
				PSUBBLK_LESS10000	pSL2 = (PSUBBLK_LESS10000)( m_pDataBuf+nPos);
				for(register int j=0; j<nUserNum; j++)
				{
					dwIDCode = pSL2->m_wIDCode + dwBaseIDCode;
					if( dwIDCode == m_dwIDCodeReal )
						return &pSL2->m_dwLicCode;
					nPos += sizeof(SUBBLK_LESS10000);
					ASSERT( nPos<TSLOCKCODECLEN );
					pSL2 ++;
				}
			}
			break;

		case LICTYPE_INDEPENDENCE:
			{
				PSUBBLK_INDEPENDENCE pSI = (PSUBBLK_INDEPENDENCE)( m_pDataBuf+nPos);
				for(register int j=0; j<nUserNum; j++)
				{
					if( pSI->m_dwIDCode == m_dwIDCode )
						return &pSI->m_dwLicCode;
					pSI ++;
					nPos += sizeof(SUBBLK_INDEPENDENCE);
					ASSERT( nPos<TSLOCKCODECLEN );
				}
			}
			break;

		default:
			ASSERT( FALSE );
			return NULL;						//	ʧ��
		}
	}
	return NULL;
}


#ifdef __CYJ_TEST_LICCODE__
//	�б���
//	��ڲ���
//		pBuf				���������, ��С = 10K*sizeof(DWORD)
//	���ز���
//		���ļ���������Ȩ������
int CLicDecode::ListIDCode(PDWORD pBuf)
{
register DWORD dwIDCode;
int nNo = 0;
	ASSERT( m_pHeader );
	int nPos = sizeof(BLKHEADER);
	for(register int i=0; i<m_pHeader->m_wBlkNum && nPos<TSLOCKCODECLEN; i++)
	{									//	��ѭ��
		PBLKHEADER pHead = (PBLKHEADER)m_pDataBuf;
		if( pHead->m_byLicOpCode != LICOP_LIC )
			return 0;
		PSUBBLKHEAD pSubHeader = (PSUBBLKHEAD)(m_pDataBuf+nPos);
		nPos += sizeof(SUBBLKHEAD);
		int nUserNum = pSubHeader->m_byUserNum - 1;
		DWORD dwBaseIDCode = UnlockIDCode( pSubHeader->m_dwIDCode );
		pBuf[nNo++] = dwBaseIDCode;
		switch( pSubHeader->m_bySubType )
		{
		case LICTYPE_CONTINUE:
			{
				dwIDCode = dwBaseIDCode;
				PSUBBLK_CONTINUE	pSC = (PSUBBLK_CONTINUE)( m_pDataBuf+nPos);
				for(register int j=0; j<nUserNum; j++)
				{
					dwIDCode ++;
					pBuf[nNo++] = dwIDCode;
					nPos += sizeof(SUBBLK_CONTINUE);
					ASSERT( nPos<TSLOCKCODECLEN );
					pSC ++;
				}
			}
			break;

		case LICTYPE_LESS100:
			{
				PSUBBLK_LESS100		pSL1 = (PSUBBLK_LESS100)( m_pDataBuf+nPos);
				for(register int j=0; j<nUserNum; j++)
				{
					dwIDCode = pSL1->m_byIDCode + dwBaseIDCode;
					pBuf[nNo++] = dwIDCode;
					nPos += sizeof(SUBBLK_LESS100);
					ASSERT( nPos<TSLOCKCODECLEN );
					pSL1 ++;
				}
			}
			break;

		case LICTYPE_LESS10000:
			{
				PSUBBLK_LESS10000	pSL2 = (PSUBBLK_LESS10000)( m_pDataBuf+nPos);
				for(register int j=0; j<nUserNum; j++)
				{
					dwIDCode = pSL2->m_wIDCode + dwBaseIDCode;
					pBuf[nNo++] = dwIDCode;
					nPos += sizeof(SUBBLK_LESS10000);
					ASSERT( nPos<TSLOCKCODECLEN );
					pSL2 ++;
				}
			}
			break;

		case LICTYPE_INDEPENDENCE:
			{
				PSUBBLK_INDEPENDENCE pSI = (PSUBBLK_INDEPENDENCE)( m_pDataBuf+nPos);
				for(register int j=0; j<nUserNum; j++)
				{
					pBuf[nNo++] = UnlockIDCode( pSI->m_dwIDCode );
					pSI ++;
					nPos += sizeof(SUBBLK_INDEPENDENCE);
					ASSERT( nPos<TSLOCKCODECLEN );
				}
			}
			break;

		default:
			ASSERT( FALSE );
			return 0;						//	ʧ��
		}
	}
	return nNo;
}

#endif // __CYJ_TEST_LICCODE__

//	��ȡ��Ȩ����
//	��:		���,	ɾ��
LICOP_METHOD CLicDecode::GetLicMethod()
{
	LICOP_METHOD opcode = (LICOP_METHOD)m_pHeader->m_byLicOpCode;
	return opcode;
}

//	�Ƿ��ɾ���öκ�
//	���ز���
//		TRUE					��ɾ��
//		FALSE					��ɾ��
BOOL CLicDecode::IsToDelete()
{
	ASSERT( m_pHeader );
	ASSERT( GetLicMethod() == LICOP_DEL );
	register PDWORD pdwCloseID = (PDWORD)( m_pDataBuf + sizeof(BLKHEADER) );
	int nCount = m_pHeader->m_wBlkNum;
	for(register int i=0; i<nCount; i++)
	{
		if( *pdwCloseID == m_dwIDCode )
			return TRUE;
		pdwCloseID ++;
	}
	return FALSE;
}

//	��ȡ�汾��
WORD CLicDecode::GetVersion()
{
	ASSERT( m_pHeader );
	if( m_pHeader == NULL )
		return 0;
	return m_pHeader->m_wVersion;
}
