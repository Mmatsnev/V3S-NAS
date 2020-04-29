///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2006-10-9
///
///		��;��
///			UDP/IP Over DVB ����
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӡ��ڲ�ʹ��!	  			     !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	��Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!			 !
///=======================================================

// DVBPSI_IP_MPE_Generator.h: interface for the CDVBPSI_IP_MPE_Generator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DVBPSI_IP_MPE_GENERATOR_H__DBCF0883_B984_4176_A0FA_D6D547F7D778__INCLUDED_)
#define AFX_DVBPSI_IP_MPE_GENERATOR_H__DBCF0883_B984_4176_A0FA_D6D547F7D778__INCLUDED_

#include "psitablegenerator.h"

#pragma pack(push,4)

class CDVBPSI_IP_MPE_Generator : public CDVBPSI_TableGeneratorBase  
{
public:
	int Build( PBYTE pBuf, int nLen );
	CDVBPSI_IP_MPE_Generator( bool bChecksumIsCRC32 = false );
	virtual ~CDVBPSI_IP_MPE_Generator();

	enum
	{
		TSDVB_IP_MPE_SECTION_MAX_SIZE = 1600,		// ���ĳ���
	};

	BYTE	m_abySectionBuf[TSDVB_IP_MPE_SECTION_MAX_SIZE];
	bool	m_bChecksumIsCRC32;					// true - CRC32 ; false - check sum
};

#pragma pack(pop)

#endif // !defined(AFX_DVBPSI_IP_MPE_GENERATOR_H__DBCF0883_B984_4176_A0FA_D6D547F7D778__INCLUDED_)
