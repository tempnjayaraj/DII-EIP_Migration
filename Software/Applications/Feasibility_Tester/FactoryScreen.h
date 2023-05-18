#if !defined(AFX_FACTORYSCREEN1_H__50CD9854_2F1A_45F0_A4CF_5A5D250E6A49__INCLUDED_)
#define AFX_FACTORYSCREEN1_H__50CD9854_2F1A_45F0_A4CF_5A5D250E6A49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FactoryScreen1.h : header file
//
#include "SharedMemory.h"
#include "CommonDefines.h"
#include "ColorsFonts.h"
#include "SnHelp.h"
#include "Control.h"

/////////////////////////////////////////////////////////////////////////////
// CFactoryScreen dialog

#define NO_TEST         0
#define BOARD_TEST      1
#define SYSTEM_TEST     2

#define MOTOR_CONTROLLER    1
#define SYSTEM_CONTROLLER   2
#define MOTHERBOARD         4

//
// Handpiece Test Detailed Error List
//
#define HAND_TEST_DSP               0x00000001  // Internal Communication Error
#define HAND_TEST_TACH              0x00000002  // X_TACH signals changed with no motor
#define HAND_TEST_24V_GND           0x00000004  // 24V short to ground detected
#define HAND_TEST_FET_A_TOP_GND     0x00000008  // FET A Top short to ground or X_MotorCurrent bad
#define HAND_TEST_FET_A_BOT_GND     0x00000010  // FET A Bottom short to ground or X_MotorCurrent bad
#define HAND_TEST_FET_B_TOP_GND     0x00000020  // FET B Top short to ground or X_MotorCurrent bad
#define HAND_TEST_FET_B_BOT_GND     0x00000040  // FET B Bottom short to ground or X_MotorCurrent bad
#define HAND_TEST_FET_C_TOP_GND     0x00000080  // FET C Top short to ground or X_MotorCurrent bad
#define HAND_TEST_FET_C_BOT_GND     0x00000100  // FET C Bottom short to ground or X_MotorCurrent bad
#define HAND_TEST_CURRENT_OR_LIMIT  0x00000200  // X_MotorCurrent or X_MotorFault bad
#define HAND_TEST_FET_A_C_OPEN      0x00000400  // FET pair A-C open or X_MotorCurrent bad
#define HAND_TEST_FET_B_A_OPEN      0x00000800  // FET pair B-A open or X_MotorCurrent bad
#define HAND_TEST_FET_B_C_OPEN      0x00001000  // FET pair B-C open or X_MotorCurrent bad
#define HAND_TEST_FET_C_B_OPEN      0x00002000  // FET pair C-B open or X_MotorCurrent bad
#define HAND_TEST_FET_A_B_OPEN      0x00004000  // FET pair A-B open or X_MotorCurrent bad
#define HAND_TEST_FET_C_A_OPEN      0x00008000  // FET pair C-A open or X_MotorCurrent bad
#define HAND_TEST_LOGIC_0           0x00010000  // X_Logic_0 bad
#define HAND_TEST_LOGIC_1           0x00020000  // X_Logic_1 bad
#define HAND_TEST_LOGIC_2           0x00040000  // X_Logic_2 bad
#define HAND_TEST_RS485             0x00080000  // X_TRCVR_LO or X_TRCVR_HI bad
#define HAND_TEST_HALLBUS           0x00100000  // X_Hall_Bus bad
#define HAND_TEST_DRILL_DIR         0x00200000  // X_Drill_Direction bad
#define HAND_TEST_DRILL_SPEED       0x00400000  // X_DrillSpeed bad
#define HAND_TEST_PHASE             0x00800000  // Phase Error

//
// Footswitch Test Detailed Error List
//
#define FOOT_TEST_FWD               0x00000001  // X_Foot_FWD_ISO bad
#define FOOT_TEST_REV               0x00000002  // X_Foot_REV_ISO bad
#define FOOT_TEST_LOGIC2            0x00000004  // X_Footswitch2 bad
#define FOOT_TEST_OSC               0x00000008  // X_Foot_OSC_ISO bad
#define FOOT_TEST_LOGIC0            0x00000010  // X_Footswitch0 bad
#define FOOT_TEST_LOGIC1            0x00000020  // X_Footswitch1 bad
#define FOOT_TEST_DSP               0x00000040  // Internal Communication Error
#define FOOT_TEST_RS485             0x00000080  // TRCVR U41 or TRCVR U43
#define FOOT_TEST_HALLBUS           0x00000100  // X_Footswitch_Hall_Bus bad

class CFactoryScreen : public CDialog
{
// Construction
public:
	CFactoryScreen(CControl* pControl, DWORD dwMode,
        CWnd* pParent = NULL);                          // constructor
	~CFactoryScreen();					                // destructor

// Dialog Data
	//{{AFX_DATA(CFactoryScreen)
	enum { IDD = IDD_FACTORY };
	CStatic	m_StaticUsageCnt;
	CBitButton m_BtnSet;
	CBitButton m_BtnRight;
	CBitButton m_BtnMiddle;
	CStatic	m_StaticTestTitle;
	CStatic	m_StaticTestText;
	CStatic	m_StaticTestListTitle;
	CStatic	m_StaticInstructionsTitle;
	CStatic	m_StaticInstructionsText;
	CEdit	m_EditTestListText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFactoryScreen)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFactoryScreen)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonMiddle();
	afx_msg void OnButtonRight();
	afx_msg void OnButtonSet();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    void DeInit(void);
    SnBool LoadDrivers(void);
	void SetupFonts();		// Setup Fonts for all controls
	void SetupBitmaps();	// Setup Bitmap button controls

	// Message Handlers
	LRESULT HandleErrorConditions(WPARAM iParam, LPARAM lParam);

    // Test Helper Functions
    void StartNewTest(CString csTest);
    void ReportTestPassed(void);
    void ReportTestFailed(CString csError);
    void AddInstructions(CString csIntructions);
    void NewInstructions(CString csIntructions);
    SnBool WaitForOK(void);
    SnBool WaitForPassFail(void);
    SnBool WaitForChangePassFail(void);
    HANDLE SetupSerialPort(TCHAR *pwPort, DWORD dwBaudRate);
    SnBool TestSerialPortLoopback(TCHAR *pwPort, DWORD dwBaudRate);
    void DrawButtonTexts(void);
    void SelectButton(CStatic *pStatic, CString csLabel);
    void DisplayTemperatures(void);
    SnBool HandpiecePhaseTest(int iPort, SnQByte *pqError);

    // Tests
    void PerformTests(void);
    SnBool SerialNumberTest(void);
    SnBool NvRamTest(void);
    SnBool SerialPortTest(void);
    SnBool TemperatureTest(void);
    SnBool BuzzerTest(void);
    SnBool UsbTest(void);
    SnBool DisplayTest(void);
    SnBool CanTest(void);
    SnBool HandpieceTest(int iPort);
    SnBool FootswitchTest(void);
    SnBool TouchTest(void);
    SnBool PowerButtonTest(void);

    // Thread Functions
    SnBool Delay(DWORD dwMilliSecond) ;
    void KillThreads(void);
    void CheckForPOST(void);
    friend DWORD WINAPI FactoryStatusThread(LPVOID pParam);

private:	
	HBRUSH			m_hBrush;
	CWnd*			m_pParent;		// Handle to Dialog window
	CControl*		m_pControl;		// Pointer to a Control Layer Object	
	CSnHelp			m_SnHelp;		// Smith & Nephew helper class
    SnBool          m_yKillThreads;
    DWORD           m_dwMode;

    SnBool          m_yPostComplete;
    SnWord          m_wTestMode;
    SnBool          m_yTemperatureTest;
    SnBool          m_yNvRamTest;
    SnBool          m_yTouchTest;
    SnBool          m_yTouchingScreen;

    SnQByte         m_qTouchPressCnt;
    SnQByte         m_qTouchReleaseCnt;
    SnQByte         m_qTouchOutOfBoundsCnt;

    CDC*            m_pTouchCDC;
    POINT           m_tOldTouchCursor;
    RECT            m_tOldTouchBox;

    float           m_fBoardTemp;
    float           m_fDspTemp;

    SnBool          m_yMiddlePressed;
    CString         m_csMiddleLabel;
    SnBool          m_yRightPressed;
    CString         m_csRightLabel;
    SnBool          m_ySetPressed;
    CString         m_csSetLabel;

    SnBool          m_yStatusThreadKilled;
    HANDLE          m_hStatusThread;
    DWORD           m_hStatusThreadID;
    HANDLE          m_hStatusThreadKilledEvent;

    CString         m_csTestList;
    CString         m_csInstructionList;

    SnBool          m_ySystemError;
    SnQByte         m_qHandpieceError;

    SnBool          m_yFailedTest;
    SnBool          m_yRepeatTest;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FACTORYSCREEN1_H__50CD9854_2F1A_45F0_A4CF_5A5D250E6A49__INCLUDED_)
