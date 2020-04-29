// DVBEPGReceiver.cpp: implementation of the CDVBEPGReceiver class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IPRecSvr.h"
#include "DVBEPGReceiver.h"
#ifndef _WIN32
	#include <MyArchive.h>
	#include <MyMemFile.h>
#endif //_WIN32

#ifdef _WIN32
	#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
	#endif
#endif //


#ifdef _WIN32
	extern "C" IDVB_EPG_Receiver * WINAPI Create_DVB_EPG_Receiver(void)
#else
	extern "C" IDVB_EPG_Receiver * Create_DVB_EPG_Receiver(void)
#endif //_WIN32
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	CDVBEPGReceiver * pInstance = new CDVBEPGReceiver;
	if( NULL == pInstance )
		return NULL;
	pInstance->AddRef();

	return static_cast<IDVB_EPG_Receiver*>(pInstance);
}



class COneChannelDefSetting
{
public:
	COneChannelDefSetting();
	virtual void Serialize( CArchive& ar );
	COneChannelDefSetting & operator =(const COneChannelDefSetting & src);
	void ToXML( CString & strOutput );

public:
	CString	m_strName;							//	����
	CString	m_strChineseName;					//	��������
	CString	m_strIDFile;						//	�����ļ���
	CString	m_strDefHomePage;					//	Ĭ����ҳ
	CString	m_strDefSavePath;					//	����·��
	CString	m_strHostIP;						//	�ಥ IP
	int		m_nHostSocketPort;						//	�ಥ�˿�
	DWORD	m_dwPID;								//	PID
	CString	m_strPostCode;						//  2003-10-14 ���õ����ʱ�
};

COneChannelDefSetting::COneChannelDefSetting()
{

}

void COneChannelDefSetting::Serialize( CArchive& ar )
{
	CString strTmp;
	int nCount = 1;								//  2003-10-14 �����ˣ���� PostCode ����
	if( ar.IsStoring() )
	{
		ar << m_strName;						//	����
		ar << m_strChineseName;					//	��������
		ar << m_strIDFile;						//	�����ļ���
		ar << m_strDefHomePage;					//	Ĭ����ҳ
		ar << m_strDefSavePath;					//	����·��
		ar << m_strHostIP;						//	�ಥ IP
		ar << m_nHostSocketPort;				//	�ಥ�˿�
		ar << m_dwPID;							//	PID
		ar << nCount;							//	û����������
		//	���䲿�֣����밴�ַ�����ʽ
		ar << m_strPostCode;					//  2003-10-14 �������һ���ʱ�
	}
	else
	{
		ar >> m_strName;						//	����
		ar >> m_strChineseName;					//	��������
		ar >> m_strIDFile;						//	�����ļ���
		ar >> m_strDefHomePage;					//	Ĭ����ҳ
		ar >> m_strDefSavePath;					//	����·��
		ar >> m_strHostIP;						//	�ಥ IP
		ar >> m_nHostSocketPort;				//	�ಥ�˿�
		ar >> m_dwPID;							//	PID
		ar >> nCount;							//	û����������
		//	���䲿�֣����밴�ַ�����ʽ
		if( nCount >= 1 )
			ar >> m_strPostCode;				//  2003-10-14 �����ˣ�������Ҫ�ж��Ƿ񳬹� 1
		//	���Ӳ��֣����밴�ַ�����ʽ
		for(int i=1; i<nCount; i++)
		{										//	Ϊ�˼��ݣ�����ն�
			ar >> strTmp;
		}

#ifdef _DEBUG
	printf("Name=%s, HomePage=%s, IP=%s, Port=%d\n", (LPCSTR)m_strName,
		(LPCSTR)m_strDefHomePage, (LPCSTR)m_strHostIP, m_nHostSocketPort );
#endif //_DEBUG

	}
}

//-------------------------------------------
//	�޸ļ�¼��
//  2003-10-14 ��� m_strPostCode
COneChannelDefSetting & COneChannelDefSetting::operator =(const COneChannelDefSetting & src)
{
	m_strName = src.m_strName;							//	����
	m_strChineseName = src.m_strChineseName;			//	��������
	m_strIDFile = src.m_strIDFile;						//	�����ļ���
	m_strDefHomePage = src.m_strDefHomePage;			//	Ĭ����ҳ
	m_strDefSavePath = src.m_strDefSavePath;			//	����·��
	m_strHostIP = src.m_strHostIP;						//	�ಥ IP
	m_nHostSocketPort = src.m_nHostSocketPort;			//	�ಥ�˿�
	m_dwPID = src.m_dwPID;								//	PID
	m_strPostCode = src.m_strPostCode;					//  2003-10-14 PostCode
	return *this;
}

///-------------------------------------------------------
/// CYJ,2004-5-17
/// Function:
///		output the paramter to EPG in XML format
/// Input parameter:
///		None
/// Output parameter:
///		None
///	Note:
///		The channel No is < 0 to indicate this is a older channel
///		VendorID=0		-> tongshi
void COneChannelDefSetting::ToXML(CString & strOutput)
{
	strOutput.Format(
"  <CHANNEL>\n<CHANNEL_ID>%d</CHANNEL_ID>\n\
        <NAME>%s</NAME>\n\
        <MULTICAST_IP>%s</MULTICAST_IP>\n\
        <MULTICAST_PORT>%d</MULTICAST_PORT>\n\
        <BRO_VENDOR_ID>0</BRO_VENDOR_ID>\n\
        <POSTCODES>%s</POSTCODES>\n\
        <ENCRYPT_PARAM>0</ENCRYPT_PARAM>\n\
        <PID>0x%X</PID>\n\
        <DEFAULT_HOME_PAGE>%s</DEFAULT_HOME_PAGE>\n\
  </CHANNEL>\n",\
		-m_nHostSocketPort,	(LPCSTR)m_strName, (LPCSTR)m_strHostIP, m_nHostSocketPort,\
		(LPCSTR)m_strPostCode, m_dwPID, (LPCSTR)m_strDefHomePage );
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDVBEPGReceiver::CDVBEPGReceiver()
{
	m_bEnableOldEPG = true;
}

CDVBEPGReceiver::~CDVBEPGReceiver()
{

}

///-------------------------------------------------------
/// CYJ,2004-5-17
/// Function:
///		Initialize receiver
/// Input parameter:
///		None
/// Output parameter:
///		None
BOOL CDVBEPGReceiver::Init(bool bSaveFileInBackground, int varMaxPacketSize)
{
fprintf( stderr, "CDVBEPGReceiver::Init\n");

	m_bEnableOldEPG = true;
	m_mapFileEPG.RemoveAll();

	return CDVBFileReceiver::Init( bSaveFileInBackground, varMaxPacketSize );
}

DWORD CDVBEPGReceiver::QueryInterface( REFIID iid,void ** ppvObject)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( IID_IDVBEPGReceiver == iid )
	{
		AddRef();
		*ppvObject = static_cast<IDVB_EPG_Receiver*>(this);
		return 0;	//	S_OK;
	}

	return CDVBFileReceiver::QueryInterface( iid, ppvObject );
}

///-------------------------------------------------------
/// CYJ,2004-5-17
/// Function:
///		Get if enable old EPG format
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CDVBEPGReceiver::GetEnableOldEPG()
{
	return m_bEnableOldEPG;
}

///-------------------------------------------------------
/// CYJ,2004-5-17
/// Function:
///		set enable older format
/// Input parameter:
///		bNewValue			enable or not
/// Output parameter:
///		None
void CDVBEPGReceiver::SetEnableOldEPG( bool bNewValue )
{
	m_bEnableOldEPG = bNewValue;
}

///-------------------------------------------------------
/// CYJ,2004-5-17
/// Function:
///		on one EPG file ok
/// Input parameter:
///		None
/// Output parameter:
///		None
void CDVBEPGReceiver::Fire_OnFileOK( IFileObject * pObject, HDATAPORT hDataPort )
{
	if( NULL == m_pFileEventObject )		//	the File OK Event is not registerd
		return;
	ASSERT( pObject );
	if( NULL == pObject )
		return;
	if( pObject->GetDataLen() < 10 )		//	������ļ����ȣ��϶�����EPG
		return;
	PBYTE pBuf = pObject->GetBuffer();
	DWORD dwTagID = *(PDWORD)pBuf;
	if( dwTagID == 0x12345678 )
	{										// Older EPG tag id
		if( false == m_bEnableOldEPG )
			return;							//  the older EPG is disabled
		if( false == TransferToXMLFormat( pObject ) )
			return;
	}
	CDVBFileReceiver::Fire_OnFileOK( pObject, hDataPort );
}

///-------------------------------------------------------
/// CYJ,2004-5-17
/// Function:
///		On sub file OK, do nothing here
/// Input parameter:
///		None
/// Output parameter:
///		None
void CDVBEPGReceiver::Fire_OnSubFileOK( IFileObject * pObject, HDATAPORT hDataPort  )
{
	//	EPG �����д��ļ���ʽ
}

///-------------------------------------------------------
/// CYJ,2004-5-17
/// Function:
///
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CDVBEPGReceiver::TransferToXMLFormat(IFileObject *pObject)
{
	CMemFile fSrc;
	CString strTmpXML;
	CString strXML = "<?xml version=\"1.0\" encoding=\"gb2312\"?>\n<EPG>\n";
	try
	{
		fSrc.Attach( pObject->GetBuffer(), pObject->GetDataLen() );

		CArchive loadArchive(&fSrc, CArchive::load|CArchive::bNoFlushOnDelete  );
#ifdef _WIN32
		loadArchive.m_bForceFlat = FALSE;
#endif //_WIN32

		DWORD dwIDFlags;
		loadArchive >> dwIDFlags;
		if( dwIDFlags != 0x12345678 )
		{
			fSrc.Detach();
			return false;
		}
		DWORD dwBroID;
		time_t	SectionBuildTime;
		loadArchive >> dwBroID;				//	��ʶ��������һ����IP��ַ
		loadArchive >> SectionBuildTime;	//	�仯ʱ��

		time_t & LastTime = m_mapFileEPG[ dwBroID ];
		if( LastTime == SectionBuildTime )
		{
			fSrc.Detach();					//	û�б仯
			return false;
		}
		LastTime = SectionBuildTime;		//	��¼�仯

		WORD	wExtLen;					//	������ȡ��������
		loadArchive >> wExtLen;				//	�����ֽ���
		int i;
		for(	i=0; i<wExtLen; i++)
		{
			BYTE byTmp;
			loadArchive >> byTmp;			//	����
		}

		int nCount;
		loadArchive >> nCount;				//	�������
		for(i=0; i<nCount; i++)
		{
			COneChannelDefSetting one;
			one.Serialize( loadArchive );
			one.ToXML( strTmpXML );
			strXML += strTmpXML;
		}
		loadArchive.Close();

		strXML += "</EPG>";
		pObject->SetBufSize( strXML.GetLength() + 1 );
		pObject->PutDataLen( strXML.GetLength() );
		memcpy( pObject->GetBuffer(), strXML, strXML.GetLength()+1 );
	}
	catch( ... )
	{
		return false;
	}
	return true;
}
