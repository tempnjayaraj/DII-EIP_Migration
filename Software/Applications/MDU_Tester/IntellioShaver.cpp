// CIntellioShaver.cpp: implementation of the CIntellioShaver class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Control.h"
#include "IntellioShaver.h"
#include <string.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIntellioShaver::CIntellioShaver()
{

}

CIntellioShaver::~CIntellioShaver()
{
	DeInit();
}

SnBool CIntellioShaver::Init()
{
	m_yConnected = FALSE;
	m_hConnectionStatusEvent = NULL;
	m_hCmdReadyEvent = NULL;

	// Intellio Shaver Ready Status
    m_yDeviceReady = FALSE;

    // Flag when the UpdateStatus has changed
    m_yNewUpdateStatus = FALSE;

    // Flag when a Handpiece Serial Number has changed
    m_yNewHandpieceASerialNumber = FALSE;
    m_yNewHandpieceBSerialNumber = FALSE;

    // Initialize the Serial Number Device IDs
    m_pcCapitalDeviceSerialNumber[11] = 0;
    m_pcHandpieceASerialNumber[11] = 1;
    m_pcHandpieceBSerialNumber[11] = 2;

	// Initialize the Update Status struct
    m_UpdateStatus.bPortAUnits =            SCD_NO_DEVICE;
    m_UpdateStatus.bPortABlade =            SCD_BLADE_OTHER;
    m_UpdateStatus.bPortAMode =             SCD_MODE_OSCILLATE_1;
    m_UpdateStatus.bPortAUpArrow =          SCD_ARROW_DISABLED;
    m_UpdateStatus.bPortADownArrow =        SCD_ARROW_DISABLED;

    m_UpdateStatus.bPortASetSpeed =         0;
    m_UpdateStatus.bPortARunState =         SCD_STATE_STOPPED;

    m_UpdateStatus.bPortBUnits =            SCD_NO_DEVICE;
    m_UpdateStatus.bPortBBlade =            SCD_BLADE_OTHER;
    m_UpdateStatus.bPortBMode =             SCD_MODE_OSCILLATE_1;
    m_UpdateStatus.bPortBUpArrow =          SCD_ARROW_DISABLED;
    m_UpdateStatus.bPortBDownArrow =        SCD_ARROW_DISABLED;

    m_UpdateStatus.bPortBSetSpeed =         0;
    m_UpdateStatus.bPortBRunState =         SCD_STATE_STOPPED;

    m_UpdateStatus.bPortAErrWarn =          0;
    m_UpdateStatus.bPortBErrWarn =          0;

    m_UpdateStatus.bCapitalDevicePopup =    0;
    m_UpdateStatus.bHandpieceOverride =     TRUE;
    m_UpdateStatus.bBladeRecall =           FALSE;
    m_UpdateStatus.bPumpPort =              SCD_PORT_A;
    m_UpdateStatus.bFootswitchPort =        SCD_PORT_A;
    m_UpdateStatus.bSettingsScreen =        FALSE;

    // Connected Pump
    m_yPumpConnection =                     FALSE;
    m_yPumpRunning =                        FALSE;
    m_bEvent =                              0;

	// Setup Blob
    CreateXmtSetupBlobHeader();
    m_bRcvSetupBlobPacket =                 1;
    m_bXmtSetupBlobPacket  =                1;
    m_yRcvSetupBlobPending =                FALSE;

	// Create named event
	m_hConnectionStatusEvent = CreateEvent(NULL, FALSE, FALSE, INTELLIO_SHAVER_CONNECTION_STATUS_EVENT);
	if( m_hConnectionStatusEvent == NULL)
		return FALSE;

	// Create named event
	m_hSetEvent = CreateEvent(NULL, FALSE, FALSE, INTELLIO_SHAVER_SET);
	if( m_hSetEvent == NULL)
	{
		if( m_hConnectionStatusEvent)
		{
			CloseHandle(m_hConnectionStatusEvent);
			m_hConnectionStatusEvent = NULL;
		}
		return FALSE;
	}

    // Create named event
	m_hCmdReadyEvent = CreateEvent(NULL, FALSE, FALSE, INTELLIO_SHAVER_COMMAND_READY);
	if( m_hCmdReadyEvent == NULL)
	{
		if( m_hConnectionStatusEvent)
		{
			CloseHandle(m_hConnectionStatusEvent);
			m_hConnectionStatusEvent = NULL;
		}
		if( m_hSetEvent)
		{
			CloseHandle(m_hSetEvent);
			m_hSetEvent = NULL;
		}
		return FALSE;
	}
	
	// Call the Base Clase Init()
	return CIntellioLink::Init();
}

void CIntellioShaver::DeInit(void)
{
	if( m_hConnectionStatusEvent)
	{
		CloseHandle( m_hConnectionStatusEvent);
		m_hConnectionStatusEvent = NULL;
	}

	if( m_hCmdReadyEvent)
	{
		CloseHandle( m_hCmdReadyEvent);
		m_hCmdReadyEvent = NULL;
	}

    if( m_hCmdReadyEvent)
	{
		CloseHandle( m_hCmdReadyEvent);
		m_hCmdReadyEvent = NULL;
	}

    // Call the Base Class DeInit();
	CIntellioLink::DeInit();
}

void CIntellioShaver::NoHandpiecePresent(DWORD dwPort)
{
    if (dwPort == PORTA && m_UpdateStatus.bPortAUnits != SCD_NO_DEVICE) {
        m_UpdateStatus.bPortAUnits =            SCD_NO_DEVICE;
        m_UpdateStatus.bPortABlade =            SCD_BLADE_OTHER;
        m_UpdateStatus.bPortAMode =             SCD_MODE_FORWARD;
        m_UpdateStatus.bPortAUpArrow =          SCD_ARROW_DISABLED;
        m_UpdateStatus.bPortADownArrow =        SCD_ARROW_DISABLED;
        m_UpdateStatus.bPortASetSpeed =         0;
        m_UpdateStatus.bPortARunState =         SCD_STATE_STOPPED;
        m_yNewUpdateStatus = TRUE;

        if (m_pcHandpieceASerialNumber[0]) {
            memset(m_pcHandpieceASerialNumber, 0, 11);
            m_yNewHandpieceASerialNumber = TRUE;
        }
    } else if (dwPort == PORTB && m_UpdateStatus.bPortBUnits != SCD_NO_DEVICE) {
        m_UpdateStatus.bPortBUnits =            SCD_NO_DEVICE;
        m_UpdateStatus.bPortBBlade =            SCD_BLADE_OTHER;
        m_UpdateStatus.bPortBMode =             SCD_MODE_FORWARD;
        m_UpdateStatus.bPortBUpArrow =          SCD_ARROW_DISABLED;
        m_UpdateStatus.bPortBDownArrow =        SCD_ARROW_DISABLED;
        m_UpdateStatus.bPortBSetSpeed =         0;
        m_UpdateStatus.bPortBRunState =         SCD_STATE_STOPPED;
        m_yNewUpdateStatus = TRUE;

        if (m_pcHandpieceBSerialNumber[0]) {
            memset(m_pcHandpieceBSerialNumber, 0, 11);
            m_yNewHandpieceBSerialNumber = TRUE;
        }
    }
}

void CIntellioShaver::UpdatePortUnitsAndMode(DWORD dwPort, SnByte bPortUnits, SnByte bPortMode)
{
    if (dwPort == PORTA) {
        if (bPortUnits != m_UpdateStatus.bPortAUnits || bPortMode != m_UpdateStatus.bPortAMode) {
            m_UpdateStatus.bPortAUnits = bPortUnits;
            m_UpdateStatus.bPortAMode = bPortMode;
            m_yNewUpdateStatus = TRUE;
        }
    } else if (dwPort == PORTB) {
        if (bPortUnits != m_UpdateStatus.bPortBUnits || bPortMode != m_UpdateStatus.bPortBMode) {
            m_UpdateStatus.bPortBUnits = bPortUnits;
            m_UpdateStatus.bPortBMode = bPortMode;
            m_yNewUpdateStatus = TRUE;
        }
    }
}

void CIntellioShaver::UpdatePortBlade(DWORD dwPort, SnWord wBladeId)
{
    SnByte bPortBlade;

    switch(wBladeId) {
    case BLADE_ID_STRAIGHT:
        bPortBlade = SCD_BLADE_MEDIUM;
        break;
    case BLADE_ID_CURVED:
        bPortBlade = SCD_BLADE_LOW;
        break;
    case BLADE_ID_BURR:
    case BLADE_ID_FAST_BURR:
        bPortBlade = SCD_BLADE_HIGH;
        break;
    default:
        bPortBlade = SCD_BLADE_OTHER;
        break;
    }

    if (dwPort == PORTA) {
        if (bPortBlade != m_UpdateStatus.bPortABlade) {
            m_UpdateStatus.bPortABlade = bPortBlade;
            m_yNewUpdateStatus = TRUE;
        }
    } else if (dwPort == PORTB) {
        if (bPortBlade != m_UpdateStatus.bPortBBlade) {
            m_UpdateStatus.bPortBBlade = bPortBlade;
            m_yNewUpdateStatus = TRUE;
        }
    }
}

void CIntellioShaver::UpdatePortArrows(DWORD dwPort, SnBool yUpArrow, SnBool yDownArrow)
{
    if (dwPort == PORTA) {
        if (yUpArrow != m_UpdateStatus.bPortAUpArrow || yDownArrow != m_UpdateStatus.bPortADownArrow) {
            m_UpdateStatus.bPortAUpArrow = yUpArrow;
            m_UpdateStatus.bPortADownArrow = yDownArrow;
            m_yNewUpdateStatus = TRUE;
        }
    } else if (dwPort == PORTB) {
        if (yUpArrow != m_UpdateStatus.bPortBUpArrow || yDownArrow != m_UpdateStatus.bPortBDownArrow) {
            m_UpdateStatus.bPortBUpArrow = yUpArrow;
            m_UpdateStatus.bPortBDownArrow = yDownArrow;
            m_yNewUpdateStatus = TRUE;
        }
    }
}

void CIntellioShaver::UpdatePortSetSpeed(DWORD dwPort, SnByte bSetSpeed)
{
    if (dwPort == PORTA) {
        if (bSetSpeed != m_UpdateStatus.bPortASetSpeed) {
            m_UpdateStatus.bPortASetSpeed = bSetSpeed;
            m_yNewUpdateStatus = TRUE;
        }
    } else if (dwPort == PORTB) {
        if (bSetSpeed != m_UpdateStatus.bPortBSetSpeed) {
            m_UpdateStatus.bPortBSetSpeed = bSetSpeed;
            m_yNewUpdateStatus = TRUE;
        }
    }
}

void CIntellioShaver::UpdatePortRunState(DWORD dwPort, SnBool yRunning)
{
    if (dwPort == PORTA) {
        if (yRunning != m_UpdateStatus.bPortARunState) {
            m_UpdateStatus.bPortARunState = yRunning;
            m_yNewUpdateStatus = TRUE;
        }
    } else if (dwPort == PORTB) {
        if (yRunning != m_UpdateStatus.bPortBRunState) {
            m_UpdateStatus.bPortBRunState = yRunning;
            m_yNewUpdateStatus = TRUE;
        }
    }
}

void CIntellioShaver::UpdatePortErrWarn(DWORD dwPort, SnByte bErrWarn)
{
    if (dwPort == PORTA) {
        if (bErrWarn != m_UpdateStatus.bPortAErrWarn) {
            m_UpdateStatus.bPortAErrWarn = bErrWarn;
            m_yNewUpdateStatus = TRUE;
        }
    } else if (dwPort == PORTB) {
        if (bErrWarn != m_UpdateStatus.bPortBErrWarn) {
            m_UpdateStatus.bPortBErrWarn = bErrWarn;
            m_yNewUpdateStatus = TRUE;
        }
    }
}

void CIntellioShaver::UpdatePopup(SnByte bPopup)
{
    if (bPopup != m_UpdateStatus.bCapitalDevicePopup) {
        m_UpdateStatus.bCapitalDevicePopup = bPopup;
        m_yNewUpdateStatus = TRUE;
    }
}

void CIntellioShaver::UpdateNvRamImage(NVRAM_DATA* ptNvRamImage)
{
    SnByte bHandpieceOverride;
    SnByte bBladeRecall;
    SnByte bPumpPort;
    SnByte bFootswitchPort;

    memcpy((SnByte *)&m_tNvRamMirror, (SnByte *)ptNvRamImage, sizeof(NVRAM_DATA));

    bHandpieceOverride =     (m_tNvRamMirror.ucFootHandCtl == FOOT_HAND_OVERRIDE_ON) ? 1 : 0;
    bBladeRecall =           (m_tNvRamMirror.ucCustDefaultMode == CUSTOM_MODE) ? 1 : 0;
    bPumpPort =              (m_tNvRamMirror.ucShaverPktCtl == PORTB) ? SCD_PORT_B : SCD_PORT_A;
    bFootswitchPort =        (m_tNvRamMirror.ucPortCtl == PORTB) ? SCD_PORT_B : SCD_PORT_A;

    if (bHandpieceOverride != m_UpdateStatus.bHandpieceOverride || bBladeRecall != m_UpdateStatus.bBladeRecall ||
        bPumpPort != m_UpdateStatus.bPumpPort || bFootswitchPort != m_UpdateStatus.bFootswitchPort) {
        m_UpdateStatus.bHandpieceOverride = bHandpieceOverride;
        m_UpdateStatus.bBladeRecall = bBladeRecall;
        m_UpdateStatus.bPumpPort = bPumpPort;
        m_UpdateStatus.bFootswitchPort = bFootswitchPort;
        m_yNewUpdateStatus = TRUE;
    }
}

void CIntellioShaver::UpdateFlashImage(SnByte* pbFlashImage, SnQByte qFlashSize)
{
    memcpy(m_pbFlashMirror, pbFlashImage, qFlashSize);
}

SnByte Checksum(SnByte *pbBuf, SnWord wSize)
{
    SnByte bChecksum = 0;

    while (wSize-- > 0)
        bChecksum += *pbBuf++;
    
    return ~((char)bChecksum) + 1;
}

void CIntellioShaver::CreateXmtSetupBlobHeader(void)
{
    m_pbXmtSetupBlob[0] = 0x10;                                     // Version of SETUP_BLOB
    m_pbXmtSetupBlob[1] = 23;                                       // Size of SETUP_BLOB Header
    m_pbXmtSetupBlob[2] = 0;
    m_pbXmtSetupBlob[3] = 143;                                      // DYONICS POWER II size of the SETUP_BLOB Data
    m_pbXmtSetupBlob[4] = 1;                                        // DYONICS POWER II Device Type
    m_pbXmtSetupBlob[5] = 1;                                        // DYONICS POWER II Sub Device Type
    strncpy((char *)&m_pbXmtSetupBlob[6], "DYONICS POWER II", 16);  // DYONICS POWER II SETUP_BLOB Name
    m_pbXmtSetupBlob[SETUP_BLOB_HDR_SIZE-1] = Checksum(m_pbXmtSetupBlob, SETUP_BLOB_HDR_SIZE-1);
}

void CIntellioShaver::UpdateXmtSetupBlobData(void)
{
    SCD_SETUP_BLOB_DATA *ptBlobData = (SCD_SETUP_BLOB_DATA *)&m_pbXmtSetupBlob[SETUP_BLOB_HDR_SIZE];

    memcpy(&ptBlobData->m_tNvRamImage, &m_tNvRamMirror, sizeof(NVRAM_DATA));
    memcpy(&ptBlobData->m_ptFlashPortImage, m_pbFlashMirror, 2 * sizeof(SAVE_DEVICE_DATA));
    memset(ptBlobData+1, 0, 23);

    m_pbXmtSetupBlob[SETUP_BLOB_TOTAL_SIZE-1] = Checksum((SnByte *)ptBlobData, sizeof(SCD_SETUP_BLOB_DATA));
}

void CIntellioShaver::SendUpdateStatus(void)
{
    XmtInOrderRequestMessage(bSCD_PORT_STATUS_MSG, sizeof(m_UpdateStatus), (SnByte *)&m_UpdateStatus);
}

void CIntellioShaver::SendUpdateIfChange(void)
{
    if (m_yConnected) {
        if (m_yNewUpdateStatus) {
            m_yNewUpdateStatus = FALSE;
            SendUpdateStatus();
        }
        if (m_yNewHandpieceASerialNumber) {
            m_yNewHandpieceASerialNumber = FALSE;
            SendSerialNumber(1);
        }
        if (m_yNewHandpieceBSerialNumber) {
            m_yNewHandpieceBSerialNumber = FALSE;
            SendSerialNumber(2);
        }
    }
}

void CIntellioShaver::UpdateSettingsScreen(SnBool yInSettingsScreen)
{
    SnByte bSettingsScreen = yInSettingsScreen ? 1 : 0;

    if (bSettingsScreen != m_UpdateStatus.bSettingsScreen) {
        m_UpdateStatus.bSettingsScreen = bSettingsScreen;
        m_yNewUpdateStatus = TRUE;
    }
}

SnBool CIntellioShaver::ValidRcvCmd(SnByte bRcvCmd)
{
    return (bRcvCmd >= 0x30 && bRcvCmd <= 0x3A);
}

SnBool CIntellioShaver::ValidRcvCmdDataLen(SnByte bRcvCmd, SnByte bRcvCmdDataLen)
{
    switch(bRcvCmd) {
    case bSCD_GET_PORT_STATUS_MSG:              // Get Port Status
        return (bRcvCmdDataLen == 0);
    case bSCD_DR_MSG:                           // Discovery Request
    case bSCD_HB_MSG:                           // Heartbeat Status
    case bSCD_PORT_STATUS_MSG_RPLY:             // Port Status Reply
    case bSCD_SET_DEVICE_INFO_MSG:              // Set Device Info
    case bSCD_LAVAGE_TOGGLE_MSG_RPLY:           // Lavage Toggle Event Reply
    case bSCD_CMD_MSG:                          // SCD Cmd
    case bSCD_CONFIGURATION_GET_MSG_PKT:        // Configuration Get Packet
    case bSCD_SN_MSG_RPLY:                      // Serial Number Reply
        return (bRcvCmdDataLen == 1);
    case bSCD_DETAILED_NAK:                     // Detailed NAK
        return (bRcvCmdDataLen == 3);
    case bSCD_CONFIGURATION_SET_MSG_PKT:        // Configuration Set Packet
        return (bRcvCmdDataLen == 24 || bRcvCmdDataLen == 49);
    }

    return FALSE;
}

void CIntellioShaver::SendDetailedNAK(SnByte bType, SnByte bCmd, SnByte bRequestNumber)
{
    SnByte pbNAK[3];

    pbNAK[0] = bType;
    pbNAK[1] = bCmd;
    pbNAK[2] = bRequestNumber;

    XmtReplyMessage(bRequestNumber, bSCD_DETAILED_NAK, sizeof(pbNAK), pbNAK);
}

void CIntellioShaver::NotifyConnectionStatus(SnBool yControllerIsConnected)
{
    if (m_yConnected != yControllerIsConnected) {
        if (yControllerIsConnected && m_yDeviceReady) {
            SendUpdateStatus();
            m_yNewUpdateStatus = FALSE;

            m_yConnected = TRUE;
            SendSerialNumber(0);
            SendSerialNumber(1);
            m_yNewHandpieceASerialNumber = FALSE;
            SendSerialNumber(2);
            m_yNewHandpieceBSerialNumber = FALSE;

            m_bRcvSetupBlobPacket =                 1;
            m_bXmtSetupBlobPacket  =                1;

        } else {
            m_yConnected = FALSE;
        }
        SetEvent(m_hConnectionStatusEvent);
    }
}

void CIntellioShaver::SendIntellioShaverEvent(SnByte bEvent)
{
    if (m_yConnected && bEvent == REMOTE_TOGGLE_LAVAGE && !m_bEvent) {
        XmtRequestMessage(bSCD_LAVAGE_TOGGLE_EVENT_MSG, 0, 0);    
    }
    m_bEvent =  bEvent;
}

void CIntellioShaver::SendSerialNumber(SnByte bDevice)
{
    if (m_yConnected) {
        char *pcSerialNumber = 0;

        switch(bDevice) {
        case 0:
            pcSerialNumber = m_pcCapitalDeviceSerialNumber;
            break;
        case 1:
            pcSerialNumber = m_pcHandpieceASerialNumber;
            break;
        case 2:
            pcSerialNumber = m_pcHandpieceBSerialNumber;
            break;
        }

        if (pcSerialNumber) {
            XmtRequestMessage(bSCD_SN_MSG, 12, (SnByte *)pcSerialNumber);
            pcSerialNumber = 0;
        }
    }
}

void CIntellioShaver::UpdateHandpieceSerialNumber(DWORD dwPort, char *pcSerialNumberStr)
{
    if (dwPort == PORTA && strlen(pcSerialNumberStr) < 11 && strcmp(pcSerialNumberStr, m_pcHandpieceASerialNumber)) {
        strcpy(m_pcHandpieceASerialNumber, pcSerialNumberStr);
        m_yNewHandpieceASerialNumber = TRUE;
    } else if (dwPort == PORTB && strlen(pcSerialNumberStr) < 11 && strcmp(pcSerialNumberStr, m_pcHandpieceBSerialNumber)) {
        strcpy(m_pcHandpieceBSerialNumber, pcSerialNumberStr);
        m_yNewHandpieceBSerialNumber = TRUE;
    }
}

void CIntellioShaver::UpdateCapitalDeviceSerialNumber(char *pcSerialNumberStr)
{
    if (strlen(pcSerialNumberStr) < 11 && strcmp(pcSerialNumberStr, m_pcCapitalDeviceSerialNumber)) {
        strcpy(m_pcCapitalDeviceSerialNumber, pcSerialNumberStr);
    }
}

void CIntellioShaver::RcvApplicationLayer(RcvMsg *ptRcvMsg)
{
    SnByte pbCmdData[iMAX_MSG_SIZE];
    static XmtMsg tXmtMsg = { 0 };
    SnByte bCmdDataLen = 0;
    SnByte bReplyCmd = 0;
    SnByte bSendPing = 0;

    // While disconnected, ignore all Msgs except Discover
    if (!m_yConnected && ptRcvMsg->bCmd != bSCD_DR_MSG) {
        return;
    }

    switch(ptRcvMsg->bCmd) {
    case bSCD_DETAILED_NAK:
    case bSCD_PORT_STATUS_MSG_RPLY:
    case bSCD_LAVAGE_TOGGLE_MSG_RPLY:
    case bSCD_SN_MSG_RPLY:
        tXmtMsg.bTask = (ptRcvMsg->pbCmdData[0] ? XMT_TASK_RCV_NAK : XMT_TASK_RCV_ACK);
        tXmtMsg.bReqNum = (ptRcvMsg->bReqNum);
		WriteMsgQueue(m_hXmtMsgQueue,(void*)&tXmtMsg, sizeof(XmtMsg), INFINITE, 0);
        break;

    case bSCD_DR_MSG:
        // If Major Protocol versions match accept Discovery Request, otherwise NAK the request
        if (SCD_PROTOCOL_MATCH(ptRcvMsg->pbCmdData[0])) {
            bReplyCmd = bSCD_DR_MSG_RPLY;               // Discovery Reply
	        pbCmdData[0] = SCD_PROTOCOL_VERSION;        // Intellio Link Major/Minor Communications Version
            pbCmdData[1] = (m_yDeviceReady ? 1 : 0);    // Device Ready
            pbCmdData[2] = 1;				            // Device type for shaver
            pbCmdData[3] = 1;				            // Device subtype for DYONICS POWER II
            bCmdDataLen = 4;

            bSendPing = XMT_TASK_DR;
        } else {
            SendDetailedNAK(2, ptRcvMsg->bCmd, ptRcvMsg->bReqNum);
        }
        break;

    case bSCD_HB_MSG:
        m_yPumpConnection = (ptRcvMsg->pbCmdData[0] & 1) ? 1 : 0;
        m_yPumpRunning = (ptRcvMsg->pbCmdData[0] & 2) ? 1 : 0;

        bReplyCmd = bSCD_HB_MSG_RPLY;               // Heartbeat Status Reply
	    pbCmdData[0] = (m_yDeviceReady ? 1 : 0);    // Device Ready
        bCmdDataLen = 1;

        bSendPing = XMT_TASK_HB;
        break;

    case bSCD_GET_PORT_STATUS_MSG:
        bReplyCmd = bSCD_PORT_STATUS_MSG;            // Port Status
        memcpy(pbCmdData, (SnByte *)&m_UpdateStatus, sizeof(m_UpdateStatus));
        bCmdDataLen = sizeof(m_UpdateStatus);
        break;

    case bSCD_SET_DEVICE_INFO_MSG:
        {
            SCD_SET tSet;
            SnBool ySendCallback = FALSE;
 
            tSet.bByte = ptRcvMsg->pbCmdData[0];

            //
            // For each field, if it has a value of 1 or 2 and is different than m_UpdateStatus then
            // flag a callback. Otherwise clear the field so it will not be updated on the set callback.
            //
            if (SET_HAS_CHANGED(bHandpieceOverride)) {
                ySendCallback = TRUE;
            } else {
                tSet.b.bHandpieceOverride = 0;
            }
            if (SET_HAS_CHANGED(bFootswitchPort)) {
                ySendCallback = TRUE;
            } else {
                tSet.b.bFootswitchPort = 0;
            }
            if (SET_HAS_CHANGED(bBladeRecall)) {
                ySendCallback = TRUE;
            } else {
                tSet.b.bBladeRecall = 0;
            }
            if (SET_HAS_CHANGED(bPumpPort)) {
                ySendCallback = TRUE;
            } else {
                tSet.b.bPumpPort = 0;
            }

            if (ySendCallback) {
                m_tSet = tSet;
		        SetEvent(m_hSetEvent);              // Signal Set available
            }
        }

        bReplyCmd = bSCD_SET_DEV_INFO_MSG_RPLY;     // Device Info Reply
        pbCmdData[0] = 0;                           // ACK
        bCmdDataLen = 1;
        break;

    case bSCD_CMD_MSG:
        if (ptRcvMsg->pbCmdData[0]) {
            DWORD wdNewCmd = KEY_UNKNOWN;

            switch (ptRcvMsg->pbCmdData[0]) {
            case 1:
                wdNewCmd = KEY_UP_PORTA;
                break;
            case 2:
                wdNewCmd = KEY_DOWN_PORTA;
                break;
            case 3:
                wdNewCmd = KEY_DELTA_MODE_PORTA;
                break;
            case 4:
                wdNewCmd = KEY_UP_PORTB;
                break;
            case 5:
                wdNewCmd = KEY_DOWN_PORTB;
                break;
            case 6:
                wdNewCmd = KEY_DELTA_MODE_PORTB;
                break;
            case 7:
                wdNewCmd = KEY_OK;
                break;
            case 8:
                PostMessage(HWND_BROADCAST, WM_INTELLIO_SHAVER_CMD, (WPARAM)KEY_EXIT_SETTINGS,(LPARAM)0);
                wdNewCmd = KEY_UNKNOWN;             // Notification already sent, no need to send as a Command
               break;
            }

            if (wdNewCmd != KEY_UNKNOWN) {
		        m_dwNewCmd = wdNewCmd;
		        SetEvent( m_hCmdReadyEvent);        // Signal New Command available
            }
        }

        bReplyCmd = bSCD_CMD_MSG_RPLY;              // SCD Cmd Reply
        pbCmdData[0] = 0;                           // ACK
        bCmdDataLen = 1;
        break;

    case bSCD_CONFIGURATION_GET_MSG_PKT:            // Configuration Get Packet
        {
            SnByte bMsgPacket = ptRcvMsg->pbCmdData[0];
            SnByte bXmtSetupBlobDataOffs = 0;

            if (m_bRcvSetupBlobPacket == 1 && !m_yRcvSetupBlobPending) {
                switch(bMsgPacket) {
                case 1:
                    UpdateXmtSetupBlobData();
                    bXmtSetupBlobDataOffs = 0;
                    bCmdDataLen = SETUP_BLOB_HDR_SIZE + 1;
                    break;
                case 2:
                    bXmtSetupBlobDataOffs = SETUP_BLOB_HDR_SIZE;
                    bCmdDataLen = 48 + 1;
                    break;
                case 3:
                    bXmtSetupBlobDataOffs = SETUP_BLOB_HDR_SIZE + 48;
                    bCmdDataLen = 48 + 1;
                    break;
                case 4:
                    bXmtSetupBlobDataOffs = SETUP_BLOB_HDR_SIZE + 48 + 48;
                    bCmdDataLen = 48 + 1;
                    break;
                default:
                    break;
                }
            }

            if (bCmdDataLen) {
                bReplyCmd = bSCD_CONFIGURATION_GET_MSG_PKT_RPLY;
                pbCmdData[0] = bMsgPacket;
                memcpy(&pbCmdData[1], &m_pbXmtSetupBlob[bXmtSetupBlobDataOffs], bCmdDataLen-1);
            }
        }
        break;

    case bSCD_CONFIGURATION_SET_MSG_PKT:            // Configuration Set Packet
        {
            SnByte bMsgPacket = ptRcvMsg->pbCmdData[0];
            SnBool yACK = FALSE;

            if (bMsgPacket == m_bRcvSetupBlobPacket && !m_yRcvSetupBlobPending) {
                switch(bMsgPacket) {
                case 1:
                    memcpy(&m_pbRcvSetupBlob[0], &ptRcvMsg->pbCmdData[1], SETUP_BLOB_HDR_SIZE);
                    if (Checksum(m_pbRcvSetupBlob, SETUP_BLOB_HDR_SIZE) == 0) {
                        m_bRcvSetupBlobPacket = 2;
                        yACK = TRUE;
                    }
                    break;
                case 2:
                    memcpy(&m_pbRcvSetupBlob[SETUP_BLOB_HDR_SIZE], &ptRcvMsg->pbCmdData[1], 48);
                    m_bRcvSetupBlobPacket = 3;
                    yACK = TRUE;
                    break;
                case 3:
                    memcpy(&m_pbRcvSetupBlob[SETUP_BLOB_HDR_SIZE+48], &ptRcvMsg->pbCmdData[1], 48);
                    m_bRcvSetupBlobPacket = 4;
                    yACK = TRUE;
                    break;
                case 4:
                    memcpy(&m_pbRcvSetupBlob[SETUP_BLOB_HDR_SIZE+48+48], &ptRcvMsg->pbCmdData[1], 48);
                    if (Checksum(&m_pbRcvSetupBlob[SETUP_BLOB_HDR_SIZE], SETUP_BLOB_DATA_SIZE+1) == 0) {
                        m_bRcvSetupBlobPacket = 1;
                        m_yRcvSetupBlobPending = TRUE;
                        yACK = TRUE;
                    }
                    break;
                default:
                    break;
                }
            }

            bReplyCmd = bSCD_CONFIGURATION_SET_MSG_PKT_RPLY;
            pbCmdData[0] = bMsgPacket;
            pbCmdData[1] = yACK ? 0 : 1;
            bCmdDataLen = 2;
        }
        break;
    }

    if(bReplyCmd) {
        XmtReplyMessage(ptRcvMsg->bReqNum, bReplyCmd, bCmdDataLen, pbCmdData);
    }
    if (bSendPing && m_yDeviceReady) {
        // Establish a New Connection or Reset the Hearbeat Timeout counter
        tXmtMsg.bTask = bSendPing;
		WriteMsgQueue(m_hXmtMsgQueue,(void*)&tXmtMsg,sizeof(XmtMsg),INFINITE,0);
        bSendPing = 0;
    }
}

SnBool CIntellioShaver::SetParameters(CControl *pControl)
{
    if (m_tSet.b.bHandpieceOverride  || m_tSet.b.bFootswitchPort) {
        SN_FOOT_STATUS tFootStatus;

        pControl->GetCmdState(GET_MC_FOOT_STATUS, &tFootStatus, sizeof(SN_FOOT_STATUS));
        if (m_tSet.b.bFootswitchPort) {
            tFootStatus.usPortControl = m_tSet.b.bFootswitchPort == 2 ? PORTB : PORTA;
        }
        if (m_tSet.b.bHandpieceOverride) {
            tFootStatus.usOverride = m_tSet.b.bHandpieceOverride == 2 ? FOOT_HAND_OVERRIDE_ON : FOOT_HAND_OVERRIDE_OFF;
        }
        pControl->SetCmdState(SET_MC_FOOT_STATUS, &tFootStatus, sizeof(SN_FOOT_STATUS));
     
		PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
                   (WPARAM)MSG_UPDATE_FOOT_STATUS, (LPARAM)0);
    }
    if (m_tSet.b.bBladeRecall) {
        SnWord wCustDefaultMode = m_tSet.b.bBladeRecall == 2 ? CUSTOM_MODE : DEFAULT_MODE;
 
        pControl->SetCmdState(SET_SYSTEM_MODE, &wCustDefaultMode, sizeof(SnWord));

		PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
                   (WPARAM)MSG_UPDATE_SYSTEM_MODE, (LPARAM)0);
    }
    if (m_tSet.b.bPumpPort) {
        SnWord wPumpPort = m_tSet.b.bPumpPort == 2 ? PORTB : PORTA;

        pControl->SetCmdState(SET_SHAVER_PACKET_CTL, &wPumpPort, sizeof(SnWord));

		PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
                   (WPARAM)MSG_UPDATE_REMOTE_PUMP_STATUS, (LPARAM)0);
    }

    SnWord wSaveNvRam = SAVE_NVRAM;
    pControl->SetCmdState(SET_SYSTEM_PARAMETERS, &wSaveNvRam, sizeof(SnWord));

    return TRUE;
}

void CIntellioShaver::ConfigureFromRcvSetupBlob(CControl *pControl)
{
    SCD_SETUP_BLOB_DATA *ptBlobData = (SCD_SETUP_BLOB_DATA *)&m_pbRcvSetupBlob[SETUP_BLOB_HDR_SIZE];
    SnWord wCustDefaultMode = ptBlobData->m_tNvRamImage.ucCustDefaultMode;
    SnWord wLanguage = ptBlobData->m_tNvRamImage.ucLanguage;
    SnWord wPumpPort = ptBlobData->m_tNvRamImage.ucShaverPktCtl;
    SN_FOOT_STATUS tFootStatus;
    SN_PORT_STATUS tPortStatus;
    SnWord wSaveNvRam = SAVE_NVRAM;

    // Foot Pedal Status
    pControl->GetCmdState(GET_MC_FOOT_STATUS, &tFootStatus, sizeof(SN_FOOT_STATUS));
	tFootStatus.usMode = ptBlobData->m_tNvRamImage.ucFootMode;
	tFootStatus.usForward = ptBlobData->m_tNvRamImage.ucFootForward;
	tFootStatus.usOverride = ptBlobData->m_tNvRamImage.ucFootHandCtl;
	tFootStatus.usPortControl = ptBlobData->m_tNvRamImage.ucPortCtl;
    pControl->SetCmdState(SET_MC_FOOT_STATUS, &tFootStatus, sizeof(SN_FOOT_STATUS));

	// Port A Status
    pControl->GetCmdState(GET_MC_PORTA_STATUS, &tPortStatus, sizeof(SN_PORT_STATUS));
    tPortStatus.usOscMode = ptBlobData->m_tNvRamImage.ucOscModePortA;	
	tPortStatus.usRevolutions = 	ptBlobData->m_tNvRamImage.ucOscPortARev;		
	tPortStatus.wDwell = ptBlobData->m_tNvRamImage.ucOscPortASec;
    pControl->SetCmdState(SET_MC_PORTA_STATUS, &tPortStatus, sizeof(SN_PORT_STATUS));

	// Port B Status
    pControl->GetCmdState(GET_MC_PORTB_STATUS, &tPortStatus, sizeof(SN_PORT_STATUS));
	tPortStatus.usOscMode = ptBlobData->m_tNvRamImage.ucOscModePortB;	
	tPortStatus.usRevolutions = 	ptBlobData->m_tNvRamImage.ucOscPortBRev;		
	tPortStatus.wDwell = ptBlobData->m_tNvRamImage.ucOscPortBSec;
    pControl->SetCmdState(SET_MC_PORTB_STATUS, &tPortStatus, sizeof(SN_PORT_STATUS));

    pControl->SetCmdState(SET_SYSTEM_MODE, &wCustDefaultMode, sizeof(SnWord));

    pControl->SetCmdState(SET_SYSTEM_LANGUAGE, &wLanguage, sizeof(SnWord));
    pControl->SetCmdState(SET_SHAVER_PACKET_CTL, &wPumpPort, sizeof(SnWord));

	pControl->SetCmdState(SET_SYSTEM_PARAMETERS, &wSaveNvRam, sizeof(wSaveNvRam));

    pControl->SetCmdState(SET_PORTA_DEVICE_DATA, (SAVE_DEVICE_DATA*)&ptBlobData->m_ptFlashPortImage[0], sizeof(SAVE_DEVICE_DATA));
    pControl->SetCmdState(SET_PORTB_DEVICE_DATA, (SAVE_DEVICE_DATA*)&ptBlobData->m_ptFlashPortImage[1], sizeof(SAVE_DEVICE_DATA));

	PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS, (WPARAM)MSG_UPDATE_ALL_SETTINGS, (LPARAM)0);
    
    m_yRcvSetupBlobPending = FALSE;
}

