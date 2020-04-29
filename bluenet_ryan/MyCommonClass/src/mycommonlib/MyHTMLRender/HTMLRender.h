///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2004-11-23
///
///		��;��
///			HTML ��ʾ
///=======================================================

#if !defined(AFX_HTMLRENDER_H__2E7426CC_9FE3_409F_A222_3BEEB4EF81D8__INCLUDED_)
#define AFX_HTMLRENDER_H__2E7426CC_9FE3_409F_A222_3BEEB4EF81D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MyString.h>

class CHTMLParser;
class CHTMLElementsBase;

#if defined(__FOR_MICROWIN_EMBED__)||defined(__FOR_MY_OSD_EMBED__)
   #include <mygdiobjs.h>
#endif //__FOR_MICROWIN_EMBED__

class CHTMLRender  
{
public:
	int SetFontSize(int nNewSize=5);
	int GetFontSize(){ return m_nFontSize; }
	CDC * GetDC(){ return m_pDC; }
	void SetDC( CDC * pDC ){m_pDC = pDC;}
	CHTMLRender(CDC * pDC);
	virtual ~CHTMLRender();

	void Render( CHTMLParser * pParser, int nWidth, int nHeight );

	void SetBold( bool bBold ){ m_bIsBold=bBold; }
	bool IsBold(){ return m_bIsBold; }

	void SetUnderLine( bool bUnderLine ){ m_bIsUnderLine = bUnderLine; }
	bool IsUnderLine(){ return m_bIsUnderLine; }

	COLORREF	GetTextColor(){ return m_colorText; }
	void		SetTextColor( COLORREF newColor );
	COLORREF	GetLinkTextColor(){ return m_colorLinkText; }
	void		SetLinkTextColor( COLORREF newColor ){ m_colorLinkText=newColor; }
	COLORREF	GetActiveLinkTextColor(){ return m_colorActiveLink; }
	void		SetActiveLinkTextColor( COLORREF newColor ){ m_colorActiveLink=newColor; }
	int			GetWidth(){return m_nWinWidth;}
	int			GetHeight(){return m_nWinHeight;}

private:
	void RenderOneItem( CHTMLElementsBase * pElement, int & x, int & y );

private:
	CFont		m_Font_x_12;		//	С���壬12x12
	CFont		m_Font_x_16;		//	�����壬16x16
	int			m_nFontSize;		//	��ǰʹ�õ������С
	CFont *		m_pFont;			//	��ǰʹ�õ�����ָ��
	CPen		m_UnderLinePen;		//	���ڻ��»��ߣ�����ɫ�뵱ǰText��ͬ�����ڸı�TextColorʱ�ı�

	CDC *		m_pDC;				//	��ͼ����
	CHTMLParser *	m_pParser;
	
	COLORREF	m_colorText;		//	��ͨ������ɫ
	COLORREF	m_colorLinkText;	//	���ӵ�������ɫ
	COLORREF	m_colorActiveLink;	//	��ǰ��������

	CMyString	m_strTitle;			//	����	

	bool		m_bIsUnderLine;		//	�Ƿ����»���
	bool		m_bIsBold;			//	�Ƿ����

	int			m_nWinWidth;
	int			m_nWinHeight;	
};

#endif // !defined(AFX_HTMLRENDER_H__2E7426CC_9FE3_409F_A222_3BEEB4EF81D8__INCLUDED_)
