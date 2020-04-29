// UnCmpMgr.cpp: implementation of the CUnCmpMgr class.
//
//////////////////////////////////////////////////////////////////////
//	2001.8.15	��� GetSvr() ����
//

#include "stdafx.h"
#include "Resource.h"
#include "UnCmpMgr.h"
#include "UnCompressObj.h"
#include "Tsdb.h"

#ifdef _WIN32
  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif
#endif //_WIN32

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUnCmpMgr::CUnCmpMgr()
{
	m_pSvr = NULL;
}

CUnCmpMgr::~CUnCmpMgr()
{

}

//	�ж��Ƿ�ѹ��
//	��ڲ���
//		nLen								��������С
//		pBuffer								������
//	���ز���
//		TRUE								�ɹ�
//		FALSE								ʧ��
BOOL CUnCmpMgr::IsCompress(int nLen, PBYTE pBuffer)
{
	return CUnCompressObj::IsCompress(nLen, pBuffer );
}

//	��ѹ����һ���ļ�
//	��ڲ���
//		nFileNo						�ļ������, ��Ӧ���� GetFileNum() ��ֵ
//		outfStatus					����ļ�����
//		pDstBuf						�û�ָ�������������, �� pDstBuf = NULL, ���Զ����仺����, �����߱���ɾ�����ڴ�
//									��������С������� GetFileInfo ��ȡ�ĳ��Ƚ��з���
//	���ز���
//		NULL						ʧ��
//		�� pDstBuf != NULL, �򷵻� pDstBuf
PBYTE CUnCmpMgr::DecodeOneFile(int nFileNo,CFileStatus & outfStatus,PBYTE pDstBuf)
{
	ASSERT( m_pSvr );
	return m_pSvr->DecodeOneFile( nFileNo, outfStatus, pDstBuf );
}

//	��ȡ�ļ�����
//	��ڲ���
//		nFileNo						�ļ����
//		outfStatus					����ļ�״̬
//	���ز���
//		�ļ�����
int CUnCmpMgr::GetFileInfo(int nFileNo,CFileStatus & outfStatus)
{
	ASSERT( m_pSvr );
	return m_pSvr->GetFileInfo( nFileNo, outfStatus );
}

//	��ȡѹ���ļ��е��ļ�����
//	���ز���
//		�ļ�����
int CUnCmpMgr::GetFileNum()
{
	ASSERT( m_pSvr );
	if( m_pSvr )
		return m_pSvr->GetFileNum();
	else
		return 0;
}

//	��ָ�����ȼ������������ݸ��ŵ�������
//	��ڲ���
//		nFileLen					�ļ�����
//		pBuf						���ݻ�����
//	���ز���
//		�ļ�����
//	ע:
//		���� TSDB ������ ARJ �� PKZIP �������ݱ���ȥ�� TSDBCOMPRESSHEAD �������, ��ԭ��������
int CUnCmpMgr::Attach(int nFileLen,PBYTE pBuf)
{
	ASSERT( nFileLen && pBuf );
	PTSDBCOMPRESSHEAD pHeader = (PTSDBCOMPRESSHEAD) pBuf;
	switch( pHeader->m_dwMethod )
	{
	case TSDBCOMPRESS_METHOD_LZHUFV100:
		m_pSvr = static_cast<CUnCompressObj*>(&m_LzhufSvr);
		break;

	case TSDBCOMPRESS_METHOD_LZSSV100:
		m_pSvr = static_cast<CUnCompressObj*>(&m_lzss );
		break;
	
	default:
		ASSERT( FALSE );
		return 0;
	}
	return m_pSvr->Attach( nFileLen, pBuf );
}

//	2001.8.15	���
//	��ȡ������ָ��
CUnCompressObj * CUnCmpMgr::GetSvr()
{
	ASSERT( m_pSvr );
	return m_pSvr;
}
