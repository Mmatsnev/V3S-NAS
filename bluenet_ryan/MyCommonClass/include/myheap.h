///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2005-1-10
///
///		��;��
///			���ѡ��ڴ����
///		
///			���ѹ�������ֻ�ܰ�ȡ����һ�εķ��䣬�������ͷ��κη�����ڴ档
///			���������Ŀ����Ϊ�˸����ڴ��еĶ�̬���ݣ����ݽṹ��
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӹ�˾���ڲ�ʹ��!				 !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	����Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!		 !
///=======================================================

#if !defined(AFX_MYHEAP_H__B623E84A_F7BC_4156_A597_93B5739F482F__INCLUDED_)
#define AFX_MYHEAP_H__B623E84A_F7BC_4156_A597_93B5739F482F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// 2016.12.17 CYJ Add
#pragma pack(push,8)

class CMyHeap  
{
public:
	bool Write( void * pBuf, int nCount );
	int GetMemoryAllocated() const;
	bool IsValid();
	void CancelLastAllocate();
	PBYTE Allocate( int nSize );
	void Reset();
	PBYTE Detach( int * pnHeapSize );
	bool Attach( PBYTE pBuf, int nSize );
	void DestroyHeap();
	bool CreateHeap( int nHeapSize = 4096 );
	CMyHeap(int nHeapSize=4096);
	CMyHeap(PBYTE pBuf, int nHeapSize);
	virtual ~CMyHeap();
	const PBYTE GetHeapBuf() const { return m_pHeapBuf; }
	const int GetHeapSize()const { return m_nHeapSize; }
	const int GetHeapMaxSize()const{ return m_nHeapMaxSize; }		//  CYJ,2009-6-23 ����

private:
	bool	m_bIsAttached;
	PBYTE	m_pHeapBuf;
	int		m_nHeapSize;		//	��ǰ�Ѵ�С
	int		m_nHeapMaxSize;		//	���Է��������ֽ���
	int		m_nLastAllocatedSize;
	int		m_nTotalBytesAllocaed;
};

// 2016.12.17 CYJ Add
#pragma pack(pop)

#endif // !defined(AFX_MYHEAP_H__B623E84A_F7BC_4156_A597_93B5739F482F__INCLUDED_)
