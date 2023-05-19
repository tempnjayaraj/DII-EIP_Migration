// TestRS485Dlg.h : header file
//

#if !defined(AFX_TestRS485DLG_H__67D09771_7877_48B7_9BD3_EC3347B042A0__INCLUDED_)
#define AFX_TestRS485DLG_H__67D09771_7877_48B7_9BD3_EC3347B042A0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Control.h"

#define TESTRS485_VERS_MAJOR			1
#define TESTRS485_VERS_MINOR			2
#define TESTRS485_VERS_BUILD			0


typedef union  
{
	struct {
	SnByte ucMotorBoardMajor;
	SnByte ucMotorBoardMinor;
    SnByte ucMotorBoardBuild;
	};
	SnQByte qMotorBoardVersion;
}SN_MOTOR_REVISION;

/////////////////////////////////////////////////////////////////////////////
// CTestRS485Dlg dialog

class CTestRS485Dlg : public CDialog
{
// Construction
public:
	CTestRS485Dlg(CControl* pControl, CWnd* pParent = NULL);	// standard constructor
	virtual ~CTestRS485Dlg();					                // destructor

// Dialog Data
	//{{AFX_DATA(CTestRS485Dlg)
	enum { IDD = IDD_TESTRS485_DIALOG };
	CButton	m_cbInvalidCommandTest;
	CStatic	m_scLeftPedalLabel;
	CStatic	m_scRightPedalLabel;
	CStatic	m_scMiddlePedalLabel;
	CStatic	m_scRightPedalPercent;
	CStatic	m_scMiddlePedalPercent;
	CStatic	m_scLeftPedalPercent;
	CStatic	m_scSoftwareVersionLabel;
	CStatic	m_scDeviceIdLabel;
	CButton	m_cbCalibrate;
	CStatic	m_scSoftwareVersion;
	CStatic	m_scDeviceId;
	CStatic	m_scCalibrate;
	CString	m_TextSoftwareVersion;
	CString	m_TextDeviceId;
	CString	m_TextCalibrateStatus;
	CString	m_TextLeftPedalPercent;
	CString	m_TextMiddlePedalPercent;
	CString	m_TextRightPedalPercent;
	CString	m_TextLeftButton;
	CString	m_TextRightButton;
	CString	m_TextResetStatus;
	CString	m_TextInvalidCommandTestStatus;
	CString	m_TextCommandDuration;
	CString	m_TextConnectDuration;
	CString	m_TextErrorCount;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestRS485Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CTestRS485Dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSoftwareVersion();
	afx_msg void OnDeviceId();
	afx_msg void OnDialog();
	afx_msg void OnExit();
	afx_msg void OnHelp();
	afx_msg void OnCalibrate();
	afx_msg void OnInvalidCommandTest();
	afx_msg void OnReset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	SnBool GetFootStatus(void);
	SnBool SetFootStatus(void);
    
	// Thread Functions
    SnBool Delay(DWORD dwMilliSecond) ;
    void KillThreads(void);
    void CheckForPOST(void);
    static void StatusThread(LPVOID pParam);

private:
    void			DeInit(void);
	void			SetupFonts();
	SnBool			TestInProgress(void);
	void			UpdateButtons(void);
	void			InvalidCommandTest(void);
	void			CalibrationTest(void);
	void			ResetTest(void);
	void			UpdateTitle(SnBool yForceUpdate);

	// Message Handlers
	LRESULT HandleErrorConditions(WPARAM iParam, LPARAM lParam);
	LRESULT ExitDialog(WPARAM iParam, LPARAM lParam);

	CDC				m_memDC;				// virtual window device context
	CControl*		m_pControl;				// Pointer to a Control Layer Object
	CWnd*			m_pParent;				// Handle to Dialog window
	SN_MOTOR_REVISION m_qMotorRev;
	CFont			m_Font;	
	CFont*          m_DefaultFont ;

    SnBool          m_yKillThreads;
    SnBool          m_yStatusThreadKilled;
    HANDLE          m_hStatusThread;
    DWORD           m_hStatusThreadID;
    HANDLE          m_hStatusThreadKilledEvent;
    HANDLE			m_hShutDownAppEvent;

	SN_FOOT_STATUS	m_tFootPedalStatus;
	SN_FOOT_STATUS	m_tPrevFootPedalStatus;
	SN_SYS_REVISION m_tRevision;
	SN_SYS_REVISION m_tPrevRevision;

	SnBool			m_bCalibrationTest;
	SnBool			m_bInvalidCommandTest;
	SnBool			m_bResetTest;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TestRS485DLG_H__67D09771_7877_48B7_9BD3_EC3347B042A0__INCLUDED_)
