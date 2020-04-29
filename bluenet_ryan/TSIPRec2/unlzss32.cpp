// UNLZSS32.cpp: implementation of the CUNLZSS32 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "unlzss32.h"

#include "crc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUNLZSS32::CUNLZSS32()
{
	m_pOutBufAutoAlloc = NULL;					//	�Զ�������ڴ�
}

CUNLZSS32::~CUNLZSS32()
{
	if( m_pOutBufAutoAlloc )
		delete[] m_pOutBufAutoAlloc;
}

//	��һ��ѹ���ļ����ŵ�CUNLZSS ����
//	��ڲ���
//		nFileLen				�ļ�����
//		pBuf					������
//	���ز���
//		�ļ�����
int CUNLZSS32::Attach(int nFileLen, PBYTE pBuf)
{
	CUnCompressObj::Detach();
	if( CUnCompressObj::IsCompress(nFileLen, pBuf) == FALSE )
		return 0;					//	����ѹ���ļ�
	CUnCompressObj::Attach( nFileLen, pBuf );
	return 1;
}

//	ȡ�ļ�����
int CUNLZSS32::GetFileNum()
{	
	if( GetHeader() )							
		return 1;
	else
		return 0;					//	û������
}

//	��ѹһ���ļ�
//	��ڲ���
//		nFileNo					�ļ����
//		outfStatus				����ļ�״̬
//		pDstBuf					�û�ָ��������,��NULL,�Զ������ڴ�
//	���ز���
//		��ѹ����ļ�������
//		NULL					ʧ��
PBYTE CUNLZSS32::DecodeOneFile(int nFileNo, CFileStatus &outfStatus, PBYTE pDstBuf)
{
	ASSERT( nFileNo == 0 );						//	��Ϊֻ��һ���ļ�

	FreeMemory();	

	PTSDBCOMPRESSHEAD pHeader = GetHeader();
	ASSERT( pHeader );
	if( CCRC::GetCRC32( pHeader->m_dwFileLen,GetDataBuf() ) != pHeader->m_dwFileCRC32 )
		return NULL;							//	CRC ����

	SetDstBuffer( pHeader->m_dwOrgFileLen, pDstBuf );		//	��������ļ�������
	Decode();									//	��ѹ�ļ�
	ASSERT( pHeader->m_dwOrgFileLen == (DWORD)m_nOutDataLen );		//	���Խ�ѹ�Ƿ���ȷ
	ASSERT( pHeader->m_dwOrgFileCRC32 == CCRC::GetCRC32(pHeader->m_dwOrgFileLen,m_pDstDataBuf) );
	m_pOutBufAutoAlloc = ReleaseDstBuffer( outfStatus.m_size );
	if( NULL == pDstBuf )
		return m_pOutBufAutoAlloc;

	m_pOutBufAutoAlloc = NULL;					//	ʹ���ⲿ���ڴ�
	return pDstBuf;
}

//	ȡһ���ļ�״̬
//	��ڲ���
//		nFileNo					�ļ����
//		outfStatus				״̬
//	���ز���
//		�ļ�����
//		0						ʧ��
int CUNLZSS32::GetFileInfo(int nFileNo, CFileStatus &outfStatus)
{
	ASSERT( nFileNo == 0 );					//	ֻ֧�� 1 ���ļ�
	PTSDBCOMPRESSHEAD pHeader = GetHeader();
	ASSERT( pHeader );
	outfStatus.m_szFullName[0] = 0;			//	�ļ���,���Ժ�ʱ��μ� TSDBFILEHEAD
	outfStatus.m_size = pHeader->m_dwOrgFileLen;
	return pHeader->m_dwOrgFileLen;
}


//	ȡ��ѹ��������
DWORD CUNLZSS32::GetCompressMethod()
{
	return TSDBCOMPRESS_METHOD_LZSSV100;
}

//	��ȡ�汾��
int CUNLZSS32::GetDecoderVersion()
{
	return CURRENT_UNLZSS_VERSION*0x100 + UNLZSS_MINOR_VER;
}

//	�ͷ��ڴ�
void CUNLZSS32::FreeMemory()
{
	if( m_pOutBufAutoAlloc )
		delete m_pOutBufAutoAlloc;
	m_pOutBufAutoAlloc = NULL;
}

//	����
void CUNLZSS32::Decode()
{
	short i, j, k, r, c;
	WORD flags;
	BYTE text_buf[N + F - 1];
	BYTE out_text_buf[ OUT_TEXT_BUF_SIZE ];		//	���������
	register int out_text_ptr = 0;						//	������ֽ���

	PTSDBCOMPRESSHEAD pHeader = GetHeader();
	ASSERT( pHeader );

	int nOrgFileLen = pHeader->m_dwOrgFileLen;
	int nCmpFileLen = pHeader->m_dwFileLen;
	int OutFileLen = 0;

	memset( text_buf,0x20,N - F);				//	Ԥ������

	r = N - F;  
	flags = 0;

	for(int nByteDecode=0; ; ) 
	{
		if ( ((flags >>= 1) & 256) == 0 ) 
		{
			c = ReadOneByte();
			nByteDecode ++;
			flags = c | 0xff00;					/* uses higher byte cleverly */
		}										/* to count eight */
		if( nByteDecode >= nCmpFileLen )
			break;
		if (flags & 1)							//	��������ļ�
		{
			c = ReadOneByte();
			nByteDecode ++;
			out_text_buf[out_text_ptr++] = (BYTE)c;	//	��һ���ֽ�
			text_buf[r++] = (BYTE)c;  
			r &= (N - 1);
		} 
		else 
		{
			i = ReadOneByte();
			j = ReadOneByte();
			nByteDecode += 2;
			i &= 0xff;
			i |= ((j & 0xf0) << 4);  
			j = (j & 0x0f) + THRESHOLD;
			for (k = 0; k <= j; k++) 
			{
				c = text_buf[(i + k) & (N - 1)];
				out_text_buf[out_text_ptr++] = (BYTE)c;	//	��һ���ֽ�
				text_buf[r++] = (BYTE)c; 
				r &= (N - 1);
			}
		}												
		if( out_text_ptr >= ( OUT_TEXT_BUF_SIZE - 2 * F ) )
		{											//	��������ļ�����
			Write( out_text_buf, out_text_ptr );
			OutFileLen += out_text_ptr;
			out_text_ptr = 0;
			if( OutFileLen >= nOrgFileLen )			//	�ļ���
				break;
		}
	}

	if( out_text_ptr )					//	��ʣ�µ��ֽڱ���
		Write( out_text_buf, out_text_ptr );
}
