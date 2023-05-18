// CustomOscScreen.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "CustomOscScreen.h"
#include "MessageBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCustomOscScreen dialog


CCustomOscScreen::CCustomOscScreen(CControl* pControl,DWORD dwMode, DWORD dwPort, CWnd* pParent /*=NULL*/)
	: CDialog(CCustomOscScreen::IDD, pParent)
{

	m_pParent = pParent;
	m_pControl = pControl;
	m_dwPort = dwPort;
	m_dwMode = dwMode;

	// create brush for background color
	m_hbrBlack = CreateSolidBrush(SN_BKGND_COLOR);
	
	//{{AFX_DATA_INIT(CCustomOscScreen)
	//}}AFX_DATA_INIT
}
CCustomOscScreen::~CCustomOscScreen()
{
	DeInit();
}
void CCustomOscScreen::DeInit()
{
	// delete brush objects
	DeleteObject( m_hbrBlack);
}
void CCustomOscScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustomOscScreen)
	DDX_Control(pDX, IDC_BUTTON_SET, m_BtnSet);
	DDX_Control(pDX, IDC_BUTTON_DEFAULT, m_BtnDefault);
	DDX_Control(pDX, IDC_BUTTON_CANCEL, m_BtnCancel);
	DDX_Control(pDX, IDC_STATIC_TEXT3, m_StaticText3);
	DDX_Control(pDX, IDC_STATIC_TEXT2, m_StaticText2);
	DDX_Control(pDX, IDC_STATIC_TEXT, m_StaticText);
	DDX_Control(pDX, IDC_STATIC_BUTTON_UP, m_ButtonUp);
	DDX_Control(pDX, IDC_STATIC_BUTTON_DOWN, m_ButtonDown);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCustomOscScreen, CDialog)
	//{{AFX_MSG_MAP(CCustomOscScreen)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_DEFAULT, OnButtonDefault)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_DOWN, OnStaticButtonDown)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_UP, OnStaticButtonUp)
	ON_MESSAGE(WM_UPDATE_STATUS, UpdateStatus)
	ON_MESSAGE(WM_INTELLIO_SHAVER_CMD, HandleIntellioShaverCmd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCustomOscScreen message handlers

HBRUSH CCustomOscScreen::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr;
	
	// Intercept the paint call to change the dialog control colors
	
	// Call the base class implementation first! Otherwise, it may
	// undo what we are trying to accomplish .
	hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
		
	// Set the Background color
	pDC->SetBkColor(SN_BKGND_COLOR);
	pDC->SetTextColor(SN_WHITE);
	hbr = m_hbrBlack;

	return hbr;
}

LRESULT CCustomOscScreen::UpdateStatus(WPARAM iParam, LPARAM lParam)
{

	switch( iParam)
	{
	case MSG_UPDATE_ALL_SETTINGS:
	    // Get the port status
	    if( m_pControl)
        {
		    if( m_dwPort == PORTA)
			    m_pControl->GetCmdState(GET_MC_PORTA_STATUS, &m_tPortStatus, sizeof(SN_PORT_STATUS));
		    else
			    m_pControl->GetCmdState(GET_MC_PORTB_STATUS, &m_tPortStatus, sizeof(SN_PORT_STATUS));
	    }
	    SetupDefaults();
        SetupTextButtons();
        HideButtons();
 	    m_BtnSet.RedrawWindow();
	    m_BtnCancel.RedrawWindow();
	    m_BtnDefault.RedrawWindow();
        break;

    default:
		break;
 
	}// end switch

	return 0L;
}

LRESULT CCustomOscScreen::HandleIntellioShaverCmd( WPARAM iParam, LPARAM lParam)
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

BOOL CCustomOscScreen::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Get the port status
	if( m_pControl)
    {
		if( m_dwPort == PORTA)
			m_pControl->GetCmdState(GET_MC_PORTA_STATUS, &m_tPortStatus, sizeof(SN_PORT_STATUS));
		else
			m_pControl->GetCmdState(GET_MC_PORTB_STATUS, &m_tPortStatus, sizeof(SN_PORT_STATUS));
	}

	SetupFonts();
	SetupDefaults();
    SetupTextButtons();
	SetupBitmaps();

	// Set the up and down arrow bitmaps
	m_ButtonUp.SetBitmap( m_ArrowUp);
	m_ButtonDown.SetBitmap( m_ArrowDown);
	
	HideButtons();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCustomOscScreen::SetupFonts()
{
	CSharedMemory mem;

	m_StaticText.SetFont(mem.m_Font30Bold, TRUE);
	m_StaticText2.SetFont(mem.m_Font16Bold, TRUE);
	m_StaticText3.SetFont(mem.m_Font16Bold, TRUE);
}

void CCustomOscScreen::SetupDefaults()
{
	CString csTemp;
	
	if( m_dwMode == OSC_MODE1)
	{
		// Set the Window Title
		::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,(WPARAM)MSG_UPDATE_TITLE,
					  (LPARAM)SN_OSCILLATE_MODE1);

		// Display seconds
		csTemp = m_SnHelp.FixedFloatToCString( m_tPortStatus.wDwell);
				
		SetDlgItemText( IDC_STATIC_TEXT, csTemp);
		SetDlgItemText( IDC_STATIC_TEXT2, m_SnHelp.GetString(SN_SECONDS));
		SetDlgItemText( IDC_STATIC_TEXT3, m_SnHelp.GetString(SN_SET_OSCILLATE_PERIOD));
	}
	else
	{
		// Set the Window Title
		::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,(WPARAM)MSG_UPDATE_TITLE,
					  (LPARAM)SN_OSCILLATE_MODE2);

		// Display # revolutions
		SetDlgItemInt( IDC_STATIC_TEXT, m_tPortStatus.usRevolutions);
		SetDlgItemText( IDC_STATIC_TEXT2, m_SnHelp.GetString(SN_REVOLUTIONS));
		SetDlgItemText( IDC_STATIC_TEXT3, m_SnHelp.GetString(SN_SET_NUMBER_REVOLUTIONS));
	}

}

void CCustomOscScreen::SetupTextButtons()
{
	CSharedMemory mem;

    // Setup SET button
 	m_BtnSet.LoadBitmaps(IDB_BITMAP_BK_BUTTON, IDB_BITMAP_BK_BUTTON_PRESSED,
        mem.m_Font20Normal, m_SnHelp.GetString(SN_SET));

	// Setup CANCEL button
	m_BtnCancel.LoadBitmaps(IDB_BITMAP_BK_BUTTON, IDB_BITMAP_BK_BUTTON_PRESSED,
        mem.m_Font20Normal, m_SnHelp.GetString(SN_CANCEL));

	// Setup Restore Default button
	m_BtnDefault.LoadBitmaps(IDB_BITMAP_BUTTON_GRAY, IDB_BITMAP_BUTTON_GRAY_PRESSED,
        mem.m_Font16Normal, m_SnHelp.GetString(SN_RESTORE_DEFAULT), SN_WHITE, 8);
}

void CCustomOscScreen::SetupBitmaps()
{
	CSharedMemory mem;

    // Load the up and down arrow bitmaps
	m_SnHelp.LoadBitmap(&m_ArrowUp, IDB_BITMAP_BUTTON_ARROW_UP);
	m_SnHelp.LoadBitmap(&m_ArrowUpPressed, IDB_BITMAP_BUTTON_ARROW_UP_PRESSED);
	m_SnHelp.LoadBitmap(&m_ArrowDown, IDB_BITMAP_BUTTON_ARROW_DOWN);
	m_SnHelp.LoadBitmap(&m_ArrowDownPressed, IDB_BITMAP_BUTTON_ARROW_DOWN_PRESSED);

}

void CCustomOscScreen::DrawLines()
{
	POINT startPoint, endPoint;
	SN_RECT rect;

	//
	// Draw rectangle
	//
	rect.x = 312;
	rect.y = 130;
	rect.width = 126;
	rect.height = 56;
	
	m_SnHelp.SetLineWidth( SN_LINE_WIDTH_4);
	m_SnHelp.DrawRectEmpty( SN_LINE_COLOR, rect);
	
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

}

void CCustomOscScreen::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	DrawLines();
	
	// Do not call CDialog::OnPaint() for painting messages
}

void CCustomOscScreen::OnButtonSet() 
{
	CString csTemp;
	SnWord wTemp;
	SnBool bStatus;

	if( m_dwMode == OSC_MODE1)
	{
		// Get the current value
		GetDlgItemText( IDC_STATIC_TEXT, csTemp);
		wTemp = m_SnHelp.CStringToFixedFloat( csTemp);
	
		// Set the seconds
		m_tPortStatus.wDwell = wTemp; 
	}
	else 
	{
		// Get the current value
		int i = GetDlgItemInt( IDC_STATIC_TEXT);
		
		// Set the # Revolutions (Profile)
		m_tPortStatus.usRevolutions = i; 
	}

   
   if (m_pControl) 
   {
		// Update the Control Layer
		if( m_dwPort == PORTA)
			m_pControl->SetCmdState(SET_MC_PORTA_STATUS, &m_tPortStatus, sizeof(SN_PORT_STATUS));
		else
			m_pControl->SetCmdState(SET_MC_PORTB_STATUS, &m_tPortStatus, sizeof(SN_PORT_STATUS));
		
		SnWord usTemp = SAVE_NVRAM;
		// Save the settings
		bStatus = m_pControl->SetCmdState(SET_SYSTEM_PARAMETERS, &usTemp, sizeof(usTemp));
		if( !bStatus)
		{
			CMessageBox dlg( m_pControl, SN_SYSTEM_ERROR, SN_CUSTOM_SETTINGS_SAVE_FAILURE);
			dlg.DoModal();
		}
   }     
    CDialog::OnOK();
}

void CCustomOscScreen::OnButtonCancel() 
{
	CDialog::OnCancel();	
}

void CCustomOscScreen::OnButtonDefault() 
{
	CString csTemp;

	if( m_dwMode == OSC_MODE1)
	{
		// Display Default seconds
		csTemp = m_SnHelp.FixedFloatToCString( m_tPortStatus.wDwellDefault);
		SetDlgItemText( IDC_STATIC_TEXT, csTemp);
	}
	else
	{
		// Display Default # revolutions
		SetDlgItemInt( IDC_STATIC_TEXT, m_tPortStatus.usRevolutionsDefault);
	}

	HideButtons();
}

void CCustomOscScreen::HideButtons()
{
	SnWord wMin,wMax;

	if( m_dwMode == OSC_MODE1)
	{
		wMax = m_tPortStatus.wDwellMax;
		wMin = m_tPortStatus.wDwellMin;
		
		CString csTemp;
		// Get current selection
		GetDlgItemText( IDC_STATIC_TEXT, csTemp);
		SnWord wTemp = m_SnHelp.CStringToFixedFloat(csTemp);
		if(wTemp == wMax)
		{
			m_ButtonUp.ShowWindow(SW_HIDE);
			m_ButtonDown.ShowWindow(SW_SHOW);
		}
		else if( wTemp == wMin)
		{
			m_ButtonDown.ShowWindow(SW_HIDE);
			m_ButtonUp.ShowWindow(SW_SHOW);
		}
		else
		{
			m_ButtonUp.ShowWindow(SW_SHOW);
			m_ButtonDown.ShowWindow(SW_SHOW);
		}
	}
	else
	{
		wMax = m_tPortStatus.usRevolutionsMax;
		wMin = m_tPortStatus.usRevolutionsMin;
		
		int i = GetDlgItemInt( IDC_STATIC_TEXT);
		if(i == wMax)
		{
			m_ButtonUp.ShowWindow(SW_HIDE);
			m_ButtonDown.ShowWindow(SW_SHOW);
		}
		else if( i == wMin)
		{
			m_ButtonUp.ShowWindow(SW_SHOW);
			m_ButtonDown.ShowWindow(SW_HIDE);
		}
		else
		{
			m_ButtonUp.ShowWindow(SW_SHOW);
			m_ButtonDown.ShowWindow(SW_SHOW);
		}
	}
}

void CCustomOscScreen::UpdateRevolutions(unsigned char mode) 
{
	int i;
	SnWord usMax,usMin,usIncrement;
	CSharedMemory mem;

	// Get the current selection
	i = GetDlgItemInt( IDC_STATIC_TEXT);

	usMax = m_tPortStatus.usRevolutionsMax;
	usMin = m_tPortStatus.usRevolutionsMin;
	usIncrement = m_tPortStatus.usRevolutionsIncrement;
	
	if( mode == DOWN)
	{
		if( (i <= usMax) && (i > usMin))
		{
			i = i - usIncrement; // decrement 
	
			SetDlgItemInt( IDC_STATIC_TEXT, i);
		}
	}
	else
	{
		if( (i < usMax) && (i >= usMin))
		{
			i = i + usIncrement; // increment 
		
			SetDlgItemInt( IDC_STATIC_TEXT, i);
		}
	}

	HideButtons();
}

void CCustomOscScreen::UpdateSeconds(unsigned char mode) 
{
	SnWord w;
	SnWord wMax,wMin,wIncrement;
	CSharedMemory mem;
	CString csTemp;

	// Get the current selection
	GetDlgItemText( IDC_STATIC_TEXT, csTemp);

	w = m_SnHelp.CStringToFixedFloat( csTemp);
	
	wMax = m_tPortStatus.wDwellMax;
	wMin = m_tPortStatus.wDwellMin;
	wIncrement = m_tPortStatus.wDwellIncrement;
	
	if( mode == DOWN)
	{
		if( (w <= wMax) && (w > wMin))
		{
			w = w - wIncrement; // decrement 
	
			csTemp = m_SnHelp.FixedFloatToCString( w);
			SetDlgItemText( IDC_STATIC_TEXT,csTemp);
		}
	}
	else
	{
		if( (w < wMax) && (w >= wMin))
		{
			w = w + wIncrement; // increment 
		
			csTemp = m_SnHelp.FixedFloatToCString( w);
			SetDlgItemText( IDC_STATIC_TEXT,csTemp);
		}
	}


	HideButtons();
}

void CCustomOscScreen::OnStaticButtonDown() 
{
	m_ButtonDown.SetBitmap( m_ArrowDownPressed);

	Sleep(50);
	if( m_dwMode == OSC_MODE1)
		UpdateSeconds(DOWN);
	else
		UpdateRevolutions(DOWN);
	
	m_ButtonDown.SetBitmap( m_ArrowDown);
}

void CCustomOscScreen::OnStaticButtonUp() 
{
	m_ButtonUp.SetBitmap( m_ArrowUpPressed);

	Sleep(50);
	if( m_dwMode == OSC_MODE1)
		UpdateSeconds(UP);
	else
		UpdateRevolutions(UP);

	m_ButtonUp.SetBitmap( m_ArrowUp);
	
}
