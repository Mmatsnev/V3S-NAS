///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2007-1-17
///
///		��;��
///			ͨ�� VOD �㲥Զ�̿��Ʊ�
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӡ��ڲ�ʹ��!	  			     !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	��Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!			 !
///=======================================================

// TSVODPSI_RemoteCtrlTblGenerator.h: interface for the CTSVODPSI_RemoteCtrlTblGenerator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSVODPSI_REMOTECTRLTBLGENERATOR_H__5D60638D_17A7_4AAB_8110_3E43A2C55975__INCLUDED_)
#define AFX_TSVODPSI_REMOTECTRLTBLGENERATOR_H__5D60638D_17A7_4AAB_8110_3E43A2C55975__INCLUDED_

#include "tsvodpsi_tablegenerator.h"

#ifndef MY_LONG64
	#ifdef _WIN32
		typedef __int64	MY_LONG64;
	#else
		typedef long long MY_LONG64;
	#endif //_WIN32
#endif // MY_LONG64

#pragma pack(push,4)

class CTSVODPSI_RemoteCtrlTblGenerator : public CTSVODPSI_TableGeneratorBase
{
public:
	CTSVODPSI_RemoteCtrlTblGenerator(BYTE byTableID, bool bDoCompress = false);
	virtual ~CTSVODPSI_RemoteCtrlTblGenerator();

	virtual PBYTE GetPrivateData( int & nOutLen );

	void Initialize();
	void SetSID( WORD wSID );
	void SetSTBID( MY_LONG64 llSTBID, MY_LONG64 llEndSTBID = 0);
	void SetSTBID( BYTE abySTBID[8] );
	void SetEncryptParameter( WORD wParameter );
	void CleanInstructions();
	int AddIns_SwitchChannel( BYTE byChNo );					// �л�Ƶ��
	int AddIns_ReceiveProgram( BYTE byPhysNo, WORD wPMT_PID );	// �տ���Ŀ
	int AddIns_VODOperatorResponse( PBYTE pBuf, BYTE byDataLen );	// VOD �㲥����ֵ
	int AddIns_PrecompiledInstructions( WORD wCommand, PBYTE pBuf, int nLen );

	enum{
		BUFSIZE_FOR_TBL_HEADER = 100,	// ���� 100 �ֽ����ڱ�ͷ
	};

	enum
	{
		RCMDID_SWITCH_CHANNEL = 1,				// �л�Ƶ��
		RCMDID_OSD_SHOW_TEXT,					// ��ʾ����
		RCMDID_RECEIVE_PROGRAM,					// ���ս�Ŀ
		RCMDID_RECOMMEND_PROGRAM,				// �Ƽ���Ŀ
		RCMDID_VOD_OPERATION_RESPONSE,			// �㲥����ֵ
	};
	
private:
	WORD	m_wSID;					// ��� != 0 �����ʾ�ñ������һ��SID�ģ���֮�����һ��STB ID
	
	MY_LONG64	m_llSTBID;
	MY_LONG64  m_llEndSTBID;
	
	PBYTE	m_pDataBuf;
	int		m_nBufSize;
	int		m_nDataLen;

	int		m_nInstructionCount;
	DWORD	m_dwEncryptParameter;

private:
	bool	AcquireMem( int nIncBytes );
};

#pragma pack(pop)

#endif // !defined(AFX_TSVODPSI_REMOTECTRLTBLGENERATOR_H__5D60638D_17A7_4AAB_8110_3E43A2C55975__INCLUDED_)
