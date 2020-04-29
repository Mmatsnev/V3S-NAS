// OneNICAdapter.h: interface for the COneNICAdapter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ONENICADAPTER_H__62B31C4D_8107_49A4_A17B_686C5AAE3949__INCLUDED_)
#define AFX_ONENICADAPTER_H__62B31C4D_8107_49A4_A17B_686C5AAE3949__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../../DVBCards/TSDVBHardwareIF.H"
#include <WIN32DPDataMgr.h>
#include "../TSDVBIFDRVInterface.h"
#include <MyDVBPSI/tspacket.h>
#include "../OneNICAdapterInterface.h"
#include "OneTunerObj.h"
#include "../DVBTuneProxy.h"

class TSDVBIFDRVDevice;
class COneNICAdapter;
class CDVBDSMCC_IP_MPE;

#define MAKE_FULL_DATAPORT( nAdapterNo, nDataPortIndex )		( ((nAdapterNo)<<16) + (nDataPortIndex) )
#define GET_ADAPTER_NO_FROM_DATAPORT( dwFullDataPortNo )		( (dwFullDataPortNo)>>16 )
#define GET_DATAPORT_INDEX_FROM_DATAPORT( dwFullDataPortNo )	( (dwFullDataPortNo)&0xFFFF )

const DWORD DATAPORT_CACHE_TS_COUNT = 21;		// ��Ϊ PCI ��������Ϊ 21

class CPIDBitmaskMgr
{
public:
	CPIDBitmaskMgr();
	~CPIDBitmaskMgr();

	void OpenOnePID( WORD wPID );
	void CloseOnePID( WORD wPID );
	BYTE IsPIDOpenned( WORD wPID );	

	void OpenAllPID();
	void CloseAllPID();
	bool IsAllPIDOpenned(){ return m_bIsAllPIDOpenned; }

protected:
	BYTE	m_abyPIDBitMask[1024];		// ���PIDΪ8092��ÿ�����ر�ʾ1λ������Ҫ1KB��1 ��ʾ�򿪣�0 ��ʾ�ر�
	bool	m_bIsAllPIDOpenned;
};

class COneDataPort : public CWIN32DPDataMgr, public CPIDBitmaskMgr
{
public:
	void AbortData();
	COneDataPort( PFILE_OBJECT pOwnerFileObj, COneNICAdapter * pAdapter, DWORD dwDataPortNo, TSDVB_PID_TYPE nType );
	virtual ~COneDataPort();
	const DWORD GetDataPortNo()const{ return m_dwDataPortNo; }
	static DWORD DataPortGetAdapterNo( DWORD dwDataPortNo )
	{ return GET_ADAPTER_NO_FROM_DATAPORT(dwDataPortNo); }

	virtual void PushOneTSPacket( PDVB_TS_PACKET pTSPacket );
	void ForceInjectOneTSPacket( PDVB_TS_PACKET pTSPacket );
	const PFILE_OBJECT GetOwnerFileObject()const { return m_pOwenerFileObject; }
	virtual DWORD GetMaxCopyDataSize( DWORD dwOutMemSize );

	BOOL AllowDummyTSPacket( BOOL bEnable );
	void FlushCacheData();
	void QueryDataPortStatus( DWORD * pdwPacketReceived, DWORD * pdwPacketLost );

	NTSTATUS ReadDataSync( KIrp I);
	NTSTATUS ReadDataSync( PBYTE pOutBuf, DWORD dwBufSize, DWORD * pdwByteRead );

protected:
	COneNICAdapter * m_pAdapter;
	TSDVB_PID_TYPE	m_nDataType;
	DWORD	m_dwDataPortNo;
	DWORD	m_dwDataLostCount;
	DWORD	m_dwTSPacketReceived;			// ���յ���TS��������
	PFILE_OBJECT m_pOwenerFileObject;
	BOOL m_bAllowDummyTSPacket;				// �Ƿ������ȡ�հ�(PID=0x1FFF)��ȱʡΪ  FALSE

	DVB_TS_PACKET	m_aTSPacketCache[DATAPORT_CACHE_TS_COUNT];
	int	m_nCachePacketCount;
};

///-------------------------------------------------------
/// CYJ,2007-3-7
/// ��������:
///		���緽ʽ�����ݶ˿�
/// �������:
///		��
/// ���ز���:
///		��
class CUDPDataPort : public COneDataPort
{
public:
	CUDPDataPort( PFILE_OBJECT pOwnerFileObj, COneNICAdapter * pAdapter, DWORD dwDataPortNo, TSDVB_PID_TYPE nType, DWORD dwIP, WORD wPort );
	virtual ~CUDPDataPort();

	virtual DWORD GetMaxCopyDataSize( DWORD dwOutMemSize );
	virtual void PushOneTSPacket( PDVB_TS_PACKET pTSPacket );
	void OnEthernetFrame( PBYTE pEthernetAddr, int nFrameLen );

	enum
	{
		IP_MPE_HASH_LEN = 16,
	};

private:
	CDVBDSMCC_IP_MPE * GetIPMPEObj( WORD wPID );

private:
	DWORD	m_dwDstIP;		// 0 ��ʾͨ�䣬��֮��ֻ���ն�Ӧ��IP;  �����ֽ���
	WORD	m_wPort;		// 0 ��ʾͨ�䣬��֮��ֻ���ն�Ӧ�Ķ˿�;  �����ֽ���
	CDVBDSMCC_IP_MPE * m_aIPMPE[IP_MPE_HASH_LEN];
};


class COneNICAdapter : public ITSDVBIFDrvInterface, public IOneNICAdapterInterface
{
public:
	COneNICAdapter(TSDVBIFDRVDevice * pDevObj, int nAdapterNo );
	virtual ~COneNICAdapter();

	// IMyUnknown
	virtual NTSTATUS QueryInterface(DWORD iid, void **ppvObject);

	// ITSDVBIFDrvInterface
	virtual WORD GetIFDrvVersion();											// ��ȡ�ӿ������İ汾��
	virtual void OnTSPacketReceived( IN PBYTE pBuf, IN int nLen );			// Ҫ����TS����Ϊ��λ���� pBuf �������е������� N ��������TS����
	virtual void OnAdapterClose();											// ���������رգ��类���û���USB���γ�

	void InjectTSPacketToDataPort( IN PBYTE pBuf, IN int nLen );

	bool Attach( ITSDVBHardwareDrvInterface * pNicDrv );
	void Detach();
	const TSDVBHWDRV_VERSION_INFO * GetAdapterInfo(){ return &m_AdapterInfo; }
	NTSTATUS CreateTunerObject( KLowerDevice * pTunerDev );

	NTSTATUS TuneProxy_Start( PFILE_OBJECT pFileObject, HANDLE hEvent );
	NTSTATUS TuneProxy_Stop( PFILE_OBJECT pFileObject );
	NTSTATUS TuneProxy_GetTuneRequest( PFILE_OBJECT pFileObject, PTSDVB_TUNERPARAM pTuneParam );
	NTSTATUS TuneProxy_SetTuneResult( PFILE_OBJECT pFileObject, PTSDVB_TUNE_PROXY_DATA pData );

	enum
	{
		MAX_DATA_PORT_COUNT = 128,
	};

	bool IsMaster( PFILE_OBJECT pFileObject ) const 
	{ return (NULL == m_pOwnerFileObj) || (m_pOwnerFileObj == pFileObject); }

	bool IsValid()const{ return (m_pAssociateNicDrv != NULL); }
	
	virtual void OnFileClose( PFILE_OBJECT pFileObject );

public:	
	virtual bool RegisterAsMaster( PFILE_OBJECT pFileObject, TSDVB_REGISTER_MASTER_MODE nRegisterMode );
	virtual NTSTATUS CreateDataPort(TSDVB_PID_TYPE nType, DWORD * pdwDataPortNo, PFILE_OBJECT pFileObject, DWORD dwIP=0, WORD wPort=0 );
	virtual NTSTATUS GetVersionInfo( PTSDVB_INTERNAL_VERSION_INFO pVersionInfo );
	
	virtual void CleanupDataPort( DWORD dwDataPortNo, PFILE_OBJECT pFileObject );
	virtual NTSTATUS EnableReceive( PFILE_OBJECT pFileObject, BOOL bEnable, BOOL * pOutOldValue );

	virtual NTSTATUS DeleteDataPort( DWORD dwDataPortNo, PFILE_OBJECT pFileObject );
	virtual NTSTATUS OnAfterTune( PFILE_OBJECT pFileObject );
	virtual NTSTATUS ResetAdapter( PFILE_OBJECT pFileObject );
	virtual NTSTATUS AllowDummyTSPacket(DWORD dwDataPortNo, BOOL bEnable, BOOL * pbOldValue, PFILE_OBJECT pFileObject );
	virtual NTSTATUS GetDataPortStatus(DWORD dwDataPortNo, DWORD * pdwOutTSPacketTotal, DWORD * pdwOutLostPacket);
	virtual NTSTATUS ReadDataPortSync( DWORD dwDataPortNo, PBYTE pBuf, DWORD dwLen, DWORD * pdwOutBytes );

	virtual NTSTATUS OpenOnePID( DWORD dwDataPortNo, WORD wPID );
	virtual NTSTATUS CloseOnePID( DWORD dwDataPortNo, WORD wPID );
	virtual NTSTATUS OpenAllPID( DWORD dwDataPortNo, DWORD dwFlags );
	virtual NTSTATUS CloseAllPID( DWORD dwDataPortNo );

	virtual NTSTATUS I2C_WriteReg( IN BYTE byAddress, IN DWORD dwNbData, IN PBYTE pDataBuff, BYTE * pbyOutValue );
	virtual NTSTATUS I2C_ReadReg( IN BYTE byAddress, IN PBYTE pbyDataOut, IN DWORD dwOutCount, IN DWORD dwNbData, OUT PBYTE pbyDataBuff, OUT DWORD * pdwOutValue ); 
	virtual NTSTATUS I2C_GetSpeed( OUT DWORD * pnOutValue );
	virtual NTSTATUS I2C_SetSpeed( IN int nI2CClockKHz, OUT DWORD * pnOutValue);
	virtual NTSTATUS I2C_SetWriteWithRestart( IN bool bAsRestart, OUT DWORD * pdwOutValue );
	virtual NTSTATUS I2C_SetSleepUSBetweenReadReg( IN int nNewValUS, OUT DWORD * pnOutValue );

	// GetTunerID
	virtual NTSTATUS GetTunerID( DWORD * pdwOutValue );
	virtual NTSTATUS Tuner_Tune( PFILE_OBJECT pFileObject, PTSDVB_TUNERPARAM pTuneParam );
	virtual NTSTATUS Tuner_TuneProxyed( PFILE_OBJECT pFileObject, PTSDVB_TUNERPARAM pTuneParam, PTSDVB_TUNE_PROXY_DATA * ppTuneResult );
	virtual NTSTATUS Tuner_SendDiSEqCCommand( PFILE_OBJECT pFileObject, PBYTE pBuf, int nLen, long * pnByteSend );
	virtual NTSTATUS Tuner_IsSignalLocked( BOOL * pbIsLocked );
	virtual NTSTATUS Tuner_GetSignal( PTSDVB_TUNER_SIGNAL_STATUS pSignalStatus, BOOL * pbIsLocked );
	virtual NTSTATUS Tuner_GetModulationTypeChecked( MODULATIONTYPE * pOutValue );
	virtual NTSTATUS Tuner_SetVoltage( PFILE_OBJECT pFileObject, TSDVB_TUNER_VOLTAGE nVoltage );
	virtual NTSTATUS Tuner_Set22KHzToneOn( PFILE_OBJECT pFileObject, BOOL bOn );

	// CAM
	virtual NTSTATUS CAM_IsCardReady(PFILE_OBJECT pFileObject,PBYTE pOutStatusByte);
	virtual NTSTATUS CAM_Reset(PFILE_OBJECT pFileObject);
	virtual NTSTATUS CAM_Memory_Read(PFILE_OBJECT pFileObject, IN WORD wAddress, OUT BYTE * pbyData );
	virtual NTSTATUS CAM_Memory_Write( PFILE_OBJECT pFileObject, IN WORD wAddress, OUT BYTE byData );
	virtual NTSTATUS CAM_IO_Read( PFILE_OBJECT pFileObject, IN WORD wAddress, OUT BYTE * pbyData );
	virtual NTSTATUS CAM_IO_Write( PFILE_OBJECT pFileObject, IN WORD wAddress, OUT BYTE byData );
	virtual BOOL	 CAM_Is_Bypasseded(PFILE_OBJECT pFileObject);
	virtual NTSTATUS CAM_SetBypassed( PFILE_OBJECT pFileObject, BOOL bBypassed );
	
	// IR
	virtual NTSTATUS IR_GetKey( PFILE_OBJECT pFileObject, DWORD * pdwIRKey );



	COneDataPort * GetDataPort( DWORD dwDataPortNo );

	void FreeAllPIDsOfDataPort( COneDataPort * pDataPort );
	void OnEthernetFrame( PBYTE pEthernetAddr, int nFrameLen );
	
	ITSDVBHardwareDrvInterface * volatile m_pAssociateNicDrv;	// ��ص�NIC�����ӿ�
	KMutex m_DevSyncObj;
	KSpinLock m_DataPortSyncObj;						// ר������ͬ�� DataPort
	int m_nAdaperNo;
	BOOL volatile m_bEnableDataReceive;

private:
	COneNICAdapter * GetAdapterObj( DWORD dwAdapterNo );

public:	
	TSDVBHWDRV_VERSION_INFO m_AdapterInfo;

private:	
	TSDVBIFDRVDevice * m_pDevObj;	
	COneDataPort * m_apDataPort[MAX_DATA_PORT_COUNT];
	PFILE_OBJECT m_pOwnerFileObj;	
	CPIDBitmaskMgr	m_FinalPIDBitMask;					// �ۺϵı�������
	TSDVB_REGISTER_MASTER_MODE	m_nRegisterMode;
	int m_nOpenDataPortMaxIndex;						// �򿪵����ݶ˿ڵ�������
	COneTunerObj	m_OneTunerObj;


	KEvent	m_NotifyProxyToDoTuneEvent;					//  CYJ,2008-11-11 ֪ͨ������Ҫ���е�̨������ֵ�ɴ���Ӧ�ó�������
	KEvent	m_ProxyHasDoneTuneEvent;					//  CYJ,2008-11-11 �����Ѿ���ɵ�̨
	TSDVB_TUNE_PROXY_DATA m_ProxyTuneParam;				//  CYJ,2008-11-11 �����õ�ת����̨
	NTSTATUS volatile m_nProxyTuneRetVal;				//  CYJ,2008-11-11 �����̨���
};

class CMySmartSpinLock
{
public:
	CMySmartSpinLock( KSpinLock & SyncObj )
		: m_SpinLock( SyncObj )
	{
		m_SpinLock.Lock();
		m_bLocked = true;
	}
	~CMySmartSpinLock()
	{
		if( m_bLocked )
			m_SpinLock.Unlock();
	}
	void Unlock()
	{
		m_SpinLock.Unlock();
		m_bLocked = false;
	}
private:
	KSpinLock & m_SpinLock;
	bool m_bLocked;
};

#endif // !defined(AFX_ONENICADAPTER_H__62B31C4D_8107_49A4_A17B_686C5AAE3949__INCLUDED_)
