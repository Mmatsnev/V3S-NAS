// MyGetVariantOptionalVal.cpp: implementation of the CMyGetVariantOptionalVal class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyGetVariantOptionalVal.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//	��ȡ��������ַ��Ϣ
//	��ڲ���
//		pBuf				�����ַ������Ϊ BYTE * ����� һάSafeArray����
//		pdwBufLen			������������ȣ����� SafeArray ����
PBYTE CMyGetVariantOptionalVal::GetBufPtrFromVariant(VARIANT *pBuf, PDWORD pdwBufLen/*=NULL*/)
{
	ASSERT( pBuf );
	if( pdwBufLen )
		*pdwBufLen = 0;

	if( V_VT( pBuf) == VT_ERROR || V_ERROR(pBuf) == DISP_E_PARAMNOTFOUND || V_VT(pBuf)==VT_EMPTY )
		return NULL;			//	����

	if( pBuf->vt == (VT_UI1|VT_ARRAY|VT_BYREF) )
		return pBuf->pbVal;

	ASSERT( FALSE );
	AfxMessageBox("û��ʵ��");

	return NULL;
}

//	��ȡ����
int CMyGetVariantOptionalVal::GetVarValue(VARIANT &varData, int nDefValue)
{
	CComVariant varTmp( varData );
	if( V_VT( &varTmp) == VT_ERROR || V_ERROR( &varTmp) == DISP_E_PARAMNOTFOUND || V_VT(&varTmp)==VT_EMPTY )
		return nDefValue;

	::VariantCopyInd( &varTmp, &varTmp );
	varTmp.ChangeType( VT_I4 );
	return varTmp.lVal;
}

//	��ȡ�ַ���
CString CMyGetVariantOptionalVal::GetStringVarValue(VARIANT &varData, LPCSTR lpszDefValue)
{
	CString strRetVal;	

	CComVariant varTmp( varData );
	if( V_VT( &varTmp) == VT_ERROR || V_ERROR( &varTmp) == DISP_E_PARAMNOTFOUND || V_VT(&varTmp)==VT_EMPTY )
	{
		if( lpszDefValue )
			strRetVal = lpszDefValue;
		return strRetVal;
	}

	::VariantCopyInd( &varTmp, &varTmp );
	varTmp.ChangeType( VT_BSTR );
	strRetVal = varTmp.bstrVal;
	return strRetVal;
}
