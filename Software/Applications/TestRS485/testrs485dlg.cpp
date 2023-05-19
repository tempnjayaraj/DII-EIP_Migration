// TestRS485Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestRS485.h"
#include "TestRS485Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

SnBool SenseRs485Cmd(SnByte* pbByte);

/////////////////////////////////////////////////////////////////////////////
// CTestRS485Dlg dialog

CTestRS485Dlg::CTestRS485Dlg(CControl* pControl, CWnd* pParent /*=NULL*/)
	: CDialog(CTestRS485Dlg::IDD, pParent),
	m_pParent(pParent),
	m_pControl(pControl),
    m_hStatusThread(NULL),
	m_yStatusThreadKilled(FALSE),
	m_hStatusThreadKilledEvent(NULL),
    m_hShutDownAppEvent(NULL),
	m_yKillThreads(FALSE),
	m_DefaultFont(NULL),
	m_bCalibrationTest(FALSE),
	m_bInvalidCommandTest(FALSE),
	m_bResetTest(FALSE)
{
	//{{AFX_DATA_INIT(CTestRS485Dlg)
	m_TextSoftwareVersion = _T("");
	m_TextDeviceId = _T("");
	m_TextCalibrateStatus = _T("");
	m_TextLeftPedalPercent = _T("");
	m_TextMiddlePedalPercent = _T("");
	m_TextRightPedalPercent = _T("");
	m_TextLeftButton = _T("");
	m_TextRightButton = _T("");
	m_TextResetStatus = _T("");
	m_TextInvalidCommandTestStatus = _T("");
	m_TextCommandDuration = _T("");
	m_TextErrorCount = _T("");
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CTestRS485Dlg::~CTestRS485Dlg()
{
    DeInit();
}

void CTestRS485Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestRS485Dlg)
	DDX_Control(pDX, IDC_INVALID_COMMAND_TEST, m_cbInvalidCommandTest);
	DDX_Control(pDX, IDC_STATIC_LEFT_FOOT, m_scLeftPedalLabel);
	DDX_Control(pDX, IDC_STATIC_RIGHT_FOOT, m_scRightPedalLabel);
	DDX_Control(pDX, IDC_STATIC_MIDDLE_FOOT, m_scMiddlePedalLabel);
	DDX_Control(pDX, IDC_TEXT_RIGHT_FOOT_PERCENT, m_scRightPedalPercent);
	DDX_Control(pDX, IDC_TEXT_MIDDLE_FOOT_PERCENT, m_scMiddlePedalPercent);
	DDX_Control(pDX, IDC_TEXT_LEFT_FOOT_PERCENT, m_scLeftPedalPercent);
	DDX_Control(pDX, IDC_STATIC_SOFTWARE_VERSION, m_scSoftwareVersionLabel);
	DDX_Control(pDX, IDC_STATIC_DEVICE_ID, m_scDeviceIdLabel);
	DDX_Control(pDX, IDC_CALIBRATE, m_cbCalibrate);
	DDX_Control(pDX, IDC_TEXT_SOFTWARE_VERSION, m_scSoftwareVersion);
	DDX_Control(pDX, IDC_TEXT_DEVICE_ID, m_scDeviceId);
	DDX_Control(pDX, IDC_TEXT_CALIBRATE, m_scCalibrate);
	DDX_Text(pDX, IDC_TEXT_SOFTWARE_VERSION, m_TextSoftwareVersion);
	DDX_Text(pDX, IDC_TEXT_DEVICE_ID, m_TextDeviceId);
	DDX_Text(pDX, IDC_TEXT_CALIBRATE, m_TextCalibrateStatus);
	DDX_Text(pDX, IDC_TEXT_LEFT_FOOT_PERCENT, m_TextLeftPedalPercent);
	DDX_Text(pDX, IDC_TEXT_MIDDLE_FOOT_PERCENT, m_TextMiddlePedalPercent);
	DDX_Text(pDX, IDC_TEXT_RIGHT_FOOT_PERCENT, m_TextRightPedalPercent);
	DDX_Text(pDX, IDC_TEXT_LEFT_BUTTON, m_TextLeftButton);
	DDX_Text(pDX, IDC_TEXT_RIGHT_BUTTON, m_TextRightButton);
	DDX_Text(pDX, IDC_TEXT_RESET, m_TextResetStatus);
	DDX_Text(pDX, IDC_TEXT_INVALID_COMMAND, m_TextInvalidCommandTestStatus);
	DDX_Text(pDX, IDC_TEXT_COMMAND_DURATION, m_TextCommandDuration);
	DDX_Text(pDX, IDC_TEXT_CONNECT_DURATION, m_TextConnectDuration);
	DDX_Text(pDX, IDC_TEXT_ERROR_COUNT, m_TextErrorCount);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTestRS485Dlg, CDialog)
	//{{AFX_MSG_MAP(CTestRS485Dlg)
	ON_COMMAND(IDM_DIALOG, OnDialog)
	ON_COMMAND(IDM_EXIT, OnExit)
	ON_COMMAND(IDM_HELP, OnHelp)
	ON_MESSAGE(WM_EXIT_DIALOG, ExitDialog)
	ON_BN_CLICKED(IDC_CALIBRATE, OnCalibrate)
	ON_BN_CLICKED(IDC_INVALID_COMMAND_TEST, OnInvalidCommandTest)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_MESSAGE(WM_ERROR_CONDITION, HandleErrorConditions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestRS485Dlg message handlers

BOOL CTestRS485Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CClientDC dc(this);

	m_memDC.CreateCompatibleDC(&dc);
	UpdateTitle(TRUE);

	SetupFonts();
	GetFootStatus();

	m_hStatusThreadKilledEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if(m_hStatusThreadKilledEvent == NULL) {
        DeInit();
		return FALSE;
	}

	// Create event
	m_hShutDownAppEvent = CreateEvent(NULL, FALSE, FALSE, _T("SnShutDownAppEvent"));
	if( m_hShutDownAppEvent == NULL)
		return FALSE;
	
	// Create thread to update the display with read data
	m_hStatusThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
									  0,
									 (LPTHREAD_START_ROUTINE)CTestRS485Dlg::StatusThread,
									  this,
									  0,
									  &m_hStatusThreadID);
		
	if (m_hStatusThread == NULL) {
		DeInit();
		return FALSE;
	}

	// Ensure the Footswitch is configured to control Port A, 
	// Hand Control Overide Off, Variable Mode, Right Footswitch Forward
	m_tFootPedalStatus.usMode = FOOT_MODE_VARIABLE;
	m_tFootPedalStatus.usForward = FOOT_FORWARD_RIGHT;
	m_tFootPedalStatus.usOverride = FOOT_HAND_OVERRIDE_OFF;
	m_tFootPedalStatus.usPortControl = PORTA;
	SetFootStatus();


	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	CenterWindow(GetDesktopWindow());	// center to the hpc screen

	// Tell the control layer to send WM messages to this screen.
	if( m_pControl)
		m_pControl->SetMessageHandler(this->m_hWnd);
	
	SetDlgItemText( IDC_TEXT_SOFTWARE_VERSION, _T("Unknown"));
	SetDlgItemText( IDC_TEXT_DEVICE_ID, _T("Unknown"));
	SetDlgItemText( IDC_TEXT_LEFT_FOOT_PERCENT, _T("0%"));
	SetDlgItemText( IDC_TEXT_MIDDLE_FOOT_PERCENT, _T("0%"));
	SetDlgItemText( IDC_TEXT_RIGHT_FOOT_PERCENT, _T("0%"));
	SetDlgItemText( IDC_TEXT_LEFT_BUTTON, _T("0"));
	SetDlgItemText( IDC_TEXT_RIGHT_BUTTON, _T("0"));
	SetDlgItemText( IDC_TEXT_COMMAND_DURATION, _T("0.00 ms"));
	SetDlgItemText( IDC_TEXT_CONNECT_DURATION, _T("0 ms"));
	SetDlgItemText( IDC_TEXT_ERROR_COUNT, _T("0"));
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTestRS485Dlg::DeInit(void)
{
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

	if( m_hShutDownAppEvent)
	{
		CloseHandle( m_hShutDownAppEvent);
		m_hShutDownAppEvent = NULL;
	}

	// Finished with the font, now delete the font object.
	m_memDC.SelectObject(m_DefaultFont);

	m_Font.DeleteObject();
}

void CTestRS485Dlg::UpdateTitle(SnBool yForceUpdate)
{
	SN_MOTOR_REVISION qMotorRev;
	CString csTitle;
	SN_SYS_REVISION tRevision;
	qMotorRev.qMotorBoardVersion = 0xFFFFFFFF;


	// Get the revision numbers from the Motor Control Board
	if(m_pControl)
	{
		m_pControl->GetCmdState(GET_SYSTEM_REVISIONS, &tRevision, sizeof(SN_SYS_REVISION));
	}
	qMotorRev.ucMotorBoardMajor = tRevision.ucMotorBoardMajor;
	qMotorRev.ucMotorBoardMinor = tRevision.ucMotorBoardMinor;
	qMotorRev.ucMotorBoardBuild = tRevision.ucMotorBoardBuild;

	if (yForceUpdate || (qMotorRev.qMotorBoardVersion != m_qMotorRev.qMotorBoardVersion)) {
		m_qMotorRev.qMotorBoardVersion = qMotorRev.qMotorBoardVersion;
		if (qMotorRev.ucMotorBoardMajor != 0xFFFFFFFF) {
			csTitle.Format(TEXT("DII Footswitch Tester: %d.%02d.%02d,   Motor Board: %d.%02d.%02d"),
				TESTRS485_VERS_MAJOR, TESTRS485_VERS_MINOR, TESTRS485_VERS_BUILD,
				qMotorRev.ucMotorBoardMajor, qMotorRev.ucMotorBoardMinor, qMotorRev.ucMotorBoardBuild);
		} else {
			csTitle.Format(TEXT("DII Footswitch Tester: %d.%02d.%02d,   Motor Board: -.--.--"),
				TESTRS485_VERS_MAJOR, TESTRS485_VERS_MINOR, TESTRS485_VERS_BUILD);
		}
		SetWindowText(csTitle);
	}
}

void CTestRS485Dlg::OnSoftwareVersion() 
{

	if(m_pControl)
	{
		m_pControl->GetCmdState(GET_SYSTEM_REVISIONS, &m_tRevision, sizeof(SN_SYS_REVISION));
	}

	UpdateTitle(FALSE);

	if (m_tRevision.ucFootswitchMajor != m_tPrevRevision.ucFootswitchMajor ||
		m_tRevision.ucFootswitchMinor != m_tPrevRevision.ucFootswitchMinor ||
		m_tRevision.ucFootswitchBuild != m_tPrevRevision.ucFootswitchBuild)
	{
		m_TextSoftwareVersion = _T("Unknown");
		if( m_tRevision.ucFootswitchMajor != 0)
		{	
			// Display the 485 Analog Footswitch version
			m_TextSoftwareVersion.Format(_T("%d.%02d.%02d"), m_tRevision.ucFootswitchMajor, m_tRevision.ucFootswitchMinor,
									  m_tRevision.ucFootswitchBuild);
		}
		SetDlgItemText( IDC_TEXT_SOFTWARE_VERSION, m_TextSoftwareVersion);
		m_tPrevRevision  = m_tRevision;
	}
}

void CTestRS485Dlg::UpdateButtons() 
{

	GetFootStatus();
	if (m_tFootPedalStatus.wButtonStatus.left != m_tPrevFootPedalStatus.wButtonStatus.left)
	{
		m_TextLeftButton.Format(_T("%d"),m_tFootPedalStatus.wButtonStatus.left);
		SetDlgItemText( IDC_TEXT_LEFT_BUTTON, m_TextLeftButton);
		m_tPrevFootPedalStatus.wButtonStatus.left = m_tFootPedalStatus.wButtonStatus.left;
	}
	if (m_tFootPedalStatus.wButtonStatus.right != m_tPrevFootPedalStatus.wButtonStatus.right)
	{
		m_TextRightButton.Format(_T("%d"),m_tFootPedalStatus.wButtonStatus.right);
		SetDlgItemText( IDC_TEXT_RIGHT_BUTTON, m_TextRightButton);
		m_tPrevFootPedalStatus.wButtonStatus.right = m_tFootPedalStatus.wButtonStatus.right;
	}

}

void CTestRS485Dlg::OnDeviceId() 
{

	GetFootStatus();
	SnBool bClearResults = TRUE;

	if (m_tFootPedalStatus.usType != m_tPrevFootPedalStatus.usType)
	{
		switch (m_tFootPedalStatus.usType)
		{
		case TYPE_DIGITAL_FOOTPEDAL:
			m_TextDeviceId.Format(TEXT("On/Off Footswitch"));
			break;
		case TYPE_ANALOG_FOOTPEDAL:
			m_TextDeviceId.Format(TEXT("Variable Speed Footswitch"));
			break;
		case TYPE_485_ANALOG_FOOTPEDAL:
			m_TextDeviceId.Format(TEXT("Dyonics Power II Footswitch"));
			break;
		case TYPE_WIRELESS_FOOTPEDAL:
			m_TextDeviceId.Format(TEXT("Wireless Footswitch"));
			break;
		default:
			m_TextDeviceId.Format(IDS_UNKNOWN);
//			bClearResults = FALSE;
			break;
		}
		SetDlgItemText( IDC_TEXT_DEVICE_ID, m_TextDeviceId);

		if (bClearResults)
		{
			if (m_tFootPedalStatus.usType )
			m_TextInvalidCommandTestStatus.Format(_T(""));
			SetDlgItemText( IDC_TEXT_INVALID_COMMAND, m_TextInvalidCommandTestStatus);

			m_TextResetStatus.Format(_T(""));
			SetDlgItemText( IDC_TEXT_RESET, m_TextResetStatus);

			m_TextCalibrateStatus =_T("");
			SetDlgItemText( IDC_TEXT_CALIBRATE, m_TextCalibrateStatus);
		}
	}

	if (m_tFootPedalStatus.pwPedalPercent[0] != m_tPrevFootPedalStatus.pwPedalPercent[0])
	{
		m_TextLeftPedalPercent.Format(_T("%d%%"),m_tFootPedalStatus.pwPedalPercent[0]);
		SetDlgItemText( IDC_TEXT_LEFT_FOOT_PERCENT, m_TextLeftPedalPercent);
	}
	if (m_tFootPedalStatus.pwPedalPercent[1] != m_tPrevFootPedalStatus.pwPedalPercent[1])
	{
		m_TextMiddlePedalPercent.Format(_T("%d%%"),m_tFootPedalStatus.pwPedalPercent[1]);
		SetDlgItemText( IDC_TEXT_MIDDLE_FOOT_PERCENT, m_TextMiddlePedalPercent);
	}
	if (m_tFootPedalStatus.pwPedalPercent[2] != m_tPrevFootPedalStatus.pwPedalPercent[2])
	{
		m_TextRightPedalPercent.Format(_T("%d%%"),m_tFootPedalStatus.pwPedalPercent[2]);
		SetDlgItemText( IDC_TEXT_RIGHT_FOOT_PERCENT, m_TextRightPedalPercent);
	}

	UpdateButtons();

	if (m_tFootPedalStatus.dCommandDuration != m_tPrevFootPedalStatus.dCommandDuration)
	{
		m_TextCommandDuration.Format(_T("%4.2f ms"),(float)m_tFootPedalStatus.dCommandDuration);
		SetDlgItemText( IDC_TEXT_COMMAND_DURATION, m_TextCommandDuration);
	}
	if (m_tFootPedalStatus.qConnectDuration != m_tPrevFootPedalStatus.qConnectDuration)
	{
		m_TextConnectDuration.Format(_T("%d ms"),m_tFootPedalStatus.qConnectDuration);
		SetDlgItemText( IDC_TEXT_CONNECT_DURATION, m_TextConnectDuration);
	}
	if (m_tFootPedalStatus.qErrorCount != m_tPrevFootPedalStatus.qErrorCount)
	{
		m_TextErrorCount.Format(_T("%d"),m_tFootPedalStatus.qErrorCount);
		SetDlgItemText( IDC_TEXT_ERROR_COUNT, m_TextErrorCount);
	}

	m_tPrevFootPedalStatus = m_tFootPedalStatus;
}

void CTestRS485Dlg::OnDialog() 
{
	// TODO: Add your command handler code here
	
}

void CTestRS485Dlg::OnExit() 
{
	// TODO: Add your command handler code here

}

LRESULT CTestRS485Dlg::ExitDialog(WPARAM iParam, LPARAM lParam)
{
	DeInit();
	
	// Kill the dialog window and return to main
	CDialog::EndDialog(IDCANCEL);

	return 0L;
}

void CTestRS485Dlg::OnHelp() 
{
	MessageBox(TEXT("Dialog Demo"), TEXT("Help"));
	
}


SnBool CTestRS485Dlg::GetFootStatus()
{

	if( m_pControl)
	{
  		m_pControl->GetCmdState(GET_MC_FOOT_STATUS, &m_tFootPedalStatus, sizeof(SN_FOOT_STATUS));
		return TRUE;
	}
	return FALSE;
}

SnBool CTestRS485Dlg::SetFootStatus()
{

	if( m_pControl)
	{
  		m_pControl->SetCmdState(GET_MC_FOOT_STATUS, &m_tFootPedalStatus, sizeof(SN_FOOT_STATUS));
		return TRUE;
	}
	return FALSE;
}

void CTestRS485Dlg::SetupFonts()
{
	SnBool bStatus;
	LOGFONT txtFont;

	// Setup the common members for all Fonts
	txtFont.lfWidth = 0;
	txtFont.lfEscapement = 0;
	txtFont.lfOrientation = 0;
	txtFont.lfItalic = FALSE;
	txtFont.lfUnderline = FALSE;
	txtFont.lfStrikeOut = 0;
	txtFont.lfCharSet = DEFAULT_CHARSET;
	txtFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	txtFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	txtFont.lfQuality = DEFAULT_QUALITY;
	txtFont.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
	wcscpy(txtFont.lfFaceName, _T("Arial"));
	
	// Create Font
	txtFont.lfHeight = 120;
	txtFont.lfWeight = FW_BOLD;

	bStatus = m_Font.CreatePointFontIndirect(&txtFont, &m_memDC);
	
	m_DefaultFont= m_memDC.SelectObject(&m_Font);

	short int fRedraw= FALSE; 

	SendMessageToDescendants(WM_SETFONT,
		(WPARAM)m_Font.m_hObject,  // Handle to font
		MAKELONG ((WORD) fRedraw, 0), 
		FALSE);  // Send to all descendants(TRUE) or just children of *this (FALSE)

}

SnBool CTestRS485Dlg::TestInProgress(void)
{
	return (m_bInvalidCommandTest || 
			m_bCalibrationTest || 
			m_bResetTest);
}

void CTestRS485Dlg::StatusThread(LPVOID pParam)
{
	CTestRS485Dlg *pClass = (CTestRS485Dlg*)pParam;

	pClass->m_yStatusThreadKilled = FALSE; // Reset the ThreadKilled flag


    // Next wait for a test button to be pressed
	while (!pClass->m_yKillThreads)
	{
		if (pClass->m_bInvalidCommandTest)
		{
			pClass->InvalidCommandTest();
			
		}
		else if(pClass->m_bResetTest)
		{
			pClass->ResetTest();
		}
		else if(pClass->m_bCalibrationTest)
		{
			pClass->CalibrationTest();
		}
		else
		{
			pClass->OnDeviceId();
			pClass->OnSoftwareVersion();
			Sleep(20);
			pClass->UpdateButtons();
			Sleep(20);
		}
        if (WaitForSingleObject(pClass->m_hShutDownAppEvent, 0) == WAIT_OBJECT_0) {
            ExitProcess(0);
        }
	}

    // Set the Thread Killed Event
	SetEvent(pClass->m_hStatusThreadKilledEvent);
}

void CTestRS485Dlg::KillThreads()
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

LRESULT CTestRS485Dlg::HandleErrorConditions(WPARAM iParam, LPARAM lParam)
{
	CString csSystemFailure;
    SnBool ySystemError = FALSE;
	SnQByte qHandpieceError =0;

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
            qHandpieceError |= iParam;
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

    if (ySystemError) 
	{
    }

	return 0L;
}



void CTestRS485Dlg::OnInvalidCommandTest() 
{
	if ((m_tFootPedalStatus.usType == TYPE_485_ANALOG_FOOTPEDAL)
		&& !TestInProgress())
	{
		Sleep(100);
		m_bInvalidCommandTest = TRUE;
	}
}

void CTestRS485Dlg::OnReset() 
{
	if ((m_tFootPedalStatus.usType == TYPE_485_ANALOG_FOOTPEDAL)
		&& !TestInProgress())
	{
		m_bResetTest = TRUE;
	}
}

void CTestRS485Dlg::OnCalibrate() 
{
	if ((m_tFootPedalStatus.usType == TYPE_485_ANALOG_FOOTPEDAL)
		&& !TestInProgress())
	{
		m_bCalibrationTest = TRUE;
	}
}

void CTestRS485Dlg::InvalidCommandTest(void)
{
	SnWord wStartStop = 1;
	SnBool bGoodResult = FALSE;
	SnBool bNotDone = FALSE;
	SnQByte qErrorCount =-1;
	
	GetFootStatus();
	if (m_pControl->SetCmdState(SET_FOOTSWITCH_INVALID_TEST, &wStartStop, sizeof(wStartStop)))
	{
		qErrorCount= m_tFootPedalStatus.qErrorCount;
		m_TextInvalidCommandTestStatus.Format(_T("Test In Progress"));
		SetDlgItemText( IDC_TEXT_INVALID_COMMAND, m_TextInvalidCommandTestStatus);
		bNotDone = TRUE;
		bGoodResult = TRUE;
	}

	while (bNotDone)
	{
		GetFootStatus();
		bNotDone = m_tFootPedalStatus.bInInvalidCommandTest;
		if (qErrorCount != m_tFootPedalStatus.qErrorCount)
			bGoodResult = FALSE;
	}
	
	m_TextInvalidCommandTestStatus.Format(_T("Test %s"),(bGoodResult)?_T("Passed"):_T("Failed"));
	SetDlgItemText( IDC_TEXT_INVALID_COMMAND, m_TextInvalidCommandTestStatus);
	if (!bGoodResult)
		Sleep(2000);
	m_bInvalidCommandTest = FALSE;
}

void CTestRS485Dlg::ResetTest(void)
{
	m_TextResetStatus.Format(_T("Reset In Progress"));
	SetDlgItemText( IDC_TEXT_RESET, m_TextResetStatus);

	SnQByte timerStop = 0;
	SnQByte timerStart = GetTickCount();
	SnBool notDone = TRUE;
	SnWord wStartStop = 1;
	SnWord ii = 0;

	if (!m_pControl->SetCmdState(SET_FOOTSWITCH_RESET, &wStartStop, sizeof(wStartStop)))
	{
		m_TextResetStatus.Format(_T("Test Failed"));
		SetDlgItemText( IDC_TEXT_RESET, m_TextResetStatus);
		m_bResetTest = FALSE;
		return;
	}
	ii =0;
	do
	{
		Sleep(10);
		GetFootStatus();
		ii++;
	}
	while(m_tFootPedalStatus.usType == TYPE_485_ANALOG_FOOTPEDAL && ii < 200 );

	do
	{
		Sleep(10);
		timerStop = GetTickCount();
		if (timerStop >= timerStart)
		{
			timerStop -= timerStart;
		}
		else
		{
			timerStop += (MAXDWORD-timerStart);
		}
		if (timerStop >= 5000)
			break;

		GetFootStatus();
	} while (m_tFootPedalStatus.bInSoftwareReset == TRUE );

	wStartStop = 0;
	m_pControl->SetCmdState(SET_FOOTSWITCH_RESET, &wStartStop, sizeof(wStartStop));
	OnDeviceId();
	OnSoftwareVersion();
	Sleep(50);

	if (timerStop >= 5000)
		m_TextResetStatus.Format(_T("Reset Failed Timed Out %d ms"),timerStop);
	if ((m_tFootPedalStatus.usType != TYPE_485_ANALOG_FOOTPEDAL) || timerStop > 750)
		m_TextResetStatus.Format(_T("Reset Failed after %d ms"),timerStop);
	else
		m_TextResetStatus.Format(_T("Reset Complete after %d ms"),m_tFootPedalStatus.qConnectDuration);
	
	SetDlgItemText( IDC_TEXT_RESET, m_TextResetStatus);
	m_bResetTest = FALSE;
}

void CTestRS485Dlg::CalibrationTest(void)
{
	m_TextCalibrateStatus =_T("");
	SetDlgItemText( IDC_TEXT_CALIBRATE, m_TextCalibrateStatus);
	
	SnWord wStartStop = 1;
	SnWord ii = 0;

	SnBool bCalStart = m_pControl->SetCmdState(SET_FOOTSWITCH_CALIBRATE, &wStartStop, sizeof(wStartStop));
	SnBool bCalStop = FALSE;
	if (bCalStart )
	{
		CString calibrating = _T("CALIBRATING ...........");
		for (int ii = 0; ii < 16 && bCalStart; ii++)
		{
			m_TextCalibrateStatus += calibrating[ii];
			SetDlgItemText( IDC_TEXT_CALIBRATE, m_TextCalibrateStatus);
			
			for (int yy = 0; yy < 5 && bCalStart; yy++)
			{
				Sleep(100);
				GetFootStatus();
				if (m_tFootPedalStatus.bInCalibration != TRUE )
					bCalStart = FALSE;
			}
		}
		SnWord wStartStop = 0;
		bCalStop = m_pControl->SetCmdState(SET_FOOTSWITCH_CALIBRATE, &wStartStop, sizeof(wStartStop));
	}
	
	if (bCalStart && bCalStop )
	{
		m_TextCalibrateStatus.Format(_T("Calibration Complete"));
		SetDlgItemText( IDC_TEXT_CALIBRATE, m_TextCalibrateStatus);
	}
	else
	{
		m_TextCalibrateStatus.Format(_T("Calibration Failed"));
		SetDlgItemText( IDC_TEXT_CALIBRATE, m_TextCalibrateStatus);
		Sleep(2000);	// Allow footswitch to store calibration results
	}

	m_bCalibrationTest = FALSE;
}