///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2007-3-1
///
///		��;��
///			���տ�����������
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӡ��ڲ�ʹ��!	  			     !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	��Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!			 !
///=======================================================
//  CYJ,2008-9-24 RegisterAsMaster, ��һ��ע��ʱ�����³�ʼ�� ��Ƶͷ �� DiSEqC �豸
//  CYJ,2007-8-10	�޸� OnAfterTune����̨���ٴβ���ԭ���Ѿ��򿪵�PID�����������Զ��ָ��Ե�̨��

// OneNICAdapter.cpp: implementation of the COneNICAdapter class.
//
//////////////////////////////////////////////////////////////////////
#include <vdw.h>
#include <windef.h>

#include "TSDVBIFDRV.h"
#include "OneNICAdapter.h"
#include "TSDVBIFDRVDevice.h"
#include "DVBDSMCC_IP_MPE.h"

#ifdef _DEBUG
	extern	KDebugOnlyTrace	t;			// Global driver trace object	
#endif //_DEBUG

#pragma hdrstop("TSDVBIFDRV.pch")

//////////////////////////////////////////////////////////////////////////
extern "C" void Sleep( DWORD dwMS );


//////////////////////////////////////////////////////////////////////////
// CPIDBitmaskMgr
//////////////////////////////////////////////////////////////////////////
CPIDBitmaskMgr::CPIDBitmaskMgr()
{
	memset( m_abyPIDBitMask, 0, sizeof(m_abyPIDBitMask) );
	m_bIsAllPIDOpenned = false;
}

CPIDBitmaskMgr::~CPIDBitmaskMgr()
{
}

///-------------------------------------------------------
/// CYJ,2007-3-6
/// ��������:
///		�� PID 
/// �������:
///		wPID			PID
/// ���ز���:
///		��
void CPIDBitmaskMgr::OpenOnePID( WORD wPID )
{
	wPID &= 0x1FFF;
	WORD wOffset = wPID >> 3;
	BYTE byMask = 1 << (wPID&7);

	ASSERT( 0 == (( m_abyPIDBitMask[wOffset] & byMask )) );
	m_abyPIDBitMask[wOffset] |= byMask;
}

///-------------------------------------------------------
/// CYJ,2007-3-6
/// ��������:
///		�ر�һ��PID
/// �������:
///		wPID			PID
/// ���ز���:
///		��
void CPIDBitmaskMgr::CloseOnePID( WORD wPID )
{
	wPID &= 0x1FFF;
	WORD wOffset = wPID >> 3;
	BYTE byMask = 1 << (wPID&7);

	ASSERT( m_abyPIDBitMask[wOffset] & byMask );
	m_abyPIDBitMask[wOffset] &= (~byMask);

	m_bIsAllPIDOpenned = false;
}

///-------------------------------------------------------
/// CYJ,2007-3-6
/// ��������:
///
/// �������:
///		��
/// ���ز���:
///		ԭ����״̬		��0�� �Ѿ���
///						0���ر�
BYTE CPIDBitmaskMgr::IsPIDOpenned( WORD wPID )
{
	if( m_bIsAllPIDOpenned )
		return 1;

	wPID &= 0x1FFF;
	WORD wOffset = wPID >> 3;
	BYTE byMask = 1 << (wPID&7);

	return ( m_abyPIDBitMask[wOffset] & byMask );
}

///-------------------------------------------------------
/// CYJ,2007-3-6
/// ��������:
///		������PID
/// �������:
///		��
/// ���ز���:
///		��
/// ˵����
///		ֻ����Ȩ�Ҷ�ռ�û����ܵ��øú���
void CPIDBitmaskMgr::OpenAllPID()
{
	memset( m_abyPIDBitMask, 0xFF, sizeof(m_abyPIDBitMask) );
	m_bIsAllPIDOpenned = true;
}

///-------------------------------------------------------
/// CYJ,2007-3-6
/// ��������:
///		�ر�����PID
/// �������:
///		��
/// ���ز���:
///		��
/// ˵����
///		ֻ����Ȩ�Ҷ�ռ�û����ܵ��øú���
void CPIDBitmaskMgr::CloseAllPID()
{
	memset( m_abyPIDBitMask, 0, sizeof(m_abyPIDBitMask) );
	m_bIsAllPIDOpenned = false;
}

//////////////////////////////////////////////////////////////////////////
// COneDataPort
//////////////////////////////////////////////////////////////////////////
COneDataPort::COneDataPort( PFILE_OBJECT pOwnerFileObj, COneNICAdapter * pAdapter, DWORD dwDataPortNo, TSDVB_PID_TYPE nType )
{
	SetPnpDeviceMode( true );		//  CYJ,2008-9-6 use as PNP Mode

	m_pAdapter = pAdapter;
	m_dwDataPortNo = dwDataPortNo;
	m_nDataType = nType;	
	m_dwDataLostCount = 0;
	m_pOwenerFileObject = pOwnerFileObj;

	ASSERT( sizeof(DVB_TS_PACKET) == DVB_TS_PACKET_SIZE );
	m_bAllowDummyTSPacket = FALSE;

	m_nCachePacketCount = 0;
	m_dwTSPacketReceived = 0;
}

COneDataPort::~COneDataPort()
{	
	ASSERT( m_pAdapter );
	if( m_pAdapter )
		m_pAdapter->FreeAllPIDsOfDataPort( this );
}

///-------------------------------------------------------
/// CYJ,2007-4-12
/// ��������:
///		�Ƿ������ȡ�հ�
/// �������:
///		bEnalbe			TRUE		����
///						FALSE		������
/// ���ز���:
///		ԭ��������
BOOL COneDataPort::AllowDummyTSPacket( BOOL bEnable )
{
	BOOL bOldValue = m_bAllowDummyTSPacket;
	m_bAllowDummyTSPacket = bEnable;
	return bOldValue;
}

///-------------------------------------------------------
/// CYJ,2007-3-7
/// ��������:
///		����һ��TS����
/// �������:
///		pTSPacket			TS ����
/// ���ز���:
///		��
void COneDataPort::PushOneTSPacket( PDVB_TS_PACKET pTSPacket )
{
	WORD wPID = pTSPacket->GetPID();
	
	if( FALSE == m_bAllowDummyTSPacket && wPID == INVALID_PID )
		return;

	if( 0 == IsPIDOpenned( wPID ) )
		return;	

	ForceInjectOneTSPacket( pTSPacket );
}

///-------------------------------------------------------
/// CYJ,2009-3-14
/// ��������:
///		ǿ������ TS ����
/// �������:
///		��
/// ���ز���:
///		��
void COneDataPort::ForceInjectOneTSPacket( PDVB_TS_PACKET pTSPacket )
{
	m_dwTSPacketReceived ++;

	memcpy( m_aTSPacketCache + m_nCachePacketCount, pTSPacket, sizeof(DVB_TS_PACKET) );
	m_nCachePacketCount ++;
	if( m_nCachePacketCount >= DATAPORT_CACHE_TS_COUNT )
	{
		if( FALSE == AddData( (PBYTE)m_aTSPacketCache, sizeof(m_aTSPacketCache) ) )
			m_dwDataLostCount ++;			// �����������
		m_nCachePacketCount = 0;
	}
}

///-------------------------------------------------------
/// CYJ,2007-4-17
/// ��������:
///		ˢ�»������е�����
/// �������:
///		��
/// ���ز���:
///		��
void COneDataPort::FlushCacheData()
{
	if( 0 == m_nCachePacketCount )
		return;
	if( FALSE == AddData( (PBYTE)m_aTSPacketCache, m_nCachePacketCount*sizeof(DVB_TS_PACKET) ) )
		m_dwDataLostCount ++;			// �����������
	m_nCachePacketCount = 0;
}

///-------------------------------------------------------
/// CYJ,2007-5-5
/// ��������:
///		�������
/// �������:
///		��
/// ���ز���:
///		��
void COneDataPort::AbortData()
{
	m_nCachePacketCount = 0;
	AbortAllData();
}

///-------------------------------------------------------
/// CYJ,2007-5-23
/// ��������:
///		��ѯ����״̬�������ԭ����״̬
/// �������:	
///		��
/// ���ز���:
///		��
void COneDataPort::QueryDataPortStatus( DWORD * pdwPacketReceived, DWORD * pdwPacketLost )
{
	if( pdwPacketReceived )
		*pdwPacketReceived = m_dwTSPacketReceived;
	m_dwTSPacketReceived = 0;

	if( pdwPacketLost )
		*pdwPacketLost = m_dwDataLostCount;
	m_dwDataLostCount = 0;
}

///-------------------------------------------------------
/// CYJ,2007-3-7
/// ��������:
///		������Կ������������
/// �������:
///		dwOutMemSize		�����������С
/// ���ز���:
///		���Ը��Ƶ��ֽ���
///	˵����
///		���� IP ��ʽ��ֻ����һ��IP��
///		����ԭʼ TS ��ʽ�����Ը��ƶ��TS���顣
///		�ڸú����ڣ����ܵ���Lock��������Ϊ�Ѿ����ù� Lock �����ˡ�
DWORD COneDataPort::GetMaxCopyDataSize( DWORD dwOutMemSize )
{	
	PDPMGRLIST pFirstPacket = m_pTailPtr;

	// ���������ҲҪ�����һ�����ݰ��Ĵ�С!
	DWORD dwTotalSize = CWIN32DPDataMgr::GetDataPacketSize( pFirstPacket, &pFirstPacket );
	ASSERT( dwTotalSize );
	if( 0 == dwTotalSize )
		return 0;
	pFirstPacket = pFirstPacket->m_pNext;

	while( m_pHeadPtr != pFirstPacket && pFirstPacket->m_dwDataLen )
	{
		DWORD dwCurPacketSize = CWIN32DPDataMgr::GetDataPacketSize( pFirstPacket, &pFirstPacket );
		if( 0 == dwCurPacketSize || (dwCurPacketSize + dwTotalSize > dwOutMemSize ) )
			break;
		dwTotalSize += dwCurPacketSize;
		ASSERT( FALSE == pFirstPacket->m_bLinkToNext );
		pFirstPacket = pFirstPacket->m_pNext;
	}
	
	return dwTotalSize;
}

///-------------------------------------------------------
/// CYJ,2007-6-21
/// ��������:
///		ͬ����ȡ���ݣ������������ݣ���������
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneDataPort::ReadDataSync( KIrp I)
{
//	TRACE("StartIO.\n");
	DWORD dwByteRead = 0;

	KMemory Mem(I.Mdl());
	// Use the memory object to create a pointer to the caller's buffer
	PUCHAR	pOutBuf	= (PBYTE) Mem.MapToSystemSpace();

	NTSTATUS Status = ReadDataSync( pOutBuf, I.ReadSize(CURRENT), &dwByteRead );

	if( Status == STATUS_SUCCESS )
		I.Information() = dwByteRead;

	return Status;
}

///-------------------------------------------------------
/// CYJ,2008-3-20
/// ��������:
///		��ȡ���ݶ˿�
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneDataPort::ReadDataSync( PBYTE pOutBuf, DWORD dwBufSize, DWORD * pdwByteRead )
{
	ASSERT( pOutBuf && dwBufSize && pdwByteRead );
	if( NULL == pOutBuf || 0 == dwBufSize || NULL == pdwByteRead )
		return STATUS_INVALID_PARAMETER;

	*pdwByteRead = 0;

	Lock();

	if( m_pHeadPtr == m_pTailPtr )
	{
		Unlock();
		return STATUS_SUCCESS;		// no data
	}	

	DWORD dwTotalSize = GetMaxCopyDataSize( dwBufSize );

	if( NULL == pOutBuf	 || dwBufSize < dwTotalSize )
	{
		Unlock();
		return STATUS_BUFFER_TOO_SMALL;
	}

	*pdwByteRead = dwTotalSize;

	while ( dwTotalSize > 0 )
	{
		RtlCopyMemory( pOutBuf, m_pTailPtr->m_pDataBuf, m_pTailPtr->m_dwDataLen );
		pOutBuf += m_pTailPtr->m_dwDataLen;
		dwTotalSize -= m_pTailPtr->m_dwDataLen;		
#ifdef _DEBUG
		ASSERT( dwTotalSize >= 0 );
#endif // _DEBUG
		m_pTailPtr->m_dwDataLen = 0;
		m_pTailPtr->m_bLinkToNext = FALSE;

		m_pTailPtr = m_pTailPtr->m_pNext;
	}

	Unlock();

	return STATUS_SUCCESS;		// no data
}

//////////////////////////////////////////////////////////////////////////
// CUDPDataPort
//////////////////////////////////////////////////////////////////////////
CUDPDataPort::CUDPDataPort( PFILE_OBJECT pOwnerFileObj, COneNICAdapter * pAdapter, DWORD dwDataPortNo, TSDVB_PID_TYPE nType, DWORD dwIP, WORD wPort )
 : COneDataPort( pOwnerFileObj, pAdapter, dwDataPortNo, nType )
{
	m_dwDstIP = dwIP;		// 0 ��ʾͨ�䣬��֮��ֻ���ն�Ӧ��IP;  �����ֽ���
	m_wPort = wPort;		// 0 ��ʾͨ�䣬��֮��ֻ���ն�Ӧ�Ķ˿�;  �����ֽ���
	memset( m_aIPMPE, 0, sizeof(m_aIPMPE) );
}

CUDPDataPort::~CUDPDataPort()
{
	for(int i=0; i<IP_MPE_HASH_LEN; i++ )
	{
		if( NULL == m_aIPMPE[i] )
			continue;			//  CYJ,2008-9-6 modify
		CDVBDSMCC_IP_MPE * pItem = m_aIPMPE[i];
		while( pItem )
		{
			CDVBDSMCC_IP_MPE * pTmp = pItem;
			pItem = pItem->GetNextItem();
			delete pTmp;
		}
	}
}

///-------------------------------------------------------
/// CYJ,2007-3-7
/// ��������:
///		������Կ������������
/// �������:
///		dwOutMemSize		�����������С
/// ���ز���:
///		���Ը��Ƶ��ֽ���
///	˵����
///		���� IP ��ʽ��ֻ����һ��IP��
///		����ԭʼ TS ��ʽ�����Ը��ƶ��TS���顣
///		�ڸú����ڣ����ܵ���Lock��������Ϊ�Ѿ����ù� Lock �����ˡ�
DWORD CUDPDataPort::GetMaxCopyDataSize( DWORD dwOutMemSize )
{	// UDP ��ʽ��ֻ����һ�����ݰ�
	return CWIN32DPDataMgr::GetMaxCopyDataSize( dwOutMemSize );
}

///-------------------------------------------------------
/// CYJ,2007-3-7
/// ��������:
///		��������
/// �������:
///		��
/// ���ز���:
///		��
void CUDPDataPort::PushOneTSPacket( PDVB_TS_PACKET pTSPacket )
{
	WORD wPID = pTSPacket->GetPID();	
	
	if( wPID == INVALID_PID )
		return;

	if( false == IsPIDOpenned( wPID ) )
		return;	

	m_dwTSPacketReceived ++;

	CDVBDSMCC_IP_MPE * pMPEItem = GetIPMPEObj( wPID );
#ifdef _DEBUG
	ASSERT( pMPEItem );
#endif //_DEBUG
	if( NULL == pMPEItem )
		return;

	pMPEItem->PushOneTSPacket( pTSPacket );
}

///-------------------------------------------------------
/// CYJ,2007-5-10
/// ��������:
///		���յ� IP ��
/// �������:
///		��
/// ���ز���:
///		��
void CUDPDataPort::OnEthernetFrame( PBYTE pEthernetAddr, int nFrameLen )
{
#ifdef _DEBUG
	ASSERT( m_pAdapter && m_pAdapter->m_pAssociateNicDrv );
	ASSERT( nFrameLen > sizeof(ETHERNET_FRAME) && pEthernetAddr );
#endif //_DEBUG

	if( NULL == pEthernetAddr || nFrameLen <= sizeof(ETHERNET_FRAME) )
		return;

	if( m_nDataType == TSDVB_PID_TYPE_UDP_NDIS )
	{	// �����͵����磬����øú��������򱣴浽 WIN32  DataPort ��
		m_pAdapter->OnEthernetFrame( pEthernetAddr, nFrameLen );
	}
	else
	{
		IP_HEADER * pIPHeader = (IP_HEADER *)( pEthernetAddr+sizeof(ETHERNET_FRAME) );
		if( pIPHeader->protocol != 0x11 )
			return;			// Not UDP
		if( m_dwDstIP && m_dwDstIP != pIPHeader->dest )
			return;			// m_dwDstIP ��0 ��ʾ��Ҫ����ƥ�䣻�����ֽ���
		int nIPLen = (pIPHeader->x & 0xF) * 4;
		UDP_HEADER * pUDPHeader = (UDP_HEADER*)( pEthernetAddr + sizeof(ETHERNET_FRAME) + nIPLen );
		if( m_wPort && pUDPHeader->dest_port != m_wPort )
			return;			// m_wPort ��0 ��ʾ��Ҫ����ƥ�䣻�����ֽ���
		if(FALSE == AddData( pEthernetAddr+sizeof(ETHERNET_FRAME), nFrameLen-sizeof(ETHERNET_FRAME) ) )
			m_dwDataLostCount ++;
	}
}

///-------------------------------------------------------
/// CYJ,2007-5-10
/// ��������:
///		��ȡ IP/MPE�������
/// �������:
///		wPID			PID
/// ���ز���:
///		��
CDVBDSMCC_IP_MPE * CUDPDataPort::GetIPMPEObj( WORD wPID )
{
	wPID &= 0x1FFF;

#ifdef _DEBUG
	ASSERT( IP_MPE_HASH_LEN == 16 );
#endif //_DEBUG

	BYTE byHashID = wPID & (IP_MPE_HASH_LEN-1);

	CDVBDSMCC_IP_MPE * pItem = m_aIPMPE[byHashID];
	while( pItem )
	{
		if( pItem->m_wPID == wPID )
			return pItem;
		pItem = pItem->GetNextItem();
	}

	// not exist, create new instance
	pItem = new (NonPagedPool) CDVBDSMCC_IP_MPE( this, wPID );
	if( NULL == pItem )
		return NULL;

	if( m_aIPMPE[byHashID] )
		m_aIPMPE[byHashID]->AppendItem( pItem );
	else
		m_aIPMPE[byHashID] = pItem;

	return pItem;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COneNICAdapter::COneNICAdapter(TSDVBIFDRVDevice * pDevObj, int nAdapterNo )
 : m_DevSyncObj( ULONG(0) )   
{
	m_nAdaperNo = nAdapterNo;
	m_pDevObj = pDevObj;
	m_pAssociateNicDrv = NULL;
	memset( &m_AdapterInfo, 0, sizeof(m_AdapterInfo) );	
	memset( m_apDataPort, 0, sizeof(m_apDataPort) );
	m_pOwnerFileObj = NULL;
	m_nOpenDataPortMaxIndex = 0;
	m_bEnableDataReceive = TRUE;
}

COneNICAdapter::~COneNICAdapter()
{

}

///-------------------------------------------------------
/// CYJ,2007-3-1
/// ��������:
///		���ӵ�һ��NIC
/// �������:
///		pNicDrv				NIC ����
/// ���ز���:
///		true				�ɹ�
///		false				ʧ��
bool COneNICAdapter::Attach(ITSDVBHardwareDrvInterface *pNicDrv)
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	if( NULL == pNicDrv )
		return false;

	if( m_pAssociateNicDrv )
		return( pNicDrv == m_pAssociateNicDrv );

	m_bEnableDataReceive = TRUE;

	m_pAssociateNicDrv = pNicDrv;
	m_AdapterInfo.m_cbSize = sizeof(m_AdapterInfo); 
	return pNicDrv->GetVersionInfo( &m_AdapterInfo );
}

///-------------------------------------------------------
/// CYJ,2007-3-1
/// ��������:
///		�Ͽ���NIC����Ĺ���
/// �������:
///		��
/// ���ز���:
///		��
void COneNICAdapter::Detach()
{
	CMySingleLock SyncObj( &m_DevSyncObj );
	
	m_pAssociateNicDrv = NULL;
	m_bEnableDataReceive = FALSE;
}

// IMyUnknown
///-------------------------------------------------------
/// CYJ,2007-3-1
/// ��������:
///		��ѯ�ӿ�
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::QueryInterface(DWORD iid, void **ppvObject)
{
	if( ppvObject )
		*ppvObject = NULL;

	if( iid == IID_IMYUNKNOWN )
		*ppvObject = static_cast<IMyUnknown*>(this);
	else if( iid == IID_DVBCARD_IFDRV_INTERFACE )
		*ppvObject = static_cast<ITSDVBIFDrvInterface *>(this);
	else
		return STATUS_NOT_IMPLEMENTED;

	return STATUS_SUCCESS;
}

// ITSDVBIFDrvInterface
///-------------------------------------------------------
/// CYJ,2007-3-1
/// ��������:
///		��ѯ�汾
/// �������:
///		��
/// ���ز���:
///		�汾��
WORD COneNICAdapter::GetIFDrvVersion()
{
	return DRV_MAJOR_VERSION*0x100 + DRV_MINOR_VERSION;
}

///-------------------------------------------------------
/// CYJ,2007-3-1
/// ��������:
///		Ҫ����TS����Ϊ��λ���� pBuf �������е������� N ��������TS����
/// �������:
///		pBuf				TS ���黺����
///		nLen				����
/// ���ز���:
///		��
/// ˵����
///		�� pBuf = NULL || 0 == nLen ʱ����ʾˢ������
void COneNICAdapter::OnTSPacketReceived( IN PBYTE pBuf, IN int nLen )
{
	if( FALSE == m_bEnableDataReceive )
		return;
	
	CMySmartSpinLock SyncObj( m_DataPortSyncObj );
	if( NULL == pBuf || 0 == nLen )
	{							// ˢ������
		for(int j=0; j<=m_nOpenDataPortMaxIndex; j++ )
		{
			if( m_apDataPort[j] )
				m_apDataPort[j]->FlushCacheData();
		}		
		return;
	}

	ASSERT( (nLen%188) == 0 && pBuf[0] == 0x47 );
	int nPacketCount = nLen / 188;	
	
	for( int i=0; i<nPacketCount; i++ )
	{
		PDVB_TS_PACKET pPacket = (PDVB_TS_PACKET)pBuf;
		pBuf += 188;
		if( false == pPacket->IsTSPaket() )
			break;					// ���ִ��󣬷���

		if( 0 == m_FinalPIDBitMask.IsPIDOpenned( pPacket->GetPID() ) )
			continue;

		for(int j=0; j<=m_nOpenDataPortMaxIndex; j++ )
		{
			if( m_apDataPort[j] )
				m_apDataPort[j]->PushOneTSPacket( pPacket );
		}		
	}
}

///-------------------------------------------------------
/// CYJ,2009-3-14
/// ��������:
///		ע�� TS ����
/// �������:
///		��
/// ���ز���:
///		��
void COneNICAdapter::InjectTSPacketToDataPort( IN PBYTE pBuf, IN int nLen )
{
	if( FALSE == m_bEnableDataReceive )
		return;

	if( NULL == pBuf || 0 == nLen )
		return;
	
	CMySmartSpinLock SyncObj( m_DataPortSyncObj );

	ASSERT( (nLen%188) == 0 && pBuf[0] == 0x47 );
	int nPacketCount = nLen / 188;	
	
	for( int i=0; i<nPacketCount; i++ )
	{
		PDVB_TS_PACKET pPacket = (PDVB_TS_PACKET)pBuf;
		pBuf += 188;
		if( false == pPacket->IsTSPaket() )
			break;					// ���ִ��󣬷���

		for(int j=0; j<=m_nOpenDataPortMaxIndex; j++ )
		{
			if( m_apDataPort[j] )
				m_apDataPort[j]->ForceInjectOneTSPacket( pPacket );
		}		
	}
}

///-------------------------------------------------------
/// CYJ,2007-3-1
/// ��������:
///		���������رգ��类���û���USB���γ�
/// �������:
///		��
/// ���ز���:
///		��
void COneNICAdapter::OnAdapterClose()
{
	if( m_pDevObj )
		m_pDevObj->OnAdapterClose( this );
	
#ifdef _DEBUG
	ASSERT( m_pAssociateNicDrv == NULL );
#endif	//_DEBUG
}

///-------------------------------------------------------
/// CYJ,2007-3-2
/// ��������:
///		�������ݶ˿�
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::CreateDataPort(TSDVB_PID_TYPE nType, DWORD * pdwDataPortNo, PFILE_OBJECT pFileObject, DWORD dwIP, WORD wPort)
{
	ASSERT( pdwDataPortNo );

	CMySmartSpinLock SyncObj( m_DataPortSyncObj );

	if( TSDVB_REGISTER_MASTER_REGISTER_EXCLUSIVE == m_nRegisterMode )
	{			// ��ǰģʽע��Ϊ����ռģʽ��
		if( false == IsMaster( pFileObject ) )
			return STATUS_SHARING_VIOLATION;	// �����ͻ
	}

	int nPacketSize, nPacketCount;
	if( nType == TSDVB_PID_TYPE_RAW_DATA )
	{
		nPacketSize = DATAPORT_CACHE_TS_COUNT*DVB_TS_PACKET_SIZE;	// ���� 200 ��������ݣ�������� 54 Mb��(54000/8)/5 = 1.3 MB
		nPacketCount = 10000/DATAPORT_CACHE_TS_COUNT;				// 1000*188*8 ��Լ��Ӧ��75Mb��ռ���ڴ棺1.5 MB
	}
	else if( nType == TSDVB_PID_TYPE_UDP_DATAPORT )
	{		// Ethernet frame
		nPacketSize = 1600;			// ���֧�� 32Mbps������ 100 ���������
		nPacketCount = 1024;		// 512*1536*10 ���ڶ�Ӧ�� 62Mb��ռ���ڴ� 768 KB
	}
	else if( nType == TSDVB_PID_TYPE_UDP_NDIS )
	{		// Ethernet frame
		nPacketSize = 1600;
		nPacketCount = 2;			// ͨ��Windows ���緢���ˣ����в���Ҫ�ܴ�Ļ�����
	}
	else
		return STATUS_UNRECOGNIZED_MEDIA;

	for(int i=0; i<MAX_DATA_PORT_COUNT; i++)
	{		
		if( m_apDataPort[i] )
			continue;							// �Ѿ�����

		DWORD dwFullDataPortNo = MAKE_FULL_DATAPORT( m_nAdaperNo, i );
		COneDataPort * pDataPort = NULL;
		if( nType == TSDVB_PID_TYPE_RAW_DATA )
			pDataPort = new (NonPagedPool) COneDataPort( pFileObject, this, dwFullDataPortNo, nType );
		else if( nType == TSDVB_PID_TYPE_UDP_DATAPORT || nType == TSDVB_PID_TYPE_UDP_NDIS ) 
		{
			CUDPDataPort * pUDPDataPort =\
				new (NonPagedPool) CUDPDataPort( pFileObject, this, dwFullDataPortNo, nType, dwIP, wPort );
			if( pUDPDataPort )
				pDataPort = static_cast<COneDataPort * >(pUDPDataPort );
		}
		if( NULL == pDataPort )
			return STATUS_NO_MEMORY;

		if( FALSE == pDataPort->AllocateDataBuf( nPacketSize, nPacketCount ) )
		{
			delete pDataPort;
			return STATUS_NO_MEMORY;
		}
		*pdwDataPortNo = pDataPort->GetDataPortNo();
		m_apDataPort[i] = pDataPort;

		if( i > m_nOpenDataPortMaxIndex )
			m_nOpenDataPortMaxIndex = i;		// ����������˿�

		return STATUS_SUCCESS;
	}

#ifdef _DEBUG
	for(i=m_nOpenDataPortMaxIndex+1; i<MAX_DATA_PORT_COUNT; i++ )
	{
		ASSERT( m_apDataPort[i] == NULL );
	}
#endif //_DEBUG

	return STATUS_TOO_MANY_OPENED_FILES;		// ʧ�ܣ�̫��򿪵��ļ�
}

///-------------------------------------------------------
/// CYJ,2007-3-7
/// ��������:
///
/// �������:
///		dwDataPortNo			���ݶ˿����
/// ���ز���:
///		���ݶ˿ڶ���
COneDataPort * COneNICAdapter::GetDataPort( DWORD dwDataPortNo )
{	
	CMySmartSpinLock SyncObj( m_DataPortSyncObj );

	WORD wPortIndex = (WORD)( GET_DATAPORT_INDEX_FROM_DATAPORT( dwDataPortNo ) );
	if( wPortIndex >= MAX_DATA_PORT_COUNT )
		return NULL;
	return m_apDataPort[wPortIndex];
}

///-------------------------------------------------------
/// CYJ,2007-3-3
/// ��������:
///		ע��/ע�����ܶ���
/// �������:
///		pfileObject				�ļ�����
///		nRegisterMode			ע�᷽ʽ
/// ���ز���:
///		true					�ɹ�
///		false					ʧ�ܣ���Ϊ�Ѿ���ע��
// �޸ļ�¼��
//  CYJ,2008-9-24 ��һ��ע��ʱ�����³�ʼ�� ��Ƶͷ �� DiSEqC �豸
bool COneNICAdapter::RegisterAsMaster(PFILE_OBJECT pFileObject, TSDVB_REGISTER_MASTER_MODE nRegisterMode)
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	if( m_pOwnerFileObj && m_pOwnerFileObj != pFileObject )
		return false;

	if( TSDVB_REGISTER_MASTER_DEREGISTER == nRegisterMode )
	{
		m_pOwnerFileObj = NULL;
		m_nRegisterMode = TSDVB_REGISTER_MASTER_DEREGISTER;
		return true;
	}

	BOOL bDoTunerInit = ( NULL == m_pOwnerFileObj );

	m_pOwnerFileObj = pFileObject;
	m_nRegisterMode = nRegisterMode;

	//  CYJ,2008-9-24 ���³�ʼ����Ƶͷģ�顣
	if( bDoTunerInit )
		m_OneTunerObj.ReInitialize( pFileObject );

	return true;
}


///-------------------------------------------------------
/// CYJ,2007-3-5
/// ��������:
///		��λ����������
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::ResetAdapter( PFILE_OBJECT pFileObject )
{
	if( false == IsValid() )
		return STATUS_DEVICE_NOT_READY;

	if( false == IsMaster( pFileObject ) )
		return STATUS_NOT_ALL_ASSIGNED;

	CMySingleLock SyncObj( &m_DevSyncObj );	

	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;		

	m_pAssociateNicDrv->Reset();		

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2007-3-5
/// ��������:
///		֪ͨ��������ɵ�̨
/// �������:
///		��
/// ���ز���:
///		��
/// �޸ļ�¼
//  CYJ,2007-8-10 ��̨���ٴβ���ԭ���Ѿ��򿪵�PID�����������Զ��ָ��Ե�̨��
NTSTATUS COneNICAdapter::OnAfterTune(PFILE_OBJECT pFileObject)
{
	if( false == IsValid() )
		return STATUS_DEVICE_NOT_READY;

	if( false == IsMaster( pFileObject ) )
		return STATUS_NOT_ALL_ASSIGNED;

	CMySingleLock SyncObj( &m_DevSyncObj );	

	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;	
	
	m_pAssociateNicDrv->OnAfterTune();
	SyncObj.Unlock();

	//  CYJ,2008-11-24  �򿪿հ���ʹ֮���������е����ݰ�������
	m_pAssociateNicDrv->OpenPID( 0x1FFF );
	Sleep( 100 );		// �ӳ� 100 ms

	//  CYJ,2007-8-9 ��̨�ˣ����´����������򿪵�PID
	//  CYJ,2008-5-20 ���������ǰ�� m_DataPortSyncObj ͬ��֮ǰ���������Ա����� Dispatch ������ USB ����
	m_pAssociateNicDrv->CloseAllPID();
	Sleep( 40 );

	// �����������
	CMySmartSpinLock DataSyncObj( m_DataPortSyncObj );
	for(int i=0; i<m_nOpenDataPortMaxIndex; i++ )
	{
		COneDataPort * pDataPort = m_apDataPort[i];
		if( NULL == pDataPort )
			continue;
	
		pDataPort->AbortData();
	}	
	DataSyncObj.Unlock();		//  CYJ,2008-5-23 add
	
	SyncObj.Lock();				//  CYJ,2008-5-23 modify
	if( m_FinalPIDBitMask.IsAllPIDOpenned() )
		m_pAssociateNicDrv->OpenAllPID();
	else
	{
		for(WORD wPID = 0; wPID < 8192; wPID ++ )
		{
			if( m_FinalPIDBitMask.IsPIDOpenned(wPID) )
				m_pAssociateNicDrv->OpenPID( wPID );
		}
	}

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2007-3-5
/// ��������:
///		ɾ��һ�����ݶ˿�
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::DeleteDataPort(DWORD dwDataPortNo, PFILE_OBJECT pFileObject )
{
	CMySmartSpinLock SyncObj( m_DataPortSyncObj );

	DWORD dwIndex = GET_DATAPORT_INDEX_FROM_DATAPORT( dwDataPortNo );
	if( dwIndex >= MAX_DATA_PORT_COUNT )
		return STATUS_ARRAY_BOUNDS_EXCEEDED;

	COneDataPort * pDataPort = m_apDataPort[dwIndex];
	if( NULL == pDataPort )
		return STATUS_DEVICE_NOT_READY;

	if( pDataPort->GetOwnerFileObject() != pFileObject )
		return STATUS_INVALID_OWNER;				// ����ɾ�����Լ����������ݶ˿�

	pDataPort->CleanUp( pFileObject );
	// �Ƿ���Ҫ�������������

	m_apDataPort[dwIndex] = NULL;
	SyncObj.Unlock();				// �˴����������������ں����ɾ��PID�У�����Irq Level��
	if( (m_nOpenDataPortMaxIndex == (int)dwIndex) && m_nOpenDataPortMaxIndex )
		m_nOpenDataPortMaxIndex --;

	delete pDataPort;	

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2007-3-5
/// ��������:
///		���������
/// �������:
///		��
/// ���ز���:
///		��
void COneNICAdapter::CleanupDataPort( DWORD dwDataPortNo, PFILE_OBJECT pFileObject )
{
	CMySmartSpinLock SyncObj( m_DataPortSyncObj );

	DWORD dwIndex = GET_DATAPORT_INDEX_FROM_DATAPORT( dwDataPortNo );
	if( dwIndex >= MAX_DATA_PORT_COUNT )
		return;

	COneDataPort * pDataPort = m_apDataPort[dwIndex];
	if( NULL == pDataPort )
		return;

	pDataPort->FlushCacheData();		//  CYJ,2008-8-30 ���
	pDataPort->AbortData();
	pDataPort->CleanUp( pFileObject );
}

///-------------------------------------------------------
/// CYJ,2007-4-12
/// ��������:
///		�Ƿ������ȡ�հ�
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::AllowDummyTSPacket(DWORD dwDataPortNo, BOOL bEnable, BOOL * pbOldValue, PFILE_OBJECT pFileObject )
{
	ASSERT( pbOldValue );
	CMySmartSpinLock SyncObj( m_DataPortSyncObj );

	DWORD dwIndex = GET_DATAPORT_INDEX_FROM_DATAPORT( dwDataPortNo );
	if( dwIndex >= MAX_DATA_PORT_COUNT )
		return STATUS_ARRAY_BOUNDS_EXCEEDED;

	COneDataPort * pDataPort = m_apDataPort[dwIndex];
	if( NULL == pDataPort )
		return STATUS_DEVICE_NOT_READY;

	*pbOldValue = pDataPort->AllowDummyTSPacket( bEnable );

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2007-3-5
/// ��������:
///		��һ��PID
/// �������:
///		dwDataPort			���ݶ˿�
///		wPID				PIDֵ
///		I					��ص�IRP
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::OpenOnePID( DWORD dwDataPortNo, WORD wPID )
{
#ifdef _DEBUG
	t.Trace( TraceInfo, __FUNCDNAME__"(0x%08X,0x%04d)\n", dwDataPortNo, wPID );
#endif // _DEBUG

	CMySingleLock SyncObj( &m_DevSyncObj );

	ASSERT( (DWORD)m_nAdaperNo == GET_ADAPTER_NO_FROM_DATAPORT(dwDataPortNo) );
	WORD wDPIndex = (WORD)( GET_DATAPORT_INDEX_FROM_DATAPORT( dwDataPortNo ) );
	if( wDPIndex >= MAX_DATA_PORT_COUNT )
		return STATUS_ARRAY_BOUNDS_EXCEEDED;
	COneDataPort * pDataPort = m_apDataPort[ wDPIndex ];
	if( NULL == pDataPort )
		return STATUS_DEVICE_NOT_READY;

	pDataPort->OpenOnePID( wPID );

	if( 0 == m_FinalPIDBitMask.IsPIDOpenned(wPID) )
	{		// ֻ����һ���򿪣��ʹ�
		m_FinalPIDBitMask.OpenOnePID( wPID );

		ASSERT( m_pAssociateNicDrv );
		if( NULL == m_pAssociateNicDrv )
			return STATUS_DEVICE_NOT_READY;
		m_pAssociateNicDrv->OpenPID( wPID );
	}

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2007-3-5
/// ��������:
///		�ر�һ��PID
/// �������:
///		dwDataPort			���ݶ˿�
///		wPID				PIDֵ
///		I					��ص�IRP
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::CloseOnePID( DWORD dwDataPortNo, WORD wPID )
{
#ifdef _DEBUG
	t.Trace( TraceInfo, __FUNCDNAME__"(0x%08X,0x%04x)\n", dwDataPortNo, wPID );
#endif // _DEBUG

	CMySingleLock SyncObj( &m_DevSyncObj );

	ASSERT( (DWORD)m_nAdaperNo == GET_ADAPTER_NO_FROM_DATAPORT(dwDataPortNo) );
	WORD wDPIndex = (WORD)( GET_DATAPORT_INDEX_FROM_DATAPORT( dwDataPortNo ) );
	if( wDPIndex >= MAX_DATA_PORT_COUNT )
		return STATUS_ARRAY_BOUNDS_EXCEEDED;
	COneDataPort * pDataPort = m_apDataPort[ wDPIndex ];
	if( NULL == pDataPort )
		return STATUS_DEVICE_NOT_READY;

	pDataPort->CloseOnePID( wPID );
	
	ASSERT( m_FinalPIDBitMask.IsPIDOpenned( wPID ) );
	
	for(int i=0; i<=m_nOpenDataPortMaxIndex; i++ )
	{
		pDataPort = m_apDataPort[ i ];
		if( NULL == pDataPort )
			continue;
		if( pDataPort->IsPIDOpenned(wPID) )
			return STATUS_SUCCESS;			// ֻҪ��һ��û�йرգ��Ͳ��ܹر�
	}

	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	m_FinalPIDBitMask.CloseOnePID( wPID );
	m_pAssociateNicDrv->ClosePID( wPID );

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2007-3-5
/// ��������:
///		������PID
/// �������:
///		dwDataPort			���ݶ˿�
///		dwFlags				��־λ��
///							BIT0 => �Ƿ�������ӣ�1 => ����
///		I					��ص�IRP
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::OpenAllPID( DWORD dwDataPortNo, DWORD dwFlags )
{
#ifdef _DEBUG
	t.Trace( TraceInfo, __FUNCDNAME__"(0x%08X,0x%08x)\n", dwDataPortNo, dwFlags );
#endif // _DEBUG

	CMySingleLock SyncObj( &m_DevSyncObj );

	ASSERT( (DWORD)m_nAdaperNo == GET_ADAPTER_NO_FROM_DATAPORT(dwDataPortNo) );
	WORD wDPIndex = (WORD)( GET_DATAPORT_INDEX_FROM_DATAPORT( dwDataPortNo ) );
	if( wDPIndex >= MAX_DATA_PORT_COUNT )
		return STATUS_ARRAY_BOUNDS_EXCEEDED;
	COneDataPort * pDataPort = m_apDataPort[ wDPIndex ];
	if( NULL == pDataPort )
		return STATUS_DEVICE_NOT_READY;

	pDataPort->OpenAllPID();
	if( dwFlags & PID_OPEN_ALL_RAW_PID_FLAG_MONITOR_ONLY )	// �����Ǽ��ӣ���ʵ�ʴ�
		return STATUS_SUCCESS;

	m_FinalPIDBitMask.OpenAllPID();

	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;
	m_pAssociateNicDrv->OpenAllPID();

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2007-3-5
/// ��������:
///		�ر�����PID
/// �������:
///		dwDataPort			���ݶ˿�
///		I					��ص�IRP
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::CloseAllPID( DWORD dwDataPortNo )
{
#ifdef _DEBUG
	t.Trace( TraceInfo, __FUNCDNAME__"(0x%08X)\n", dwDataPortNo );
#endif // _DEBUG

	CMySingleLock SyncObj( &m_DevSyncObj );

	ASSERT( (DWORD)m_nAdaperNo == GET_ADAPTER_NO_FROM_DATAPORT(dwDataPortNo) );
	WORD wDPIndex = (WORD)( GET_DATAPORT_INDEX_FROM_DATAPORT( dwDataPortNo ) );
	if( wDPIndex >= MAX_DATA_PORT_COUNT )
		return STATUS_ARRAY_BOUNDS_EXCEEDED;
	COneDataPort * pDataPort = m_apDataPort[ wDPIndex ];
	if( NULL == pDataPort )
		return STATUS_DEVICE_NOT_READY;

	pDataPort->CloseAllPID();
	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	// �ȹر�ȫ����Ȼ�������δ�
	m_pAssociateNicDrv->CloseAllPID();
	m_FinalPIDBitMask.CloseAllPID();

	// ��ѯ�����˿ڣ��ָ�PID
	for(WORD wPID = 0; wPID < 8192; wPID ++ )
	{
		for(int i=0; i<=m_nOpenDataPortMaxIndex; i++ )
		{
			pDataPort = m_apDataPort[ i ];
			if( NULL == pDataPort )
				continue;
			if( pDataPort->IsPIDOpenned(wPID) )
			{
				m_FinalPIDBitMask.OpenOnePID( wPID );
				m_pAssociateNicDrv->OpenPID( wPID );
				break;
			}
		}
	}

	return STATUS_SUCCESS;
}


///-------------------------------------------------------
/// CYJ,2007-3-6
/// ��������:
///		д I2C
/// �������:
///		I				IRP
///		byAddress		I2C address
///		dwNbData		Bytes to write
///		pDataBuff		the data to be written
///		pbyOutValue		the I2C bus status after wrote
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::I2C_WriteReg( IN BYTE byAddress, IN DWORD dwNbData, IN PBYTE pDataBuff, BYTE * pbyOutValue )
{
	ASSERT( pbyOutValue );

	CMySingleLock SyncObj( &m_DevSyncObj );

	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	*pbyOutValue = m_pAssociateNicDrv->I2C_WriteReg( byAddress, dwNbData, pDataBuff );
	
	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2007-3-6
/// ��������:
///		�� I2C
/// �������:
///		I				IRP
///		byAddress		I2C Address
///		pbyDataOut		the data to be written before read, such as sub address.
///		dwOutCount		bytes to be written
///		dwNbData		bytes to read
///		pbyDataBuff		the buffer for reading
///		pdwOutValue		0 failed, 1 succ
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::I2C_ReadReg( IN BYTE byAddress, IN PBYTE pbyDataOut, IN DWORD dwOutCount, IN DWORD dwNbData, OUT PBYTE pbyDataBuff, OUT DWORD * pdwOutValue )
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	if( m_pAssociateNicDrv->I2C_ReadReg( byAddress, pbyDataOut, dwOutCount, dwNbData, pbyDataBuff ) )
		*pdwOutValue = dwNbData;
	else
		*pdwOutValue = 0;
	
	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2007-3-6
/// ��������:
///		��ȡ I2C �ٶ�
/// �������:
///		I				IRP
///		pnOutValue		old I2C speed
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::I2C_GetSpeed( OUT DWORD * pnOutValue )
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	*pnOutValue = m_pAssociateNicDrv->I2C_GetSpeed();

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2007-3-6
/// ��������:
///		���� I2C �ٶ�
/// �������:
///		I					IRP
///		nI2CClockKHz		new clock, in KHz
///		pnOutValue			old I2C speed
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::I2C_SetSpeed( IN int nI2CClockKHz, OUT DWORD * pnOutValue)
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	*pnOutValue = m_pAssociateNicDrv->I2C_SetSpeed( nI2CClockKHz );

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2007-3-6
/// ��������:
///		����д�Ƿ��� Restart ��ʼ
/// �������:
///		bAsRestart			begin write with restart signal
///		pdwOutValue			old setting
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::I2C_SetWriteWithRestart( IN bool bAsRestart, OUT DWORD * pdwOutValue )
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	*pdwOutValue = m_pAssociateNicDrv->I2C_SetWriteWithRestart( bAsRestart );

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2007-3-6
/// ��������:
///		����д����֮����ӳ�
/// �������:
///		I				IRP
///		nNewValUS		new delay, in uS
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::I2C_SetSleepUSBetweenReadReg( IN int nNewValUS, OUT DWORD * pnOutValue )
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	*pnOutValue = m_pAssociateNicDrv->I2C_SetSleepUSBetweenReadReg( nNewValUS );

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2007-3-14
/// ��������:
///		��ȡ Tuner ID
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::GetTunerID( DWORD * pdwOutValue )
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	*pdwOutValue = m_pAssociateNicDrv->GetTunerID();

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2007-3-8
/// ��������:
///		�ļ�������ر�
/// �������:
///		��
/// ���ز���:
///		��
void COneNICAdapter::OnFileClose( PFILE_OBJECT pFileObject )
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	for(int i=0; i<=m_nOpenDataPortMaxIndex; i++ )
	{
		COneDataPort * pDataPort = m_apDataPort[i];
		if( NULL == pDataPort || pDataPort->GetOwnerFileObject() != pFileObject )
			continue;

		pDataPort->AbortData();
		pDataPort->CleanUp( pFileObject );
		delete pDataPort;
		m_apDataPort[i] = NULL;
	}

	// ע��
	if( m_pOwnerFileObj && m_pOwnerFileObj == pFileObject )
	{
		m_pOwnerFileObj = NULL;
		m_nRegisterMode = TSDVB_REGISTER_MASTER_DEREGISTER;
	}

	m_nOpenDataPortMaxIndex = 0;
	for(i=0; i<MAX_DATA_PORT_COUNT; i++ )
	{
		if( m_apDataPort[i] )
			m_nOpenDataPortMaxIndex = i;
	}

	if( 0 == m_nOpenDataPortMaxIndex && NULL == m_apDataPort[0] )
	{
		m_FinalPIDBitMask.CloseAllPID();		
		if( m_pAssociateNicDrv )
			m_pAssociateNicDrv->CloseAllPID();			// û���κ����ݶ˿ڣ��ر�ȫ��PID
	}
}

///-------------------------------------------------------
/// CYJ,2007-3-15
/// ��������:
///		�ͷ� pDataPort ��Ӧ��PID
/// �������:
///		��
/// ���ز���:
///		��
void COneNICAdapter::FreeAllPIDsOfDataPort(COneDataPort *pDataPort)
{
	ASSERT( pDataPort );
	if( NULL == pDataPort )
		return;
	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return;

	CMySingleLock SyncObj( &m_DevSyncObj );
	
	for(WORD wPID=0; wPID<8192; wPID++ )
	{
		if( FALSE == pDataPort->IsPIDOpenned(wPID) )
			continue;
		pDataPort->CloseOnePID( wPID );
		ASSERT( m_FinalPIDBitMask.IsPIDOpenned( wPID ) );
		
		// ������û�������˿ڻ�ռ����
		BOOL bPIDUsing = FALSE;
		for(int i=0; i<=m_nOpenDataPortMaxIndex; i++ )
		{
			COneDataPort * pDPTmp = m_apDataPort[ i ];
			if( NULL == pDPTmp || pDPTmp == pDataPort )
				continue;
			if( pDPTmp->IsPIDOpenned(wPID) )
			{
				bPIDUsing = TRUE;
				break;
			}
		}		
		if( bPIDUsing )			// ��������
			continue;
		m_FinalPIDBitMask.CloseOnePID( wPID );
		m_pAssociateNicDrv->ClosePID( wPID );
	}
}

///-------------------------------------------------------
/// CYJ,2007-3-21
/// ��������:
///		CAM ���Ƿ�����ҽӺ�
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::CAM_IsCardReady(PFILE_OBJECT pFileObject,PBYTE pOutStatusByte)
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	if( false == IsMaster( pFileObject ) )
		return STATUS_NOT_ALL_ASSIGNED;

	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	return m_pAssociateNicDrv->CAM_IsCardReady( pOutStatusByte );
}

///-------------------------------------------------------
/// CYJ,2007-3-21
/// ��������:
///		��λ CAM ��
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::CAM_Reset(PFILE_OBJECT pFileObject)
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	if( false == IsMaster( pFileObject ) )
		return STATUS_NOT_ALL_ASSIGNED;

	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	return m_pAssociateNicDrv->CAM_Reset();
}

///-------------------------------------------------------
/// CYJ,2007-3-21
/// ��������:
///		CAM ���ڴ��
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::CAM_Memory_Read( PFILE_OBJECT pFileObject, IN WORD wAddress, OUT BYTE * pbyData )
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	if( false == IsMaster( pFileObject ) )
		return STATUS_NOT_ALL_ASSIGNED;

	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	return m_pAssociateNicDrv->CAM_Memory_Read( wAddress, pbyData );
}

///-------------------------------------------------------
/// CYJ,2007-3-21
/// ��������:
///		CAM ���ڴ�д
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::CAM_Memory_Write( PFILE_OBJECT pFileObject, IN WORD wAddress, OUT BYTE byData )
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	if( false == IsMaster( pFileObject ) )
		return STATUS_NOT_ALL_ASSIGNED;

	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	return m_pAssociateNicDrv->CAM_Memory_Write( wAddress, byData );
}

///-------------------------------------------------------
/// CYJ,2007-3-21
/// ��������:
///		CAM ��IO ��
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::CAM_IO_Read( PFILE_OBJECT pFileObject, IN WORD wAddress, OUT BYTE * pbyData )
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	if( false == IsMaster( pFileObject ) )
		return STATUS_NOT_ALL_ASSIGNED;

	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	return m_pAssociateNicDrv->CAM_IO_Read( wAddress, pbyData );
}

///-------------------------------------------------------
/// CYJ,2007-3-21
/// ��������:
///		CAM �� IO д
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::CAM_IO_Write( PFILE_OBJECT pFileObject, IN WORD wAddress, OUT BYTE byData )
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	if( false == IsMaster( pFileObject ) )
		return STATUS_NOT_ALL_ASSIGNED;

	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	return m_pAssociateNicDrv->CAM_IO_Write( wAddress, byData );
}

///-------------------------------------------------------
/// CYJ,2007-3-21
/// ��������:
///		CAM ���Ƿ���·
/// �������:
///		��
/// ���ز���:
///		��
BOOL	 COneNICAdapter::CAM_Is_Bypasseded(PFILE_OBJECT pFileObject)
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	if( false == IsMaster( pFileObject ) )
		return STATUS_NOT_ALL_ASSIGNED;

	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	return m_pAssociateNicDrv->CAM_Is_Bypasseded();
}

///-------------------------------------------------------
/// CYJ,2007-3-21
/// ��������:
///		���� CAM ����·
/// �������:
///		bBypassed			�Ƿ���·
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::CAM_SetBypassed( PFILE_OBJECT pFileObject, BOOL bBypassed )
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	if( false == IsMaster( pFileObject ) )
		return STATUS_NOT_ALL_ASSIGNED;

	ASSERT( m_pAssociateNicDrv );
	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	return m_pAssociateNicDrv->CAM_SetBypassed( bBypassed );
}

///-------------------------------------------------------
/// CYJ,2007-4-13
/// ��������:
///		��ȡ�汾��Ϣ
/// �������:
///		pVersionInfo		����汾��
/// ���ز���:
///		STATUS_SUCCESS		�ɹ�
NTSTATUS COneNICAdapter::GetVersionInfo( PTSDVB_INTERNAL_VERSION_INFO pVersionInfo )
{
	memset( pVersionInfo, 0, sizeof(TSDVB_INTERNAL_VERSION_INFO) );
	pVersionInfo->m_wSize = sizeof(TSDVB_INTERNAL_VERSION_INFO);

	pVersionInfo->m_dwFirmwareVersion = m_AdapterInfo.m_dwFirmwareVersion;	// Ӳ���汾	

	pVersionInfo->m_wDriverVersion = m_AdapterInfo.m_wVersion;				// �����汾
	pVersionInfo->m_wDriverBuildNo = m_AdapterInfo.m_wBuildNo;				// ���������
	
	memcpy( pVersionInfo->m_abyIDCode, m_AdapterInfo.m_abyIDCode, 8 );
	memcpy( pVersionInfo->m_abyMACAddr, m_AdapterInfo.m_abyMACAddr, 6 );
	
	pVersionInfo->m_dwCapability = m_AdapterInfo.m_dwCapability ;				// ����������
	pVersionInfo->m_byMCU_I2C_Address = m_AdapterInfo.m_byMCU_I2C_Address;		// I2C address

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2007-4-16
/// ��������:
///		��ȡ���ⰴ��
/// �������:
///		��
/// ���ز���:
///		��
/// ˵��
///		�� Master ��ɫ����ִ�и�ָ��
NTSTATUS COneNICAdapter::IR_GetKey( PFILE_OBJECT pFileObject, DWORD * pdwIRKey )
{
#if 0	//  CYJ,2008-12-8��ȡ���� GetIRKey �������ж�
	if( false == IsMaster( pFileObject ) )
		return STATUS_NOT_ALL_ASSIGNED;
#endif // 0

	CMySingleLock SyncObj( &m_DevSyncObj );	

	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	return m_pAssociateNicDrv->IRD_GetKeyPressed( *pdwIRKey );
}

///-------------------------------------------------------
/// CYJ,2007-5-5
/// ��������:
///		�Ƿ��������
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::EnableReceive(PFILE_OBJECT pFileObject, BOOL bEnable, BOOL *pOutOldValue)
{
	if( false == IsMaster( pFileObject ) )
		return STATUS_NOT_ALL_ASSIGNED;
	
	CMySingleLock SyncObj( &m_DevSyncObj );	

	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	if( pOutOldValue )	
		*pOutOldValue = m_bEnableDataReceive;	

	if( bEnable )
		m_pAssociateNicDrv->ClosePID( 0x1FFF );	// ���ý����ˣ���ֹ���տհ�
	else
		m_pAssociateNicDrv->OpenPID( 0x1FFF );	// ���տհ�������ǰ�����ݡ���������

	m_bEnableDataReceive = bEnable;

	SyncObj.Unlock();

	if( FALSE == bEnable )
	{					// clean data port data
		CMySmartSpinLock SyncObj( m_DataPortSyncObj );
		for(int j=0; j<=m_nOpenDataPortMaxIndex; j++ )
		{
			if( m_apDataPort[j] )
				m_apDataPort[j]->AbortData();
		}		
	}

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2007-5-10
/// ��������:
///		���յ�һ����̫֡
/// �������:
///		��
/// ���ز���:
///		��
void COneNICAdapter::OnEthernetFrame( PBYTE pEthernetAddr, int nFrameLen )
{
	if( NULL == m_pAssociateNicDrv )
		return;

	ETHERNET_FRAME * pFrame = (ETHERNET_FRAME*)pEthernetAddr;
	memcpy( &pFrame->Source, m_AdapterInfo.m_abyMACAddr, 6 );
	pFrame->FrameType = 8;		// Ethernet frame IP type

	m_pAssociateNicDrv->SendOutEthernetData( pEthernetAddr, nFrameLen );
}

///-------------------------------------------------------
/// CYJ,2007-5-23
/// ��������:
///		��ѯ���ݶ˿����ݽ���״̬
/// �������:
///		dwDataPortNo		���ݶ˿ں�
///		pdwOutTSPacketTotal	���յ���TS�������
///		pdwOutLostPacket	��ʧ�����ݸ���
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::GetDataPortStatus(DWORD dwDataPortNo, DWORD * pdwOutTSPacketTotal, DWORD * pdwOutLostPacket)
{
	CMySmartSpinLock SyncObj( m_DataPortSyncObj );

	ASSERT( (DWORD)m_nAdaperNo == GET_ADAPTER_NO_FROM_DATAPORT(dwDataPortNo) );
	WORD wDPIndex = (WORD)( GET_DATAPORT_INDEX_FROM_DATAPORT( dwDataPortNo ) );
	if( wDPIndex >= MAX_DATA_PORT_COUNT )
		return STATUS_ARRAY_BOUNDS_EXCEEDED;
	COneDataPort * pDataPort = m_apDataPort[ wDPIndex ];
	if( NULL == pDataPort )
		return STATUS_DEVICE_NOT_READY;

	pDataPort->QueryDataPortStatus( pdwOutTSPacketTotal, pdwOutLostPacket );

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2008-3-20
/// ��������:
///		ͬ����ȡ����
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::ReadDataPortSync( DWORD dwDataPortNo, PBYTE pBuf, DWORD dwLen, DWORD * pdwOutBytes )
{
	CMySmartSpinLock SyncObj( m_DataPortSyncObj );

	ASSERT( (DWORD)m_nAdaperNo == GET_ADAPTER_NO_FROM_DATAPORT(dwDataPortNo) );
	WORD wDPIndex = (WORD)( GET_DATAPORT_INDEX_FROM_DATAPORT( dwDataPortNo ) );
	if( wDPIndex >= MAX_DATA_PORT_COUNT )
		return STATUS_ARRAY_BOUNDS_EXCEEDED;
	COneDataPort * pDataPort = m_apDataPort[ wDPIndex ];
	if( NULL == pDataPort )
		return STATUS_DEVICE_NOT_READY;

	return pDataPort->ReadDataSync( pBuf, dwLen, pdwOutBytes );
}

///-------------------------------------------------------
/// CYJ,2008-5-20
/// ��������:
///		���� Tuner ����
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::CreateTunerObject( KLowerDevice * pTunerDev )
{
	ASSERT( pTunerDev );
	return m_OneTunerObj.Initialize( this, pTunerDev );
}

///-------------------------------------------------------
/// CYJ,2008-5-20
/// ��������:
///		��̨
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::Tuner_Tune( PFILE_OBJECT pFileObject, PTSDVB_TUNERPARAM pTuneParam )
{
	CMySingleLock SyncObj( &m_DevSyncObj );

	ASSERT( pFileObject && pTuneParam );
	if( NULL == pFileObject || NULL == pTuneParam )
		return STATUS_INVALID_PARAMETER;

	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	if( FALSE == m_OneTunerObj.IsValid() )
		return STATUS_DEVICE_NOT_CONNECTED;

	return m_OneTunerObj.Tune( pFileObject, pTuneParam );
}

///-------------------------------------------------------
/// CYJ,2008-11-11
/// ��������:
///		ͨ������ ��̨
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::Tuner_TuneProxyed( PFILE_OBJECT pFileObject, PTSDVB_TUNERPARAM pTuneParam, PTSDVB_TUNE_PROXY_DATA * ppTuneResult )
{
#ifdef _DEBUG
	DbgPrint( "Tuner_TuneProxyed, FileObject=%p \n", pFileObject );
#endif //_DEBUG

	ASSERT( pFileObject && pTuneParam && ppTuneResult );
	if( NULL == pFileObject || NULL == pTuneParam || NULL == ppTuneResult )
		return STATUS_INVALID_PARAMETER;

	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;

	if( FALSE == m_OneTunerObj.IsValid() )
		return STATUS_DEVICE_NOT_CONNECTED;

	if( FALSE == m_NotifyProxyToDoTuneEvent.IsValid() || FALSE == m_ProxyHasDoneTuneEvent.IsValid() )
	{	// δʹ�����ô���

	#ifdef _DEBUG
		DbgPrint( "Call Tuner_Tune, Directly, since No Proxy Setted\n" );
	#endif //_DEBUG

		return Tuner_Tune( pFileObject, pTuneParam );
	}

#ifdef _DEBUG
	DbgPrint( "Tuner_TuneProxyed, Wait for proxy tunning\n" );
#endif //_DEBUG

	memcpy( &m_ProxyTuneParam.m_TuneParam, pTuneParam, sizeof(TSDVB_TUNERPARAM) );

	m_NotifyProxyToDoTuneEvent.Set();
	m_ProxyHasDoneTuneEvent.Clear();
	m_nProxyTuneRetVal = STATUS_IO_TIMEOUT;

	// �ȴ����������ɵ�̨
	LARGE_INTEGER llTimeOut;
	llTimeOut.QuadPart = -(10I64)*1000*1000*10;		// 10 Second timeout
	m_ProxyHasDoneTuneEvent.Wait( KernelMode,FALSE,&llTimeOut );

	m_NotifyProxyToDoTuneEvent.Clear();
	m_ProxyHasDoneTuneEvent.Clear();

	*ppTuneResult = &m_ProxyTuneParam;

#ifdef _DEBUG
	DbgPrint( "Tuner_TuneProxyed, Tune Done. Result=0x%08X\n", m_nProxyTuneRetVal );
#endif //_DEBUG

	return m_nProxyTuneRetVal;
}

///-------------------------------------------------------
/// CYJ,2008-5-21
/// ��������:
///		���� DiSEqC ����
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::Tuner_SendDiSEqCCommand( PFILE_OBJECT pFileObject, PBYTE pBuf, int nLen, long * pnByteSend )
{
	CMySingleLock SyncObj( &m_DevSyncObj );	

	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;
	if( FALSE == m_OneTunerObj.IsValid() )
		return STATUS_DEVICE_NOT_CONNECTED;

	ASSERT( pFileObject && pBuf && nLen && pnByteSend );
	if( NULL == pFileObject || NULL == pBuf || nLen <= 0 || NULL == pnByteSend )
		return STATUS_INVALID_PARAMETER;

	if( false == IsMaster( pFileObject ) )
		return STATUS_SHARING_VIOLATION;

	int nByteSend = m_OneTunerObj.SendDiSEqCCommand( pFileObject, pBuf, nLen );

	*pnByteSend = nByteSend;

	return (nByteSend > 0 ? STATUS_SUCCESS : STATUS_RECEIVE_PARTIAL );
}

///-------------------------------------------------------
/// CYJ,2008-5-21
/// ��������:
///		�ź��Ƿ�����
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::Tuner_IsSignalLocked( BOOL * pbIsLocked )
{
	CMySingleLock SyncObj( &m_DevSyncObj );	

	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;
	if( FALSE == m_OneTunerObj.IsValid() )
		return STATUS_DEVICE_NOT_CONNECTED;

	ASSERT( pbIsLocked );
	if( NULL == pbIsLocked )
		return STATUS_INVALID_PARAMETER;

	*pbIsLocked = m_OneTunerObj.IsSingalLocked();

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2008-5-21
/// ��������:
///		��ȡ�ź�
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::Tuner_GetSignal( PTSDVB_TUNER_SIGNAL_STATUS pSignalStatus, BOOL * pbIsLocked )
{
	CMySingleLock SyncObj( &m_DevSyncObj );	

	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;
	if( FALSE == m_OneTunerObj.IsValid() )
		return STATUS_DEVICE_NOT_CONNECTED;

	ASSERT( pSignalStatus );
	if( NULL == pSignalStatus )
		return STATUS_INVALID_PARAMETER;

	BOOL bIsLocked = m_OneTunerObj.GetSignal( pSignalStatus );

	if( pbIsLocked )
		*pbIsLocked = bIsLocked;

	return STATUS_SUCCESS;		// locked
}

///-------------------------------------------------------
/// CYJ,2008-5-21
/// ��������:
///		��ȡ��֤��ʵ�ʵĸ�Ƶͷ����
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::Tuner_GetModulationTypeChecked( MODULATIONTYPE * pOutValue )
{
	CMySingleLock SyncObj( &m_DevSyncObj );	

	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;
	if( FALSE == m_OneTunerObj.IsValid() )
		return STATUS_DEVICE_NOT_CONNECTED;

	ASSERT( pOutValue );
	if( NULL == pOutValue )
		return STATUS_INVALID_PARAMETER;

	*pOutValue = m_OneTunerObj.GetModulationType();

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2008-5-21
/// ��������:
///		���õ�ѹ
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::Tuner_SetVoltage( PFILE_OBJECT pFileObject, TSDVB_TUNER_VOLTAGE nVoltage )
{
	CMySingleLock SyncObj( &m_DevSyncObj );	

	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;
	if( FALSE == m_OneTunerObj.IsValid() )
		return STATUS_DEVICE_NOT_CONNECTED;

	if( false == IsMaster( pFileObject ) )
		return STATUS_SHARING_VIOLATION;

	m_OneTunerObj.SetVoltage( pFileObject, nVoltage );

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2008-5-21
/// ��������:
///		���� 22KHz ��Ƶ
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::Tuner_Set22KHzToneOn(PFILE_OBJECT pFileObject, BOOL bOn )
{
	CMySingleLock SyncObj( &m_DevSyncObj );	

	if( NULL == m_pAssociateNicDrv )
		return STATUS_DEVICE_NOT_READY;
	if( FALSE == m_OneTunerObj.IsValid() )
		return STATUS_DEVICE_NOT_CONNECTED;

	if( false == IsMaster( pFileObject ) )
		return STATUS_SHARING_VIOLATION;

	m_OneTunerObj.Set22KHzToneOn( pFileObject, bOn );

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2008-11-11
/// ��������:
///		���õ�̨����
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::TuneProxy_Start( PFILE_OBJECT pFileObject, HANDLE hEvent )
{
#ifdef _DEBUG
	DbgPrint( "TuneProxy_Start, FileObject=%p, hEvent=%p.\n", pFileObject, hEvent );
#endif //_DEBUG

	ASSERT( pFileObject );
	if( false == IsMaster(pFileObject) )
		return STATUS_SHARING_VIOLATION;	// �����ͻ

	if( NULL == hEvent )
		return STATUS_INVALID_PARAMETER;

	if( m_ProxyHasDoneTuneEvent.IsValid() )
		m_ProxyHasDoneTuneEvent.Invalidate();
	if( m_NotifyProxyToDoTuneEvent.IsValid() )
		m_NotifyProxyToDoTuneEvent.Invalidate();

	m_ProxyHasDoneTuneEvent.Initialize( NotificationEvent );
	m_NotifyProxyToDoTuneEvent.Initialize( hEvent );

	m_NotifyProxyToDoTuneEvent.Clear();
	m_ProxyHasDoneTuneEvent.Clear();

	if( FALSE == m_ProxyHasDoneTuneEvent.IsValid() || FALSE == m_NotifyProxyToDoTuneEvent.IsValid() )
		return STATUS_OBJECT_NAME_INVALID;
	
	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2008-11-11
/// ��������:
///		ֹͣ��̨����
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::TuneProxy_Stop( PFILE_OBJECT pFileObject )
{
#ifdef _DEBUG
	DbgPrint( "TuneProxy_Stop, FileObject=%p.\n", pFileObject );
#endif //_DEBUG

	ASSERT( pFileObject );
	if( false == IsMaster(pFileObject) )
		return STATUS_SHARING_VIOLATION;	// �����ͻ

	if( m_ProxyHasDoneTuneEvent.IsValid() )
		m_ProxyHasDoneTuneEvent.Invalidate();
	if( m_NotifyProxyToDoTuneEvent.IsValid() )
		m_NotifyProxyToDoTuneEvent.Invalidate();

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2008-11-11
/// ��������:
///		��ȡ��̨��������
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::TuneProxy_GetTuneRequest( PFILE_OBJECT pFileObject, PTSDVB_TUNERPARAM pTuneParam )
{
#ifdef _DEBUG
	DbgPrint( "TuneProxy_GetTuneRequest, %p Get Tune Result.\n", pFileObject );
#endif //_DEBUG

	ASSERT( pFileObject );
	if( false == IsMaster(pFileObject) )
		return STATUS_SHARING_VIOLATION;	// �����ͻ

	if( NULL == pTuneParam )
		return STATUS_INVALID_PARAMETER;

	memcpy( pTuneParam, &m_ProxyTuneParam.m_TuneParam, sizeof(TSDVB_TUNERPARAM) );	

	return STATUS_SUCCESS;
}

///-------------------------------------------------------
/// CYJ,2008-11-11
/// ��������:
///		���õ�̨������
/// �������:
///		��
/// ���ز���:
///		��
NTSTATUS COneNICAdapter::TuneProxy_SetTuneResult( PFILE_OBJECT pFileObject, PTSDVB_TUNE_PROXY_DATA pData )
{
#ifdef _DEBUG
	DbgPrint( "TuneProxy_SetTuneResult, %p Set Tune Result. 0x%08X\n", pFileObject, pData->m_TuneResult.m_nResult );
#endif //_DEBUG

	ASSERT( pFileObject && pData );
	if( NULL == pFileObject || NULL == pData )
		return STATUS_INVALID_PARAMETER;

	if( false == IsMaster(pFileObject) )
		return STATUS_SHARING_VIOLATION;	// �����ͻ

	memcpy( &m_ProxyTuneParam, pData, sizeof(m_ProxyTuneParam) );

	m_nProxyTuneRetVal = pData->m_TuneResult.m_nResult;

	m_ProxyHasDoneTuneEvent.Set();

	return STATUS_SUCCESS;
}
