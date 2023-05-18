#if !defined(AFX_KEYPADDLG_H__CBE80252_0592_4E16_ACC9_485E79B0FDA9__INCLUDED_)
#define AFX_KEYPADDLG_H__CBE80252_0592_4E16_ACC9_485E79B0FDA9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// KeypadDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CKeypadDlg dialog

class CKeypadDlg : public CDialog
{
// Construction
public:
	CKeypadDlg(int ControlID, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CKeypadDlg)
	enum { IDD = IDD_DIALOG_KEYPAD };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeypadDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CKeypadDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButton0();
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	afx_msg void OnButton3();
	afx_msg void OnButton4();
	afx_msg void OnButton5();
	afx_msg void OnButton6();
	afx_msg void OnButton7();
	afx_msg void OnButton8();
	afx_msg void OnButton9();
	afx_msg void OnButton1Decimal();
	afx_msg void OnClear();
	virtual void OnOK();
	afx_msg void OnButtonMinus();
	afx_msg void OnButtonA();
	afx_msg void OnButtonB();
	afx_msg void OnButtonC();
	afx_msg void OnButtonD();
	afx_msg void OnButtonE();
	afx_msg void OnButtonF();
	afx_msg void OnButtonHex();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void ConcateString( CString csString);

private:
	CString m_csInputString;
	CWnd*	m_pParent;
	int		m_ControlId;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYPADDLG_H__CBE80252_0592_4E16_ACC9_485E79B0FDA9__INCLUDED_)
