///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2004-11-26
///
///		��;��
///			�ҵ�HTML��ʾ�����
///=======================================================

// MyHTMLRender.cpp: implementation of the CMyHTMLRender class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyHTMLRender.h"
#include "HTMLParser.h"
#include "HTMLTokenizer.h"
#include "MyCompoundFileObj.h"
#include "HTMLRender.h"

#ifdef _WIN32
	#include <MyCommonToolsLib.h>
#else
	#include <mygdiobjs.h>
#endif //_WIN32

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyHTMLRender::CMyHTMLRender(CDC * pDC)
{
	m_pDC = pDC;
	m_pFileObj = new CMyCompoundFileObj;
	m_pParser = new CHTMLParser( pDC );
	m_pRender = new CHTMLRender( pDC ); 
	m_pActiveLinkURL = new CMyString;
}

CMyHTMLRender::~CMyHTMLRender()
{
	if( m_pFileObj )
		delete m_pFileObj;
	if( m_pParser )
		delete m_pParser;
	if( m_pRender )
		delete m_pRender;
	if( m_pActiveLinkURL )
		delete m_pActiveLinkURL;
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		��ʾ
/// Input parameter:
///		nWidth			width
///		nHeight			heigh
///		pDC				overload pDC object
/// Output parameter:
///		None
void CMyHTMLRender::Render(int nWidth, int nHeight, CDC * pDC )
{	
	if( false == IsValid() )
		return;	
	if( pDC )
	{
		m_pDC = pDC;
		m_pRender->SetDC( pDC );
	}
	m_pRender->Render( m_pParser, nWidth, nHeight );
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		������ҳ
/// Input parameter:
///		pBuf					������������ΪNULL����ʱʹ���ϴ����õ�����
///		nLen					��С
///		lpszStartFileName		��ҳ�ļ�����ȱʡΪ INDEX.HTM
/// Output parameter:
///		None
///	Note:
///		����ʾ�����У������ͷ� pBuf
bool CMyHTMLRender::Parse(PBYTE pBuf, int nLen, LPCSTR lpszStartFileName)
{
	if( false == IsValid() )
		return false;
	if( NULL == pBuf || 0 == nLen )
	{								//	ȱʡΪԭ�����ļ�
		if( 0 == m_pFileObj->GetCount() )
			return Parse( lpszStartFileName );		// �Ǹ����ĵ���ʽ
	}	
	else if( false == m_pFileObj->Attach( pBuf, nLen ) )
		return false;

	return m_pParser->Parse( m_pFileObj, lpszStartFileName );
}

///-------------------------------------------------------
/// CYJ,2004-12-6
/// Function:
///		����һ���������������ø����ĵ�
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CMyHTMLRender::Parse( PBYTE pBuf, int nLen, LPCSTR lpszContent, int nContentBufSize, LPCSTR lpszRequest )
{
	if( false == IsValid() )
		return false;
	if( NULL == lpszContent || 0 == nLen )
		return false;
	if( NULL == pBuf || 0 == nLen )
	{								//	ȱʡΪԭ�����ļ�
		if( 0 == m_pFileObj->GetCount() )
			return false;
	}	
	else if( false == m_pFileObj->Attach( pBuf, nLen ) )
		return false;

	return m_pParser->Parse( m_pFileObj, lpszContent, nContentBufSize, lpszRequest );
}

///-------------------------------------------------------
/// CYJ,2004-12-6
/// Function:
///		����һ���ĵ�
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CMyHTMLRender::Parse( LPCSTR lpszStartFileName )
{
	if( false == IsValid() )
		return false;
	if( NULL == lpszStartFileName )
		return false;
	return m_pParser->Parse( lpszStartFileName );
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		�Ƿ���Ч
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CMyHTMLRender::IsValid()
{
	if( NULL == m_pFileObj )
		return false;
	if( NULL == m_pParser )
		return false;
	if( NULL == m_pRender )
		return false;
	return true;
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		��ȡ��Ч������
/// Input parameter:
///		None
/// Output parameter:
///		NULL				failed
LPCSTR CMyHTMLRender::GetActiveLinkURL( bool bHaseBaseURL )
{
	if( m_pParser )
	{
		*m_pActiveLinkURL = m_pParser->GetActiveLinkURL( bHaseBaseURL );
		return (char*)(*m_pActiveLinkURL);
	}
	return "";
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		��һ������
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyHTMLRender::MoveBackActiveLink()
{
	if( m_pParser )
		m_pParser->MoveBackActiveLink();
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		��һ������
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyHTMLRender::MoveToNextActiveLink()
{
	if( m_pParser )
		m_pParser->MoveToNextActiveLink();
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
CMyString CMyHTMLRender::GetLinkURL( int nIndex, CMyString * pstrID )
{
	if( m_pParser )
		return m_pParser->GetLinkURL( nIndex, pstrID );
	return CMyString("");
}

///-------------------------------------------------------
/// CYJ,2005-6-3
/// ��������:
///		�����µ���Ч����
/// �������:
///		nNewIndex			�µ����������±�
/// ���ز���:
///		ԭ�������
int CMyHTMLRender::SetActiveLinkIndex( int nNewIndex )
{
	if( m_pParser )
		return m_pParser->SetActiveLinkIndex( nNewIndex );
	return 0;
}

///-------------------------------------------------------
/// CYJ,2005-6-3
/// ��������:
///		��ȡ��ǰ��Ч���ӵ����
/// �������:
///		��
/// ���ز���:
///		��
int CMyHTMLRender::GetActiveLinkIndex()
{
	if( m_pParser )
		return m_pParser->GetActiveLinkIndex();
	return 0;
}

///-------------------------------------------------------
/// CYJ,2005-6-3
/// ��������:
///		��ȡ�������Ӹ���
/// �������:
///		��
/// ���ز���:
///		��
int CMyHTMLRender::GetLinkUrlCount()
{
	if( m_pParser )
		return m_pParser->GetLinkUrlCount();
	return 0;
}

