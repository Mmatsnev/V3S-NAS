///---------------------------------------------------------------
///	
///	������
///	2004.11.30	����ͨ�������������ι�˾
///
///	�ļ���;��
///		MicroWindow ͼ����
///---------------------------------------------------------------

#include "stdafx.h"
#include "mwin_image.h"
#include "string.h"

CMWin_Image::CMWin_Image()
{
	memset( &m_ImgInfo, 0, sizeof(m_ImgInfo) );
	m_nImageID = 0;
}

CMWin_Image::~CMWin_Image()
{
	DeleteObject();
}

void CMWin_Image::DeleteObject()
{
	if( m_nImageID )
		GdFreeImage( m_nImageID );
	m_nImageID = 0;
}

bool CMWin_Image::LoadFromFile( HDC hDC,  LPCSTR lpszFileName, int nFlags )
{
	DeleteObject();	
	m_nImageID = GdLoadImageFromFile( hDC->psd, (char*)lpszFileName, nFlags );
	if ( 0 == m_nImageID  )
		return false;
	return GdGetImageInfo( m_nImageID, &m_ImgInfo );
}

bool CMWin_Image::LoadFromBuffer( HDC hDC, PBYTE pBuffer, int nLen, int nFlags )
{
	DeleteObject();
	
	m_nImageID = GdLoadImageFromBuffer( hDC->psd, (void*)pBuffer, nLen, nFlags );	
	if ( 0 == m_nImageID  )
		return false;
	return GdGetImageInfo( m_nImageID, &m_ImgInfo );
}

void CMWin_Image::Draw( HDC hDC, int x, int y, int nWidthToFit, int nHeightToFit )
{
	if( 0 == m_nImageID )
		return;
	// ��Ϊ GdDrawImageToFit ʹ����Ļ�������꣬������Ҫ����һ��ӳ��
	POINT ptTmp = {x, y};
	::ClientToScreen( hDC->hwnd, &ptTmp );
	GdDrawImageToFit( hDC->psd, ptTmp.x, ptTmp.y, nWidthToFit, nHeightToFit, m_nImageID );
}
