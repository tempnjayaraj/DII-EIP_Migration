#if !defined(AFX_LANGUAGEPOPUP_H__B72945D8_4DE2_4FF0_951C_F9C94C470B3A__INCLUDED_)
#define AFX_LANGUAGEPOPUP_H__B72945D8_4DE2_4FF0_951C_F9C94C470B3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LanguagePopUp.h : header file
//
#include "ColorsFonts.h"
#include "SnHelp.h"
#include "SharedMemory.h"
/////////////////////////////////////////////////////////////////////////////
// CLanguagePopUp dialog

class CLanguagePopUp : public CDialog
{
// Construction
public:
	CLanguagePopUp(CControl* pControl, SnWord usLanguage, CWnd* pParent = NULL);   // standard constructor
	virtual ~CLanguagePopUp();				 // destructor
// Dialog Data
	//{{AFX_DATA(CLanguagePopUp)
	enum { IDD = IDD_LANGUAGE_POPUP };
	CBitButton m_BtnYes;
	CBitButton m_BtnNo;
	CStatic	m_StaticTitle;
	CStatic	m_StaticText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLanguagePopUp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLanguagePopUp)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonYes();
	afx_msg void OnButtonNo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void DeInit();
	void SetupBitmaps();
	void SetupFonts();				// Setup Fonts for all controls
	void SetupDefaults();			// Setup default Text

    // Message Handlers
	LRESULT UpdateStatus(WPARAM iParam, LPARAM lParam);
    LRESULT HandleIntellioShaverCmd(WPARAM iParam, LPARAM lParam);

private:
	CControl*		m_pControl;		// Pointer to a Control Layer Object
	HBRUSH			m_hbrWhite;
	HBRUSH			m_hbrYellow;
	CString			m_csNewLanguage;
	SnWord			m_usCurrentLanguage;
	SnWord			m_usNewLanguage;
	CSnHelp			m_SnHelp;		// Smith & Nephew helper class
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LANGUAGEPOPUP_H__B72945D8_4DE2_4FF0_951C_F9C94C470B3A__INCLUDED_)
