// UnCompressObj.cpp: implementation of the CUnCompressObj class.
//
//////////////////////////////////////////////////////////////////////
//	2001.8.15	��Ӵ��麯�� FreeMemory�������ͷ��ڴ�

#include "stdafx.h"
#include "UnCompressObj.h"

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

CUnCompressObj::CUnCompressObj()
{
	m_pTSDBCmpHead = NULL;
	m_pDstDataBuf = NULL;								//	Ŀ������
	m_nOutDataLen = 0;									//	�����������С
}

CUnCompressObj::~CUnCompressObj()
{
	if( m_pDstDataBuf )
		delete m_pDstDataBuf ;
	m_pDstDataBuf = NULL;
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
PBYTE CUnCompressObj::DecodeOneFile(int nFileNo,CFileStatus & outfStatus,PBYTE pDstBuf)
{
	return NULL;
}

//	��ȡ�ļ�����
//	��ڲ���
//		nFileNo						�ļ����
//		outfStatus					����ļ�״̬
//	���ز���
//		�ļ�����
int CUnCompressObj::GetFileInfo(int nFileNo,CFileStatus & outfStatus)
{
	return 0;
}

//	��ȡѹ���ļ��е��ļ�����
//	���ز���
//		�ļ�����
int CUnCompressObj::GetFileNum()
{
	return 0;
}

//	��ָ�����ȼ������������ݸ��ŵ�������
//	��ڲ���
//		nFileLen					�ļ�����
//		pBuf						���ݻ�����
//	���ز���
//		�ļ�����
int CUnCompressObj::Attach(int nFileLen,PBYTE pBuf)
{
	ASSERT( nFileLen && pBuf );
	m_pTSDBCmpHead = (PTSDBCOMPRESSHEAD) pBuf;
	m_pSrcDataBuf = GetDataBuf();
	m_nSrcDataLen = m_pTSDBCmpHead->m_dwFileLen;
	m_nDataRead = 0;
	return 1;
}

//	ȡѹ������
DWORD CUnCompressObj::GetCompressMethod()
{
	return 0;
}

//	��ȡ��ѹ����İ汾��
int CUnCompressObj::GetDecoderVersion()
{
	return 0;
}

//	�������
void CUnCompressObj::Detach()
{
	m_pTSDBCmpHead = NULL;
}

//	��ȡѹ������ͷ
PTSDBCOMPRESSHEAD CUnCompressObj::GetHeader()
{
	ASSERT( m_pTSDBCmpHead );
	return m_pTSDBCmpHead ;
}

//	��ȡ���ݻ�����
PBYTE CUnCompressObj::GetDataBuf()
{
	ASSERT( m_pTSDBCmpHead  );
	PBYTE pRet = (PBYTE)m_pTSDBCmpHead;
	pRet += m_pTSDBCmpHead->m_cbSize;
	return pRet;
}

//	�������������
//	��ڲ���
//		nDstBufLen						��������С
//		pBuffer							���������
//	���ز���
//		��
//	ע:
//		�� pBuffer = NULL, ���Զ������������, �������߱������,�ͷŸ��ڴ�
void CUnCompressObj::SetDstBuffer(int nDstBufLen,PBYTE pBuffer)
{
	ASSERT( nDstBufLen );						//	������������С
	if( pBuffer == NULL )
		m_pDstDataBuf = new BYTE [ nDstBufLen ];
	else
		m_pDstDataBuf = pBuffer;				//	Ŀ������

	m_pOutDataBufPtr = m_pDstDataBuf;			//	��������ݻ�����ָ��
	m_nOutDataLen = 0;							//	ʵ�������������С
	m_nDstBufSize = nDstBufLen;					//	�����������С
}

//	��ȡ���������
//	��ڲ���
//		outfLen							�������ļ�����
//	���ز���
//		���������
PBYTE CUnCompressObj::ReleaseDstBuffer(long &outfLen)
{
	outfLen = m_nOutDataLen;	
	PBYTE pBuffer = m_pDstDataBuf;
	m_pDstDataBuf = NULL;
	return pBuffer;
}

//	��ȡ 1 �ֽ�
//	���ز���
//		EOF								�ļ�����
//		����							��������
int CUnCompressObj::ReadOneByte()
{
	ASSERT( m_pSrcDataBuf );
	if( m_nDataRead >= m_nSrcDataLen )
		return EOF;
	m_nDataRead ++;
	return *m_pSrcDataBuf++;
}

//	��� 1 �ֽ�
//	��ڲ���
//		byData							����
void CUnCompressObj::OutputOneByte(BYTE byData)
{
	ASSERT( m_pOutDataBufPtr );
	if( m_nOutDataLen < m_nDstBufSize && m_pOutDataBufPtr )
	{
		*m_pOutDataBufPtr ++ = byData;
		m_nOutDataLen ++;
	}
}

//	����ָ���ֽ�����
//	��ڲ���
//		pBuf							���ݻ�����
//		nCount							�ֽ���
void CUnCompressObj::Write(PBYTE pBuf, int nCount)
{
	int nLen = m_nDstBufSize - m_nOutDataLen;
	ASSERT( nLen >= 0 );
	if( nLen == 0 || m_pOutDataBufPtr == NULL )
		return;
	if( nLen > nCount )
		nLen = nCount;
	memcpy( m_pOutDataBufPtr, pBuf, nLen );
	m_pOutDataBufPtr += nLen;
	m_nOutDataLen += nLen;
}

//	�ж��Ƿ�ѹ��
//	���ز���
//		TRUE							ѹ��
//		FALSE							��ѹ��
BOOL CUnCompressObj::IsCompress(int nLen, PBYTE pBuffer)
{
	ASSERT( nLen && pBuffer );
	PTSDBCOMPRESSHEAD pHead = (PTSDBCOMPRESSHEAD)pBuffer;
	if( pBuffer == NULL || nLen<sizeof(TSDBCOMPRESSHEAD) || nLen < pHead->m_cbSize )
		return FALSE;
	if( pHead->m_CLSID != CLSID_TSDBCOMPRESSHEADER )
		return FALSE;
	if( pHead->m_dwHeaderCRC32 != \
		CCRC::GetCRC32( pHead->m_cbSize - (sizeof(TSDBCOMPRESSHEAD)-offsetof(TSDBCOMPRESSHEAD,m_dwMethod)), (PBYTE)&pHead->m_dwMethod ) )
	{
		return FALSE;
	}
	return TRUE;
}
