// MessageBox.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "MessageBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMessageBox dialog

CMessageBox::CMessageBox(CControl* pControl, SnQByte qTitleId, SnQByte qMessageTextId, SnBool bBtnOk, SnBool bBtnYesNo, CWnd* pParent /*=NULL*/)
	: CDialog(CMessageBox::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMessageBox)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pControl = pControl;
	m_qTitleId = qTitleId;
	m_qMessageTextId = qMessageTextId;

	m_bBtnOk    = bBtnOk;
	m_bBtnYesNo = bBtnYesNo;

	// create solid brush for background color
	m_hbrWhite  = CreateSolidBrush(SN_WHITE);
	m_hbrYellow = CreateSolidBrush(SN_YELLOW);
}

CMessageBox::~CMessageBox()
{
	// Clear the Pop Screen for the Intellio Shaver
    UpdateIntellioShaverPopup(0);
	
	// Delete solid brush object
	DeleteObject( m_hbrWhite);
	DeleteObject( m_hbrYellow);
}

void CMessageBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMessageBox)
	DDX_Control(pDX, IDC_BUTTON_OK, m_BtnOk);
	DDX_Control(pDX, IDC_BUTTON_CANCEL, m_BtnNo);
	DDX_Control(pDX, IDC_BUTTON_ACCEPT, m_BtnYes);
	DDX_Control(pDX, IDC_STATIC_TITLE, m_StaticTitle);
	DDX_Control(pDX, IDC_STATIC_MESSAGE_TEXT, m_StaticMessageText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMessageBox, CDialog)
	//{{AFX_MSG_MAP(CMessageBox)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_OK, OnButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_ACCEPT, OnButtonAccept)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	ON_MESSAGE(WM_INTELLIO_SHAVER_CMD, HandleIntellioShaverCmd)
	ON_MESSAGE(WM_UPDATE_STATUS, UpdateStatus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessageBox message handlers

BOOL CMessageBox::OnInitDialog() 
{
	CDialog::OnInitDialog();

    if (m_pControl)
    {
        SnByte bPopup = 0;

        switch(m_qMessageTextId)
	    {
	    case SN_CUSTOM_SETTINGS_SAVE_FAILURE:
		    bPopup = SCD_PU1;
		    break;
	    case SN_CUSTOM_SETTINGS_RETRIEVE_FAILURE:
		    bPopup = SCD_PU3;
		    break;
	    case SN_SET_SPEEDS_SAVE_FAILURE:
		    bPopup = SCD_PU2;
		    break;
	    case SN_SET_SPEEDS_RETRIEVE_FAILURE:
		    bPopup = SCD_PU4;
		    break;
	    }	

        // Update Intellio Shaver
        UpdateIntellioShaverPopup(bPopup);
        SendIntellioShaverUpdateIfChange();

		// Tell the control layer to send WM messages to this screen.
		m_pControl->SetMessageHandler(this->m_hWnd);
	}
	
	SetupTextButtons();
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
	SetDlgItemText( IDC_STATIC_TITLE, m_SnHelp.GetString(m_qTitleId));
	SetDlgItemText( IDC_STATIC_MESSAGE_TEXT, m_SnHelp.GetString(m_qMessageTextId));	

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMessageBox::SetupFonts()
{
	CSharedMemory mem;

	// Set fonts
	m_StaticTitle.SetFont(mem.m_Font30Normal, TRUE);
	m_StaticMessageText.SetFont(mem.m_Font20Normal, TRUE);
}

void CMessageBox::SetupTextButtons()
{
	CSharedMemory mem;

    if (m_bBtnOk)
    {
	    // Setup OK button
	    m_BtnOk.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND_WHITEBK, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED_WHITEBK,
            mem.m_Font20Normal, m_SnHelp.GetString(SN_OK));
    }
	else if (m_bBtnYesNo)
    {
        // Setup the Yes button 
	    m_BtnYes.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND_WHITEBK, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED_WHITEBK,
            mem.m_Font20Normal, m_SnHelp.GetString(SN_YES));

	    // Setup No button
	    m_BtnNo.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND_WHITEBK, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED_WHITEBK,
            mem.m_Font20Normal, m_SnHelp.GetString(SN_NO));
    }
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

LRESULT CMessageBox::HandleIntellioShaverCmd( WPARAM iParam, LPARAM lParam)
{

	if( m_pControl != NULL)
	{
	
		switch(iParam)
		{
			case KEY_OK:
				OnButtonOk();
				break;
						
			case KEY_EXIT_SETTINGS:
				OnButtonCancel();
				break;

            default:
				break;
		}
	}

	return 0L;
}

LRESULT CMessageBox::UpdateStatus(WPARAM iParam, LPARAM lParam)
{

	switch( iParam)
	{
	case MSG_UPDATE_ALL_SETTINGS:
        SetupTextButtons();
        
        if (m_bBtnOk) {
            m_BtnOk.RedrawWindow();
        } else {
	        m_BtnYes.RedrawWindow();
	        m_BtnNo.RedrawWindow();
        }

	    SetDlgItemText( IDC_STATIC_TITLE, m_SnHelp.GetString(m_qTitleId));
	    SetDlgItemText( IDC_STATIC_MESSAGE_TEXT, m_SnHelp.GetString(m_qMessageTextId));	
	    m_StaticTitle.RedrawWindow();
	    m_StaticMessageText.RedrawWindow();
		break;
	
		
	default:
		break;
 
	}// end switch

	return 0L;
}

void CMessageBox::OnButtonOk() 
{
	CDialog::OnOK();	
}

void CMessageBox::OnButtonAccept() 
{
	CDialog::OnOK();
}

void CMessageBox::OnButtonCancel() 
{
	CDialog::OnCancel();
}
