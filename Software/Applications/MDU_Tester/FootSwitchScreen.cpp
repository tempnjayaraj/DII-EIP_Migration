// FootSwitchScreen.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "FootSwitchScreen.h"
#include "MessageBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFootSwitchScreen dialog


CFootSwitchScreen::CFootSwitchScreen(CControl* pControl, CWnd* pParent /*=NULL*/)
	: CDialog(CFootSwitchScreen::IDD, pParent)
{
	m_pParent = pParent;
	m_pControl = pControl;
	m_bMode = TRUE;
	m_bForward = TRUE;
	m_bHandCtl = TRUE;
	m_bPortCtl = TRUE;

	// create brush for background color
	m_hbrBlack = CreateSolidBrush(SN_BKGND_COLOR);
	
	//{{AFX_DATA_INIT(CFootSwitchScreen)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}
CFootSwitchScreen::~CFootSwitchScreen()
{
	DeInit();
}
void CFootSwitchScreen::DeInit()
{
	// delete brush objects
	DeleteObject( m_hbrBlack);
}
void CFootSwitchScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFootSwitchScreen)
	DDX_Control(pDX, IDC_BUTTON_SET, m_BtnSet);
	DDX_Control(pDX, IDC_BUTTON_CANCEL, m_BtnCancel);
	DDX_Control(pDX, IDC_STATIC_TEXT4, m_StaticText4);
	DDX_Control(pDX, IDC_STATIC_BUTTON_PORTB, m_StaticButtonPortB);
	DDX_Control(pDX, IDC_STATIC_BUTTON_PORTA, m_StaticButtonPortA);
	DDX_Control(pDX, IDC_STATIC_BUTTON_RIGHT, m_StaticButtonRight);
	DDX_Control(pDX, IDC_STATIC_BUTTON_ON, m_StaticButtonOn);
	DDX_Control(pDX, IDC_STATIC_BUTTON_OFF, m_StaticButtonOff);
	DDX_Control(pDX, IDC_STATIC_BUTTON_LEFT, m_StaticButtonLeft);
	DDX_Control(pDX, IDC_STATIC_BUTTON_MODE2, m_StaticButtonMode2);
	DDX_Control(pDX, IDC_STATIC_BUTTON_MODE1, m_StaticButtonMode1);
	DDX_Control(pDX, IDC_STATIC_TEXT3, m_StaticText3);
	DDX_Control(pDX, IDC_STATIC_TEXT2, m_StaticText2);
	DDX_Control(pDX, IDC_STATIC_TEXT1, m_StaticText1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFootSwitchScreen, CDialog)
	//{{AFX_MSG_MAP(CFootSwitchScreen)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_STATIC_BUTTON_MODE1, OnStaticButtonMode1)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_MODE2, OnStaticButtonMode2)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_LEFT, OnStaticButtonLeft)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_RIGHT, OnStaticButtonRight)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_ON, OnStaticButtonOn)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_OFF, OnStaticButtonOff)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_PORTA, OnStaticButtonPorta)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_PORTB, OnStaticButtonPortb)
	ON_MESSAGE(WM_UPDATE_STATUS, UpdateStatus)
	ON_MESSAGE(WM_INTELLIO_SHAVER_CMD, HandleIntellioShaverCmd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFootSwitchScreen message handlers

HBRUSH CFootSwitchScreen::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{

	HBRUSH hbr;
	
	// Intercept the paint call to change the dialog control colors
	
	// Call the base class implementation first! Otherwise, it may
	// undo what we are trying to accomplish .
	hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	int nCtrlID = pWnd->GetDlgCtrlID();
	switch(nCtrlID)
	{
		case IDC_STATIC_TEXT1:
			if(m_tFootStatus.usType != TYPE_485_ANALOG_FOOTPEDAL &&
               m_tFootStatus.usType != TYPE_ANALOG_FOOTPEDAL &&
               m_tFootStatus.usType != TYPE_WIRELESS_FOOTPEDAL)
			{
				m_StaticText1.ShowWindow(SW_HIDE);
			}
			else
			{
				m_StaticText1.ShowWindow(SW_SHOW);
				pDC->SetBkColor(SN_BKGND_COLOR);
				pDC->SetTextColor(WHITE);
				hbr = m_hbrBlack;
			}
			break;

		case IDC_STATIC_TEXT2:
			if(m_tFootStatus.usType != TYPE_485_ANALOG_FOOTPEDAL &&
               m_tFootStatus.usType != TYPE_WIRELESS_FOOTPEDAL)
			{
				m_StaticText2.ShowWindow(SW_HIDE);
				
			}
			else
			{
				m_StaticText2.ShowWindow(SW_SHOW);
				pDC->SetBkColor(SN_BKGND_COLOR);
				pDC->SetTextColor(WHITE);
				hbr = m_hbrBlack;
			}
			break;

	
		default :
			pDC->SetBkColor(SN_BKGND_COLOR);
			pDC->SetTextColor(WHITE);
			hbr = m_hbrBlack;
			break;

	}
	
	return hbr;	
		
}	

LRESULT CFootSwitchScreen::UpdateStatus(WPARAM iParam, LPARAM lParam)
{

	switch( iParam)
	{
	case MSG_UPDATE_ALL_SETTINGS:
        GetPortStatus();

        SetupDefaults();
        SetupTextButtons();
        m_BtnSet.RedrawWindow();
        m_BtnCancel.RedrawWindow();

        m_bMode = TRUE;
		m_bForward = TRUE;
        m_bHandCtl = TRUE;
        m_bPortCtl = TRUE;
	    DrawButtons();
        break;

	case MSG_UPDATE_FOOT_STATUS:
        GetPortStatus();

        m_bMode = TRUE;
		m_bForward = TRUE;
        m_bHandCtl = TRUE;
        m_bPortCtl = TRUE;
		DrawButtons(); //  redraw buttons
		m_bMode = TRUE;
		m_bForward = TRUE;
        m_bHandCtl = TRUE;
        m_bPortCtl = TRUE;
		DrawButtons(); //  redraw buttons NOTE: Must be redrawn twice 
		m_StaticText1.RedrawWindow();
		m_StaticText2.RedrawWindow();
		m_StaticText1.ShowWindow(SW_SHOW);
		m_StaticText2.ShowWindow(SW_SHOW);
		break;
	
		
	default:
		break;
 
	}// end switch

	return 0L;
}

LRESULT CFootSwitchScreen::HandleIntellioShaverCmd( WPARAM iParam, LPARAM lParam)
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

BOOL CFootSwitchScreen::OnInitDialog() 
{
	CDialog::OnInitDialog();

	GetPortStatus();
	
	SetupFonts();
	SetupDefaults();
    SetupTextButtons();
	SetupBitmaps();
	DrawLines();

	DrawButtons();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CFootSwitchScreen::SetupFonts()
{
	CSharedMemory mem;

	m_StaticText1.SetFont(mem.m_Font20Normal, TRUE);
	m_StaticText2.SetFont(mem.m_Font20Normal, TRUE);
	m_StaticText3.SetFont(mem.m_Font20Normal, TRUE);
	m_StaticText4.SetFont(mem.m_Font20Normal, TRUE);
}

void CFootSwitchScreen::SetupDefaults()
{
	::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,(WPARAM)MSG_UPDATE_TITLE, (LPARAM)SN_FOOTSWITCH);

	SetDlgItemText( IDC_STATIC_TEXT1, m_SnHelp.GetString(SN_MODE));
	SetDlgItemText( IDC_STATIC_TEXT2, m_SnHelp.GetString(SN_FORWARD));
	SetDlgItemText( IDC_STATIC_TEXT3, m_SnHelp.GetString(SN_HAND_CONTROL_OVERRIDE));
	SetDlgItemText( IDC_STATIC_TEXT4, m_SnHelp.GetString(SN_PORT_CONTROL));	
}

void CFootSwitchScreen::SetupTextButtons()
{
	CSharedMemory mem;
	
	// Setup SET button
	m_BtnSet.LoadBitmaps(IDB_BITMAP_BK_BUTTON, IDB_BITMAP_BK_BUTTON_PRESSED,
        mem.m_Font20Normal, m_SnHelp.GetString(SN_SET));

	// Setup CANCEL button
	m_BtnCancel.LoadBitmaps(IDB_BITMAP_BK_BUTTON, IDB_BITMAP_BK_BUTTON_PRESSED,
        mem.m_Font20Normal, m_SnHelp.GetString(SN_CANCEL));
}

void CFootSwitchScreen::SetupBitmaps()
{
	CSharedMemory mem;
	
	m_SnHelp.LoadBitmap(&m_ButtonOn, IDB_BITMAP_GRAY_BUTTON_ON);
	m_SnHelp.LoadBitmap(&m_ButtonOff, IDB_BITMAP_GRAY_BUTTON_OFF);
	m_SnHelp.LoadBitmap(&m_ButtonDisabled, IDB_BITMAP_GRAY_BUTTON_DISABLED);
}

void CFootSwitchScreen::OnButtonCancel() 
{
	// Return to previous screen
	CDialog::OnCancel();
}

void CFootSwitchScreen::OnButtonSet() 
{
	if( m_pControl)
    {
		SnBool bStatus;

		// Set status
		m_pControl->SetCmdState(SET_MC_FOOT_STATUS, &m_tFootStatus, sizeof(SN_FOOT_STATUS));

		SnWord usTemp = SAVE_NVRAM;
		// Save the settings
		bStatus = m_pControl->SetCmdState(SET_SYSTEM_PARAMETERS, &usTemp, sizeof(usTemp));
		if( !bStatus)
		{
			CMessageBox dlg( m_pControl, SN_SYSTEM_ERROR, SN_CUSTOM_SETTINGS_SAVE_FAILURE);
			dlg.DoModal();
		}
	}
	
	// Return to previous screen
	CDialog::OnOK();
}

void CFootSwitchScreen::DrawLines()
{
	POINT startPoint, endPoint;

	// Set the lind width
	m_SnHelp.SetLineWidth( SN_LINE_WIDTH_6);


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

	// Horizontal line
	startPoint.x = 220;
	startPoint.y = 179;
	endPoint.x = 800;
	endPoint.y = 179;
	m_SnHelp.DrawLine( SN_LINE_COLOR, startPoint, endPoint);

	// Horizontal line
	startPoint.x = 220;
	startPoint.y = 290;
	endPoint.x = 800;
	endPoint.y = 290;
	m_SnHelp.DrawLine( SN_LINE_COLOR, startPoint, endPoint);
	
	// Draw Vertical Line
	startPoint.x = 220;
	startPoint.y = 68;
	endPoint.x = 220;
	endPoint.y = 403;
	m_SnHelp.DrawLine( SN_LINE_COLOR, startPoint, endPoint);
}

void CFootSwitchScreen::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	DrawLines();
	m_bMode = TRUE;
	m_bForward = TRUE;
	m_bHandCtl = TRUE;
	m_bPortCtl = TRUE;
	DrawButtons();

	// Do not call CDialog::OnPaint() for painting messages
}
void CFootSwitchScreen::GetPortStatus()
{

	// Get the port status
	if( m_pControl)
    {
		m_pControl->GetCmdState(GET_MC_FOOT_STATUS, &m_tFootStatus, sizeof(SN_FOOT_STATUS));
	
		// Get the current language selection
	    SnBool bStatus = m_pControl->GetCmdState(GET_SYSTEM_LANGUAGE, &m_usLanguage, sizeof(m_usLanguage));
		if(!bStatus)
			m_usLanguage = LANGUAGE_ENGLISH;
	
	}
	else
		m_usLanguage = LANGUAGE_ENGLISH;

}
void CFootSwitchScreen::DrawButtons()
{
	CSharedMemory mem;
	
	// Draw buttons

	if( m_tFootStatus.usMode == FOOT_MODE_ON_OFF && m_bMode == TRUE &&
	  (m_tFootStatus.usType == TYPE_485_ANALOG_FOOTPEDAL ||
       m_tFootStatus.usType == TYPE_ANALOG_FOOTPEDAL ||
       m_tFootStatus.usType == TYPE_WIRELESS_FOOTPEDAL)) 
	{
		// On/Off mode selected
		m_StaticButtonMode1.ShowWindow( SW_SHOW);	
		m_StaticButtonMode2.ShowWindow( SW_SHOW);
		
		DWORD yPos;
		if( m_usLanguage == LANGUAGE_FRENCH)
			yPos = 12;
		else if(m_usLanguage == LANGUAGE_PORTUGUESE)
			yPos = 7;
		else
			yPos = 18;

		m_StaticButtonMode1.SetBitmap( m_ButtonOn);
		m_SnHelp.DrawTextOnStaticMulti(&m_StaticButtonMode1,
									      mem.m_Font16Normal,
									      m_SnHelp.GetString(SN_ON_OFF),
									      SN_BKGND_COLOR, yPos);
		
	
		m_StaticButtonMode2.SetBitmap( m_ButtonOff);	
	    m_SnHelp.DrawTextOnStaticSingle(&m_StaticButtonMode2,
										      mem.m_Font16Normal,
										      m_SnHelp.GetString(SN_VARIABLE),
										      WHITE);

		m_bMode = FALSE;
	}
	else if( m_tFootStatus.usMode == FOOT_MODE_VARIABLE && m_bMode == TRUE &&
	       (m_tFootStatus.usType == TYPE_485_ANALOG_FOOTPEDAL ||
            m_tFootStatus.usType == TYPE_ANALOG_FOOTPEDAL ||
            m_tFootStatus.usType == TYPE_WIRELESS_FOOTPEDAL))
	{ 
		// Variable Mode selected
		m_StaticButtonMode1.ShowWindow( SW_SHOW);	
		m_StaticButtonMode2.ShowWindow( SW_SHOW);
		
		m_StaticButtonMode2.SetBitmap( m_ButtonOn);	
	    m_SnHelp.DrawTextOnStaticSingle(&m_StaticButtonMode2,
										      mem.m_Font16Normal,
										      m_SnHelp.GetString(SN_VARIABLE),
										      SN_BKGND_COLOR);
		DWORD yPos;
		if( m_usLanguage == LANGUAGE_FRENCH)
			yPos = 12;
		else if(m_usLanguage == LANGUAGE_PORTUGUESE)
			yPos = 7;
		else
			yPos = 18;

		m_StaticButtonMode1.SetBitmap( m_ButtonOff);
		m_SnHelp.DrawTextOnStaticMulti(&m_StaticButtonMode1,
									      mem.m_Font16Normal,
									      m_SnHelp.GetString(SN_ON_OFF),
									      WHITE, yPos);

		m_bMode = FALSE;
	}
	else if (m_bMode)
	{
		m_StaticButtonMode1.ShowWindow( SW_HIDE);	
		m_StaticButtonMode2.ShowWindow( SW_HIDE);
	}


	if( m_tFootStatus.usForward == FOOT_FORWARD_LEFT && m_bForward == TRUE &&
        (m_tFootStatus.usType == TYPE_485_ANALOG_FOOTPEDAL ||
         m_tFootStatus.usType == TYPE_WIRELESS_FOOTPEDAL)) 
	{
		// Forward Left selected
		m_StaticButtonRight.ShowWindow(SW_SHOW);
		m_StaticButtonLeft.ShowWindow(SW_SHOW);

		m_StaticButtonLeft.SetBitmap( m_ButtonOn);
	    m_SnHelp.DrawTextOnStaticSingle(&m_StaticButtonLeft,
										      mem.m_Font16Normal,
										      m_SnHelp.GetString(SN_LEFT),
										      SN_BKGND_COLOR);
	
		m_StaticButtonRight.SetBitmap( m_ButtonOff);	
	    m_SnHelp.DrawTextOnStaticSingle(&m_StaticButtonRight,
										      mem.m_Font16Normal,
										      m_SnHelp.GetString(SN_RIGHT),
										      WHITE);
		m_bForward = FALSE;
	}
	else if( m_tFootStatus.usForward == FOOT_FORWARD_RIGHT && m_bForward == TRUE &&
        (m_tFootStatus.usType == TYPE_485_ANALOG_FOOTPEDAL ||
         m_tFootStatus.usType == TYPE_WIRELESS_FOOTPEDAL))
	{ 
		// Forward Right selected
		m_StaticButtonRight.ShowWindow(SW_SHOW);
		m_StaticButtonLeft.ShowWindow(SW_SHOW);
		
		m_StaticButtonRight.SetBitmap( m_ButtonOn);	
	    m_SnHelp.DrawTextOnStaticSingle(&m_StaticButtonRight,
										      mem.m_Font16Normal,
										      m_SnHelp.GetString(SN_RIGHT),
										      SN_BKGND_COLOR);

		m_StaticButtonLeft.SetBitmap( m_ButtonOff);
	    m_SnHelp.DrawTextOnStaticSingle(&m_StaticButtonLeft,
										      mem.m_Font16Normal,
										      m_SnHelp.GetString(SN_LEFT),
										      WHITE);
		m_bForward = FALSE;
	}
	else if( m_bForward)
	{
		m_StaticButtonRight.ShowWindow(SW_HIDE);
		m_StaticButtonLeft.ShowWindow(SW_HIDE);
		
	}


	if( m_tFootStatus.usOverride == FOOT_HAND_OVERRIDE_ON && m_bHandCtl == TRUE) 
	{
		// Hand Control override On selected
		m_StaticButtonOn.SetBitmap( m_ButtonOn);
	    m_SnHelp.DrawTextOnStaticSingle(&m_StaticButtonOn,
										      mem.m_Font16Normal,
										      m_SnHelp.GetString(SN_ON),
										      SN_BKGND_COLOR);
	
		m_StaticButtonOff.SetBitmap( m_ButtonOff);	
	    m_SnHelp.DrawTextOnStaticSingle(&m_StaticButtonOff,
										      mem.m_Font16Normal,
										      m_SnHelp.GetString(SN_OFF),
										      WHITE);
		m_bHandCtl = FALSE;
	}
	else if( m_tFootStatus.usOverride == FOOT_HAND_OVERRIDE_OFF && m_bHandCtl == TRUE) 
	{ 
		// Hand Control override Off selected
		m_StaticButtonOff.SetBitmap( m_ButtonOn);	
	    m_SnHelp.DrawTextOnStaticSingle(&m_StaticButtonOff,
										      mem.m_Font16Normal,
										      m_SnHelp.GetString(SN_OFF),
										      SN_BKGND_COLOR);

		m_StaticButtonOn.SetBitmap( m_ButtonOff);
	    m_SnHelp.DrawTextOnStaticSingle(&m_StaticButtonOn,
										      mem.m_Font16Normal,
										      m_SnHelp.GetString(SN_ON),
										      WHITE);
		m_bHandCtl = FALSE;
	}

	if( m_tFootStatus.usPortControl == PORTA && m_bPortCtl == TRUE) 
	{
		DWORD yPos;

		if( m_usLanguage == LANGUAGE_GERMAN)
			yPos = 12;
		else
			yPos = 18;
		
		
		// PortA Port Control selected
		m_StaticButtonPortA.SetBitmap( m_ButtonOn);
	    m_SnHelp.DrawTextOnStaticMulti(&m_StaticButtonPortA,
										      mem.m_Font16Normal,
										      m_SnHelp.GetString(SN_PORTA),
										      SN_BKGND_COLOR, yPos);
	
		m_StaticButtonPortB.SetBitmap( m_ButtonOff);	
	    m_SnHelp.DrawTextOnStaticMulti(&m_StaticButtonPortB,
										      mem.m_Font16Normal,
										      m_SnHelp.GetString(SN_PORTB),
										      WHITE, yPos);
		m_bPortCtl = FALSE;
	}
	else if( m_tFootStatus.usPortControl == PORTB && m_bPortCtl == TRUE) 
	{ 
		DWORD yPos;

		if( m_usLanguage == LANGUAGE_GERMAN)
			yPos = 12;
		else
			yPos = 18;
		
		// PortB Port Control selected
		m_StaticButtonPortB.SetBitmap( m_ButtonOn);	
	    m_SnHelp.DrawTextOnStaticMulti(&m_StaticButtonPortB,
										      mem.m_Font16Normal,
										      m_SnHelp.GetString(SN_PORTB),
										      SN_BKGND_COLOR, yPos);

		m_StaticButtonPortA.SetBitmap( m_ButtonOff);
	    m_SnHelp.DrawTextOnStaticMulti(&m_StaticButtonPortA,
										      mem.m_Font16Normal,
										      m_SnHelp.GetString(SN_PORTA),
										      WHITE, yPos);
		m_bPortCtl = FALSE;
	}
 
}

void CFootSwitchScreen::OnStaticButtonMode1() 
{

	m_tFootStatus.usMode = FOOT_MODE_ON_OFF;
	m_bMode = TRUE;
	DrawButtons();

		
}

void CFootSwitchScreen::OnStaticButtonMode2() 
{
	m_tFootStatus.usMode = FOOT_MODE_VARIABLE;
	m_bMode = TRUE;
	DrawButtons();	

}

void CFootSwitchScreen::OnStaticButtonLeft() 
{
	if( m_tFootStatus.usType == TYPE_485_ANALOG_FOOTPEDAL ||
        m_tFootStatus.usType == TYPE_WIRELESS_FOOTPEDAL)
	{
		m_tFootStatus.usForward = FOOT_FORWARD_LEFT;
		m_bForward = TRUE;
		DrawButtons();
	}
	
}

void CFootSwitchScreen::OnStaticButtonRight() 
{
	if( m_tFootStatus.usType == TYPE_485_ANALOG_FOOTPEDAL ||
        m_tFootStatus.usType == TYPE_WIRELESS_FOOTPEDAL)
	{
		m_tFootStatus.usForward = FOOT_FORWARD_RIGHT;
		m_bForward = TRUE;
		DrawButtons();
	}
	
}

void CFootSwitchScreen::OnStaticButtonOn() 
{
	m_tFootStatus.usOverride = FOOT_HAND_OVERRIDE_ON;
	m_bHandCtl = TRUE;
	DrawButtons();
	
}

void CFootSwitchScreen::OnStaticButtonOff() 
{
	m_tFootStatus.usOverride = FOOT_HAND_OVERRIDE_OFF;
	m_bHandCtl = TRUE;
	DrawButtons();
	
}

void CFootSwitchScreen::OnStaticButtonPorta() 
{
	m_tFootStatus.usPortControl = PORTA;
	m_bPortCtl = TRUE;
	DrawButtons();
	
}

void CFootSwitchScreen::OnStaticButtonPortb() 
{
	m_tFootStatus.usPortControl = PORTB;
	m_bPortCtl = TRUE;
	DrawButtons();
	
}
