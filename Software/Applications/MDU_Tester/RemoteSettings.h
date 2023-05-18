#if !defined(AFX_REMOTESETTINGS_H__F5A7AB89_4DAC_438F_B604_C3FE106CC84F__INCLUDED_)
#define AFX_REMOTESETTINGS_H__F5A7AB89_4DAC_438F_B604_C3FE106CC84F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RemoteSettings.h : header file
//
#include "ColorsFonts.h"
#include "SnHelp.h"
#include "Control.h"
/////////////////////////////////////////////////////////////////////////////
// CRemoteSettings dialog

class CRemoteSettings : public CDialog
{
// Construction
public:
	CRemoteSettings(CControl* pControl, CWnd* pParent = NULL);   // standard constructor
	virtual ~CRemoteSettings();				// destructor
// Dialog Data
	//{{AFX_DATA(CRemoteSettings)
	enum { IDD = IDD_REMOTE_SETTINGS };
	CBitButton m_BtnSet;
	CBitButton m_BtnCancel;
	CStatic	m_StaticText1;
	CStatic	m_StaticButtonPortB;
	CStatic	m_StaticButtonPortA;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRemoteSettings)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRemoteSettings)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonCancel();
	afx_msg void OnButtonSet();
	afx_msg void OnStaticButtonPortA();
	afx_msg void OnStaticButtonPortB();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void DeInit();					// Frees all resources
	void SetupTextButtons();
	void SetupBitmaps();
	void DrawLines();
	void DrawButtons();
	void SetupDefaults();
	void SetupFonts();

    // Message Handlers
	LRESULT	UpdateStatus(WPARAM iParam, LPARAM lParam);
    LRESULT HandleIntellioShaverCmd(WPARAM iParam, LPARAM lParam);

private:

	CControl*		m_pControl;		// Pointer to a Control Layer Object
	CWnd*			m_pParent;		// Handle to Dialog window
	CSnHelp			m_SnHelp;		// Smith & Nephew helper class
	SnBool			m_bDrawLines;
	SnWord			m_usShaverPacketCtl;
	HBRUSH			m_hbr;
	SnWord			m_usLanguage;

	CBitmap			m_ButtonOn;
	CBitmap			m_ButtonOff;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REMOTESETTINGS_H__F5A7AB89_4DAC_438F_B604_C3FE106CC84F__INCLUDED_)
