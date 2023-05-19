#if !defined(AFX_MESSAGEBOX_H__9A27F2A1_9CD1_48CE_9587_6E9B383ABF17__INCLUDED_)
#define AFX_MESSAGEBOX_H__9A27F2A1_9CD1_48CE_9587_6E9B383ABF17__INCLUDED_

#include "resource.h"
#include "Control.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MessageBox.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CMessageBox dialog

/////////////////////////////////////////////////////////////////////////////
// CMessageBox dialog

class CMessageBox : public CDialog
{
// Construction
public:
	CMessageBox(CControl* pControl, CString csTitle, CString csMessageText, SnBool bBtnOk = TRUE, SnBool bBtnYesNo = FALSE, CWnd* pParent = NULL);   // standard constructor
	virtual ~CMessageBox();				 // destructor
// Dialog Data
	//{{AFX_DATA(CMessageBox)
	enum { IDD = IDD_MESSAGE_BOX };
	CStatic	m_StaticTitle;
	CStatic	m_StaticMessageText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMessageBox)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMessageBox)
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonOk();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnButtonAccept();
	afx_msg void OnButtonCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// Message Handlers
	void SetupBitmaps();			// Load bitmaps
	void SetupFonts();				// Setup Fonts for all controls

private:
	CControl*		m_pControl;		// Pointer to a Control Layer Object
	CFont			m_Font;	
	HBRUSH			m_hbrWhite;
	HBRUSH			m_hbrYellow;
	SnBool			m_bBtnOk;
	SnBool			m_bBtnYesNo;
	CString			m_csMessageText;
	CString			m_csTitle;
	CBitmapButton	m_BtnOk;
	CBitmapButton	m_BtnYes;
	CBitmapButton	m_BtnNo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MESSAGEBOX_H__9A27F2A1_9CD1_48CE_9587_6E9B383ABF17__INCLUDED_)
