// MessageBox.cpp : implementation file
//

#include "stdafx.h"
#include "ColorFonts.h"
#include "MessageBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMessageBox dialog


CMessageBox::CMessageBox(CControl* pControl, CString csTitle, CString csMessageText, SnBool bBtnOk, SnBool bBtnYesNo, CWnd* pParent /*=NULL*/)
	: CDialog(CMessageBox::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMessageBox)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pControl = pControl;
	m_csMessageText = csMessageText;
	m_csTitle = csTitle;

	m_bBtnOk = bBtnOk;
	m_bBtnYesNo = bBtnYesNo;

	// create solid brush for background color
	m_hbrWhite = CreateSolidBrush(SN_WHITE);
	m_hbrYellow = CreateSolidBrush(SN_YELLOW);
}
CMessageBox::~CMessageBox()
{
	// Delete solid brush object
	DeleteObject( m_hbrWhite);
	DeleteObject( m_hbrYellow);
}

void CMessageBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMessageBox)
	DDX_Control(pDX, IDC_STATIC_TITLE, m_StaticTitle);
	DDX_Control(pDX, IDC_STATIC_MESSAGE_TEXT, m_StaticMessageText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMessageBox, CDialog)
	//{{AFX_MSG_MAP(CMessageBox)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_OK, OnButtonOk)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_BUTTON_ACCEPT, OnButtonAccept)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessageBox message handlers

BOOL CMessageBox::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if( m_pControl)
	{
		// Tell the control layer to send WM messages to this screen.
		m_pControl->SetMessageHandler(this->m_hWnd);
	}
	
	SetupBitmaps();
	SetupFonts();
	
	if( m_bBtnOk)
	{
		// Hide the Yes and No buttons
		m_BtnYes.ShowWindow(SW_HIDE);
		m_BtnNo.ShowWindow(SW_HIDE);
	}
	else if( m_bBtnYesNo)
	{
		// Hide the OK button
		m_BtnOk.ShowWindow(SW_HIDE);
	}
	
	// Setup static text boxes
	SetDlgItemText( IDC_STATIC_TITLE, m_csTitle);
	SetDlgItemText( IDC_STATIC_MESSAGE_TEXT, m_csMessageText);	

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CMessageBox::SetupFonts()
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
	txtFont.lfHeight = 100;
	txtFont.lfWeight = FW_BOLD;
	bStatus = m_Font.CreatePointFontIndirect(&txtFont, NULL);

	
	m_StaticTitle.SetFont(&m_Font, TRUE);
	m_StaticMessageText.SetFont(&m_Font, TRUE);
}

void CMessageBox::SetupBitmaps()
{

	//
}

HBRUSH CMessageBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
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

void CMessageBox::OnButtonOk() 
{
	CDialog::OnOK();	
}

void CMessageBox::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CString csTemp;
	

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
		

		switch( nIDCtl)
		{
			case IDC_BUTTON_OK:	
				if(m_bBtnOk)
					m_BtnOk.ShowWindow(SW_SHOW);
			break;

			case IDC_BUTTON_ACCEPT:	
				if(m_bBtnYesNo)
				m_BtnYes.ShowWindow(SW_SHOW);
			break;
			
			case IDC_BUTTON_CANCEL:	
				if(m_bBtnYesNo)
					m_BtnNo.ShowWindow(SW_SHOW);
	
			break;
						
			default:
					
				break;

		}

	}
}

void CMessageBox::OnButtonAccept() 
{
		CDialog::OnOK();
	
}

void CMessageBox::OnButtonCancel() 
{
		CDialog::OnCancel();
	
}
