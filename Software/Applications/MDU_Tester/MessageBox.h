#if !defined(AFX_MESSAGEBOX_H__23ADF9F8_DD90_431A_8841_16E0A9B90A34__INCLUDED_)
#define AFX_MESSAGEBOX_H__23ADF9F8_DD90_431A_8841_16E0A9B90A34__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MessageBox.h : header file
//
#include "ColorsFonts.h"
#include "SnHelp.h"
#include "SharedMemory.h"
/////////////////////////////////////////////////////////////////////////////
// CMessageBox dialog

/////////////////////////////////////////////////////////////////////////////
// CMessageBox dialog

class CMessageBox : public CDialog
{
// Construction
public:
	CMessageBox(CControl* pControl, SnQByte qTitleId, SnQByte qMessageTextId, SnBool bBtnOk = TRUE, SnBool bBtnYesNo = FALSE, CWnd* pParent = NULL);   // standard constructor
	virtual ~CMessageBox();				 // destructor
// Dialog Data
	//{{AFX_DATA(CMessageBox)
	enum { IDD = IDD_MESSAGE_BOX };
	CBitButton m_BtnOk;
	CBitButton m_BtnNo;
	CBitButton m_BtnYes;
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
	afx_msg void OnButtonAccept();
	afx_msg void OnButtonCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// Message Handlers
	LRESULT HandleIntellioShaverCmd( WPARAM iParam, LPARAM lParam);
    LRESULT UpdateStatus(WPARAM iParam, LPARAM lParam);

	void SetupTextButtons();		// Load text buttons
	void SetupFonts();				// Setup Fonts for all controls

    inline void UpdateIntellioShaverPopup(SnByte bPopup) {if(m_pControl) m_pControl->UpdateIntellioShaverPopup(bPopup);}
    inline void SendIntellioShaverUpdateIfChange() {if(m_pControl) m_pControl->SendIntellioShaverUpdateIfChange();}

private:
	CControl*		m_pControl;		// Pointer to a Control Layer Object
	HBRUSH			m_hbrWhite;
	HBRUSH			m_hbrYellow;
	SnBool			m_bBtnOk;
	SnBool			m_bBtnYesNo;
	SnQByte			m_qTitleId;
	SnQByte			m_qMessageTextId;
	CSnHelp			m_SnHelp;		// Smith & Nephew helper class
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MESSAGEBOX_H__23ADF9F8_DD90_431A_8841_16E0A9B90A34__INCLUDED_)
