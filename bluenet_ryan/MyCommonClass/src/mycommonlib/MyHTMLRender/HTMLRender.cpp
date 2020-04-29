///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2004-11-23
///
///		��;��
///			HTML ��ʾ
///=======================================================

#include "stdafx.h"
#include "HTMLRender.h"
#include "HTMLParser.h"

#ifdef __FOR_MICROWIN_EMBED__
  #include <mygdiobjs.h>
#endif //__FOR_MICROWIN_EMBED__

#if defined(_DEBUG) && defined(_WIN32)
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHTMLRender::CHTMLRender(CDC * pDC)
{
	m_pDC = pDC;
	m_pParser = NULL;
	m_nFontSize = 5;				//	��ǰʹ�õ������С
	m_pFont = NULL;					//	��ǰʹ�õ�����ָ��

	m_colorText = 0xFFFFFF;			//	��ͨ������ɫ���׵�
	m_colorLinkText = 0xFF8080;		//	���ӵ�������ɫ����ɫ
	m_colorActiveLink = 0xFF00FF;	//	��ǰ����

	m_strTitle = "û�б���";

	m_bIsUnderLine = false;		//	�Ƿ����»���
	m_bIsBold = false;			//	�Ƿ����	
	
#if defined(_WIN32)||defined(__FOR_MICROWIN_EMBED__)
	m_Font_x_12.CreatePointFont( 160, "����" );
	m_Font_x_16.CreatePointFont( 240, "����" );
#elif defined(__FOR_MY_OSD_EMBED__)
	m_Font_x_12.CreatePointFont( pDC, 160, "����" );
	m_Font_x_16.CreatePointFont( pDC, 240, "����" );
#else
	!!!!!!!!!!!!	
#endif //_WIN32		
}

#include <stdio.h>

CHTMLRender::~CHTMLRender()
{
}

///-------------------------------------------------------
/// CYJ,2004-11-23
/// Function:
///		��ʾ
/// Input parameter:
///		pDC				��ʾ
///		pParser			������
///		nWidth			���
///		nHeight			�߶�
/// Output parameter:
///		None
void CHTMLRender::Render( CHTMLParser * pParser, int nWidth, int nHeight )
{
	m_pParser = pParser;
	m_nWinWidth = nWidth;
	m_nWinHeight = nHeight;

#ifdef _WIN32
	int nSavedEnv = m_pDC->SaveDC();
#endif //_WIN32	
	m_pDC->SelectObject( &m_Font_x_16 );	
	m_pDC->SetBkMode( TRANSPARENT );
	
	int x = 0;
	int y = 0;
	pParser->m_HTMLRootElement.SetCoordinate( 0, 0, nWidth, nHeight );
	RenderOneItem( &pParser->m_HTMLRootElement, x, y );

#ifdef _WIN32
	m_pDC->RestoreDC( nSavedEnv );
#elif defined(__FOR_MY_OSD_EMBED__)
	m_pDC->SelectObject( &m_Font_x_16 );	
#endif //_WIN32	
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		����һ��Ԫ��
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLRender::RenderOneItem( CHTMLElementsBase * pElement, int & x, int & y )
{
	pElement->OnRender( this, x, y );

	CHTMLElementsBase * pParentItem = pElement->GetParent();
	if( NULL == pParentItem )
		pParentItem = pElement;

	if( x > pParentItem->GetWidth()+pParentItem->GetX() )
	{
		y += pElement->OnNewLine();
		x = pParentItem->GetX();
	}
	CHTMLElementsBase * pChild = pElement->GetChild();
	if( pChild )
		RenderOneItem( pChild, x, y );
	pElement->OnEndRender( this, x ,y );

	CHTMLElementsBase * pNext = pElement->GetNext();
	if( pNext )
		RenderOneItem( pNext, x, y );
}

///-------------------------------------------------------
/// CYJ,2004-11-23
/// Function:
///		���������С
/// Input parameter:
///		nNewSize			�µ������С
/// Output parameter:
///		����ԭ���������С
int CHTMLRender::SetFontSize(int nNewSize)
{
	int nRetVal = m_nFontSize;

	m_nFontSize = nNewSize;
	if( m_nFontSize >= 5 )
		m_pFont = &m_Font_x_16;
	else
		m_pFont = &m_Font_x_12;

	GetDC()->SelectObject( m_pFont );	

	return nRetVal;
}

///-------------------------------------------------------
/// CYJ,2004-11-24
/// Function:
///		Set text color
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLRender::SetTextColor( COLORREF newColor )
{
	m_colorText = newColor;
	GetDC()->SetTextColor( newColor );

	GetDC()->SelectObject( (CPen*) NULL );
	m_UnderLinePen.DeleteObject();
	m_UnderLinePen.CreatePen( PS_SOLID, 1, newColor );
	GetDC()->SelectObject( &m_UnderLinePen );
}