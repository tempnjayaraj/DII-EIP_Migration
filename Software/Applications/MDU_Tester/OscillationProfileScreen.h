#if !defined(AFX_OSCILLATIONPROFILESCREEN_H__D56132B4_06EC_42A2_B3AB_07A83010F6B8__INCLUDED_)
#define AFX_OSCILLATIONPROFILESCREEN_H__D56132B4_06EC_42A2_B3AB_07A83010F6B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OscillationProfileScreen.h : header file
//

#include "ColorsFonts.h"
#include "SnHelp.h"
#include "SharedMemory.h"

//////////////////////////////////////////////////////////////////////////////
// COscillationProfileScreen dialog

class COscillationProfileScreen : public CDialog
{
// Construction
public:
	COscillationProfileScreen(CControl* pControl, CWnd* pParent = NULL);   // standard constructor
	virtual ~COscillationProfileScreen();				// destructor

// Dialog Data
	//{{AFX_DATA(COscillationProfileScreen)
	enum { IDD = IDD_OSCILLATION_PROFILE };
	CBitButton m_BtnDone;
	CBitButton m_BtnAdjustPortB;
	CBitButton m_BtnAdjustPortA;
	CStatic	m_StaticText2;
	CStatic	m_StaticText1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COscillationProfileScreen)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COscillationProfileScreen)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonDone();
	afx_msg void OnPaint();
	afx_msg void OnButtonAdjustPortA();
	afx_msg void OnButtonAdjustPortB();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private: // helper functions
	void DeInit();
	void DrawLines();				// draws all lines and rectangles 
	void SetupTextButtons();		// Load text buttons
	void SetupFonts();				// Setup Fonts for all controls
	void SetupDefaults();			// Setup default Text
	void GetPortStatus();
 
    // Message Handlers
	LRESULT UpdateStatus(WPARAM iParam, LPARAM lParam);
    LRESULT HandleIntellioShaverCmd(WPARAM iParam, LPARAM lParam);

private: // local member variables
	
	CWnd*			m_pParent;		// Handle to Dialog window
	CControl*		m_pControl;		// Pointer to a Control Layer Object
	CSnHelp			m_SnHelp;		// Smith & Nephew helper class

	SN_PORT_STATUS	m_tPortAStatus;
	SN_PORT_STATUS	m_tPortBStatus;

	HBRUSH			m_hbr;

    SnBool          m_yActiveDialog;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OSCILLATIONPROFILESCREEN_H__D56132B4_06EC_42A2_B3AB_07A83010F6B8__INCLUDED_)
