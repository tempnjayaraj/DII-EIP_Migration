// IconStatusBar.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "IconStatusBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIconStatusBar dialog


CIconStatusBar::CIconStatusBar(CControl* pControl, int iTitle, CWnd* pParent /*=NULL*/)
	: CDialog(CIconStatusBar::IDD, pParent)
{
	m_pParent = pParent;
	m_pControl = pControl;
	m_bSpinImpeller = FALSE;

	m_bKillImpellerThread =  FALSE;
	m_hImpellerThread = NULL;

	// create brush for background color
	m_hbr = CreateSolidBrush(SN_BKGND_COLOR);

	//{{AFX_DATA_INIT(CIconStatusBar)
	//}}AFX_DATA_INIT
}

CIconStatusBar::~CIconStatusBar()
{
	KillImpellerThread();

	// Close Event Handles
	if( m_hImpellerThreadKilledEvent)
	{
		CloseHandle(m_hImpellerThreadKilledEvent);
		m_hImpellerThreadKilledEvent = NULL;
	}

	// Delete brush objects
	DeleteObject( m_hbr);
}

void CIconStatusBar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIconStatusBar)
	DDX_Control(pDX, IDC_STATIC_TITLE, m_StaticTitle);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CIconStatusBar, CDialog)
	//{{AFX_MSG_MAP(CIconStatusBar)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_UPDATE_STATUS, UpdateStatus)
	ON_MESSAGE(WM_UPDATE_REMOTE_STATUS, UpdateRemoteStatus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIconStatusBar message handlers

HBRUSH CIconStatusBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr;
	
	// Intercept the paint call to change the dialog control colors
	
	// Call the base class implementation first! Otherwise, it may
	// undo what we are trying to accomplish .
	hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// Set the Background color
	pDC->SetBkColor(SN_BKGND_COLOR);
	pDC->SetTextColor(WHITE);
	
	return m_hbr;
}

BOOL CIconStatusBar::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CSharedMemory mem;
	SnBool bStatus;

	// Create event
	m_hImpellerThreadKilledEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if(m_hImpellerThreadKilledEvent == NULL)
		return FALSE;
	
	// Initialize Shared memory if necessary
	bStatus = mem.GetInitStatus();
	if( !bStatus)
		mem.Init( m_pParent, m_pControl);

	// Setup the screen
	SetupBitmaps();
	SetupFonts();

	m_tPortAIcons.m_StaticPump.Create(NULL, WS_VISIBLE | SS_BITMAP,
                             CRect(74, 8, 74 + 52 - 1, 8 + 53 - 1), this);
	m_tPortAIcons.m_StaticHand.Create(NULL, WS_VISIBLE | SS_BITMAP,
                             CRect(128, 8, 128 + 52 - 1, 8 + 53 - 1), this);
	m_tPortAIcons.m_StaticFoot.Create(NULL, WS_VISIBLE | SS_BITMAP,
                             CRect(174, 8, 174 + 52 - 1, 8 + 53 - 1), this);

	m_tPortBIcons.m_StaticPump.Create(NULL, WS_VISIBLE | SS_BITMAP,
                             CRect(538, 8, 538 + 52 - 1, 8 + 53 - 1), this);
	m_tPortBIcons.m_StaticHand.Create(NULL, WS_VISIBLE | SS_BITMAP,
                             CRect(592, 8, 592 + 52 - 1, 8 + 53 - 1), this);
	m_tPortBIcons.m_StaticFoot.Create(NULL, WS_VISIBLE | SS_BITMAP,
                             CRect(644, 8, 644 + 52 - 1, 8 + 53 - 1), this);

	m_StaticIntellio.Create(NULL, WS_VISIBLE | SS_BITMAP,
                             CRect(748, 8, 748 + 52 - 1, 8 + 53 - 1), this);

    // Hide all icons until we know what devices are installed
	m_tPortAIcons.m_StaticPump.ShowWindow(SW_HIDE);
	m_tPortAIcons.m_StaticHand.ShowWindow(SW_HIDE);
	m_tPortAIcons.m_StaticFoot.ShowWindow(SW_HIDE);

	m_tPortBIcons.m_StaticPump.ShowWindow(SW_HIDE);
	m_tPortBIcons.m_StaticHand.ShowWindow(SW_HIDE);
	m_tPortBIcons.m_StaticFoot.ShowWindow(SW_HIDE);

	m_StaticIntellio.ShowWindow(SW_HIDE);
	
    // Get Port and Footswitch Information
    UpdateStatus(MSG_UPDATE_PORT_STATUS, 0);
	UpdateStatus(MSG_UPDATE_FOOT_STATUS, 0);

    // Get Pump and Intellio Shaver status
	if( m_pControl) 
	{
        CPump::PumpType	ePumpType;
	    SnBool bRunning = FALSE;

		// Check for Pump connection
		bRunning = m_pControl->GetPumpStatus( &ePumpType);
        if( ePumpType != CPump::PUMP_TYPE_UNKNOWN)
			UpdateRemoteStatus(MSG_UPDATE_REMOTE_PUMP_STATUS, MSG_REMOTE_CONNECTED);
        if( bRunning)
			UpdateRemoteStatus(MSG_UPDATE_REMOTE_PUMP_STATUS, MSG_PUMP_RUNNING);

        // Check for Intellio Shaver connection
        if( m_pControl->GetIntellioShaverStatus())
			UpdateRemoteStatus(MSG_UPDATE_REMOTE_INTELLIO_SHAVER_STATUS, MSG_REMOTE_CONNECTED);
    }

    this->ShowWindow(SW_SHOW);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CIconStatusBar::SetupBitmaps()
{
	// Load the Icon bitmaps
	m_SnHelp.LoadBitmap(&m_tPortAIcons.m_BitmapMduIcon, IDB_BITMAP_SHAVER_ICON_PORTA);
	m_SnHelp.LoadBitmap(&m_tPortAIcons.m_BitmapSawIcon, IDB_BITMAP_SAW_ICON_PORTA);
	m_SnHelp.LoadBitmap(&m_tPortAIcons.m_BitmapDrillIcon, IDB_BITMAP_DRILL_ICON_PORTA);
	m_SnHelp.LoadBitmap(&m_tPortAIcons.m_BitmapFootPedalIcon, IDB_BITMAP_FOOT_PEDAL_ICON_PORTA);

	m_SnHelp.LoadBitmap(&m_tPortBIcons.m_BitmapMduIcon, IDB_BITMAP_SHAVER_ICON_PORTB);
	m_SnHelp.LoadBitmap(&m_tPortBIcons.m_BitmapSawIcon, IDB_BITMAP_SAW_ICON_PORTB);
	m_SnHelp.LoadBitmap(&m_tPortBIcons.m_BitmapDrillIcon, IDB_BITMAP_DRILL_ICON_PORTB);
	m_SnHelp.LoadBitmap(&m_tPortBIcons.m_BitmapFootPedalIcon, IDB_BITMAP_FOOT_PEDAL_ICON_PORTB);

	m_SnHelp.LoadBitmap(&m_BitmapIntellioIcon, IDB_BITMAP_CONNECTED_BRIDGE_ICON);
	m_SnHelp.LoadBitmap(&m_BitmapImpellerGrayIcon, IDB_BITMAP_PUMP_GRAY_ICON);
	m_SnHelp.LoadBitmap(&m_BitmapImpellerBlueIcon, IDB_BITMAP_PUMP_BLUE_ICON);
	m_SnHelp.LoadBitmap(&m_BitmapImpellerRotate30Icon, IDB_BITMAP_PUMP_ROTATE_30);
	m_SnHelp.LoadBitmap(&m_BitmapImpellerRotate60Icon, IDB_BITMAP_PUMP_ROTATE_60);
} 

void CIconStatusBar::SetupFonts()
{
	CSharedMemory mem;

	SnWord usLanguage;

	// Get the current language
	if( m_pControl) 
	{
		// Get the saved language selection
		SnBool bStatus = m_pControl->GetCmdState(GET_SYSTEM_LANGUAGE, &usLanguage, sizeof(usLanguage));
		if(!bStatus)
			usLanguage = LANGUAGE_ENGLISH;
	}
	else
		usLanguage = LANGUAGE_ENGLISH;		
			
				
				
	if( usLanguage == LANGUAGE_ENGLISH || usLanguage == LANGUAGE_DANISH || usLanguage == LANGUAGE_DUTCH ||
		usLanguage == LANGUAGE_SWEDISH)
		m_StaticTitle.SetFont(mem.m_Font25Normal, TRUE);
	else
		m_StaticTitle.SetFont(mem.m_Font18Normal, TRUE);
}

LRESULT CIconStatusBar::UpdateRemoteStatus(WPARAM iParam, LPARAM lParam)
{

	switch( iParam)
	{
	
		case MSG_UPDATE_REMOTE_INTELLIO_SHAVER_STATUS:
			
			switch(lParam)
			{
				case MSG_REMOTE_DISCONNECTED:
					m_StaticIntellio.ShowWindow(SW_HIDE);
					break;

				case MSG_REMOTE_CONNECTED:
					m_StaticIntellio.SetBitmap(m_BitmapIntellioIcon);
					m_StaticIntellio.ShowWindow(SW_SHOW);
					break;

				default:
					break;

			}
			
			break;
			

		case MSG_UPDATE_REMOTE_PUMP_STATUS:
		
			SnWord usShaverPcktCtl;
			CStatic* pPumpPortBitmap;
			CStatic* pOtherPortBitmap;
			SnBool bSpinImpeller;

			if( m_pControl)
				m_pControl->GetCmdState(GET_SHAVER_PACKET_CTL, &usShaverPcktCtl, sizeof(usShaverPcktCtl));	

			if( usShaverPcktCtl == PORTA)
			{
				pPumpPortBitmap  = &m_tPortAIcons.m_StaticPump;
				pOtherPortBitmap = &m_tPortBIcons.m_StaticPump;
			}
			else if( usShaverPcktCtl == PORTB)
			{
				pPumpPortBitmap  = &m_tPortBIcons.m_StaticPump;
				pOtherPortBitmap = &m_tPortAIcons.m_StaticPump;
			}
			else
			{
				break;
			}
			
			switch(lParam)
			{
				case MSG_REMOTE_DISCONNECTED:
					pPumpPortBitmap->ShowWindow(SW_HIDE);
					// Kill impeller thread
					KillImpellerThread();
					break;

				case MSG_REMOTE_CONNECTED:
					pPumpPortBitmap->SetBitmap(m_BitmapImpellerGrayIcon);
					pOtherPortBitmap->SetBitmap(m_BitmapImpellerGrayIcon);
				
					pPumpPortBitmap->ShowWindow(SW_SHOW);
					m_bSpinImpeller = FALSE;
					CreateImpellerThread();
					break;

				case MSG_PUMP_RUNNING:
					// Spin the impeller
					pPumpPortBitmap->SetBitmap(m_BitmapImpellerBlueIcon);
					m_bSpinImpeller = TRUE;
					break;

				case MSG_PUMP_NOT_RUNNING:
					pPumpPortBitmap->SetBitmap(m_BitmapImpellerGrayIcon);
					pPumpPortBitmap->ShowWindow(SW_SHOW);
					m_bSpinImpeller = FALSE;
					break;


				case MSG_PORT_MAPPING_STATUS:
					
					bSpinImpeller = m_bSpinImpeller;
					if(bSpinImpeller)
					{
						// Turn the animation off
						m_bSpinImpeller = FALSE;
						Sleep(100); // wait 
					}
					
					pPumpPortBitmap->ShowWindow(SW_SHOW);
					pOtherPortBitmap->ShowWindow(SW_HIDE);

					if(bSpinImpeller)
					{
						// Turn the animation back on
						m_bSpinImpeller = TRUE;
					}
					break;
				
				default:
					break;

			}
	

			break;
		
		default:
			break;
	
	}

	return 0L;
}

LRESULT CIconStatusBar::UpdateStatus(WPARAM iParam, LPARAM lParam)
{
	CSharedMemory mem;
	SnWord wTitle = 0;
	SN_PORT_STATUS tPortAStatus = { 0 };
	SN_PORT_STATUS tPortBStatus = { 0 };
	SN_FOOT_STATUS tFootStatus  = { 0 };
	SnWord wFootType = 0;
	
	if( m_pControl)
	{
		// Get Port status
		m_pControl->GetCmdState(GET_MC_PORTA_STATUS, &tPortAStatus, sizeof(SN_PORT_STATUS));
		m_pControl->GetCmdState(GET_MC_PORTB_STATUS, &tPortBStatus, sizeof(SN_PORT_STATUS));
		m_pControl->GetCmdState(GET_MC_FOOT_STATUS,  &tFootStatus,  sizeof(SN_FOOT_STATUS));
	}
	
	switch( iParam)
	{
		
	case MSG_UPDATE_TITLE:
		SetupFonts();
		if (lParam)
		{
			// Display the text on the Title Bar
			SetDlgItemText( IDC_STATIC_TITLE, m_SnHelp.GetString(lParam));
			m_StaticTitle.ShowWindow(SW_SHOW);
		} 
		else
		{
			m_StaticTitle.ShowWindow(SW_HIDE);
		}
		break;
		
		
	case MSG_UPDATE_PORT_STATUS:
		UpdatePortStatus(tPortAStatus.usType, &m_tPortAIcons);
		UpdatePortStatus(tPortBStatus.usType, &m_tPortBIcons);
		break;
		
		
	case MSG_UPDATE_FOOT_STATUS:
		
		// Clear Foot status icons and update Intellio packet
		if( tFootStatus.usFootAssignedPort == PORTA )
		{
			// Assign foot pedal Icon to Port A
			m_tPortAIcons.m_StaticFoot.SetBitmap(m_tPortAIcons.m_BitmapFootPedalIcon);
			m_tPortAIcons.m_StaticFoot.ShowWindow(SW_SHOW);
			m_tPortBIcons.m_StaticFoot.ShowWindow(SW_HIDE);
		}
		else if( tFootStatus.usFootAssignedPort == PORTB )
		{
			// Assign foot pedal Icon to Port B
			m_tPortBIcons.m_StaticFoot.SetBitmap(m_tPortBIcons.m_BitmapFootPedalIcon);
			m_tPortBIcons.m_StaticFoot.ShowWindow(SW_SHOW);
			m_tPortAIcons.m_StaticFoot.ShowWindow(SW_HIDE);
		}
		else
		{
			// Hide Foot Pedal icons
			m_tPortAIcons.m_StaticFoot.ShowWindow(SW_HIDE);
			m_tPortBIcons.m_StaticFoot.ShowWindow(SW_HIDE);
		}
		break;
		
	default:
		break;
			
	}// end switch
	
    this->UpdateWindow();
	
	return 0L;
}


void CIconStatusBar::UpdatePortStatus(SnWord usType, SN_PORT_ICONS* ptPortIcons)
{
	switch (usType)
	{
		
	case TYPE_MDU_UTLRA_IUR:
	case TYPE_MDU_STANDARD:
	case TYPE_MDU_STANDARD_CTL:
	case TYPE_MDU_MINI:
	case TYPE_MDU_RELIANT:
	case TYPE_MDU_RELIANT_CTL:
	case TYPE_MDU_RELIANT_BF:
	case TYPE_MDU_POWERMINI:
	case TYPE_MDU_POWERMINI_CTL:
	case TYPE_MDU_POWERMINI_BF:
		ptPortIcons->m_StaticHand.SetBitmap(ptPortIcons->m_BitmapMduIcon);
		ptPortIcons->m_StaticHand.ShowWindow(SW_SHOW);
		break;
		
	case TYPE_DP_DRILL:
		ptPortIcons->m_StaticHand.SetBitmap(ptPortIcons->m_BitmapDrillIcon);
		ptPortIcons->m_StaticHand.ShowWindow(SW_SHOW);
		break;
		
	case TYPE_DP_SAW:
		ptPortIcons->m_StaticHand.SetBitmap(ptPortIcons->m_BitmapSawIcon);
		ptPortIcons->m_StaticHand.ShowWindow(SW_SHOW);
		break;
		
	default:
		ptPortIcons->m_StaticHand.ShowWindow(SW_HIDE);
		break;
		
	} // End switch
}

void CIconStatusBar::CreateImpellerThread()
{
	// Create impeller thread
	if( !m_hImpellerThread)
	{
		m_bKillImpellerThread = FALSE;
		
		m_hImpellerThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
											0,
											ImpellerThread,
											this,
											0,
											&m_hImpellerThreadID);
				
		if (m_hImpellerThread == NULL)
		{
			//System Resourse Failure 
			::PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)SYSTEM_RESOURCE_ERROR, (LPARAM)0);
		
		}
		DEBUGMSG(TRUE, (TEXT("CIconStatusBar ImpellerThread: 0x%08X\n"),m_hImpellerThreadID));

	}
}

void CIconStatusBar::KillImpellerThread()
{

	if( m_hImpellerThread)
	{

		DWORD waitStatus;
		DWORD exitCode;
		
		m_bKillImpellerThread = TRUE;

		// wait for thread to signal its been terminated
		if( m_hImpellerThreadKilledEvent)
			waitStatus = WaitForSingleObject( m_hImpellerThreadKilledEvent,THREAD_TERMINATION_WAIT );

		if( waitStatus == WAIT_TIMEOUT) 
		{
			GetExitCodeThread( m_hImpellerThread, &exitCode);
		
			TerminateThread( m_hImpellerThread, exitCode);
	
		}
	
		CloseHandle( m_hImpellerThread);
		m_hImpellerThread = NULL;
		m_bKillImpellerThread = FALSE;
	}
}

DWORD WINAPI ImpellerThread(LPVOID pParam)
{
	CIconStatusBar *pClass = (CIconStatusBar*)pParam;


	while(!pClass->m_bKillImpellerThread)
	{
				
		// Rotate bitmap
		if( !pClass->m_bKillImpellerThread && pClass->m_bSpinImpeller)
			pClass->SpinImpeller();

		else if( !pClass->m_bKillImpellerThread && !pClass->m_bSpinImpeller)
			Sleep(50);

	}

	// Set the Thread Killed Event
	SetEvent( pClass->m_hImpellerThreadKilledEvent);
	
	return 0;
}

void CIconStatusBar::DrawImpeller(int iX, CBitmap *pBitmap)
{
    CBitmap *pOldBitmap;
    EnterCriticalSection(&CSnHelp::m_DisplayCs);
    pOldBitmap = CSnHelp::m_hMemoryCDC.SelectObject(pBitmap);
    CSnHelp::m_hDisplayCDC->BitBlt(iX, 8, 52, 53,
        &CSnHelp::m_hMemoryCDC, 0, 0, SRCCOPY);
    CSnHelp::m_hMemoryCDC.SelectObject(pOldBitmap);
	LeaveCriticalSection(&CSnHelp::m_DisplayCs);
}

void CIconStatusBar::SpinImpeller()
{

 	SnWord usShaverPcktCtl;
	RECT tWindow;

	if( m_pControl)
		m_pControl->GetCmdState(GET_SHAVER_PACKET_CTL, &usShaverPcktCtl, sizeof(usShaverPcktCtl));	
	if( usShaverPcktCtl == PORTA)
		m_tPortAIcons.m_StaticPump.GetWindowRect(&tWindow);
	else
		m_tPortBIcons.m_StaticPump.GetWindowRect(&tWindow);
	
	if(!m_bKillImpellerThread  && m_bSpinImpeller)
        DrawImpeller(tWindow.left, &m_BitmapImpellerBlueIcon);
	if(!m_bKillImpellerThread  && m_bSpinImpeller)
		Sleep(40);
	if(!m_bKillImpellerThread  && m_bSpinImpeller)
        DrawImpeller(tWindow.left, &m_BitmapImpellerRotate30Icon);
	if(!m_bKillImpellerThread  && m_bSpinImpeller)
		Sleep(40);
	if(!m_bKillImpellerThread  && m_bSpinImpeller)
        DrawImpeller(tWindow.left, &m_BitmapImpellerRotate60Icon);
	if(!m_bKillImpellerThread  && m_bSpinImpeller)
		Sleep(40);
}
