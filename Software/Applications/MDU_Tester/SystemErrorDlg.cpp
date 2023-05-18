// SystemErrorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "SystemErrorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSystemErrorDlg dialog


CSystemErrorDlg::CSystemErrorDlg(CControl* pControl, DWORD dwErrorNum, CString csMsg1, CWnd* pParent /*=NULL*/)
	: CDialog(CSystemErrorDlg::IDD, pParent)
{
	m_pParent = pParent;
	m_pControl = pControl;
	m_csMsg1 = csMsg1;
	m_dwErrorNum = dwErrorNum;

	// create solid brush for background color
	m_hbr = CreateSolidBrush(SN_BKGND_COLOR);
	
	
	//{{AFX_DATA_INIT(CSystemErrorDlg)
	//}}AFX_DATA_INIT
}
CSystemErrorDlg::~CSystemErrorDlg()
{
	
	// Delete solid brush object
	DeleteObject( m_hbr);
		
}

void CSystemErrorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSystemErrorDlg)
	DDX_Control(pDX, IDC_STATIC_TEXT3, m_StaticText3);
	DDX_Control(pDX, IDC_STATIC_TEXT2, m_StaticText2);
	DDX_Control(pDX, IDC_STATIC_TEXT1, m_StaticText1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSystemErrorDlg, CDialog)
	//{{AFX_MSG_MAP(CSystemErrorDlg)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSystemErrorDlg message handlers

HBRUSH CSystemErrorDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
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

BOOL CSystemErrorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if( m_pControl) 
	{
	    // Terminate any running Control threads
        m_pControl->ShutDownThreads();
	}
	
	SetupFonts();
	
	CString csTemp, csError;

	csTemp = m_SnHelp.GetString(SN_SYSTEM_ERROR);

	csError.Format(_T("%s %d"), csTemp, m_dwErrorNum);

	SetDlgItemText( IDC_STATIC_TEXT1, csError);
	SetDlgItemText( IDC_STATIC_TEXT2, m_csMsg1);

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CSystemErrorDlg::SetupFonts()
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
	txtFont.lfHeight = 250;
	txtFont.lfWeight = FW_BOLD;
	bStatus = m_Font.CreatePointFontIndirect(&txtFont, NULL);

	
	m_StaticText1.SetFont(&m_Font, TRUE);
	m_StaticText2.SetFont(&m_Font, TRUE);
	m_StaticText3.SetFont(&m_Font, TRUE);
}
