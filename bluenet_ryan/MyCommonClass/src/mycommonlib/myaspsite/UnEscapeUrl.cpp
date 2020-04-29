// UnEscapeUrl.cpp: implementation of the CUnEscapeUrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UnEscapeUrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#if defined(_DEBUG) && defined(_WIN32)
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

//	����
CMyString CUnEscapeUrl::Decode(const CMyString &strUrl, BOOL bQuery)
{
	CMyString strDecoded = strUrl;
	strDecoded.TrimLeft();
	strDecoded.TrimRight();
	if( strDecoded.IsEmpty() )
		return strDecoded;

	// special processing or query strings....
	if ( bQuery )
		strDecoded.Replace('+',' ');

	// first see if there are any %s to decode....
	if ( strDecoded.Find( '%' ) != -1 )
	{
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
					if( ch1 == 'u' || ch1 == 'U' )
					{							//	IE �µı���, ֱ��ʹ��WideChar����,��: ��==>%u6211
						for(int i=0; i<4; i++)
						{
							ch1 = *pszSrc++;
							wWideChar <<= 4;
							wWideChar |= (ch1 >= 'A') ? ((ch1&0xdf)-'A'+0xa) : (ch1-'0');
						}
						wWideChar = WideToMultiByte( wWideChar );		//	ת����MultiBytes
						*((PWORD)pszDst) = wWideChar;
						pszDst += 2;
						nLen -= 4;
						continue;
					}
					else
					{
						*pszDst = DecodeOneByte( pszSrc-1 );
						pszSrc ++;
						pszDst ++;
						nLen -= 2;					
					}
				}
			}
			else
			{
				*pszDst = ch;				//	��ͨ������
				pszDst ++;
			}
		}
		strDecoded.ReleaseBuffer( nLen );	//	�ͷŻ�����
		UpdateWideChar( strDecoded );
	}
	return strDecoded;
}

//	����һ���ֽ�
//	��ڲ���
//		pszEsc				��������ַ�, ��ʽ��: %EA
//	����һ���ֽڵ�����
BYTE CUnEscapeUrl::DecodeOneByte(LPCSTR pszEsc)
{
	BYTE ch1 = *pszEsc++;
	ch1 = (ch1 >= 'A') ? ((ch1&0xdf)-'A'+0xa) : (ch1-'0');
	BYTE ch2 = *pszEsc;
	ch2 = (ch2 >= 'A') ? ((ch2&0xdf)-'A'+0xa) : (ch2-'0');	
	return ch1 = ch1*16 + ch2;
}

//	���ֽ�
WORD CUnEscapeUrl::WideToMultiByte(WORD dwWideChar)
{
	return dwWideChar;
}

//	����Wide Char to MultiChar
void CUnEscapeUrl::UpdateWideChar(CMyString &strUrl)
{
	int nLen = strUrl.GetLength();
	char * pszSrc = strUrl.GetBuffer( nLen+2 );
	pszSrc[nLen] = 0;					//	���䲿���� 0
	pszSrc[nLen+1] = 0;
	char * pszDst = pszSrc;
	BYTE ch = *pszSrc;
	while( ch )
	{
		ch = *pszSrc ++;
		if( ch < 0x80 )
		{								//	��ͨ���ַ�
			*pszDst = ch;
			pszDst ++;
			continue;
		}
		else
		{
			if( (ch&0xF0)==0xE0 )
			{
				WORD wWideChar = ch;
				if( pszSrc[0] >= 0x60 && (pszSrc[0]&0xE0)<=0xA0 && pszSrc[1]>= 0x60 && (pszSrc[1]&0xC0)<=0xA0 )
				{							//	Ŀǰ����֪��IE�ı��뷽ʽ
					wWideChar = ch<<12;
					wWideChar |= ( (pszSrc[0]&0x3F) << 6 );
					wWideChar |= ( pszSrc[1]&0x3F );
					wWideChar = WideToMultiByte( wWideChar );
					*((PWORD)pszDst) = wWideChar;
					pszDst += 2;
					pszSrc += 2;
					nLen --;
					continue;
				}
/*				else if( (pszSrc[0]&0xE0) == 0xC0 )
				{
					wWideChar = ((pszSrc[0]&0xF)<<6)|(pszSrc[1]&0x3f);
					wWideChar = WideToMultiByte( wWideChar );
					*((PWORD)pszDst) = wWideChar;
					pszDst += 2;
					pszSrc += 2;
					nLen --;
					continue;
				}
*/
			}
			*pszDst = ch;
			pszDst ++;
		}
	}
	strUrl.ReleaseBuffer( nLen );
}

//	����
CMyString CUnEscapeUrl::Decode(LPCSTR lpszUrl, BOOL bQuery)
{
	CMyString strTmp = lpszUrl;
	return Decode( strTmp, bQuery );
}
