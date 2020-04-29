//////////////////////////////////////////////////////////////////////////
//			TSDB ������Э��
//
//			����ͨ�������������ι�˾
//			������
//			1999.6.15
//
//			�޸ļ�¼
//------------------------------------------------------------------------
// 2000.11.2	TSDBLOCKHEAD ��ӳ�Ա���� m_dwOrgCRC32 �� m_dwOffset �ֶ�
//
//
/////////////////////////////////////////////////////////////////////////

#ifndef __TSDB_INCLUDE_19990615__
#define __TSDB_INCLUDE_19990615__

#include "crc.h"				//	CRC CLASS

// 2016.4.23 CYJ Add
#include <stdint.h>

#if _MSC_VER >= 1100
#undef  TSDB_DEFINE_GUID
#define TSDB_DEFINE_GUID EXTERN_GUID
#else
#define TSDB_DEFINE_GUID DEFINE_GUID
#endif

//	��������
//	���ļ� {57A72F61-B84E-11d2-B30C-00C04FCCA334}
TSDB_DEFINE_GUID(CLSID_TSDBMULFILEHEAD, 0x57a72f61, 0xb84e, 0x11d2, 0xb3, 0xc, 0x0, 0xc0, 0x4f, 0xcc, 0xa3, 0x34);

//	�����ļ� {57A72F62-B84E-11d2-B30C-00C04FCCA334}
TSDB_DEFINE_GUID(CLSID_TSDBFILEHEADER,  0x57a72f62, 0xb84e, 0x11d2, 0xb3, 0xc, 0x0, 0xc0, 0x4f, 0xcc, 0xa3, 0x34);

//	ѹ������ͷ
TSDB_DEFINE_GUID(CLSID_TSDBCOMPRESSHEADER,0x7a70d342, 0x2232, 0x11d3, 0xb8, 0xe9, 0x0, 0x50, 0x4, 0x86, 0x8e, 0xaa);

//	��������ͷ	{CEA772C7-7045-4f26-9973-7211C0377441}
TSDB_DEFINE_GUID(CLSID_TSDBLOCKHEADER,0xcea772c7, 0x7045, 0x4f26, 0x99, 0x73, 0x72, 0x11, 0xc0, 0x37, 0x74, 0x41);

//	ÿ�������ֽ�
#define		DATABYTES_PERLINE		42

#pragma pack(push,1)						//	��һ���ֽڶ������

///////////////////////////////////////////////////////////////////
//	ͨ���ṹ����
#ifndef __TONGSHI_TSDB_STRUCT__
#define __TONGSHI_TSDB_STRUCT__
	typedef union tagTSDBCHANNEL
	{
		struct
		{
			BYTE	m_nSubPage;						//	��ҳ��
			BYTE	m_nPageNo;						//	ҳ��
			BYTE	m_nMag;							//	��־��
			BYTE	m_nVBINo;						//	��Ч VBI ���
		};
		DWORD		m_dwData;						//	����
	}TSDBCHANNEL,*PTSDBCHANNEL;
#endif // __TONGSHI_TSDB_STRUCT__

////////////////////////////////////////////////////////////////////
//	���ļ����
typedef struct tagTSDBMULFILEHEAD
{
	GUID		m_CLSID;					//	GUID, CLSID_TSDBMULFILEHEAD
	WORD		m_cbSize;					//	�ļ�ͷ��С
	DWORD		m_dwHeaderCRC32;			//	�ļ�ͷ CRC32
	WORD		m_wVersion;					//	�汾��, Ϊ�Ժ������׼��
	BYTE		m_cFileNum;					//	�ļ��ĸ���
	WORD		m_wFileDataOfs[2];			//	���� = ?, �� m_cbSize ȷ��, ������ 2 �ļ�
}TSDBMULFILEHEAD,*PTSDBMULFILEHEAD;

#define MULFILEHEAD_MAXSIZE					(sizeof(TSDBMULFILEHEAD)+sizeof(WORD)*100)
#define MULFILEHEAD_MINFILELENGATE		6144	//	< 6K ���ļ���Ҫ�ö��ļ�
//////////////////////////////////////////////////////////////////////
#define TSDBFILEHEADER_VERSION			0x101
#define TSDB_NORMALFILELEN				10240

//	�����ļ���װ
typedef struct tagTSDBFILEHEADER
{
	GUID		m_CLSID;						//	GUID, CLSID_TSDBFILEHEADER
	WORD		m_cbSize;						//	�ļ�ͷ��С, ����
	DWORD		m_dwHeaderCRC32;				//	�ļ�ͷ�� CRC32
	WORD		m_wVersion;						//	�汾��, Ϊ�Ժ������׼��
	DWORD		m_dwFileCRC32;					//	�ļ� / ��� CRC32
	DWORD		m_dwFileLen;					//	�ļ�����
	WORD		m_cbFileNameLenCount;			//	�ļ����ַ�����
	union{
		struct
		{
			DWORD			m_bHugeFile:1;				//	�Ƿ���ļ�
			DWORD			m_bHasAttrib:1;				//	�Ƿ����ļ�����
			DWORD			m_bWinSock:1;				//	�Ƿ��� WinSocket ��ַ
			DWORD			m_bHasMuiticast:1;			//	�Ƿ��� Multicast ��ַ
			DWORD			m_bFlagRes:28;
		};
		DWORD	m_dwFlags;
	};											//	�ڴ˺��в������ȵ��ļ���, �������Ա�
}TSDBFILEHEADER,*PTSDBFILEHEADER;

//////////////////////////////////////////////////////////////////////////
//	���ļ�
//	���ļ����ռ�¼��չ��
#define HUGEFILEMSG_EXTNAME			".TS"
#define HUGEFILE_SIZEGATE			20480		//	20K

//	���ݽṹ
typedef struct tagTSDBHUGEFILEHEAD
{
	WORD		m_cbSize;						//	���ݰ���С
	WORD		m_wVersion;						//	�汾
	DWORD		m_dwFileLen;					//	�ļ�����
	DWORD		m_dwFileCRC32;					//	�ļ� CRC32
	WORD		m_wTotalBlock;					//	�ܿ���
	WORD		m_wBlockNo;						//	�����
	DWORD		m_dwFilePosition;				//	��ʼƫ��
	WORD		m_wBlockSize;					//	���С
	char		m_szTmpFileName[25];			//	��ʱ�ļ���
//	1.02 �������ϰ汾
	struct										//	1.02 ���ϰ汾�Ŵ���
	{
		DWORD	m_bNotSaveTmpFile:1;			//	��������ʱ�ļ�����Ӧ�ó�����
		DWORD	m_dwRes0:31;					//	�������� �� 0
	}m_Flags;
}TSDBHUGEFILEHEAD,*PTSDBHUGEFILEHEAD;

///////////////////////////////////////////////////////////////////////////
//  �ļ�����
// 2016.4.23 CYJ Modify, change time_t to int32_t, since linux-64, time_t is 64 bits
typedef struct tagTSDBFILEATTRIBHEAD
{
	WORD		m_cbSize;						//	���ݰ���С
	DWORD		m_dwAttribute;					//	�ļ�����
	int32_t		m_CreateTime;					//	�ļ�����ʱ��
	int32_t		m_LastAccessTime;				//	�ϴη���ʱ��
	int32_t		m_LastWriteTime;				//	�ϴθ���ʱ��
	DWORD		m_dwPurpose;					//	�ļ���;
	BYTE		m_ExtData[1];					//	��������
}TSDBFILEATTRIBHEAD,*PTSDBFILEATTRIBHEAD;

////////////////////////////////////////////////////////////////////////////
//	SOCKET ��ַ
typedef struct tagTSDBSOCKETHEAD
{
	WORD		m_cbSize;						//	Socket ��ַ
}TSDBSOCKETHEAD,*PTSDBSOCKETHEAD;

///////////////////////////////////////////////////////////////////////////
//	���ַ����
typedef struct tagTSDBMULTICAST
{
	WORD		m_cbSize;						//	����ͷ��С
	WORD		m_cbUnitSize;					//	���ݵ�Ԫ��С
	WORD		m_nPacketNum;					//	���ݰ���
	BYTE		m_Data[1];						//	����
}TSDBMULTICAST,*PTSDBMULTICAST;

///////////////////////////////////////////////////////////////////////////
//	����ѹ��
typedef struct tagTSDBCOMPRESSHEAD
{
	GUID		m_CLSID;						//	GUID, CLSID_TSDBFILEHEADER
	WORD		m_cbSize;						//	�ļ�ͷ��С, ����
	DWORD		m_dwHeaderCRC32;				//	�ļ�ͷ�� CRC32
	DWORD		m_dwMethod;						//	ѹ������
	WORD		m_wVersion;						//	ѹ���汾
	DWORD		m_dwFileLen;					//	ѹ�����ļ�����
	DWORD		m_dwFileCRC32;					//	ѹ�����ļ�CRC32
	DWORD		m_dwOrgFileLen;					//	ѹ��ǰ���ļ�����
	DWORD		m_dwOrgFileCRC32;				//	ѹ��ǰ���ļ� CRC32
}TSDBCOMPRESSHEAD,*PTSDBCOMPRESSHEAD;			//	���и�������

//	ѹ���������ȶ���
typedef enum
{
	TSDBCOMPRESS_METHOD_NOCMP =	0,				//	��ѹ��
	TSDBCOMPRESS_METHOD_AUTO,					//	�Զ�ѹ��
	TSDBCOMPRESS_METHOD_ARG241 = 0x20,			//	ARJ	ϵ��
	TSDBCOMPRESS_METHOD_PKZIP204 = 0x30,		//	PKZIP ϵ��
	TSDBCOMPRESS_METHOD_LZSSV100 = 0x40,		//	LZSS
	TSDBCOMPRESS_METHOD_LZHUFV100 = 0x50,		//	LZHUF
} TSDBCOMPRESSMETHODENUM;

///////////////////////////////////////////////////////////////////////////
//	���ݼ���
typedef struct tagTSDBLOCKHEAD
{
	GUID	m_CLSID;							//	GUID
	WORD	m_wHeadSize;						//	����ͷ��С
	DWORD	m_dwHeadCrc32;						//	����ͷ CRC32
	DWORD	m_dwSysCodeIndex;					//	ϵͳ��������
	DWORD	m_dwDataLen;						//	���ݳ���
	DWORD	m_dwOffset;							//	���ݿ����ļ��е�ƫ��
	DWORD	m_dwOrgCRC32;						//	ԭʼ CRC32
}TSDBLOCKHEAD, *PTSDBLOCKHEAD;

#pragma pack(pop)						//	��һ���ֽڶ������

#endif // __TSDB_INCLUDE_19990615__
