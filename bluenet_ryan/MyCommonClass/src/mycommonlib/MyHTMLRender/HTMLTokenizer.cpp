///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2004-11-22
///
///		��;��
///			�Զ���HTML�ַ�������
///=======================================================

// HTMLTokenizer.cpp: implementation of the CHTMLTokenizer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HTMLTokenizer.h"

#if defined(_DEBUG) && defined(_WIN32)
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHTMLTokenizer::CHTMLTokenizer()
{
	m_pszNextBegin = NULL;
	m_pszBegin = NULL;
}

CHTMLTokenizer::~CHTMLTokenizer()
{

}

///-------------------------------------------------------
/// CYJ,2004-11-22
/// Function:
///		���ô�������HTML�ı�
/// Input parameter:
///		lpszHTML				��������HTML�ı�
///		nLen					��������С
/// Output parameter:
///		None
/// Note:
///		���� nLen < strlen(lpszHTMLContent)����ʱ��ֻ������ nLen Ϊֹ
void CHTMLTokenizer::SetText( LPCSTR lpszHTML, int nLen )
{
	m_pszBegin = m_strSrcHTML.GetBuffer( nLen+2 );
	m_pszBegin[nLen] = 0;
	m_pszBegin[nLen+1] = 0;
	memcpy( m_pszBegin, lpszHTML, nLen );
	m_strSrcHTML.ReleaseBuffer( nLen );

	m_pszBegin = m_strSrcHTML.GetBuffer( nLen+2 );	
	m_pszNextBegin = m_pszBegin;
}

///-------------------------------------------------------
/// CYJ,2004-11-22
/// Function:
///		��ȡ��һ����Ԫ
/// Input parameter:
///		strParam				���������
/// Output parameter:
///		Ԫ������
HTML_ELEMENT_ENUM CHTMLTokenizer::NextToken(CMyString &strParam)
{
	if( NULL == m_pszNextBegin || 0 == *m_pszNextBegin )
		return HTML_TAG_END;		//	����

	m_pszBegin = m_pszNextBegin;	
	SkipBlankChar();
	char cFirstChar = *m_pszBegin;
	if( '<' != cFirstChar )
	{								//  ��ͨ�ַ���
		DoNormalText( strParam );
		strParam.TrimLeft();
		strParam.TrimRight();
		return HTML_TAG_TEXT;
	}

	m_pszBegin ++;
	SkipBlankChar();				//	�����ո�

	if( '/' == *m_pszBegin )
	{								//	һ�������Ľ�β
		m_pszBegin ++;
		strParam = GetString();
		m_pszNextBegin = strchr( m_pszBegin, '>' );
		if( m_pszNextBegin )
			m_pszNextBegin ++;		//	������һ��Ԫ��
		return HTML_TAG_RIGHT_SEPARATOR;
	}
	CMyString strTag = GetString();
	strTag.TrimLeft();
	strTag.TrimRight();
	strTag.MakeUpper();
	HTML_ELEMENT_ENUM RetVal = GetTagType( strTag );
	if( HTML_TAG_ERROR == RetVal )
		return HTML_TAG_ERROR;

	m_pszNextBegin = strchr( m_pszBegin, '>' );
	if( m_pszNextBegin )
		*m_pszNextBegin = 0;
	else
		return HTML_TAG_ERROR;		//	��������

	strParam = m_pszBegin;			//	��ȡ����

	if( m_pszNextBegin )
	{
		*m_pszNextBegin = '>';		//	�ָ�
		m_pszNextBegin ++;			//	������һ��Ԫ��
	}

	strParam.Remove( '\'' );
	strParam.Remove( '\"' );
	strParam.TrimLeft();
	strParam.TrimRight();
	return RetVal;
}

///-------------------------------------------------------
/// CYJ,2004-11-22
/// Function:
///		������ͨ�ַ���
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLTokenizer::DoNormalText(CMyString &strParam)
{
	m_pszNextBegin = strchr( m_pszBegin, '<' );
	if( m_pszNextBegin )
		*m_pszNextBegin = 0;		//	���ó��ַ���β�ַ�0

	strParam = m_pszBegin;		//	����ַ���
	strParam.TrimRight();

	if( m_pszNextBegin )
		*m_pszNextBegin = '<';	//	�ٻָ�Ϊ�ַ� '<'
}

///-------------------------------------------------------
/// CYJ,2004-11-22
/// Function:
///		skip blank char
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLTokenizer::SkipBlankChar()
{
	BYTE cVar;
	while( (cVar = (BYTE)(*m_pszBegin)) )
	{
		if( cVar <= 0x20 )
			m_pszBegin ++;
		else
			break;
	}
}

///-------------------------------------------------------
/// CYJ,2004-11-22
/// Function:
///		Get one string
/// Input parameter:
///		bModifyBeginPtr			modify begin ptr, default is true
/// Output parameter:
///		None
CMyString CHTMLTokenizer::GetString(bool bModifyBeginPtr)
{	
	SkipBlankChar();

	char szTmp[100];
	szTmp[0] = 0;
	char * pszTmp = m_pszBegin;
	BYTE cTmpChar;
	for(WORD i=0; i<sizeof(szTmp)-1; i++)	
	{
		cTmpChar = BYTE( *pszTmp );
		if( cTmpChar <= 0x20 || cTmpChar == '>' )
		{
			szTmp[i] = 0;
			break;
		}
		szTmp[i] = cTmpChar;
		pszTmp ++;
	}
	if( bModifyBeginPtr )
		m_pszBegin = pszTmp;

	return CMyString( szTmp );
}

///-------------------------------------------------------
/// CYJ,2004-11-22
/// Function:
///		Get type
/// Input parameter:
///		None
/// Output parameter:
///		None
HTML_ELEMENT_ENUM CHTMLTokenizer::GetTagType(CMyString &strTag)
{
	static struct
	{
		const char *		m_pszTag;
		HTML_ELEMENT_ENUM	m_nTypeID;
	}TagTypeTable[]=
	{
		{"A",		HTML_TAG_A		},		
		{"BR",		HTML_TAG_BR		},		
		{"IMAGE",	HTML_TAG_IMAGE	},	
		{"IMG",		HTML_TAG_IMAGE	},	
		{"FONT",	HTML_TAG_FONT	},
		{"U",		HTML_TAG_U		},
		{"B",		HTML_TAG_B		},
		{"DIV",		HTML_TAG_DIV	},
		{"PRE",		HTML_TAG_PRE	},
		{"HTML",	HTML_TAG_HTML	},
		{"TITLE",	HTML_TAG_TITLE	},
		{"BODY",	HTML_TAG_BODY	},	
		{"LINE",	HTML_TAG_LINE   },
		{"RECT",	HTML_TAG_RECT	},
		{"P",		HTML_TAG_P		},
	};

	if( strTag[0] == '!' )				//  CYJ,2005-5-9 ֧��ע��
		return HTML_TAG_COMMENT;
	
	int nCount = sizeof(TagTypeTable)/sizeof(TagTypeTable[0]);
	for(int i=0; i<nCount; i++)
	{
		if( strTag == TagTypeTable[i].m_pszTag )
			return TagTypeTable[i].m_nTypeID;
	}
	return HTML_TAG_UNKNOWN;
}
