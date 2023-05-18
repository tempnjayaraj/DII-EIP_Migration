#include "stdafx.h"
#include "CommonDefines.h"
#include "IntellioLink.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

SnBool CIntellioLink::Init(void)
{
	//
    // Setup COM port for no HW/SW handshake, 19200 baud, 8 bit, no parity
    // with 1 stop bit
    //
    static DCB tDCB = {
        sizeof(DCB),            // DCBlength
        CBR_115200,             // BaudRate
        TRUE,                   // fBinary - Binary Mode(Always TRUE)
        FALSE,                  // fParity - No Parity check
        FALSE,                  // fOutxCtsFlow - No CTS control
        FALSE,                  // fOutxDsrFlow - No DSR control
        DTR_CONTROL_DISABLE,    // fDtrControl - No DTR control
        FALSE,                  // fDsrSensitivity - No DSR sensitivity
        TRUE,                   // fTXContinueOnXoff - NA
        FALSE,                  // fOutX - No XOFF flow control
        FALSE,                  // fInX - No XON flow control
        FALSE,                  // fErrorChar - No parity error replacement
        FALSE,                  // fNull - Null bytes are not discarded
        RTS_CONTROL_DISABLE,    // fRtsControl - No RTS control
        FALSE,                  // fAbortOnError - Error does not stop read/writes
        0,                      // fDummy2 - NA
        0,                      // wReserved - NA
        0,                      // XonLim - NA
        0,                      // XoffLim - NA
        8,                      // ByteSize
        NOPARITY,               // Parity
        ONESTOPBIT,             // StopBits
        0,                      // XonChar - NA
        1,                      // XoffChar - Must be different from XonChar for SetCommState
        0,                      // ErrorChar - NA
        0,                      // EofChar - NA
        0,                      // EvtChar - NA
        0                       // wReserved1
    };

	// Timeouts for read operations
    static COMMTIMEOUTS tCommTimeouts = {
        0,		 // ReadIntervalTimeout = 0
        0,       // ReadTotalTimeoutMultiplier = 0
        50,      // ReadTotalTimeoutConstant (Wait up to 50ms for read operations)
        0,       // WriteTotalTimeoutMultiplier 
        50       // WriteTotalTimeoutConstant (Wait up to 50ms for write operations)
    };

    m_yControllerIsConnected = FALSE;
    m_dwCommBreakStartTime = 0;

    m_yFirstRcvReqNum = TRUE;
	m_bLastRcvReqNum = 0xFF;
    m_bNextXmtReqNum = 1;
    InitializeCriticalSection (&m_csReqNumAccess); 


    m_hSerialIn = CreateFile(
		TEXT("COM3:"),				// lpFileName
		GENERIC_READ,				// dwDesiredAccess
        FILE_SHARE_WRITE,			// dwShareMode
		NULL,						// lpSecurityAttributes
		OPEN_EXISTING,				// dwCreationDisposition
		0,							// dwFlagsAndAttributes
		NULL						// hTemplateFile
		);

 
	if((m_hSerialIn == INVALID_HANDLE_VALUE) ||
            (SetCommState(m_hSerialIn, &tDCB) == FALSE) ||
            (SetCommTimeouts(m_hSerialIn, &tCommTimeouts) == FALSE)) {
        DeInit();
        return FALSE;
    }

    m_hSerialOut = CreateFile(
		TEXT("COM3:"),				// lpFileName
		GENERIC_WRITE,				// dwDesiredAccess
        0,							// dwShareMode
		NULL,						// lpSecurityAttributes
		OPEN_EXISTING,				// dwCreationDisposition
		0,							// dwFlagsAndAttributes
		NULL						// hTemplateFile
		);

    if((m_hSerialOut == INVALID_HANDLE_VALUE) ||
            (SetCommState(m_hSerialOut, &tDCB) == FALSE) ||
            (SetCommTimeouts(m_hSerialOut, &tCommTimeouts) == FALSE)) {
        DeInit();
        return FALSE;
    }

    m_hXmtMsgQueue = NULL;
    m_hRcvThreadKilledEvent = NULL;
    m_hXmtThreadKilledEvent = NULL;

    m_yRunThreads = TRUE;

	// Create event to indicate that the thread was stopped
	m_hRcvThreadKilledEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if(m_hRcvThreadKilledEvent == NULL)
	{
		DeInit();
		return FALSE;
	}

	// Create event to indicate that the thread was stopped
	m_hXmtThreadKilledEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if(m_hXmtThreadKilledEvent == NULL)
	{
		DeInit();
		return FALSE;
	}

    m_hRcvThread = CreateThread(
                    (LPSECURITY_ATTRIBUTES)NULL,
					0,
					RcvThread,
					this,
					0,
					&m_hRcvThreadID
                    );
	if(m_hRcvThread == NULL) {
        DeInit();
        return FALSE;
    }
	DEBUGMSG(TRUE, (TEXT("Intellio Link RcvThread: 0x%08X\n"),m_hRcvThreadID));

	// Bump up the priority to keep the response times in spec
	SetThreadPriority(m_hRcvThread,THREAD_PRIORITY_ABOVE_NORMAL);

    m_hXmtThread = CreateThread(
                    (LPSECURITY_ATTRIBUTES)NULL,
					0,
					XmtThread,
					this,
					0,
					&m_hXmtThreadID
                    );
	if(m_hXmtThread == NULL) {
        DeInit();
        return FALSE;
    }
	DEBUGMSG(TRUE, (TEXT("Intellio Link XmtThread: 0x%08X\n"),m_hXmtThreadID));

	// Bump up the priority to keep the response times in spec
	SetThreadPriority(m_hXmtThread,THREAD_PRIORITY_ABOVE_NORMAL);

    return TRUE;
}

void CIntellioLink::DeInit(void)
{
	DWORD waitStatus;
	DWORD exitCode;
	DWORD currentThread = GetCurrentThreadId();

    m_yRunThreads = FALSE;

    // Close down the serial port out
    if(m_hSerialOut != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hSerialOut);
        m_hSerialOut = INVALID_HANDLE_VALUE;
    }

    // Wait for XmtThread to terminate, if it does not, try to kill it
    if (m_hXmtThread != NULL && currentThread != (unsigned long)m_hXmtThread) {
 	    waitStatus = WaitForSingleObject( m_hXmtThreadKilledEvent, THREAD_TERMINATION_WAIT );
	    if( waitStatus == WAIT_TIMEOUT) {
		    GetExitCodeThread( m_hXmtThread, &exitCode);
		    TerminateThread( m_hXmtThread, exitCode);
	    }

	    ClearCommBreak(m_hSerialOut);	// Remove RS232 marking state
        CloseHandle(m_hXmtThread);
        m_hXmtThread = NULL;
    }

    if (m_hXmtThreadKilledEvent != NULL) {
        CloseHandle( m_hXmtThreadKilledEvent);
        m_hXmtThreadKilledEvent = NULL;
    }

   // Close down the serial port in, this will cause the RcvThread to wake up
     if(m_hSerialIn != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hSerialIn);
        m_hSerialIn = INVALID_HANDLE_VALUE;
    }

    // Wait for RcvThread to terminate, if it does not, try to kill it
    if(m_hRcvThread != NULL) {
		waitStatus = WaitForSingleObject( m_hRcvThreadKilledEvent, THREAD_TERMINATION_WAIT );
	    if( waitStatus == WAIT_TIMEOUT) {
		    GetExitCodeThread( m_hRcvThread, &exitCode);
		    TerminateThread( m_hRcvThread, exitCode);
	    }
        CloseHandle(m_hRcvThread);
        m_hRcvThread = NULL;
    }

    if (m_hRcvThreadKilledEvent != NULL) {
        CloseHandle( m_hRcvThreadKilledEvent);
        m_hRcvThreadKilledEvent = NULL;
    }

    // Close down the Xmt Msg Queue, this will cause the XmtThread to wake up
    if(m_hXmtMsgQueue != NULL) {
        CloseHandle(m_hXmtMsgQueue);
        m_hXmtMsgQueue = NULL;
    }

    DeleteCriticalSection(&m_csReqNumAccess);
}

void CIntellioLink::ConnectController(void)
{
	if(m_yControllerIsConnected == FALSE) {
        DEBUGMSG(TRUE, (TEXT("ConnectController()\n")));
		m_yControllerIsConnected = TRUE;
		NotifyConnectionStatus(TRUE);
	}
}

void CIntellioLink::DisconnectController(CONNECT_STATE eConnectState)
{
	if((eConnectState == CONNECT_STATE_DISCONNECT) || (m_yControllerIsConnected == TRUE)) {
        DEBUGMSG(TRUE, (TEXT("DisconnectController()\n")));
		NotifyConnectionStatus(FALSE);
	    fflush(m_hSerialOut);
        m_hMsgList.EmptyMsgList();
	}
	m_yControllerIsConnected = FALSE;
}

SnByte CIntellioLink::GetNextReqNum()
{
    SnByte bReqNum = m_bNextXmtReqNum;

    while (!TryEnterCriticalSection(&m_csReqNumAccess)) {
        DEBUGMSG(TRUE, (TEXT("GetNextReqNum() busy\n")));
    }
    if (++m_bNextXmtReqNum >= 128)
        m_bNextXmtReqNum = 1;
	LeaveCriticalSection(&m_csReqNumAccess);

    return bReqNum;
}

DWORD WINAPI CIntellioLink::XmtThread(LPVOID pParam)
{
    CIntellioLink* pIntellioLink = (CIntellioLink*)pParam;
    int iReplyWaitTime = 0, iHeartBeatWaitTime = 0,iWaitTime = 0;
    DWORD dwHeartBeatStartTime = 0;
	DWORD dwNumberOfBytesRead, dwFlags, dwRet;
	MSGQUEUEOPTIONS tMsgQueueOptions;
    SnBool ySendMsg = FALSE, yResendMsg = FALSE;
	XmtMsg tXmtMsg, tHoldMsg; 
	HANDLE hXmtMsgQueue;

	tMsgQueueOptions.dwSize = sizeof(tMsgQueueOptions);
	tMsgQueueOptions.dwFlags = MSGQUEUE_ALLOW_BROKEN;
	tMsgQueueOptions.dwMaxMessages = 0;
	tMsgQueueOptions.cbMaxMessage = sizeof(XmtMsg);
	tMsgQueueOptions.bReadAccess = TRUE;

	hXmtMsgQueue = CreateMsgQueue((LPCWSTR)TEXT("IntellioShaverXmtMsgQueue"), &tMsgQueueOptions);
    PurgeComm( pIntellioLink->m_hSerialOut, PURGE_TXCLEAR );
	
	pIntellioLink->DisconnectController(CONNECT_STATE_DISCONNECT);

    while(pIntellioLink->m_yRunThreads) {
        SnBool yReadStatus;

        // Calculate the shortest Reply Msg timeout using the longest time a Request has been wating for a Reply
        int iMaxTimeWaiting = pIntellioLink->m_hMsgList.GetLongestTimeRequestHasBeenWaiting();
        if (iMaxTimeWaiting) {
            if (iMaxTimeWaiting >= iREPLY_TIMEOUT_MS) {
                iReplyWaitTime = 1;
            } else {
                iReplyWaitTime = iREPLY_TIMEOUT_MS - iMaxTimeWaiting;
            }
        } else {
            // If GetLongestTimeRequestHasBeenWaiting() returns 0, there are no Requests waiting fror a Reply
            iReplyWaitTime = 0;
        }

        // Calculate how long to wait given the shortest Reply timeout and Heart Beat timeout, (1s max)
        if (iHeartBeatWaitTime <= 0) {
            iWaitTime = iHB_TIMEOUT_MS;
        } else {
            if (iReplyWaitTime <= 0) {
                iWaitTime = iHeartBeatWaitTime;
            } else {
                iWaitTime = (iReplyWaitTime < iHeartBeatWaitTime) ? iReplyWaitTime : iHeartBeatWaitTime;
            }
        }
                
		yReadStatus = ReadMsgQueue(hXmtMsgQueue,&tXmtMsg,sizeof(tXmtMsg),&dwNumberOfBytesRead,
            iWaitTime, &dwFlags);

        if (dwNumberOfBytesRead > 0 && dwNumberOfBytesRead != sizeof(tXmtMsg)) {
			DEBUGMSG(TRUE, (TEXT("ReadMsgQueue() read %d out of %d bytes, timeout?\n"), dwNumberOfBytesRead, sizeof(tXmtMsg)));
        }
        if (pIntellioLink->m_yRunThreads == FALSE) {
            break;
        }
        if (yReadStatus == FALSE) {
			DWORD dwError = GetLastError();

			switch(dwError) {
			default:				
				DEBUGMSG(TRUE, (TEXT("ErrorUnknown: 0x%08x\n"), dwError));
				continue;

            case ERROR_INVALID_HANDLE:
				DEBUGMSG(TRUE, (TEXT("ErrorInvalidHandle\n")));
                continue;

			case ERROR_INSUFFICIENT_BUFFER:
				DEBUGMSG(TRUE, (TEXT("ErrorInsufficientBuffer\n")));
				continue;

			case ERROR_PIPE_NOT_CONNECTED:
				DEBUGMSG(TRUE, (TEXT("ErrorPipeNotConnected\n")));
				continue;

			case ERROR_TIMEOUT:
				if(iReplyWaitTime) {
					tXmtMsg.bTask = XMT_TASK_TIMEOUT;	// Timeout waiting for reply
				} else {
					tXmtMsg.bTask = XMT_TASK_NONE;	    // Nothing to do, wait for next message
				}
				break;
			}
		}

        // Disconnect if it has been 1s or more since last HB, otherwise calculate how long till timeout
		if(pIntellioLink->m_yControllerIsConnected) {
            int iTimeSinceLastHeartBeat = pIntellioLink->m_hMsgList.TimeInMsSince(dwHeartBeatStartTime);
			if(iTimeSinceLastHeartBeat >= iHB_TIMEOUT_MS) {
				pIntellioLink->DisconnectController(CONNECT_STATE_AUTO);
            } else {
                iHeartBeatWaitTime = iHB_TIMEOUT_MS - iTimeSinceLastHeartBeat;
            }
        } else {
            iHeartBeatWaitTime = 0;
        }

        switch(tXmtMsg.bTask) {
        default:	// FallThrough
		case XMT_TASK_NONE:
			break;

		case XMT_TASK_DR:
            if (pIntellioLink->m_yControllerIsConnected) {
                pIntellioLink->DisconnectController(CONNECT_STATE_DISCONNECT);
            }
			pIntellioLink->ConnectController();
			dwHeartBeatStartTime = GetTickCount();
            break;

        case XMT_TASK_HB:
            // Reset Heart Beat timer
			dwHeartBeatStartTime = GetTickCount();
            break;

        case XMT_TASK_IN_ORDER_REQUEST:
        case XMT_TASK_REQUEST:
            // Do not send Requests when not connected or when the Request is already waiting for a response
            if (!pIntellioLink->m_yControllerIsConnected) {
                break;
            }
            if (tXmtMsg.bTask == XMT_TASK_IN_ORDER_REQUEST) {
                MSG_LST_RET eRet = pIntellioLink->m_hMsgList.ScheduleForAfterExistingRequest(&tXmtMsg);
                // Ran out of free Msg nodes, this should never happen so disconnect
                if (eRet == MSG_LST_FAIL) {
                    pIntellioLink->DisconnectController(CONNECT_STATE_AUTO);
                    break;
                }
                // There is already a command of this type waiting for a Reply and this message is not set to run after
                // so do not send it now.
                if (eRet == MSG_LST_ADDED) {
                    break;
                }
            }
            // Fallthrough

        case XMT_TASK_REPLY:
            ySendMsg = TRUE;
            break;

        case XMT_TASK_RCV_ACK:
            if(pIntellioLink->m_yControllerIsConnected) {
                // Received a Reply, remove the Request from the Wait List
                tHoldMsg.bReqNum = 0; 
                if (pIntellioLink->m_hMsgList.RemoveReqMsg(tXmtMsg.bReqNum, &tXmtMsg, &tHoldMsg) != MSG_LST_REMOVED) {
                    DEBUGMSG(TRUE, (TEXT("ACK of Request 0x%02x, no messsage on wait list\n"), tXmtMsg.bReqNum));
                }
                // If there was a Msg on hold, send it out now
                if (tHoldMsg.bReqNum) {
                    tXmtMsg = tHoldMsg;
                    ySendMsg = TRUE;
                }
            }
            break;

        case XMT_TASK_TIMEOUT:
            tHoldMsg.bReqNum = 0;
            if (pIntellioLink->m_hMsgList.RemoveExpiredMsg(iREPLY_TIMEOUT_MS, &tXmtMsg, &tHoldMsg) == MSG_LST_REMOVED) {
                yResendMsg = TRUE;
            } else {
                DEBUGMSG(TRUE, (TEXT("Request timeout, no messsage on wait list\n")));
            } 
            break;

        case XMT_TASK_RCV_NAK:
            tHoldMsg.bReqNum = 0; 
            if (pIntellioLink->m_hMsgList.RemoveReqMsg(tXmtMsg.bReqNum, &tXmtMsg, &tHoldMsg) == MSG_LST_REMOVED) {
                yResendMsg = TRUE;
            } else {
                DEBUGMSG(TRUE, (TEXT("NAK of Request 0x%02x, no messsage on wait list\n"), tXmtMsg.bReqNum));
            }
            break;
        }

        // Resend a message if needed
        if (yResendMsg) {
            if (!pIntellioLink->m_yControllerIsConnected) {
                ySendMsg = FALSE;
            }
            if (tXmtMsg.bRetries < iMAX_XMT_RETRIES) {
                tXmtMsg.bRetries++;
                DEBUGMSG(TRUE, (TEXT("Request 0x%02x retry #%d\n"), tXmtMsg.bReqNum, tXmtMsg.bRetries));
                ySendMsg = TRUE;
            } else {
				pIntellioLink->DisconnectController(CONNECT_STATE_AUTO);
                yResendMsg = FALSE;
            }
        }

        if (ySendMsg) {
             // Send Message
			SetCommMask(pIntellioLink->m_hSerialOut,EV_TXEMPTY | EV_ERR);
			if (WriteFile(pIntellioLink->m_hSerialOut, (LPCVOID)tXmtMsg.pbMsg,
                  (DWORD)tXmtMsg.bNumBytes, &dwRet, NULL )) {

                // Wait for the data to be sent and then mark the time
			    WaitCommEvent(pIntellioLink->m_hSerialOut,&dwRet,NULL);

                // Only add Requests of Command ID > 0x32 to wait list
                if ((tXmtMsg.bTask == XMT_TASK_IN_ORDER_REQUEST || tXmtMsg.bTask == XMT_TASK_REQUEST) &&
                    tXmtMsg.pbMsg[1] > 0x32) {
                    if (!yResendMsg) {
                        tXmtMsg.bRetries = 0;
                    }
                    pIntellioLink->m_hMsgList.AddToMsgList(&tXmtMsg, GetTickCount(),
                        tHoldMsg.bReqNum ? &tHoldMsg : NULL);
                }
           } else {
                 // Error during serial write, skip recording Msg
				DEBUGMSG(TRUE, (TEXT("Xmt Msg Send Write Error\n")));
				FlushFileBuffers (pIntellioLink->m_hSerialOut);
            }
            ySendMsg = FALSE;
            yResendMsg = FALSE;
        }
   }

	pIntellioLink->DisconnectController(CONNECT_STATE_DISCONNECT);
	CloseMsgQueue(hXmtMsgQueue);

    SetEvent(pIntellioLink->m_hXmtThreadKilledEvent);
	return (0);
}

void CIntellioLink::XmtMessage(XMT_TASK eTask, SnByte bReqNum, SnByte bCmd, SnByte bCmdDataLen, SnByte *pbCmdData)
{
	XmtMsg tXmtMsg;
	SnByte* pbDstBuf = &tXmtMsg.pbMsg[0];
	SnByte bCheckSum = 0;
	SnByte bSrcByte = 0;
	enum {
		DST_STATE_START,
		DST_STATE_COMMAND,
		DST_STATE_SEQUENCE_NUMBER,
		DST_STATE_DATA_LENGTH,
		DST_STATE_DATA,
		DST_STATE_CHECKSUM,
		DST_STATE_END,
		DST_STATE_DONE} eDstState;

	// NUMBYTES = START COMMAND SEQUENCE DATA_LEN <DATA> CHECKSUM END

	eDstState = DST_STATE_START;

	while(eDstState != DST_STATE_DONE) {
		switch(eDstState) {
		default:
			// DST_STATE_DONE never reached in switch
			continue;

		case DST_STATE_START:
			bSrcByte = GetXmtPID();
			bCheckSum = bSrcByte;
			eDstState = DST_STATE_COMMAND;
			break;

		case DST_STATE_COMMAND:
			bSrcByte = bCmd;
			bCheckSum += bSrcByte;
			eDstState = DST_STATE_SEQUENCE_NUMBER;
			break;

		case DST_STATE_SEQUENCE_NUMBER:
            if (eTask == XMT_TASK_IN_ORDER_REQUEST || eTask == XMT_TASK_REQUEST) {
			    bSrcByte = (0x80 | GetNextReqNum());
            } else {
                bSrcByte = (bReqNum & 0x7f);
            }
			bCheckSum += bSrcByte;
			eDstState = DST_STATE_DATA_LENGTH;
			break;

		case DST_STATE_DATA_LENGTH:
			bSrcByte = bCmdDataLen;
			bCheckSum += bSrcByte;
            if (bCmdDataLen > 0) {
                eDstState = DST_STATE_DATA;
            } else {
                eDstState = DST_STATE_CHECKSUM;
            }
			break;

		case DST_STATE_DATA:
			bSrcByte = *pbCmdData++;
			bCheckSum += bSrcByte;
			if(--bCmdDataLen == 0) {
				eDstState = DST_STATE_CHECKSUM;
			}
			break;

		case DST_STATE_CHECKSUM:
			bSrcByte = ~((char)bCheckSum) + 1;
			eDstState = DST_STATE_END;
			break;

		case DST_STATE_END:
			bSrcByte = bMSG_END_BYTE;
			eDstState = DST_STATE_DONE;
			break;
		}
		*pbDstBuf++ = bSrcByte;
    }

	tXmtMsg.bTask = eTask;
    tXmtMsg.bReqNum = tXmtMsg.pbMsg[2] & 0x7f;
	tXmtMsg.bNumBytes = pbDstBuf - &tXmtMsg.pbMsg[0];
	WriteMsgQueue(m_hXmtMsgQueue,(LPVOID)&tXmtMsg,sizeof(tXmtMsg),INFINITE,0);
}

void CIntellioLink::RcvTransportLayer(SnByte bReqNum, RcvMsg *ptRcvMsg)
{
    // Only check Requests, not Replys
    if (bReqNum & 0x80) {
        bReqNum &= 0x7f;
        if(m_yFirstRcvReqNum) {
            m_bLastRcvReqNum = bReqNum;
            m_yFirstRcvReqNum = FALSE;
        } else {
            SnByte bRcvReqNum = m_bLastRcvReqNum;
            if (++bRcvReqNum >= 128)
                bRcvReqNum = 1;
            if(bRcvReqNum != bReqNum) {
				DEBUGMSG(TRUE, (TEXT("Rcv Request Number: Expect=%d Got=%d\n"), bRcvReqNum, bReqNum));
            }
            m_bLastRcvReqNum = bReqNum;
        }
    }
    RcvApplicationLayer(ptRcvMsg);
}

DWORD WINAPI CIntellioLink::RcvThread(LPVOID pParam)
{
    CIntellioLink* pIntellioLink = (CIntellioLink*)pParam;
	MSGQUEUEOPTIONS tMsgQueueOptions;
    SnByte bRcvByte;
	DWORD dwRet = 0;
    RcvMsg tRcvMsg;
    SnByte bCheckSum;
	SnByte bCmdDataLen;
    SnByte bNAKType = 0;

	enum {
		RCV_MSG_PROCESS_ID,
		RCV_MSG_COMMAND,
		RCV_MSG_SEQUENCE_NUMBER,
		RCV_MSG_LENGTH,
		RCV_MSG_DATA,
		RCV_MSG_CHECKSUM,
		RCV_MSG_END
	} eMsgFrameState = RCV_MSG_PROCESS_ID;

	tMsgQueueOptions.dwSize = sizeof(tMsgQueueOptions);
	tMsgQueueOptions.dwFlags = MSGQUEUE_ALLOW_BROKEN;
	tMsgQueueOptions.dwMaxMessages = 0;
	tMsgQueueOptions.cbMaxMessage = sizeof(XmtMsg);
	tMsgQueueOptions.bReadAccess = FALSE;

	pIntellioLink->m_hXmtMsgQueue = CreateMsgQueue((LPCWSTR)TEXT("IntellioShaverXmtMsgQueue"),&tMsgQueueOptions);
    PurgeComm( pIntellioLink->m_hSerialIn, PURGE_RXCLEAR );
    
    while(pIntellioLink->m_yRunThreads) {
		SetCommMask(pIntellioLink->m_hSerialIn, EV_RXCHAR | EV_ERR);
		WaitCommEvent(pIntellioLink->m_hSerialIn, &dwRet, NULL);

		while(pIntellioLink->m_yRunThreads) {
            SnBool yRead = ReadFile(pIntellioLink->m_hSerialIn, &bRcvByte, 1, &dwRet, NULL);

			if(pIntellioLink->m_yRunThreads == FALSE || yRead == FALSE || dwRet !=1) {
				break;
			}

            switch(eMsgFrameState){
			case RCV_MSG_PROCESS_ID:
                if (bRcvByte == pIntellioLink->GetRcvPID()) {
                    tRcvMsg.bCmd = 0;
                    tRcvMsg.bReqNum = 0;
                    bCheckSum = bRcvByte;
                    eMsgFrameState = RCV_MSG_COMMAND;
                }
                break;

            case RCV_MSG_COMMAND:
                tRcvMsg.bCmd = bRcvByte;
                if (pIntellioLink->ValidRcvCmd(bRcvByte)) {
                    bCheckSum += bRcvByte;
                    eMsgFrameState = RCV_MSG_SEQUENCE_NUMBER;
                } else {
                    bNAKType = SCD_NAK_CMD_ID;          // Send NAK - Command ID not supported
                }
                break;

			case RCV_MSG_SEQUENCE_NUMBER:
                tRcvMsg.bReqNum = bRcvByte;
                bCheckSum += bRcvByte;
                eMsgFrameState = RCV_MSG_LENGTH;
				break;

			case RCV_MSG_LENGTH:
                if (pIntellioLink->ValidRcvCmdDataLen(tRcvMsg.bCmd, bRcvByte)) {
                    bCmdDataLen = bRcvByte;
                    tRcvMsg.bCmdDataLen = 0;
                    bCheckSum += bRcvByte;
                    eMsgFrameState = (bCmdDataLen > 0 ? RCV_MSG_DATA : RCV_MSG_CHECKSUM);
                } else {
                    bNAKType = SCD_NAK_CMD_DATA_LEN;    // Send NAK - Command data length not supported
                }
				break;

			case RCV_MSG_DATA:
				if(bCmdDataLen > 0) {
					bCheckSum += bRcvByte;
                    tRcvMsg.pbCmdData[tRcvMsg.bCmdDataLen++] = bRcvByte;
					bCmdDataLen--;
				}
				if(bCmdDataLen == 0) {
					eMsgFrameState = RCV_MSG_CHECKSUM;
				}
				break;

			case RCV_MSG_CHECKSUM:
				bCheckSum += bRcvByte;
                if (bCheckSum == 0) {
				    eMsgFrameState = RCV_MSG_END;
                } else {
                    bNAKType = SCD_NAK_CHECKSUM;        // Send NAK - Checksum error
                }
				break;

			case RCV_MSG_END:
                if (bRcvByte == bMSG_END_BYTE) {
                    pIntellioLink->RcvTransportLayer(tRcvMsg.bReqNum, &tRcvMsg);
                    eMsgFrameState = RCV_MSG_PROCESS_ID;
                } else {
                    bNAKType = SCD_NAK_FRAMING;         // Send NAK - Framing error
                }
				break;

			default:
				break;
			}
            if (bNAKType) {
                pIntellioLink->SendDetailedNAK(bNAKType, tRcvMsg.bCmd, tRcvMsg.bReqNum);
                eMsgFrameState = RCV_MSG_PROCESS_ID;
                bNAKType = 0;
            }
        }
    }

    if (pIntellioLink->m_hXmtMsgQueue) {
        CloseMsgQueue(pIntellioLink->m_hXmtMsgQueue);
        pIntellioLink->m_hXmtMsgQueue = NULL;
    }

    SetEvent(pIntellioLink->m_hRcvThreadKilledEvent);
	return(0);
}
