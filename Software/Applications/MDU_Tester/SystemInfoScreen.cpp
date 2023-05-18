// SystemInfoScreen.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "SystemInfoScreen.h"
#include "MessageBox.h"
#include "SerialNumberPopUp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSystemInfoScreen dialog


CSystemInfoScreen::CSystemInfoScreen(CControl* pControl, CWnd* pParent /*=NULL*/)
	: CDialog(CSystemInfoScreen::IDD, pParent)
{
	m_pParent = pParent;
	m_pControl = pControl;

	// create brush for background color
	m_hbr = CreateSolidBrush(SN_BKGND_COLOR);
	
    m_yResetDlg = FALSE;

	//{{AFX_DATA_INIT(CSystemInfoScreen)
	//}}AFX_DATA_INIT
}

CSystemInfoScreen::~CSystemInfoScreen()
{
    DeInit();
}

void CSystemInfoScreen::DeInit()
{
	// delete brush objects
	DeleteObject( m_hbr);
}

void CSystemInfoScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSystemInfoScreen)
	DDX_Control(pDX, IDC_STATIC_MODEL2, m_StaticSerialNumber);
	DDX_Control(pDX, IDC_STATIC9, m_Static9);
	DDX_Control(pDX, IDC_BUTTON_RESET, m_BtnReset);
	DDX_Control(pDX, IDC_BUTTON_DONE, m_BtnDone);
	DDX_Control(pDX, IDC_STATIC_VERSION_PORTB, m_StaticVerPortB);
	DDX_Control(pDX, IDC_STATIC_VERSION_PORTA, m_StaticVerPortA);
	DDX_Control(pDX, IDC_STATIC_VERSION_FOOT, m_StaticVerFoot);
	DDX_Control(pDX, IDC_STATIC_VERSION_MC, m_StaticVerMc);
	DDX_Control(pDX, IDC_STATIC_VERSION_APP, m_StaticVerApp);
	DDX_Control(pDX, IDC_STATIC8, m_Static8);
	DDX_Control(pDX, IDC_STATIC7, m_Static7);
	DDX_Control(pDX, IDC_STATIC6, m_Static6);
	DDX_Control(pDX, IDC_STATIC5, m_Static5);
	DDX_Control(pDX, IDC_STATIC4, m_Static4);
	DDX_Control(pDX, IDC_STATIC_VERSION, m_StaticVersionNumber);
	DDX_Control(pDX, IDC_STATIC3, m_Static3);
	DDX_Control(pDX, IDC_STATIC2, m_Static2);
	DDX_Control(pDX, IDC_STATIC1, m_Static1);
	DDX_Control(pDX, IDC_STATIC_MODEL, m_StaticModelNumber);
	DDX_Control(pDX, IDC_STATIC_COPYRIGHT, m_StaticCopyright);
    DDX_Control(pDX, IDC_STATIC_BUTTON_HANDPIECE_SERIALNUMBER_PORTA, m_BtnHandpieceSerialNumberSettingsPortA);
    DDX_Control(pDX, IDC_STATIC_BUTTON_HANDPIECE_SERIALNUMBER_PORTB, m_BtnHandpieceSerialNumberSettingsPortB);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSystemInfoScreen, CDialog)
	//{{AFX_MSG_MAP(CSystemInfoScreen)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_DONE, OnButtonDone)
	ON_BN_CLICKED(IDC_BUTTON_RESET, OnButtonReset)
	ON_MESSAGE(WM_UPDATE_STATUS, UpdateStatus)
	ON_MESSAGE(WM_INTELLIO_SHAVER_CMD, HandleIntellioShaverCmd)
    ON_BN_CLICKED(IDC_STATIC_BUTTON_HANDPIECE_SERIALNUMBER_PORTA, OnButtonHandpieceSerialNumberSettingsPortA)
    ON_BN_CLICKED(IDC_STATIC_BUTTON_HANDPIECE_SERIALNUMBER_PORTB, OnButtonHandpieceSerialNumberSettingsPortB)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSystemInfoScreen message handlers

HBRUSH CSystemInfoScreen::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
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

void CSystemInfoScreen::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	DrawLines();
	
	// Do not call CDialog::OnPaint() for painting messages
}

BOOL CSystemInfoScreen::OnInitDialog() 
{
	CDialog::OnInitDialog();

	SnBool bStatus;
	CSharedMemory mem;

	// Initialize Shared memory if necessary
	bStatus = mem.GetInitStatus();
	if( !bStatus)
		mem.Init( m_pParent, m_pControl);

    m_bHandpieceSerialNumberFlagPortA = FALSE;
    m_bHandpieceSerialNumberFlagPortB = FALSE;
	// Setup the screen
	SetupTextButtons();
    SetupBitmaps();
	SetupFonts();
	SetupDefaults();
    DrawBitmaps(m_bHandpieceSerialNumberFlagPortA, m_bHandpieceSerialNumberFlagPortB);

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CSystemInfoScreen::UpdateStatus(WPARAM iParam, LPARAM lParam)
{
	switch( iParam)
	{
		case MSG_UPDATE_FOOT_STATUS:
	
		   if( m_pControl)
			{
				SN_SYS_REVISION tRevision;

				m_pControl->GetCmdState(GET_SYSTEM_REVISIONS, &tRevision, sizeof(SN_SYS_REVISION));
				
				if( tRevision.ucFootswitchMajor != 0)
				{
					CString csTemp;
					m_StaticVerFoot.ShowWindow(SW_SHOW);
					m_Static8.ShowWindow(SW_SHOW);
					
					// Display the 485 Analog Footswitch version
					csTemp.Format(_T("%d.%02d.%02d"), tRevision.ucFootswitchMajor, tRevision.ucFootswitchMinor,
											  tRevision.ucFootswitchBuild);
			
					SetDlgItemText( IDC_STATIC_VERSION_FOOT, csTemp);
				}
				else
				{
					// Hide static windows
					m_StaticVerFoot.ShowWindow(SW_HIDE);
					m_Static8.ShowWindow(SW_HIDE);
				}
			}
			break;
	
		case MSG_UPDATE_PORT_STATUS:
            if( m_pControl)
		    {
				SN_SYS_REVISION tRevision;

				m_pControl->GetCmdState(GET_SYSTEM_REVISIONS, &tRevision, sizeof(SN_SYS_REVISION));
				UpdateHandPieceVersion(lParam, tRevision);
                DrawBitmaps(m_bHandpieceSerialNumberFlagPortA, m_bHandpieceSerialNumberFlagPortB);   
		    }
		    break;

	    case MSG_UPDATE_ALL_SETTINGS:
	        SetupTextButtons();
            m_BtnDone.RedrawWindow();
            if (m_yResetDlg == FALSE) {
                m_BtnReset.RedrawWindow();
            }
	        SetupDefaults();
            DrawBitmaps(m_bHandpieceSerialNumberFlagPortA, m_bHandpieceSerialNumberFlagPortB);
            break;

		default:
			break;
 
	}// end switch

	return 0;
}

LRESULT CSystemInfoScreen::HandleIntellioShaverCmd( WPARAM iParam, LPARAM lParam)
{
	if( m_pControl != NULL && iParam == KEY_EXIT_SETTINGS)
    {
        ::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
            (WPARAM)MSG_UPDATE_TITLE, (LPARAM)SN_NO_TITLE);
        
        DeInit();

	    // Kill the dialog window and return to main
	    CDialog::EndDialog(IDCANCEL);
	}

	return 0L;
}

void CSystemInfoScreen::SetupTextButtons()
{
	CSharedMemory mem;

    // Setup Done button
	m_BtnDone.LoadBitmaps(IDB_BITMAP_BK_BUTTON, IDB_BITMAP_BK_BUTTON_PRESSED,
        mem.m_Font20Normal, m_SnHelp.GetString(SN_DONE));
	
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
				
    // Setup System Reset button
	if( usLanguage == LANGUAGE_DUTCH || usLanguage == LANGUAGE_SWEDISH)
	{
	    m_BtnReset.LoadBitmaps(IDB_BITMAP_BUTTON_GRAY, IDB_BITMAP_BUTTON_GRAY_PRESSED,
            mem.m_Font15Normal, m_SnHelp.GetString(SN_FACTORY_DEFAULT));	
	}
	else
	{
	    m_BtnReset.LoadBitmaps(IDB_BITMAP_BUTTON_GRAY, IDB_BITMAP_BUTTON_GRAY_PRESSED,
            mem.m_Font18Normal, m_SnHelp.GetString(SN_FACTORY_DEFAULT), SN_WHITE, 8);	
	}

}

void CSystemInfoScreen::SetupBitmaps()
{
    m_SnHelp.LoadBitmap(&m_ButtonOn, IDB_BITMAP_TESTDOT);
}

void CSystemInfoScreen::DrawBitmaps(SnBool bHandpieceSerialNumberFlagPortA, SnBool bHandpieceSerialNumberFlagPortB)
{
    if(bHandpieceSerialNumberFlagPortA)
    {
        m_BtnHandpieceSerialNumberSettingsPortA.ShowWindow(SW_SHOW);
        m_BtnHandpieceSerialNumberSettingsPortA.SetBitmap(m_ButtonOn);
        m_bHandpieceSerialNumberFlagPortA = FALSE;
    }
    else
    {
        m_BtnHandpieceSerialNumberSettingsPortA.ShowWindow(SW_HIDE);
    }

    if(bHandpieceSerialNumberFlagPortB)
    {
        m_BtnHandpieceSerialNumberSettingsPortB.ShowWindow(SW_SHOW);
        m_BtnHandpieceSerialNumberSettingsPortB.SetBitmap(m_ButtonOn);
        m_bHandpieceSerialNumberFlagPortB = FALSE;
    }
    else
    {
        m_BtnHandpieceSerialNumberSettingsPortB.ShowWindow(SW_HIDE);
    }
}

void CSystemInfoScreen::SetupFonts()
{
	CSharedMemory mem;
	
	// fixed text
	m_Static1.SetFont(mem.m_Font25Bold, TRUE);
	m_Static2.SetFont(mem.m_Font25Bold, TRUE);
	m_Static3.SetFont(mem.m_Font25Bold, TRUE);
	m_StaticModelNumber.SetFont(mem.m_Font25Bold, TRUE);
	m_StaticSerialNumber.SetFont(mem.m_Font25Bold, TRUE);
	m_StaticVersionNumber.SetFont(mem.m_Font25Bold, TRUE);

	// Application and Motor Controller Revision numbers
	m_Static4.SetFont(mem.m_Font18Normal, TRUE);
	m_Static5.SetFont(mem.m_Font18Normal, TRUE);
	m_Static6.SetFont(mem.m_Font18Normal, TRUE);
	m_Static7.SetFont(mem.m_Font18Normal, TRUE);
	m_Static8.SetFont(mem.m_Font18Normal, TRUE);
	m_Static9.SetFont(mem.m_Font25Bold, TRUE);
	m_StaticVerApp.SetFont(mem.m_Font18Normal, TRUE);
	m_StaticVerMc.SetFont(mem.m_Font18Normal, TRUE);
	m_StaticVerPortA.SetFont(mem.m_Font18Normal, TRUE);
	m_StaticVerPortB.SetFont(mem.m_Font18Normal, TRUE);
	m_StaticVerFoot.SetFont(mem.m_Font18Normal, TRUE);
	m_StaticCopyright.SetFont(mem.m_Font16Normal, TRUE);
}

void CSystemInfoScreen::SetupDefaults()
{
    // Setup static text boxes
	SetDlgItemText( IDC_STATIC1, SN_DYONICS_MDU_TEST);

  	SetDlgItemText( IDC_STATIC2, m_SnHelp.GetString(SN_MODEL));
	SetDlgItemText( IDC_STATIC3, m_SnHelp.GetString(SN_VERSION));
	SetDlgItemText( IDC_STATIC_MODEL, SN_SHAVER_REF);
	SetDlgItemText( IDC_STATIC_COPYRIGHT, SN_COPYRIGHT);

    if( m_pControl)
	{
        char pcSerialNumber[SERIAL_NUMBER_STORE];
		SN_SYS_REVISION tRevision;
		CString csTemp;

        m_pControl->GetSerialNumber(pcSerialNumber);
        if (pcSerialNumber[0] != 0) {
		    csTemp = pcSerialNumber;
            SetDlgItemText( IDC_STATIC9, _T("SN"));
            SetDlgItemText( IDC_STATIC_MODEL2, csTemp);
        } else {
            SetDlgItemText( IDC_STATIC9, _T(""));
            SetDlgItemText( IDC_STATIC_MODEL2, _T(""));
        }

	    m_pControl->GetCmdState(GET_SYSTEM_REVISIONS, &tRevision, sizeof(SN_SYS_REVISION));
	    
		csTemp.Format(_T("%d.%02d.%02d"), tRevision.ucSystemMajor, tRevision.ucSystemMinor, tRevision.ucSystemBuild);

		SetDlgItemText( IDC_STATIC_VERSION, csTemp);
	
		SetDlgItemText( IDC_STATIC4, m_SnHelp.GetString(SN_APPLICATION_VERSION));
		SetDlgItemText( IDC_STATIC5, m_SnHelp.GetString(SN_MOTOR_CONTROLLER_VERSION));
		SetDlgItemText( IDC_STATIC6, m_SnHelp.GetString(SN_HANDPIECE_VERSION_PORTA));
		SetDlgItemText( IDC_STATIC7, m_SnHelp.GetString(SN_HANDPIECE_VERSION_PORTB));
		SetDlgItemText( IDC_STATIC8, m_SnHelp.GetString(SN_FOOTSWITCH_VERSION));
	
		SetDlgItemText( IDC_STATIC_VERSION_APP, SN_APP_VERSION);
	
		csTemp.Format(_T("%d.%02d.%02d"), tRevision.ucMotorBoardMajor, tRevision.ucMotorBoardMinor,
									  tRevision.ucMotorBoardBuild);

		SetDlgItemText( IDC_STATIC_VERSION_MC, csTemp);

		if( tRevision.ucFootswitchMajor != 0 || tRevision.ucFootswitchMinor != 0 || tRevision.ucFootswitchBuild != 0)
		{
			// Display the 485 Analog Footswitch version
			csTemp.Format(_T("%d.%02d.%02d"), tRevision.ucFootswitchMajor, tRevision.ucFootswitchMinor,
									  tRevision.ucFootswitchBuild);
	
			SetDlgItemText( IDC_STATIC_VERSION_FOOT, csTemp);

		}
		else
		{
			// Hide static windows
			m_StaticVerFoot.ShowWindow(SW_HIDE);
			m_Static8.ShowWindow(SW_HIDE);
		}
		UpdateHandPieceVersion(PORTA, tRevision);
		UpdateHandPieceVersion(PORTB, tRevision);

	}
	else
	{
		SetDlgItemText( IDC_STATIC_VERSION, m_SnHelp.GetString(SN_VERSION_NUMBER));
	}

    ::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
        (WPARAM)MSG_UPDATE_TITLE, (LPARAM)SN_SYSTEM_INFORMATION);
}

void CSystemInfoScreen::DrawLines()
{
	POINT startPoint, endPoint;

	// Set the lind width
	m_SnHelp.SetLineWidth( SN_LINE_WIDTH_6);

	//
	// Draw lines
	//

	// Horizontal
	startPoint.x = 0;
	startPoint.y = 68;
	endPoint.x = 800;
	endPoint.y = 68;
	m_SnHelp.DrawLine( SN_LINE_COLOR, startPoint, endPoint);
}

void CSystemInfoScreen::OnButtonDone() 
{
	// Return 
	CDialog::OnOK();
}

void CSystemInfoScreen::OnButtonReset() 
{
	// Are you sure?
	int nResponse;
    m_yResetDlg = TRUE;
	CMessageBox dlg( m_pControl, SN_SYSTEM_RESET, SN_ARE_YOU_SURE, FALSE, TRUE);
	nResponse = dlg.DoModal();
    m_yResetDlg = FALSE;

	if (nResponse == IDOK)
	{
		if (m_pControl)
		{
            SnWord wPrevShaverPacketCtl, wShaverPacketCtl;
        	CSharedMemory mem;

            // Get Pump status from control layer before reset
		    m_pControl->GetCmdState(GET_SHAVER_PACKET_CTL, &wPrevShaverPacketCtl, sizeof(wPrevShaverPacketCtl));	

			// Reset the system
		    m_pControl->EraseAndRestoreDefaults();
	
            // Get Pump status from control layer after reset
		    m_pControl->GetCmdState(GET_SHAVER_PACKET_CTL, &wShaverPacketCtl, sizeof(wShaverPacketCtl));	
            if( wShaverPacketCtl != wPrevShaverPacketCtl)
            {
		        // Shaver packet routing has changed
                ::PostMessage(HWND_BROADCAST, WM_UPDATE_REMOTE_STATUS, (WPARAM) MSG_UPDATE_REMOTE_PUMP_STATUS,
						    (LPARAM) MSG_PORT_MAPPING_STATUS);
		    }

            // Get PortA and PortB status from the control layer

			m_pControl->GetCmdState(GET_MC_PORTA_STATUS, &m_tPortAStatus, sizeof(SN_PORT_STATUS));
			m_pControl->GetCmdState(GET_MC_PORTB_STATUS, &m_tPortBStatus, sizeof(SN_PORT_STATUS));

			// Set defaults
			m_tPortAStatus.usForward = m_tPortAStatus.usForwardDefault;	// Current Forward setting	
			m_tPortAStatus.usReverse = m_tPortAStatus.usReverseDefault;	// Current Reverse setting
			m_tPortAStatus.usForward2 = m_tPortAStatus.usForward2Default;	// Current Forward setting	
			m_tPortAStatus.usReverse2 = m_tPortAStatus.usReverse2Default;	// Current Reverse setting
			m_tPortAStatus.usOscillateRpm = m_tPortAStatus.usOscillateRpmDefault;// Current Oscillate setting
			m_tPortAStatus.wOscillateSeconds = m_tPortAStatus.wOscillateSecondsDefault;
			m_tPortAStatus.usPercent = m_tPortAStatus.usPercentDefault;	// Current Hand Powered Tool setting

			m_tPortBStatus.usForward = m_tPortBStatus.usForwardDefault;		
			m_tPortBStatus.usReverse = m_tPortBStatus.usReverseDefault;	
			m_tPortBStatus.usForward2 = m_tPortBStatus.usForward2Default;		
			m_tPortBStatus.usReverse2 = m_tPortBStatus.usReverse2Default;	
			m_tPortBStatus.usOscillateRpm = m_tPortBStatus.usOscillateRpmDefault;
			m_tPortBStatus.wOscillateSeconds = m_tPortBStatus.wOscillateSecondsDefault;
			m_tPortBStatus.usPercent = m_tPortBStatus.usPercentDefault;	
	
			// Update the Control Layer
			m_pControl->SetCmdState(SET_MC_PORTA_STATUS, &m_tPortAStatus, sizeof(SN_PORT_STATUS));
			m_pControl->SetCmdState(SET_MC_PORTB_STATUS, &m_tPortBStatus, sizeof(SN_PORT_STATUS));
        }
	}
	else if (nResponse == IDCANCEL)
	{
		// Don't do anything	
	}	
}


void CSystemInfoScreen::UpdateHandPieceVersion(long lPort, SN_SYS_REVISION &tRevision)
{
	SnByte major;
	SnByte minor;
	SnByte build;
	CStatic * staticVerPort = NULL;
	CStatic * staticString = NULL;
    SnQByte qPort = 0; 
	SnWord dlgItemId; 

	if( lPort == PORTA)
	{
		dlgItemId = IDC_STATIC_VERSION_PORTA;
		major = tRevision.ucPortAMajor;
		minor = tRevision.ucPortAMinor;
		build = tRevision.ucPortABuild;
		staticVerPort = &m_StaticVerPortA;
		staticString = &m_Static6;
        qPort = GET_MC_PORTA_STATUS;
	}
	else if( lPort == PORTB)
	{
		dlgItemId = IDC_STATIC_VERSION_PORTB;
		major = tRevision.ucPortBMajor;
		minor = tRevision.ucPortBMinor;
		build = tRevision.ucPortBBuild;
		staticVerPort = &m_StaticVerPortB;
		staticString = &m_Static7;
        qPort = GET_MC_PORTB_STATUS;
	}
	else
	{
		return;
	}

	if( major != 0 || minor != 0 || build != 0)
	{
        SN_PORT_STATUS tPortStatus;
		CString csTemp;
		
	    m_pControl->GetCmdState(qPort, &tPortStatus, sizeof(SN_PORT_STATUS));

		// Display the 485 Handpiece version
        if (tPortStatus.pcSerialNumber[0] != 0) {
            CString csSerialNumber = tPortStatus.pcSerialNumber;
            csTemp.Format(_T("%d.%02d.%02d  SN %s"), major, minor, build, csSerialNumber);
        } else {
            csTemp.Format(_T("%d.%02d.%02d"), major, minor, build);
        }

        SetDlgItemText( dlgItemId, csTemp);

        staticVerPort->ShowWindow(SW_SHOW);
		staticString->ShowWindow(SW_SHOW);

        if(lPort == PORTA)
            m_bHandpieceSerialNumberFlagPortA = TRUE;            

        if(lPort == PORTB)
            m_bHandpieceSerialNumberFlagPortB = TRUE;
	}
	else
	{
		// Hide static windows
		staticVerPort->ShowWindow(SW_HIDE);
		staticString->ShowWindow(SW_HIDE);
	}
}


void CSystemInfoScreen::OnButtonHandpieceSerialNumberSettingsPortA()
{
    CSerialNumberPopUp dlg(m_pControl, 1);
    dlg.DoModal();
}

void CSystemInfoScreen::OnButtonHandpieceSerialNumberSettingsPortB()
{
    CSerialNumberPopUp dlg(m_pControl, 2);
    dlg.DoModal();
}