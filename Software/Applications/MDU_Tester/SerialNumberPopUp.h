#if !defined(AFX_SERIALNUMBERPOPUP_H__64593335_901D_43D0_A3C7_495E9E775F74__INCLUDED_)
#define AFX_SERIALNUMBERPOPUP_H__64593335_901D_43D0_A3C7_495E9E775F74__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SerialNumber.h : header file
//
#include "SnTypes.h"
#include "SnHelp.h"
#include "SharedMemory.h"
#include "Driver.h"

extern volatile SnWord wReliantLongSerialNumberEnable;

/////////////////////////////////////////////////////////////////////////////
// CSerialNumberPopUp dialog

class CSerialNumberPopUp : public CDialog
{
// Construction
public:
	CSerialNumberPopUp(CControl* pControl, SnWord wEnteringFromScreen, CWnd* pParent = NULL);   // standard constructor
	virtual ~CSerialNumberPopUp();                      // destructor

// Dialog Data
	//{{AFX_DATA(CSerialNumberPopUp)
	enum { IDD = IDD_SERIAL_NUMBER };
	CStatic	m_StaticTitle;
	CStatic	m_StaticSerialNumber;
	CStatic	m_StaticInstructions;
	CBitButton	m_BtnPadEnter;
	CBitButton	m_BtnPadDel;
	CBitButton	m_BtnPad9;
	CBitButton	m_BtnPad8;
	CBitButton	m_BtnPad7;
	CBitButton	m_BtnPad6;
	CBitButton	m_BtnPad5;
	CBitButton	m_BtnPad4;
	CBitButton	m_BtnPad3;
	CBitButton	m_BtnPad2;
	CBitButton	m_BtnPad1;
	CBitButton	m_BtnPad0;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSerialNumberPopUp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSerialNumberPopUp)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonNumpad0();
	afx_msg void OnButtonNumpad1();
	afx_msg void OnButtonNumpad2();
	afx_msg void OnButtonNumpad3();
	afx_msg void OnButtonNumpad4();
	afx_msg void OnButtonNumpad5();
	afx_msg void OnButtonNumpad6();
	afx_msg void OnButtonNumpad7();
	afx_msg void OnButtonNumpad8();
	afx_msg void OnButtonNumpad9();
	afx_msg void OnButtonNumpadDel();
	afx_msg void OnButtonNumpadEnter();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private: // helper functions
	void DeInit();
	void SetupTextButtons();		// Load text buttons
	void SetupFonts();				// Setup Fonts for all controls
    void AddDigit(char cDigit);

private: // local member variables
	CControl*		m_pControl;		// Pointer to a Control Layer Object
	CWnd*			m_pParent;		// Handle to Dialog window
	CSnHelp			m_SnHelp;		// Smith & Nephew helper class
	HBRUSH			m_hbrBlack;
    char            m_pcSerialNumber[SERIAL_NUMBER_STORE];
    SnWord          m_wEnteringFromScreen;
    SN_PORT_STATUS  m_tPortAStatus;
    SN_PORT_STATUS  m_tPortBStatus;
    CString         m_csTextPcSerialNumber;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERIALNUMBERPOPUP_H__64593335_901D_43D0_A3C7_495E9E775F74__INCLUDED_)
