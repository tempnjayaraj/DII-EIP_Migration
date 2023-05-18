#if !defined(AFX_CUSTOMOSCSCREEN_H__AFA68AF4_9E12_45D9_BF45_A6EBABA25DF1__INCLUDED_)
#define AFX_CUSTOMOSCSCREEN_H__AFA68AF4_9E12_45D9_BF45_A6EBABA25DF1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomOscScreen.h : header file
//
#include "ColorsFonts.h"
#include "SnHelp.h"
#include "SharedMemory.h"
/////////////////////////////////////////////////////////////////////////////
// CCustomOscScreen dialog

class CCustomOscScreen : public CDialog
{
// Construction
public:
	CCustomOscScreen(CControl* pControl, DWORD dwMode, DWORD dwPort, CWnd* pParent = NULL);   // standard constructor
	virtual ~CCustomOscScreen();

// Dialog Data
	//{{AFX_DATA(CCustomOscScreen)
	enum { IDD = IDD_OSC_MODE };
	CBitButton m_BtnSet;
	CBitButton m_BtnDefault;
	CBitButton m_BtnCancel;
	CStatic	m_StaticText3;
	CStatic	m_StaticText2;
	CStatic	m_StaticText;
	CStatic	m_ButtonUp;
	CStatic	m_ButtonDown;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustomOscScreen)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCustomOscScreen)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnButtonSet();
	afx_msg void OnButtonCancel();
	afx_msg void OnButtonDefault();
	afx_msg void OnStaticButtonDown();
	afx_msg void OnStaticButtonUp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void DeInit();
	void SetupDefaults();
    void SetupTextButtons();
    void SetupBitmaps();
	void DrawLines();
	void SetupFonts();
	void HideButtons();
	void UpdateRevolutions(unsigned char mode); 
	void UpdateSeconds(unsigned char mode);
 
    // Message Handlers
	LRESULT UpdateStatus(WPARAM iParam, LPARAM lParam);
    LRESULT HandleIntellioShaverCmd(WPARAM iParam, LPARAM lParam);

private:

	CWnd*			m_pParent;		// Handle to Dialog window
	CControl*		m_pControl;		// Pointer to a Control Layer Object
	CSnHelp			m_SnHelp;		// Smith & Nephew helper class
	HBRUSH			m_hbrBlack;
	SN_PORT_STATUS	m_tPortStatus;
	
	DWORD			m_dwPort;
	DWORD			m_dwMode;

	CBitmap			m_ArrowUp;
	CBitmap			m_ArrowUpPressed;
	CBitmap			m_ArrowDown;
	CBitmap			m_ArrowDownPressed;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOMOSCSCREEN_H__AFA68AF4_9E12_45D9_BF45_A6EBABA25DF1__INCLUDED_)
