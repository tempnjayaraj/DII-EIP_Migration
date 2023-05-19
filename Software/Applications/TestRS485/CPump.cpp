#include <stdafx.h>
#include "SnTypes.h"
#include "cserialport.h"
#include "cpump.h"

//-----------------------------------------------------------------------------------------------

SnBool CPump::Close(void)
{
	SnBool yRet = TRUE;
    PumpStateMsg tPumpStateMsg;
	DWORD currentThread = GetCurrentThreadId();

    m_yRunThreads = FALSE;

	// Close the serial port first to free up PumpRcvISRThread
	if(!CSerialPort::Close()) {
		yRet = FALSE;
	}

    if(m_hPumpRcvISRThread != NULL) {
		if(WaitForSingleObject(m_hPumpRcvISRThread,200) == WAIT_TIMEOUT) {
			yRet = FALSE;
            TerminateThread(m_hPumpRcvISRThread,0);
		}
		if(CloseHandle(m_hPumpRcvISRThread) == FALSE) {
			yRet = FALSE;
		}
		m_hPumpRcvISRThread = NULL;
	}

    tPumpStateMsg.eMsg = PUMP_STATE_MSG_DONE;
    if(!WriteMsgQueue(
            m_hPumpStateMsgQueue,(LPVOID)&tPumpStateMsg,sizeof(tPumpStateMsg),INFINITE,0)) {
        yRet = FALSE;
    }

    if(m_hPumpStateThread != NULL) {
		if(WaitForSingleObject(m_hPumpStateThread,200) == WAIT_TIMEOUT) {
			yRet = FALSE;
            TerminateThread(m_hPumpStateThread,0);
		}
		if(CloseHandle(m_hPumpStateThread) == FALSE) {
			yRet = FALSE;
		}
		m_hPumpStateThread = NULL;
    }

	if(m_hPumpXmtThread != NULL && currentThread != (unsigned long)m_hPumpXmtThread) {
		if(WaitForSingleObject(m_hPumpXmtThread,200) == WAIT_TIMEOUT) {
			yRet = FALSE;
            TerminateThread(m_hPumpXmtThread,0);
		}
		if(CloseHandle(m_hPumpXmtThread) == FALSE) {
			yRet = FALSE;
		}
		m_hPumpXmtThread = NULL;
	}
	CloseMsgQueue(m_hPumpXmtMsgQueue);

	return yRet;
}

SnBool CPump::Open(const SnByte *pbData)
{
	SnBool yRet;
    PumpXmtMsg tPumpXmtMsg;
	MSGQUEUEOPTIONS tPumpStateMsgQueueOptions;
	MSGQUEUEOPTIONS tPumpXmtMsgQueueOptions;

	tPumpStateMsgQueueOptions.dwSize = sizeof(tPumpStateMsgQueueOptions);
	tPumpStateMsgQueueOptions.dwFlags = MSGQUEUE_ALLOW_BROKEN;
	tPumpStateMsgQueueOptions.dwMaxMessages = 0;
	tPumpStateMsgQueueOptions.cbMaxMessage = sizeof(PumpStateMsg);
	tPumpStateMsgQueueOptions.bReadAccess = FALSE;

	m_hPumpStateMsgQueue = CreateMsgQueue(
		(LPCWSTR)TEXT("PumpStateMsgQueue"),
		&tPumpStateMsgQueueOptions
		);

	if(!pbData) {
	   return FALSE;
    }

	tPumpXmtMsgQueueOptions.dwSize = sizeof(tPumpXmtMsgQueueOptions);
	tPumpXmtMsgQueueOptions.dwFlags = MSGQUEUE_ALLOW_BROKEN;
	tPumpXmtMsgQueueOptions.dwMaxMessages = 0;
	tPumpXmtMsgQueueOptions.cbMaxMessage = sizeof(tPumpXmtMsg);
	tPumpXmtMsgQueueOptions.bReadAccess = FALSE;

	m_hPumpXmtMsgQueue = CreateMsgQueue(
		(LPCWSTR)TEXT("PumpXmtMsgQueue"),
		&tPumpXmtMsgQueueOptions
		);
    tPumpXmtMsg.eMsg = PUMP_XMT_MSG_SET;
    if(!WriteMsgQueue(m_hPumpXmtMsgQueue,(LPVOID)&tPumpXmtMsg,sizeof(tPumpXmtMsg),INFINITE,0)) {
        return FALSE;
    }
	
	m_hPumpRcvISRThread = NULL;
    m_hPumpStateThread = NULL;
    m_hPumpXmtThread = NULL;

    SetPumpStatus(PUMP_TYPE_UNKNOWN);

    yRet = CSerialPort::Open(_T("COM2:"), CBR_1200);
	if(!yRet) {
		Close();
		return yRet;
	}

	yRet = SetCommMask(m_hSerialPortIn,EV_RXCHAR | EV_ERR| EV_BREAK);
	if(!yRet) {
		Close();
		return yRet;
	}

	m_yRunThreads = TRUE;

	m_hPumpXmtThread = CreateThread(NULL,0,PumpXmtThread,this,0,&m_hPumpXmtThreadID);
	if(m_hPumpXmtThread == NULL) {
		Close();
		return FALSE;
	}
	DEBUGMSG(TRUE, (TEXT("PumpXmtThread: 0x%08X\n"),m_hPumpXmtThreadID));

	m_hPumpStateThread = CreateThread(NULL,0,PumpStateThread,this,0,&m_hPumpStateThreadID);
	if(m_hPumpStateThread == NULL) {
		Close();
		return FALSE;
	}
	DEBUGMSG(TRUE, (TEXT("PumpStateThread: 0x%08X\n"),m_hPumpStateThreadID));

	m_hPumpRcvISRThread = CreateThread(NULL,0,PumpRcvISRThread,this,0,&m_hPumpRcvISRThreadID);
	if(m_hPumpRcvISRThread == NULL) {
		Close();
		return FALSE;
	}
	DEBUGMSG(TRUE, (TEXT("PumpRcvISRThread: 0x%08X\n"),m_hPumpRcvISRThreadID));

	return TRUE;
}

//-----------------------------------------------------------------------------------------------

SnBool CPump::XmtPacket(const SnByte* pbData)
{
    PumpXmtMsg tPumpXmtMsg;

    if(!pbData) {
	   return FALSE;
    }

    tPumpXmtMsg.eMsg = PUMP_XMT_MSG_NEW;
    memcpy(tPumpXmtMsg.pbData,pbData,5);   // dst src count
    return
        (SnBool)WriteMsgQueue(m_hPumpXmtMsgQueue,(LPVOID)&tPumpXmtMsg,sizeof(tPumpXmtMsg),INFINITE,0);
}

//-----------------------------------------------------------------------------------------------

DWORD WINAPI PumpRcvISRThread(LPVOID lpParameter)
{
	CPump* poPump = (CPump*)lpParameter;
    CPump::PumpStateMsg tPumpStateMsg;
        // Buffered data while collecting a packet
    SnByte bRcvIndex;
    SnByte pbRcvBytes[4];   // 3 bytes for packet, plus one for checksum
    DWORD dwLastPacketTime;
    DWORD dwEventMask;
    SnByte bData;

    tPumpStateMsg.eMsg = CPump::PUMP_STATE_MSG_DATA;

	bRcvIndex = 0;
	pbRcvBytes[0] = 0;
	pbRcvBytes[3] = 0;

    while(poPump->m_yRunThreads) {
		if(!WaitCommEvent(poPump->m_hSerialPortIn,&dwEventMask,0) || !dwEventMask) {
			break;
		}
		if(!SetCommMask(poPump->m_hSerialPortIn,EV_RXCHAR | EV_ERR| EV_BREAK)) {
			break;
		}

		while(poPump->Read(&bData)) {
			if(pbRcvBytes[0] == CPump::m_bSYNC1) {
				pbRcvBytes[bRcvIndex++] = bData;
				pbRcvBytes[3] += bData;
				if(bRcvIndex == 3) {
					if(pbRcvBytes[3] == 0) {
                        dwLastPacketTime = GetTickCount();
                        tPumpStateMsg.bData = pbRcvBytes[1];
                        WriteMsgQueue(
                            poPump->m_hPumpStateMsgQueue,
                            (LPVOID)&tPumpStateMsg,
                            sizeof(tPumpStateMsg),
                            INFINITE,
                            0
                            );
						bRcvIndex = 0;
						pbRcvBytes[3] = 0;
					} else {
                        SnByte b;

						// search for sync and discard all bytes prior to it
						for(b = 0; b < 3; b++) {
							if(pbRcvBytes[b] == CPump::m_bSYNC1) {
								break;
							}
						}
						if(b == 3) {
							// no sync found, discard all bytes
							bRcvIndex = 0;
							pbRcvBytes[3] = 0;
						} else {
							// discard all bytes prior to sync
							// decrement checksum for discarded bytes
							// copy down other bytes to beginning
							// adjust byte index
							SnByte bSrc;
							SnByte bDst;

							for(bDst = bSrc = 0; bSrc < 3; bSrc++) {
								if(bSrc >= b) {
									pbRcvBytes[3] -= pbRcvBytes[bDst];
									pbRcvBytes[bDst++] = pbRcvBytes[bSrc];
								}
							}
							bRcvIndex = b;
						}
					}
				}
			} else {
				if(bData == CPump::m_bSYNC1) {
					pbRcvBytes[0] = pbRcvBytes[3] = CPump::m_bSYNC1;
					bRcvIndex = 1;
				} else {
					// clear buffer and discard received byte
					bRcvIndex = 0;
					pbRcvBytes[3] = 0;

                    //
                    // If this bogus byte came after a connection timeout amount of time,
                    // set pump type to unknown
                    //
					DWORD dwDeltaTime = GetTickCount();
 					if(dwDeltaTime >= dwLastPacketTime) {
						// Normal case
						dwDeltaTime = dwDeltaTime - dwLastPacketTime;
					} else {
						// Count has wrapped (once)
						dwDeltaTime = ~((DWORD)0) - dwLastPacketTime + dwDeltaTime + 1;
					}
					if (dwDeltaTime > CPump::iT_CONNECTION_TIMEOUT) {
                        poPump->SetPumpStatus(CPump::PUMP_TYPE_UNKNOWN);
                    }
				}
			}
		}
	}

    tPumpStateMsg.eMsg = CPump::PUMP_STATE_MSG_DONE;
    WriteMsgQueue(
        poPump->m_hPumpStateMsgQueue,
        (LPVOID)&tPumpStateMsg,
        sizeof(tPumpStateMsg),
        INFINITE,
        0
        );

	return 0;
}

//-----------------------------------------------------------------------------------------------

DWORD WINAPI PumpStateThread(LPVOID lpParameter)
{
	CPump* poPump = (CPump*)lpParameter;
	MSGQUEUEOPTIONS tPumpStateMsgQueueOptions;
    HANDLE hPumpStateMsgQueue;
    CPump::PumpStateMsg tPumpStateMsg;
    DWORD dwNumberOfBytesRead;
    SnBool yReadStatus;
    SnBool yRunThread = TRUE;
    CPump::PumpXmtMsg tPumpXmtMsg;
    DWORD dwFlags;
	
	tPumpStateMsgQueueOptions.dwSize = sizeof(tPumpStateMsgQueueOptions);
	tPumpStateMsgQueueOptions.dwFlags = MSGQUEUE_ALLOW_BROKEN;
	tPumpStateMsgQueueOptions.dwMaxMessages = 0;
    tPumpStateMsgQueueOptions.cbMaxMessage = sizeof(CPump::PumpStateMsg);
	tPumpStateMsgQueueOptions.bReadAccess = TRUE;
	
	hPumpStateMsgQueue = CreateMsgQueue(
		(LPCWSTR)TEXT("PumpStateMsgQueue"),
		&tPumpStateMsgQueueOptions
		);
	
    poPump->SetPumpStatus(CPump::PUMP_TYPE_UNKNOWN);
	
    tPumpXmtMsg.eMsg = CPump::PUMP_XMT_MSG_REPEAT;
	
    while(yRunThread) {
		yReadStatus = ReadMsgQueue(
			hPumpStateMsgQueue,
			&tPumpStateMsg,
			sizeof(tPumpStateMsg),
			&dwNumberOfBytesRead,
			CPump::iT_CONNECTION_TIMEOUT,
			&dwFlags
			);
        if(yReadStatus == FALSE) {
			DWORD dwError = GetLastError();
			
			switch(dwError) {
			default:				
				DEBUGMSG(TRUE, (TEXT("ErrorUnknown: 0x%08x\n"),dwError));
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
				// Write some bogus data to see if the "Old" style pump responds
				poPump->Write(0xFF);
				yReadStatus = ReadMsgQueue(
					hPumpStateMsgQueue,
					&tPumpStateMsg,
					sizeof(tPumpStateMsg),
					&dwNumberOfBytesRead,
					CPump::iT_CONNECTION_TIMEOUT,
					&dwFlags
					);
				if(yReadStatus == FALSE) {
                    tPumpStateMsg.eMsg = CPump::PUMP_STATE_MSG_TIMEOUT;
                }
				break;
			}
		}
        switch(tPumpStateMsg.eMsg) {
        default:
            // FallThrough
        case CPump::PUMP_STATE_MSG_DONE:
            yRunThread = FALSE;
            break;
			
        case CPump::PUMP_STATE_MSG_DATA:
            switch(tPumpStateMsg.bData) {
            default:
                // Unknown, ignore packet
                poPump->SetPumpStatus(CPump::PUMP_TYPE_UNKNOWN);
                break;
				
            case 0x80:
                // Old style sync packet -- Old Pump
                poPump->SetPumpStatus(CPump::PUMP_TYPE_FMS);
                break;
				
            case 0xC1:
                // New style sync packet, new Pump, running
                poPump->SetPumpStatus(CPump::PUMP_TYPE_DYONICS25,TRUE);
                break;
				
            case 0x81:
                // New style sync packet, new Pump, not running
                poPump->SetPumpStatus(CPump::PUMP_TYPE_DYONICS25,FALSE);
                break;
            }
            // Ping pump
            WriteMsgQueue(
				poPump->m_hPumpXmtMsgQueue,
				(LPVOID)&tPumpXmtMsg,
				sizeof(tPumpXmtMsg),
				INFINITE,
				0
				);
            break;
			
			case CPump::PUMP_STATE_MSG_TIMEOUT:
				poPump->SetPumpStatus(CPump::PUMP_TYPE_UNKNOWN);
				break;
        }
    }
	
    poPump->SetPumpStatus(CPump::PUMP_TYPE_UNKNOWN);
	CloseMsgQueue(hPumpStateMsgQueue);
	
    tPumpXmtMsg.eMsg = CPump::PUMP_XMT_MSG_DONE;
    return (SnBool)
        !WriteMsgQueue(poPump->m_hPumpXmtMsgQueue,(LPVOID)&tPumpXmtMsg,sizeof(tPumpXmtMsg),INFINITE,0);
}

//-----------------------------------------------------------------------------------------------

void CPump::XmtPacketData(SnByte* pbData)
{
	int iTry = 0;
	int ii = 0;
    PumpType ePumpType;
    SnByte bNewCheckSum = 0;
	SnByte bMsgSize;
	SnByte bPumpMsg[PUMP_D25_MESSAGE_LEN];

    // 0     1       2       3     4     5                   6
    // sync2 speedlo speedhi state blade optional_footswitch cksum
    (void)GetPumpStatus(&ePumpType);

    if(ePumpType == PUMP_TYPE_DYONICS25)
	{
		bMsgSize	= PUMP_D25_MESSAGE_LEN - 1;
		bPumpMsg[0]	= m_bSYNC3;
	}
	else
	{
		bMsgSize	= PUMP_FMS_MESSAGE_LEN - 1;
		bPumpMsg[0]	= m_bSYNC2;
	}
	bNewCheckSum -= bPumpMsg[0];

	for ( ii = 1; ii < bMsgSize; ii++)
	{
		bPumpMsg[ii]  = pbData[ii-1];
		bNewCheckSum -= pbData[ii-1];
	}

	bPumpMsg[ii] = bNewCheckSum;
	bMsgSize += 1;

    if(ePumpType != PUMP_TYPE_UNKNOWN) 
	{
        for(iTry = 0; iTry < 3; iTry++) 
		{
			if (!Write(bPumpMsg,bMsgSize))
			{
                continue; // try again
            }
            break;
        }
    }
}

DWORD WINAPI PumpXmtThread(LPVOID lpParameter)
{
	CPump* poPump = (CPump*)lpParameter;
	MSGQUEUEOPTIONS tPumpXmtMsgQueueOptions;
    HANDLE hXmtMsgQueue;
    CPump::PumpXmtMsg tPumpXmtMsg;
    DWORD dwNumberOfBytesRead;
    DWORD dwFlags;
    SnBool yRunThread = TRUE;
    SnByte pbData[5];

    tPumpXmtMsgQueueOptions.dwSize = sizeof(tPumpXmtMsgQueueOptions);
	tPumpXmtMsgQueueOptions.dwFlags = MSGQUEUE_ALLOW_BROKEN;
	tPumpXmtMsgQueueOptions.dwMaxMessages = 0;
    tPumpXmtMsgQueueOptions.cbMaxMessage = sizeof(CPump::PumpStateMsg);
	tPumpXmtMsgQueueOptions.bReadAccess = TRUE;

	hXmtMsgQueue = CreateMsgQueue(
		(LPCWSTR)TEXT("PumpXmtMsgQueue"),
		&tPumpXmtMsgQueueOptions
		);

    while(yRunThread) {
		if(ReadMsgQueue(
                        hXmtMsgQueue,
                        &tPumpXmtMsg,
                        sizeof(tPumpXmtMsg),
                        &dwNumberOfBytesRead,
                        INFINITE,
                        &dwFlags
                        )) {
            switch(tPumpXmtMsg.eMsg) {
            default:
                // FallThrough
            case CPump::PUMP_XMT_MSG_DONE:
                yRunThread = FALSE;
                break;

            case CPump::PUMP_XMT_MSG_SET:
                memcpy(pbData,tPumpXmtMsg.pbData,5);   // dst src count
                break;

            case CPump::PUMP_XMT_MSG_NEW:
				memcpy(pbData,tPumpXmtMsg.pbData,5);   // dst src count
                poPump->XmtPacketData(pbData);
                break;

            case CPump::PUMP_XMT_MSG_REPEAT:
                poPump->XmtPacketData(pbData);
                break;
            }
        }
    }
    CloseMsgQueue(hXmtMsgQueue);
	return 0;
}

//-----------------------------------------------------------------------------------------------
