//
// SoftwareUpgradeDlg.h : header file
//

#if !defined(AFX_SOFTWAREUPGRADEDLG_H__F0805CE7_44A3_4F28_8F1A_685498B80682__INCLUDED_)
#define AFX_SOFTWAREUPGRADEDLG_H__F0805CE7_44A3_4F28_8F1A_685498B80682__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Resource.h"
#include "ColorFonts.h"
#include "Driver.h"
#include "Util.h"
#if RS485_FOOT_UPGRADE || RS485_MDU_UPGRADE
#include "ControllerTypes.h"
#endif

#define MC_REVISION     0

#if RS485_FOOT_UPGRADE || RS485_MDU_UPGRADE
// Serial
#define MC_SERIAL_NUMWORDS	        (offsetof(Status_Control, tSerial.wNumWords))/2
#define MC_SERIAL_CMD_RESULTS       (offsetof(Status_Control, tSerial.wCmdResult))/2
#define MC_SERIAL_RCVERRCNT0        (offsetof(Status_Control, tSerial.wRcvErrCnt0))/2
#define MC_SERIAL_RCVERRCNT1        (offsetof(Status_Control, tSerial.wRcvErrCnt1))/2


typedef union SnWordBytes {
    struct {
        SnByte bLS;
        SnByte bMS;
    };
    struct {
        unsigned B0:1;
        unsigned B1:1;
        unsigned B2:1;
        unsigned B3:1;
        unsigned B4:1;
        unsigned B5:1;
        unsigned B6:1;
        unsigned B7:1;
        unsigned B8:1;
        unsigned B9:1;
        unsigned B10:1;
        unsigned B11:1;
        unsigned B12:1;
        unsigned B13:1;
        unsigned B14:1;
        unsigned B15:1;
    };
    struct {
        unsigned bVariableDataLS: 8;
        unsigned bVariableDataMS: 2;
        unsigned yAckNak: 1;
        unsigned bECC: 4;
        unsigned yParity: 1;
    };
    SnWord w;
} SnWordFlavors;

//
// Possible Mux Source Selections for Port A or Port B
//
#define MUX_RCV_HAND            4
#define MUX_XMT_HAND            5
#define MUX_RCV_FOOT            6
#define MUX_XMT_FOOT            7
#define MUX_MASK                0x0700
#define MUX_SHIFT               8

//
//  Bitfields for Command Requests
//
#define CMD_REQUEST_PORT        0x8000
#define CMD_REQUEST_BAUD        0x0800
#define CMD_REQUEST_ENABLE      0x0400
#define CMD_REQUEST_FOOT        0x0200
#define CMD_REQUEST_XMT         0x0100
#define CMD_REQUEST_DATA        0x00ff

#define PORTA					1
#define PORTB					2
#define HAND_PORTA				PORTA
#define HAND_PORTB				PORTB
#define FOOT_FRONT				3
#define FOOT_AUX				4

#define HAND_PORTA_CMD(cmd)         (CMD_REQUEST_BAUD|CMD_REQUEST_ENABLE|CMD_REQUEST_XMT|(cmd))
#define FOOT_FRONT_CMD(cmd)         (CMD_REQUEST_ENABLE|CMD_REQUEST_FOOT|CMD_REQUEST_XMT|(cmd))

#define SERIAL_CMD_VERS         0x00
#define SERIAL_CMD_DEV_TYPE     0xb1

#define SERIAL_CMD_REQ_2        0xd2
#define SERIAL_CMD_REQ_3        0x63
#define SERIAL_CMD_REQ_4        0xe4
#define SERIAL_CMD_REQ_5        0x55
#define SERIAL_CMD_REQ_6        0x36
#define SERIAL_CMD_REQ_7        0x87
#define SERIAL_CMD_REQ_8        0x78
#define SERIAL_CMD_REQ_9        0xc9
#define SERIAL_CMD_REQ_10       0xaa
#define SERIAL_CMD_REQ_11       0x1b
#define SERIAL_CMD_REQ_12       0x9c
#define SERIAL_CMD_REQ_13       0x2d
#define SERIAL_CMD_REQ_14       0x4e
#endif

typedef struct {
    SnByte bMajor;
    SnByte bMinor;
    SnByte bBuild;
    SnByte bValid;
} SN_REVISION;

/////////////////////////////////////////////////////////////////////////////
// CSoftwareUpgradeDlg dialog

class CSoftwareUpgradeDlg : public CDialog
{
// Construction
public:
	CSoftwareUpgradeDlg(CDriver* pDriver, FILE *ptFile,
        CWnd* pParent = NULL);	                // Standard constructor
    ~CSoftwareUpgradeDlg();                     // Standard destructor

// Dialog Data
	//{{AFX_DATA(CSoftwareUpgradeDlg)
	enum { IDD = IDD_SOFTWAREUPGRADE_DIALOG };
	CStatic	m_StaticStatus3;
	CStatic	m_StaticStatus2;
	CStatic	m_StaticStatus1;
	CStatic	m_StaticTitle;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSoftwareUpgradeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    HBRUSH m_hBrush;

	// Generated message map functions
	//{{AFX_MSG(CSoftwareUpgradeDlg)
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonDone();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnButtonStart();
	afx_msg void OnButtonCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:    // Local Functions
    void SetupFonts(void);
    void SetupButtons(void);
    void SetupRevisions(void);
    SnBool WriteVerifyFlashData(SnQByte qAddr, SnByte *pbData, SnQByte qBytes);
    SnBool SoftwareUpgrade(FILE *ptIn);
    void UpgradeSoftware(void);
    SnBool GetFileRevisions(void);
    SnBool GetMotorBoardRevision(SN_REVISION* ptRevision);
    SnBool GetSystemRevision(SN_REVISION* ptRevision);
#if RS485_FOOT_UPGRADE
    SnBool GetFootswitchRevision(SN_REVISION* ptRevision);
#endif
#if RS485_MDU_UPGRADE
    SnBool GetMduRevision(SN_REVISION* ptRevision);
#endif
#if RS485_FOOT_UPGRADE || RS485_MDU_UPGRADE
	SnBool ErrorCorrectSerialResponse(SnWord *pwResponse);
    SnBool SendSerialRequests(SnQByte qNumRequests, SnWord *pwResults, SnWord wTimeout);
#endif
    void StatusMsg(CString csMsg);
    friend DWORD WINAPI UpgradeSoftwareThread(LPVOID pParam);
    void SetupSerialNumberDisplay();
    void SetupUpgradeDisplay();
 
private:    // Local Meber Variables
	CDriver*        m_pDriver;		// Pointer to a Driver Object
    FILE*           m_ptFile;       // Handle to open Upgrade File on USB Drive
    HANDLE          m_hStartCancelEvent;
    SnBool          m_yCancel;
    SnBool          m_ySerialNumberCheck;
#if RS485_FOOT_UPGRADE
	SN_REVISION		m_tCurFootRev;
#endif
#if RS485_MDU_UPGRADE
	SN_REVISION		m_tCurMduRev;
#endif
    SN_REVISION     m_tCurSysRev;
    SN_REVISION     m_tCurMotorRev;
    SN_REVISION     m_tUpgSysRev;
    SN_REVISION     m_tUpgMotorRev;
#if RS485_FOOT_UPGRADE
	SN_REVISION		m_tUpgFootRev;
#endif
#if RS485_MDU_UPGRADE
	SN_REVISION		m_tUpgMduRev;
#endif
	CBitmapButton	m_BtnCancel;	// Handle to Bitmap/Button
	CBitmapButton	m_BtnStart;		// Handle to Bitmap/Button
	CBitmapButton	m_BtnDone;		// Handle to Bitmap/Button

	BOOL			m_bDetail;
    char            m_pcSerialNumber[SERIAL_NUMBER_SIZE];

};

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOFTWAREUPGRADEDLG_H__F0805CE7_44A3_4F28_8F1A_685498B80682__INCLUDED_)
