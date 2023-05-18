// LanguagePopUp.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "LanguagePopUp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLanguagePopUp dialog


CLanguagePopUp::CLanguagePopUp(CControl* pControl, SnWord usLanguage, CWnd* pParent /*=NULL*/)
	: CDialog(CLanguagePopUp::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLanguagePopUp)
	//}}AFX_DATA_INIT

	m_usNewLanguage = usLanguage;
	m_pControl = pControl;

	// create solid brush for background color
	m_hbrWhite = CreateSolidBrush(SN_WHITE);
	m_hbrYellow = CreateSolidBrush(SN_YELLOW);
}

void CLanguagePopUp::DeInit()
{
	// Delete solid brush object
	DeleteObject(m_hbrWhite);
	DeleteObject(m_hbrYellow);
}

CLanguagePopUp::~CLanguagePopUp()
{
	DeInit();
}

void CLanguagePopUp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLanguagePopUp)
	DDX_Control(pDX, IDC_BUTTON_YES, m_BtnYes);
	DDX_Control(pDX, IDC_BUTTON_NO, m_BtnNo);
	DDX_Control(pDX, IDC_STATIC_TITLE, m_StaticTitle);
	DDX_Control(pDX, IDC_STATIC_TEXT, m_StaticText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLanguagePopUp, CDialog)
	//{{AFX_MSG_MAP(CLanguagePopUp)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_YES, OnButtonYes)
	ON_BN_CLICKED(IDC_BUTTON_NO, OnButtonNo)
	ON_MESSAGE(WM_INTELLIO_SHAVER_CMD, HandleIntellioShaverCmd)
	ON_MESSAGE(WM_UPDATE_STATUS, UpdateStatus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLanguagePopUp message handlers

HBRUSH CLanguagePopUp::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr;
	
	// Intercept the paint call to change the dialog control colors
	
	// Call the base class implementation first! Otherwise, it may
	// undo what we are trying to accomplish .
	hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	int nCtrlID = pWnd->GetDlgCtrlID();
	switch(nCtrlID)
	{
		case IDC_STATIC_TITLE:
			pDC->SetBkColor(SN_YELLOW);
			pDC->SetTextColor(SN_BKGND_COLOR);
			hbr = m_hbrYellow;
			break;
		
		default :
			pDC->SetBkColor(SN_WHITE);
			pDC->SetTextColor(SN_BLACK);
			hbr = m_hbrWhite;
			break;

	}
	
	return hbr;

}

LRESULT CLanguagePopUp::UpdateStatus(WPARAM iParam, LPARAM lParam)
{

	switch( iParam)
	{
	case MSG_UPDATE_ALL_SETTINGS:
	    // Get current language
	    if( m_pControl) 
	    {
		    // Get the saved language selection
		    SnBool bStatus = m_pControl->GetCmdState(GET_SYSTEM_LANGUAGE, &m_usCurrentLanguage, sizeof(m_usCurrentLanguage));
		    if(!bStatus)
			    m_usCurrentLanguage = LANGUAGE_ENGLISH;
	    }
		break;
	
		
	default:
		break;
 
	}// end switch

	return 0L;
}

LRESULT CLanguagePopUp::HandleIntellioShaverCmd( WPARAM iParam, LPARAM lParam)
{
	if( m_pControl != NULL && iParam == KEY_EXIT_SETTINGS)
    {
	    // Load the current language dll, user does not want to change languages
	    if( m_usCurrentLanguage != m_usNewLanguage)
	    {
		    // Load the dll
		    CSharedMemory mem;

		    mem.LoadLanguageDll( m_usCurrentLanguage);
	    }		
        
        DeInit();

	    // Kill the dialog window and return to main
	    CDialog::EndDialog(IDCANCEL);
	}

	return 0L;
}

BOOL CLanguagePopUp::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Get current language
	if( m_pControl) 
	{
		// Get the saved language selection
		SnBool bStatus = m_pControl->GetCmdState(GET_SYSTEM_LANGUAGE, &m_usCurrentLanguage, sizeof(m_usCurrentLanguage));
		if(!bStatus)
			m_usCurrentLanguage = LANGUAGE_ENGLISH;
	}
	else
		m_usCurrentLanguage = LANGUAGE_ENGLISH;		
			

	if( m_usCurrentLanguage != m_usNewLanguage)
	{
		// Load the dll
		CSharedMemory mem;

		mem.LoadLanguageDll( m_usNewLanguage);
	}	
	
	switch(m_usNewLanguage)
	{
		case LANGUAGE_ENGLISH:
				m_SnHelp.GetString(SN_ENGLISH, m_csNewLanguage);
			break;
		case LANGUAGE_GERMAN:
				m_SnHelp.GetString(SN_GERMAN, m_csNewLanguage);
			break;
		case LANGUAGE_ITALIAN:
				m_SnHelp.GetString(SN_ITALIAN, m_csNewLanguage);
			break;
		case LANGUAGE_SPANISH:
				m_SnHelp.GetString(SN_SPANISH, m_csNewLanguage);
			break;
		case LANGUAGE_FRENCH:
				m_SnHelp.GetString(SN_FRENCH, m_csNewLanguage);
			break;			
		case LANGUAGE_DANISH:
				m_SnHelp.GetString(SN_DANISH, m_csNewLanguage);
			break;
		case LANGUAGE_DUTCH:
				m_SnHelp.GetString(SN_DUTCH, m_csNewLanguage);
			break;
		case LANGUAGE_NORWEGIAN:
				m_SnHelp.GetString(SN_NORWEGIAN, m_csNewLanguage);
			break;
		case LANGUAGE_PORTUGUESE:
				m_SnHelp.GetString(SN_PORTUGUESE, m_csNewLanguage);
			break;
		case LANGUAGE_SWEDISH:
				m_SnHelp.GetString(SN_SWEDISH, m_csNewLanguage);
			break;
	
		default:
				m_SnHelp.GetString(SN_ENGLISH, m_csNewLanguage);
			break;
	}
	
	SetupBitmaps();
	SetupFonts();
	SetupDefaults();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLanguagePopUp::SetupBitmaps()
{
	// Setup Yes button
	m_BtnYes.LoadBitmaps(IDB_BITMAP_BTN_CHECK_MARK, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED_WHITEBK);

	// Setup No button
	m_BtnNo.LoadBitmaps(IDB_BITMAP_BTN_X_MARK, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED_WHITEBK);
}

void CLanguagePopUp::SetupFonts()
{
	CSharedMemory mem;

	m_StaticTitle.SetFont(mem.m_Font30Normal, TRUE);
	m_StaticText.SetFont(mem.m_Font20Normal, TRUE);
}	


void CLanguagePopUp::SetupDefaults()
{
	CString csTemp;
	CString csLeft;
	CString csRight;
	int index;
	int lengthNewLanguage;
	int lengthText;
	int i;

	m_SnHelp.GetString(SN_LANGUAGE_TEXT, csTemp);

	lengthNewLanguage = m_csNewLanguage.GetLength();
	lengthText = csTemp.GetLength();
	index = csTemp.Find( '<' );

	// Extract the string that comes before the '<' 
	for( i = 0; i < index; i++)
	{
		csLeft = csLeft + csTemp.GetAt(i);
			
	}	
		
	// Extract the string that comes after the '<' 
	for( i = index + 1; i < lengthText; i++)
	{
		csRight = csRight + csTemp.GetAt( i);
	}
		
	// Concatenate all 3 strings
	csTemp = csLeft + m_csNewLanguage + csRight;


	SetDlgItemText( IDC_STATIC_TITLE, m_SnHelp.GetString(SN_LANGUAGE_SELECTION));
	SetDlgItemText(IDC_STATIC_TEXT, csTemp);
}

void CLanguagePopUp::OnButtonYes() 
{
	
	CDialog::OnOK();
	
}

void CLanguagePopUp::OnButtonNo() 
{
	// Load the current language dll, user does not want to change languages
	if( m_usCurrentLanguage != m_usNewLanguage)
	{
		// Load the dll
		CSharedMemory mem;

		mem.LoadLanguageDll( m_usCurrentLanguage);
	}		
	
	CDialog::OnCancel();
	
}

