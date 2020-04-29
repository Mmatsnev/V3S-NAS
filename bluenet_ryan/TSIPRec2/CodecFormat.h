#ifndef __CODECFORMAT_INCLUDE_H_19991105__
#define __CODECFORMAT_INCLUDE_H_19991105__

#pragma pack(push,1)

	#define  TSLOCKCODECLEN 6600 
	typedef enum {
		LICTYPE_CONTINUE = 0,						//	n �������Ŀ���
		LICTYPE_LESS100,							//	n �����Ų�ֵС�� 100 �Ŀ������
		LICTYPE_LESS10000,							//	n �����Ų�ֵС�� 10000 �Ŀ������
		LICTYPE_INDEPENDENCE,						//	n ���໥�����Ŀ���
	}LICTYPE_ENUM;
	typedef enum {									//	��Ȩ����
		LICOP_LIC = 0,
		LICOP_DEL,
	}LICOP_METHOD;
	typedef struct tagBLKHEADER						//	���ݿ�����ͷ
	{
		DWORD	m_dwCRC32;							//	���ݵ� CRC32
		WORD	m_wVersion;							//	�汾��
		DWORD	m_dwMinIDCode;						//	��С����
		DWORD	m_dwMaxIDCode;						//	��󿨺�
		WORD	m_wBlkNum;							//	�ӿ���
		DWORD	m_dwSysCodeIndex;					//	ϵͳ��������
		BYTE	m_byLicOpCode;						//	��Ȩ����ָ��
	}BLKHEADER,*PBLKHEADER;

	typedef struct tagSUBBLKHEAD
	{
		BYTE	m_bySubType;						//	������
		BYTE	m_byUserNum;						//	�������͵��û���
		DWORD	m_dwIDCode;							//	�û�����
		DWORD	m_dwLicCode;						//	��Ȩ��
	}SUBBLKHEAD,*PSUBBLKHEAD;

	typedef struct tagSUBBLK_CONTINUE				//	n ���������ŵ����
	{
		DWORD	m_dwLicCode;						//	��Ȩ��
	}SUBBLK_CONTINUE,*PSUBBLK_CONTINUE;

	typedef struct tagSUBBLK_LESS100
	{												//	n �����Ų�ֵ������100�Ŀ���
		BYTE	m_byIDCode;
		DWORD	m_dwLicCode;
	}SUBBLK_LESS100,*PSUBBLK_LESS100;

	typedef struct tagSUBBLK_LESS10000
	{												//	n �����Ų�ֵ������ 10000
		WORD	m_wIDCode;							//	����
		DWORD	m_dwLicCode;						//	��Ȩ��
	}SUBBLK_LESS10000,*PSUBBLK_LESS10000;

	typedef struct tagSUBBLK_INDEPENDENCE
	{												//	n �������Ŀ���
		DWORD	m_dwIDCode;							//	����
		DWORD	m_dwLicCode;						//	��Ȩ��
	}SUBBLK_INDEPENDENCE,*PSUBBLK_INDEPENDENCE;

#pragma pack(pop)

#endif // __CODECFORMAT_INCLUDE_H_19991105__