#if !defined(AFX_ALPHAKEYPADDLG_H__D245C634_977E_4691_A7F9_BA4AFD18A857__INCLUDED_)
#define AFX_ALPHAKEYPADDLG_H__D245C634_977E_4691_A7F9_BA4AFD18A857__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AlphaKeypadDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAlphaKeypadDlg dialog

class CAlphaKeypadDlg : public CDialog
{
// Construction
public:
	CAlphaKeypadDlg(int ControlID, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAlphaKeypadDlg)
	enum { IDD = IDD_DIALOG_KEYPAD_ALPHA };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlphaKeypadDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAlphaKeypadDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonA();
	virtual void OnOK();
	afx_msg void OnClear();
	afx_msg void OnButtonB();
	afx_msg void OnButtonC();
	afx_msg void OnButtonD();
	afx_msg void OnButtonE();
	afx_msg void OnButtonF();
	afx_msg void OnButtonG();
	afx_msg void OnButtonH();
	afx_msg void OnButtonI();
	afx_msg void OnButtonJ();
	afx_msg void OnButtonK();
	afx_msg void OnButtonL();
	afx_msg void OnButtonM();
	afx_msg void OnButtonN();
	afx_msg void OnButtonO();
	afx_msg void OnButtonP();
	afx_msg void OnButtonQ();
	afx_msg void OnButtonR();
	afx_msg void OnButtonS();
	afx_msg void OnButtonT();
	afx_msg void OnButtonU();
	afx_msg void OnButtonV();
	afx_msg void OnButtonW();
	afx_msg void OnButtonX();
	afx_msg void OnButtonY();
	afx_msg void OnButtonZ();
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

#endif // !defined(AFX_ALPHAKEYPADDLG_H__D245C634_977E_4691_A7F9_BA4AFD18A857__INCLUDED_)
