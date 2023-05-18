#ifndef _CIntellioLink_H
#define _CIntellioLink_H

#include "SnTypes.h"
#include "MsgList.h"

// NAKs
#define SCD_NAK_GENERAL_ERR     1
#define SCD_NAK_VERSION_NUM     2  
#define SCD_NAK_PROTOCOL_ID     3
#define SCD_NAK_CMD_ID          4
#define SCD_NAK_CMD_DATA_LEN    5
#define SCD_NAK_CHECKSUM        6
#define SCD_NAK_FRAMING         7

class CIntellioLink
{
public:
    //
    // Various constants defined by the protocol
    //

    // Protocol characters
	enum {
        bMSG_END_BYTE = 0xFC,
        
        //
		// Implementation constants
		//
        iMAX_XMT_RETRIES = 2,       // Max #retries before giving up xmt
        iHB_TIMEOUT_MS = 1000,      // Wait up to 1s of no HB before Disconnect
        iREPLY_TIMEOUT_MS = 250     // Wait up 250ms for Reply to Request before Retry
	};

    //
    // Xmt thread does one of these tasks
    //
    typedef enum XMT_TASK {
		XMT_TASK_NONE,
        XMT_TASK_TIMEOUT,
        XMT_TASK_IN_ORDER_REQUEST,
        XMT_TASK_REQUEST,
        XMT_TASK_REPLY,
        XMT_TASK_RCV_ACK,
        XMT_TASK_RCV_NAK,
		XMT_TASK_DR,
        XMT_TASK_HB
    } XMT_TASK;

	typedef enum CONNECT_STATE {
		CONNECT_STATE_AUTO,
		CONNECT_STATE_DISCONNECT
	} CONNECT_STATE;


	HANDLE m_hXmtMsgQueue;              // Queue for messages to xmt thread

    inline CIntellioLink(void):
        m_hSerialIn(INVALID_HANDLE_VALUE),
        m_hSerialOut(INVALID_HANDLE_VALUE),
		m_hXmtMsgQueue(NULL),
        m_hXmtThread(NULL),
        m_hRcvThread(NULL)
    {
    }

    inline virtual ~CIntellioLink() {}

    //
    // These functions can be extended by the Device layer
    //
    virtual SnBool Init(void);
    virtual void DeInit(void);

    //
    // These functions are defined by the Device layer
    //

    virtual const SnByte GetRcvPID() = 0;
    virtual const SnByte GetXmtPID() = 0;
    virtual SnBool ValidRcvCmd(SnByte bRcvCmd) = 0;
    virtual SnBool ValidRcvCmdDataLen(SnByte bRcvCmd, SnByte bRcvCmdDataLen) = 0;
    virtual void RcvApplicationLayer(RcvMsg *ptRcvMsg) = 0;
    virtual void SendUpdateStatus(void) = 0;
    virtual void SendDetailedNAK(SnByte bType, SnByte bCmd, SnByte bSeqNum) = 0;
 
	// Take whatever actions are necessary to put device in a safe state when
    // the controller connection status changes.
    virtual void NotifyConnectionStatus(SnBool yControllerIsConnected) = 0;

	//
    // Functions to handle received data at various layers of the protocol
    //
    void RcvTransportLayer(SnByte bSeqNum, RcvMsg *ptRcvMsg);

    //
    // Function to transmit a message
    //
    void XmtMessage(XMT_TASK eTask, SnByte bRequestNumber, SnByte bCmd, SnByte bCmdDataLen, SnByte *pbCmdData);

    //
    // Thread launched to service rcv data
    //
    static DWORD WINAPI RcvThread(LPVOID pParam);

    //
    // Thread launched to monitor activity
    // Calls connection lost on timeout
    //
    static DWORD WINAPI MonitorThread(LPVOID pParam);

    //
    // Thread launched to service xmt data
    //
    static DWORD WINAPI XmtThread(LPVOID pParam);

private:
	CRITICAL_SECTION m_csReqNumAccess;                          // Rcv & Xmt thread access control for GetNextReqNum() 
    MsgList m_hMsgList;

	SnBool m_yIgnoreRcvHandshake;								// Should received ACKs etc be ignored?
    SnBool m_yFirstRcvReqNum;									// Is this the first rcv sequence number ever?
    SnByte m_bLastRcvReqNum;					    	        // Request number for last rcv packet
    SnByte m_bNextXmtReqNum;								    // Request number of the next Xmt Request Message
    SnBool m_yRunThreads;										// Flag that enables threads to keep on running

    HANDLE m_hSerialIn;											// Serial In port for Intellio Link
    HANDLE m_hSerialOut;										// Serial Out port for Intellio Link
    HANDLE m_hXmtThread;										// Handle to xmt thread
    DWORD  m_hXmtThreadID;
    HANDLE m_hRcvThread;										// Handle to rcv thread
    DWORD  m_hRcvThreadID;
	SnBool m_yControllerIsConnected;							// Connection status with controller
	DWORD m_dwCommBreakStartTime;								// When did last comm break start (should be held min 2ms)

    HANDLE m_hRcvThreadKilledEvent;                             // Set upon RcvThread exit
    HANDLE m_hXmtThreadKilledEvent;                             // Set upon XmtThread exit

    SnByte m_bRcvPID;                                           // Protocol ID of Received Commands
    SnByte m_bXmtPID;                                           // Protocol ID of Transmitted Commands

    SnByte GetNextReqNum();
    void ConnectController(void);								// Actions to do when controller is first connected to device
	void DisconnectController(CONNECT_STATE eConnectState);		// Actions to do when controller is first disconnected from device
};

#endif  // ifndef _CCIntellioLink_H