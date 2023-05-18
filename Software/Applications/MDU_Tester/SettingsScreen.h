#if !defined(AFX_SETTINGSSCREEN_H__8D5E5D03_12E1_4DE8_97DE_9C77046C18D4__INCLUDED_)
#define AFX_SETTINGSSCREEN_H__8D5E5D03_12E1_4DE8_97DE_9C77046C18D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SettingsScreen.h : header file
//
#include "ColorsFonts.h"
#include "SnHelp.h"
#include "OscillationProfileScreen.h"
#include "SystemInfoScreen.h"
#include "LanguageScreen.h"
#include "FootSwitchScreen.h"
#include "Control.h"

/////////////////////////////////////////////////////////////////////////////
// CSettingsScreen dialog

class CSettingsScreen : public CDialog
{
// Construction
public:
	CSettingsScreen(CControl* pControl, CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingsScreen();				// destructor

// Dialog Data
	//{{AFX_DATA(CSettingsScreen)
	enum { IDD = IDD_SETTINGS };
	CBitButton m_BtnSystemInformation;
	CBitButton m_BtnResetB;
	CBitButton m_BtnResetA;
	CBitButton m_BtnMode;
	CBitButton m_BtnLanguage;
	CBitButton m_BtnInterface;
	CBitButton m_BtnFootswitch;
	CBitButton m_BtnDone;
	CBitButton m_BtnCustomSettings;
	CStatic	m_StaticReset;
	CStatic	m_StaticMode;
	CStatic	m_StaticInterface;
	CStatic	m_StaticFootswitch;
	CStatic	m_StaticSystemInformation;
	CStatic	m_StaticLanguage;
	CStatic	m_StaticCustomSettings;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsScreen)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSettingsScreen)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonDone();
	afx_msg void OnButtonCustomSettings();
	afx_msg void OnButtonSystemInformation();
	afx_msg void OnButtonLanguage();
	afx_msg void OnButtonCustom();
	afx_msg void OnButtonFootswitch();
	afx_msg void OnButtonInterface();
	afx_msg void OnButtonMode();
	afx_msg void OnButtonResetBladeA();
	afx_msg void OnButtonResetBladeB();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private: // helper functions
	void DeInit();					// Frees all resources
	void DrawLines();				// draws all lines and rectangles 
	void SetupTextButtons();		// Load text buttons
	void SetupBitmaps();			// Load bitmaps
	void SetupFonts();				// Setup Fonts for all controls
	void SetupDefaults();			// Setup default Text

    // Message Handlers
	LRESULT	UpdateStatus(WPARAM iParam, LPARAM lParam);
    LRESULT HandleIntellioShaverCmd(WPARAM iParam, LPARAM lParam);

private: // local member variables
	
	CControl*		m_pControl;		// Pointer to a Control Layer Object
	CWnd*			m_pParent;		// Handle to Dialog window
	CSnHelp			m_SnHelp;		// Smith & Nephew helper class

	HBRUSH			m_hbr;

	SN_PORT_STATUS	m_tPortAStatus;
	SN_PORT_STATUS	m_tPortBStatus;
	SnWord			m_usSystemMode;

    SnBool          m_yActiveDialog;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSSCREEN_H__8D5E5D03_12E1_4DE8_97DE_9C77046C18D4__INCLUDED_)
