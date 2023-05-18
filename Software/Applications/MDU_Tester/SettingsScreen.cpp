// SettingsScreen.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "SettingsScreen.h"
#include "RemoteSettings.h"
#include "MessageBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSettingsScreen dialog


CSettingsScreen::CSettingsScreen(CControl* pControl, CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsScreen::IDD, pParent)
{
	m_pParent = pParent;
	m_pControl = pControl;
	m_usSystemMode = DEFAULT_MODE;
    m_yActiveDialog = TRUE;

	// create brush for background color
	m_hbr = CreateSolidBrush(SN_BKGND_COLOR);

	
	//{{AFX_DATA_INIT(CSettingsScreen)
	//}}AFX_DATA_INIT
}

CSettingsScreen::~CSettingsScreen()
{
    DeInit();
}

void CSettingsScreen::DeInit()
{
	// delete brush objects
	DeleteObject( m_hbr);
}

void CSettingsScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingsScreen)
	DDX_Control(pDX, IDC_BUTTON_SYSTEM_INFORMATION, m_BtnSystemInformation);
	DDX_Control(pDX, IDC_BUTTON_RESET_BLADE_B, m_BtnResetB);
	DDX_Control(pDX, IDC_BUTTON_RESET_BLADE_A, m_BtnResetA);
	DDX_Control(pDX, IDC_BUTTON_MODE, m_BtnMode);
	DDX_Control(pDX, IDC_BUTTON_LANGUAGE, m_BtnLanguage);
	DDX_Control(pDX, IDC_BUTTON_INTERFACE, m_BtnInterface);
	DDX_Control(pDX, IDC_BUTTON_FOOTSWITCH, m_BtnFootswitch);
	DDX_Control(pDX, IDC_BUTTON_DONE, m_BtnDone);
	DDX_Control(pDX, IDC_BUTTON_CUSTOM_SETTINGS, m_BtnCustomSettings);
	DDX_Control(pDX, IDC_STATIC_RESET, m_StaticReset);
	DDX_Control(pDX, IDC_STATIC_MODE, m_StaticMode);
	DDX_Control(pDX, IDC_STATIC_INTERFACE, m_StaticInterface);
	DDX_Control(pDX, IDC_STATIC_FOOTSWITCH, m_StaticFootswitch);
	DDX_Control(pDX, IDC_STATIC_SYSTEM_INFORMATION, m_StaticSystemInformation);
	DDX_Control(pDX, IDC_STATIC_LANGUAGE, m_StaticLanguage);
	DDX_Control(pDX, IDC_STATIC_CUSTOM_SETTINGS, m_StaticCustomSettings);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSettingsScreen, CDialog)
	//{{AFX_MSG_MAP(CSettingsScreen)
	ON_WM_CTLCOLOR()
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_BUTTON_DONE, OnButtonDone)
	ON_BN_CLICKED(IDC_BUTTON_CUSTOM_SETTINGS, OnButtonCustomSettings)
	ON_BN_CLICKED(IDC_BUTTON_SYSTEM_INFORMATION, OnButtonSystemInformation)
	ON_BN_CLICKED(IDC_BUTTON_LANGUAGE, OnButtonLanguage)
	ON_BN_CLICKED(IDC_BUTTON_FOOTSWITCH, OnButtonFootswitch)
	ON_BN_CLICKED(IDC_BUTTON_INTERFACE, OnButtonInterface)
	ON_BN_CLICKED(IDC_BUTTON_MODE, OnButtonMode)
	ON_BN_CLICKED(IDC_BUTTON_RESET_BLADE_A, OnButtonResetBladeA)
	ON_MESSAGE(WM_UPDATE_STATUS, UpdateStatus)
	ON_MESSAGE(WM_INTELLIO_SHAVER_CMD, HandleIntellioShaverCmd)
	ON_BN_CLICKED(IDC_BUTTON_RESET_BLADE_B, OnButtonResetBladeB)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsScreen message handlers

HBRUSH CSettingsScreen::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
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

void CSettingsScreen::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CSharedMemory mem;
	
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
		
		if (nIDCtl == IDC_BUTTON_MODE)
		{
			SnWord usLanguage;
			DWORD yPos;

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
		
					
			if( usLanguage == LANGUAGE_DUTCH)
			{
				yPos = 14;	
			}
			else
			{
				yPos = 12;
			}
			
			if( m_usSystemMode == DEFAULT_MODE)
			{
			
				
				m_SnHelp.DrawTextOnStaticMulti((CStatic*)&m_BtnMode,
										  mem.m_Font14Normal,
										  m_SnHelp.GetString(SN_SELECT_BLADE_RECALL),
										  SN_BKGND_COLOR,yPos);
			
				SetDlgItemText( IDC_STATIC_MODE, m_SnHelp.GetString(SN_IN_BLADE_DEFAULT));
			}
			else
			{
				m_SnHelp.DrawTextOnStaticMulti((CStatic*)&m_BtnMode,
										  mem.m_Font14Normal,
										  m_SnHelp.GetString(SN_SELECT_BLADE_DEFAULT),
										  SN_BLACK,yPos);

				SetDlgItemText( IDC_STATIC_MODE, m_SnHelp.GetString(SN_IN_BLADE_RECALL));
			
			}
			DrawLines();
		}
	}
}

LRESULT CSettingsScreen::UpdateStatus(WPARAM iParam, LPARAM lParam)
{
	switch( iParam)
	{		
	case MSG_UPDATE_PORT_STATUS:

	    // Something has changed get PortA and PortB status from the control layer
	    if( m_pControl)
	    {
		    m_pControl->GetCmdState(GET_MC_PORTA_STATUS, &m_tPortAStatus, sizeof(SN_PORT_STATUS));
		    m_pControl->GetCmdState(GET_MC_PORTB_STATUS, &m_tPortBStatus, sizeof(SN_PORT_STATUS));
	    
	    }

		if( m_tPortAStatus.usType == TYPE_INVALID && m_tPortBStatus.usType == TYPE_INVALID)
		{
			// Hide the Blade Reset buttons no devices connected

			m_BtnResetA.ShowWindow(SW_HIDE);
			m_BtnResetB.ShowWindow(SW_HIDE);
			m_StaticReset.ShowWindow(SW_HIDE);
		}
		else if(m_tPortAStatus.usType == TYPE_INVALID && m_tPortBStatus.usType != TYPE_INVALID)
		{
			m_BtnResetA.ShowWindow(SW_HIDE);
			m_BtnResetB.ShowWindow(SW_SHOW);
			m_StaticReset.ShowWindow(SW_SHOW);
		}
		else if(m_tPortBStatus.usType == TYPE_INVALID && m_tPortAStatus.usType != TYPE_INVALID)
		{
			m_BtnResetA.ShowWindow(SW_SHOW);
			m_BtnResetB.ShowWindow(SW_HIDE);
			m_StaticReset.ShowWindow(SW_SHOW);
		}
		else
		{
			m_BtnResetA.ShowWindow(SW_SHOW);
			m_BtnResetB.ShowWindow(SW_SHOW);
			m_StaticReset.ShowWindow(SW_SHOW);
		}

		break;

	case MSG_UPDATE_LANGUAGE:
        SetupTextButtons();
        break;

    case MSG_UPDATE_SYSTEM_MODE:
		if( m_pControl)
			m_pControl->GetCmdState(GET_SYSTEM_MODE, &m_usSystemMode, sizeof(m_usSystemMode));	
        
        m_BtnMode.RedrawWindow(); // redraw button
        break;

    case MSG_UPDATE_ALL_SETTINGS:
        SetupTextButtons();
	    SetupDefaults();
		if( m_pControl)
			m_pControl->GetCmdState(GET_SYSTEM_MODE, &m_usSystemMode, sizeof(m_usSystemMode));	
        m_BtnMode.RedrawWindow();
        m_BtnDone.RedrawWindow();
        break;

    default:
		break;
 
	}// end switch

	return 0L;
}

LRESULT CSettingsScreen::HandleIntellioShaverCmd( WPARAM iParam, LPARAM lParam)
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

BOOL CSettingsScreen::OnInitDialog() 
{
	CDialog::OnInitDialog();

 	// Get PortA and PortB status from the control layer
	if( m_pControl)
	{
		m_pControl->GetCmdState(GET_MC_PORTA_STATUS, &m_tPortAStatus, sizeof(SN_PORT_STATUS));
		m_pControl->GetCmdState(GET_MC_PORTB_STATUS, &m_tPortBStatus, sizeof(SN_PORT_STATUS));
		m_pControl->GetCmdState(GET_SYSTEM_MODE, &m_usSystemMode, sizeof(m_usSystemMode));
	}
	
	// Setup the screen
    SetupTextButtons();
	SetupBitmaps();
	SetupFonts();
	SetupDefaults();

	UpdateStatus(MSG_UPDATE_PORT_STATUS, 0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSettingsScreen::SetupTextButtons()
{
	CSharedMemory mem;

    // Setup Done button
	m_BtnDone.LoadBitmaps(IDB_BITMAP_BK_BUTTON, IDB_BITMAP_BK_BUTTON_PRESSED,
        mem.m_Font20Normal, m_SnHelp.GetString(SN_DONE));

    // Setup Reset Port A button
	m_BtnResetA.LoadBitmaps(IDB_BITMAP_GRAY_ROUND_BLUE_RING, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED,
        mem.m_Font20Normal, m_SnHelp.GetString(SN_A), SN_BLACK);

    // Setup Reset Port B button
	m_BtnResetB.LoadBitmaps(IDB_BITMAP_GRAY_ROUND_YELLOW_RING, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED,
        mem.m_Font20Normal, m_SnHelp.GetString(SN_B), SN_BLACK);
}

void CSettingsScreen::SetupBitmaps()
{
	// Setup Custom Settings button
	m_BtnCustomSettings.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED);

	// Setup Footswitch button 
	m_BtnFootswitch.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED);

	// Setup Interface button
	m_BtnInterface.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED);

	// Setup System Information button
	m_BtnSystemInformation.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED);

	// Setup Language button
	m_BtnLanguage.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED);

	// Setup Mode button
	m_BtnMode.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_LARGE, IDB_BITMAP_GRAY_BUTTON_LARGE_PRESSED);

}

void CSettingsScreen::SetupFonts()
{
	CSharedMemory mem;
	
	// fixed text
	m_StaticCustomSettings.SetFont(mem.m_Font25Normal, TRUE);
	m_StaticSystemInformation.SetFont(mem.m_Font25Normal, TRUE);
	m_StaticLanguage.SetFont(mem.m_Font25Normal, TRUE);
	m_StaticFootswitch.SetFont( mem.m_Font25Normal, TRUE);
	m_StaticInterface.SetFont( mem.m_Font25Normal, TRUE);
	m_StaticMode.SetFont(mem.m_Font16Normal, TRUE);
	m_StaticReset.SetFont( mem.m_Font20Normal, TRUE);
}

void CSettingsScreen::SetupDefaults()
{
    // Setup static text boxes
	SetDlgItemText( IDC_STATIC_CUSTOM_SETTINGS, m_SnHelp.GetString(SN_OSCILLATE_MODE));
	SetDlgItemText( IDC_STATIC_FOOTSWITCH, m_SnHelp.GetString(SN_FOOTSWITCH));
	SetDlgItemText( IDC_STATIC_SYSTEM_INFORMATION, m_SnHelp.GetString(SN_SYSTEM_INFORMATION));
	SetDlgItemText( IDC_STATIC_LANGUAGE, m_SnHelp.GetString(SN_LANGUAGE));
	SetDlgItemText( IDC_STATIC_INTERFACE, m_SnHelp.GetString(SN_INTERFACE));
	SetDlgItemText( IDC_STATIC_RESET, m_SnHelp.GetString(SN_BLADE_RESET));

	if (m_yActiveDialog)
    {
        ::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
                (WPARAM)MSG_UPDATE_TITLE, (LPARAM)SN_SETTINGS);
    }
}

void CSettingsScreen::DrawLines()
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


	// Draw Vertical Line
	startPoint.x = 560;
	startPoint.y = 68;
	endPoint.x = 560;
	endPoint.y = 148;
	m_SnHelp.DrawLine( SN_LINE_COLOR, startPoint, endPoint);

	// Horizontal
	startPoint.x = 560;
	startPoint.y = 148;
	endPoint.x = 800;
	endPoint.y = 148;
	m_SnHelp.DrawLine( SN_LINE_COLOR, startPoint, endPoint);

}

void CSettingsScreen::OnButtonDone() 
{
    ::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
        (WPARAM)MSG_UPDATE_TITLE, (LPARAM)SN_NO_TITLE);

	// Return 
	CDialog::OnOK();	
}

void CSettingsScreen::OnButtonCustomSettings() 
{
    m_yActiveDialog = FALSE;

    COscillationProfileScreen dlg( m_pControl);
	int nResponse = dlg.DoModal();
	
    m_yActiveDialog = TRUE;


    ::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
        (WPARAM)MSG_UPDATE_TITLE, (LPARAM)SN_SETTINGS);
}

void CSettingsScreen::OnButtonSystemInformation() 
{
    m_yActiveDialog = FALSE;

    CSystemInfoScreen dlg(m_pControl);
	int nResponse = dlg.DoModal();

    m_yActiveDialog = TRUE;

	 // Get PortA and PortB status from the control layer in case the system was reset to factory defaults
	if( m_pControl)
	{
		m_pControl->GetCmdState(GET_MC_PORTA_STATUS, &m_tPortAStatus, sizeof(SN_PORT_STATUS));
		m_pControl->GetCmdState(GET_MC_PORTB_STATUS, &m_tPortBStatus, sizeof(SN_PORT_STATUS));
		m_pControl->GetCmdState(GET_SYSTEM_MODE, &m_usSystemMode, sizeof(m_usSystemMode));
	}

	// Update the Screen in case the language changed
	SetupDefaults();
	
	m_BtnMode.RedrawWindow(); // redraw button

}

void CSettingsScreen::OnButtonLanguage() 
{
    m_yActiveDialog = FALSE;
	
  	CLanguageScreen dlg(m_pControl);
	int nResponse = dlg.DoModal();

    m_yActiveDialog = TRUE;

    // Update the Screen in case the language changed
	SetupDefaults();
}

void CSettingsScreen::OnButtonFootswitch() 
{
    m_yActiveDialog = FALSE;

    CFootSwitchScreen dlg( m_pControl);
	int nResponse = dlg.DoModal();
	
    m_yActiveDialog = TRUE;

    ::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,(WPARAM)MSG_UPDATE_TITLE, (LPARAM)SN_SETTINGS);		
}

void CSettingsScreen::OnButtonInterface() 
{
    m_yActiveDialog = FALSE;

	CRemoteSettings dlg(m_pControl);
	int nResponse = dlg.DoModal();
	
    m_yActiveDialog = TRUE;
	
    ::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,(WPARAM)MSG_UPDATE_TITLE, (LPARAM)SN_SETTINGS);
}

void CSettingsScreen::OnButtonMode() 
{
	if( m_usSystemMode == DEFAULT_MODE)
		m_usSystemMode = CUSTOM_MODE;
	else
		m_usSystemMode = DEFAULT_MODE;

	if( m_pControl)
    {
		SnBool bStatus;
		// Set the mode
		m_pControl->SetCmdState(SET_SYSTEM_MODE, &m_usSystemMode, sizeof(m_usSystemMode));
		
		SnWord usTemp = SAVE_NVRAM;
		// Save the settings
		bStatus = m_pControl->SetCmdState(SET_SYSTEM_PARAMETERS, &usTemp, sizeof(usTemp));
		if( !bStatus)
		{
			CMessageBox dlg( m_pControl, SN_SYSTEM_ERROR, SN_CUSTOM_SETTINGS_SAVE_FAILURE);
			dlg.DoModal();
		}
	}

	m_BtnMode.RedrawWindow(); // redraw button
}

void CSettingsScreen::OnButtonResetBladeA() 
{
	// Get PortA status from the control layer
	if( m_pControl)
	{
		m_pControl->GetCmdState(GET_MC_PORTA_STATUS, &m_tPortAStatus, sizeof(SN_PORT_STATUS));
	}
	
	// Set defaults
	m_tPortAStatus.usForward = m_tPortAStatus.usForwardDefault;	// Current Forward setting	
	m_tPortAStatus.usReverse = m_tPortAStatus.usReverseDefault;	// Current Reverse setting
	m_tPortAStatus.usForward2 = m_tPortAStatus.usForward2Default;	// Current Forward setting	
	m_tPortAStatus.usReverse2 = m_tPortAStatus.usReverse2Default;	// Current Reverse setting
	m_tPortAStatus.usOscillateRpm = m_tPortAStatus.usOscillateRpmDefault;// Current Oscillate setting
	m_tPortAStatus.wOscillateSeconds = m_tPortAStatus.wOscillateSecondsDefault;
	m_tPortAStatus.usPercent = m_tPortAStatus.usPercentDefault;	// Current Hand Powered Tool setting

	if (m_pControl) 
	{
		// Update the Control Layer
		m_pControl->SetCmdState(SET_MC_PORTA_STATUS, &m_tPortAStatus, sizeof(SN_PORT_STATUS));

		SnWord usTemp = SAVE_FLASH;
		// Save the settings
		m_pControl->SetCmdState(SET_SYSTEM_PARAMETERS, &usTemp, sizeof(usTemp));
	}     
	
}

void CSettingsScreen::OnButtonResetBladeB() 
{
	// Get PortB status from the control layer
	if( m_pControl)
	{
		m_pControl->GetCmdState(GET_MC_PORTB_STATUS, &m_tPortBStatus, sizeof(SN_PORT_STATUS));
	}
	
	// Set defaults
	m_tPortBStatus.usForward = m_tPortBStatus.usForwardDefault;		
	m_tPortBStatus.usReverse = m_tPortBStatus.usReverseDefault;	
	m_tPortBStatus.usForward2 = m_tPortBStatus.usForward2Default;		
	m_tPortBStatus.usReverse2 = m_tPortBStatus.usReverse2Default;	
	m_tPortBStatus.usOscillateRpm = m_tPortBStatus.usOscillateRpmDefault;
	m_tPortBStatus.wOscillateSeconds = m_tPortBStatus.wOscillateSecondsDefault;
	m_tPortBStatus.usPercent = m_tPortBStatus.usPercentDefault;	
	
	if (m_pControl) 
	{
		// Update the Control Layer
		m_pControl->SetCmdState(SET_MC_PORTB_STATUS, &m_tPortBStatus, sizeof(SN_PORT_STATUS));

		SnWord usTemp = SAVE_FLASH;
		// Save the settings
		m_pControl->SetCmdState(SET_SYSTEM_PARAMETERS, &usTemp, sizeof(usTemp));
	}     
}
