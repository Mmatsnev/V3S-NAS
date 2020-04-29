// CommParamParser.cpp: implementation of the CCommParamParser class.
//
//////////////////////////////////////////////////////////////////////
//	2002.8.5	�޸ģ�֧�ַָ��Ϊ ':' �� '='

#include "stdafx.h"
#define __USE_ISOC99
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <MyMemFile.h>
#include <CommParamParser.h>
#include <MySyncObj.h>

#ifdef _WIN32
	#pragma message( "This file can only be compiled under Linux" )
	#pragma warning( error : 1999 )
#endif // _WIN32

extern "C" ICommParamParser * CreateCommParser()
{
    CCommParamParser * pItem = new CCommParamParser;
    if( NULL == pItem )
    	return NULL;
    pItem->AddRef();
    return static_cast<ICommParamParser *>( pItem );
}

CCommParameterBlock::CCommParameterBlock()
{
}

CCommParameterBlock::~CCommParameterBlock()
{
}

long CCommParameterBlock::AddRef(void)
{
	return 1;
}

long CCommParameterBlock::Release(void)
{
    return 1;
}

DWORD CCommParameterBlock::QueryInterface( REFIID iid, void **ppvObject)
{
	ICommParameterBlock * pObj = static_cast<ICommParameterBlock *>( this );
	*ppvObject = (void*)pObj;
	return 0;			// S_OK
}

//////////////////////////////////////////////
//	����:
//		���ַ�����ȡ����
//	��ڲ���:
//		lpszKey				������
//		lpszDefValue		ȱʡ��ֵ
LPCSTR CCommParameterBlock::GetAsString(LPCSTR lpszKey, LPCSTR lpszDefValue)
{
	ASSERT( lpszKey );
	CMyString strKey = lpszKey;
	strKey.MakeUpper();
	if( FALSE == Lookup( strKey, m_GetAsStringRetVal ) )
		m_GetAsStringRetVal = lpszDefValue;
	return m_GetAsStringRetVal;
}

//////////////////////////////////////////////
//	����:
//		��DWORD��ȡ����
//	��ڲ���:
//		lpszKey				������
//		dwDefValue			ȱʡ��ֵ, 0
DWORD CCommParameterBlock::GetAsDWORD(LPCSTR lpszKey, DWORD dwDefValue)
{
	ASSERT( lpszKey );
	CMyString strRetVal;
	CMyString strKey = lpszKey;
	strKey.MakeUpper();
	if( FALSE == Lookup( strKey, strRetVal ) )
		return dwDefValue;
	strRetVal.TrimLeft();
	if( strRetVal.GetLength() < 3 || 'x' != tolower(strRetVal[1]) )
		return strtoul( strRetVal, NULL, 10 );

	strRetVal.Delete( 0, 2 );			//	16������ 0x ��ͷ
	return strtoul( strRetVal, NULL, 16 );
}

//////////////////////////////////////////////
//	����:
//		��DWORD��ȡ����
//	��ڲ���:
//		lpszKey				������
//		dwDefValue			ȱʡ��ֵ , 0
int CCommParameterBlock::GetAsInt(LPCSTR lpszKey, int nDefValue)
{
	ASSERT( lpszKey );
	CMyString strRetVal;
	CMyString strKey = lpszKey;
	strKey.MakeUpper();
	if( FALSE == Lookup( strKey, strRetVal ) )
		return nDefValue;
	strRetVal.TrimLeft();
	return atoi( strRetVal );
}

//////////////////////////////////////////////
//	����:
//		��DWORD��ȡ����
//	��ڲ���:
//		lpszKey				������
//		fDefValue			ȱʡ��ֵ , 0.0f
double CCommParameterBlock::GetAsDouble(LPCSTR lpszKey, double fDefValue)
{
	ASSERT( lpszKey );
	CMyString strRetVal;
	CMyString strKey = lpszKey;
	strKey.MakeUpper();
	if( FALSE == Lookup( strKey, strRetVal ) )
		return fDefValue;
	strRetVal.TrimLeft();
	return atof( strRetVal );
}

//////////////////////////////////////////////
//	����:
//		�����ڵķ�ʽ��ȡ
//	��ڲ���:
//		lpszKey				������
//		DefTime				ȱʡʱ��
//	���ز���:
//		time_t����ʱ��
//	ע��
//		ʱ���ʽ���� yyyy-mm-dd,hh:mm:ss
//		����������ʱ������ȱʡʱ��
time_t CCommParameterBlock::GetAsDate(LPCSTR lpszKey, time_t DefTime)
{
	ASSERT( lpszKey );
	CMyString strTmp;
	CMyString strKey = lpszKey;
	strKey.MakeUpper();
	if( FALSE == Lookup( strKey, strTmp ) )
		return DefTime;
	strTmp.TrimLeft();
	strTmp.Replace( '/', ' ' );			// '/' => '-'
	strTmp.Replace( ',', ' ' );
	strTmp.Replace( ':', ' ' );
	strTmp.Replace( '-', ' ' );

	int anValue[6];
    int i;
	for(i=0; i<5; i++)
	{
		int nPos = strTmp.Find( ' ' );
		ASSERT( nPos > 0 );
		if( nPos <= 0 )
			return DefTime;					//	����

		anValue[i] = atoi( strTmp.Left(nPos) );		// year
		strTmp.Delete( 0, nPos+1 );
		strTmp.TrimLeft();
	}
	anValue[5] = atoi( strTmp );
	for(i=0; i<6; i++)
	{
		if( anValue[i] < 0 )
			return DefTime;
	}

    struct tm TmpTime;
    memset( &TmpTime, 0, sizeof(TmpTime) );
	TmpTime.tm_sec = anValue[5];
    TmpTime.tm_min = anValue[4];
    TmpTime.tm_hour = anValue[3];
    TmpTime.tm_mday = anValue[2];
	TmpTime.tm_mon = anValue[1];
    TmpTime.tm_year = anValue[0];

    return mktime( &TmpTime );
}

//////////////////////////////////////////////
///	����:
///		��ȡ LARGE_INTEGER ���͵���ֵ
///	��ڲ���:
//		lpszKey				������
//		nDefValue			ȱʡ��ֵ
///	���ز���:
///		�з��ŵ� Large Integer
LONGLONG CCommParameterBlock::GetAsLargeInteger(LPCSTR lpszKey, LONGLONG nDefValue)
{
	LONGLONG RetVal;

	ASSERT( lpszKey );
	CMyString strTmp;
	CMyString strKey = lpszKey;
	strKey.MakeUpper();
	if( FALSE == Lookup( strKey, strTmp ) )
		RetVal = nDefValue;
	else
	{
		strTmp.TrimLeft();
		strTmp.TrimRight();
		if( strTmp.GetLength() > 20 )			//	����20λ��һ��������
			RetVal = nDefValue;
		else
        {
#ifdef _WIN32
			RetVal = _atoi64( strTmp );
#else
			RetVal = strtoll( strTmp, NULL, 10 );
#endif // WIN32
        }
	}

	return RetVal;
}

//////////////////////////////////////////////
///	2002.9.3 ���
///	����:
///		���޷��Ŵ�����
///	��ڲ���:
///		lpszKey				������
///		uDefValue			ȱʡֵ
///	���ز���:
///		ULARGE_INTEGER ���͵���ֵ
ULONGLONG CCommParameterBlock::GetAsULARGE_INTEGER(LPCSTR lpszKey, ULONGLONG uDefValue)
{
	ULONGLONG RetVal;

	ASSERT( lpszKey );
	CMyString strTmp;
	CMyString strKey = lpszKey;
	strKey.MakeUpper();
	if( FALSE == Lookup( strKey, strTmp ) )
		RetVal = uDefValue;
	else
	{
		strTmp.TrimLeft();
		strTmp.TrimRight();
		if( strTmp.GetLength() > 20 )			//	����20λ��һ��������
			RetVal = uDefValue;
		else if( strTmp.GetLength() > 2 && strTmp[0] == '0' && toupper(strTmp[1]) == 'X' )
		{
			strTmp.Delete( 0, 2 );				//	ɾ�� "0x"
#ifdef _WIN32
			CMyString strLow, strHigh;
			int nLen = strTmp.GetLength();
			if( nLen > 8 )
			{
				strHigh = strTmp.Left( nLen - 8 );
				strLow = strTmp.Mid( nLen - 8);
			}
			else
				strLow = strTmp;
            UINTER_LARGER TmpRetVal;
			TmpRetVal.LowPart = strtoul( strLow, NULL, 16 );
			TmpRetVal.HighPart = strtoul( strHigh, NULL, 16 );

            RetVal = TmpRetVal.QuadPart;
#else	// Linux
            RetVal = strtoull( strTmp, NULL, 16 );
#endif // _WIN32
		}
		else
        {
#ifdef _WIN32
			RetVal = _atoi64( strTmp );
#else	//	Linux
			RetVal = strtoll( strTmp, NULL, 16 );
#endif // _WIN32
        }
	}

	return RetVal;
}

BOOL CCommParameterBlock::SetAsLine(LPCSTR lpszLine)
{
	ASSERT( lpszLine );
	CMyString strTmp = lpszLine;
	ASSERT( FALSE == strTmp.IsEmpty() );
	return SetValue( strTmp );
}

BOOL CCommParameterBlock::SetValue(CMyString &strLine)
{
	strLine.TrimLeft();
	int nPos = strLine.FindOneOf( ":=" );	//	ֻҪ����һ������
	if( nPos < 0 )
		return FALSE;						//	������У�����
	CMyString strVarName = strLine.Left( nPos );
	strVarName.TrimLeft();
	strVarName.TrimRight();
	strVarName.MakeUpper();
	if( strVarName.IsEmpty() )				//	������������Ϊ���ַ���
	{
		ASSERT( FALSE );					//	����û�б�����
		return FALSE;
	}

	CMyString strParameter = strLine.Mid( nPos + 1 );
	strParameter.TrimLeft();
	strParameter.TrimRight();

	SetAt( strVarName, strParameter );			//	����ֵ
	return TRUE;
}

BOOL CCommParameterBlock::SetAsString(LPCSTR lpszVarName, LPCSTR lpszValue)
{
	ASSERT( lpszVarName && lpszValue );
	CMyString strTmp;
	strTmp.Format( "%s : %s", lpszVarName, lpszValue );
	return SetValue( strTmp );
}

BOOL CCommParameterBlock::SetAsDWORD(LPCSTR lpszVarName, DWORD dwValue)
{
	ASSERT( lpszVarName );
	CMyString strTmp;
	strTmp.Format( "%s : 0x%X", lpszVarName, dwValue );
	return SetValue( strTmp );
}

BOOL CCommParameterBlock::SetAsInt(LPCSTR lpszVarName, int nValue)
{
	ASSERT( lpszVarName );
	CMyString strTmp;
	strTmp.Format( "%s : %d", lpszVarName, nValue );
	return SetValue( strTmp );
}

BOOL CCommParameterBlock::SetAsDate(LPCSTR lpszVarName,time_t t)
{
	ASSERT( lpszVarName && t >=0 );
    CMyString strTmp;

#ifdef _WIN32
	CTime tmp( t );
	strTmp.Format("%s : %s", lpszVarName, tmp.Format( "%Y-%m-%d %H:%M:%S" ) );
#else  // Linux
	struct tm * pTmpTime = localtime( &t );
    strTmp.Format("%s : %04d-%2d-%d,%02d:%02d:%02d", pTmpTime->tm_year,\
    		pTmpTime->tm_mon, pTmpTime->tm_mday, pTmpTime->tm_hour,\
            pTmpTime->tm_min, pTmpTime->tm_sec );
#endif // _WIN32

	return SetValue( strTmp );
}

BOOL CCommParameterBlock::SetAsLargeInteger(LPCSTR lpszVarName, LONGLONG nValue)
{
	ASSERT( lpszVarName );
	if( NULL == lpszVarName )
		return FALSE;
	char szTmp[100];
#ifdef _WIN32
	_i64toa( nValue, szTmp, 10 );
#else // Linux
	sprintf( szTmp, "%lli", nValue );
#endif // _WIN32
	return SetAsString( lpszVarName, szTmp );
}

BOOL CCommParameterBlock::SetAsULargeInteger(LPCSTR lpszVarName, ULONGLONG Value)
{
	ASSERT( lpszVarName );
	if( NULL == lpszVarName )
		return FALSE;

	char szTmp[100];

#ifdef _WIN32
	ULARGE_INTEGER TmpValue;
	TmpValue.QuadPart = Value;
	sprintf( szTmp, "%s : 0x%X%08X",lpszVarName, TmpValue.HighPart, TmpValue.LowPart );
#else // Linux
	sprintf( szTmp, "%s : 0x%llX",lpszVarName, Value );
#endif // _WIN32

	return SetValue( szTmp );
}

//------------------------------------------
// Function:
//		Clone one object
// Input Parameter:
//		None
// Output Parameter:
//		NULL		   	failed
//		else			succ
ICommParameterBlock * CCommParameterBlock::CloneBlock()
{
	CCommParameterBlock * pNewItem = new CCommParameterBlock;
    if( NULL == pNewItem )
    	return NULL;
	pNewItem->AddRef();
	pNewItem->operator =( *this );
    return static_cast<ICommParameterBlock*>( pNewItem );
}

//------------------------------------------
// Function:
//
// Input Parameter:
//
// Output Parameter:
//
LPCSTR CCommParameterBlock::GetBlockName()
{
	return (LPCSTR)m_strBlockName;
}


//------------------------------------------
// Function:
//
// Input Parameter:
//
// Output Parameter:
//
CMyString CCommParameterBlock::GetSectionHead(CArchive & ar)
{
	CMyString strTmp;
	while(TRUE)
	{												//	�ȶ�ȡ��ͷ
		if( FALSE == ar.ReadString( strTmp ) )
			return strTmp;
		strTmp.TrimLeft();
		if( strTmp.IsEmpty() || ';' == strTmp[0] )
			continue;								// ���л�ע����
		if( '<' != strTmp[0] )
        {
        	strTmp.ReleaseBuffer( 0 );
			return strTmp;
        }
		strTmp.Delete( 0 );		//	ɾ�� '<'
        strTmp.TrimRight();
		int nLen = strTmp.GetLength();
		if( nLen )
			strTmp.Delete( nLen-1 );		//	ɾ�� '>'
		strTmp.TrimLeft();
		strTmp.TrimRight();
        if( strTmp.IsEmpty() )
        	continue;
		break;
	}
	return strTmp;
}

//	����������ȡ����
bool CCommParameterBlock::LoadFromStream(CArchive &ar, BOOL bGetSectionHead )
{
	if( bGetSectionHead )
    {
		m_strBlockName = GetSectionHead( ar );
        if( m_strBlockName.IsEmpty() )
        	return false;
    }

	CMyString strTmp;
	while( TRUE )
	{
		if( FALSE == ar.ReadString( strTmp ) )
			return false;
		strTmp.TrimLeft();
		if( strTmp.IsEmpty() || ';' == strTmp[0] )
			continue;								// ���л�ע����
		if( '<' == strTmp[0] )
		{
#ifdef _DEBUG
			CMyString strT = m_strBlockName;
			strT.MakeUpper();
			strTmp.MakeUpper();
			ASSERT( strTmp.Find( strT ) >= 0 );
#endif // _DEBUG
			return true;  							//	������//	������
		}
		strTmp.TrimLeft();
		SetValue( strTmp );
	}
}


//	���浽��������
bool CCommParameterBlock::WriteToStream(CArchive &ar, BOOL bWriteEndFlag /*= TRUE*/ )
{
	POSITION pos = GetStartPosition();
	if( NULL == pos )
		return false;								//	��

	CMyString strTmp;
	strTmp.Format( "<%s>\r\n", m_strBlockName );
	ar.WriteString( strTmp );				//	���ͷ

	CMyString strVarName;
	CMyString strValue;
	while( pos )
	{
		GetNextAssoc( pos, strVarName, strValue );
		strTmp.Format( "  %s : %s\r\n", strVarName, strValue );
		ar.WriteString( strTmp );
	}

	if( bWriteEndFlag )
	{
		strTmp.Format( "</%s>\r\n", m_strBlockName );
		ar.WriteString( strTmp );				//	���β
	}
    return true;
}

CCommParameterBlock & CCommParameterBlock::operator=( const CCommParameterBlock & src )
{
	//	need to be implemented
    m_GetAsStringRetVal = src.m_GetAsStringRetVal;
    m_strBlockName = src.m_strBlockName;

    RemoveAll();
    POSITION pos = src.GetStartPosition();
    CMyString strVarName;
    CMyString strValue;

#ifdef _DEBUG
	int nCount = src.GetCount();
#endif // _DEBUG

    while( pos )
    {
    	src.GetNextAssoc( pos, strVarName, strValue );
        SetAt( strVarName, strValue );
    }

#ifdef _DEBUG
	assert( nCount == GetCount() );
    pos = src.GetStartPosition();
    CMyString strValue1;
    while( pos )
    {
		src.GetNextAssoc( pos, strVarName, strValue );

        assert( Lookup( strVarName, strValue1 ) );
        assert( strValue1 == strValue );
    }
#endif //_DEBUG

	return *this;
}


//////////////////////////////////////////////////////////////////////
// class CCommParamParser Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCommParamParser::CCommParamParser()
{
	m_nRefTimes = 0;
}

CCommParamParser::~CCommParamParser()
{

}

void CCommParamParser::Serialize( CArchive& ar )
{
	if( ar.IsLoading() )
		LoadFromStream( ar );
	else
	{
		m_strBlockName = "Parameter";
		if( FALSE == IsEmpty() )
			WriteToStream( ar, FALSE );
		else
		{											//	2002.7.27 ��ӣ���û�в�������ǿ�����һ��<Parameter>
			CMyString strTmp;
			strTmp.Format( "<%s>\r\n", m_strBlockName );
			ar.WriteString( strTmp );				//	���ͷ
		}
		int nCount = m_ParameterArray.GetSize();
		for(int i=0; i<nCount; i++)
		{
			m_ParameterArray[i].WriteToStream( ar );
		}
		ar.WriteString( "</Parameter>" );
	}
}

//////////////////////////////////////////////
//	����:
//		��ȡ����Ԫ�ظ���
int CCommParamParser::GetSize()
{
	return m_ParameterArray.GetSize();
}

//////////////////////////////////////////////
//	����:
//		��������Ԫ��
//	��ڲ���:
//		nNo		��ţ�����С�� GetSize �ķ���ֵ
//	���ز���:
//		����Ԫ��
ICommParameterBlock * CCommParamParser::ElementAt(int nNo)
{
	int nCount = m_ParameterArray.GetSize();
    if( 0 == nCount )
    	return NULL;
	ASSERT( nNo < nCount );
	if( nNo >= nCount )
		nNo = 0;
	m_ParameterArray[nNo].AddRef();
	return static_cast<ICommParameterBlock*>( &m_ParameterArray[nNo] );
}

//////////////////////////////////////////////
//	����:
//		��������Ԫ��
//	��ڲ���:
//		lpszArrayName ��������
//	���ز���:
//		NULL		ʧ��
//		����		����Ԫ��
ICommParameterBlock * CCommParamParser::ElementAt(LPCSTR lpszArrayName)
{
	int nCount = m_ParameterArray.GetSize();
	for(int i=0; i<nCount; i++)
	{
		if( 0 == m_ParameterArray[i].m_strBlockName.CompareNoCase( lpszArrayName ) )
        {
	        m_ParameterArray[i].AddRef();
			static_cast<ICommParameterBlock*>( &m_ParameterArray[i] );
        }
	}
	return NULL;
}

//	����������������
bool CCommParamParser::LoadFromStream(CArchive &ar)
{
	RemoveAll();
	m_ParameterArray.RemoveAll();

	m_strBlockName = GetSectionHead(ar);
#ifdef _DEBUG
	if( m_strBlockName.Compare( "Parameter" ) )
		TRACE("Find one header = %s\n", m_strBlockName );
#endif // _DEBUG

	CMyString strTmp;
	while( ar.ReadString( strTmp ) )
	{
		strTmp.TrimLeft();
		if( strTmp.IsEmpty() || ';' == strTmp[0] )
			continue;								// ���л�ע����
		if( '<' != strTmp[0] )
		{
			SetValue( strTmp );
			continue;
		}
		strTmp.Delete( 0 );
		strTmp.TrimLeft();
		if( strTmp.IsEmpty() )
			continue;
		else if( '/' != strTmp[0] )
		{
			int nLen = strTmp.GetLength();
			if( nLen )
				strTmp.Delete( nLen-1 );		//	ɾ�� '>'
			strTmp.TrimLeft();
			strTmp.TrimRight();

			CCommParameterBlock blk;
			blk.m_strBlockName = strTmp;
			blk.LoadFromStream( ar );
			m_ParameterArray.Add( blk );
			continue;
		}
		else
			return true;				   		//	�������˳�
	}
	RemoveAll();
	m_ParameterArray.RemoveAll();
	return false;
}

//	��ָ�����ļ��ж�ȡ���ݽ��з���
BOOL CCommPaamParser::Parse(CFile *pFile)
{
	ASSERT( pFile );

	CArchive loadArchive( pFile, CArchive::load | CArchive::bNoFlushOnDelete );
	try
	{
		Serialize( loadArchive );
		loadArchive.Close();
	}
	catch( ... )
	{
		loadArchive.Abort();
		return FALSE;
	}

	if( IsEmpty() && 0 == m_ParameterArray.GetSize() )
		return FALSE;

	return TRUE;
}

//	�Ӹ������ڴ��з���
BOOL CCommParamParser::Parse(PBYTE pBuf, DWORD dwBufLen)
{
	CMemFile f;
	f.Attach( pBuf, dwBufLen );
	BOOL bRetVal = Parse( &f );
	f.Detach();
	return bRetVal;
}


BOOL CCommParamParser::Parse(LPCSTR lpszFileName)
{
	CFile f;
	if( FALSE == f.Open( lpszFileName, CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite ) )
		return FALSE;
	BOOL bRetVal = Parse( &f );

	try
	{
		f.Close();
	}
	catch( ... )
	{
		f.Abort();
		return FALSE;
	}

	return bRetVal;
}


//	д��ָ�����ļ���
BOOL CCommParamParser::Write(CFile *pFile)
{
	ASSERT( pFile );

	CArchive loadArchive( pFile, CArchive::store| CArchive::bNoFlushOnDelete );
	try
	{
		Serialize( loadArchive );
		loadArchive.Close();
	}
	catch ( ... )
	{
		loadArchive.Abort();
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////
//	����:
//		ɾ��ָ��������
//	��ڲ���:
//		nNo				�������
//	���ز���:
//		��
void CCommParamParser::RemoveAt(int nNo)
{
	ASSERT( nNo >=0 && nNo < GetSize() );
	if( nNo < 0 || nNo >= GetSize() )
	{
		ASSERT( FALSE );				//	��������
		return;
	}
	m_ParameterArray.RemoveAt( nNo );
}

//////////////////////////////////////////////
//	����:
//		�жϲ������Ƿ���������
//	��ڲ���:
//		pBuf			��������ַ
//		dwLen			����������
//	���ز���:
//		TRUE			����
//		FALSE			����Ҫ����
BOOL CCommParamParser::IsParameterBlockEnd(PBYTE pBuf, DWORD dwLen)
{
	ASSERT( pBuf && dwLen );
	if( NULL == pBuf || dwLen < 25 )		//	<Parameter>\r\n</Parameter>������Ҫ25�ַ�
		return FALSE;

	PBYTE pLastNewLine = pBuf + dwLen;
	PBYTE pEndBuf = pLastNewLine - 1;
	char szTmp[100];
	while( pEndBuf > pBuf )
	{
		if( 0xA != *pEndBuf )				//	�ҵ���һ��
		{
			pEndBuf --;
			continue;
		}
		int nCount = pLastNewLine - pEndBuf - 1;
		pLastNewLine = pEndBuf;
		if( nCount <= 0 )
		{
			pEndBuf --;
			continue;
		}
		if( nCount > sizeof(szTmp)-1 )
			nCount = sizeof(szTmp)-1;
		memcpy( szTmp, pEndBuf+1, nCount );
		szTmp[ nCount ] =0;
		pEndBuf --;
		char * pszLine = szTmp;
		while( *pszLine && *pszLine <= 0x20 )
			pszLine ++;
        CMyString strTmp( pszLine );
        strTmp.MakeUpper();
		if( 0 == strncmp( strTmp, "</PARAMETER>", 12 ) )
			return TRUE;
	}
	return FALSE;
}


long CCommParamParser::AddRef(void)
{
	return InterlockedIncrement( &m_nRefTimes );
}

long CCommParamParser::Release(void)
{
	long nRetVal = InterlockedDecrement( &m_nRefTimes );
    if( nRetVal )
    	return nRetVal;

    delete this;
    return 0;
}

DWORD CCommParamParser::QueryInterface( REFIID iid, void **ppvObject)
{
#ifdef _DEBUG
	assert( ppvObject );
#endif //_DEBUG
    if( NULL == ppvObject )
    	return 0x80004003;		// E_POINTER

	AddRef();
	if( iid == IID_ICOMMPARAMPARSER )
    {
    	*ppvObject = static_cast<ICommParamParser*>( this );
        return 0;
    }
    else
		return CCommParameterBlock::QueryInterface( iid, ppvObject );
}

ICommParamParser * CCommParamParser::CloneParser()
{
#ifdef _DEBUG
	assert( false );
#endif //_DEBUG
	return NULL;
}

int CCommParamParser::Write( PBYTE pBuf, DWORD dwLen )
{
#ifdef _DEBUG
	assert( false );
#endif //_DEBUG
	return 0;
}

void CCommParamParser::Write( LPCSTR lpszFileName )
{
	#ifdef _DEBUG
		assert( false );
	#endif //_DEBUG

}

