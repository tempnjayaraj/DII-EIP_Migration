// FactoryScreen.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "FactoryScreen.h"
#include "SerialNumberPopUp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFactoryScreen dialog


CFactoryScreen::CFactoryScreen(CControl* pControl, DWORD dwMode, CWnd* pParent /*=NULL*/)
	: CDialog(CFactoryScreen::IDD, pParent)
{
	// Initialize member variables
	m_pParent = pParent;
	m_pControl = pControl;
    m_dwMode = dwMode;

    m_yPostComplete = FALSE;
    m_wTestMode = NO_TEST;

    m_yTemperatureTest = FALSE;
    m_yNvRamTest = FALSE;
    m_yTouchTest = FALSE;
    m_yTouchingScreen = FALSE;

    m_qTouchPressCnt = 0;
    m_qTouchReleaseCnt = 0;
    m_qTouchOutOfBoundsCnt = 0;

    m_yMiddlePressed = FALSE;
    m_csMiddleLabel = SN_CLEAR_TEXT;
    m_yRightPressed = FALSE;
    m_csRightLabel = SN_CLEAR_TEXT;
    m_ySetPressed = FALSE;
    m_csSetLabel = SN_CLEAR_TEXT;

    m_hStatusThread = NULL;
	m_yStatusThreadKilled = FALSE;
	m_hStatusThreadKilledEvent = NULL;

    m_yKillThreads = FALSE;

    m_csTestList = SN_CLEAR_TEXT;
    m_csInstructionList = SN_CLEAR_TEXT;

    // create solid brush for background color
	m_hBrush = CreateSolidBrush(SN_BKGND_COLOR);

    m_ySystemError = FALSE;
    m_qHandpieceError = 0;

    m_yFailedTest = FALSE;
    m_yRepeatTest = FALSE;

	//{{AFX_DATA_INIT(CFactoryScreen)
	//}}AFX_DATA_INIT
}

CFactoryScreen::~CFactoryScreen()
{
    DeInit();
}

void CFactoryScreen::DeInit(void)
{
	if (m_pControl) {
		// Tell the control layer we're no longer in TEST MODE
		unsigned short usInput = 0;
		// Tell the control layer we're in test mode
		m_pControl->SetCmdState(SET_FACTORY_TEST_MODE, &usInput, sizeof(usInput));
	}
	
	// Terminate any running threads
	KillThreads();
	
	if (m_hStatusThread) {
		CloseHandle(m_hStatusThread);
		m_hStatusThread = NULL;
	}

	if (m_hStatusThreadKilledEvent) {
		CloseHandle(m_hStatusThreadKilledEvent);
		m_hStatusThreadKilledEvent = NULL;
	}

	// Delete solid brush object
	DeleteObject(m_hBrush);
}

void CFactoryScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFactoryScreen)
	DDX_Control(pDX, IDC_STATIC_USAGE_CNT, m_StaticUsageCnt);
	DDX_Control(pDX, IDC_BUTTON_SET_DEFAULT, m_BtnSet);
	DDX_Control(pDX, IDC_BUTTON_RIGHT, m_BtnRight);
	DDX_Control(pDX, IDC_BUTTON_MIDDLE, m_BtnMiddle);
	DDX_Control(pDX, IDC_STATIC_TEST_TITLE, m_StaticTestTitle);
	DDX_Control(pDX, IDC_STATIC_TEST_TEXT, m_StaticTestText);
	DDX_Control(pDX, IDC_STATIC_TEST_LIST_TITLE, m_StaticTestListTitle);
	DDX_Control(pDX, IDC_STATIC_INSTRUCTIONS_TITLE, m_StaticInstructionsTitle);
	DDX_Control(pDX, IDC_STATIC_INSTRUCTIONS_TEXT, m_StaticInstructionsText);
	DDX_Control(pDX, IDC_EDIT_TEST_LIST_TEXT, m_EditTestListText);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFactoryScreen, CDialog)
	//{{AFX_MSG_MAP(CFactoryScreen)
	ON_WM_CTLCOLOR()
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_BUTTON_MIDDLE, OnButtonMiddle)
	ON_BN_CLICKED(IDC_BUTTON_RIGHT, OnButtonRight)
	ON_BN_CLICKED(IDC_BUTTON_SET_DEFAULT, OnButtonSet)
	ON_WM_SETCURSOR()
	ON_MESSAGE(WM_ERROR_CONDITION, HandleErrorConditions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFactoryScreen message handlers

HBRUSH CFactoryScreen::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hBrush;
	
	// Intercept the paint call to change the dialog control colors
	
	// Call the base class implementation first! Otherwise, it may
	// undo what we are trying to accomplish .
	hBrush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// Set the Background color
	pDC->SetBkColor(SN_BKGND_COLOR);
	pDC->SetTextColor(WHITE);
		
	return m_hBrush;
}

BOOL CFactoryScreen::OnInitDialog() 
{
    CString csTestList;
    CString csUsageCnt;
	CDialog::OnInitDialog();
	CSharedMemory mem;
    SnBool yStatus;

    // If shared memory has not been initialized do it now
	yStatus = mem.GetInitStatus();
	if (!yStatus)
		mem.Init(m_pParent, m_pControl);

    SetupFonts(); 
	SetupBitmaps();

    SetDlgItemText(IDC_STATIC_TEST_LIST_TITLE, TEXT("Test List (Pass/Fail)"));
    m_csTestList =  TEXT("POST...\t\t");
	SetDlgItemText(IDC_EDIT_TEST_LIST_TEXT, m_csTestList);
	SetDlgItemText(IDC_STATIC_INSTRUCTIONS_TITLE, TEXT("Instructions"));	
	SetDlgItemText(IDC_STATIC_INSTRUCTIONS_TEXT, TEXT("Please wait for Power On Self Test to complete."));	

    csUsageCnt.Format(_T("Usage Count: %d"), m_pControl ? m_pControl->GetUsageCnt() : 0);
    SetDlgItemText(IDC_STATIC_USAGE_CNT, csUsageCnt);

    m_StaticTestTitle.ShowWindow(SW_HIDE);
    m_StaticTestText.ShowWindow(SW_HIDE);
    m_csMiddleLabel = TEXT("Board Tests");
    m_csRightLabel = TEXT("System Tests");
    m_csSetLabel = TEXT("Set Default");
    m_BtnMiddle.ShowWindow(SW_HIDE);
    m_BtnRight.ShowWindow(SW_HIDE);
    m_BtnSet.ShowWindow(SW_HIDE);

    // Create event to indicate thread termination
	m_hStatusThreadKilledEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if(m_hStatusThreadKilledEvent == NULL) {
        DeInit();
		return FALSE;
	}

	// create thread used for purging indicator
	m_hStatusThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
									  0,
									  FactoryStatusThread,
									  this,
									  0,
									  &m_hStatusThreadID);
		
	if (m_hStatusThread == NULL) {
		DeInit();
		return FALSE;
	}
	DEBUGMSG(TRUE, (TEXT("FactoryStatusThread: 0x%08X\n"),m_hStatusThreadID));

	SnWord wInput = FACTORY_MODE;
	// Tell the control layer we're in test mode
	m_pControl->SetCmdState(SET_FACTORY_TEST_MODE, &wInput, sizeof(wInput));

	// Tell the control layer to send WM messages to this screen.
	m_pControl->SetMessageHandler(this->m_hWnd);

    return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFactoryScreen::SetupFonts()
{
	CSharedMemory mem;

	m_StaticTestListTitle.SetFont(mem.m_Font16Normal, TRUE);
	m_EditTestListText.SetFont(mem.m_Font16Normal, TRUE);
	m_StaticInstructionsTitle.SetFont(mem.m_Font16Normal, TRUE);
	m_StaticInstructionsText.SetFont(mem.m_Font16Normal, TRUE);
	m_StaticTestTitle.SetFont(mem.m_Font16Normal, TRUE);
	m_StaticTestText.SetFont(mem.m_Font16Normal, TRUE);
	m_StaticUsageCnt.SetFont(mem.m_Font16Normal, TRUE);
}

void CFactoryScreen::SetupBitmaps()
{
    //
	// Setup Buttons
	//
	m_BtnMiddle.LoadBitmaps(IDB_BITMAP_BK_BUTTON, IDB_BITMAP_BK_BUTTON_PRESSED);
	m_BtnRight.LoadBitmaps(IDB_BITMAP_BK_BUTTON, IDB_BITMAP_BK_BUTTON_PRESSED);
	m_BtnSet.LoadBitmaps(IDB_BITMAP_BK_BUTTON, IDB_BITMAP_BK_BUTTON_PRESSED);
}

void CFactoryScreen::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CSharedMemory mem;
	CString csTemp;

	if( (lpDrawItemStruct->itemAction == ODA_FOCUS))
	{
		// Don't do anything, we don't want a button redrawn
		// when it gains or loses focus
	}
	else
	{
		// Call the base class implementation first! Otherwise, it may
		// undo what we are trying to accomplish .
		CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
		
		switch( nIDCtl)
		{
			case IDC_BUTTON_MIDDLE:
				m_SnHelp.DrawTextOnButton(&m_BtnMiddle,
										  mem.m_Font16Normal,
										  m_csMiddleLabel);
                break;
			case IDC_BUTTON_RIGHT:
				m_SnHelp.DrawTextOnButton(&m_BtnRight,
										  mem.m_Font16Normal,
										  m_csRightLabel);
				break;
			case IDC_BUTTON_SET_DEFAULT:
				m_SnHelp.DrawTextOnButton(&m_BtnSet,
										  mem.m_Font16Normal,
                                          m_csSetLabel);
                break;
			
			default:	
				break;
		}
	}
}

LRESULT CFactoryScreen::HandleErrorConditions(WPARAM iParam, LPARAM lParam)
{
	CString csSystemFailure;
    SnBool ySystemError = FALSE;

    switch( iParam) {
	case COMMUNICATION_ERROR:
		csSystemFailure += TEXT("Internal Communication Error\r");
        ySystemError = TRUE;
		break;
	case SYSTEM_RESOURCE_ERROR:
		csSystemFailure += TEXT("System Resource Failure\r");
        ySystemError = TRUE;
		break;
	case TEMPERATURE_ERROR:
		if (lParam == MAX_TEMPERATURE) {
			csSystemFailure += TEXT("Temperature Failure\r");
            ySystemError = TRUE;
		}
		break;
	case WATCHDOG_TIMER_ERROR:
		csSystemFailure += TEXT("Watchdog Failure\r");
        ySystemError = TRUE;
        break;

	case MOTORA_SHORT_CIRCUIT:
	case MOTORA_SHORT_CIRCUIT_TIMEOUT:
	case MOTORB_SHORT_CIRCUIT:
	case MOTORB_SHORT_CIRCUIT_TIMEOUT:
	case MOTORA_STALL:
	case MOTORA_STALL_AND_CURRENT_LIMIT:
	case MOTORB_STALL:
	case MOTORB_STALL_AND_CURRENT_LIMIT:
	case MOTORA_TAC_FAULT:
	case MOTORB_TAC_FAULT:
	case HALL_PATTERN_FAULT_PORTA:
	case HALL_PATTERN_FAULT_PORTB:
	case MOTORA_CURRENT_LIMIT:
	case MOTORA_CURRENT_LIMIT_TIMEOUT:
	case MOTORB_CURRENT_LIMIT:
	case MOTORB_CURRENT_LIMIT_TIMEOUT:
        if(lParam) {
            m_qHandpieceError |= iParam;
        }
		break;

	case UNKNOWN_DEVICE_ID_PORTA:
	case UNKNOWN_DEVICE_ID_PORTB:
    case HANDPIECE_STUCK_BUTTON_PORTA:
	case HANDPIECE_STUCK_BUTTON_PORTB:
	case UNKNOWN_BLADE_ID_PORTA:
	case UNKNOWN_BLADE_ID_PORTB:
	case MOTORA_TORQUE_LIMIT:
	case MOTORB_TORQUE_LIMIT:
		break;

    case FOOTSWITCH_STUCK_PEDAL:
	case UNKNOWN_FOOTSWITCH_ID:
    case FOOTSWITCH_LOW_BATTERY:
		break;

	default:
		break;

	} // end switch statement

    if (ySystemError && !m_ySystemError) {
        m_csTestList += TEXT("\nSYSTEM FAILURE");
        SetDlgItemText(IDC_EDIT_TEST_LIST_TEXT, m_csTestList);
        NewInstructions(csSystemFailure);
        AddInstructions(TEXT("\rRecord the System Error and shut down the system."));
        m_BtnMiddle.ShowWindow(SW_HIDE);
        m_BtnRight.ShowWindow(SW_HIDE);
        m_ySystemError = TRUE;
    }

	return 0L;
}

SnBool CFactoryScreen::Delay(DWORD dwMilliSecond)
{

	
	if(m_yKillThreads) {
        return FALSE;
    }
	Sleep(dwMilliSecond);
	if(m_yKillThreads) {
        return FALSE;
    }

    return TRUE;
}

void CFactoryScreen::KillThreads()
{
	DWORD waitStatus;
	DWORD exitCode;

	// Terminate any running threads
	m_yKillThreads = TRUE;

	if (m_yStatusThreadKilled == FALSE) {
		// wait for the thread to terminate, time out after a certain period of time
        if (m_hStatusThreadKilledEvent) {
			waitStatus = WaitForSingleObject(m_hStatusThreadKilledEvent, CTL_THREAD_TERMINATION_WAIT);
			
		    if (waitStatus == WAIT_TIMEOUT) {
			    GetExitCodeThread(m_hStatusThread, &exitCode);
			    TerminateThread(m_hStatusThread, exitCode);
		    }
        }
		m_yStatusThreadKilled = TRUE;
	}
}

void CFactoryScreen::CheckForPOST(void)
{
    if (!m_yPostComplete && m_pControl->CheckPowerOnSelfTest()) {
        m_csTestList += TEXT("PASSED\r\n");
        SetDlgItemText(IDC_EDIT_TEST_LIST_TEXT, m_csTestList);
	    NewInstructions(TEXT("Select one of the Test Buttons below or press the "));
		AddInstructions(TEXT("Set Default Button to reset the Dyonics II Control Unit Settings "));
		AddInstructions(TEXT("to Factory Defaults."));	
        m_BtnMiddle.ShowWindow(SW_SHOW);
        m_BtnRight.ShowWindow(SW_SHOW);
        m_BtnSet.ShowWindow(SW_SHOW);
        m_yPostComplete = TRUE;
    }
}

SnBool CFactoryScreen::WaitForOK(void)
{
    m_csMiddleLabel = TEXT("OK"); 
    m_BtnMiddle.ShowWindow(SW_SHOW);
    AddInstructions(TEXT("\r\rPress the OK button to continue."));
    m_yMiddlePressed = FALSE;
    while (!m_yKillThreads && !m_yMiddlePressed) {
        if (m_yTemperatureTest) {
            DisplayTemperatures();
        }
        if (Delay(100) == FALSE) {
            return FALSE;
        }
	}
    m_BtnMiddle.ShowWindow(SW_HIDE);
    if (m_yKillThreads) {
        return FALSE;
    } else {
        return TRUE;
    }
}

SnBool CFactoryScreen::WaitForPassFail(void)
{
    AddInstructions(TEXT("\r\rPress the PASS or FAIL button to continue."));

    m_csMiddleLabel = TEXT("PASS"); 
    m_csRightLabel = TEXT("FAIL"); 
    m_BtnMiddle.ShowWindow(SW_SHOW);
    m_BtnRight.ShowWindow(SW_SHOW);

    m_yMiddlePressed = FALSE;
    m_yRightPressed = FALSE;
    while (!m_yKillThreads && !m_yMiddlePressed && !m_yRightPressed) {
        if (Delay(100) == FALSE) {
            return FALSE;
        }
	}

    m_BtnMiddle.ShowWindow(SW_HIDE);
    m_BtnRight.ShowWindow(SW_HIDE);

    if (m_yKillThreads) {
        return FALSE;
    } else {
        return TRUE;
    }
}

SnBool CFactoryScreen::WaitForChangePassFail(void)
{
    AddInstructions(TEXT("\r\rPress the CHANGE, PASS or FAIL button to continue."));

    m_csMiddleLabel = TEXT("PASS"); 
    m_csRightLabel = TEXT("FAIL"); 
    m_csSetLabel = TEXT("CHANGE"); 
    m_BtnMiddle.ShowWindow(SW_SHOW);
    m_BtnRight.ShowWindow(SW_SHOW);
    m_BtnSet.ShowWindow(SW_SHOW);

    m_yMiddlePressed = FALSE;
    m_yRightPressed = FALSE;
    m_ySetPressed = FALSE;
    while (!m_yKillThreads && !m_ySetPressed && !m_yMiddlePressed && !m_yRightPressed) {
        if (Delay(100) == FALSE) {
            return FALSE;
        }
	}

    m_BtnMiddle.ShowWindow(SW_HIDE);
    m_BtnRight.ShowWindow(SW_HIDE);
    m_BtnSet.ShowWindow(SW_HIDE);

    if (m_yKillThreads) {
        return FALSE;
    } else {
        return TRUE;
    }
}

DWORD WINAPI FactoryStatusThread(LPVOID pParam)
{
	CFactoryScreen *pClass = (CFactoryScreen*)pParam;

	pClass->m_yStatusThreadKilled = FALSE; // Reset the ThreadKilled flag

    // First wait for POST to finnish
	while (!pClass->m_yKillThreads && !pClass->m_yPostComplete) {
        pClass->CheckForPOST();
        if (pClass->Delay(100) == FALSE) {
            return(1);
        }
	}

    // Next wait for a test button to be pressed
	while (!pClass->m_yKillThreads && pClass->m_wTestMode == NO_TEST) {
        if (pClass->Delay(100) == FALSE) {
            return(1);
        }
	}

    // Run the tests
    pClass->PerformTests();

    // Set the Thread Killed Event
	SetEvent(pClass->m_hStatusThreadKilledEvent);
	return(0);
}

HANDLE CFactoryScreen::SetupSerialPort(TCHAR *pwPort, DWORD dwBaudRate)
{
    HANDLE hCommPort;

    //
    // Setup COM port for no HW/SW handshake, COMM_BAUD baud, 8 bit, no parity
    // with 1 stop bit
    //
    static DCB tDCB = {
        sizeof(DCB),            // DCBlength
        0,                      // BaudRate
        TRUE,                   // fBinary - Binary Mode (Always TRUE)
        FALSE,                  // fParity - No Parity check
        FALSE,                  // fOutxCtsFlow - No CTS control
        FALSE,                  // fOutxDsrFlow - No DSR control
        DTR_CONTROL_DISABLE,    // fDtrControl - No DTR control
        FALSE,                  // fDsrSensitivity - No DSR sensitivity
        TRUE,                   // fTXContinueOnXoff - NA
        FALSE,                  // fOutX - No XOFF flow control
        FALSE,                  // fInX - No XON flow control
        FALSE,                  // fErrorChar - No parity error replacement
        FALSE,                  // fNull - Null bytes are not discarded
        RTS_CONTROL_DISABLE,    // fRtsControl - No RTS control
        FALSE,                  // fAbortOnError - Error does not stop read/writes
        0,                      // fDummy2 - NA
        0,                      // wReserved - NA
        0,                      // XonLim - NA
        0,                      // XoffLim - NA
        8,                      // ByteSize
        NOPARITY,               // Parity
        ONESTOPBIT,             // StopBits
        0,                      // XonChar - NA
        1,                      // XoffChar - NA
        0,                      // ErrorChar - NA
        0,                      // EofChar - NA
        0,                      // EvtChar - NA
        0                       // wReserved1
    };

    //
    // Disable all timeouts for COM port
    //
    static COMMTIMEOUTS tCommTimeouts = {
        0,          // ReadIntervalTimeout = 0
        50,         // ReadTotalTimeoutMultiplier = 50
        0,          // ReadTotalTimeoutConstant = 0
        50,          // WriteTotalTimeoutMultiplier = 50
        0           // WriteTotalTimeoutConstant = 0
    };

    tDCB.BaudRate = dwBaudRate;

    hCommPort = CreateFile(pwPort, GENERIC_READ|GENERIC_WRITE,0, NULL, OPEN_EXISTING, 0, NULL);

    if(hCommPort == INVALID_HANDLE_VALUE) {
        return NULL;
    }
    if((SetCommState(hCommPort, &tDCB) == FALSE) || (SetCommTimeouts(hCommPort, &tCommTimeouts) == FALSE)) {
        CloseHandle(hCommPort);
        return NULL;
    }

    return hCommPort;
}

SnBool CFactoryScreen::TestSerialPortLoopback(TCHAR *pwPort, DWORD dwBaudRate)
{
    HANDLE hCommPort;
    SnByte bXmtByte;
    SnByte bRcvByte;
    DWORD dwRet;
	int iCount;

    hCommPort = SetupSerialPort(pwPort, dwBaudRate);
    if(hCommPort == NULL) {
        return FALSE;
    }

    // Flush out any characters in the buffer
    do {
        ReadFile(hCommPort, &bRcvByte, 1, &dwRet, NULL);
    } while (dwRet == 1);

    // Test every possible character (a bit of overkill...)
    for(iCount = 0; iCount < 256; iCount++) {
		bXmtByte = (SnByte)iCount;
        if ((WriteFile(hCommPort, &bXmtByte, 1, &dwRet, NULL) == FALSE) || (dwRet != 1)) {
            CloseHandle(hCommPort);
            return FALSE;
        }

        if ((ReadFile(hCommPort, &bRcvByte, 1, &dwRet, NULL) == FALSE) || (dwRet != 1) || (bRcvByte != bXmtByte)) {
            CloseHandle(hCommPort);
            return FALSE;
        }
    }

    // Close down the serial port
    CloseHandle(hCommPort);

    return TRUE;
}

void CFactoryScreen::StartNewTest(CString csTest)
{
    m_csInstructionList = SN_CLEAR_TEXT;
    m_csTestList += csTest;
    SetDlgItemText(IDC_EDIT_TEST_LIST_TEXT, m_csTestList);
}

void CFactoryScreen::ReportTestPassed(void)
{
    m_csTestList += TEXT("PASSED\r\n");
    SetDlgItemText(IDC_EDIT_TEST_LIST_TEXT, m_csTestList);
	m_yRepeatTest = FALSE;
}

void CFactoryScreen::ReportTestFailed(CString csError)
{
	CString csTempList = m_csTestList;
	int iGrowBottom = 0;
	RECT tWindowRect;
	CDC *ptCDC;

    m_csInstructionList = csError;
    m_csTestList += TEXT("FAILED\r\n");
    SetDlgItemText(IDC_EDIT_TEST_LIST_TEXT, m_csTestList);
    AddInstructions(TEXT("\rPress OK to continue or Repeat to re-run the test."));

	// Figure out if the Window needs to grow to display all of the text.
	// If so set iGrowBottom to the amount the Window needs to grow.
	// The Max bottom is 350, anything more than that it will cover the buttons.
	ptCDC = m_StaticInstructionsText.GetDC();
	if (ptCDC) {
		RECT tClipRect, tTextRect;
		CFont *ptOldFont, *ptNewFont;

		ptNewFont = m_StaticInstructionsText.GetFont();
		ptOldFont = ptCDC->SelectObject(ptNewFont);
		ptCDC->GetClipBox(&tClipRect);
		tTextRect = tClipRect;
		tTextRect.bottom = 350;
		ptCDC->DrawText(m_csInstructionList, &tTextRect, DT_WORDBREAK | DT_CALCRECT);
		ptCDC->SelectObject(ptOldFont);
	    m_StaticInstructionsText.ReleaseDC(ptCDC);

		if (tTextRect.bottom > tClipRect.bottom) {
			iGrowBottom = tTextRect.bottom - tClipRect.bottom;
			m_StaticInstructionsText.GetWindowRect(&tWindowRect);
		}
	}

	// Grow the Window bottom the needed amount and re-display it.
	if (iGrowBottom > 0) {
		RECT tNewRect = tWindowRect;

		tNewRect.bottom += iGrowBottom;
		m_StaticInstructionsText.SetWindowPos(&wndTop, tNewRect.left, tNewRect.top,
			(tNewRect.right - tNewRect.left),
			(tNewRect.bottom - tNewRect.top), SWP_SHOWWINDOW);
	}
    SetDlgItemText(IDC_STATIC_INSTRUCTIONS_TEXT, m_csInstructionList);

    m_csMiddleLabel = TEXT("OK"); 
    m_csRightLabel = TEXT("Repeat"); 
    m_BtnMiddle.ShowWindow(SW_SHOW);
    m_BtnRight.ShowWindow(SW_SHOW);
    m_yMiddlePressed = FALSE;
    m_yRightPressed = FALSE;
    while (!m_yKillThreads && !m_yMiddlePressed && !m_yRightPressed) {
        if (Delay(100) == FALSE) {
            return;
        }
	}
	if (m_yRightPressed) {
		m_csTestList = csTempList;
		SetDlgItemText(IDC_EDIT_TEST_LIST_TEXT, m_csTestList);
		m_yRepeatTest = TRUE;
	} else {
		m_yFailedTest = TRUE;
		m_yRepeatTest = FALSE;
	}

	// Restore the Window Size
	if (iGrowBottom > 0) {
		m_StaticInstructionsText.SetWindowPos(&wndTop, tWindowRect.left, tWindowRect.top,
			(tWindowRect.right - tWindowRect.left),
			(tWindowRect.bottom - tWindowRect.top), SWP_SHOWWINDOW);
	}

    m_BtnMiddle.ShowWindow(SW_HIDE);
    m_BtnRight.ShowWindow(SW_HIDE);
}

void CFactoryScreen::AddInstructions(CString csIntructions)
{
    m_csInstructionList += csIntructions;
    SetDlgItemText(IDC_STATIC_INSTRUCTIONS_TEXT, m_csInstructionList);
}

void CFactoryScreen::NewInstructions(CString csIntructions)
{
    m_csInstructionList = csIntructions;
    SetDlgItemText(IDC_STATIC_INSTRUCTIONS_TEXT, m_csInstructionList);
}

SnBool CFactoryScreen::SerialNumberTest(void)
{
    char pcSerialNumber[SERIAL_NUMBER_STORE];
    CString csLine;

    StartNewTest(TEXT("Serial Number...\t"));

    // Force the entry of the Serial Number if it is not set
    m_pControl->GetSerialNumber(pcSerialNumber);
    if (pcSerialNumber[0] == 0) {
        CSerialNumberPopUp dlg(m_pControl);
	    dlg.DoModal();
    }

    do {
        // Verify the Serial Number and allow it to be changed
        do {
            m_pControl->GetSerialNumber(pcSerialNumber);

            csLine = TEXT("The Serial Number for this unit is: ");
            csLine += pcSerialNumber;
		    NewInstructions(csLine);
 		    AddInstructions(TEXT("\r\rVerify the Serial Number against the back label."));

            if (WaitForChangePassFail() == FALSE) {
			    return FALSE;
		    }

            if (m_ySetPressed) {
                CSerialNumberPopUp dlg(m_pControl);
	            dlg.DoModal();
            }
        } while(m_ySetPressed);

        if (m_yMiddlePressed) {
            ReportTestPassed();
        } else {
            ReportTestFailed(TEXT("Serial Number Check Failed\r"));
        }
    } while (m_yRepeatTest);
 

    return TRUE;
}

SnBool CFactoryScreen::NvRamTest(void)
{
    SnWord wBattery;

    // Nv Ram Test
    StartNewTest(TEXT("Nv Ram...\t\t"));

	// Get the battery status
	m_pControl->GetCmdState(GET_SYSTEM_BATTERY_STATUS, &wBattery, sizeof(SnWord));

    m_yNvRamTest = TRUE;

    // Check for date not being set
    if (wBattery == BATTERY_FAILED) {
		do {
			NewInstructions(TEXT("The NvRam has been reset. If this is the first time "));
			AddInstructions(TEXT("this test has been run then you should power down the sytem. "));
            AddInstructions(TEXT("Wait for 5 seconds and power it on again."));
			AddInstructions(TEXT("\r\rOtherwise press the FAIL button continue."));

			m_csRightLabel = TEXT("FAIL"); 
			m_BtnRight.ShowWindow(SW_SHOW);

			m_yRightPressed = FALSE;
			while (!m_yKillThreads && !m_yRightPressed) {
				if (Delay(100) == FALSE) {
					return FALSE;
				}
			}

			m_BtnRight.ShowWindow(SW_HIDE);

			if (m_yKillThreads) {
				return FALSE;
			}

			ReportTestFailed(TEXT("NvRam Check Failed\r"));
		} while (m_yRepeatTest);
    } else 
	    ReportTestPassed();

    m_yNvRamTest = FALSE;

    return TRUE;
}

SnBool CFactoryScreen::SerialPortTest(void)
{
    // Intellio Link, Pump and Wireless Serial Port Test
    StartNewTest(TEXT("Serial Ports...\t"));

	do {
		CString csErrors = SN_CLEAR_TEXT;
		SnBool ySerialPortFailed = FALSE;

		NewInstructions(TEXT("Make sure the Intellio Link, Pump and Wireless test connectors are in place."));
		if (WaitForOK() == FALSE) {
			return FALSE;
		}

		if (TestSerialPortLoopback(TEXT("COM3:"), CBR_115200) == FALSE) {
			csErrors += TEXT("Intellio Link Port Test Failed\r");
			ySerialPortFailed = TRUE;
		}
		if (TestSerialPortLoopback(TEXT("COM2:"), CBR_115200) == FALSE) {
			csErrors += TEXT("Pump Port Test Failed\r");
			ySerialPortFailed = TRUE;
		}
		if (TestSerialPortLoopback(TEXT("COM4:"), CBR_115200) == FALSE) {
			csErrors += TEXT("Wireless Port Test Failed\r");
			ySerialPortFailed = TRUE;
		}
		if (ySerialPortFailed) {
			ReportTestFailed(csErrors);
		} else {
			ReportTestPassed();
		}
	} while (m_yRepeatTest);

    return TRUE;
}

void CFactoryScreen::DisplayTemperatures(void)
{
 	static CString csOldTemp;
    CString csTemperature;
    SnBool yStatus;

    yStatus = m_pControl->GetTemperatures(&m_fBoardTemp, &m_fDspTemp);
    if (!yStatus) {
        return;
    }

	csTemperature.Format(_T("Board: %.1f\rDsp: %.1f"),
        m_fBoardTemp, m_fDspTemp);

    if (csTemperature != csOldTemp) {
        SetDlgItemText(IDC_STATIC_TEST_TEXT, csTemperature);
        csOldTemp = csTemperature;
    }
}

SnBool CFactoryScreen::TemperatureTest(void)
{
    // Dsp / On Board Temperature Test
    StartNewTest(TEXT("Temperature...\t"));

	do {
		CString csErrors = SN_CLEAR_TEXT;
		SnBool yTemperatureFailed = FALSE;
		CString csTemp;

		NewInstructions("About to verify that the system temperatures....");

		SetDlgItemText(IDC_STATIC_TEST_TITLE, TEXT("Temperature (C)"));
		SetDlgItemText(IDC_STATIC_TEST_TEXT, SN_CLEAR_TEXT);
		m_StaticTestTitle.ShowWindow(SW_SHOW);
		m_StaticTestText.ShowWindow(SW_SHOW);

		m_yTemperatureTest = TRUE;

		if (WaitForOK() == FALSE) {
			return FALSE;
		}

		m_yTemperatureTest = FALSE;

		m_StaticTestTitle.ShowWindow(SW_HIDE);
		m_StaticTestText.ShowWindow(SW_HIDE);

		if (m_fBoardTemp < 10.0f || m_fBoardTemp > 40.0f) {
			csTemp.Format(TEXT("Board Temperature {%.1f} out of spec\r"), m_fBoardTemp);
			csErrors += csTemp;
			yTemperatureFailed = TRUE;
		}
		if (m_fDspTemp < 30.0f || m_fDspTemp > 70.0f) {
			csTemp.Format(TEXT("Dsp Temperature {%.1f} out of spec\r"), m_fDspTemp);
			csErrors += csTemp;
			yTemperatureFailed = TRUE;
		}
		if (yTemperatureFailed) {
			ReportTestFailed(csErrors);
		} else {
			ReportTestPassed();
		}
	} while (m_yRepeatTest);

    return TRUE;
}

SnBool CFactoryScreen::BuzzerTest(void)
{
    // System Processor and Motor DSP Buzzer Test
    StartNewTest(TEXT("Buzzer...\t\t"));

	do {
		CString csErrors = SN_CLEAR_TEXT;
		SnBool yBuzzerFailed = FALSE;
		SnWord wBeeper;

		NewInstructions(TEXT("Verify you hear the System Processor Buzzer."));

		// Turn ON the Beeper
		wBeeper = BEEPER_ON;
		m_pControl->SetCmdState(SET_SYSTEM_BEEPER, &wBeeper, sizeof(SnWord));

		if (Delay(500) == FALSE) {
			return FALSE;
		}

		// Turn OFF the Beeper
		wBeeper = BEEPER_OFF;
		m_pControl->SetCmdState(SET_SYSTEM_BEEPER, &wBeeper, sizeof(SnWord));

		if (WaitForPassFail() == FALSE) {
			return FALSE;
		}
		if (m_yRightPressed) {
			csErrors += TEXT("Buzzer Failed System Processor\r");
			yBuzzerFailed = TRUE;
		}

		NewInstructions(TEXT("Verify you hear the Motor DSP Buzzer."));

		// Turn ON the Beeper
		wBeeper = BEEPER_ON;
		m_pControl->SetCmdState(SET_MC_BEEPER, &wBeeper, sizeof(SnWord));

		if (Delay(500) == FALSE) {
			return FALSE;
		}

		// Turn OFF the Beeper
		wBeeper = BEEPER_OFF;
		m_pControl->SetCmdState(SET_MC_BEEPER, &wBeeper, sizeof(SnWord));

		if (WaitForPassFail() == FALSE) {
			return FALSE;
		}
		if (m_yRightPressed) {
			csErrors += TEXT("Buzzer Failed Motor DSP\r");
			yBuzzerFailed = TRUE;
		}

		if (yBuzzerFailed) {
			ReportTestFailed(csErrors);
		} else {
			ReportTestPassed();
		}
	} while (m_yRepeatTest);

    return TRUE;
}

SnBool CFactoryScreen::UsbTest(void)
{
    // Host USB Test (Both Slots)
    StartNewTest(TEXT("USB...\t\t"));

	do {
		DWORD dwAttributes;

		NewInstructions(TEXT("Make sure there is a Factory Mode USB key in both USB slots "));
		AddInstructions(TEXT("or a Service Mode USB Key in both USB slots."));
		if (WaitForOK() == FALSE) {
			return FALSE;
		}

		dwAttributes = GetFileAttributes(_T("\\Hard Disk2\\FactoryMode.txt"));
		if (dwAttributes == 0xFFFFFFFF) {
			dwAttributes = GetFileAttributes(_T("\\Hard Disk2\\ServiceMode.txt"));
			if (dwAttributes == 0xFFFFFFFF) {
				ReportTestFailed(TEXT("USB Test Failed\r"));
			} else {
				ReportTestPassed();
			}
		} else {
			ReportTestPassed();
		}
	} while (m_yRepeatTest);

    return TRUE;
}

SnBool CFactoryScreen::DisplayTest(void)
{
    CDC* pCDC;
    RECT tWindow;
    RECT tClient;
    int iStartX;
    int iRedStartY;
    int iGreenStartY;
    int iBlueStartY;
    int iBarHeight;
    int iWidth;
    int iHeight;
    int iColor;
    int iX;

    // Display Test
    StartNewTest(TEXT("Display...\t\t"));

    SetDlgItemText(IDC_STATIC_TEST_TITLE, TEXT("Display Test"));

    m_StaticTestText.GetWindowRect(&tWindow);
    m_StaticTestText.GetClientRect(&tClient);
    iWidth = tClient.right;
    iHeight = tClient.bottom;
    if (tClient.right < 256) {
        return FALSE;
    }

    iStartX = tWindow.left + ((tWindow.right - tWindow.left - 256) / 2);
    iBarHeight = (tClient.bottom / 3);
    iRedStartY = tWindow.top + ((tWindow.bottom - tWindow.top - (iBarHeight * 3)) / 2);
    iGreenStartY = iRedStartY + iBarHeight;
    iBlueStartY = iGreenStartY + iBarHeight;

    pCDC = GetDC();

    do {
        m_StaticTestTitle.ShowWindow(SW_SHOW);

        pCDC->FillSolidRect(&tWindow, RGB(255, 255, 255));

        for (iX = iStartX, iColor = 0; iX < (iStartX + 256); iX++, iColor++) {
            pCDC->FillSolidRect(iX, iRedStartY, 1, iBarHeight, RGB(iColor, 0, 0));
            pCDC->FillSolidRect(iX, iGreenStartY, 1, iBarHeight, RGB(0, iColor, 0));
            pCDC->FillSolidRect(iX, iBlueStartY, 1, iBarHeight, RGB(0, 0, iColor));
        }

		NewInstructions(TEXT("Verify that the Red/Green/Blue ramps below appear correct."));
		if (WaitForPassFail() == FALSE) {
			return FALSE;
		}

		m_StaticTestTitle.ShowWindow(SW_HIDE);
		pCDC->FillSolidRect(&tWindow, RGB(0, 0, 0));

		if (m_yRightPressed) {
			ReportTestFailed(TEXT("Display Test Failed\r"));
		} else {
			ReportTestPassed();
		}
	} while (m_yRepeatTest);

    ReleaseDC(pCDC);

    return TRUE;
}


SnBool CFactoryScreen::HandpiecePhaseTest(int iPort, SnQByte *pqError)
{
    const SnWord pwSingleFETs[6] = {
        0xBE00,                 // Turn on FET A Top
        0xBD00,                 // Turn on FET A Bottom
        0xBB00,                 // Turn on FET B Top
        0xB700,                 // Turn on FET B Bottom
        0xAF00,                 // Turn on FET C Top
        0x9F00                  // Turn on FET C Bottom
    };
    const SnWord pwDualFETs[6] = {
        0x9F01,                 // Turn on FET pair A-C
        0xBD04,                 // Turn on FET pair B-A
        0x9F04,                 // Turn on FET pair B-C
        0xB710,                 // Turn on FET pair C-B
        0xB701,                 // Turn on FET pair A-B
        0xBD10                  // Turn on FET pair C-A
    };
    SnBool yStatus;
    SnBool yRet = FALSE;
    SnWord wTmp;
    float fTmp;
    int iCnt = 0;
	SnWord wNewMotorStatus = (iPort == PORTA) ? 0x0080: 0x8000;

    // Configure both ports for no motor
    wTmp = wNewMotorStatus;
	yStatus = m_pControl->WriteDsp(MC_PORT_TYPE, 1, &wTmp);
    if (!yStatus) {
        if (pqError)
            *pqError = HAND_TEST_DSP;
		goto PhaseTestError;
    }

	do {
		Sleep(1);	// Allow time for the DSP to respond to the write
		yStatus = m_pControl->ReadDsp(MC_PORT_TYPE, 1, &wTmp);
		iCnt++;
		if (iCnt > 5)
		{
			// DSP failed to clear the value indicating success
			yStatus = FALSE;
		}
	}while (yStatus && (wTmp & wNewMotorStatus));
	
    if (!yStatus) {
        if (pqError)
            *pqError = HAND_TEST_DSP;
		goto PhaseTestError;
    }

    // Start (also runs during boot)
    wTmp = 8;
	yStatus = m_pControl->WriteDsp(MC_BLDCX_MODE(iPort), 1, &wTmp);
    if (!yStatus) {
        if (pqError)
            *pqError = HAND_TEST_DSP;
		goto PhaseTestError;
    }

    // Wait for done
    do {
        if (Delay(300) == FALSE) {
		    goto PhaseTestError;
        }
	    yStatus = m_pControl->ReadDsp(MC_BLDCX_MODE(iPort), 1, &wTmp);
        if (!yStatus) {
            if (pqError)
                *pqError = HAND_TEST_DSP;
		    goto PhaseTestError;
        }
    } while (wTmp != 0 && !m_yKillThreads);

	yStatus = m_pControl->ReadDsp(MC_BLDCX_FAULT(iPort), 1, &wTmp);
    if (!yStatus) {
        if (pqError)
            *pqError = HAND_TEST_DSP;
		goto PhaseTestError;
    }

    // Failed with port disabled, further phase testing prevented
    // 24V Short to Ground
    if (wTmp & 0x0040) {
        if (pqError)
            *pqError = HAND_TEST_24V_GND;
		goto PhaseTestError;
    }

    // Configure PID loop to apply power

    // Set 2 amp DSP current limit
    fTmp = 2.0f;
	yStatus = m_pControl->WriteDsp(MC_BLDCX_ILIM(iPort), 2, (SnWord *)&fTmp);
    if (!yStatus) {
        if (pqError)
            *pqError = HAND_TEST_DSP;
		goto PhaseTestError;
    }
    // Scaling variable
    fTmp = 1.0f;
	yStatus = m_pControl->WriteDsp(MC_BLDCX_FRPMA(iPort), 2, (SnWord *)&fTmp);
    if (!yStatus) {
        if (pqError)
            *pqError = HAND_TEST_DSP;
		goto PhaseTestError;
    }
    // Scaling variable
    fTmp = 1.0f;
	yStatus = m_pControl->WriteDsp(MC_BLDCX_KF(iPort), 2, (SnWord *)&fTmp);
    if (!yStatus) {
        if (pqError)
            *pqError = HAND_TEST_DSP;
		goto PhaseTestError;
    }
    // 100% pwm duty cycle
    wTmp = 1200;
	yStatus = m_pControl->WriteDsp(MC_BLDCX_TLIMB(iPort), 1, &wTmp);
    if (!yStatus) {
        if (pqError)
            *pqError = HAND_TEST_DSP;
		goto PhaseTestError;
    }
    // 30 TAC changes per revolution
    wTmp = 30;
	yStatus = m_pControl->WriteDsp(MC_BLDCX_TACPERREVN(iPort), 1, &wTmp);
    if (!yStatus) {
        if (pqError)
            *pqError = HAND_TEST_DSP;
		goto PhaseTestError;
    }
    wTmp = 1;
	yStatus = m_pControl->WriteDsp(MC_BLDCX_TACPERREVD(iPort), 1, &wTmp);
    if (!yStatus) {
        if (pqError)
            *pqError = HAND_TEST_DSP;
		goto PhaseTestError;
    }
    // 20000 acceleration
    fTmp = 20000.0f;
	yStatus = m_pControl->WriteDsp(MC_BLDCX_ACCEL(iPort), 2, (SnWord *)&fTmp);
    if (!yStatus) {
        if (pqError)
            *pqError = HAND_TEST_DSP;
		goto PhaseTestError;
    }
    // 10000 rpm max allowed speed
    wTmp = 10000;
	yStatus = m_pControl->WriteDsp(MC_BLDCX_VELMAX(iPort), 1, &wTmp);
    if (!yStatus) {
        if (pqError)
            *pqError = HAND_TEST_DSP;
		goto PhaseTestError;
    }
    // 1200 rpm target speed
    wTmp = 1200;
	yStatus = m_pControl->WriteDsp(MC_BLDCX_VELOCITY_SET(iPort), 1, &wTmp);
    if (!yStatus) {
        if (pqError)
            *pqError = HAND_TEST_DSP;
		goto PhaseTestError;
    }

	// Configure both port for brushless motors
	wTmp = wNewMotorStatus;
    wTmp |= (iPort == PORTA) ? 0x0002: 0x0200;

	yStatus = m_pControl->WriteDsp(MC_PORT_TYPE, 1, &wTmp);
    if (!yStatus) {
        if (pqError)
            *pqError = HAND_TEST_DSP;
		goto PhaseTestError;
    }

	iCnt = 0;
	while (yStatus && iCnt < 5)	{
		Sleep(1);	// Allow time for the DSP to respond to the write
		yStatus = m_pControl->ReadDsp(MC_PORT_TYPE, 1, &wTmp);
		iCnt++;
		if (!(wTmp & wNewMotorStatus)) {
			// Motor cleared the value indicating success
			iCnt = 20;
		}
	}
	
	if (iCnt !=20) {
        if (pqError)
			*pqError = HAND_TEST_DSP;
		goto PhaseTestError;
	}

    //
    // Turn on each FET and check for current.
    // If current present, then there is a short to ground.
    // If current limit is set and there is no current present then the current limit line is bad.
    //
    for (iCnt = 0; iCnt < 6; iCnt++) {
        // Load next FET pattern
	    yStatus = m_pControl->WriteDsp(MC_BLDCX_FCOMMTABLE7(iPort), 1, (SnWord *)&pwSingleFETs[iCnt]);
        if (!yStatus) {
            if (pqError)
                *pqError = HAND_TEST_DSP;
		    goto PhaseTestError;
        }
        // Turn on PID loop
        wTmp = 2;
	    yStatus = m_pControl->WriteDsp(MC_BLDCX_MODE(iPort), 1, &wTmp);
        if (!yStatus) {
            if (pqError)
                *pqError = HAND_TEST_DSP;
		    goto PhaseTestError;
        }

        // Allow PID to ramp up
        if (Delay(300) == FALSE) {
		    goto PhaseTestError;
        }

        // Check for current, there should be none
        yStatus = m_pControl->ReadDsp(MC_BLDCX_CURRENT(iPort), 2, (SnWord *)&fTmp);
        if (!yStatus) {
            if (pqError)
                *pqError = HAND_TEST_DSP;
		    goto PhaseTestError;
        }
        // Error - Current detected
        if (fTmp >= 0.5f) {
            if (pqError)
                *pqError = HAND_TEST_FET_A_TOP_GND << iCnt;
		    goto PhaseTestError;
        }

        // No current detected, so double check the current limit fault line
        yStatus = m_pControl->ReadDsp(MC_BLDCX_FAULT(iPort), 1, &wTmp);
        if (!yStatus) {
            if (pqError)
                *pqError = HAND_TEST_DSP;
		    goto PhaseTestError;
        }
        // Error - Current Limit detected, but no current detected
        if (wTmp & 0x0010) {
            if (pqError)
                *pqError = HAND_TEST_CURRENT_OR_LIMIT;
		    goto PhaseTestError;
        }

        // Turn off PID loop
        wTmp = 0;
	    yStatus = m_pControl->WriteDsp(MC_BLDCX_MODE(iPort), 1, &wTmp);
        if (!yStatus) {
            if (pqError)
                *pqError = HAND_TEST_DSP;
		    goto PhaseTestError;
        }
    }

    //
    // Turn two FETs at a time and check for current limit to kick in.
    // If current is not present then there is an open.
    // If the current is around 2 amp but the current limit flag is not set, then the current limit
    // line is open.
    //

    for (iCnt = 0; iCnt < 6; iCnt++) {
        // Load next FET pattern
	    yStatus = m_pControl->WriteDsp(MC_BLDCX_FCOMMTABLE7(iPort), 1, (SnWord *)&pwDualFETs[iCnt]);
        if (!yStatus) {
            if (pqError)
                *pqError = HAND_TEST_DSP;
		    goto PhaseTestError;
        }

        // Turn on PID loop
        wTmp = 2;
	    yStatus = m_pControl->WriteDsp(MC_BLDCX_MODE(iPort), 1, &wTmp);
        if (!yStatus) {
            if (pqError)
                *pqError = HAND_TEST_DSP;
		    goto PhaseTestError;
        }

        // Allow PID to ramp up
        if (Delay(300) == FALSE) {
		    goto PhaseTestError;
        }

        // Check for current, it should be greater than or equal to 0.5 amps
        yStatus = m_pControl->ReadDsp(MC_BLDCX_CURRENT(iPort), 2, (SnWord *)&fTmp);
        if (!yStatus) {
            if (pqError)
                *pqError = HAND_TEST_DSP;
		    goto PhaseTestError;
        }

        // Error - No Current detected
        if (fTmp < 0.5f) {
            if (pqError)
                *pqError = HAND_TEST_FET_A_C_OPEN << iCnt;
		    goto PhaseTestError;
        }

        // Check for Current Limit fault
        yStatus = m_pControl->ReadDsp(MC_BLDCX_FAULT(iPort), 1, &wTmp);
        if (!yStatus) {
            if (pqError)
                *pqError = HAND_TEST_DSP;
		    goto PhaseTestError;
        }

        // Error - Current present, but no Current Limit detected
        if ((wTmp & ERROR_MOTOR_ILIM) == 0) {
            if (pqError)
                *pqError = HAND_TEST_CURRENT_OR_LIMIT;
		    goto PhaseTestError;
        }

        // Turn off PID loop
        wTmp = 0;
	    yStatus = m_pControl->WriteDsp(MC_BLDCX_MODE(iPort), 1, &wTmp);
        if (!yStatus) {
            if (pqError)
                *pqError = HAND_TEST_DSP;
		    goto PhaseTestError;
        }
    }

    yRet = TRUE;
    if (pqError)
        *pqError = 0;

PhaseTestError:

    // Turn all FETs off
    wTmp = 0x3f00;
	yStatus = m_pControl->WriteDsp(MC_BLDCX_FCOMMTABLE7(iPort), 1, &wTmp);
	if (!yStatus)
		yRet = FALSE;

    // Turn off PID loop
    wTmp = 0;
	yStatus = m_pControl->WriteDsp(MC_BLDCX_MODE(iPort), 1, &wTmp);
	if (!yStatus)
		yRet = FALSE;

    return yRet;
}

SnBool CFactoryScreen::HandpieceTest(int iPort)
{
    CString csConnector;
    SnQByte qHandpieceError;
    SnWord wType, wMode;
    SnBool yStatus;

    if (iPort == HAND_PORTA) {
        StartNewTest(TEXT("Handpiece A...\t"));
        csConnector = TEXT("port A connector");
    } else {
		iPort = HAND_PORTB;
        StartNewTest(TEXT("Handpiece B...\t"));
        csConnector = TEXT("port B connector");
    } 

	do {
		CString csErrors = SN_CLEAR_TEXT;

		m_qHandpieceError = 0;
		qHandpieceError = 0;


		m_csRightLabel = TEXT("FAIL");

		NewInstructions(TEXT("Make sure a DYONICS POWER II Footswitch (72201092) is plugged into the Test Fixture (20600579) "));
		AddInstructions(TEXT("and plug the Test Fixture handpiece cable into the "));
		AddInstructions(csConnector);
		AddInstructions(TEXT(". "));

		if (WaitForOK() == FALSE) {
			return FALSE;
		}

		wType = (iPort == HAND_PORTA) ? m_pControl->GetPortAType() : m_pControl->GetPortBType();
		if (wType != TYPE_MDU_TEST) {
			yStatus = m_pControl->ReadDsp(MC_DIGITAL_STATE_DATA_B, 1, &wType);
			if (!yStatus) {
				qHandpieceError |= HAND_TEST_DSP;
			} else {
				if (iPort == HAND_PORTA) {
					wType = wType & 0x7;
				} else {
					wType = (wType >> 3) & 0x7;
				}
				if ((wType & 2) == 0) {
					qHandpieceError |= HAND_TEST_LOGIC_1;
				}
				if ((wType & 4) == 0) {
					qHandpieceError |= HAND_TEST_LOGIC_2;
				}
				if (wType == LOGIC_ID_RS485) {
					qHandpieceError |= HAND_TEST_RS485;
				}
			}
			goto HandpieceTestEnd;
		}

		if (HandpiecePhaseTest(iPort, &qHandpieceError) == FALSE) {
			goto HandpieceTestEnd;
		}

        NewInstructions(TEXT("Plug a PowerMax Elite MDU into the "));
		AddInstructions(csConnector);
		AddInstructions(TEXT(". "));

		if (WaitForOK() == FALSE) {
			return FALSE;
		}

		wType = (iPort == HAND_PORTA) ? m_pControl->GetPortAType() : m_pControl->GetPortBType();
		if (wType != TYPE_MDU_STANDARD && wType != TYPE_MDU_STANDARD_CTL) {
			yStatus = m_pControl->ReadDsp(MC_DIGITAL_STATE_DATA_B, 1, &wType);
			if (!yStatus) {
				qHandpieceError |= HAND_TEST_DSP;
			} else {
				if (iPort == HAND_PORTA) {
					wType = wType & 0x7;
				} else {
					wType = (wType >> 3) & 0x7;
				}
				if ((wType & 1) == 0) {
					qHandpieceError |= HAND_TEST_LOGIC_0;
				}
			}

			yStatus = m_pControl->ReadDsp(MC_HALLBUSX_DEVICE_EXIST(iPort), 1, &wType);
			if (!yStatus) {
				qHandpieceError |= HAND_TEST_DSP;
			} else {
				if ((wType & HALL_MDU_MASK) != HALL_MDU_STANDARD_CTL) {
					qHandpieceError |= HAND_TEST_HALLBUS;
				}
			}
		}

		NewInstructions(TEXT("Plug the Drill into the "));
		AddInstructions(csConnector);
		AddInstructions(TEXT(". "));

		if (WaitForOK() == FALSE) {
			return FALSE;
		}

		wType = (iPort == HAND_PORTA) ? m_pControl->GetPortAType() : m_pControl->GetPortBType();
		if (wType != TYPE_DP_DRILL) {
			qHandpieceError |= HAND_TEST_LOGIC_2;
		} else {
			SnBool yDrillDirFailed = FALSE;
			SnBool yDrillSpeedFailed = FALSE;

			m_yRightPressed = FALSE;
			NewInstructions(TEXT("Turn the direction switch on the bottom of the drill to REVERSE. "));
			AddInstructions(TEXT("Press the FAIL button if the direction switch is set to REVERSE."));
			m_BtnRight.ShowWindow(SW_SHOW);
			do {
				if (Delay(300) == FALSE) {
					return FALSE;
				}
				wMode = (iPort == HAND_PORTA) ? m_pControl->GetPortADeviceMode() : m_pControl->GetPortBDeviceMode();
			} while (!m_yKillThreads && wMode != MOTOR_REVERSE && !m_yRightPressed);
			if (m_yKillThreads) {
				return FALSE;
			}
			m_BtnRight.ShowWindow(SW_HIDE);

			if (m_yRightPressed) {
				yDrillDirFailed = TRUE;
			}

			m_yRightPressed = FALSE;
			NewInstructions(TEXT("Turn the direction switch on the bottom of the drill to FORWARD. "));
			AddInstructions(TEXT("Press the FAIL button if the direction switch is set to FORWARD."));
			m_BtnRight.ShowWindow(SW_SHOW);
			do {
				if (Delay(300) == FALSE) {
					return FALSE;
				}
				wMode = (iPort == HAND_PORTA) ? m_pControl->GetPortADeviceMode() : m_pControl->GetPortBDeviceMode();
			} while (!m_yKillThreads && wMode != MOTOR_FORWARD && !m_yRightPressed);
			if (m_yKillThreads) {
				return FALSE;
			}
			m_BtnRight.ShowWindow(SW_HIDE);

			if (m_yRightPressed) {
				yDrillDirFailed = TRUE;
			}

			if (yDrillDirFailed) {
				qHandpieceError |= HAND_TEST_DRILL_DIR;
			}

			if (iPort == HAND_PORTA) {
				m_pControl->SetPortAMaxPercent();
			} else {
				m_pControl->SetPortBMaxPercent();
			}

			m_yRightPressed = FALSE;
			NewInstructions(TEXT("Press and hold the trigger switch all the way in on the drill. "));
			AddInstructions(TEXT("Press the FAIL button if the trigger switch is all the way in."));
			m_BtnRight.ShowWindow(SW_SHOW);
			do {
				if (Delay(300) == FALSE) {
					return FALSE;
				}
				wMode = (iPort == HAND_PORTA) ? m_pControl->GetPortAVelocity() : m_pControl->GetPortBVelocity();
			} while (!m_yKillThreads && wMode < ((DP_DRILL_MAX_VELOCITY * 9) / 10) && !m_yRightPressed);
			if (m_yKillThreads) {
				return FALSE;
			}
			m_BtnRight.ShowWindow(SW_HIDE);

			if (m_yRightPressed) {
				yDrillSpeedFailed = TRUE;
			}

			m_yRightPressed = FALSE;
			NewInstructions(TEXT("Release the trigger switch on the Drill. "));
			AddInstructions(TEXT("Press the FAIL button if the trigger switch is released."));
			m_BtnRight.ShowWindow(SW_SHOW);
			do {
				if (Delay(100) == FALSE) {
					return FALSE;
				}
				wMode = (iPort == HAND_PORTA) ? m_pControl->GetPortAVelocity() : m_pControl->GetPortBVelocity();
			} while (!m_yKillThreads && wMode != 0 && !m_yRightPressed);
			if (m_yKillThreads) {
				return FALSE;
			}
			m_BtnRight.ShowWindow(SW_HIDE);

			if (m_yRightPressed) {
				yDrillSpeedFailed = TRUE;
			}

			if (yDrillSpeedFailed) {
				qHandpieceError |= HAND_TEST_DRILL_SPEED;
			}
		}

		NewInstructions(TEXT("Plug a Mini-Motor MDU into the "));
		AddInstructions(csConnector);
		AddInstructions(TEXT(". "));

		if (WaitForOK() == FALSE) {
			return FALSE;
		}

		wType = (iPort == HAND_PORTA) ? m_pControl->GetPortAType() : m_pControl->GetPortBType();
		if (wType != TYPE_MDU_MINI) {
			qHandpieceError |= HAND_TEST_LOGIC_1;
		}

		NewInstructions(TEXT("Plug a PowerMini MDU into the "));
		AddInstructions(csConnector);
		AddInstructions(TEXT(". "));

		if (WaitForOK() == FALSE) {
			return FALSE;
		}

		wType = (iPort == HAND_PORTA) ? m_pControl->GetPortAType() : m_pControl->GetPortBType();
		if (wType != TYPE_MDU_POWERMINI  && wType != TYPE_MDU_POWERMINI_CTL) {
			qHandpieceError |= HAND_TEST_RS485;
		}

HandpieceTestEnd:

		if (m_qHandpieceError) {
			if (m_qHandpieceError & (MOTORA_SHORT_CIRCUIT|MOTORA_SHORT_CIRCUIT_TIMEOUT|MOTORB_SHORT_CIRCUIT|MOTORB_SHORT_CIRCUIT_TIMEOUT)) {
				qHandpieceError |= HAND_TEST_24V_GND;
			}
			if (m_qHandpieceError & (MOTORA_TAC_FAULT|MOTORB_TAC_FAULT|MOTORA_STALL|MOTORB_STALL)) {
				qHandpieceError |= HAND_TEST_TACH;
			}
			if (m_qHandpieceError & (HALL_PATTERN_FAULT_PORTA|HALL_PATTERN_FAULT_PORTB)) {
				qHandpieceError |= HAND_TEST_HALLBUS;
			}
			if (m_qHandpieceError & (MOTORA_STALL_AND_CURRENT_LIMIT|MOTORB_STALL_AND_CURRENT_LIMIT|MOTORA_CURRENT_LIMIT|MOTORB_CURRENT_LIMIT)) {
				qHandpieceError |= HAND_TEST_PHASE;
			}
		}

		if (qHandpieceError) {
			if (iPort == HAND_PORTA) {
				csErrors = TEXT("Handpiece A test failed. ");
			} else {
				csErrors = TEXT("Handpiece B test failed. ");
			}
			if (m_wTestMode == BOARD_TEST) {
				csErrors += TEXT("Check the following:\r\r");
				if (qHandpieceError & HAND_TEST_DSP) {
					csErrors += TEXT("Interboard communication, ");
				}
				if (qHandpieceError & HAND_TEST_TACH) {
					csErrors += (iPort == HAND_PORTA) ? TEXT("A_TACH0, A_TACH1, A_TACH2, ") : TEXT("B_TACH0, B_TACH1, B_TACH2, ");
				}
				if (qHandpieceError & HAND_TEST_24V_GND) {
					csErrors += TEXT("24V to GND short, ");
				}
				if (qHandpieceError & HAND_TEST_FET_A_TOP_GND) {
					csErrors += TEXT("Phase A Top short, ");
				}
				if (qHandpieceError & HAND_TEST_FET_A_BOT_GND) {
					csErrors += TEXT("Phase A Bot short, ");
				}
				if (qHandpieceError & HAND_TEST_FET_B_TOP_GND) {
					csErrors += TEXT("Phase B Top short, ");
				}
				if (qHandpieceError & HAND_TEST_FET_B_BOT_GND) {
					csErrors += TEXT("Phase B Bot short, ");
				}
				if (qHandpieceError & HAND_TEST_FET_C_TOP_GND) {
					csErrors += TEXT("Phase C Top short, ");
				}
				if (qHandpieceError & HAND_TEST_FET_C_BOT_GND) {
					csErrors += TEXT("Phase C Bot short, ");
				}
				if (qHandpieceError & 0x0000fff8) {
					csErrors += (iPort == HAND_PORTA) ? TEXT("A_MotorCurrent, ") : TEXT("B_MotorCurrent, ");
				}
				if (qHandpieceError & HAND_TEST_CURRENT_OR_LIMIT) {
					csErrors += (iPort == HAND_PORTA) ? TEXT("A_MotorFault, ") : TEXT("B_MotorFault, ");
				}
				if (qHandpieceError & HAND_TEST_FET_A_C_OPEN) {
					csErrors += TEXT("Phase A-C open, ");
				}
				if (qHandpieceError & HAND_TEST_FET_B_A_OPEN) {
					csErrors += TEXT("Phase B-A open, ");
				}
				if (qHandpieceError & HAND_TEST_FET_B_C_OPEN) {
					csErrors += TEXT("Phase B-C open, ");
				}
				if (qHandpieceError & HAND_TEST_FET_C_B_OPEN) {
					csErrors += TEXT("Phase C-B open, ");
				}
				if (qHandpieceError & HAND_TEST_FET_A_B_OPEN) {
					csErrors += TEXT("Phase A-B open, ");
				}
				if (qHandpieceError & HAND_TEST_FET_C_A_OPEN) {
					csErrors += TEXT("Phase C-A open, ");
				}
				if (qHandpieceError & HAND_TEST_LOGIC_0) {
					csErrors += (iPort == HAND_PORTA) ? TEXT("A_Logic0, ") : TEXT("B_Logic0, ");
				}
				if (qHandpieceError & HAND_TEST_LOGIC_1) {
					csErrors += (iPort == HAND_PORTA) ? TEXT("A_Logic1, ") : TEXT("B_Logic1, ");
				}
				if (qHandpieceError & HAND_TEST_LOGIC_2) {
					csErrors += (iPort == HAND_PORTA) ? TEXT("A_Logic2, ") : TEXT("B_Logic2, ");
				}
				if (qHandpieceError & HAND_TEST_RS485) {
					csErrors += (iPort == HAND_PORTA) ? TEXT("A_TRCVR_LO, A_TRCVR_HI, ") : TEXT("B_TRCVR_LO, B_TRCVR_HI, ");
				}
				if (qHandpieceError & HAND_TEST_HALLBUS) {
					csErrors += (iPort == HAND_PORTA) ? TEXT("A_Hall_Bus, ") : TEXT("B_Hall_Bus, ");
				}
				if (qHandpieceError & HAND_TEST_DRILL_DIR) {
					csErrors += (iPort == HAND_PORTA) ? TEXT("A_Drill_Direction, ") : TEXT("B_Drill_Direction, ");
				}
				if (qHandpieceError & HAND_TEST_DRILL_SPEED) {
					csErrors += (iPort == HAND_PORTA) ? TEXT("A_DrillSpeed, ") : TEXT("B_DrillSpeed, ");
				}
			} else {
				csErrors += TEXT("(Wire Harness)");
			}
            csErrors += TEXT("\r");
			ReportTestFailed(csErrors);
		} else {
			ReportTestPassed();
		}
	} while (m_yRepeatTest);

    return TRUE;
}

SnBool CFactoryScreen::FootswitchTest()
{
    CString csConnector, csButtons;
    SnQByte qFootswitchError;
    SnWord wMode, wPressed, wValid, wState, wPrevPressed;
    SnBool yStatus;

    StartNewTest(TEXT("Footswitch...\t"));
    csConnector = TEXT("front panel connector");

	do {
		CString csErrors = SN_CLEAR_TEXT;

		qFootswitchError = 0;
		m_csRightLabel = TEXT("FAIL");

	    if (m_pControl) {
            m_pControl->DisableFootswitch();
	    }

		NewInstructions(TEXT("Plug the Test Fixture (20600579) footswitch cable into the "));
		AddInstructions(csConnector);
		AddInstructions(TEXT(". "));

		if (WaitForOK() == FALSE) {
			return FALSE;
		}

		SetDlgItemText(IDC_STATIC_TEST_TITLE, TEXT("Footswitch"));
		SetDlgItemText(IDC_STATIC_TEST_TEXT, SN_CLEAR_TEXT);
		m_StaticTestTitle.ShowWindow(SW_SHOW);
		m_StaticTestText.ShowWindow(SW_SHOW);

		m_BtnRight.ShowWindow(SW_SHOW);

		m_yRightPressed = FALSE;
		NewInstructions(TEXT("Release all buttons.\r"));
		AddInstructions(TEXT("Press the FAIL button if no buttons are being pressed."));
		wValid = 0;
        wState = 0;
        wPressed = 0;
		do {
			if (Delay(100) == FALSE) {
				return FALSE;
			}

			wPrevPressed = wPressed;
            wPressed = 0;
            csButtons = "";

			yStatus = m_pControl->ReadDsp(MC_ANALOG_AVG10, 1, &wMode);
			if (!yStatus) {
				qFootswitchError |= FOOT_TEST_DSP;
				goto FootswitchTestEnd;
			}
			if((SnSWord)wMode <= DIGITAL_ON_OFF_THRESHOLD_VALUE) {
				wPressed |= FOOT_TEST_FWD;
			}
			yStatus = m_pControl->ReadDsp(MC_ANALOG_AVG11, 1, &wMode);
			if (!yStatus) {
				qFootswitchError |= FOOT_TEST_DSP;
				goto FootswitchTestEnd;
			}
			if ((SnSWord)wMode <= DIGITAL_ON_OFF_THRESHOLD_VALUE) {
				wPressed |= FOOT_TEST_REV;
			}
			yStatus = m_pControl->ReadDsp(MC_ANALOG_AVG12, 1, &wMode);
			if (!yStatus) {
				qFootswitchError |= FOOT_TEST_DSP;
				goto FootswitchTestEnd;
			}
			if ((SnSWord)wMode <= DIGITAL_ON_OFF_THRESHOLD_VALUE) {
				wPressed |= FOOT_TEST_OSC;
			}
			yStatus = m_pControl->ReadDsp(MC_DIGITAL_STATE_DATA_D, 1, &wMode);
			if (!yStatus) {
				qFootswitchError |= FOOT_TEST_DSP;
				goto FootswitchTestEnd;
			}
			if (wMode & 0x0004)
				wPressed |= FOOT_TEST_LOGIC2;
			if (wMode & 0x0002)
				wPressed |= FOOT_TEST_LOGIC1;
			if (wMode & 0x0001)
				wPressed |= FOOT_TEST_LOGIC0;

            if (wPressed & FOOT_TEST_LOGIC1) {
                if (csButtons != "")
                    csButtons += ", ";
                csButtons += "LOGIC 1";
            }
            if (wPressed & FOOT_TEST_LOGIC0) {
                if (csButtons != "")
                    csButtons += ", ";
                csButtons += "I'M HERE";
            }
            if (wPressed & FOOT_TEST_FWD) {
                if (csButtons != "")
                    csButtons += ", ";
                csButtons += "FORWARD";
            }
            if (wPressed & FOOT_TEST_REV) {
                if (csButtons != "")
                    csButtons += ", ";
                csButtons += "REVERSE";
            }
            if (wPressed & FOOT_TEST_OSC) {
                if (csButtons != "")
                    csButtons += ", ";
                csButtons += "SPEED DN";
            }
            if (wPressed & FOOT_TEST_LOGIC2) {
                if (csButtons != "")
                    csButtons += ", ";
                csButtons += "SPEED UP";
            }

            if (wPressed != wPrevPressed)
                SetDlgItemText(IDC_STATIC_TEST_TEXT, csButtons);

            if (wPressed == 0) {
                switch (wPrevPressed) {
                case 0:
                    if (wState == 0) {
		                NewInstructions(TEXT("Press and release the I'M HERE button.\r"));
		                AddInstructions(TEXT("Press the FAIL button if the I'M HERE button has been pressed."));
                        wState++;
				    }
				    break;
			    case FOOT_TEST_LOGIC0:
				    if (wState == 1 && !(wValid & FOOT_TEST_LOGIC0)) {
					    wValid |= wPrevPressed;
		                NewInstructions(TEXT("Press and release the FORWARD button.\r"));
		                AddInstructions(TEXT("Press the FAIL button if the FORWARD button has been pressed."));
                        wState++;
				    }
                    break;
			    case FOOT_TEST_FWD:
				    if (wState == 2 && !(wValid & FOOT_TEST_FWD)) {
					    wValid |= wPrevPressed;
		                NewInstructions(TEXT("Press and release the REVERSE button.\r"));
		                AddInstructions(TEXT("Press the FAIL button if the REVERSE button has been pressed."));
                        wState++;
				    }
				    break;
			    case FOOT_TEST_REV:
				    if (wState == 3 && !(wValid & FOOT_TEST_REV)) {
					    wValid |= wPrevPressed;
		                NewInstructions(TEXT("Press and release the SPEED DN button.\r"));
		                AddInstructions(TEXT("Press the FAIL button if the SPEED DN button has been pressed."));
                        wState++;
				    }
				    break;
			    case FOOT_TEST_OSC:
				    if (wState == 4 && !(wValid & FOOT_TEST_OSC)) {
					    wValid |= wPrevPressed;
						wState++;
				    }
				    break;
			    default:
				    break;
                }
            }

		} while (!m_yKillThreads && wState != 5 && !m_yRightPressed);
		if (m_yKillThreads) {
			return FALSE;
		}

		m_StaticTestTitle.ShowWindow(SW_HIDE);
		m_StaticTestText.ShowWindow(SW_HIDE);
		m_BtnRight.ShowWindow(SW_HIDE);

#if ALWAYS_PASS
		m_yRightPressed = FALSE;
#else
		if (m_yRightPressed) {
			qFootswitchError |= (~wValid & 0x003f);
		}
#endif

	    if (m_pControl) {
            m_pControl->EnableFootswitch();
	    }

		NewInstructions(TEXT("Plug the DYONICS POWER II Footswitch (72201092) into the "));
		AddInstructions(csConnector);
		AddInstructions(TEXT(". "));

		if (WaitForOK() == FALSE) {
			return FALSE;
		}

#if !ALWAYS_PASS
		if (m_pControl->GetFootPedalType() != TYPE_485_ANALOG_FOOTPEDAL) {
			qFootswitchError |= FOOT_TEST_RS485;
		}
#endif

		NewInstructions(TEXT("Plug the DYONICS POWER Footswitch (7205396) into the "));
		AddInstructions(csConnector);
		AddInstructions(TEXT(". "));

		if (WaitForOK() == FALSE) {
			return FALSE;
		}

#if !ALWAYS_PASS
		if (m_pControl->GetFootPedalType() != TYPE_DIGITAL_FOOTPEDAL) {
			qFootswitchError |= FOOT_TEST_HALLBUS;
		}
#endif


FootswitchTestEnd:

		if (qFootswitchError) {
			csErrors = TEXT("Front Footswitch test failed. ");
			if (m_wTestMode == BOARD_TEST) {
				csErrors += TEXT("Check the following:\r\r");
				if (qFootswitchError & FOOT_TEST_DSP) {
					csErrors += TEXT("Interboard communication, ");
				}
				if (qFootswitchError & FOOT_TEST_FWD) {
					csErrors += TEXT("A_Foot_FWD_ISO, ");
				}
				if (qFootswitchError & FOOT_TEST_REV) {
					csErrors += TEXT("A_Foot_REV_ISO, ");
				}
				if (qFootswitchError & FOOT_TEST_LOGIC2) {
					csErrors += TEXT("A_Footswitch2, ");
				}
				if (qFootswitchError & FOOT_TEST_OSC) {
					csErrors += TEXT("A_Foot_OSC_ISO, ");
				}
				if (qFootswitchError & FOOT_TEST_LOGIC0) {
					csErrors += TEXT("A_Footswitch0, ");
				}
				if (qFootswitchError & FOOT_TEST_LOGIC1) {
					csErrors += TEXT("A_Footswitch1, ");
				}
				if (qFootswitchError & FOOT_TEST_RS485) {
					csErrors += TEXT("TRCVR U41, ");
				}
				if (qFootswitchError & FOOT_TEST_HALLBUS) {
					csErrors += TEXT("A_Footswitch_Hall_Bus, ");
				}
			} else {
				csErrors += TEXT("(Wire Harness)");
			}
            csErrors += TEXT("\r");
			ReportTestFailed(csErrors);
		} else {
			ReportTestPassed();
		}
	} while (m_yRepeatTest);

    return TRUE;
}

#define TOUCH_DELTA_X   (72/2)
#define TOUCH_DELTA_Y   (52/2)
#define TOUCH_DOT_X     551
#define TOUCH_DOT_Y     333

SnBool CFactoryScreen::TouchTest(void)
{
    SnBool yTouchTestPassed;
    RECT tTouchDot;

    // Touch Test
    StartNewTest(TEXT("Touch Screen...\t"));

    SetDlgItemText(IDC_STATIC_TEST_TITLE, TEXT("Touch Screen Test"));
    m_StaticTestTitle.ShowWindow(SW_SHOW);


    tTouchDot.left = TOUCH_DOT_X - 2;
    tTouchDot.right = TOUCH_DOT_X + 2;
    tTouchDot.top = TOUCH_DOT_Y - 2;
    tTouchDot.bottom = TOUCH_DOT_Y + 2;

    m_pTouchCDC = GetDC();
    m_pTouchCDC->FillSolidRect(&tTouchDot, RGB(255, 255, 255));

    do {
        m_qTouchPressCnt = 0;
        m_qTouchReleaseCnt = 0;
        m_qTouchOutOfBoundsCnt = 0;
        m_yTouchingScreen = FALSE;
        yTouchTestPassed = TRUE;

		NewInstructions(TEXT("Use a stylus to touch and hold on the white dot for 5 seconds.\n"));

        m_yTouchTest = TRUE;

        // Wait for First Touch
        while (!m_yKillThreads && m_qTouchPressCnt == 0) {
            if (Delay(100) == FALSE) {
                return FALSE;
            }
	    }

        SnQByte qTenthsOfSecondCnt = 0;

        // Wait for Five Seconds of Hold Time
        while (!m_yKillThreads && m_qTouchPressCnt == 1 && m_qTouchReleaseCnt == 0 && m_qTouchOutOfBoundsCnt == 0) {
            if ((qTenthsOfSecondCnt % 10) == 0) {
                CString csHoldString;
                SnQByte qSeconds = 5 - (qTenthsOfSecondCnt/10);

                csHoldString.Format(_T("Keep holding the stylus on the white dot for %d more second%s.\n"),
                    qSeconds, qSeconds > 1 ? _T("s") : _T(""));
                NewInstructions(csHoldString);
            }
            if (Delay(100) == FALSE) {
                return FALSE;
            }
            qTenthsOfSecondCnt++;
            if (qTenthsOfSecondCnt >= 50)
                break;
	    }

        if (m_qTouchPressCnt == 1 && m_qTouchReleaseCnt == 0 && m_qTouchOutOfBoundsCnt == 0 && qTenthsOfSecondCnt >= 50) {

		    NewInstructions(TEXT("Release the stylus from the screen.\n"));

            // Wait for Touch Release
            while (!m_yKillThreads && m_qTouchReleaseCnt == 0) {
                if (Delay(100) == FALSE) {
                    return FALSE;
                }
	        }
            m_yTouchTest = FALSE;
			ReportTestPassed();
        } else {
            m_yTouchTest = FALSE;
			ReportTestFailed(TEXT("Touch Screen Test Failed\r"));
		}
    } while (m_yRepeatTest);

    m_StaticTestTitle.ShowWindow(SW_HIDE);
    m_pTouchCDC->FillSolidRect(&tTouchDot, RGB(0, 0, 0));

    ReleaseDC(m_pTouchCDC);


    return TRUE;
}

BOOL CFactoryScreen::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
    static RECT tNullRect = {0, 0, 0, 0};

    if (m_yTouchTest) {
        POINT tNewTouchCursor;
        RECT tNewTouchBox;
        SnBool yTouchChanged;

        GetCursorPos(&tNewTouchCursor);

        yTouchChanged = FALSE;
        tNewTouchCursor = tNewTouchCursor;
        tNewTouchBox.left = tNewTouchCursor.x - TOUCH_DELTA_X;
        tNewTouchBox.right = tNewTouchCursor.x + TOUCH_DELTA_X;
        tNewTouchBox.top = tNewTouchCursor.y - TOUCH_DELTA_Y;
        tNewTouchBox.bottom = tNewTouchCursor.y + TOUCH_DELTA_Y;

        if (message & 1) {
            m_qTouchPressCnt++;
            yTouchChanged = TRUE;
            m_yTouchingScreen = TRUE;
            m_pTouchCDC->DrawDragRect(&tNewTouchBox, CSize(1,1),  NULL, CSize(1,1), NULL, NULL);
        } else if (message & 2) {
            m_qTouchReleaseCnt++;
            yTouchChanged = TRUE;
            m_yTouchingScreen = FALSE;
            m_pTouchCDC->DrawDragRect(&tNullRect, CSize(1,1),  &m_tOldTouchBox, CSize(1,1), NULL, NULL);
        } else if (m_yTouchingScreen) {
            if (tNewTouchCursor.x != m_tOldTouchCursor.x || tNewTouchCursor.y != m_tOldTouchCursor.y) {
                yTouchChanged = TRUE;
                m_pTouchCDC->DrawDragRect(&tNewTouchBox, CSize(1,1),  &m_tOldTouchBox, CSize(1,1), NULL, NULL);
            }
        }
        if (yTouchChanged &&
           (tNewTouchCursor.x < (TOUCH_DOT_X-TOUCH_DELTA_X) || tNewTouchCursor.x > (TOUCH_DOT_X+TOUCH_DELTA_X) ||
            tNewTouchCursor.y < (TOUCH_DOT_Y-TOUCH_DELTA_Y) || tNewTouchCursor.y > (TOUCH_DOT_Y+TOUCH_DELTA_Y))) {
            m_qTouchOutOfBoundsCnt++;
        }
        m_tOldTouchCursor = tNewTouchCursor;
        m_tOldTouchBox = tNewTouchBox;
    } else if (m_yTouchingScreen) {
        m_yTouchingScreen = FALSE;
        m_pTouchCDC->DrawDragRect(&tNullRect, CSize(1,1),  &m_tOldTouchBox, CSize(1,1), NULL, NULL);
    }

	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

SnBool CFactoryScreen::PowerButtonTest(void)
{
    // Power Button Test
    StartNewTest(TEXT("Power Button...\t"));

	do {
		NewInstructions(TEXT("Verify that the Power Button is illuminated."));
		if (WaitForPassFail() == FALSE) {
			return FALSE;
		}

		if (m_yRightPressed) {
			ReportTestFailed(TEXT("Power Button Test Failed\r"));
		} else {
			ReportTestPassed();
        }
    } while (m_yRepeatTest);

    //
    // If using a factory mode key and all system tests pass, erase the flash and save defaults
    //
    if (m_dwMode == FACTORY_MODE && m_wTestMode == SYSTEM_TEST && m_yFailedTest == FALSE) {
        m_pControl->EraseAndRestoreDefaults();
    }

    return TRUE;
}

void CFactoryScreen::PerformTests(void)
{
    if (m_wTestMode == SYSTEM_TEST) {
        if (SerialNumberTest() == FALSE) {
            return;
        }
    }
    if (NvRamTest() == FALSE) {
        return;
    }
    if (m_wTestMode == SYSTEM_TEST) {
        if (TouchTest() == FALSE) {
            return;
        }
    }
    if (SerialPortTest() == FALSE) {
        return;
    }
    if (BuzzerTest() == FALSE) {
        return;
    }
    if (UsbTest() == FALSE) {
        return;
    }
    if (DisplayTest() == FALSE) {
        return;
    }
    if (HandpieceTest(HAND_PORTA) == FALSE) {
        return;
    }
    if (HandpieceTest(HAND_PORTB) == FALSE) {
        return;
    }
    if (FootswitchTest() == FALSE) {
        return;
    }
    if (m_wTestMode == BOARD_TEST) {
        if (TemperatureTest() == FALSE) {
            return;
        }
    }
    if (PowerButtonTest() == FALSE) {
        return;
    }

    NewInstructions(TEXT("All tests are complete. \r\r"));
    AddInstructions(TEXT("Record the results of tests and press the "));
    AddInstructions(TEXT("power button to power down the system."));

    for (;;) {
        if (Delay(100) == FALSE) {
            return;
        }
    }
}

void CFactoryScreen::OnButtonMiddle() 
{
    if (m_wTestMode != NO_TEST) {
        m_yMiddlePressed = TRUE;
    } else {
        m_BtnMiddle.ShowWindow(SW_HIDE);
        m_BtnRight.ShowWindow(SW_HIDE);
        m_BtnSet.ShowWindow(SW_HIDE);
        m_StaticUsageCnt.ShowWindow(SW_HIDE);

        m_wTestMode = BOARD_TEST;
    }
}

void CFactoryScreen::OnButtonRight() 
{
    if (m_wTestMode != NO_TEST) {
        m_yRightPressed = TRUE;
    } else {
        m_BtnMiddle.ShowWindow(SW_HIDE);
        m_BtnRight.ShowWindow(SW_HIDE);
        m_BtnSet.ShowWindow(SW_HIDE);
        m_StaticUsageCnt.ShowWindow(SW_HIDE);

        m_wTestMode = SYSTEM_TEST;
    }
}

void CFactoryScreen::OnButtonSet() 
{
    if (m_wTestMode != NO_TEST) {
        m_ySetPressed = TRUE;
    } else {
	    if (m_pControl) {
            m_pControl->EraseAndRestoreDefaults();
	    }
    }
}
