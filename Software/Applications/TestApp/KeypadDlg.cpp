// KeypadDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestApp.h"
#include "KeypadDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeypadDlg dialog


CKeypadDlg::CKeypadDlg(int ControlID, CWnd* pParent /*=NULL*/)
	: CDialog(CKeypadDlg::IDD, pParent)
{
	m_pParent = pParent;
	m_ControlId = ControlID;
	//{{AFX_DATA_INIT(CKeypadDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CKeypadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKeypadDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CKeypadDlg, CDialog)
	//{{AFX_MSG_MAP(CKeypadDlg)
	ON_BN_CLICKED(IDC_BUTTON_0, OnButton0)
	ON_BN_CLICKED(IDC_BUTTON_1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON_2, OnButton2)
	ON_BN_CLICKED(IDC_BUTTON_3, OnButton3)
	ON_BN_CLICKED(IDC_BUTTON_4, OnButton4)
	ON_BN_CLICKED(IDC_BUTTON_5, OnButton5)
	ON_BN_CLICKED(IDC_BUTTON_6, OnButton6)
	ON_BN_CLICKED(IDC_BUTTON_7, OnButton7)
	ON_BN_CLICKED(IDC_BUTTON_8, OnButton8)
	ON_BN_CLICKED(IDC_BUTTON_9, OnButton9)
	ON_BN_CLICKED(IDC_BUTTON1_DECIMAL, OnButton1Decimal)
	ON_BN_CLICKED(ID_CLEAR, OnClear)
	ON_BN_CLICKED(IDC_BUTTON_MINUS, OnButtonMinus)
	ON_BN_CLICKED(IDC_BUTTON_A, OnButtonA)
	ON_BN_CLICKED(IDC_BUTTON_B, OnButtonB)
	ON_BN_CLICKED(IDC_BUTTON_C, OnButtonC)
	ON_BN_CLICKED(IDC_BUTTON_D, OnButtonD)
	ON_BN_CLICKED(IDC_BUTTON_E, OnButtonE)
	ON_BN_CLICKED(IDC_BUTTON_F, OnButtonF)
	ON_BN_CLICKED(IDC_BUTTON_HEX, OnButtonHex)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeypadDlg message handlers
BOOL CKeypadDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_csInputString = _T("");
	return TRUE;  // return TRUE unless you set the focus to a control
	              
}

void CKeypadDlg::ConcateString( CString csString)
{
		
	// concatinate string 
	m_csInputString += csString;

	// write to the control window
	m_pParent->SetDlgItemText(m_ControlId, m_csInputString);
}

void CKeypadDlg::OnButton0() 
{
	ConcateString(_T("0"));
	
}

void CKeypadDlg::OnButton1() 
{
	ConcateString(_T("1"));
}

void CKeypadDlg::OnButton2() 
{
	ConcateString(_T("2"));
	
}

void CKeypadDlg::OnButton3() 
{
	ConcateString(_T("3"));
	
}

void CKeypadDlg::OnButton4() 
{
	ConcateString(_T("4"));
	
}

void CKeypadDlg::OnButton5() 
{
	ConcateString(_T("5"));
	
}

void CKeypadDlg::OnButton6() 
{
	ConcateString(_T("6"));
	
}

void CKeypadDlg::OnButton7() 
{
	ConcateString(_T("7"));
	
}

void CKeypadDlg::OnButton8() 
{
	ConcateString(_T("8"));
	
}

void CKeypadDlg::OnButton9() 
{
	ConcateString(_T("9"));
	
}

void CKeypadDlg::OnButton1Decimal() 
{
	ConcateString(_T("."));
	
}

void CKeypadDlg::OnClear() 
{
	m_csInputString = _T("");
	ConcateString("");
}

void CKeypadDlg::OnOK() 
{
	CDialog::OnOK();
	// give control back to the parent window
	SetActiveWindow();
}

void CKeypadDlg::OnButtonMinus() 
{
	ConcateString(_T("-"));
}

void CKeypadDlg::OnButtonA() 
{
	ConcateString(_T("A"));
}

void CKeypadDlg::OnButtonB() 
{
	ConcateString(_T("B"));
}

void CKeypadDlg::OnButtonC() 
{
	ConcateString(_T("C"));
}

void CKeypadDlg::OnButtonD() 
{
	ConcateString(_T("D"));
}

void CKeypadDlg::OnButtonE() 
{
	ConcateString(_T("E"));
	
}

void CKeypadDlg::OnButtonF() 
{
	ConcateString(_T("F"));
}

void CKeypadDlg::OnButtonHex() 
{
	ConcateString(_T("0x"));
	
}
