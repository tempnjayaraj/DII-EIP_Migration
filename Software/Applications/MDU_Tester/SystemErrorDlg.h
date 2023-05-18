#if !defined(AFX_SYSTEMERRORDLG_H__4AB442C8_F2EF_43DF_9209_597EAD3C9D24__INCLUDED_)
#define AFX_SYSTEMERRORDLG_H__4AB442C8_F2EF_43DF_9209_597EAD3C9D24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SystemErrorDlg.h : header file
//
#include "ColorsFonts.h"
#include "SnHelp.h"
#include "SharedMemory.h"

class CSystemErrorDlg : public CDialog
{
// Construction
public:
	CSystemErrorDlg(CControl* pControl, DWORD dwErrorNum, CString csMsg1, CWnd* pParent = NULL);	// standard constructor
	~CSystemErrorDlg();					// destructor

// Dialog Data
	//{{AFX_DATA(CSystemErrorDlg)
	enum { IDD = IDD_SYSTEM_ERROR };
	CStatic	m_StaticText3;
	CStatic	m_StaticText2;
	CStatic	m_StaticText1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSystemErrorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSystemErrorDlg)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void SetupFonts();

private: // local member variables
	CControl*		m_pControl;		// Pointer to a Control Layer Object
	HBRUSH			m_hbr;
	CWnd*			m_pParent;	// Handle to Dialog window
	CString			m_csMsg1;
	DWORD			m_dwErrorNum;
	CFont			m_Font;	
	CSnHelp			m_SnHelp;	// Smith & Nephew helper class
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYSTEMERRORDLG_H__4AB442C8_F2EF_43DF_9209_597EAD3C9D24__INCLUDED_)
