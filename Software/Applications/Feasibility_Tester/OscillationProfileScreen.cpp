// OscillationProfileScreen.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "OscillationProfileScreen.h"
#include "CustomOscScreen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COscillationProfileScreen dialog


COscillationProfileScreen::COscillationProfileScreen(CControl* pControl, CWnd* pParent /*=NULL*/)
	: CDialog(COscillationProfileScreen::IDD, pParent)
{	
	m_pParent = pParent;
	m_pControl = pControl;

	// create brush for background color
	m_hbr = CreateSolidBrush(SN_BKGND_COLOR);
	
	//{{AFX_DATA_INIT(COscillationProfileScreen)
	//}}AFX_DATA_INIT
}

COscillationProfileScreen::~COscillationProfileScreen()
{
    DeInit();
}

void COscillationProfileScreen::DeInit()
{
	// delete brush objects
	DeleteObject( m_hbr);
}

void COscillationProfileScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COscillationProfileScreen)
	DDX_Control(pDX, IDC_BUTTON_DONE, m_BtnDone);
	DDX_Control(pDX, IDC_BUTTON_ADJUST_PORTB, m_BtnAdjustPortB);
	DDX_Control(pDX, IDC_BUTTON_ADJUST_PORTA, m_BtnAdjustPortA);
	DDX_Control(pDX, IDC_STATIC_TEXT2, m_StaticText2);
	DDX_Control(pDX, IDC_STATIC_TEXT1, m_StaticText1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COscillationProfileScreen, CDialog)
	//{{AFX_MSG_MAP(COscillationProfileScreen)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_DONE, OnButtonDone)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_ADJUST_PORTA, OnButtonAdjustPortA)
	ON_BN_CLICKED(IDC_BUTTON_ADJUST_PORTB, OnButtonAdjustPortB)
	ON_MESSAGE(WM_UPDATE_STATUS, UpdateStatus)
	ON_MESSAGE(WM_INTELLIO_SHAVER_CMD, HandleIntellioShaverCmd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COscillationProfileScreen message handlers

HBRUSH COscillationProfileScreen::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
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

LRESULT COscillationProfileScreen::UpdateStatus(WPARAM iParam, LPARAM lParam)
{

	switch( iParam)
	{
	case MSG_UPDATE_ALL_SETTINGS:
	    GetPortStatus();
	    SetupTextButtons();
	    m_BtnDone.RedrawWindow();
	    m_BtnAdjustPortA.RedrawWindow();
        m_BtnAdjustPortB.RedrawWindow();
	    SetupDefaults();
        break;

    default:
		break;
 
	}// end switch

	return 0L;
}

LRESULT COscillationProfileScreen::HandleIntellioShaverCmd( WPARAM iParam, LPARAM lParam)
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

BOOL COscillationProfileScreen::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    m_yActiveDialog = TRUE;

	GetPortStatus();

	// Setup the screen
	SetupTextButtons();
	SetupFonts();
	SetupDefaults();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COscillationProfileScreen::SetupTextButtons()
{
	CSharedMemory mem;

	// Setup Done button
	m_BtnDone.LoadBitmaps(IDB_BITMAP_BK_BUTTON, IDB_BITMAP_BK_BUTTON_PRESSED,
        mem.m_Font20Normal, m_SnHelp.GetString(SN_DONE));

    // Setup Adjust Port A button
	m_BtnAdjustPortA.LoadBitmaps(IDB_BITMAP_BUTTON_GRAY, IDB_BITMAP_BUTTON_GRAY_PRESSED,
        mem.m_Font20Normal, m_SnHelp.GetString(SN_ADJUST));

    // Setup Adjust Port B button
	m_BtnAdjustPortB.LoadBitmaps(IDB_BITMAP_BUTTON_GRAY, IDB_BITMAP_BUTTON_GRAY_PRESSED,
        mem.m_Font20Normal, m_SnHelp.GetString(SN_ADJUST));
}


void COscillationProfileScreen::DrawLines() 
{
	POINT startPoint, endPoint;

	// Set the line width
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

	// Horizontal
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

void COscillationProfileScreen::SetupFonts()
{
	CSharedMemory mem;
	
	m_StaticText1.SetFont(mem.m_Font20Bold, TRUE);
	m_StaticText2.SetFont(mem.m_Font20Bold, TRUE);
}

void COscillationProfileScreen::SetupDefaults()
{
	SetDlgItemText(IDC_STATIC_TEXT1, m_SnHelp.GetString(SN_PORTA));
	SetDlgItemText(IDC_STATIC_TEXT2, m_SnHelp.GetString(SN_PORTB));

    if (m_yActiveDialog)
    {
	    // Set the Window Title
	    ::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
            (WPARAM)MSG_UPDATE_TITLE, (LPARAM)SN_OSCILLATE_MODE);
    }
}

void COscillationProfileScreen::OnButtonDone() 
{
	CDialog::OnOK();
}

void COscillationProfileScreen::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	DrawLines();
	
	// Do not call CDialog::OnPaint() for painting messages
}

void COscillationProfileScreen::GetPortStatus()
{
	// Get the port status
	if( m_pControl)
    {
	    m_pControl->GetCmdState(GET_MC_PORTA_STATUS, &m_tPortAStatus, sizeof(SN_PORT_STATUS));
	    m_pControl->GetCmdState(GET_MC_PORTB_STATUS, &m_tPortBStatus, sizeof(SN_PORT_STATUS));
	}
}

void COscillationProfileScreen::OnButtonAdjustPortA() 
{

	GetPortStatus();	

	if( m_tPortAStatus.usOscMode == OSC_MODE1 || m_tPortAStatus.yForceOscMode1)
	{
        m_yActiveDialog = FALSE;

        CCustomOscScreen dlg(m_pControl, OSC_MODE1, PORTA);
		dlg.DoModal();

        m_yActiveDialog = TRUE;
	}
	else if(  m_tPortAStatus.usOscMode == OSC_MODE2)
	{
        m_yActiveDialog = FALSE;

		CCustomOscScreen dlg(m_pControl, OSC_MODE2, PORTA);
		dlg.DoModal();

        m_yActiveDialog = TRUE;
	}
	
	// Set the Window Title
	::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
        (WPARAM)MSG_UPDATE_TITLE, (LPARAM)SN_OSCILLATE_MODE);
}

void COscillationProfileScreen::OnButtonAdjustPortB() 
{
	GetPortStatus();	

	if( m_tPortBStatus.usOscMode == OSC_MODE1 || m_tPortBStatus.yForceOscMode1)
	{
		CCustomOscScreen dlg(m_pControl, OSC_MODE1, PORTB);
		dlg.DoModal();
	}
	else if( m_tPortBStatus.usOscMode == OSC_MODE2)
	{
		CCustomOscScreen dlg(m_pControl, OSC_MODE2, PORTB);
		dlg.DoModal();

	}
	
	// Set the Window Title
	::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
        (WPARAM)MSG_UPDATE_TITLE, (LPARAM)SN_OSCILLATE_MODE);
}
