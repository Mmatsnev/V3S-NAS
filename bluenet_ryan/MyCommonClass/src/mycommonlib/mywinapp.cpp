///=======================================================
///
///     ���ߣ�������
///     ����ͨ��
///     ���ڣ�2005.3.9
///
///	    ��;��
///         My Windows application base
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	���ļ������ڡ�ͨ�ӡ��ڲ�ʹ��!	  			     !
///!													 !
///!	�Ҳ���֤���ļ��İٷ�֮����ȷ!					 !
///!	��Ҫʹ�ø��ļ�������Ҫ�е��ⷽ��ķ���!			 !
///=======================================================

#include "stdafx.h"
#include "mywinapp.h"
#include <MyRingPool.h>
#include <sys/time.h>
#include <MyList.h>
#include <stdio.h>

typedef struct tagMYMSG
{
    CMyCommandResponser *   m_pResponser;   // responser, just like hWnd
    int     m_nMessage;     // Message ID, such as WM_KEYDOWN
    int     m_wParam;       // parameter 1
    int     m_lParam;       // parameter 2
}MYMSG,*PMYMSG;

//////////////////////////////////////////////////////////
// �����Ի���1024����Ϣ
// ֻ���ڴ��� PostMessage ���ã�����SendMessage����ֱ�ӵ�����ص� DefWindowProc
#define MAX_MSG_BUF_COUNT   1024
static CMyRingPool<MYMSG>   s_MsgRingBuf;
static CMyCommandResponser * s_ActiveResponser = NULL;

//--------------------------------------------------------------
// 
CMyCommandResponser	* GetActiveResponser()
{
	return s_ActiveResponser;
}

//////////////////////////////////////////////////////////
// CTimerMgr
class CTimerMgr
{
public:
    CTimerMgr();
    ~CTimerMgr();

    int SetTimer( CMyCommandResponser * pResponser, int nIDEvent, int nElapse );
    void KillTimer( CMyCommandResponser * pResponser, int nIDEvent );
    void DoSchedule();

    struct tagTIMER_STRUCT
    {
        CMyCommandResponser * m_pResponser;
        int m_nEventID;
        int m_nElapse;
#ifdef _WIN32
        LONGLONG m_llExpireTick;
#else
        long long m_llExpireTick;
#endif //_WIN32
    };

private:
    CMyList<tagTIMER_STRUCT> m_listTimers;
private:
    int SetTimer( tagTIMER_STRUCT & ItemData );
#ifdef _DEBUG
    void CheckExpireTickOrder();
#endif //_DEBUG
};

#ifndef _WIN32
static DWORD s_dwStartTicks = 0;

extern "C" DWORD WINAPI GetTickCount()
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return ((t.tv_sec * 1000) + (t.tv_usec / 25000) * 25) - s_dwStartTicks;
}
#endif //_WIN32	
	
CTimerMgr::CTimerMgr()
{    
	s_dwStartTicks = 0;
	s_dwStartTicks = GetTickCount();
}

CTimerMgr::~CTimerMgr()
{
}

int CTimerMgr::SetTimer( CMyCommandResponser * pResponser, int nIDEvent, int nElapse )
{
    tagTIMER_STRUCT OneItem;
    OneItem.m_pResponser = pResponser;
    OneItem.m_nEventID = nIDEvent;
    OneItem.m_nElapse = nElapse;

    return SetTimer( OneItem );
}

int CTimerMgr::SetTimer( tagTIMER_STRUCT & ItemData )
{
    if( ItemData.m_pResponser == NULL || ItemData.m_nElapse <= 0 || ItemData.m_nEventID <= 0 )
    {
        ASSERT( 0 );
        return 0;
    }

    KillTimer( ItemData.m_pResponser, ItemData.m_nEventID );

    ItemData.m_llExpireTick = GetTickCount();
    ItemData.m_llExpireTick += ItemData.m_nElapse;

    if( m_listTimers.IsEmpty() )
    {			// empty list, insert directly
        m_listTimers.AddTail( ItemData );
        return ItemData.m_nEventID;
    }

    POSITION pos = m_listTimers.GetHeadPosition();
    while( pos )
    {
        POSITION posOld = pos;
        tagTIMER_STRUCT & ExistItem = m_listTimers.GetNext( pos );
        if( ExistItem.m_llExpireTick <= ItemData.m_llExpireTick )
            continue;			// early expire
        m_listTimers.InsertBefore( posOld, ItemData );

#ifdef _DEBUG
        CheckExpireTickOrder();
#endif //_DEBUG

        return ItemData.m_nEventID;
    }

    // all item's expire tick is less the mine, so add to the tail
    m_listTimers.AddTail( ItemData );

#ifdef _DEBUG
    CheckExpireTickOrder();
#endif //_DEBUG

    return ItemData.m_nEventID;
}

///--------------------------------------------------------------------------
/// CYJ, 2005.3.28
/// Function:
///	Kill Timer
/// Input Parameter:
///	pResponser			responser
///	nIDEvent				event, -1 kill all timer associatived to the pResponser
/// Output:
///	None
void CTimerMgr::KillTimer( CMyCommandResponser * pResponser, int nIDEvent )
{
    if( m_listTimers.IsEmpty() )
        return;
    POSITION pos = m_listTimers.GetHeadPosition();
    while( pos )
    {
        POSITION posOld = pos;
        tagTIMER_STRUCT & OneItem = m_listTimers.GetNext( pos );
        if( OneItem.m_pResponser == pResponser 
			&& ( OneItem.m_nEventID == nIDEvent || -1 == nIDEvent ) )
        {
            m_listTimers.RemoveAt( posOld );
            return;
        }
    }
}

void CTimerMgr::DoSchedule()
{
#ifdef _WIN32
    LONGLONG llCurTick = GetTickCount();
#else
    long long llCurTick = GetTickCount();
#endif //_Win32

    while( false == m_listTimers.IsEmpty() )
    {
        tagTIMER_STRUCT & OneItem = m_listTimers.GetHead();
        if( llCurTick < OneItem.m_llExpireTick )
            return;			// no more event
        // call the event handle
#ifdef _DEBUG
        printf("%p time out, id=%d, %08lu, next expire=%08lu.\n",
            OneItem.m_pResponser, OneItem.m_nEventID, DWORD(OneItem.m_llExpireTick),
            DWORD(llCurTick+OneItem.m_nElapse) );
#endif //_DEBUG

		tagTIMER_STRUCT ItemData = m_listTimers.RemoveHead();
        SetTimer( ItemData );		// �ٴ�����

		//	�ڵ��� OnTimer ʱ����Timer���ܱ�ɾ����������Ҫ�ڵ���֮ǰ����Timer
        ASSERT( OneItem.m_pResponser );
        OneItem.m_pResponser->DefWindowProc( WM_TIMER, OneItem.m_nEventID, 0 );        
    }
}

#ifdef _DEBUG
void CTimerMgr::CheckExpireTickOrder()
{
#ifdef _WIN32
    LONGLONG llCurTick = 0;
#else
    long long llCurTick = 0;
#endif //_Win32

    if( m_listTimers.IsEmpty() )
        return;
    POSITION pos = m_listTimers.GetHeadPosition();
    while( pos )
    {
        tagTIMER_STRUCT & OneItem = m_listTimers.GetNext( pos );
        ASSERT( llCurTick <= OneItem.m_llExpireTick );
        llCurTick = OneItem.m_llExpireTick;
    }
}

#endif //_DEBUG

///-------------------------------------------------------
static CTimerMgr   s_TimerMgr;

//////////////////////////////////////////////////////////
/// CYJ,2005.3.9
/// ��������:
///		�ύ�ӳٴ������Ϣ
/// �������:
///		pWnd            ����Ķ���
///     nMessage        ��Ϣ
///     wParam          ���� 1
///     lParam          ���� 2
/// ���ز���:
///		true            �ύ�ɹ�
///     false           ʧ��
bool PostMessage( CMyCommandResponser * pWnd, int nMessage, int wParam, int lParam )
{
#ifdef _DEBUG
    ASSERT( pWnd );
#endif //_DEBUG

    if( NULL == pWnd )
        return false;
    MYMSG * pMsg = s_MsgRingBuf.GetNextWriteUnit();
    if( NULL == pMsg )
        return false;
    pMsg->m_pResponser = pWnd;
    pMsg->m_nMessage = nMessage;
    pMsg->m_wParam = wParam;
    pMsg->m_lParam = lParam;

    s_MsgRingBuf.IncreaseWritePtr();

    return true;
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///		�ύ�ӳٴ������Ϣ
/// �������:
///		pWnd            ����Ķ���
///     nMessage        ��Ϣ
///     wParam          ���� 1
///     lParam          ���� 2
/// ���ز���:
///     ��Ϣ����������ֵ
long SendMessage( CMyCommandResponser * pWnd, int nMessage, int wParam, int lParam )
{
    ASSERT( pWnd );
    if( NULL == pWnd )
        return -1;
    return pWnd->DefWindowProc( nMessage, wParam, lParam );
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///		���캯��
/// �������:
///		x,y         ��ʼ���꣬ȱʡΪ0
///     cx,cy       ��ȡ��߶ȣ�ȱʡΪ0���Զ���ȡ
/// ���ز���:
///		��
CMyCommandResponser::CMyCommandResponser(int x, int y, int cx, int cy)
{
	m_pParent = s_ActiveResponser;		// ���浱ǰ��Ч��
	s_ActiveResponser = this;
}

CMyCommandResponser::~CMyCommandResponser()
{
	s_ActiveResponser = m_pParent;
	KillTimer( -1 );		// kill all timer associated with this responser
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///		һ��������
/// �������:
///		vtChar                 Virtual Key value
///   		nFlags              	Bit0-Bit14			1
///					Bit15-Bit23			Scan Code
///					Bit24=>SHIFT,		1 => Pressed
///					Bit25=>ALT, 		1 => Pressed
///					Bit26=>CONTROL status	1 ==> Pressed
///				  	Bit27 => Caps lock, 	1 => Locked
///					Bit28=> Num Lock	1 => Loced
///					Bit29-31			reserved
/// ���ز���:
///		��
/// ˵����
///     ����ASCII�ַ�����������ŵ��� OnChar ����
void CMyCommandResponser::OnKeyDown( WORD vtChar, int nFlags )
{
	if( m_pParent )
		m_pParent->OnKeyDown( vtChar, nFlags );
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///		����һ���ַ�
/// �������:
//////		vtChar                 Virtual Key value
///   		nFlags              	Bit0-Bit14			1
///					Bit15-Bit23			Scan Code
///					Bit24=>SHIFT,		1 => Pressed
///					Bit25=>ALT, 		1 => Pressed
///					Bit26=>CONTROL status	1 ==> Pressed
///				  	Bit27 => Caps lock, 	1 => Locked
///					Bit28=> Num Lock	1 => Loced
///					Bit29-31			reserved
/// ���ز���:
///		��
/// ˵����
///     �ڴ�֮ǰ��OnKeyDownһ���ᱻ����
void CMyCommandResponser::OnChar( WORD vtChar, int nFlags )
{
	if( m_pParent )
		m_pParent->OnChar( vtChar, nFlags );
}

//-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///		һ��������
/// �������:
///		vtChar                 Virtual Key value
///   		nFlags              	Bit0-Bit14			1
///					Bit15-Bit23			Scan Code
///					Bit24=>SHIFT,		1 => Pressed
///					Bit25=>ALT, 		1 => Pressed
///					Bit26=>CONTROL status	1 ==> Pressed
///				  	Bit27 => Caps lock, 	1 => Locked
///					Bit28=> Num Lock	1 => Loced
///					Bit29-31			reserved
/// ���ز���:
///		��
void CMyCommandResponser::OnKeyUp( WORD vtChar, int nFlags )
{
	if( m_pParent )
		m_pParent->OnKeyUp( vtChar, nFlags );
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///		�����걻���º��ɿ�
/// �������:
///		x, y        �������
///     nFlags      shift, control, alt ��״̬
/// ���ز���:
///		��
void CMyCommandResponser::OnLButtonUp( int x,int y, int nFlags )
{	
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///		�����걻���º��ɿ�
/// �������:
///		x, y        �������
///     nFlags      shift, control, alt ��״̬
/// ���ز���:
///		��
void CMyCommandResponser::OnLButtonDown( int x,int y, int nFlags )
{
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:///		������˫
// �������:
///		x, y        �������
///     nFlags      shift, control, alt ��״̬
/// ���ز���:
///		��
void CMyCommandResponser::OnLButtonDbClk( int x, int y, int nFlags )
{
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///		�����걻���º��ɿ�
/// �������:
///		x, y        �������
///     nFlags      shift, control, alt ��״̬
/// ���ز���:
///		��
void CMyCommandResponser::OnRButtonUp( int x,int y, int nFlags )
{
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///		�����걻���º��ɿ�
/// �������:
///		x, y        �������
///     nFlags      shift, control, alt ��״̬
/// ���ز���:
///		��
void CMyCommandResponser::OnRButtonDown( int x,int y, int nFlags )
{
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:///		������˫
// �������:
///		x, y        �������
///     nFlags      shift, control, alt ��״̬
/// ���ز���:
///		��
void CMyCommandResponser::OnRButtonDbClk( int x, int y, int nFlags )
{
}

/// CYJ,2005.3.9
/// ��������:///		������˫
// �������:
///		x, y        �������
///     nFlags      shift, control, alt ��״̬
/// ���ز���:
///		��
void CMyCommandResponser::OnMouseMove( int x, int y, int nFlags )
{
}

///---------------------------------------------------------------------------
/// CYJ,	2005.3.28
/// Function:
///		On Front key pressed
/// Input Parameter:
///		byKeyMask		key bit mask, bit0~bit6 	for combination of the front key, and bit7 = 1 mean this key is repeated
///		byOldKeyMask		old key bit mask
/// Output Paramere:
///		None
void CMyCommandResponser::OnFrontKeyPressed( BYTE byKeyMask, BYTE byOldKeyMask )
{
	if( m_pParent )
		m_pParent->OnFrontKeyPressed( byKeyMask, byOldKeyMask );
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///		��ʱ��
/// �������:
///		nEventID        ʱ�����ͣ�ͬSetTimer �ķ���ֵ
/// ���ز���:
///		��
void CMyCommandResponser::OnTimer( int nEventID )
{
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///		���ö�ʱ��
/// �������:
///		nEventID       ��ʱ���¼�
///     nElapse        ��ʱ���ڣ���λ:����
/// ���ز���:
///		>=0            �ɹ�
///     <0             ʧ��
int CMyCommandResponser::SetTimer( int nEventID, int nElapse )
{
    ASSERT( nElapse > 0 );
    return s_TimerMgr.SetTimer( this, nEventID, nElapse );
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///		ɾ��һ����ʱ��
/// �������:
///		nEventID        ��ʱ��ʱ�䣬��SetTimer����
/// ���ز���:
///		��
void CMyCommandResponser::KillTimer( int nEventID )
{
    s_TimerMgr.KillTimer( this, nEventID );
}

///-------------------------------------------------------
/// CYJ,2005.3.10
/// ��������:
///	ȱʡ�Ĵ�����Ϣ������
/// �������:
///	��
/// ���ز���:
///	��
long CMyCommandResponser::DefWindowProc( int nMessage, int wParam, int lParam )
{
    switch( nMessage )
    {
    case WM_TIMER:
        OnTimer( wParam );
        break;
    case WM_KEYDOWN:
        OnKeyDown( (WORD)wParam, lParam );
        break;
    case WM_CHAR:
        OnChar( char(wParam), lParam );
        break;
    }
    return 0;
}

///-------------------------------------------------------
/// CYJ,2005.3.10
/// ��������:
///	�ݽ���Ϣ
/// �������:
///	��
/// ���ز���:
///	��
bool CMyCommandResponser::PostMessage( int nMessage, int wParam, int lParam )
{
    return ::PostMessage( this, nMessage, wParam, lParam );
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///	    ������Ϣ
/// �������:
///	    nMessage            ��Ϣ
///     wParam              ���� 1
///     lParam              ���� 2
/// ���ز���:
///	    ����ֵ
long CMyCommandResponser::SendMessage( int nMessage, int wParam, int lParam )
{
    return DefWindowProc( nMessage, wParam, lParam );
}


///////////////////////////////////////////////////////////////
/// CMyWinApp
CMyWinApp::CMyWinApp()
{
}

CMyWinApp::~CMyWinApp()
{
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///		��ʼ��ʵ��
/// �������:
///		��
/// ���ز���:
///		true            ����ִ��
///     false           �˳�ִ�г���
bool CMyWinApp::InitInstance()
{	
    return true;
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///		�˳�ǰ��֪ͨ�������ͷ���Դ
/// �������:
///		��
/// ���ز���:
///		����ķ���ֵ
int CMyWinApp::ExitInstance()
{
    return 0;
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///		������������壬��Ҫ�����¼����ȡ�һ������²���Ҫ�ӹܸú���
/// �������:
///		��
/// ���ز���:
///		��
void CMyWinApp::Run()
{
    while( DispatchMessage() )
    {
        s_TimerMgr.DoSchedule();        // ���ȶ�ʱ��
        CheckKeyboardMouseInput();      // �����̡��������

        OnIdle();
    }
}

void CMyWinApp::PumpOneMessage()
{
   	if( false == DispatchMessage() )
		::PostMessage( NULL, WM_QUIT, 0, 0 );	// ��ȡ���˳������˻ظ���Ϣ
		
	s_TimerMgr.DoSchedule();        // ���ȶ�ʱ��
	CheckKeyboardMouseInput();      // �����̡��������
}

///-------------------------------------------------------
/// CYJ,2005.3.11
/// ��������:
///	�ɷ���Ϣ
/// �������:
///	��
/// ���ز���:
///	true            ��������
///     false           ���յ� WM_QUIT ��Ϣ��Ӧ���˳�����
bool CMyWinApp::DispatchMessage()
{
    PMYMSG pMsg;
    while( (pMsg = s_MsgRingBuf.GetCurrentReadUnit() ) )
    {
        if( pMsg->m_nMessage == WM_QUIT )
            return false;           // �����˳�
        if( pMsg->m_pResponser )
            pMsg->m_pResponser->DefWindowProc( pMsg->m_nMessage, pMsg->m_wParam, pMsg->m_lParam );

        s_MsgRingBuf.Skip( 1 );     // ����Ϣ�Ѿ�����
    }

    return true;
}

///-------------------------------------------------------
/// CYJ,
/// ��������:
///		����ʱ����
/// �������:
///		��
/// ���ز���:
///		��
void CMyWinApp::OnIdle()
{
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///		����ѭ����һ���� main ����������
/// �������:
///		��
/// ���ز���:
///		��
int CMyWinApp::DoRun()
{
    if( false == PrepareEnv() )
        return -1;
    if( InitInstance() )
    	Run();
    return ExitInstance();
}


///-------------------------------------------------------
/// CYJ,2005.3.11
/// ��������:
///	׼������
/// �������:
///	��
/// ���ز���:
///	��
bool CMyWinApp::PrepareEnv()
{
	s_ActiveResponser = NULL;
    if( false == s_MsgRingBuf.Initialize(MAX_MSG_BUF_COUNT) )
        return false;

    return true;
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// ��������:
///		��ѯ���̡���ꡢ��ʱ���¼�
/// �������:
///		��
/// ���ز���:
///		true               exist the application
///     false
void CMyWinApp::CheckKeyboardMouseInput()
{
}

///-------------------------------------------------------
/// CYJ,2005.3.28
/// ��������:
///		���������Ϣ
/// �������:
// 		dwFlags			Bit0 Left button down, 
//						Bit1 Right button
//						Bit2 Shift is Pressed, 
//						Bit3 Contrl is pressed
//						Bit4 Mid button down
//						��16����Ϊ�ϴε�״̬
//		dwPositon			��16����Ϊx���꣬��16����Ϊy����
/// ���ز���:
///		None
void CMyWinApp::HandleMouseEvent( DWORD dwFlags, DWORD dwPositon )
{
	if( NULL == s_ActiveResponser )
		return;	
	DWORD dwNow = GetTickCount();
	
	DWORD dwLastStatus = dwFlags >> 16;
	dwLastStatus ^= dwFlags;
	
#define DOUBLE_CLICK_MS		500

	if( (dwLastStatus & 3) == 0 )
		s_ActiveResponser->OnMouseMove( dwPositon>>16,dwPositon&0xFFFF,dwFlags&0xFFFF );
	else
	{
		if( dwLastStatus & 1 )
		{				// Left button action
			static DWORD dwLastLeftButtonAction = 0;
			if( dwFlags & 1 )
			{			// Left Button down
				if( DWORD(dwNow - dwLastLeftButtonAction) < DOUBLE_CLICK_MS )
					s_ActiveResponser->OnLButtonDbClk(dwPositon>>16,dwPositon&0xFFFF,dwFlags&0xFFFF );
				else
					s_ActiveResponser->OnLButtonDown(dwPositon>>16,dwPositon&0xFFFF,dwFlags&0xFFFF );
			}
			else
			{			// Left button Up
				s_ActiveResponser->OnLButtonUp(dwPositon>>16,dwPositon&0xFFFF,dwFlags&0xFFFF );
			}	
			dwLastLeftButtonAction = dwNow;
		}
		if( dwLastStatus & 2 )
		{				// right button action
			static DWORD dwLastRightButtonAction = 0;
			if( dwFlags & 2 )
			{			// Right Button down
				if( DWORD(dwNow - dwLastRightButtonAction) < DOUBLE_CLICK_MS )
					s_ActiveResponser->OnRButtonDbClk(dwPositon>>16,dwPositon&0xFFFF,dwFlags&0xFFFF );
				else
					s_ActiveResponser->OnRButtonDown(dwPositon>>16,dwPositon&0xFFFF,dwFlags&0xFFFF );
			}
			else
			{			// Right button Up
				s_ActiveResponser->OnRButtonUp(dwPositon>>16,dwPositon&0xFFFF,dwFlags&0xFFFF );
			}	
			dwLastRightButtonAction = dwNow;
		}		
	}
}
