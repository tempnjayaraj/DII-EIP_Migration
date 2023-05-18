// RemoteSettings.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "RemoteSettings.h"
#include "SharedMemory.h"
#include "MessageBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRemoteSettings dialog


CRemoteSettings::CRemoteSettings(CControl* pControl, CWnd* pParent /*=NULL*/)
	: CDialog(CRemoteSettings::IDD, pParent)
{

	m_pParent = pParent;
	m_pControl = pControl;

	m_bDrawLines = TRUE;
	// create brush for background color
	m_hbr = CreateSolidBrush(SN_BKGND_COLOR);
	
	//{{AFX_DATA_INIT(CRemoteSettings)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CRemoteSettings::~CRemoteSettings()
{
    DeInit();
}

void CRemoteSettings::DeInit()
{
	// delete brush objects
	DeleteObject( m_hbr);
}

void CRemoteSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRemoteSettings)
	DDX_Control(pDX, IDC_BUTTON_SET, m_BtnSet);
	DDX_Control(pDX, IDC_BUTTON_CANCEL, m_BtnCancel);
	DDX_Control(pDX, IDC_STATIC_TEXT1, m_StaticText1);
	DDX_Control(pDX, IDC_STATIC_BUTTON_PORTB, m_StaticButtonPortB);
	DDX_Control(pDX, IDC_STATIC_BUTTON_PORTA, m_StaticButtonPortA);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRemoteSettings, CDialog)
	//{{AFX_MSG_MAP(CRemoteSettings)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_PORTA, OnStaticButtonPortA)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_PORTB, OnStaticButtonPortB)
	ON_MESSAGE(WM_UPDATE_STATUS, UpdateStatus)
	ON_MESSAGE(WM_INTELLIO_SHAVER_CMD, HandleIntellioShaverCmd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRemoteSettings message handlers

HBRUSH CRemoteSettings::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
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

void CRemoteSettings::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	DrawLines();
	DrawButtons();

	// Do not call CDialog::OnPaint() for painting messages
}

BOOL CRemoteSettings::OnInitDialog() 
{
	CDialog::OnInitDialog();


	if( m_pControl)
	{
		m_pControl->GetCmdState(GET_SHAVER_PACKET_CTL, &m_usShaverPacketCtl, sizeof(m_usShaverPacketCtl));	
	
		// Get the current language selection
	    SnBool bStatus = m_pControl->GetCmdState(GET_SYSTEM_LANGUAGE, &m_usLanguage, sizeof(m_usLanguage));
		if(!bStatus)
			m_usLanguage = LANGUAGE_ENGLISH;
	
	}
	else
	{
		m_usShaverPacketCtl = PORTA;
		m_usLanguage = LANGUAGE_ENGLISH;
	}

	SetupTextButtons();
	SetupBitmaps();
	SetupFonts();
	SetupDefaults();
	DrawButtons();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CRemoteSettings::DrawLines()
{
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


	startPoint.x = 0;
	startPoint.y = 179;
	endPoint.x = 800;
	endPoint.y = 179;
	m_SnHelp.DrawLine( SN_LINE_COLOR, startPoint, endPoint);

}

void CRemoteSettings::SetupTextButtons()
{
	CSharedMemory mem;

    // Setup SET button
	m_BtnSet.LoadBitmaps(IDB_BITMAP_BK_BUTTON, IDB_BITMAP_BK_BUTTON_PRESSED,
        mem.m_Font20Normal, m_SnHelp.GetString(SN_SET));

	// Setup CANCEL button, Associate button with bitmap 
	m_BtnCancel.LoadBitmaps(IDB_BITMAP_BK_BUTTON, IDB_BITMAP_BK_BUTTON_PRESSED,
        mem.m_Font20Normal, m_SnHelp.GetString(SN_CANCEL));
}

void CRemoteSettings::SetupBitmaps()
{
	m_SnHelp.LoadBitmap(&m_ButtonOn, IDB_BITMAP_GRAY_BUTTON_ON);
	m_SnHelp.LoadBitmap(&m_ButtonOff, IDB_BITMAP_GRAY_BUTTON_OFF);
}

void CRemoteSettings::OnButtonCancel() 
{
	// Return to previous screen
	CDialog::OnCancel();
}

void CRemoteSettings::OnButtonSet() 
{
	if( m_pControl)
    {
		BOOL bStatus;
		SnWord usTemp = m_usShaverPacketCtl;
		// Set status
		m_pControl->SetCmdState(SET_SHAVER_PACKET_CTL, &usTemp, sizeof(usTemp));

		usTemp = SAVE_NVRAM;
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

void CRemoteSettings::DrawButtons()
{
	CSharedMemory mem;
	DWORD yPos;

	if( m_usLanguage == LANGUAGE_GERMAN)
		yPos = 12;
	else
		yPos = 18;
	
	if( m_usShaverPacketCtl == PORTA) 
	{
		
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

	}
	else if( m_usShaverPacketCtl == PORTB) 
	{ 
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
	}
 
}

void CRemoteSettings::SetupFonts()
{
	CSharedMemory mem;

	m_StaticText1.SetFont(mem.m_Font20Normal, TRUE);
}

void CRemoteSettings::SetupDefaults()
{
	::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,(WPARAM)MSG_UPDATE_TITLE, (LPARAM)SN_INTERFACE);
	SetDlgItemText( IDC_STATIC_TEXT1, m_SnHelp.GetString(SN_PORT_CONTROL));
	
}
void CRemoteSettings::OnStaticButtonPortA() 
{
	m_usShaverPacketCtl = PORTA;
	DrawButtons();
}

void CRemoteSettings::OnStaticButtonPortB() 
{
	m_usShaverPacketCtl = PORTB;
	DrawButtons();
}

LRESULT CRemoteSettings::UpdateStatus(WPARAM iParam, LPARAM lParam)
{
	switch( iParam)
	{		
	case MSG_UPDATE_REMOTE_PUMP_STATUS:
		if( m_pControl)
			m_pControl->GetCmdState(GET_SHAVER_PACKET_CTL, &m_usShaverPacketCtl, sizeof(SnWord));	

	    DrawButtons();
		break;

    case MSG_UPDATE_ALL_SETTINGS:
	    if( m_pControl)
	    {
		    m_pControl->GetCmdState(GET_SHAVER_PACKET_CTL, &m_usShaverPacketCtl, sizeof(m_usShaverPacketCtl));	
	    
		    // Get the current language selection
	        SnBool bStatus = m_pControl->GetCmdState(GET_SYSTEM_LANGUAGE, &m_usLanguage, sizeof(m_usLanguage));
		    if(!bStatus)
			    m_usLanguage = LANGUAGE_ENGLISH;
	    
	    }
        SetupTextButtons();
	    m_BtnSet.RedrawWindow();
	    m_BtnCancel.RedrawWindow();
	    SetupDefaults();
	    DrawButtons();
        break;

    default:
		break;
 
	}// end switch

	return 0L;
}

LRESULT CRemoteSettings::HandleIntellioShaverCmd( WPARAM iParam, LPARAM lParam)
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

