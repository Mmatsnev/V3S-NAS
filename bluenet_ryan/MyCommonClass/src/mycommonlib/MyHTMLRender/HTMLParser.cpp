///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2004-11-22
///
///		��;��
///			HTML ��������ʾ��
///=======================================================

#include "stdafx.h"
#include "HTMLParser.h"
#include "HTMLTokenizer.h"
#include "MyCompoundFileObj.h"
#include <mylibhttp/myhttpfile.h>
#include <stdlib.h>

#ifndef _WIN32
	#include <MyFile.h>
#endif //_WIN32

#if defined(_DEBUG) && defined(_WIN32)
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern "C" char *b_strupper( char* );

class CMyMemBufferManager
{
public:
	CMyMemBufferManager()
	{
		m_nBufLen = 0;
		m_pBuffer = NULL;
		m_bAutoFreeBuf = false;
	}
	~CMyMemBufferManager()
	{
		Invalid();		
	}
	void Invalid()
	{
		if( m_bAutoFreeBuf )
			delete m_pBuffer;
		m_nBufLen = 0;
		m_pBuffer = NULL;
		m_bAutoFreeBuf = false;
	}
	operator PBYTE() const { return m_pBuffer; }
	operator int() const { return m_nBufLen; }
public:
	int		m_nBufLen;
	PBYTE	m_pBuffer;
	bool	m_bAutoFreeBuf;
};

static CMyString DecodeURL( LPCSTR lpszURL )
{
	CMyString strDecoded = lpszURL;

	strDecoded.Replace( '+', ' ' );

	// first see if there are any %s to decode....
	if( strDecoded.Find( '%' ) < 0 )
		return strDecoded;
	
	char * pszSrc;
	char * pszDst;
	int nLen = strDecoded.GetLength();
	pszSrc = strDecoded.GetBuffer( nLen );
	pszDst = pszSrc;
	// iterate through the string, changing %dd to special char....
	while( *pszSrc )
	{
		char ch = *pszSrc ++;
		if ( ch == '%' )
		{
			if ( *pszSrc == '%' )
			{								//	����Ϊ�˱��� %
				*pszDst = '%';				//	%
				pszSrc ++;
				pszDst ++;
				nLen --;					//	���� 1 ���ַ��ı���	, %% ==> %
				continue;
			}
			else
			{
				WORD wWideChar = 0;
				BYTE ch1 = *pszSrc++;
				BYTE ch2 = *pszSrc++;
				ch1 = (ch1 >= 'A') ? ((ch1&0xdf)-'A'+0xa) : (ch1-'0');
				ch2 = (ch2 >= 'A') ? ((ch2&0xdf)-'A'+0xa) : (ch2-'0');	
				*pszDst ++ = ch1*16 + ch2;
				nLen -= 2;					
			}
		}
		else
		{
			*pszDst = ch;				//	��ͨ������
			pszDst ++;
		}
	}
	strDecoded.ReleaseBuffer( nLen );	//	�ͷŻ�����

	return strDecoded;
}

///////////////////////////////////////////////////////////////////////
// CMyRequest
CMyRequest::CMyRequest( BSModule * pModule, const char * lpszFunctionName )
	:CBSExternFunctionTemplete( pModule, lpszFunctionName )
{
	m_pszSrcRequest = NULL;
}

CMyRequest::~CMyRequest()
{
	if( m_pszSrcRequest )
		free( (void*)m_pszSrcRequest );
}

void CMyRequest::OnFunctionCall()
{
	if( GetArgCount() < 1 )
		return;
	if( IsArgTypeString(0) )
	{	
		CBSAutoReleaseString strName = GetArgAsString( 0 );
		if( NULL == strName.m_pString || 0 == *strName.m_pString )
			SetRetValue( "" );
		else
			SetRetValue( m_mapRequest[strName.m_pString] );
	}
	else
	{					// as number
		int nIndex = GetArgAsInt( 0 );
		if( nIndex < 0 || nIndex >= m_astrRequest.GetSize() )	
			SetRetValue( "" );
		else
			SetRetValue( m_astrRequest[nIndex] );
	}
}

void CMyRequest::SafeDelete()
{
	delete this;
}

void CMyRequest::SetRequst( LPCSTR lpszRequest )
{
	m_mapRequest.RemoveAll();
	m_astrRequest.RemoveAll();
	if( NULL == lpszRequest || 0 == *lpszRequest )
		return;	
	if( m_pszSrcRequest )
		free( (void*)m_pszSrcRequest );
	CMyString strTmp = DecodeURL( lpszRequest );
	m_pszSrcRequest = strdup( (char*)strTmp );
	if( NULL == m_pszSrcRequest )
		return;
	int nLen = strlen( m_pszSrcRequest);

	char * pszEnd;
	char * pszHead = m_pszSrcRequest;
	while( pszHead && nLen )
	{
		pszEnd = NULL;
		for(int i=0; i<nLen; i++ )
		{
			if( pszHead[i] == '&' )
			{
				pszEnd = pszHead + i;
				break;					 
			}
		}
		if( pszEnd )
			*pszEnd ++ = 0;
		nLen -= (pszEnd - pszHead);
		char * pszSplit = strchr( pszHead, '=' );
		if( pszSplit == NULL )
		{
			pszHead = pszEnd;
			continue;							//	����ĸ�ʽ
		}
		*pszSplit ++ = 0;
		b_strupper( pszHead );
		m_mapRequest[pszHead] = pszSplit;
		LPCSTR lpszTTT = m_mapRequest[pszHead];
		m_astrRequest.Add( pszSplit );
		pszHead = pszEnd;
	}
}
	
///////////////////////////////////////////////////////////////////////
// CMyResponseWrite 
CMyResponseWrite::CMyResponseWrite( BSModule * pModule, const char * lpszFunctionName, CMyString * pOutString )
	:CBSExternFunctionTemplete( pModule, lpszFunctionName )
{
	m_pOutString = pOutString;
}

// ResponseWrite
void CMyResponseWrite::OnFunctionCall()
{
	if( NULL == m_pOutString )
		return;
	if( GetArgCount() < 1  )
		return;
	CBSAutoReleaseString strText = GetArgAsString( 0 );
	if( NULL == strText.m_pString || 0 == strText.m_pString )
		return;
	*m_pOutString += strText.m_pString;
}

void CMyResponseWrite::SafeDelete()
{
	delete this;
}

///////////////////////////////////////////////////////////////////////
// CMyResponseWriteBlock
CMyResponseWriteBlock::CMyResponseWriteBlock( BSModule * pModule, const char * lpszFunctionName, CMyStringArray * pArray, CMyString * pOutString )
	:CBSExternFunctionTemplete( pModule, lpszFunctionName )
{
	m_pBlockString = pArray;
	m_pOutString = pOutString;
}

// ResponseWriteBlock
void CMyResponseWriteBlock::OnFunctionCall()
{
	if( NULL == m_pBlockString )
		return;
	if( GetArgCount() < 1  )
		return;
	int nIndex = GetArgAsInt( 0 );
	if( nIndex < 0 || nIndex >= m_pBlockString->GetSize() )
		return;
	*m_pOutString += m_pBlockString->ElementAt( nIndex );
}

void CMyResponseWriteBlock::SafeDelete()
{
	delete this;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHTMLParser::CHTMLParser(CDC * pDC)
	: m_HTMLRootElement( NULL )
{
	m_pCurrentParent = &m_HTMLRootElement;
	m_nActiveLinkIndex = 0;
	m_bIsLocalFile = TRUE;
	m_pDC = pDC;
	
	m_pRequestObj = NULL;
	m_pWriteObj = NULL;
	m_pWriteBlockObj = NULL;
	m_pfnOnModuleCreateDelete = NULL;
}

CHTMLParser::~CHTMLParser()
{
	DeleteAllElements();

	ASSERT( m_pRequestObj == NULL );
	ASSERT( m_pWriteBlockObj == NULL );
	ASSERT( m_pWriteObj == NULL );
}

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		�����ļ�
/// Input parameter:
///		pSrcFile				Դ�ļ�
///		lpszStartFileName		ԭʼ�ļ���
/// Output parameter:
///		None
bool CHTMLParser::Parse( CMyCompoundFileObj * pSrcFile, LPCSTR lpszStartFileName )
{	
	CMyString strNewStartFileName = SetRequestString( lpszStartFileName );
	lpszStartFileName = strNewStartFileName;
	
	if( NULL == lpszStartFileName )
		lpszStartFileName = "INDEX.HTM";
	m_pSrcFile = pSrcFile;
	m_strBaseURL = "";

	PONE_COMPOUND_FILE pFileData = pSrcFile->Find( lpszStartFileName );
	if( NULL == pFileData )
		return false;

	m_pCurrentElement = &m_HTMLRootElement;	
	m_aActionElements.RemoveAll();
	DeleteAllElements();
	m_HTMLRootElement.Preset();
	m_nActiveLinkIndex = 0;

	CMyString strTmp = lpszStartFileName;
	int nPos = strTmp.Find( '.' );
	if( nPos > 0 )
		strTmp = strTmp.Mid( nPos+1 );
	else
		strTmp = "";
	strTmp.MakeUpper();
	if( strTmp.Find("HTM") >= 0 || strTmp.Find("TXT") >= 0 )
		return Parse( (LPCSTR)pFileData->m_pDataBuf, pFileData->m_nFileLen );
	else
	{								//	ͼ���ļ�
		strTmp.Format("src=%s", lpszStartFileName );
		CreateInstance( HTML_TAG_IMAGE, strTmp );
		DoSpecialSetting( HTML_TAG_IMAGE );		
		return true;
	}		
}

///-------------------------------------------------------
/// CYJ,2004-12-6
/// Function:
///		����һ����������ͬʱ���ø����ĵ�����
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CHTMLParser::Parse( CMyCompoundFileObj * pSrcFile, LPCSTR lpszHTMLBuf, int nLen, LPCSTR lpszRequest )
{
	SetRequestString( lpszRequest );

	m_pSrcFile = pSrcFile;
	m_strBaseURL = "";

	m_pCurrentElement = &m_HTMLRootElement;	
	m_aActionElements.RemoveAll();
	DeleteAllElements();
	m_HTMLRootElement.Preset();
	m_nActiveLinkIndex = 0;

	return Parse( lpszHTMLBuf, nLen );
}

///-------------------------------------------------------
/// CYJ,2004-12-6
/// Function:
///		����ʵ����ҳ�����Ǹ����ĵ�
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CHTMLParser::Parse( LPCSTR lpszStartFileName )
{
	CMyString strNewStartFileName = SetRequestString( lpszStartFileName );
	lpszStartFileName = strNewStartFileName;

	m_pSrcFile = NULL;

	m_pCurrentElement = &m_HTMLRootElement;	
	m_aActionElements.RemoveAll();
	DeleteAllElements();
	m_HTMLRootElement.Preset();
	m_nActiveLinkIndex = 0;
	
	m_strBaseURL = lpszStartFileName;
	CMyString strRefFileName = m_strBaseURL;
	int nPos = m_strBaseURL.ReverseFind( '\\' );
	if( nPos < 0 )
		nPos = m_strBaseURL.ReverseFind( '/' );
	if( nPos >= 0 )
	{
		strRefFileName = m_strBaseURL.Mid( nPos+1 );
		m_strBaseURL.ReleaseBuffer( nPos+1 );
	}
	else
	{
		strRefFileName = m_strBaseURL;
		m_strBaseURL = "";
	}
	
	// http://
	if( 0 == m_strBaseURL.Left(7).CompareNoCase( "http://" ) )
		m_bIsLocalFile = FALSE;
	else
	{
		if( 0 == m_strBaseURL.Left(7).CompareNoCase( "file://" ) )
			m_strBaseURL.Delete( 0, 7 );
		m_bIsLocalFile = TRUE;
	}

	CMyString strTmp = strRefFileName;
	nPos = strTmp.Find( '.' );
	if( nPos > 0 )
		strTmp = strTmp.Mid( nPos+1 );
	else
		strTmp = "";
	strTmp.MakeUpper();

	if( strTmp.Find("HTM") < 0 && strTmp.Find("TXT") < 0 )
	{						//	ͼ�Ρ�ͼ��
		strTmp.Format("src=%s", (char*)strRefFileName );
		CreateInstance( HTML_TAG_IMAGE, strTmp );
		DoSpecialSetting( HTML_TAG_IMAGE );		
		return true;
	}

	CMyMemBufferManager MemHelper;
	if( false == ReadFile( strRefFileName, MemHelper ) )
		return false;
	
	return Parse( (LPCSTR)MemHelper.m_pBuffer, MemHelper.m_nBufLen );
}

///-------------------------------------------------------
/// CYJ,2004-11-22
/// Function:
///		���� HTML ��ҳ
/// Input parameter:
///		lpszHTMLContent			����ʾ����ҳ����
///		nLen					�ı�����
/// Output parameter:
///		true					�ɹ�
///		false					ʧ��
/// Note:
///		���� nLen < strlen(lpszHTMLContent)����ʱ��ֻ������ nLen Ϊֹ
bool CHTMLParser::Parse(LPCSTR lpszHTMLContent, int nLen)
{
	CMyString strTmpSrcPage;
	char * pszTmpBuf = strTmpSrcPage.GetBuffer( nLen+1 );
	if( NULL == pszTmpBuf )
		return false;
	strncpy( pszTmpBuf, lpszHTMLContent, nLen );
	strTmpSrcPage.ReleaseBuffer( nLen );
	CMyString strResultPage;

	if( SplitASPPage( strTmpSrcPage, strResultPage ) )
	{
		m_strAspResponse.ReleaseBuffer( 0 );
		strTmpSrcPage.ReleaseBuffer( 0 );
		Run( (char*)strResultPage );
		strResultPage.ReleaseBuffer( 0 );
		lpszHTMLContent = (char*)m_strAspResponse;
		nLen = m_strAspResponse.GetLength();
	}

	m_pCurrentElement = &m_HTMLRootElement;	
	m_aActionElements.RemoveAll();
	DeleteAllElements();
	m_HTMLRootElement.Preset();

	CHTMLTokenizer tokenizer;
	tokenizer.SetText( lpszHTMLContent, nLen );

	HTML_ELEMENT_ENUM nTagType;
	CMyString strParam;	

	while( (nTagType=tokenizer.NextToken( strParam )) < HTML_TAG_ERROR )
	{
		if( HTML_TAG_RIGHT_SEPARATOR == nTagType )
		{
			strParam.MakeUpper();

#ifdef _DEBUG_
			m_pCurrentParent->CHTMLElementsBase::Dump();
			TRACE("</%s>\n", strParam);
#endif // _DEBUG

			PopElement();
			continue;
		}

		CreateInstance( nTagType, strParam );
		ASSERT( m_pCurrentElement );		

		if( HTML_TAG_TITLE == nTagType )
		{
			tokenizer.NextToken( m_strTitle );
			m_pCurrentElement->SetParameter( m_strTitle );
		}
		else 
			DoSpecialSetting( nTagType );		

#ifdef _DEBUG_
		m_pCurrentElement->Dump();
#endif //_DEBUG

	}

#ifdef _DEBUG
	TRACE("\n\n\n------------------------------------------------\n");
	Dump( &m_HTMLRootElement );
#endif //_DEBUG

	m_nActiveLinkIndex = 0;
	if( m_aActionElements.GetSize() )
		m_aActionElements[0]->SetActiveLink();

	return true;
}

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		����һЩ���⴦��
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLParser::DoSpecialSetting(HTML_ELEMENT_ENUM nTagType)
{
	ASSERT( m_pCurrentElement );
	if( NULL == m_pCurrentElement )
		return;

	CMyMemBufferManager MemHelper;
	//	����Ϊ���⴦��	
	if( HTML_TAG_IMAGE == nTagType )
	{				//	��Ҫ����ͼ��
		CHTML_IMAGE_Element * pImage = static_cast<CHTML_IMAGE_Element*>(m_pCurrentElement);

		if( ReadFile( pImage->GetFileName(), MemHelper ) )
			pImage->SetImageData( MemHelper.m_pBuffer, MemHelper.m_nBufLen );

		//  CYJ,2005-4-11 set active link image data
		CMyString strActiveURL = pImage->GetActiveImgFileName();
		if( strActiveURL.GetLength() && ReadFile( strActiveURL, MemHelper ) )
			pImage->SetImageData( MemHelper.m_pBuffer, MemHelper.m_nBufLen, false );
	}
	else if( HTML_TAG_BODY == nTagType )
	{
		CHTML_BODY_Element * pImage = static_cast<CHTML_BODY_Element*>(m_pCurrentElement);
		CMyString strFileName = pImage->GetFileName();
		if( false == strFileName.IsEmpty() )
		{
			if( ReadFile( (char*)strFileName, MemHelper ) )
				pImage->SetImageData( MemHelper.m_pBuffer, MemHelper.m_nBufLen );			
		}
	}
}

///-------------------------------------------------------
/// CYJ,2004-11-22
/// Function:
///		������һ��
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLParser::PopElement()
{
	CHTMLElementsBase * pParent = m_pCurrentParent->GetParent();
	if( pParent )
		m_pCurrentParent = pParent;
}

///-------------------------------------------------------
/// CYJ,2004-11-22
/// Function:
///		����һ���µ�Ԫ��
/// Input parameter:
///		None
/// Output parameter:
///		None
///	Note:
///		�����µ�ʵ��������m_pCurrentElement��ֵ
void CHTMLParser::CreateInstance(HTML_ELEMENT_ENUM nTagType, CMyString & strParam)
{
	CHTMLElementsBase * pChild = NULL;
	bool bNewAsParent = true;
	switch( nTagType )
	{
	case HTML_TAG_TEXT:
		{
			CHTMLTextElement * pTextChild = new CHTMLTextElement( m_pCurrentParent );
			pChild = static_cast<CHTMLElementsBase*>(pTextChild);
			bNewAsParent = false;
			if( FALSE == m_pCurrentParent->GetLinkURL().IsEmpty() )
			{
				CHTMLLinkedElementBase * pLinkedElement = static_cast<CHTMLLinkedElementBase*>( pTextChild );
				m_aActionElements.Add( pLinkedElement );	//	��������
			}
		}
		break;
	case HTML_TAG_IMAGE:
		{
			CHTML_IMAGE_Element * pImageChild = new CHTML_IMAGE_Element( m_pCurrentParent );
			pChild = static_cast<CHTMLElementsBase*>( pImageChild );
			bNewAsParent = false;
			if( FALSE == m_pCurrentParent->GetLinkURL().IsEmpty() )
			{
				CHTMLLinkedElementBase * pLinkedElement = static_cast<CHTMLLinkedElementBase*>( pImageChild );
				m_aActionElements.Add( pLinkedElement );	//	��������
			}
		}
		break;

	case HTML_TAG_A:
		pChild = new CHTML_A_Element( m_pCurrentParent );
		break;
	case HTML_TAG_DIV:
		pChild = new CHTML_DIV_Element( m_pCurrentParent );
		break;
	case HTML_TAG_BR:
		pChild = new CHTML_BR_Element( m_pCurrentParent );
		bNewAsParent = false;
		break;	
	case HTML_TAG_FONT:
		pChild = new CHTML_FONT_Element( m_pCurrentParent );
		break;
	case HTML_TAG_U:
		pChild = new CHTML_U_Element( m_pCurrentParent );
		break;
	case HTML_TAG_B:
		pChild = new CHTML_B_Element( m_pCurrentParent );		
		break;
	case HTML_TAG_HTML:		
		pChild = new CHTML_Dummy_Element( m_pCurrentParent, "HTML" );	// �յ�		
		break;
	case HTML_TAG_BODY:
		pChild = new CHTML_BODY_Element( m_pCurrentParent );
		break;
	case HTML_TAG_TITLE:
		pChild = new CHTML_Dummy_Element( m_pCurrentParent, "TITLE" );	// �յ�		
		break;
	case HTML_TAG_PRE:
		pChild = new CHTML_Dummy_Element( m_pCurrentParent, "PRE" );	// �յ�,PRE
		break;
	case HTML_TAG_UNKNOWN:
		pChild = new CHTML_Dummy_Element( m_pCurrentParent, "UNKNOWN" );	// �յ�		
		break;
	case HTML_TAG_LINE:
		bNewAsParent = false;
		pChild = new CHTML_LINE_Element( m_pCurrentParent );
		break;
	case HTML_TAG_RECT:
		bNewAsParent = false;
		pChild = new CHTML_RECT_Element( m_pCurrentParent );	
		break;
	case HTML_TAG_P:			//  CYJ,2005-8-2 add
		pChild = new CHTML_P_Element( m_pCurrentParent );
		break;
	case HTML_TAG_COMMENT:
		return;
	default:
		return;
	}

	pChild->SetParameter( strParam );	

	pChild->SetCoordinate( m_pCurrentParent->GetX(), m_pCurrentParent->GetY(),
		m_pCurrentParent->GetWidth(), m_pCurrentParent->GetHeight() );
	if( HTML_TAG_DIV != nTagType )		// DIV ������ָ�� z-index
		pChild->Set_Z_Index( m_pCurrentParent->Get_Z_Index() );

	m_pCurrentParent->AppendChild( pChild );

	if( bNewAsParent )	
		m_pCurrentParent = pChild;		

	m_pCurrentElement = pChild;
	pChild->SetCDC( m_pDC );
	m_aAllElements.Add( pChild );
}


#ifdef _DEBUG
void CHTMLParser::Dump( CHTMLElementsBase * pElement )
{
	pElement->Dump();
	CHTMLElementsBase * pChild = pElement->GetChild();
	if( pChild )
		Dump( pChild );
	pElement->CHTMLElementsBase::Dump();
	if( pChild )
		TRACE("</%s>\n", pElement->GetTagName() );
	else
		TRACE("\n");

	CHTMLElementsBase * pNext = pElement->GetNext();
	if( pNext )
		Dump( pNext );
}
#endif //_DEBUG

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		ɾ������HTMLԪ��
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLParser::DeleteAllElements()
{
	int nCount = m_aAllElements.GetSize();
	for(int i=0; i<nCount; i++)
	{
		delete m_aAllElements[i];
	}
	m_aAllElements.RemoveAll();
	m_HTMLRootElement.Preset();
	m_pCurrentParent = &m_HTMLRootElement;
}

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		�ƶ�����һ����Ч����
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLParser::MoveToNextActiveLink()
{
	if( 0 == m_aActionElements.GetSize() )
		return;

	m_aActionElements[m_nActiveLinkIndex]->SetActiveLink( false );

	if( ++m_nActiveLinkIndex >= m_aActionElements.GetSize() )
		m_nActiveLinkIndex = 0;
	
	m_aActionElements[m_nActiveLinkIndex]->SetActiveLink();
}

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		�ƻص���һ����Ч����
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLParser::MoveBackActiveLink()
{
	if( 0 == m_aActionElements.GetSize() )
		return;
	
	m_aActionElements[m_nActiveLinkIndex]->SetActiveLink( false );

	if( --m_nActiveLinkIndex < 0 )
		m_nActiveLinkIndex = m_aActionElements.GetSize()-1;

	m_aActionElements[m_nActiveLinkIndex]->SetActiveLink();
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		��ȡ��ǰѡ�е�����URL
/// Input parameter:
///		None
/// Output parameter:
///		����Ϊ0���ַ�������ʾû����Ч����
CMyString CHTMLParser::GetActiveLinkURL( bool bHaseBaseURL )
{
	CMyString strRetVal;
	if( 0 == m_aActionElements.GetSize() )
		return strRetVal;
	if( m_nActiveLinkIndex < 0 || m_nActiveLinkIndex>= m_aActionElements.GetSize() )
		return strRetVal;

	if( m_pSrcFile )
		return m_aActionElements[m_nActiveLinkIndex]->GetLinkURL();
	else
	{
		if( bHaseBaseURL )
			strRetVal = m_strBaseURL;
		strRetVal += m_aActionElements[m_nActiveLinkIndex]->GetLinkURL();
	}
	return strRetVal;
}

///-------------------------------------------------------
/// CYJ,2004-12-6
/// Function:
///		��ȡһ���ļ�
/// Input parameter:
///		lpszRefFileName		����ļ���
///		MemMgr				�ڴ����
/// Output parameter:
///		NULL				ʧ��
///		����				�ɹ�
///	Note:
///		��������Ҫ�ͷ��ڴ�
bool CHTMLParser::ReadFile(LPCSTR lpszRefFileName, CMyMemBufferManager & MemMgr )
{
	MemMgr.Invalid();			//	�ͷ��ڴ�

	if( m_pSrcFile )
	{				//	�����ĵ���ʽ����Ӧ�õ����������
		PONE_COMPOUND_FILE pImgData = m_pSrcFile->Find( lpszRefFileName );
		if( NULL == pImgData )
			return false;
		MemMgr.m_bAutoFreeBuf = false;
		MemMgr.m_nBufLen = pImgData->m_nFileLen;
		MemMgr.m_pBuffer = pImgData->m_pDataBuf;
		return true;
	}

	CMyString strFileName;
	strFileName.Format("%s%s", (char*)m_strBaseURL , lpszRefFileName );
#ifdef _DEBUG	
	TRACE("CHTMLParse::ReadFile, URL=%s\n", (char*)strFileName );
#endif //_DEBUG	
	if( m_bIsLocalFile )
	{								//	�����ļ���ʽ
		CFile fSrc;
		if( FALSE == fSrc.Open( (LPCSTR)strFileName, CFile::modeRead|CFile::typeBinary ) )
			return false;
		int nLen = fSrc.GetLength();
		PBYTE pBuf = new BYTE[nLen];
		if( NULL == pBuf )
			return false;
		try
		{
			fSrc.Read( pBuf, nLen );
			fSrc.Close();
		}
		catch( ... )
		{
			delete pBuf;
			fSrc.Abort();
		}
		MemMgr.m_pBuffer = pBuf;
		MemMgr.m_nBufLen = nLen;
		MemMgr.m_bAutoFreeBuf = true;
		return true;
	}
	else
	{								// http
		CMyHTTPFile	HttpFile;
		if( FALSE == HttpFile.Open( (LPCSTR)strFileName ) )
			return false;
		PBYTE pBuf = (PBYTE) malloc( 4096 );
		if( NULL == pBuf )
			return false;
		int nBytesRead = 0;
		int nRetVal;
		while( (nRetVal = HttpFile.Read( pBuf+nBytesRead, 4096 ) ) )
		{
			nBytesRead += nRetVal;
			pBuf = (PBYTE)realloc( pBuf, ((nBytesRead+4096)+1023)&(0xFFFFFC00) );
			if( NULL == pBuf )
				return false;
		}
		MemMgr.m_pBuffer = pBuf;
		MemMgr.m_nBufLen = nBytesRead;
		MemMgr.m_bAutoFreeBuf = true;
		return true;					
	}

	return false;
}

///-------------------------------------------------------
/// CYJ,2005-4-29
/// ��������:
///		On Basic module created
/// �������:
///		��
/// ���ز���:
///		��
void CHTMLParser::OnModuleCreated( BSModule * pModule )
{
	ASSERT( m_pRequestObj == NULL );
	ASSERT( m_pWriteBlockObj == NULL );
	ASSERT( m_pWriteObj == NULL );
	
	m_pRequestObj = new CMyRequest( pModule, "Request" );
	m_pWriteObj = new CMyResponseWrite( pModule, "ResponseWrite", &m_strAspResponse );
	m_pWriteBlockObj = new CMyResponseWriteBlock( pModule, "ResponseWriteBlock", &m_astrASPBlock, &m_strAspResponse );

	if( m_pRequestObj && false == m_strRequestString.IsEmpty() )
	{
		m_pRequestObj->SetRequst( (char*)m_strRequestString );
		m_strRequestString = "";
	}
	if( m_pfnOnModuleCreateDelete )
		m_pfnOnModuleCreateDelete( pModule, true, m_dwOnModuleCallBackUserData );
}

///-------------------------------------------------------
/// CYJ,2005-4-29
/// ��������:
///		print
/// �������:
///		��
/// ���ز���:
///		��
/// �޸ļ�¼��
///		2006.6.30, ֻ֧�ִ�ӡ����
void CHTMLParser::Print( void * pData, bool bIsString )
{	
	if( bIsString )
	{
		if( pData && (char*)pData )
			m_strAspResponse += (char*)pData;
	}
	else
	{
		CMyString strTmp;
		strTmp.Format( "%d", (int)( *(double*)pData+0.05 ) );	// 2006.6.30 CYJ Modify, support integer only
		m_strAspResponse += strTmp;
	}
}

///-------------------------------------------------------
/// CYJ,2005-4-29
/// ��������:
///		ģ�鼴����ɾ��
/// �������:
///		��
/// ���ز���:
///		��
void CHTMLParser::OnDeleteModule( BSModule * pModule )
{
	if( m_pRequestObj )
		m_pRequestObj->Release();
	m_pRequestObj = NULL;

	if( m_pWriteObj )
		m_pWriteObj->Release();
	m_pWriteObj = NULL;

	if( m_pWriteBlockObj )
		m_pWriteBlockObj->Release();
	m_pWriteBlockObj = NULL;

	if( m_pfnOnModuleCreateDelete )
		m_pfnOnModuleCreateDelete( pModule, false, m_dwOnModuleCallBackUserData );
}

//	���� ASP ��ҳ����
//	��ڲ���
//		strSrcASP				���ֽ�� ASP ��ҳ�������ƻ�
//		strScriptBuf			���������
//	���ز���
//		true					��һ��ASP
//		false 					��������ҳ
bool CHTMLParser::SplitASPPage(CMyString & strSrcASP, CMyString &strScriptBuf)
{
	int nLen = strSrcASP.GetLength();
	char * pszHead = strSrcASP.GetBuffer( nLen+1 );
	pszHead[ nLen ] = 0;					//	����
	m_astrASPBlock.RemoveAll();
	m_strAspResponse = "";
	
	CMyString strTmp;
	int nBlockNo = 0;
	while( pszHead )
	{
		char * pszScriptStart = strstr( pszHead, "<%" );
		if( pszScriptStart == NULL )		//	û�з��ֽű� 
		{
			if( *pszHead )					//	������
			{
				strTmp = pszHead;
				m_astrASPBlock.Add( strTmp );
				strTmp.Format("\n\nResponseWriteBlock(%d)\n\n",nBlockNo);
				strScriptBuf += strTmp;
			}
			break;							//	����
		}
		*pszScriptStart ++ = 0;
		pszScriptStart ++;					//	���� <%
		char * pszScriptEnd = strstr( pszScriptStart, "%>" );
		if( pszScriptEnd == NULL )
			return false;
		* pszScriptEnd = 0;
		pszScriptEnd += 2;

		if( *pszHead )					//	������
		{
			strTmp = pszHead;
			m_astrASPBlock.Add( strTmp );
			strTmp.Format("\n\nResponseWriteBlock(%d)\n\n",nBlockNo);
			strScriptBuf += strTmp;
			nBlockNo ++;
		}			
		strScriptBuf += ( pszScriptStart );					       
		pszHead = pszScriptEnd;
	}
	ASSERT( nBlockNo == m_astrASPBlock.GetSize()-1 );
	strSrcASP.ReleaseBuffer( 0 );
	return (m_astrASPBlock.GetSize()>0);
}

///-------------------------------------------------------
/// CYJ,2005-4-29
/// ��������:
///		Set request
/// �������:
///		lpszRequest			�ļ��� & ����
/// ���ز���:
///		�µ��ļ���
CMyString CHTMLParser::SetRequestString(LPCSTR lpszRequest)
{
	CMyString strRetVal = lpszRequest;
	strRetVal.TrimLeft();
	m_strRequestString = strRetVal;
	int nPos = m_strRequestString.Find( '?' );
	if( nPos >= 0 )
	{
		m_strRequestString.Delete( 0, nPos+1 );		// ���ļ�������ȡ����
		strRetVal.ReleaseBuffer( nPos );
		strRetVal.TrimRight();
	}
	return strRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-4-29
/// ��������:
///		Register on Module call back
/// �������:
///		��
/// ���ز���:
///		��
PFN_OnModuleCreateDelete CHTMLParser::SetOnModuleCreateDeleteCallBack( PFN_OnModuleCreateDelete pfn, DWORD dwUserData )
{
	PFN_OnModuleCreateDelete pRetVal = m_pfnOnModuleCreateDelete;
	m_pfnOnModuleCreateDelete = pfn;
	m_dwOnModuleCallBackUserData = dwUserData;
	return pRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-5-12
/// ��������:
///		register
/// �������:
///		��
/// ���ز���:
///		��
extern "C" void MyHTMLRegisterModuleCallBack( CHTMLParser * pParser, PFN_OnModuleCreateDelete pFn, DWORD dwUserData )
{	
	ASSERT( pParser );
	if( pParser )
		pParser->SetOnModuleCreateDeleteCallBack( pFn, dwUserData );
}

///-------------------------------------------------------
/// CYJ,2005-6-3
/// ��������:
///		��ȡ�������Ӹ���
/// �������:
///		��
/// ���ز���:
///		��
int CHTMLParser::GetLinkUrlCount()
{
	return m_aActionElements.GetSize();
}

///-------------------------------------------------------
/// CYJ,2005-6-3
/// ��������:
///		��ȡ��ǰ��Ч���ӵ����
/// �������:
///		��
/// ���ز���:
///		��
int CHTMLParser::GetActiveLinkIndex()
{
	return m_nActiveLinkIndex;
}

///-------------------------------------------------------
/// CYJ,2005-6-3
/// ��������:
///		�����µ���Ч����
/// �������:
///		nNewIndex			�µ����������±�
/// ���ز���:
///		ԭ�������
int CHTMLParser::SetActiveLinkIndex(int nNewIndex)
{
	int nRetVal = m_nActiveLinkIndex;
	if( m_aActionElements.GetSize() )
	{	
		m_aActionElements[m_nActiveLinkIndex]->SetActiveLink( false );

		if( nNewIndex < 0 )
			m_nActiveLinkIndex = m_aActionElements.GetSize()-1;
		else if( nNewIndex >= m_aActionElements.GetSize() )
			nNewIndex = 0;

		m_nActiveLinkIndex = nNewIndex;

		m_aActionElements[m_nActiveLinkIndex]->SetActiveLink();		
	}

	return nRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-6-3
/// ��������:
///		��ȡָ����ŵ�������ID
/// �������:
///		nIndex		�����±�
///		pstrID		���ID��ȱʡΪ NULL
/// ���ز���:
///		ָ����ŵ�����
CMyString CHTMLParser::GetLinkURL(int nIndex, CMyString *pstrID)
{
	CMyString strRetVal;
	if( nIndex < 0 || nIndex >= m_aActionElements.GetSize() )
		return strRetVal;

	if( pstrID )
	{	
		CHTMLElementsBase * pElement = m_aActionElements[nIndex]->GetElementOfLinkURL();
		if( pElement )
		{
			CHTML_A_Element * pA = (CHTML_A_Element *)pElement;
			*pstrID = pA->GetID();
		}
		else
			*pstrID = "";
	}

	return strRetVal;
}
