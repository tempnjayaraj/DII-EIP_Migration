#if !defined(AFX_LANGUAGESCREEN_H__864B986B_56F0_4B8D_8719_3E6935624CB7__INCLUDED_)
#define AFX_LANGUAGESCREEN_H__864B986B_56F0_4B8D_8719_3E6935624CB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LanguageScreen.h : header file
//

#include "ColorsFonts.h"
#include "SnHelp.h"
#include "SharedMemory.h"
#include "Control.h"
#include "LanguagePopUp.h"

/////////////////////////////////////////////////////////////////////////////
// CLanguageScreen dialog

class CLanguageScreen : public CDialog
{
// Construction
public:
	CLanguageScreen(CControl* pControl, CWnd* pParent = NULL);   // standard constructor
	virtual ~CLanguageScreen();				// destructor

// Dialog Data
	//{{AFX_DATA(CLanguageScreen)
	enum { IDD = IDD_LANGUAGE };
	CBitButton m_BtnDone;
	CStatic	m_StaticLanguage9;
	CStatic	m_StaticLanguage8;
	CStatic	m_StaticLanguage7;
	CStatic	m_StaticLanguage6;
	CStatic	m_StaticLanguage5;
	CStatic	m_StaticLanguage4;
	CStatic	m_StaticLanguage3;
	CStatic	m_StaticLanguage2;
	CStatic	m_StaticLanguage10;
	CStatic	m_StaticLanguage1;
	CStatic	m_StaticButtonLanguage9;
	CStatic	m_StaticButtonLanguage8;
	CStatic	m_StaticButtonLanguage7;
	CStatic	m_StaticButtonLanguage6;
	CStatic	m_StaticButtonLanguage5;
	CStatic	m_StaticButtonLanguage4;
	CStatic	m_StaticButtonLanguage3;
	CStatic	m_StaticButtonLanguage2;
	CStatic	m_StaticButtonLanguage10;
	CStatic	m_StaticButtonLanguage1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLanguageScreen)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLanguageScreen)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonDone();
	afx_msg void OnStaticButtonLanguage1();
	afx_msg void OnStaticButtonLanguage2();
	afx_msg void OnStaticButtonLanguage3();
	afx_msg void OnStaticButtonLanguage4();
	afx_msg void OnStaticButtonLanguage5();
	afx_msg void OnStaticButtonLanguage6();
	afx_msg void OnStaticButtonLanguage7();
	afx_msg void OnStaticButtonLanguage8();
	afx_msg void OnStaticButtonLanguage9();
	afx_msg void OnStaticButtonLanguage10();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private: // helper functions
	void DeInit();
	void DrawLines();				// draws all lines and rectangles 
    void SetupTextButtons();        // Load text buttons
	void SetupBitmaps();			// Load bitmaps
	void SetupFonts();				// Setup Fonts for all controls
	void SetupDefaults();			// Setup default Text
	void DrawBitmaps();							
	void ChangeLanguage();

    // Message Handlers
	LRESULT UpdateStatus(WPARAM iParam, LPARAM lParam);
    LRESULT HandleIntellioShaverCmd(WPARAM iParam, LPARAM lParam);

private: // local member variables
	
	CControl*		m_pControl;		// Pointer to a Control Layer Object
	CWnd*			m_pParent;		// Handle to Dialog window
	CSnHelp			m_SnHelp;		// Smith & Nephew helper class
	HBRUSH			m_hbr;
	SnWord			m_usLanguage;
	CBitmap			m_ButtonOn;
	CBitmap			m_ButtonOff;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LANGUAGESCREEN_H__864B986B_56F0_4B8D_8719_3E6935624CB7__INCLUDED_)
