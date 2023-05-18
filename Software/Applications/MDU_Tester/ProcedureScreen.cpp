// ProcedureScreen.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "ProcedureScreen.h"
#include "MessageBox.h"
#include "LanguagePopUp.h"
#include "HandpieceCountPopUpPortA.h"
#include "HandpieceCountPopUpPortB.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//void RedrawHandpieceCountPopUpPortB(void);

/////////////////////////////////////////////////////////////////////////////
// CProcedureScreen dialog
SnSQByte             CProcedureScreen::m_sqCreationCount = 0;

CBitmap             CProcedureScreen::m_WindowLock;
CBitmap             CProcedureScreen::m_WindowLockPressed;
CBitmap             CProcedureScreen::m_ArrowUp;
CBitmap             CProcedureScreen::m_ArrowUpPressed;
CBitmap             CProcedureScreen::m_ArrowDown;
CBitmap             CProcedureScreen::m_ArrowDownPressed;
CBitmap             CProcedureScreen::m_ButtonHandpieceCountPortA;
CBitmap             CProcedureScreen::m_ButtonHandpieceCountPortB;
CBitmap	            CProcedureScreen::m_ArrowLeftGreen;
CBitmap             CProcedureScreen::m_ArrowLeftWhite;
CBitmap             CProcedureScreen::m_ArrowRightGreen;
CBitmap             CProcedureScreen::m_ArrowRightWhite;
CBitmap             CProcedureScreen::m_BitmapTestDot;
CBitmap             CProcedureScreen::m_BitmapTestDotBlue;
CBitmap             CProcedureScreen::m_BitmapTestDotOrange;
CBitmap             CProcedureScreen::m_BitmapTestDotYellow;

CBitmap             CProcedureScreen::m_hIdleSetSpeedDigit[10];
CBitmap             CProcedureScreen::m_hRunSetSpeedDigit[10];

CBrush             CProcedureScreen::m_hBrGreen;
CBrush             CProcedureScreen::m_hBrBlack;
CBrush             CProcedureScreen::m_hBrYellow;

CProcedureScreen::CProcedureScreen(CControl* pControl, CWnd* pParent /*=NULL*/)
	: CDialog(CProcedureScreen::IDD, pParent),
	m_pParent(pParent),
	m_pControl(pControl),
	m_PortA(pControl,NULL, PORTA),
	m_PortB(pControl,NULL, PORTB)
{
	m_PortA.SetParent(this);
	m_PortB.SetParent(this);

	m_dwBeeperCount = 0;

	// Button Scrolling members
	m_bWaitToScroll = TRUE;
	m_bRemoteKeyDown = FALSE;
	m_bReDraw = TRUE;

    m_bFatalError = FALSE;


    m_bKillThreads = FALSE;
	m_uiScrollingTimer = 0;
	m_uiRemoteTimer = 0;
	m_hButtonScrollThread = NULL;		
	m_hButtonScrollThreadKilledEvent = NULL; 

	if (m_sqCreationCount ==0)
	{
		// Create brushes 
		m_hBrBlack.CreateSolidBrush(SN_BKGND_COLOR);
		m_hBrGreen.CreateSolidBrush(SN_GREEN);
		m_hBrYellow.CreateSolidBrush(SN_YELLOW);
	}
	m_sqCreationCount++;

	//{{AFX_DATA_INIT(CProcedureScreen)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CProcedureScreen::~CProcedureScreen()
{
	DeInit();

}
void CProcedureScreen::DeInit()
{
    int iDigit;

	SetDrawRect(FALSE);
	m_bReDraw = FALSE;
	
	m_bKillThreads = TRUE;

	// Kill the timer if it's running
	StopScrollingTimer();

	// Kill the timer if it's running
	StopRemoteTimer();

	// Terminate running thread
	KillThreads();

	// Close Event Handles
	if( m_hButtonScrollThreadKilledEvent)
	{
		CloseHandle( m_hButtonScrollThreadKilledEvent);
		m_hButtonScrollThreadKilledEvent = NULL;
	}

	// Close Thread Handles
	if( m_hButtonScrollThread != NULL)
	{
		CloseHandle( m_hButtonScrollThread);
		m_hButtonScrollThread = NULL;
	}
	
	m_PortA.DeInit();
	m_PortB.DeInit();

    // Delete Set Speed Digit Bitmaps
    for (iDigit = 0; iDigit < 10; iDigit++) {
        DeleteObject(m_hIdleSetSpeedDigit[iDigit]);
        DeleteObject(m_hRunSetSpeedDigit[iDigit]);
    }
	m_sqCreationCount--;
	if (m_sqCreationCount <= 0)
	{
		// Delete brush objects
		m_hBrBlack.DeleteObject();
		m_hBrGreen.DeleteObject();
		m_hBrYellow.DeleteObject();
	}

}

void CProcedureScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcedureScreen)
	DDX_Control(pDX, IDC_BUTTON_SETTINGS, m_BtnSettings);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CProcedureScreen, CDialog)
	//{{AFX_MSG_MAP(CProcedureScreen)
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BUTTON_SETTINGS, OnButtonSettings)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_MESSAGE(WM_INTELLIO_SHAVER_CMD, HandleIntellioShaverCmd)
	ON_MESSAGE(WM_REMOTE_CMD, HandleFootCmd)
	ON_MESSAGE(WM_ERROR_CONDITION, HandleErrorConditions)
	ON_MESSAGE(WM_UPDATE_STATUS, UpdateStatus)
	ON_MESSAGE(WM_EXIT_DIALOG, ExitDialog)
	ON_BN_CLICKED(SN_BUTTON_DOWNARROW_PORTA, OnButtonDownArrowPressedPortA)
	ON_BN_CLICKED(SN_BUTTON_UPARROW_PORTA, OnButtonUpArrowPressedPortA)
	ON_BN_CLICKED(SN_BUTTON_WINDOWLOCK_PORTA, OnButtonWindowLockPressedPortA)
	ON_BN_CLICKED(SN_BUTTON_DELTAMODE_PORTA, OnButtonDeltaModePortA)
	ON_BN_CLICKED(SN_BUTTON_DOWNARROW_PORTB, OnButtonDownArrowPressedPortB)
	ON_BN_CLICKED(SN_BUTTON_UPARROW_PORTB, OnButtonUpArrowPressedPortB)
	ON_BN_CLICKED(SN_BUTTON_WINDOWLOCK_PORTB, OnButtonWindowLockPressedPortB)
	ON_BN_CLICKED(SN_BUTTON_DELTAMODE_PORTB, OnButtonDeltaModePortB)
	ON_BN_CLICKED(SN_TEXT_ACTIVEWARNING_PORTA, OnStaticTextActiveWarningPortA)
	ON_BN_CLICKED(SN_TEXT_ACTIVEWARNING_PORTB, OnStaticTextActiveWarningPortB)
	ON_WM_SHOWWINDOW()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcedureScreen message handlers

HBRUSH CProcedureScreen::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{

	HBRUSH hBr;
	
	// Intercept the paint call to change the dialog control colors
	
	// Call the base class implementation first! Otherwise, it may
	// undo what we are trying to accomplish .
	hBr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	int nCtrlID = pWnd->GetDlgCtrlID();
	switch(nCtrlID)
	{
		case SN_TEXT_ACTIVEWARNING_PORTA:
			if(m_PortA.DwWarning() != 0)
			{
				pDC->SetBkColor(SN_YELLOW);
				pDC->SetTextColor(SN_BKGND_COLOR);
				hBr = m_hBrYellow;
			}
			else
			{
				pDC->SetBkColor(SN_BKGND_COLOR);
				pDC->SetTextColor(SN_WHITE);
				hBr = m_hBrBlack;
			}
			break;

		case SN_TEXT_ACTIVEWARNING_PORTB:
			if(m_PortB.DwWarning() != 0)
			{
				pDC->SetBkColor(SN_YELLOW);
				pDC->SetTextColor(SN_BKGND_COLOR);
				hBr = m_hBrYellow;
			}
			else
			{
				pDC->SetBkColor(SN_BKGND_COLOR);
				pDC->SetTextColor(SN_WHITE);
				hBr = m_hBrBlack;
			}
			break;
		
		default :
			pDC->SetBkColor(SN_BKGND_COLOR);
			pDC->SetTextColor(SN_WHITE);
			hBr = m_hBrBlack;
			break;

	}
	
	return hBr;	
		
}

BOOL CProcedureScreen::OnInitDialog() 
{
	// Call the Base class first
	CDialog::OnInitDialog();
	
	CSharedMemory mem;
	SnBool bStatus;
	
	// Initialize Shared memory 
	bStatus = mem.GetInitStatus();
	if( !bStatus)
		mem.Init( m_pParent, m_pControl);

	// Setup the screen
    SetupTextButtons();
    SetupBitmaps();

    CreateSetSpeedBitmaps(mem.m_Font25Bold);

    // Set up Port A
	if (m_PortA.Init() == FALSE)
	{
		DeInit();
		return FALSE;
	}

	// Set up Port B
	if (m_PortB.Init() == FALSE)
	{
		DeInit();
		return FALSE;
	}
	

	// Create events 
	m_hButtonScrollThreadKilledEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if(m_hButtonScrollThreadKilledEvent == NULL)
	{
		DeInit();
		return FALSE;
	}


	// Create thread that monitors button key down states
	m_hButtonScrollThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
					 0,
					 ButtonScrollThread,
					 this,
					 0,
					 &m_hButtonScrollThreadID);
	
	if (m_hButtonScrollThread == NULL)
	{
        DeInit();
		return FALSE;
	}

	DEBUGMSG(TRUE, (TEXT("ProcedureScreen ButtonScrollThread: 0x%08X\n"),m_hButtonScrollThreadID));

    SetDrawRect(TRUE);

	::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,(WPARAM)MSG_UPDATE_TITLE, (LPARAM)SN_SHAVER_REF);

	// Tell the control layer to send WM messages to this screen.
	if( m_pControl)
		m_pControl->SetMessageHandler(this->m_hWnd);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CProcedureScreen::ExitDialog(WPARAM iParam, LPARAM lParam)
{
	DeInit();
	
	// Kill the dialog window and return to main
	CDialog::EndDialog(IDCANCEL);

	return 0L;
}

LRESULT CProcedureScreen::UpdateStatus(WPARAM iParam, LPARAM lParam)
{
	CSharedMemory mem;
	SnWord wFootType = 0;

	switch( iParam)
	{
		
		case MSG_UPDATE_FOOT_STATUS:
			{
				SN_FOOT_STATUS tFootStatus = GetFootStatus();
				if( tFootStatus.usFootAssignedPort != PORTA &&  tFootStatus.usFootAssignedPort != PORTB)
				{
					// Turn Scrolling off if it's on
					if( m_bRemoteKeyDown)
					{
						m_bRemoteKeyDown = FALSE;
						StopScrolling();
					}
				}
				// Process the channel not specified since both change
				m_PortA.UpdateStatus(iParam);
				m_PortB.UpdateStatus(iParam);
			}
			break;
		
	
		case MSG_UPDATE_RUNNING_STATUS:
			if (lParam == PORTA)
				m_PortA.UpdateStatus(iParam);
			if (lParam == PORTB)
				m_PortB.UpdateStatus(iParam);

			if(m_PortA.PortRunning() || m_PortB.PortRunning())
				m_BtnSettings.ShowWindow(SW_HIDE); // Hide settings button motor(s) running
			else
				m_BtnSettings.ShowWindow(SW_SHOW); 
			break;
		
	    case MSG_UPDATE_LANGUAGE:
            SetupTextButtons();
            m_PortA.SetupTextButtons();
            m_PortB.SetupTextButtons();
            break;

		case MSG_UPDATE_ALL_SETTINGS:
            SetupTextButtons();
            m_BtnSettings.RedrawWindow();
            m_PortA.UpdateStatus(iParam);
			m_PortB.UpdateStatus(iParam);
            if(m_PortA.DwWarning()) {
			    m_PortA.DisplayWarning();
            }
            if (m_PortB.DwWarning()) {
			    m_PortB.DisplayWarning();
            }
            break;

		case MSG_UPDATE_HANDPIECE_DUAL_RPM_ON:
		case MSG_UPDATE_HANDPIECE_DUAL_RPM_OFF:
		case MSG_UPDATE_BLADE_STATUS:
		case MSG_UPDATE_MODE_STATUS:
		case MSG_UPDATE_PORT_STATUS:
 		default:
			if (lParam == PORTA)
				m_PortA.UpdateStatus(iParam);
			if (lParam == PORTB)
				m_PortB.UpdateStatus(iParam);
			break;

	}// end switch

 
	return 0L;
}

LRESULT CProcedureScreen::HandleIntellioShaverCmd( WPARAM iParam, LPARAM lParam)
{
	if( m_pControl != NULL)
	{

		switch(iParam)
		{
			case KEY_DOWN_PORTA:
			case KEY_UP_PORTA:
			case KEY_DELTA_MODE_PORTA:
				m_PortA.HandleIntellioShaverCmd( iParam, lParam);
				break;

			case KEY_DOWN_PORTB:
			case KEY_UP_PORTB:
			case KEY_DELTA_MODE_PORTB:
				m_PortB.HandleIntellioShaverCmd( iParam, lParam);
				break;

			default:
				break;

		}

	}

	return 0L;
}

LRESULT CProcedureScreen::HandleFootCmd( WPARAM iParam, LPARAM lParam)
{

   	// Get current footswitch status
	SN_FOOT_STATUS tFootStatus = GetFootStatus();

	switch (iParam)
	{
		case KEY_STOP_SET_SPEED: 
			if( m_bRemoteKeyDown)
			{
				m_bRemoteKeyDown = FALSE;
				StopScrolling();
			}
			break;
	
		case KEY_SET_SPEED_UP:

			if( (tFootStatus.usFootAssignedPort == PORTA) && !m_PortA.m_bDownArrowPressed && !m_PortA.m_bUpArrowPressed)
			{
				m_bRemoteKeyDown = TRUE;
				OnButtonUpArrowPressedPortA();
			}
			else if( (tFootStatus.usFootAssignedPort == PORTB) && !m_PortB.m_bDownArrowPressed && !m_PortB.m_bUpArrowPressed)
			{
				m_bRemoteKeyDown = TRUE;
				OnButtonUpArrowPressedPortB();
			}
			break;
	
		case KEY_SET_SPEED_DOWN:

			if( (tFootStatus.usFootAssignedPort == PORTA) && !m_PortA.m_bDownArrowPressed && !m_PortA.m_bUpArrowPressed)
			{
				m_bRemoteKeyDown = TRUE;
				OnButtonDownArrowPressedPortA();
			}
			else if( (tFootStatus.usFootAssignedPort == PORTB) && !m_PortB.m_bDownArrowPressed && !m_PortB.m_bUpArrowPressed)
			{
				m_bRemoteKeyDown = TRUE;
				OnButtonDownArrowPressedPortB();
			}
			break;

		default:
			break;

    }

	// Need to release scrolling buttons if disconnect occurs
	if( m_bRemoteKeyDown)
	{
		if( m_uiRemoteTimer)
			StopRemoteTimer(); // Stop the timer if it's running
		
		StartRemoteTimer(); // Start the timer
	}
	else
	{
		// Kill the timer if it's running
		if( m_uiRemoteTimer)
			StopRemoteTimer();
	}

	return 0L;
}

LRESULT CProcedureScreen::HandleErrorConditions(WPARAM iParam, LPARAM lParam)
{
	SnBool bFatalError = FALSE;
	SN_FOOT_STATUS tFootStatus;
	SnBool bFlashSaverError = FALSE;
	DWORD dwErrorNum = 0;

	DWORD dwPrevPortAWarning = m_PortA.DwWarning();
	DWORD dwPrevPortBWarning = m_PortB.DwWarning();
	DWORD dwPrevBeeperCount = m_dwBeeperCount;
	
	switch( iParam)
	{

		case COMMUNICATION_ERROR:
			dwErrorNum = 2;
			bFatalError = TRUE;
			break;

		case SYSTEM_RESOURCE_ERROR:
			dwErrorNum = 4;
			bFatalError = TRUE;
			break;

		case WATCHDOG_TIMER_ERROR:
			dwErrorNum = 3;
			bFatalError = TRUE;
			break;

		case FLASH_SAVE_ERROR:
			bFlashSaverError = TRUE;
			break;

		// System Warnings
		case TEMPERATURE_ERROR:
			// If the temperature is excessive shut down the Pump
			// otherwise issue a warning to the screen
			if(lParam == MAX_TEMPERATURE)
			{
				dwErrorNum = 1;
				bFatalError = TRUE;
			}
			else if( lParam == TEMPERATURE_WARNING)
			{
				m_PortA.SetWarning(TEMPERATURE_WARNING);
				m_PortB.SetWarning(TEMPERATURE_WARNING);
				m_dwBeeperCount |= TEMPERATURE_WARNING;
			}
			else
			{
				// Warning has cleared
				m_PortA.ClearWarning(TEMPERATURE_WARNING);
				m_PortB.ClearWarning(TEMPERATURE_WARNING);
				m_dwBeeperCount &= ~TEMPERATURE_WARNING;
			}
			break;

		case MOTORA_SHORT_CIRCUIT:
		case MOTORA_SHORT_CIRCUIT_TIMEOUT:
			if(lParam)
			{
				if( iParam == MOTORA_SHORT_CIRCUIT)
				{
					m_PortA.SetWarning(MOTORA_SHORT_CIRCUIT);
					m_dwBeeperCount |= MOTORA_SHORT_CIRCUIT;
				}
				else
				{
					m_PortA.SetWarning(MOTORA_SHORT_CIRCUIT_TIMEOUT);
					m_dwBeeperCount |= MOTORA_SHORT_CIRCUIT_TIMEOUT;
				}
			}
			else
			{
				// Warning has cleared
				if( iParam == MOTORA_SHORT_CIRCUIT)
				{
					m_PortA.ClearWarning(MOTORA_SHORT_CIRCUIT);
					m_dwBeeperCount &= ~MOTORA_SHORT_CIRCUIT;
				}
				else
				{
					m_PortA.ClearWarning(MOTORA_SHORT_CIRCUIT_TIMEOUT);
					m_dwBeeperCount &= ~MOTORA_SHORT_CIRCUIT_TIMEOUT;
				}
			}
			break;

		case MOTORB_SHORT_CIRCUIT:
		case MOTORB_SHORT_CIRCUIT_TIMEOUT:
			if(lParam)
			{
				if( iParam == MOTORB_SHORT_CIRCUIT)
				{
					m_PortB.SetWarning(MOTORB_SHORT_CIRCUIT);
					m_dwBeeperCount |= MOTORB_SHORT_CIRCUIT;
				}
				else
				{
					m_PortB.SetWarning(MOTORB_SHORT_CIRCUIT_TIMEOUT);
					m_dwBeeperCount |= MOTORB_SHORT_CIRCUIT_TIMEOUT;
				}
			}
			else
			{
				// Warning has cleared
				if( iParam == MOTORB_SHORT_CIRCUIT)
				{
					m_PortB.ClearWarning(MOTORB_SHORT_CIRCUIT);
					m_dwBeeperCount &= ~MOTORB_SHORT_CIRCUIT;
				}
				else
				{
					m_PortB.ClearWarning(MOTORB_SHORT_CIRCUIT_TIMEOUT);
					m_dwBeeperCount &= ~MOTORB_SHORT_CIRCUIT_TIMEOUT;
				}
			}
			break;

		case MOTORA_STALL:
			if(lParam)
			{
				m_PortA.SetWarning(MOTORA_STALL);
				m_dwBeeperCount |= MOTORA_STALL;
			}
			else
			{
				// Warning has cleared
				m_PortA.ClearWarning(MOTORA_STALL);
				m_dwBeeperCount &= ~MOTORA_STALL;
			}
			break;

		case MOTORA_STALL_AND_CURRENT_LIMIT:
			if(lParam)
			{
				m_PortA.SetWarning(MOTORA_STALL_AND_CURRENT_LIMIT);
				m_dwBeeperCount |= MOTORA_STALL_AND_CURRENT_LIMIT;
			}
			else
			{
				// Warning has cleared
				m_PortA.ClearWarning(MOTORA_STALL_AND_CURRENT_LIMIT);
				m_dwBeeperCount &= ~MOTORA_STALL_AND_CURRENT_LIMIT;
			}
			break;

		case MOTORB_STALL:
			if(lParam)
			{
				m_PortB.SetWarning(MOTORB_STALL);
				m_dwBeeperCount |= MOTORB_STALL;
			}
			else
			{
				// Warning has cleared
				m_PortB.ClearWarning(MOTORB_STALL);
				m_dwBeeperCount &= ~MOTORB_STALL;
			}
			break;

		case MOTORB_STALL_AND_CURRENT_LIMIT:
			if(lParam)
			{
				m_PortB.SetWarning(MOTORB_STALL_AND_CURRENT_LIMIT);
				m_dwBeeperCount |= MOTORB_STALL_AND_CURRENT_LIMIT;
			}
			else
			{
				// Warning has cleared
				m_PortB.ClearWarning(MOTORB_STALL_AND_CURRENT_LIMIT);
				m_dwBeeperCount &= ~MOTORB_STALL_AND_CURRENT_LIMIT;
			}
			break;

		case MOTORA_TAC_FAULT:
			if(lParam)
			{
				m_PortA.SetWarning(MOTORA_TAC_FAULT);
				m_dwBeeperCount |= MOTORA_TAC_FAULT;
			}
			else
			{
				// Warning has cleared
				m_PortA.ClearWarning(MOTORA_TAC_FAULT);
				m_dwBeeperCount &= ~MOTORA_TAC_FAULT;
			}
			break;

		case MOTORB_TAC_FAULT:
			if(lParam)
			{
				m_PortB.SetWarning(MOTORB_TAC_FAULT);
				m_dwBeeperCount |= MOTORB_TAC_FAULT;
			}
			else
			{
				// Warning has cleared
				m_PortB.ClearWarning(MOTORB_TAC_FAULT);
				m_dwBeeperCount &= ~MOTORB_TAC_FAULT;
			}
			break;

		case  UNKNOWN_BLADE_ID_PORTA:
			if(lParam)
				m_PortA.SetWarning(UNKNOWN_BLADE_ID_PORTA);
			else
			{
				// Warning has cleared
				m_PortA.ClearWarning(UNKNOWN_BLADE_ID_PORTA);
			}
			break;

		case UNKNOWN_BLADE_ID_PORTB:
			if(lParam)
				m_PortB.SetWarning(UNKNOWN_BLADE_ID_PORTB);
			else
			{
				// Warning has cleared
				m_PortB.ClearWarning(UNKNOWN_BLADE_ID_PORTB);
			}
			break;

		case UNKNOWN_DEVICE_ID_PORTA:
			if(lParam)
				m_PortA.SetWarning(UNKNOWN_DEVICE_ID_PORTA);
			else
			{
				// Warning has cleared
				m_PortA.ClearWarning(UNKNOWN_DEVICE_ID_PORTA);
			}
			break;

		case UNKNOWN_DEVICE_ID_PORTB:
			if(lParam)
				m_PortB.SetWarning(UNKNOWN_DEVICE_ID_PORTB);
			else
			{
				// Warning has cleared
				m_PortB.ClearWarning(UNKNOWN_DEVICE_ID_PORTB);
			}
			break;

		case HALL_PATTERN_FAULT_PORTA:
			if(lParam)
				m_PortA.SetWarning(HALL_PATTERN_FAULT_PORTA);
			else
			{
				// Warning has cleared
				m_PortA.ClearWarning(HALL_PATTERN_FAULT_PORTA);
			}
			break;

		case HALL_PATTERN_FAULT_PORTB:
			if(lParam)
				m_PortB.SetWarning(HALL_PATTERN_FAULT_PORTB);
			else
			{
				// Warning has cleared
				m_PortB.ClearWarning(HALL_PATTERN_FAULT_PORTB);
			}
			break;
		
		case MOTORA_CURRENT_LIMIT:
			if(lParam)
			{
				m_PortA.SetWarning(MOTORA_CURRENT_LIMIT);
				m_dwBeeperCount |= MOTORA_CURRENT_LIMIT;
			}
			else
			{
				// Warning has cleared
				m_PortA.ClearWarning(MOTORA_CURRENT_LIMIT);
				m_dwBeeperCount &= ~MOTORA_CURRENT_LIMIT;
			}
			break;

		case MOTORB_CURRENT_LIMIT:
			if(lParam)
			{
				m_PortB.SetWarning(MOTORB_CURRENT_LIMIT);
				m_dwBeeperCount |= MOTORB_CURRENT_LIMIT;
			}
			else
			{
				// Warning has cleared
				m_PortB.ClearWarning(MOTORB_CURRENT_LIMIT);
				m_dwBeeperCount &= ~MOTORB_CURRENT_LIMIT;
			}
			break;

		case MOTORA_CURRENT_LIMIT_TIMEOUT:
			if(lParam)
			{
				m_PortA.SetWarning(MOTORA_CURRENT_LIMIT_TIMEOUT);
				m_dwBeeperCount |= MOTORA_CURRENT_LIMIT_TIMEOUT;
			}
			else
			{
				// Warning has cleared
				m_PortA.ClearWarning(MOTORA_CURRENT_LIMIT_TIMEOUT);
				m_dwBeeperCount &= ~MOTORA_CURRENT_LIMIT_TIMEOUT;
			}
			break;
		
		case MOTORB_CURRENT_LIMIT_TIMEOUT:
			if(lParam)
			{
				m_PortB.SetWarning(MOTORB_CURRENT_LIMIT_TIMEOUT);
				m_dwBeeperCount |= MOTORB_CURRENT_LIMIT_TIMEOUT;
			}
			else
			{
				// Warning has cleared
				m_PortB.ClearWarning(MOTORB_CURRENT_LIMIT_TIMEOUT);
				m_dwBeeperCount &= ~MOTORB_CURRENT_LIMIT_TIMEOUT;
			}
			break;
		
		case MOTORA_TORQUE_LIMIT:
			if(lParam)
				m_PortA.SetWarning(MOTORA_TORQUE_LIMIT);
			else
			{
				// Warning has cleared
				m_PortA.ClearWarning(MOTORA_TORQUE_LIMIT);
			}
			break;

		case MOTORB_TORQUE_LIMIT:
			if(lParam)
				m_PortB.SetWarning(MOTORB_TORQUE_LIMIT);
			else
			{
				// Warning has cleared
				m_PortB.ClearWarning(MOTORB_TORQUE_LIMIT);
			}
			break;
		
		case FOOTSWITCH_LOW_BATTERY:
			// Get current footswitch status
			tFootStatus = GetFootStatus();
			if(lParam)
			{	
				if(tFootStatus.usPortControl == PORTA)
				{
					m_PortA.SetWarning(FOOTSWITCH_LOW_BATTERY);
					m_PortB.ClearWarning(FOOTSWITCH_LOW_BATTERY);
				}
				else
				{
					m_PortA.ClearWarning(FOOTSWITCH_LOW_BATTERY);
					m_PortB.SetWarning(FOOTSWITCH_LOW_BATTERY);
				}
				m_dwBeeperCount |= FOOTSWITCH_LOW_BATTERY;
			}
			else
			{
				// Warning has cleared
				m_PortA.ClearWarning(FOOTSWITCH_LOW_BATTERY);
				m_PortB.ClearWarning(FOOTSWITCH_LOW_BATTERY);
				m_dwBeeperCount  &= ~FOOTSWITCH_LOW_BATTERY;
			}
			break;

		case FOOTSWITCH_STUCK_PEDAL:
		  	// Get current footswitch status
			tFootStatus = GetFootStatus();
			if(lParam)
			{
				if(tFootStatus.usPortControl == PORTA)
				{
					m_PortA.SetWarning(FOOTSWITCH_STUCK_PEDAL);
					m_PortB.ClearWarning(FOOTSWITCH_STUCK_PEDAL);
				}
				else
				{
					m_PortA.ClearWarning(FOOTSWITCH_STUCK_PEDAL);
					m_PortB.SetWarning(FOOTSWITCH_STUCK_PEDAL);
				}
				m_dwBeeperCount |= FOOTSWITCH_STUCK_PEDAL;
			}
			else
			{
				// Warning has cleared
				m_PortA.ClearWarning(FOOTSWITCH_STUCK_PEDAL);
				m_PortB.ClearWarning(FOOTSWITCH_STUCK_PEDAL);
				m_dwBeeperCount  &= ~FOOTSWITCH_STUCK_PEDAL;
			}
			break;

		case UNKNOWN_FOOTSWITCH_ID:
		   	// Get current footswitch status
			tFootStatus = GetFootStatus();
			if(lParam)
			{
				if(tFootStatus.usPortControl == PORTA)
				{
					m_PortA.SetWarning(UNKNOWN_FOOTSWITCH_ID);
					m_PortB.ClearWarning(UNKNOWN_FOOTSWITCH_ID);
				}
				else
				{
					m_PortA.ClearWarning(UNKNOWN_FOOTSWITCH_ID);
					m_PortB.SetWarning(UNKNOWN_FOOTSWITCH_ID);
				}
				m_dwBeeperCount |= UNKNOWN_FOOTSWITCH_ID;
			}
			else
			{
				// Warning has cleared
				m_PortA.ClearWarning(UNKNOWN_FOOTSWITCH_ID);
				m_PortB.ClearWarning(UNKNOWN_FOOTSWITCH_ID);
				m_dwBeeperCount  &= ~UNKNOWN_FOOTSWITCH_ID;
			}
			break;

		case HANDPIECE_STUCK_BUTTON_PORTA:
			if(lParam)
			{
				m_PortA.SetWarning(HANDPIECE_STUCK_BUTTON_PORTA);
				m_dwBeeperCount |= HANDPIECE_STUCK_BUTTON_PORTA;
			}
			else
			{
				// Warning has cleared
				m_PortA.ClearWarning(HANDPIECE_STUCK_BUTTON_PORTA);
				m_dwBeeperCount &= ~HANDPIECE_STUCK_BUTTON_PORTA;
			}
			break;
		
		case HANDPIECE_STUCK_BUTTON_PORTB:
			if(lParam)
			{
				m_PortB.SetWarning(HANDPIECE_STUCK_BUTTON_PORTB);
				m_dwBeeperCount |= HANDPIECE_STUCK_BUTTON_PORTB;
			}
			else
			{
				// Warning has cleared
				m_PortB.ClearWarning(HANDPIECE_STUCK_BUTTON_PORTB);
				m_dwBeeperCount &= ~HANDPIECE_STUCK_BUTTON_PORTB;
			}
			break;

		case FOOTSWITCH_REQUIRED_PORTA:
			if(lParam)
			{
				m_PortA.SetWarning(FOOTSWITCH_REQUIRED_PORTA);

			}
			else
			{
				// Warning has cleared
				m_PortA.ClearWarning(FOOTSWITCH_REQUIRED_PORTA);

			}
			break;
		
		case FOOTSWITCH_REQUIRED_PORTB:
			if(lParam)
			{
				m_PortB.SetWarning(FOOTSWITCH_REQUIRED_PORTB);

			}
			else
			{
				// Warning has cleared
				m_PortB.ClearWarning(FOOTSWITCH_REQUIRED_PORTB);

			}
			break;

		default:
				
			break;

	} // end switch statement
	
	if(bFatalError && m_bFatalError == FALSE)
	{
        m_bFatalError = TRUE;

		if( m_pControl)
		{
			// Tell the Control layer NOT to send Windows messages to this screen
			m_pControl->SetMessageHandler(NULL);

			Beeper(BEEPER_ON); // turn on the beeper
			
		}

		// Launch the "System Error" Screen
		// This call will never return. User must reboot
		CSystemErrorDlg errDlg(m_pControl, dwErrorNum, SN_CLEAR_TEXT);
		m_bDrawRect = FALSE;
		errDlg.DoModal();

	}
	else if( bFlashSaverError)
	{
		m_bDrawRect = FALSE;
		CMessageBox dlg( m_pControl, SN_SYSTEM_ERROR, SN_SET_SPEEDS_SAVE_FAILURE);
		dlg.DoModal();

		// Redraw the screen
		m_bReDraw = TRUE;

		// Tell the control layer to send WM messages to this screen.
		if( m_pControl)
			m_pControl->SetMessageHandler(this->m_hWnd);

	}
	else
	{
        SnBool yWarningChange = FALSE;
        if(m_PortA.DwWarning() != dwPrevPortAWarning) {
			m_PortA.DisplayWarning();
            yWarningChange = TRUE;
        }

        if (m_PortB.DwWarning() != dwPrevPortBWarning) {
			m_PortB.DisplayWarning();
            yWarningChange = TRUE;
        }

        if (yWarningChange && m_pControl) {
            m_pControl->SendIntellioShaverUpdateIfChange();
        }

		if (m_dwBeeperCount != dwPrevBeeperCount)
			Beeper(m_dwBeeperCount ? BEEPER_ON : BEEPER_OFF);
	}
	return 0L;
}

void CProcedureScreen::Beeper(SnWord usState)
{

	if( (usState == BEEPER_ON))
	{
		// Turn on the beeper
		m_pControl->SetCmdState(SET_SYSTEM_BEEPER, &usState, sizeof(usState));
	}
	else if( (usState == BEEPER_OFF) && (m_dwBeeperCount <= 0))
	{
		// Turn the beeper off
		m_pControl->SetCmdState(SET_SYSTEM_BEEPER, &usState, sizeof(usState));
	}
}


void CProcedureScreen::OnLButtonDown(UINT nFlags, CPoint point) 
{
    RECT tWindow;
    CPoint tBasePoint;

    // Conver to base screen co-ords
    this->GetWindowRect(&tWindow);
    tBasePoint.x = point.x + tWindow.left;
    tBasePoint.y = point.y + tWindow.top;

    if (!m_PortA.m_bDownArrowHidden && !m_PortA.m_bDownArrowPressed &&
        ButtonPressed(m_PortA.m_pDownArrowRect, &tBasePoint))
    {
        m_PortA.OnButtonDownArrowPressed();
    }
    else if (!m_PortA.m_bUpArrowHidden && !m_PortA.m_bUpArrowPressed &&
        ButtonPressed(m_PortA.m_pUpArrowRect, &tBasePoint))
    {
        m_PortA.OnButtonUpArrowPressed();
    }
    else if (!m_PortB.m_bDownArrowHidden && !m_PortB.m_bDownArrowPressed &&
        ButtonPressed(m_PortB.m_pDownArrowRect, &tBasePoint))
    {
        m_PortB.OnButtonDownArrowPressed();
    }
    else if (!m_PortB.m_bUpArrowHidden && !m_PortB.m_bUpArrowPressed &&
        ButtonPressed(m_PortB.m_pUpArrowRect, &tBasePoint))
    {
        m_PortB.OnButtonUpArrowPressed();
    }
    else if (ButtonPressed(m_PortA.m_pHandpieceCountButtonPortARect, &tBasePoint))
    {
        m_PortA.OnButtonHandpieceCountPressedPortA();
        m_bReDraw = TRUE;
    }
    else if (ButtonPressed(m_PortB.m_pHandpieceCountButtonPortBRect, &tBasePoint))
    {
        m_PortB.OnButtonHandpieceCountPressedPortB();
        m_bReDraw = TRUE;
    }
    else
    {
    	CDialog::OnLButtonDown(nFlags, point);
    }
}

void CProcedureScreen::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(!m_bRemoteKeyDown)
		StopScrolling(); // Disable scrolling

	// Update WindowLock Button for both Ports
	m_PortA.OnLButtonUp(nFlags, point);
	m_PortB.OnLButtonUp(nFlags, point);
	
	CDialog::OnLButtonUp(nFlags, point);
}

BOOL CProcedureScreen::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	RECT *ptRect;
    POINT tCursor;
	int windowLockPressed = 0;

    GetCursorPos(&tCursor);

    // Check if any button is already pressed
 	if( m_PortA.m_bDownArrowPressed)
    {
        ptRect = m_PortA.m_pDownArrowRect;
    }
	else if( m_PortA.m_bUpArrowPressed)
    {
        ptRect = m_PortA.m_pUpArrowRect;
    }
	else if( m_PortA.m_bWindowLockPressed)
    {
        ptRect = m_PortA.m_pWindowLockRect;
		windowLockPressed = PORTA;
    }
	else if( m_PortB.m_bDownArrowPressed)
    {
        ptRect = m_PortB.m_pDownArrowRect;
    }
	else if( m_PortB.m_bUpArrowPressed)
    {
        ptRect = m_PortB.m_pUpArrowRect;
    }
	else if( m_PortB.m_bWindowLockPressed)
    {
        ptRect = m_PortB.m_pWindowLockRect;
		windowLockPressed = PORTB;
    }
	else
    {
        return CDialog::OnSetCursor(pWnd, nHitTest, message);
    }

    // If a button is pressed and the touch drifts off the button, release the button
    if (!ButtonPressed(ptRect, &tCursor))
	{
		if(windowLockPressed == PORTA)
			m_PortA.OnLButtonUp(0, tCursor);
		else if(windowLockPressed == PORTB)
			m_PortB.OnLButtonUp(0, tCursor);
        else if(!m_bRemoteKeyDown)
			StopScrolling();
    }
	
	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CProcedureScreen::DrawLines()
{
	if (!m_bDrawRect)
		return;

	// draws lines and rectangles 
	POINT startPoint, endPoint;
	CSharedMemory mem;
	COLORREF colorRef;
	
	colorRef = SN_LINE_COLOR;

	//
	// Draw lines
	//
	// 

	m_SnHelp.SetLineWidth( SN_LINE_WIDTH_6); // Set the line width

	// Top horizontal line
	startPoint.x = 0;
	startPoint.y = 68;
	endPoint.x = 800;
	endPoint.y = 68;
	m_SnHelp.DrawLine( SN_LINE_COLOR, startPoint, endPoint);

	// Bottom horizontal line
	startPoint.x = 0;
	startPoint.y = 403;
	endPoint.x = 800;
	endPoint.y = 403;
	m_SnHelp.DrawLine( SN_LINE_COLOR, startPoint, endPoint);

	// Vertical line
	startPoint.x = 400;
	startPoint.y = 68;
	endPoint.x = 400;
	endPoint.y = 403;
	m_SnHelp.DrawLine( SN_LINE_COLOR, startPoint, endPoint);

}

void CProcedureScreen::SetupTextButtons()
{
	CSharedMemory mem;

    // Setup Settings button 
	m_BtnSettings.LoadBitmaps(IDB_BITMAP_BK_BUTTON, IDB_BITMAP_BK_BUTTON_PRESSED,
        mem.m_Font20Normal, m_SnHelp.GetString(SN_SETTINGS));	
}

void CProcedureScreen::SetupBitmaps()
{
	// Load the Window Lock Bitmaps
	m_SnHelp.LoadBitmap(&m_WindowLock, IDB_BITMAP_BUTTON_WINDOW);
	m_SnHelp.LoadBitmap(&m_WindowLockPressed, IDB_BITMAP_BUTTON_WINDOW_PRESSED);
	
	// Load the up and down arrow bitmaps
	m_SnHelp.LoadBitmap(&m_ArrowUp, IDB_BITMAP_BUTTON_ARROW_UP);
	m_SnHelp.LoadBitmap(&m_ArrowUpPressed, IDB_BITMAP_BUTTON_ARROW_UP_PRESSED);
	m_SnHelp.LoadBitmap(&m_ArrowDown, IDB_BITMAP_BUTTON_ARROW_DOWN);
	m_SnHelp.LoadBitmap(&m_ArrowDownPressed, IDB_BITMAP_BUTTON_ARROW_DOWN_PRESSED);

	// Load direction bitmaps
	m_SnHelp.LoadBitmap(&m_ArrowLeftGreen, IDB_BITMAP_ARROW_LEFT);
	m_SnHelp.LoadBitmap(&m_ArrowLeftWhite, IDB_BITMAP_ARROW_LEFT_WHITE);
	m_SnHelp.LoadBitmap(&m_ArrowRightGreen, IDB_BITMAP_ARROW_RIGHT);
	m_SnHelp.LoadBitmap(&m_ArrowRightWhite, IDB_BITMAP_ARROW_RIGHT_WHITE);

	// Load Test Dot bitmaps
	m_SnHelp.LoadBitmap(&m_BitmapTestDot, IDB_BITMAP_TESTDOT);
	m_SnHelp.LoadBitmap(&m_BitmapTestDotBlue, IDB_BITMAP_TESTDOT_BLUE);
	m_SnHelp.LoadBitmap(&m_BitmapTestDotOrange, IDB_BITMAP_TESTDOT_ORANGE);
	m_SnHelp.LoadBitmap(&m_BitmapTestDotYellow, IDB_BITMAP_TESTDOT_YELLOW);
}	

void CProcedureScreen::OnButtonSettings() 
{
	// if either port has a running device you can't get to the settings menu
	if(!m_PortA.PortRunning() && !m_PortB.PortRunning() && m_pControl && m_pControl->CheckPowerOnSelfTest())
	{
		m_bDrawRect = FALSE;
		if( m_pControl)
		{
			// Disable devices
			SnWord usTemp = (SnWord)FALSE;
			m_pControl->SetCmdState(SET_MC_CONTROLS, &usTemp, sizeof(usTemp));
		}
		
        UpdateIntellioShaverSettingsScreen(TRUE);
        SendIntellioShaverUpdateIfChange();

		// Tell the control layer not to send WM messages to this screen.
		m_pControl->SetMessageHandler(NULL);
		
		// Launch the settings screen
		CSettingsScreen dlg(m_pControl);
		dlg.DoModal();
	
		// Tell the control layer to send WM messages to this screen.
		m_pControl->SetMessageHandler(this->m_hWnd);
	
        UpdateIntellioShaverSettingsScreen(FALSE);
        SendIntellioShaverUpdateIfChange();

		if( m_pControl)
		{
			// Enable devices
			SnWord usTemp = (SnWord)TRUE;
			m_pControl->SetCmdState(SET_MC_CONTROLS, &usTemp, sizeof(usTemp));
		
#if WINDOWLOCK_BUTTON_DIGITAL_FOOT
			// Check to see if the footswitch has been remapped
			SN_FOOT_STATUS tFootStatus = GetFootStatus();

			SnBool bShowWindowLock;

			bShowWindowLock = (( tFootStatus.usType == TYPE_DIGITAL_FOOTPEDAL) && (tFootStatus.usFootAssignedPort == PORTA));
			m_PortA.OnButtonSettings(bShowWindowLock);

			bShowWindowLock = (( tFootStatus.usType == TYPE_DIGITAL_FOOTPEDAL) && (tFootStatus.usFootAssignedPort == PORTB));
			m_PortB.OnButtonSettings(bShowWindowLock);
#else
			m_PortA.OnButtonSettings(TRUE);
			m_PortB.OnButtonSettings(TRUE);
#endif
		}
		
		::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,(WPARAM)MSG_UPDATE_TITLE, (LPARAM)SN_SHAVER_REF);

		m_bReDraw = TRUE;
	}
}


void CProcedureScreen::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	if( m_bReDraw)
	{
		m_bDrawRect = TRUE;

        DrawLines();
		m_PortA.RedrawNonStatics();
		m_PortB.RedrawNonStatics();
		m_PortA.DrawWindowLockButton();
		m_PortB.DrawWindowLockButton();
        
        //RedrawHandpieceCountPopUpPortB();

		m_bReDraw = FALSE;
	}

	// Do not call CDialog::OnPaint() for painting messages
}


void CProcedureScreen::KillThreads()
{
	DWORD waitStatus;
	DWORD exitCode;

	// Terminate all running threads
	m_bKillThreads = TRUE;

	if( m_hButtonScrollThread)
	{
		// wait for the thread to terminate, time out after a certain period of time
		if( m_hButtonScrollThreadKilledEvent)
			waitStatus = WaitForSingleObject( m_hButtonScrollThreadKilledEvent,THREAD_TERMINATION_WAIT );

		if( waitStatus == WAIT_TIMEOUT) 
		{
			GetExitCodeThread( m_hButtonScrollThread, &exitCode);
		
			TerminateThread( m_hButtonScrollThread, exitCode);
	
		}
		// Close the thread handle
		CloseHandle( m_hButtonScrollThread);
		m_hButtonScrollThread = NULL;
		
	}
	
	m_PortA.KillThread();
	m_PortB.KillThread();

}


void CProcedureScreen::StartScrollingTimer()
{
	if( m_uiScrollingTimer == 0)
		m_uiScrollingTimer = SetTimer(1, KEY_DOWN_TIME,0);
}

void CProcedureScreen::StopScrollingTimer()
{
	if( m_uiScrollingTimer != 0)
	{
		KillTimer( m_uiScrollingTimer);
		m_uiScrollingTimer = 0;
	}
}

void CProcedureScreen::StartRemoteTimer()
{
	if( m_uiRemoteTimer == 0)
		m_uiRemoteTimer = SetTimer(2, 400,0);
}

void CProcedureScreen::StopRemoteTimer()
{
	if( m_uiRemoteTimer != 0)
	{
		KillTimer( m_uiRemoteTimer);
		m_uiRemoteTimer = 0;
	}
}

void CProcedureScreen::OnTimer(UINT nIDEvent) 
{
	if( nIDEvent == 1)
	{
		StopScrollingTimer(); // stop the timer
		m_bWaitToScroll = FALSE; // Ok to scroll
	}
	else if( nIDEvent == 2)
	{
		StopRemoteTimer();

		// Turn Scrolling off if it's on
		if( m_bRemoteKeyDown)
		{
			m_bRemoteKeyDown = FALSE;
			StopScrolling();
		}

	}
}

void CProcedureScreen::StopScrolling()
{
	// Stop the timer a button has been released
	StopScrollingTimer();
	
	m_bWaitToScroll = TRUE; // Reset the scrolling flag
	
	if( m_PortA.m_bDownArrowPressed)
	{
		m_PortA.m_bDownArrowPressed = FALSE;
        m_PortA.DrawDownArrow(TRUE);
	}
	else if( m_PortA.m_bUpArrowPressed)
	{
		m_PortA.m_bUpArrowPressed = FALSE;
        m_PortA.DrawUpArrow(TRUE);
	}
	else if( m_PortB.m_bDownArrowPressed)
	{
		m_PortB.m_bDownArrowPressed = FALSE;
        m_PortB.DrawDownArrow(TRUE);
	}
	else if( m_PortB.m_bUpArrowPressed)
	{
		m_PortB.m_bUpArrowPressed = FALSE;
        m_PortB.DrawUpArrow(TRUE);
	}
	else
	{
		// Don't do anything
	}
}

DWORD WINAPI ButtonScrollThread(LPVOID pParam)
{
	CProcedureScreen *pClass = (CProcedureScreen*)pParam;
	
	while(!pClass->m_bKillThreads)
	{
		if (!pClass->m_bWaitToScroll)
		{
			if( pClass->m_PortA.m_bDownArrowPressed)
			{
				if (pClass->m_PortA.ArrowScrollScaler())
					pClass->m_PortA.UpdateData( DOWN);
			}
			else if( pClass->m_PortA.m_bUpArrowPressed)
			{
				if (pClass->m_PortA.ArrowScrollScaler())
					pClass->m_PortA.UpdateData( UP);
			}
			else if( pClass->m_PortB.m_bDownArrowPressed)
			{
				if (pClass->m_PortB.ArrowScrollScaler())
					pClass->m_PortB.UpdateData( DOWN);
			}
			else if( pClass->m_PortB.m_bUpArrowPressed)
			{
				if (pClass->m_PortB.ArrowScrollScaler())
					pClass->m_PortB.UpdateData( UP);
			}
		}
	
		Sleep(50);
	}
	
	// Set the Thread Killed Event
	SetEvent( pClass->m_hButtonScrollThreadKilledEvent);
	
	return 0;
}


void CProcedureScreen::DisplayWarningDetails(DWORD dwPortActiveWarning)
{

	if ( !dwPortActiveWarning)
	{

	}
	else
	{
		DWORD	dwTextId = 0;

		switch (dwPortActiveWarning)
		{
		case SN_TEMPERATURE_FAULT:
			dwTextId = SN_TEMPERATURE_FAULT_DETAIL;
			break;
		case  SN_INVALID_BLADE_ID:
			dwTextId = SN_INVALID_BLADE_ID_DETAIL;
			break;
		case  SN_INVALID_DEVICE_ID:
			dwTextId = SN_INVALID_DEVICE_ID_DETAIL;
			break;
		case  SN_MOTOR_STALL:
			dwTextId = SN_MOTOR_STALL_DETAIL;
			break;
		case  SN_MOTOR_STALL_AND_CURRENT_LIMIT:
			dwTextId = SN_MOTOR_STALL_AND_CURRENT_LIMIT_DETAIL;
			break;
		case  SN_MOTOR_TAC_FAULT:
			dwTextId = SN_MOTOR_TAC_FAULT_DETAIL;
			break;
		case  SN_SHORT_CIRCUIT_DETECTED:
			dwTextId = SN_SHORT_CIRCUIT_DETAIL;
			break;
		case  SN_UNKNOWN_FOOT_ID:
			dwTextId = SN_UNKNOWN_FOOT_ID_DETAIL;
			break;
		case  SN_HAND_STUCK_BUTTON:
			dwTextId = SN_HAND_STUCK_BUTTON_DETAIL;
			break;
		case  SN_FOOT_STUCK_PEDAL:
			dwTextId = SN_FOOT_STUCK_PEDAL_DETAIL;
			break;
		case  SN_MOTOR_CURRENT_LIMIT_TIMEOUT:
			dwTextId = SN_MOTOR_CURRENT_LIMIT_TIMEOUT_DETAIL;	
			break;
		case  SN_MOTOR_CURRENT_LIMIT:
			dwTextId = SN_MOTOR_CURRENT_LIMIT_DETAIL;
			break;
		case  SN_INVALID_HALL_PATTERN:
			dwTextId = SN_INVALID_HALL_PATTERN_DETAIL;
			break;
		case  SN_FOOT_LOW_BATTERY:
			dwTextId = SN_FOOT_LOW_BATTERY_DETAIL;
			break;
		case  SN_FOOTSWITCH_REQUIRED:
			dwTextId = SN_FOOTSWITCH_REQUIRED_DETAIL;
			break;
		}
			
		CMessageBox dlg( m_pControl, SN_WARNING, dwTextId);

		m_bDrawRect = FALSE;
		dlg.DoModal();

		if( m_pControl)
			m_pControl->SetMessageHandler(this->m_hWnd);

		m_bReDraw = TRUE;
	}
}

SN_FOOT_STATUS CProcedureScreen::GetFootStatus()
{
	SN_FOOT_STATUS tFootStatus;

	if( m_pControl)
  			m_pControl->GetCmdState(GET_MC_FOOT_STATUS, &tFootStatus, sizeof(SN_FOOT_STATUS));

	return tFootStatus;
}

SnBool CProcedureScreen::CreateSetSpeedBitmaps(CFont *pFont)
{
    const TCHAR *pwDigits = TEXT("0123456789");
    RECT tRect = {0, 0, SN_SET_SPEED_WIDTH, SN_SET_SPEED_HEIGHT};
    CBitmap * pOldBitmap;
    CSharedMemory mem;
    CFont * pOldFont;
    SnQByte qDigit;
    CDC hMemCDC;

    // Create a Memory DC that is compatible with the Display
    hMemCDC.CreateCompatibleDC(CSnHelp::m_hDisplayCDC);

    // Setup the Font for the Set Speed Display
    pOldFont = hMemCDC.SelectObject( pFont);

    // Setup the Colors for Text Draw (White on Black)
    hMemCDC.SetTextColor( SN_WHITE);
    hMemCDC.SetBkColor( SN_BLACK);

    // Create a HBITMAP handle for each Set Speed Digit (White on Black)
    for (qDigit = 0; qDigit < 10; qDigit++) {
        m_hIdleSetSpeedDigit[qDigit].CreateCompatibleBitmap(CSnHelp::m_hDisplayCDC, SN_SET_SPEED_WIDTH, SN_SET_SPEED_HEIGHT);
        pOldBitmap = hMemCDC.SelectObject( m_hIdleSetSpeedDigit + qDigit);
        hMemCDC.FillRect( &tRect, &m_hBrBlack);
        hMemCDC.DrawText( &pwDigits[qDigit], 1, &tRect, DT_LEFT | DT_VCENTER);
        hMemCDC.SelectObject( pOldBitmap);
    }

    // Setup the Colors for Text Draw (Black on Green)
    hMemCDC.SetTextColor( SN_BLACK);
    hMemCDC.SetBkColor( SN_GREEN);

    // Create a HBITMAP handle for each Set Speed Digit (Black on Green)
    for (qDigit = 0; qDigit < 10; qDigit++) {
        m_hRunSetSpeedDigit[qDigit].CreateCompatibleBitmap(CSnHelp::m_hDisplayCDC, SN_SET_SPEED_WIDTH, SN_SET_SPEED_HEIGHT);
        pOldBitmap = hMemCDC.SelectObject( m_hRunSetSpeedDigit + qDigit);
        hMemCDC.FillRect( &tRect, &m_hBrGreen);
        hMemCDC.DrawText( &pwDigits[qDigit], 1, &tRect, DT_LEFT | DT_VCENTER);
        hMemCDC.SelectObject( pOldBitmap);
    }

    hMemCDC.SelectObject( pOldFont);
    hMemCDC.DeleteDC();

    return TRUE;
}
