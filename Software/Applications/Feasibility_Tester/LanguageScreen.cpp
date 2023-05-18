// LanguageScreen.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "LanguageScreen.h"
#include "MessageBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CLanguageScreen::CLanguageScreen(CControl* pControl, CWnd* pParent /*=NULL*/)
	: CDialog(CLanguageScreen::IDD, pParent)
{
	
	m_pParent = pParent;
	m_pControl = pControl;

	// create solid brush for background color
	m_hbr = CreateSolidBrush(SN_BKGND_COLOR);

	//{{AFX_DATA_INIT(CLanguageScreen)
	//}}AFX_DATA_INIT
}

CLanguageScreen::~CLanguageScreen()
{
	// Delete solid brush object
	DeInit();
}

void CLanguageScreen::DeInit()
{
	// delete brush objects
	DeleteObject( m_hbr);
}

void CLanguageScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLanguageScreen)
	DDX_Control(pDX, IDC_BUTTON_DONE, m_BtnDone);
	DDX_Control(pDX, IDC_STATIC_LANGUAGE9, m_StaticLanguage9);
	DDX_Control(pDX, IDC_STATIC_LANGUAGE8, m_StaticLanguage8);
	DDX_Control(pDX, IDC_STATIC_LANGUAGE7, m_StaticLanguage7);
	DDX_Control(pDX, IDC_STATIC_LANGUAGE6, m_StaticLanguage6);
	DDX_Control(pDX, IDC_STATIC_LANGUAGE5, m_StaticLanguage5);
	DDX_Control(pDX, IDC_STATIC_LANGUAGE4, m_StaticLanguage4);
	DDX_Control(pDX, IDC_STATIC_LANGUAGE3, m_StaticLanguage3);
	DDX_Control(pDX, IDC_STATIC_LANGUAGE2, m_StaticLanguage2);
	DDX_Control(pDX, IDC_STATIC_LANGUAGE10, m_StaticLanguage10);
	DDX_Control(pDX, IDC_STATIC_LANGUAGE1, m_StaticLanguage1);
	DDX_Control(pDX, IDC_STATIC_BUTTON_LANGUAGE9, m_StaticButtonLanguage9);
	DDX_Control(pDX, IDC_STATIC_BUTTON_LANGUAGE8, m_StaticButtonLanguage8);
	DDX_Control(pDX, IDC_STATIC_BUTTON_LANGUAGE7, m_StaticButtonLanguage7);
	DDX_Control(pDX, IDC_STATIC_BUTTON_LANGUAGE6, m_StaticButtonLanguage6);
	DDX_Control(pDX, IDC_STATIC_BUTTON_LANGUAGE5, m_StaticButtonLanguage5);
	DDX_Control(pDX, IDC_STATIC_BUTTON_LANGUAGE4, m_StaticButtonLanguage4);
	DDX_Control(pDX, IDC_STATIC_BUTTON_LANGUAGE3, m_StaticButtonLanguage3);
	DDX_Control(pDX, IDC_STATIC_BUTTON_LANGUAGE2, m_StaticButtonLanguage2);
	DDX_Control(pDX, IDC_STATIC_BUTTON_LANGUAGE10, m_StaticButtonLanguage10);
	DDX_Control(pDX, IDC_STATIC_BUTTON_LANGUAGE1, m_StaticButtonLanguage1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLanguageScreen, CDialog)
	//{{AFX_MSG_MAP(CLanguageScreen)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_DONE, OnButtonDone)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_LANGUAGE1, OnStaticButtonLanguage1)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_LANGUAGE2, OnStaticButtonLanguage2)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_LANGUAGE3, OnStaticButtonLanguage3)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_LANGUAGE4, OnStaticButtonLanguage4)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_LANGUAGE5, OnStaticButtonLanguage5)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_LANGUAGE6, OnStaticButtonLanguage6)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_LANGUAGE7, OnStaticButtonLanguage7)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_LANGUAGE8, OnStaticButtonLanguage8)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_LANGUAGE9, OnStaticButtonLanguage9)
	ON_BN_CLICKED(IDC_STATIC_BUTTON_LANGUAGE10, OnStaticButtonLanguage10)
	ON_MESSAGE(WM_INTELLIO_SHAVER_CMD, HandleIntellioShaverCmd)
	ON_MESSAGE(WM_UPDATE_STATUS, UpdateStatus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLanguageScreen message handlers

HBRUSH CLanguageScreen::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
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

void CLanguageScreen::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	DrawLines();
}

LRESULT CLanguageScreen::UpdateStatus(WPARAM iParam, LPARAM lParam)
{

	switch( iParam)
	{
	case MSG_UPDATE_ALL_SETTINGS:
		SetupDefaults();
		DrawBitmaps();
        SetupTextButtons();
		m_BtnDone.RedrawWindow();
		break;
	
		
	default:
		break;
 
	}// end switch

	return 0L;
}

LRESULT CLanguageScreen::HandleIntellioShaverCmd( WPARAM iParam, LPARAM lParam)
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

BOOL CLanguageScreen::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Setup the screen
    SetupTextButtons();
	SetupBitmaps();
	SetupFonts();
	SetupDefaults();
	DrawBitmaps();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLanguageScreen::SetupTextButtons()
{
	CSharedMemory mem;

    // Setup Done button
	m_BtnDone.LoadBitmaps(IDB_BITMAP_BK_BUTTON, IDB_BITMAP_BK_BUTTON_PRESSED,
        mem.m_Font20Normal, m_SnHelp.GetString(SN_DONE));
}

void CLanguageScreen::SetupBitmaps()
{
	// Load the language button bitmaps
	m_SnHelp.LoadBitmap(&m_ButtonOn, IDB_BITMAP_GRAY_BUTTON_SMALL_ON);
	m_SnHelp.LoadBitmap(&m_ButtonOff, IDB_BITMAP_GRAY_BUTTON_SMALL_OFF);
}

void CLanguageScreen::DrawBitmaps()
{
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

	// Make sure all the buttons are deselected
	m_StaticButtonLanguage1.SetBitmap( m_ButtonOff);
    m_StaticButtonLanguage2.SetBitmap( m_ButtonOff);
	m_StaticButtonLanguage3.SetBitmap( m_ButtonOff);
	m_StaticButtonLanguage4.SetBitmap( m_ButtonOff);
	m_StaticButtonLanguage5.SetBitmap( m_ButtonOff);
	m_StaticButtonLanguage6.SetBitmap( m_ButtonOff);
	m_StaticButtonLanguage7.SetBitmap( m_ButtonOff);
	m_StaticButtonLanguage8.SetBitmap( m_ButtonOff);
	m_StaticButtonLanguage9.SetBitmap( m_ButtonOff);
	m_StaticButtonLanguage10.SetBitmap( m_ButtonOff);


	// Set the selected language button
	switch( usLanguage)
	{

		case LANGUAGE_ENGLISH:
				m_StaticButtonLanguage1.SetBitmap( m_ButtonOn);
			break;

		case LANGUAGE_GERMAN:
			m_StaticButtonLanguage2.SetBitmap( m_ButtonOn);
			break;
		
		case LANGUAGE_ITALIAN:
			m_StaticButtonLanguage3.SetBitmap( m_ButtonOn);
			break;
	
		case LANGUAGE_SPANISH:
			m_StaticButtonLanguage4.SetBitmap( m_ButtonOn);
			break;

		case LANGUAGE_FRENCH:
			m_StaticButtonLanguage5.SetBitmap( m_ButtonOn);
			break;

		case LANGUAGE_DANISH:
			m_StaticButtonLanguage6.SetBitmap( m_ButtonOn);
			break;

		case LANGUAGE_DUTCH:
			m_StaticButtonLanguage7.SetBitmap( m_ButtonOn);
			break;

		case LANGUAGE_NORWEGIAN:
			m_StaticButtonLanguage8.SetBitmap( m_ButtonOn);
			break;

		case LANGUAGE_PORTUGUESE:
			m_StaticButtonLanguage9.SetBitmap( m_ButtonOn);
			break;

		case LANGUAGE_SWEDISH:
			m_StaticButtonLanguage10.SetBitmap( m_ButtonOn);
			break;

		default:
			break;
	}
}

void CLanguageScreen::DrawLines() 
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
}

void CLanguageScreen::SetupFonts()
{
	CSharedMemory mem;
	
	m_StaticLanguage1.SetFont(mem.m_Font25Normal, TRUE);
	m_StaticLanguage2.SetFont(mem.m_Font25Normal, TRUE);
	m_StaticLanguage3.SetFont(mem.m_Font25Normal, TRUE);
	m_StaticLanguage4.SetFont(mem.m_Font25Normal, TRUE);
	m_StaticLanguage5.SetFont(mem.m_Font25Normal, TRUE);
	m_StaticLanguage6.SetFont(mem.m_Font25Normal, TRUE);
	m_StaticLanguage7.SetFont(mem.m_Font25Normal, TRUE);
	m_StaticLanguage8.SetFont(mem.m_Font25Normal, TRUE);
	m_StaticLanguage9.SetFont(mem.m_Font25Normal, TRUE);
	m_StaticLanguage10.SetFont(mem.m_Font25Normal, TRUE);
}	


void CLanguageScreen::SetupDefaults()
{
	// Set the text in the title bar
	::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,(WPARAM)MSG_UPDATE_TITLE, (LPARAM)SN_LANGUAGE);

	// Setup static text boxes
	SetDlgItemText( IDC_STATIC_LANGUAGE1, m_SnHelp.GetString(SN_ENGLISH));
	SetDlgItemText( IDC_STATIC_LANGUAGE2, m_SnHelp.GetString(SN_GERMAN));
	SetDlgItemText( IDC_STATIC_LANGUAGE3, m_SnHelp.GetString(SN_ITALIAN));
	SetDlgItemText( IDC_STATIC_LANGUAGE4, m_SnHelp.GetString(SN_SPANISH));
	SetDlgItemText( IDC_STATIC_LANGUAGE5, m_SnHelp.GetString(SN_FRENCH));
	SetDlgItemText( IDC_STATIC_LANGUAGE6, m_SnHelp.GetString(SN_DANISH));
	SetDlgItemText( IDC_STATIC_LANGUAGE7, m_SnHelp.GetString(SN_DUTCH));
	SetDlgItemText( IDC_STATIC_LANGUAGE8, m_SnHelp.GetString(SN_NORWEGIAN));
	SetDlgItemText( IDC_STATIC_LANGUAGE9, m_SnHelp.GetString(SN_PORTUGUESE));
	SetDlgItemText( IDC_STATIC_LANGUAGE10, m_SnHelp.GetString(SN_SWEDISH));
}

void CLanguageScreen::OnButtonDone() 
{
	CDialog::OnOK();
	
}

void CLanguageScreen::ChangeLanguage()
{
	// Ask for user intervention. Are you Sure?
	CLanguagePopUp dlg(m_pControl, m_usLanguage);

	int nResponse = dlg.DoModal();

	if (nResponse == IDOK)
	{
		// Save the setting
		if( m_pControl) 
		{	
			// Set the language selection
			SnBool bStatus = m_pControl->SetCmdState(SET_SYSTEM_LANGUAGE, &m_usLanguage, sizeof(m_usLanguage));
			if(!bStatus)
				m_usLanguage = LANGUAGE_ENGLISH;

			SnWord usTemp = SAVE_NVRAM;
			// Save the setting
			bStatus = m_pControl->SetCmdState(SET_SYSTEM_PARAMETERS, &usTemp, sizeof(usTemp));
			if( !bStatus)
			{
				CMessageBox dlg( m_pControl, SN_SYSTEM_ERROR, SN_CUSTOM_SETTINGS_SAVE_FAILURE);
				dlg.DoModal();
			}
		}

		// Redraw the buttons
		SetupDefaults();
		// Redraw the buttons
		DrawBitmaps();
		// Redraw the buttons
        SetupTextButtons();
		m_BtnDone.RedrawWindow();

        // Let everyone know the language was updated
        ::PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,(WPARAM)MSG_UPDATE_LANGUAGE, (LPARAM)0);
	}
	else if (nResponse == IDCANCEL)
	{
		// Don't do anything	
	}
}

void CLanguageScreen::OnStaticButtonLanguage1() 
{
	CSharedMemory mem;
	m_usLanguage = LANGUAGE_ENGLISH;
	ChangeLanguage();
}

void CLanguageScreen::OnStaticButtonLanguage2() 
{
	CSharedMemory mem;
	m_usLanguage = LANGUAGE_GERMAN;
	ChangeLanguage();
}

void CLanguageScreen::OnStaticButtonLanguage3() 
{
	CSharedMemory mem;
	m_usLanguage = LANGUAGE_ITALIAN;
	ChangeLanguage();
}

void CLanguageScreen::OnStaticButtonLanguage4() 
{
	CSharedMemory mem;
	m_usLanguage = LANGUAGE_SPANISH;
	ChangeLanguage();
}

void CLanguageScreen::OnStaticButtonLanguage5() 
{
	CSharedMemory mem;
	m_usLanguage = LANGUAGE_FRENCH;
	ChangeLanguage();
}

void CLanguageScreen::OnStaticButtonLanguage6() 
{
	CSharedMemory mem;
	m_usLanguage = LANGUAGE_DANISH;
	ChangeLanguage();
}

void CLanguageScreen::OnStaticButtonLanguage7() 
{
	CSharedMemory mem;
	m_usLanguage = LANGUAGE_DUTCH;
	ChangeLanguage();
}

void CLanguageScreen::OnStaticButtonLanguage8() 
{
	CSharedMemory mem;
	m_usLanguage = LANGUAGE_NORWEGIAN;
	ChangeLanguage();
}

void CLanguageScreen::OnStaticButtonLanguage9() 
{
	CSharedMemory mem;
	m_usLanguage = LANGUAGE_PORTUGUESE;
	ChangeLanguage();
}

void CLanguageScreen::OnStaticButtonLanguage10() 
{
	CSharedMemory mem;
	m_usLanguage = LANGUAGE_SWEDISH;
	ChangeLanguage();
}

