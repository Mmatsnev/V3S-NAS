///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2005-5-12
///
///		��;��
///			ASP �ⲿ����
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӡ��ڲ�ʹ��!	  			     !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	��Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!			 !
///=======================================================

#ifndef __MYHTML_ASP_EXT_FUNCTION_20050512__
#define __MYHTML_ASP_EXT_FUNCTION_20050512__

#include <BScript/BScriptEngine.h>
class CHTMLParser;

typedef void (*PFN_OnModuleCreateDelete)( BSModule * pModule, bool bIsCreate, DWORD dwUserData );

extern "C" void MyHTMLRegisterModuleCallBack( CHTMLParser * pParser, PFN_OnModuleCreateDelete pFn, DWORD dwUserData );

#endif	// __MYHTML_ASP_EXT_FUNCTION_20050512__
