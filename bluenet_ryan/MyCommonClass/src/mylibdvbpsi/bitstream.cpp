#include "stdafx.h"
#include "bitstream.h"

// this class will increment Err if you proceed past the end of the buffer
CMyBitStream::CMyBitStream (unsigned char *pBuffer, unsigned long Length, int *Err)
{
	m_rdbfr = pBuffer;
	m_rdbfr_length = Length;
	m_nbits = 0;

	//  CYJ,2005-1-14 add
	m_pWritePtr = pBuffer;
	m_nWriteBitsInCache = 0;
	m_dwWriteCache = 0;
	m_nTotalWriteBits = 0;

	initbits (Err);
}

CMyBitStream::~CMyBitStream ()
{
}
			   
///-------------------------------------------------------
/// CYJ,2005-1-14
/// ��������:
///		initialize bits
/// �������:
///		Err						output error count
///		bRssetWriteParam		reset write ptr
/// ���ز���:
///		��
void CMyBitStream::initbits (int *Err, bool bRssetWriteParam)
{
	m_incnt = 0;
	m_rdptr = m_rdbfr + m_rdbfr_length;
	m_rdmax = m_rdptr;
	m_bitcnt = 0;
	m_bfr = 0;
	m_rdptr = m_rdbfr;
	flushbits (0, Err);

	m_pWritePtr = m_rdbfr;
	if( bRssetWriteParam )
	{
		m_nWriteBitsInCache = 0;
		m_dwWriteCache = 0;
		m_nTotalWriteBits = 0;
	}
}

// return next n bits (right adjusted) without advancing
unsigned int CMyBitStream::showbits (int n)
{
	return m_bfr >> (32-n);
}

// advance by n bits
void CMyBitStream::flushbits (int n, int *Err)
{
	int incnt;

	m_nbits += n;

	if (n == 32)
		m_bfr = 0;
	else
		m_bfr <<= n;	
	incnt = m_incnt -= n;
	if (incnt <= 24)
	{
		if (m_rdptr < m_rdbfr+(m_rdbfr_length-4))
		{
			do
			{
				m_bfr |= *m_rdptr++ << (24 - incnt);
				incnt += 8;
			}
			while (incnt <= 24);
		}
		else
		{
			do
			{
				if (m_rdptr >= m_rdbfr+m_rdbfr_length)
				{
					if( Err )				//  2005-1-14 add, �򻯲���
						(*Err)++;
					m_bfr |= 0 << (24 - incnt);
				}
				else
					m_bfr |= *m_rdptr++ << (24 - incnt);
				incnt += 8;
			}
			while (incnt <= 24);
		}
		m_incnt = incnt;
	}
}

void CMyBitStream::flushbits32 (int *Err)
{
	int incnt;

	m_nbits += 32;

	m_bfr = 0;
	incnt = m_incnt;
	incnt -= 32;
    while (incnt <= 24)
    {
		if (m_rdptr >= m_rdbfr+m_rdbfr_length)
		{
			if( Err )				//  CYJ,2005-1-14 add,�򻯲���
				(*Err)++;
			m_bfr |= 0 << (24 - incnt);
		}
		else
			m_bfr |= *m_rdptr++ << (24 - incnt);
		incnt += 8;
    }
	m_incnt = incnt;
}


// return next n bits (right adjusted)
unsigned int CMyBitStream::getbits (int n, int *Err)
{
	unsigned int l;

	l = showbits (n);
	flushbits (n, Err);

	return l;
}

unsigned int CMyBitStream::getbits32 (int *Err)
{
	unsigned int l;

	l = showbits (32);
	flushbits32 (Err);

	return l;
}

void CMyBitStream::align (int *Err)
{
	int incnt;

	if (m_incnt != 32)
	{
		incnt = m_incnt % 8;
		flushbits (incnt, Err);
	}
}

long CMyBitStream::getnbits ()
{
	return m_nbits;
}

//  CYJ,2005-1-14 add
unsigned int g_bitstream_bit_msk[33] =
{
  0x00000000, 0x00000001, 0x00000003, 0x00000007,
  0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
  0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
  0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
  0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
  0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
  0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
  0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
  0xffffffff,
};

///-------------------------------------------------------
/// CYJ,2005-1-14
/// ��������:
///		���N������
/// �������:
///		dwData		�����������
///		nBits		����ı��ظ�����1��32֮��
/// ���ز���:
///		��
void CMyBitStream::PutBits( register unsigned long dwData, int nBits )
{	
	m_nTotalWriteBits += nBits;

	while( nBits > 0 )
	{						//	�������
		if( !SaveCacheByteToBuf() )
			return;					//	��������

		int nBitNeed = 32 - m_nWriteBitsInCache;
		if( nBitNeed > nBits )
			nBitNeed = nBits;

		dwData &= g_bitstream_bit_msk[nBits];
		register DWORD dwTmp = (dwData>>(nBits-nBitNeed) );
		dwTmp <<= (32-(m_nWriteBitsInCache+nBitNeed));
		m_dwWriteCache |= dwTmp;
		m_nWriteBitsInCache += nBitNeed;
		nBits -= nBitNeed;		
	}	
}

///-------------------------------------------------------
/// CYJ,2005-1-14
/// ��������:
///		���� Cache �е��ֽڵ���������
/// �������:
///		��
/// ���ز���:
///		��
bool CMyBitStream::SaveCacheByteToBuf()
{
	if( m_pWritePtr >= m_rdmax )
		return false;							//	������Χ
	while( m_nWriteBitsInCache >= 8 )
	{
		*m_pWritePtr ++ = BYTE(m_dwWriteCache >> 24);
		m_dwWriteCache <<= 8;
		m_nWriteBitsInCache -= 8;			
	}
	return true;
}

///-------------------------------------------------------
/// CYJ,2005-1-14
/// ��������:
///		���һ������
/// �������:
///		byData			������ı���
/// ���ز���:
///		��
void CMyBitStream::PutBit( unsigned char byData )
{
	PutBits( byData, 1 );
}

///-------------------------------------------------------
/// CYJ,2005-1-14
/// ��������:
///		���32����
/// �������:
///		��
/// ���ز���:
///		��
void CMyBitStream::PutBits32( unsigned long dwData )
{
	PutBits( dwData, 32 );
}

///-------------------------------------------------------
/// CYJ,2005-1-14
/// ��������:
///		���16��������
/// �������:
///		wData			����
/// ���ز���:
///		��
void CMyBitStream::PutBits16( unsigned short wData )
{
	PutBits( wData,16 );
}

///-------------------------------------------------------
/// CYJ,2005-1-14
/// ��������:
///		���8��������
/// �������:
///		��
/// ���ز���:
///		��
void CMyBitStream::PutBits8( unsigned char byData )
{
	PutBits( byData, 8 );
}

///-------------------------------------------------------
/// CYJ,2005-1-14
/// ��������:
///		���������ˢ�»���
/// �������:
///		��
/// ���ز���:
///		��
void CMyBitStream::FinishWrite()
{
	SaveCacheByteToBuf();
	if( m_nWriteBitsInCache )
	{
		PutBits8( 0 );
		SaveCacheByteToBuf();
	}
	m_dwWriteCache = 0;
	m_nWriteBitsInCache = 0;
}
