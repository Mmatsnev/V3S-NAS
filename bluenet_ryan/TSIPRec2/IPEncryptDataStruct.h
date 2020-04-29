///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2002-11-13
///
///=======================================================

#ifndef __IP_ENCRYPT_DATA_STRUCT_INCLUDE_20021113__
#define __IP_ENCRYPT_DATA_STRUCT_INCLUDE_20021113__

///--------------------------------------------------------
///	һ��Ϊͨ���ļ�����ר�õ� IP ���ܼ��ܡ�
///	������£�һ��Ϊ�ಥ/UDP����ʽ��
///	��ʱ���ܺ�����ݿ���ͨ��UDP�ķ�ʽ�ٴβ���

#define TS_IP_ENCRYPT_DATA_TAG	0x45495354		// 'TSIE', TongShi IP Encrypt
#define TS_IP_ENCRYPT_VERSION	0x100			//	��ǰ�ļ��ܰ汾

#pragma pack( push,1 )							//	һ���ֽڶ���

typedef struct tagTS_IP_ENCRYPT_STRUCT
{
	DWORD	m_dwTag;							//	Ӧ�õ��� TS_IP_ENCRYPT_DATA_TAG������һ������ͨ�ӵ�
	DWORD	m_dwHeadCRC32;						//	����ͷ�� CRC32
	WORD	m_wVersion;							//	�汾�ţ���ǰΪ 0x100
	WORD	m_wSrcDataLen;						//	ԭʼ����
	DWORD	m_dwSrcDataCRC32;					//	ԭʼ���ݵ�CRC32
	DWORD	m_dwSysCodeIndex;					//	�����õ�ϵͳ��������
	DWORD	m_dwDrvSN:24;						//	��������ʹ�õ����кţ�3 �ֽڵ�HEX����
	DWORD	m_dwReserved:8;						//	����������Ϊ 0
}TS_IP_ENCRYPT_STRUCT,*PTS_IP_ENCRYPT_STRUCT;






#pragma pack( pop )

#endif // __IP_ENCRYPT_DATA_STRUCT_INCLUDE_20021113__