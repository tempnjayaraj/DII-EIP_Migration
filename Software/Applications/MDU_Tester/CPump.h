#ifndef CPUMP_H
#define CPUMP_H

#include "CSerialPort.h" 

//---------------------------------------------------------------------------------------------

// Remote commands
#define REMOTE_NO_ACTIVITY				0
#define	REMOTE_INCREASE_SET_PRESSURE	1
#define	REMOTE_DECREASE_SET_PRESSURE	2
#define REMOTE_STOP_SET_PRESSURE		3
#define	REMOTE_INCREASE_FLOW_LIMIT		4
#define	REMOTE_DECREASE_FLOW_LIMIT		5
#define REMOTE_STOP_FLOW_LIMIT			6
#define	REMOTE_START_PUMP				7
#define	REMOTE_STOP_PUMP				8
#define	REMOTE_START_LAVAGE				9
#define	REMOTE_STOP_LAVAGE				10
#define REMOTE_TOGGLE_LAVAGE            11

#define PUMP_FMS_MESSAGE_LEN	6
#define PUMP_D25_MESSAGE_LEN	7

class CPump: public CSerialPort
{
public:
    //------------------------------------------------------------------------------
    // enums, typedefs, etc.
	typedef enum {
		PUMP_TYPE_UNKNOWN = 0,
		PUMP_TYPE_FMS,
		PUMP_TYPE_DYONICS25
	} PumpType;
	
    typedef enum {
        PUMP_STATE_MSG_DONE = 0,
        PUMP_STATE_MSG_DATA,
        PUMP_STATE_MSG_TIMEOUT
    } PUMP_STATE_MSG;

    typedef struct PumpStateMsg {
        PUMP_STATE_MSG eMsg;
        SnByte bData;
    } PumpStateMsg;

    typedef enum {
        PUMP_XMT_MSG_DONE = 0,
        PUMP_XMT_MSG_SET,
        PUMP_XMT_MSG_NEW,
        PUMP_XMT_MSG_REPEAT,
    } PUMP_XMT_MSG;

    typedef struct PumpXmtMsg {
        PUMP_XMT_MSG eMsg;
        SnByte pbData[5];
    } PumpXmtMsg;

    //------------------------------------------------------------------------------
    // public member functions
	inline CPump(void):
	    CSerialPort(),
		m_bPumpStatus(PUMP_TYPE_UNKNOWN)
	{
	}

	inline virtual ~CPump(void)
	{
	}

	virtual SnBool Open(const SnByte *pbData);
	virtual SnBool Close(void);

    // Provide 5 data bytes to send to (new) pump
    // Routine fills in rest of packet
    // Returns true if it succeeds in sending bytes
	SnBool XmtPacket(const SnByte* pbData);

    // Return pump type and pump status in an atomic operation
    inline SnBool GetPumpStatus(PumpType *pePumpType)
    {
        SnByte b = m_bPumpStatus;
        if(pePumpType) {
            *pePumpType = (PumpType)(b & 0x7f);
        }
        return b & 0x80;
    }

    //------------------------------------------------------------------------------
    // friend functions
	friend DWORD WINAPI PumpRcvISRThread(LPVOID lpParameter);
	friend DWORD WINAPI PumpStateThread(LPVOID lpParameter);
	friend DWORD WINAPI PumpXmtThread(LPVOID lpParameter);

    //------------------------------------------------------------------------------
    // Public variables

private:
    //------------------------------------------------------------------------------
    // enums, typedefs, etc.
    enum {
        m_bSYNC1 = 0xC5,
        m_bSYNC2 = 0x81,
        m_bSYNC3 = 0x18
    };

    enum {
        iT_CONNECTION_TIMEOUT = 500    // ms
    };

    //------------------------------------------------------------------------------
    // private member functions

	// private copy constructor
	CPump(const CPump &oOld)
	{
	}

    inline void SetPumpStatus(PumpType ePumpType,SnBool yRunning = FALSE)
    {
        m_bPumpStatus = (SnByte)(yRunning?0x80:0) | (SnByte)ePumpType;
    }

    // Low level routine to send packet data with retries
    void XmtPacketData(SnByte* pbData);

    //------------------------------------------------------------------------------
    // private variables
	SnBool m_yRunThreads;
	HANDLE m_hPumpRcvISRThread;
 	DWORD  m_hPumpRcvISRThreadID;
    HANDLE m_hPumpStateThread;
    DWORD  m_hPumpStateThreadID;
    HANDLE m_hPumpXmtThread;
    DWORD  m_hPumpXmtThreadID;
    HANDLE m_hPumpXmtMsgQueue;  // Version of xmt msg queue for writing
    DWORD  m_hPumpXmtMsgQueueID;  // Version of xmt msg queue for writing
    HANDLE m_hPumpStateMsgQueue;    // Version of pump state msg queue for writing
    DWORD  m_hPumpStateMsgQueueID;    // Version of pump state msg queue for writing

    // Last known pump type and running state, based on last received packet
    SnByte m_bPumpStatus;
};

#endif