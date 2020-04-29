///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2005-1-11
///
///		��;��
///			����dvb�ϵ� crc32 & crc16
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӡ��ڲ�ʹ��!	  			     !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	��Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!			 !
///=======================================================

#ifndef __DVB_CRC_H_20050111__
#define __DVB_CRC_H_20050111__

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

DWORD DVB_GetCRC32( PBYTE pBuf, int nLen, DWORD dwLastCRC = 0xFFFFFFFF);
unsigned short DVB_GetCRC16(unsigned char * pBuf, int nLen, unsigned short wLastCRC=0xFFFF);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __DVB_CRC_H_20050111__
