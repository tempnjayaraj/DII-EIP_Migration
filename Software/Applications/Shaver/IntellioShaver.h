// IntellioShaver.h: interface for the CIntellioShaver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONNECTEDPUMP_H__2DD91014_8043_4313_BE39_7ABBC5E39B7B__INCLUDED_)
#define AFX_CONNECTEDPUMP_H__2DD91014_8043_4313_BE39_7ABBC5E39B7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NVRAMFlash.h"
#include "IntellioLink.h"
#include "RemoteCmds.h"

typedef struct
{
    SnByte bDeviceReady:        1;
} SCD_DEV_STATUS;

typedef struct
{
    SnByte bPortAUnits:         2;
    SnByte bPortABlade:         2;
    SnByte bPortAMode:          2;
    SnByte bPortAUpArrow:       1;
    SnByte bPortADownArrow:     1;

    SnByte bPortASetSpeed:      7;
    SnByte bPortARunState:      1;

    SnByte bPortBUnits:         2;
    SnByte bPortBBlade:         2;
    SnByte bPortBMode:          2;
    SnByte bPortBUpArrow:       1;
    SnByte bPortBDownArrow:     1;

    SnByte bPortBSetSpeed:      7;
    SnByte bPortBRunState:      1;

    SnByte bPortAErrWarn:       4;
    SnByte bPortBErrWarn:       4;

    SnByte bCapitalDevicePopup: 3;
    SnByte bHandpieceOverride:  1;
    SnByte bBladeRecall:        1;
    SnByte bPumpPort:           1;
    SnByte bFootswitchPort:     1;
    SnByte bSettingsScreen:     1;
} SCD_PORT_STATUS;

// Port Units
#define SCD_NO_DEVICE           0
#define SCD_UNIT_RPM            1
#define SCD_UNIT_RATE           2
#define SCD_UNIT_PERCENT        3

// Port Blades
#define SCD_BLADE_LOW           0
#define SCD_BLADE_MEDIUM        1
#define SCD_BLADE_HIGH          2
#define SCD_BLADE_OTHER         3

// Port Modes
#define SCD_MODE_FORWARD        0
#define SCD_MODE_REVERSE        1
#define SCD_MODE_OSCILLATE_1    2
#define SCD_MODE_OSCILLATE_2    3

// Port Arrows
#define SCD_ARROW_DISABLED      0
#define SCD_ARROW_ENABLED       1

// Port Run State
#define SCD_STATE_STOPPED       0
#define SCD_STATE_RUNNING       1

// Pump and Footswitch Port
#define SCD_PORT_A              0
#define SCD_PORT_B              1

// Capital Device Popups
#define SCD_PU1					1
#define SCD_PU2					2
#define SCD_PU3					3
#define SCD_PU4					4

// Warnings and Errors
#define SCD_PW1					0x1
#define SCD_PW2					0x2
#define SCD_PW3					0x3
#define SCD_PW4					0x4
#define SCD_PW5					0x5
#define SCD_PW6					0x6
#define SCD_PW7					0x7
#define SCD_PW8					0x8
#define SCD_PW10				0x9
#define SCD_PW11				0xA
#define SCD_PW12				0xB
#define SCD_PW13				0xC
#define SCD_PW14				0xD
#define SCD_PW15				0xE
#define SCD_PW16				0xF

typedef union
{
    struct {
        SnByte bHandpieceOverride:  2;
        SnByte bBladeRecall:        2;
        SnByte bPumpPort:           2;
        SnByte bFootswitchPort:     2;
    } b;
    SnByte bByte;
} SCD_SET;

#define	INTELLIO_SHAVER_CONNECTION_STATUS_EVENT	_T("IntellioShaverConnectionStatus")
#define INTELLIO_SHAVER_SET		                _T("IntellioShaverSet")
#define INTELLIO_SHAVER_COMMAND_READY		    _T("IntellioShaverCommandReady")

#define SET_HAS_CHANGED(bField)   ((tSet.b.bField > 0 && tSet.b.bField < 3) && ((tSet.b.bField - 1) != m_UpdateStatus.bField))

typedef struct {
	NVRAM_DATA		    m_tNvRamImage;
	SAVE_DEVICE_DATA	m_ptFlashPortImage[2];
} SCD_SETUP_BLOB_DATA;

#define SETUP_BLOB_HDR_SIZE     23
#define SETUP_BLOB_DATA_SIZE    sizeof(SCD_SETUP_BLOB_DATA)+23
#define SETUP_BLOB_TOTAL_SIZE   (SETUP_BLOB_HDR_SIZE + SETUP_BLOB_DATA_SIZE + 1)

// Serial Protcol Version Number
#define SCD_PROTOCOL_MAJOR      2
#define SCD_PROTOCOL_MINOR      1

#define SCD_PROTOCOL_VERSION    ((SCD_PROTOCOL_MAJOR << 4) | SCD_PROTOCOL_MINOR)
#define SCD_PROTOCOL_MATCH(v)   (SCD_PROTOCOL_MAJOR == ((v >> 4) & 0xf))

class CControl;

class CIntellioShaver : public CIntellioLink
{
public:
	CIntellioShaver();
	virtual ~CIntellioShaver();

    SnBool Init(void);
    void DeInit(void);

    void NoHandpiecePresent(DWORD dwPort);
    void UpdatePortUnitsAndMode(DWORD dwPort, SnByte bPortUnits, SnByte bPortMode);
    void UpdatePortBlade(DWORD dwPort, SnWord wBladeId);
    void UpdatePortArrows(DWORD dwPort, SnBool yUpArrow, SnBool yDownArrow);
    void UpdatePortSetSpeed(DWORD dwPort, SnByte bSetSpeed);
    void UpdatePortRunState(DWORD dwPort, SnBool yRunning);
    void UpdatePortErrWarn(DWORD dwPort, SnByte bErrWarn);
    void UpdatePopup(SnByte bPopup);
    void UpdateNvRamImage(NVRAM_DATA* ptNvRamImage);
    void UpdateFlashImage(SnByte* pbFlashImage, SnQByte qFlashSize);
    void UpdateSettingsScreen(SnBool yInSettingsScreen);
    void SendUpdateIfChange(void);
    void SendSerialNumber(SnByte bDevice);


	inline SnBool GetConnectionStatus(){ return m_yConnected;}
	inline void SetReadyStatus(SnBool yReady){ m_yDeviceReady =  yReady;}
    inline ConfigChangePending(){ return m_yRcvSetupBlobPending; }
	inline DWORD GetNewCommand(){ return m_dwNewCmd;}

    // Return Connected Pump Status
    inline SnBool GetPumpStatus(CPump::PumpType *pePumpType)
    {
        *pePumpType = m_yPumpConnection ? CPump::PUMP_TYPE_DYONICS25 : CPump::PUMP_TYPE_UNKNOWN;
        return m_yPumpRunning;
    }

    SnBool SetParameters(CControl *pControl);
	void SendIntellioShaverEvent(SnByte bEvent);
    void UpdateHandpieceSerialNumber(DWORD dwPort, char *pcSerialNumberStr);
    void UpdateCapitalDeviceSerialNumber(char *pcSerialNumberStr);
    void ConfigureFromRcvSetupBlob(CControl *pControl);

    inline void XmtRequestMessage(SnByte bCmd, SnByte bCmdDataLen, SnByte *pbCmdData) \
    {if (m_yDeviceReady)XmtMessage(XMT_TASK_REQUEST, 0, bCmd, bCmdDataLen, pbCmdData);}
    inline void XmtInOrderRequestMessage(SnByte bCmd, SnByte bCmdDataLen, SnByte *pbCmdData) \
    {if (m_yDeviceReady)XmtMessage(XMT_TASK_IN_ORDER_REQUEST, 0, bCmd, bCmdDataLen, pbCmdData);}
    inline void XmtReplyMessage(SnByte bRequestNumber, SnByte bCmd, SnByte bCmdDataLen, SnByte *pbCmdData) \
    {if (m_yDeviceReady)XmtMessage(XMT_TASK_REPLY, bRequestNumber, bCmd, bCmdDataLen, pbCmdData);}

private:
    //
    // SCD Message IDs
    //
	enum {
        bSCD_DETAILED_NAK =                     0x30,   // Detailed NAK
        bSCD_DR_MSG =                           0x31,   // Discovery Request
        bSCD_DR_MSG_RPLY =                      0x31,   // Discovery Request Reply
        bSCD_HB_MSG =                           0x32,   // Heart Beat Status
        bSCD_HB_MSG_RPLY =                      0x32,   // Heart Beat Status Reply
        bSCD_PORT_STATUS_MSG =                  0x33,   // Port Status
        bSCD_PORT_STATUS_MSG_RPLY =             0x33,   // Port Status Reply
        bSCD_GET_PORT_STATUS_MSG =              0x34,   // Get Port Status
        bSCD_SET_DEVICE_INFO_MSG =              0x35,   // Set Device Info
        bSCD_SET_DEV_INFO_MSG_RPLY =            0x35,   // Set Device Info Reply
        bSCD_LAVAGE_TOGGLE_EVENT_MSG  =         0x36,   // Lavage Toggle Event
        bSCD_LAVAGE_TOGGLE_MSG_RPLY =           0x36,   // Lavage Toggle Reply
        bSCD_CMD_MSG =                          0x37,   // SCD Command
        bSCD_CMD_MSG_RPLY =                     0x37,   // SCD Command Reply
        bSCD_CONFIGURATION_GET_MSG_PKT =        0x38,   // Configuration Get Packet
        bSCD_CONFIGURATION_GET_MSG_PKT_RPLY =   0x38,   // Configuration Get Packet Reply
        bSCD_CONFIGURATION_SET_MSG_PKT =        0x39,   // Configuration Set Packet
        bSCD_CONFIGURATION_SET_MSG_PKT_RPLY =   0x39,   // Configuration Set Packet Reply
        bSCD_SN_MSG =                           0x3A,   // Serial Number
        bSCD_SN_MSG_RPLY =                      0x3A    // Serial Number Reply
	};

    //
    // SCD Bridge Methods
    //
    inline const SnByte GetRcvPID() {return 0x35;}
    inline const SnByte GetXmtPID() {return 0x53;}
    SnBool ValidRcvCmd(SnByte bRcvCmd);
    SnBool ValidRcvCmdDataLen(SnByte bRcvCmd, SnByte bRcvCmdDataLen);
    void NotifyConnectionStatus(SnBool bControllerIsConnected);
    void RcvApplicationLayer(RcvMsg *ptRcvMsg);
    void SendDetailedNAK(SnByte bType, SnByte bCmd, SnByte bSeqNum);

    void GetUpdateStatus( int *piNumBytes,SnByte* pucBuf);
    void CreateXmtSetupBlobHeader(void);
    void UpdateXmtSetupBlobData(void);
    void SendUpdateStatus(void);

private:
	SCD_PORT_STATUS	    m_UpdateStatus;
    SnBool              m_yDeviceReady;
	HANDLE			    m_hConnectionStatusEvent;
	HANDLE			    m_hSetEvent;
	HANDLE			    m_hCmdReadyEvent;
	SnBool			    m_yConnected;	// TRUE = Connection established 
	DWORD			    m_dwNewCmd;
	SCD_SET			    m_tSet;
    SnBool              m_yNewUpdateStatus;
    SnBool              m_yNewCapitalDeviceSerialNumber;
    SnBool              m_yNewHandpieceASerialNumber;
    SnBool              m_yNewHandpieceBSerialNumber;
    SnBool              m_yPumpConnection;
    SnBool              m_yPumpRunning;
    SnByte              m_bEvent;
    SnByte			    m_pbFlashMirror[FLASH_SAVE_SIZE];
	NVRAM_DATA		    m_tNvRamMirror;
    SnByte              m_pbRcvSetupBlob[SETUP_BLOB_TOTAL_SIZE];
    SnByte              m_pbXmtSetupBlob[SETUP_BLOB_TOTAL_SIZE];
    SnByte              m_bRcvSetupBlobPacket;
    SnByte              m_bXmtSetupBlobPacket;
    SnBool              m_yRcvSetupBlobPending;
    char                m_pcCapitalDeviceSerialNumber[SERIAL_NUMBER_STORE];
    char                m_pcHandpieceASerialNumber[SERIAL_NUMBER_STORE];
    char                m_pcHandpieceBSerialNumber[SERIAL_NUMBER_STORE];
};

#endif // !defined(AFX_CONNECTEDPUMP_H__2DD91014_8043_4313_BE39_7ABBC5E39B7B__INCLUDED_)
