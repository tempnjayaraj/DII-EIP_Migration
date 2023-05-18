#if !defined(AFX_FOOTSWITCHSCREEN_H__066A5DE9_6152_4553_BDEF_3A69BE067EB9__INCLUDED_)
#define AFX_FOOTSWITCHSCREEN_H__066A5DE9_6152_4553_BDEF_3A69BE067EB9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FootSwitchScreen.h : header file
//
#include "ColorsFonts.h"
#include "SnHelp.h"
#include "SharedMemory.h"
/////////////////////////////////////////////////////////////////////////////
// CFootSwitchScreen dialog

class CFootSwitchScreen : public CDialog
{
// Construction
public:
	CFootSwitchScreen(CControl* pControl, CWnd* pParent = NULL);   // standard constructor
	virtual ~CFootSwitchScreen(); // Destructor

// Dialog Data
	//{{AFX_DATA(CFootSwitchScreen)
	enum { IDD = IDD_FOOTSWITCH };
	CBitButton m_BtnSet;
	CBitButton m_BtnCancel;
	CStatic	m_StaticText4;
	CStatic	m_StaticButtonPortB;
	CStatic	m_StaticButtonPortA;
	CStatic	m_StaticButtonRight;
	CStatic	m_StaticButtonOn;
	CStatic	m_StaticButtonOff;
	CStatic	m_StaticButtonLeft;
	CStatic	m_StaticButtonMode2;
	CStatic	m_StaticButtonMode1;
	CStatic	m_StaticText3;
	CStatic	m_StaticText2;
	CStatic	m_StaticText1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFootSwitchScreen)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFootSwitchScreen)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonCancel();
	afx_msg void OnButtonSet();
	afx_msg void OnPaint();
	afx_msg void OnStaticButtonMode1();
	afx_msg void OnStaticButtonMode2();
	afx_msg void OnStaticButtonLeft();
	afx_msg void OnStaticButtonRight();
	afx_msg void OnStaticButtonOn();
	afx_msg void OnStaticButtonOff();
	afx_msg void OnStaticButtonPorta();
	afx_msg void OnStaticButtonPortb();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void DeInit();
	void SetupDefaults();
	void SetupTextButtons();
	void SetupBitmaps();
	void DrawLines();
	void GetPortStatus();
	void DrawButtons();
	void SetupFonts();

	// Message Handlers
	LRESULT UpdateStatus(WPARAM iParam, LPARAM lParam);
    LRESULT HandleIntellioShaverCmd(WPARAM iParam, LPARAM lParam);

private:
	CWnd*			m_pParent;		// Handle to Dialog window
	CControl*		m_pControl;		// Pointer to a Control Layer Object
	CSnHelp			m_SnHelp;		// Smith & Nephew helper class
	HBRUSH			m_hbrBlack;

	SnWord			m_usLanguage;
	
	SnBool			m_bMode;
	SnBool			m_bForward;
	SnBool			m_bHandCtl;
	SnBool			m_bPortCtl;
	
	SN_FOOT_STATUS m_tFootStatus;
	SN_PORT_STATUS	m_tPortAStatus;
	SN_PORT_STATUS	m_tPortBStatus;
	
	CBitmap			m_ButtonOn;
	CBitmap			m_ButtonOff;	
	CBitmap			m_ButtonDisabled;	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FOOTSWITCHSCREEN_H__066A5DE9_6152_4553_BDEF_3A69BE067EB9__INCLUDED_)
