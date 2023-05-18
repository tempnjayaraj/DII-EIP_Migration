// TestAppDlg.h : header file
//

#if !defined(AFX_TESTAPPDLG_H__EBD70C0D_C25B_45C1_B6B6_2B71F2947B57__INCLUDED_)
#define AFX_TESTAPPDLG_H__EBD70C0D_C25B_45C1_B6B6_2B71F2947B57__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "SnTypes.h"
#include "Driver.h"
#include "Logger.h"
#include "ConfigDlg.h"
#include "SetDateTimeDlg.h"

#define FILE_NAME_DATA_COLLECTION	_T("\\Hard Disk\\Data.csv")

#define TESTAPP_VERS_MAJOR			1
#define TESTAPP_VERS_MINOR			0
#define TESTAPP_VERS_BUILD			1

#define TMP_BUF_SIZE			1024
#define MAX_FILE_SIZE			0xFFFFFFFF

typedef struct {
    char pcName[32];
    SnQByte qBits;
    SnQByte qType;
    SnQByte qOffset;
    SnQByte qWriteData;
    SnQByte qFormat;
} FlashConfig;

/////////////////////////////////////////////////////////////////////////////
// CTestAppDlg dialog

class CTestAppDlg : public CDialog
{
// Construction
public:
	CTestAppDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CTestAppDlg();	// default destructor

	// Dialog Data
	//{{AFX_DATA(CTestAppDlg)
	enum { IDD = IDD_TESTAPP_DIALOG };
	CEdit	m_EditReadWriteConfig9;
	CEdit	m_EditReadWriteConfig8;
	CEdit	m_EditReadWriteConfig7;
	CEdit	m_EditReadWriteConfig12;
	CEdit	m_EditReadWriteConfig11;
	CEdit	m_EditReadWriteConfig10;
	CButton	m_ButtonWriteWindow9;
	CButton	m_ButtonWriteWindow8;
	CButton	m_ButtonWriteWindow7;
	CButton	m_ButtonWriteWindow12;
	CButton	m_ButtonWriteWindow11;
	CButton	m_ButtonWriteWindow10;
	CButton	m_StopDataCollection;
	CButton	m_StartDataCollection;
	CEdit	m_EditReadWriteConfig6;
	CEdit	m_EditConfig6Static;
	CEdit	m_EditReadWriteConfig5;
	CEdit	m_EditConfig5Static;
	CEdit	m_EditReadWriteConfig4;
	CEdit	m_EditConfig4Static;
	CEdit	m_EditReadWriteConfig3;
	CEdit	m_EditConfig3Static;
	CButton	m_ButtonWriteWindow6;
	CButton	m_ButtonWriteWindow5;
	CButton	m_ButtonWriteWindow4;
	CButton	m_ButtonWriteWindow3;
	CEdit	m_EditReadWriteConfig2;
	CEdit	m_EditConfig2Static;
	CButton	m_ButtonWriteWindow2;
	CButton	m_ButtonStartConfig1;
	CButton	m_ButtonEnterConfig1;
	CButton m_ButtonWriteWindow1;
	CEdit	m_EditReadWriteConfig1;
	CEdit	m_EditConfig1Static;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestAppDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CTestAppDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonConfig1();
	afx_msg void OnButtonStartConfig1();
	afx_msg void OnButtonEnterConfig1();
	afx_msg void OnButtonWriteWindow1();
	afx_msg void OnButtonCal();
	afx_msg void OnSetfocusEditReadWriteConfig1();
	afx_msg void OnButtonConfig2();
	afx_msg void OnButtonWriteWindow2();
	afx_msg void OnSetfocusEditReadWriteConfig2();
	afx_msg void OnButtonConfig3();
	afx_msg void OnButtonWriteWindow3();
	afx_msg void OnSetfocusEditReadWriteConfig3();
	afx_msg void OnButtonConfig4();
	afx_msg void OnButtonWriteWindow4();
	afx_msg void OnSetfocusEditReadWriteConfig4();
	afx_msg void OnButtonConfig5();
	afx_msg void OnButtonWriteWindow5();
	afx_msg void OnSetfocusEditReadWriteConfig5();
	afx_msg void OnButtonConfig6();
	afx_msg void OnButtonWriteWindow6();
	afx_msg void OnSetfocusEditReadWriteConfig6();
	afx_msg void OnButtonSaveConfiguration();
	afx_msg void OnButtonStartWriteDiskThread();
	afx_msg void OnButtonStopWriteDiskThread();
	afx_msg void OnButtonConfig7();
	afx_msg void OnButtonConfig8();
	afx_msg void OnButtonConfig9();
	afx_msg void OnButtonConfig10();
	afx_msg void OnButtonConfig11();
	afx_msg void OnButtonConfig12();
	afx_msg void OnButtonWriteWindow7();
	afx_msg void OnButtonWriteWindow8();
	afx_msg void OnButtonWriteWindow9();
	afx_msg void OnButtonWriteWindow10();
	afx_msg void OnButtonWriteWindow11();
	afx_msg void OnButtonWriteWindow12();
	afx_msg void OnSetfocusEditReadWriteConfig7();
	afx_msg void OnSetfocusEditReadWriteConfig8();
	afx_msg void OnSetfocusEditReadWriteConfig9();
	afx_msg void OnSetfocusEditReadWriteConfig10();
	afx_msg void OnSetfocusEditReadWriteConfig11();
	afx_msg void OnSetfocusEditReadWriteConfig12();
	afx_msg void OnButtonStartDataCollection();
	afx_msg void OnButtonStopDataCollection();
	afx_msg void OnSetFocusChangeEditDownTime();
	afx_msg void OnButtonSetTime();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void UpdateTitle(SnBool yForceUpdate);
	void SetupScreen( void);
    void UpdateControlInt(int iID, SnQByte qData, CONFIG *ptConfig, SnBool yForceUpdate, SnBool yValid);
    void ConvertDlgItemNum(CONFIG *ptConfig, int iID);
    SnQByte GetDlgItemNum(int iID);
    SnBool WriteWindowData(CONFIG *ptConfig, int iID);
    void SaveWindow(CONFIG *ptConfig, FlashConfig *ptFlashConfig, int iID);
	SnBool SaveConfigurationInfo();
    void RecallWindow(CONFIG *ptConfig, FlashConfig *ptFlashConfig, int iID);
	SnBool RecallConfigurationInfo();
	void  GetBuffer( char* dest, CString csSource);
	void StripParam(char * p_param_buf);
	char* Slower(char * str);
	char* Supper(char * str);
	void ShowTime();
	friend void UpdateWindow(CTestAppDlg *pClass, CONFIG *ptConfig, int iID, SnQByte *pqRecordCount);
    friend DWORD WINAPI UpdateDisplayThread(LPVOID pParam);

private:
	CLogger*	m_hLogger;
	CDriver*	m_hDriver;
	DWORD		m_SaveCount;
	DWORD		m_SetupCount;
	HANDLE		m_hUpdateDisplayThread;
    HANDLE      m_hShutDownAppEvent;
	CONFIG		m_ConfigOne;
	CONFIG		m_ConfigTwo;
	CONFIG		m_ConfigThree;
	CONFIG		m_ConfigFour;
	CONFIG		m_ConfigFive;
	CONFIG		m_ConfigSix;
	CONFIG		m_ConfigSeven;
	CONFIG		m_ConfigEight;
	CONFIG		m_ConfigNine;
	CONFIG		m_ConfigTen;
	CONFIG		m_ConfigEleven;
	CONFIG		m_ConfigTwelve;

	CWnd*		m_pParent;
	SnBool		m_FileSetup;
	SnBool		m_KillUpdateThread;
	SnBool		m_ShutDown;
	SnBool		m_SaveData;

	SnBool		m_WindowsDirty;

	DWORD		m_Sleep;

	SnQByte		m_qMotorRev;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTAPPDLG_H__EBD70C0D_C25B_45C1_B6B6_2B71F2947B57__INCLUDED_)
