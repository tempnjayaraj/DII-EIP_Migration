#if !defined(AFX_SYSTEMINFOSCREEN_H__AC9ECA65_8AE9_4DD4_9162_E6A3E06A95A2__INCLUDED_)
#define AFX_SYSTEMINFOSCREEN_H__AC9ECA65_8AE9_4DD4_9162_E6A3E06A95A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SystemInfoScreen.h : header file
//
#include "CommonDefines.h"
#include "ColorsFonts.h"
#include "SnHelp.h"
#include "Control.h"
#include "SharedMemory.h"

/////////////////////////////////////////////////////////////////////////////
// CSystemInfoScreen dialog

class CSystemInfoScreen : public CDialog
{
// Construction
public:
	CSystemInfoScreen(CControl* pControl, CWnd* pParent = NULL);   // standard constructor
	virtual ~CSystemInfoScreen();				// destructor

// Dialog Data
	//{{AFX_DATA(CSystemInfoScreen)
	enum { IDD = IDD_SYSTEM_INFO };
	CStatic	m_StaticSerialNumber;
	CStatic	m_Static9;
	CBitButton m_BtnReset;
	CBitButton m_BtnDone;
	CStatic m_StaticVerPortB;
	CStatic m_StaticVerPortA;
	CStatic m_StaticVerFoot;
	CStatic	m_StaticVerMc;
	CStatic	m_StaticVerApp;
	CStatic m_Static8;
	CStatic m_Static7;
	CStatic m_Static6;
	CStatic	m_Static5;
	CStatic	m_Static4;
	CStatic	m_StaticVersionNumber;
	CStatic	m_Static3;
	CStatic	m_Static2;
	CStatic	m_Static1;
	CStatic	m_StaticModelNumber;
	CStatic m_StaticCopyright;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSystemInfoScreen)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSystemInfoScreen)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonDone();
	afx_msg void OnButtonReset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private: // helper functions
	void DeInit();
	void DrawLines();				// draws all lines and rectangles 
	void SetupTextButtons();		// Load text bitmaps
	void SetupFonts();				// Setup Fonts for all controls
	void SetupDefaults();			// Setup default Text	
	
	// Message Handler
	LRESULT UpdateStatus(WPARAM iParam, LPARAM lParam);
    LRESULT HandleIntellioShaverCmd(WPARAM iParam, LPARAM lParam);

private: // local member variables
	
	CControl*		m_pControl;		// Pointer to a Control Layer Object
	CWnd*			m_pParent;		// Handle to Dialog window
	CSnHelp			m_SnHelp;		// Smith & Nephew helper class
	HBRUSH			m_hbr;
    SnBool          m_yResetDlg;    // In Reset confirmation dialog

	SN_PORT_STATUS	m_tPortAStatus;
	SN_PORT_STATUS	m_tPortBStatus;

	void UpdateHandPieceVersion(long lPort, SN_SYS_REVISION &tRevision);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYSTEMINFOSCREEN_H__AC9ECA65_8AE9_4DD4_9162_E6A3E06A95A2__INCLUDED_)
