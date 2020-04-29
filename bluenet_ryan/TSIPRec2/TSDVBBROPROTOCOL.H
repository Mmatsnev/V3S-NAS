//	DVB ����Э��

#ifndef __TSDVB_MULTICAST_BRO_PROTOCOL_20010903__
#define __TSDVB_MULTICAST_BRO_PROTOCOL_20010903__

// 2016.4.23 CYJ Add
#include <stdint.h>

typedef enum
{
	TSDVB_OFFSET = 0,						//	ͨ�� Offset ȷ��ƫ��
	TSDVB_PAGELEN_864,						//	ÿҳ���� = 864
	TSDVB_PAGELEN_1024,						//	ÿҳ���� = 1024
}TSDVBMULTICASTBRO_MODE;

#pragma pack(push,1)						//	��һ���ֽڶ������

typedef enum
{
	EPFI_SUBFILE_ID = 0,					//	���ļ����
}EXTPARAM_FINCTION_INDEX;

typedef struct tagEXTPARAM_SUBFILEID
{											//	���ļ�������m_wFunctionIndex = 0
	DWORD	m_dwSubFileID;					//	���ļ���ţ��� 0 ��ʼ
	DWORD	m_dwTotalSubFileCount;			//	���ļ��ܸ�����Ϊ0��Ч
	BYTE	m_abyReserved[2];				//	���������� �� 0
}EXTPARAM_SUBFILEID,*PEXTPARAM_SUBFILEID;

//	DVB Multicast ����
// 2016.4.23 CYJ Modify, changed time_t to uint32_t
typedef struct tagTSDVBMULTICASTPACKET0
{
	DWORD		m_dwHeaderCRC32;				//	����ͷ CRC32
	WORD		m_cbSize;						//	����С
	uint32_t	m_PacketTime;					//	���ʱ��, ����ʱ�䷽��:���  Bit31 = 1  ���������� * 256 + ���������
	TSDBCHANNEL	m_chFile;
	union
	{
		char		m_cFileName[13];			//	�ļ���, �����ַ��������� '\0'
		struct
		{
			BYTE	m_byReserved_0;				//	����������Ϊ0����Ϊ 0 ��ʾ���ַ���
			BYTE	m_byXorChkSum;				//	��11�ֽڵ�XOR Check Sum
			BYTE	m_byFunctionIndex;			//	�����������μ� EXTPARAM_FINCTION_INDEX
			union
			{									//	ֻ����10�ֽ�
				EXTPARAM_SUBFILEID	m_SubFileID;		// ���ļ����
			};
		}m_ExtParam;
	};
	WORD		m_wPageLen:12;					//	ҳ���ȣ���� 4K
	WORD		m_wMode:4;						//	�ļ����ȱ�ʾ��ʽ
	union
	{
		DWORD	m_dwFileLen;					//	�ļ�����, m_dwFileLenMode = TSDVB_OFFSET
		struct
		{
			DWORD	m_dwFTotalLen:21;			//	�ܹ����ļ�ҳ��,m_wMode ȷ����m_wMode != TSDVB_OFFSET
			DWORD	m_dwPageNo:11;				//	�̶���ҳ����
		};
	};
	union
	{
		DWORD	m_dwPageOfs;					//	��ҳ�������ļ��е�ƫ��,TSDVB_OFFSET
		DWORD	m_dwSysCodeIndex;				//	����ϵͳ����,m_wMode != TSDVB_OFFSET
	};
	DWORD		m_dwPageCRC32;					//	ҳ CRC32
public:
	DWORD	GetFileLen()
	{
		if( TSDVB_OFFSET == m_wMode )
			return m_dwFileLen;
		else
			return m_dwFTotalLen;
		ASSERT( FALSE );
		return 0;
	};
	DWORD	GetPageOfsInFile()
	{
		switch( m_wMode )
		{
		case TSDVB_OFFSET:
			return m_dwPageOfs;
		case TSDVB_PAGELEN_864:
			return m_dwPageNo * 864;
		case TSDVB_PAGELEN_1024:
			return m_dwPageNo * 864;
		default:
			ASSERT( FALSE );
		}
		return 0;
	};
	WORD	GetPageLen()
	{
		switch( m_wMode )
		{
		case TSDVB_OFFSET:
			return 1024;
		case TSDVB_PAGELEN_864:						//	ÿҳ���� = 864
			return 864;
		case TSDVB_PAGELEN_1024:					//	ÿҳ���� = 1024
			return 1024;
		default:
			ASSERT( FALSE );
			return 1024;
		}
	};

	///-------------------------------------------------------
	/// 2003-4-3
	/// ���ܣ�
	///		��ȡ��չ�������
	/// ��ڲ�����
	///		��
	/// ���ز�����
	///		>=0					��չ�������
	///		<0					ʧ��
	int GetExtParamFunctionIndex()
	{
		if( m_ExtParam.m_byReserved_0 )
			return -1;			//	������չ
		BYTE byXorValue = 0;
		for(int i=2; i<13; i++)
		{
			byXorValue ^= (BYTE)m_cFileName[i];
		}
		if( byXorValue != m_ExtParam.m_byXorChkSum )
			return -2;
		return m_ExtParam.m_byFunctionIndex;
	}
}TSDVBMULTICASTPACKET0,*PTSDVBMULTICASTPACKET0;					//	48 �ֽ�

#pragma pack(pop)						//	��һ���ֽڶ������



#endif // __TSDVB_MULTICAST_BRO_PROTOCOL_20010903__