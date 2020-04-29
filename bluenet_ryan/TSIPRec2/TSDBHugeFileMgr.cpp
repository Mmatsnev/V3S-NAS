// TSDBHugeFileMgr.cpp: implementation of the CTSDBHugeFileMgr class.
//
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
//	2002.3.29	Ϊ�˹�����ļ������޸� UpdateObj & ClearBuf ������delete pObj ==> pObj->Release

#include "stdafx.h"
#include "resource.h"
#include "TSDBHugeFileMgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTSDBHugeFileMgr::CTSDBHugeFileMgr()
{
	m_pLastFind = NULL;
}

CTSDBHugeFileMgr::~CTSDBHugeFileMgr()
{
	ClearBuf();
}

//	��������
//	��ڲ���
//		pHeader							���ļ�ͷ
//		pDataBuf						���ݻ�����
//	���ز���
//		TRUE							���ļ����ճɹ�
//		FALSE							������
BOOL CTSDBHugeFileMgr::SaveBlock(PTSDBHUGEFILEHEAD pHeader, PBYTE pDataBuf)
{
	ASSERT( pHeader && pDataBuf );
	CTSDBHugeFileObj * pObj = FindObj( pHeader );
	if( pObj == NULL )
	{									//	�ô��ļ�Ϊ�µĴ��ļ�, ��Ҫ����
		pObj = AddHelpObj( pHeader );
		if( !pObj )
		{
			UpdateObj();				//	ɾ������Ķ���
			pObj = AddHelpObj( pHeader );
			if( !pObj )
				return FALSE;			//	��������̫��Ķ���ʹ����
		}
	}
	return pObj->SaveBlock( pHeader, pDataBuf );
}

//	���ɲ���Ӷ���
//	��ڲ���
//		pHeader							���ļ������Զ���
//	���ز���
//		����ָ��
CTSDBHugeFileObj * CTSDBHugeFileMgr::AddHelpObj(PTSDBHUGEFILEHEAD pHeader)
{
	ASSERT( pHeader );
	if( m_HelpObj.GetSize() >= MAX_HUGEOBJ_NUM )
		return NULL;
	CTSDBHugeFileObj * pObj = new CTSDBHugeFileObj();
	if( !pObj )
		return NULL;					//	�����ڴ�ʧ��
	if( pObj->Attach( pHeader ) == FALSE )
	{
		delete pObj;
		return NULL;
	}
	pObj->AddRef();						//	2002.3.29 ȷ������ʹ�ã��������ü�����
	m_HelpObj.Add( pObj );
	return pObj;
}

//	��������ͷ
//	��ڲ���
//		pHeader							����ͷ
//	���ز���
//		����ָ��
CTSDBHugeFileObj * CTSDBHugeFileMgr::FindObj(PTSDBHUGEFILEHEAD pHeader)
{
	int nNum = m_HelpObj.GetSize();
	if( m_pLastFind && m_pLastFind->IsSameObj( pHeader ) )
		return m_pLastFind;
	CTSDBHugeFileObj * pObj = NULL;
	for(int i=0; i<nNum; i++)
	{
		pObj = (CTSDBHugeFileObj *)m_HelpObj[i];
		ASSERT( pObj );
		if( pObj->IsSameObj( pHeader ) )
			return pObj;
	}
	return NULL;
}

//	�������
//	ɾ��ʱ����С, ������Ķ���
void CTSDBHugeFileMgr::UpdateObj()
{
	m_pLastFind = NULL;
	int nNum = m_HelpObj.GetSize();
	CTSDBHugeFileObj * pObj;
	int nNoToDelete = -1;
	time_t	MinTime = CTime::GetCurrentTime().GetTime();
	for(int i=0; i<nNum; i++)
	{
		pObj = (CTSDBHugeFileObj *)m_HelpObj[i];
		ASSERT( pObj );
		if( MinTime >= pObj->m_LastAccessTime )
		{
			MinTime = pObj->m_LastAccessTime ;
			nNoToDelete = i;
		}
	}
	ASSERT( nNoToDelete != -1 );
	pObj = (CTSDBHugeFileObj *)m_HelpObj[nNoToDelete];
	pObj->Release();				//	2002.3.29, delete pObj ==> pObj->Release��Ϊ�˹������
	m_HelpObj.RemoveAt( nNoToDelete );
}

//	�����������
void CTSDBHugeFileMgr::ClearBuf()
{
	int nNum = m_HelpObj.GetSize();
	for( int i=0; i<nNum; i ++ )
	{
		CTSDBHugeFileObj * pObj = (CTSDBHugeFileObj *)m_HelpObj[0];
		ASSERT( pObj );
		pObj->Release();			//	2002.3.29, delete pObj ==> pObj->Release��Ϊ�˹������
		m_HelpObj.RemoveAt(0);
	}
	m_pLastFind = NULL;
}
