//	FilePurpose.H
//			�ļ���;�������

#ifndef __FILE_PURPOSE_INCLUDE_H__
#define __FILE_PURPOSE_INCLUDE_H__

typedef enum
{
	TSDB_FP_NORMALDATA = 0,						//	��ͨ�ļ�,�޶��������;
	TSDB_FP_UPDATE_SYSFILE,						//	ϵͳ�������
	TSDB_FP_UPDATE_UPDATENOW,					//	�����������ļ�,һ��Ϊ�����ļ�
	TSDB_FP_UPDATE_APP,							//	OEM Ӧ�ó�������

	TSDB_FP_LICENCE_MSG = 0x1000,				//	��Ϣ�����������Ȩ�ļ�

	TSDB_FP_RESFORAPP = 0x7FFF0000,				//	0x7FFF0000 ~ 0x7FFFFFFF ΪӦ�ó����Զ�����;����Ҫ�������ض��ļ�����

	TSDB_FP_RESERVED = -2,						//	����
	TSDB_FP_UNKNOWN = -1						//	δ֪�ļ���;, ����
}TSDBFILEPURPOSE;


#endif //__FILE_PURPOSE_INCLUDE_H__
