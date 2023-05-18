// AlphaKeypadDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestApp.h"
#include "AlphaKeypadDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAlphaKeypadDlg dialog


CAlphaKeypadDlg::CAlphaKeypadDlg(int ControlID, CWnd* pParent /*=NULL*/)
	: CDialog(CAlphaKeypadDlg::IDD, pParent)
{
	m_pParent = pParent;
	m_ControlId = ControlID;
	
	//{{AFX_DATA_INIT(CAlphaKeypadDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAlphaKeypadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAlphaKeypadDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAlphaKeypadDlg, CDialog)
	//{{AFX_MSG_MAP(CAlphaKeypadDlg)
	ON_BN_CLICKED(IDC_BUTTON_A, OnButtonA)
	ON_BN_CLICKED(IDCLEAR, OnClear)
	ON_BN_CLICKED(IDC_BUTTON_B, OnButtonB)
	ON_BN_CLICKED(IDC_BUTTON_C, OnButtonC)
	ON_BN_CLICKED(IDC_BUTTON_D, OnButtonD)
	ON_BN_CLICKED(IDC_BUTTON_E, OnButtonE)
	ON_BN_CLICKED(IDC_BUTTON_F, OnButtonF)
	ON_BN_CLICKED(IDC_BUTTON_G, OnButtonG)
	ON_BN_CLICKED(IDC_BUTTON_H, OnButtonH)
	ON_BN_CLICKED(IDC_BUTTON_I, OnButtonI)
	ON_BN_CLICKED(IDC_BUTTON_J, OnButtonJ)
	ON_BN_CLICKED(IDC_BUTTON_K, OnButtonK)
	ON_BN_CLICKED(IDC_BUTTON_L, OnButtonL)
	ON_BN_CLICKED(IDC_BUTTON_M, OnButtonM)
	ON_BN_CLICKED(IDC_BUTTON_N, OnButtonN)
	ON_BN_CLICKED(IDC_BUTTON_O, OnButtonO)
	ON_BN_CLICKED(IDC_BUTTON_P, OnButtonP)
	ON_BN_CLICKED(IDC_BUTTON_Q, OnButtonQ)
	ON_BN_CLICKED(IDC_BUTTON_R, OnButtonR)
	ON_BN_CLICKED(IDC_BUTTON_S, OnButtonS)
	ON_BN_CLICKED(IDC_BUTTON_T, OnButtonT)
	ON_BN_CLICKED(IDC_BUTTON_U, OnButtonU)
	ON_BN_CLICKED(IDC_BUTTON_V, OnButtonV)
	ON_BN_CLICKED(IDC_BUTTON_W, OnButtonW)
	ON_BN_CLICKED(IDC_BUTTON_X, OnButtonX)
	ON_BN_CLICKED(IDC_BUTTON_Y, OnButtonY)
	ON_BN_CLICKED(IDC_BUTTON_Z, OnButtonZ)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlphaKeypadDlg message handlers
/////////////////////////////////////////////////////////////////////////////
BOOL CAlphaKeypadDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_csInputString = _T("");
	return TRUE;  // return TRUE unless you set the focus to a control
	              
}

void CAlphaKeypadDlg::ConcateString( CString csString)
{
		
	// concatinate string 
	m_csInputString += csString;

	// write to the control window
	m_pParent->SetDlgItemText(m_ControlId, m_csInputString);
}

void CAlphaKeypadDlg::OnOK() 
{
	CDialog::OnOK();
	// give control back to the parent window
	SetActiveWindow();
}

void CAlphaKeypadDlg::OnClear() 
{
	m_csInputString = _T("");
	ConcateString("");
}

void CAlphaKeypadDlg::OnButtonA() 
{
	ConcateString(_T("A"));	
}

void CAlphaKeypadDlg::OnButtonB() 
{
	ConcateString(_T("B"));	
}

void CAlphaKeypadDlg::OnButtonC() 
{
	ConcateString(_T("C"));	
}

void CAlphaKeypadDlg::OnButtonD() 
{
	ConcateString(_T("D"));	
}

void CAlphaKeypadDlg::OnButtonE() 
{
	ConcateString(_T("E"));	
}

void CAlphaKeypadDlg::OnButtonF() 
{
	ConcateString(_T("F"));	
}

void CAlphaKeypadDlg::OnButtonG() 
{
	ConcateString(_T("G"));	
}

void CAlphaKeypadDlg::OnButtonH() 
{
	ConcateString(_T("H"));	
}

void CAlphaKeypadDlg::OnButtonI() 
{
	ConcateString(_T("I"));	
}

void CAlphaKeypadDlg::OnButtonJ() 
{
	ConcateString(_T("J"));	
}

void CAlphaKeypadDlg::OnButtonK() 
{
	ConcateString(_T("K"));	
}

void CAlphaKeypadDlg::OnButtonL() 
{
	ConcateString(_T("L"));	
}

void CAlphaKeypadDlg::OnButtonM() 
{
	ConcateString(_T("M"));	
}

void CAlphaKeypadDlg::OnButtonN() 
{
	ConcateString(_T("N"));	
}

void CAlphaKeypadDlg::OnButtonO() 
{
	ConcateString(_T("O"));	
}

void CAlphaKeypadDlg::OnButtonP() 
{
	ConcateString(_T("P"));	
}

void CAlphaKeypadDlg::OnButtonQ() 
{
	ConcateString(_T("Q"));	
}

void CAlphaKeypadDlg::OnButtonR() 
{
	ConcateString(_T("R"));	
}

void CAlphaKeypadDlg::OnButtonS() 
{
	ConcateString(_T("S"));	
}

void CAlphaKeypadDlg::OnButtonT() 
{
	ConcateString(_T("T"));	
}

void CAlphaKeypadDlg::OnButtonU() 
{
	ConcateString(_T("U"));	
}

void CAlphaKeypadDlg::OnButtonV() 
{
	ConcateString(_T("V"));	
}

void CAlphaKeypadDlg::OnButtonW() 
{
	ConcateString(_T("W"));	
}

void CAlphaKeypadDlg::OnButtonX() 
{
	ConcateString(_T("X"));	
}

void CAlphaKeypadDlg::OnButtonY() 
{
	ConcateString(_T("Y"));	
}

void CAlphaKeypadDlg::OnButtonZ() 
{
	ConcateString(_T("Z"));	
}
