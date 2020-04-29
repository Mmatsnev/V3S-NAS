// MyCompoundFileObj.h: interface for the CMyCompoundFileObj class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYCOMPOUNDFILEOBJ_H__88DB27D3_F0A1_482D_AAE2_6E95D34FCC84__INCLUDED_)
#define AFX_MYCOMPOUNDFILEOBJ_H__88DB27D3_F0A1_482D_AAE2_6E95D34FCC84__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <time.h>

typedef struct tagONE_COMPOUND_FILE
{
	int		m_nFileLen;				//	�ļ�����
	time_t	m_nLastModifyTime;		//	����޸�ʱ��
	LPCSTR	m_pszFileName;			//	�ļ�������'\0'��β
	PBYTE	m_pDataBuf;				//	���ݻ�����
}ONE_COMPOUND_FILE,*PONE_COMPOUND_FILE;

class CMyCompoundFileObj  
{
public:
	int GetCount(){ return m_nFileCount; }
	void Detach();
	bool Attach( PBYTE pBuf, int nLen );
	CMyCompoundFileObj();
	virtual ~CMyCompoundFileObj();
	ONE_COMPOUND_FILE & ElementAt( BYTE nIndex )
	{
		if( nIndex > m_nFileCount )
			nIndex = 0;
		return m_aFileObjs[nIndex];
	}
	ONE_COMPOUND_FILE & operator [](int nIndex ){ return ElementAt(nIndex); }
	PONE_COMPOUND_FILE Find( LPCSTR lpszFileName, bool bNoCase = true );

	void ToLittleEndian( PBYTE pSrc, PBYTE pDst, int nCount )
	{
		pSrc += (nCount-1);
		for(int i=0; i<nCount; i++)
		{
			*pDst ++ = *pSrc--;
		}
	}

#ifdef _DEBUG
	void Dump();
#endif //_DEBUG

	enum{
		MAX_FILE_COUNT = 256,
	};

	int	m_nFileCount;
	ONE_COMPOUND_FILE	m_aFileObjs[MAX_FILE_COUNT];
};

#endif // !defined(AFX_MYCOMPOUNDFILEOBJ_H__88DB27D3_F0A1_482D_AAE2_6E95D34FCC84__INCLUDED_)
