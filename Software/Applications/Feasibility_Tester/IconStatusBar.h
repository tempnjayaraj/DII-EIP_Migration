#if !defined(AFX_ICONSTATUSBAR_H__B6041B7C_1BC7_4B6C_977C_F1387E0755B8__INCLUDED_)
#define AFX_ICONSTATUSBAR_H__B6041B7C_1BC7_4B6C_977C_F1387E0755B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IconStatusBar.h : header file
//

#include "SharedMemory.h"
#include "ColorsFonts.h"
#include "SnHelp.h"

typedef struct
{
	CStatic         m_StaticPump;
	CStatic         m_StaticHand;
	CStatic         m_StaticFoot;

    CBitmap			m_BitmapMduIcon;
	CBitmap			m_BitmapDrillIcon;				
	CBitmap			m_BitmapSawIcon;
	CBitmap			m_BitmapFootPedalIcon;
} SN_PORT_ICONS;

/////////////////////////////////////////////////////////////////////////////
// CIconStatusBar dialog

class CIconStatusBar : public CDialog
{
// Construction
public:
	CIconStatusBar(CControl* pControl, int iTitle, CWnd* pParent = NULL);   // standard constructor
	virtual ~CIconStatusBar();				// destructor

// Dialog Data
	//{{AFX_DATA(CIconStatusBar)
	enum { IDD = IDD_ICON_STATUS_BAR };
	CStatic	m_StaticTitle;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIconStatusBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CIconStatusBar)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private: // helper functions
	void DrawLines();				// draws all lines and rectangles 
	void SetupBitmaps();			// Load bitmaps
	void SetupFonts();				// Setup Fonts for all controls
    void DrawImpeller(int iX, CBitmap *pBitmap);
	void SpinImpeller();
	void KillImpellerThread();
	void CreateImpellerThread();
	void UpdatePortStatus(SnWord usType, SN_PORT_ICONS* ptPortIcons);
	
	// Message Handlers
	LRESULT UpdateStatus(WPARAM iParam, LPARAM lParam);
	LRESULT UpdateRemoteStatus(WPARAM iParam, LPARAM lParam);

	// Threads
	friend DWORD WINAPI ImpellerThread(LPVOID pParam);
								
private: // local member variables
	CControl*		m_pControl;		// Pointer to a Control Layer Object
	CWnd*			m_pParent;		// Handle to Dialog window
	CSnHelp			m_SnHelp;		// Smith & Nephew helper class
	HBRUSH			m_hbr;
	SnBool			m_bSpinImpeller;
	SnBool			m_bKillImpellerThread;
	HANDLE			m_hImpellerThread;
	DWORD			m_hImpellerThreadID;
	HANDLE			m_hImpellerThreadKilledEvent;

	CStatic         m_StaticIntellio;

	CBitmap			m_BitmapImpellerGrayIcon;
	CBitmap			m_BitmapImpellerBlueIcon;
	CBitmap			m_BitmapIntellioIcon;
	CBitmap			m_BitmapImpellerRotate30Icon;
	CBitmap			m_BitmapImpellerRotate60Icon;
	
	SN_PORT_ICONS	m_tPortAIcons;
	SN_PORT_ICONS	m_tPortBIcons;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ICONSTATUSBAR_H__B6041B7C_1BC7_4B6C_977C_F1387E0755B8__INCLUDED_)
