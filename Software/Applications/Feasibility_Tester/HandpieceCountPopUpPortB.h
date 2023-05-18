#if !defined(AFX_HANDPIECEPOPUPPORTB_H__B72945D8_4DE2_4FF0_951C_F9C94C470B3A__INCLUDED_)
#define AFX_HANDPIECEPOPUPPORTB_H__B72945D8_4DE2_4FF0_951C_F9C94C470B3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HandpieceCountPopUpPortB.h : header file
//
#include "ColorsFonts.h"
#include "SnHelp.h"
#include "SharedMemory.h"
/////////////////////////////////////////////////////////////////////////////
// CHandpieceCountPopUpPortB dialog

class CHandpieceCountPopUpPortB : public CDialog
{
// Construction
public:
	CHandpieceCountPopUpPortB(CControl* pControl, CWnd* pParent = NULL);   // standard constructor
	virtual ~CHandpieceCountPopUpPortB();				 // destructor
// Dialog Data
	//{{AFX_DATA(CHandpieceCountPopUpPortB)
	enum { IDD = IDD_HANDPIECE_COUNT_POPUP_PORTB };
	CBitButton m_BtnYes;
    CBitButton m_BtnNo;
	CStatic	m_StaticTitle;
	CStatic	m_StaticText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHandpieceCountPopUpPortB)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CHandpieceCountPopUpPortB)
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
    void UpdateMotorRunTime(SnBool yDisplay);
    void UpdateShaverPowerOnCount(SnBool yDisplay);
	//static DWORD WINAPI UpdateStatus(LPVOID pParam);
    LRESULT CHandpieceCountPopUpPortB::UpdateStatus(WPARAM iParam, LPARAM lParam);

private:
	CControl*		m_pControl;		// Pointer to a Control Layer Object
	HBRUSH			m_hbrWhite;
	HBRUSH			m_hbrYellow;
	CString			m_csNewLanguage;
	SnWord			m_usCurrentLanguage;
	SnWord			m_usNewLanguage;
	CSnHelp			m_SnHelp;		// Smith & Nephew helper class

    SnQByte         m_lOldMotorRunTime;
    SnQByte         m_lOldShaverPowerOnCount;

    SN_PORT_STATUS  m_tPortBStatus;

    CString			m_csTextMotorRunTime;
    CString         m_csTextShaverPowerOnCount;

    SnBool          m_bKillThreads;
    
    HANDLE          m_hDisplayThread;
    DWORD           m_hDisplayThreadID;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HANDPIECEPOPUPPORTB_H__B72945D8_4DE2_4FF0_951C_F9C94C470B3A__INCLUDED_)
