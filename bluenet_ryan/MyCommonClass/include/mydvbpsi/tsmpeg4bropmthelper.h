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

// TSMPEG4BroPMTHelper.h: interface for the CTSMPEG4BroPMTHelper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSMPEG4BROPMTHELPER_H__CE08827C_F899_46FB_992E_681EFA5F1C7D__INCLUDED_)
#define AFX_TSMPEG4BROPMTHELPER_H__CE08827C_F899_46FB_992E_681EFA5F1C7D__INCLUDED_

#include "dvbpsitablesdefine.h"
#include "dvbdescriptorsdefine.h"

#pragma pack(push,4)

class CTSMPEG4BroPMTHelper  
{
public:
	static int GetTongshiMPEG4BroFormat( const PDVB_PSI_TABLE_PMT pTable );
	static DVBPSI_TSVB_MY_VIDEO_DESCRIPTOR * GetVideoDr( const PDVB_PSI_TABLE_PMT pTable, WORD & wPID );
	static DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR * GetAudioDr( const PDVB_PSI_TABLE_PMT pTable, WORD & wPID, DWORD dwLanguage = 0 );
private:
	static DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR * MatchAudioDr(const PDVB_PSI_TABLE_PMT pTable, int nProgramNo, DWORD dwLanguage );
};

#pragma pack(pop)

#endif // !defined(AFX_TSMPEG4BROPMTHELPER_H__CE08827C_F899_46FB_992E_681EFA5F1C7D__INCLUDED_)
