///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2005-2-17
///
///		��;��
///			ͨ��MPEG4���������࣬�����ж��Ƿ�ͨ�Ӳ���
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӡ��ڲ�ʹ��!	  			     !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	��Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!			 !
///=======================================================

#include "stdafx.h"
#include "tsmpeg4bropmthelper.h"
#include "tspacket.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///-------------------------------------------------------
/// CYJ,2005-2-17
/// ��������:
///		�ж��Ƿ�ͨ�Ӳ���
/// �������:
///		pTable				PMT ��
/// ���ز���:
///		>=0					�ɹ� 
///		<0					ʧ��
int CTSMPEG4BroPMTHelper::GetTongshiMPEG4BroFormat( const PDVB_PSI_TABLE_PMT pTable )
{
	if( 0 == pTable->m_wCount || NULL == pTable->m_pDescriptor )
		return -1;
	PDVBPSI_DECODED_DESCRIPTOR_BASE pDr = pTable->m_pDescriptor->Find( DVBPSI_DTID_TSVB_IDENTITY );
	if( NULL == pDr )
		return -1;
	DVBPSI_TSVB_IDITENTITY_DESCRIPTOR * pTSIDDr = (DVBPSI_TSVB_IDITENTITY_DESCRIPTOR*)pDr;
	if( pTSIDDr->m_dwFourCC != TONGSHI_VIDEO_BRO_FOURCC )
		return -1;
	return pTSIDDr->m_wFormatTag;
}


///-------------------------------------------------------
/// CYJ,2005-2-17
/// ��������:
///		��ȡ��Ƶ��Ŀ������
/// �������:
///		pTable			PMT ��
///		wPID			���PID
/// ���ز���:
///		NULL			ʧ��
///		����			��Ƶ������
DVBPSI_TSVB_MY_VIDEO_DESCRIPTOR * CTSMPEG4BroPMTHelper::GetVideoDr( const PDVB_PSI_TABLE_PMT pTable, WORD & wPID )
{
	ASSERT( GetTongshiMPEG4BroFormat(pTable) >= 0 );
	wPID = INVALID_PID;
	
	if( 0 == pTable->m_wStreamCount )
		return NULL;
	int nNo = pTable->FindStream( 0xA0 );			//	�Զ��岥��
	if( nNo < 0 )
		return NULL;

	PDVBPSI_DECODED_DESCRIPTOR_BASE pDr = pTable->m_aStreams[nNo].m_pDescriptor->Find( DVBPSI_DTID_TSVB_MY_VIDEO );
	if( NULL == pDr )
		return NULL;
	wPID = pTable->m_aStreams[nNo].m_wES_PID;
	return (DVBPSI_TSVB_MY_VIDEO_DESCRIPTOR*)pDr;
}

///-------------------------------------------------------
/// CYJ,2005-2-17
/// ��������:
///		��ȡ��Ƶ��Ŀ������
/// �������:
///		pTable			PMT ��
///		wPID			���PID
///		dwLanguage		ƥ������ԣ�ȱʡΪ0����ʾ�������ж�
/// ���ز���:
///		NULL			ʧ��
///		����			��Ƶ������
/// ˵����
///		PID ��������Ƶ��ͬ������ͬ��������һ��PES������
DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR * CTSMPEG4BroPMTHelper::GetAudioDr( const PDVB_PSI_TABLE_PMT pTable, WORD & wPID, DWORD dwLanguage )
{
	ASSERT( GetTongshiMPEG4BroFormat(pTable) >= 0 );
	wPID = INVALID_PID;

	if( 0 == pTable->m_wStreamCount )
		return NULL;

	DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR * pRetVal = NULL;

	int nNo = pTable->FindStream( 0xA0 );				// ���ж��Ƿ���ͬһ��TS������
	if( nNo >= 0 )										// 2 ������Ƶ����
	{
		pRetVal = MatchAudioDr( pTable, nNo, dwLanguage );
		if( pRetVal )
		{
			wPID = pTable->m_aStreams[nNo].m_wES_PID;
			return pRetVal;
		}
	}
	
	for(int i=0; i<pTable->m_wStreamCount; i++ )
	{
		if( pTable->m_aStreams[i].m_byStreamType != 0xA1 )		// 0xA1 ������Ƶ����
			continue;
		pRetVal = MatchAudioDr( pTable, i, dwLanguage );
		if( NULL == pRetVal )
			continue;
		wPID = pTable->m_aStreams[i].m_wES_PID;
		return pRetVal;
	}

	return NULL;
}

///-------------------------------------------------------
/// CYJ,2005-2-17
/// ��������:
///		ƥ����Ƶ
/// �������:
///		pTable			PMT ��
///		nProgramNo		m_aProgram �����±�
///		dwLanguage		���ԣ���Ϊ 0�����ʾ�������Ե�����
/// ���ز���:
///		NULL			ʧ��
///		����			������
DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR * CTSMPEG4BroPMTHelper::MatchAudioDr(const PDVB_PSI_TABLE_PMT pTable, int nProgramNo, DWORD dwLanguage )
{
	ASSERT( pTable && nProgramNo >= 0 );
	PDVBPSI_DECODED_DESCRIPTOR_BASE pDr = pTable->m_aStreams[nProgramNo].m_pDescriptor;
	while( pDr )
	{
		if( pDr->m_byDescriptorTag == DVBPSI_DTID_TSVB_MY_AUDIO )
		{
			DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR * pAudioDr = (DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR*)pDr;
			if( 0 == dwLanguage || dwLanguage == pAudioDr->m_dwISOLanguageID )
				return pAudioDr;
		}
		pDr = pDr->m_pNext;
	}
	return NULL;
}
