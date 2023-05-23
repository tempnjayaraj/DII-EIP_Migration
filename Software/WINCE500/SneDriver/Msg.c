//
// Includes common to all the MDD files.
//
#define WINCEOEM 1

#include <windows.h>

#include "SnTypes.h"
#include "SnIoctl.h"
#include "SneDriver.h"

SnBool g_yWaitingForReady = FALSE;
SnBool g_yFaultOnLastCmd = FALSE;

#define MASTER_IRQ_SLAVE    0x00400000
#define SLAVE_IRQ_MASTER    0x00800000
#define MASTER_XMT_TOGGLE   0x01000000
#define MASTER_RCV_TOGGLE   0x02000000
#define MASTER_OUTPUT       0x04000000
#define MASTER_RESERVED     0x08000000
#define SLAVE_XMT_TOGGLE    0x10000000
#define SLAVE_RCV_TOGGLE    0x20000000
#define SLAVE_OUTPUT        0x40000000
#define SLAVE_READY         0x80000000

#define SLAVE_WAIT_LOOP_CNT 1000
#define SLAVE_TIMEOUT_IN_MS 3

SnQByte g_qSlaveXmtToggle;
SnQByte g_qSlaveRcvToggle;

SnQByte GetTickDelta(SnQByte qTimerStart)
{
	SnQByte	qTimerEnd = GetTickCount();		// Get the "current time"
	if (qTimerEnd >= qTimerStart)
	{
		// Calculate the delta time
		return (qTimerEnd - qTimerStart);
	}
	else
	{
		// Timer has wrapped so calculate the time as the amount of time before the
		// wrap + the time after the counter wrapped.
		return ((MAXDWORD - qTimerStart) + qTimerEnd);
	}
}

void InitMsg(void)
{
    // Configure PB22,PB24-PB27 to be a PIO output
    // Configure PB28-PB31 to be a PIO input
    //     Set PB22 to High
    //     Set PB24-PB27 to Low
    //     Disable Multi Output on PB22,PB24-PB31
    //     Enable Output on PB22,PB24-PB27
    //     Enable PIO control of PB22,PB24-PB31
    //     Enable Pullups on PB22-PB31
    g_ptPioB->qPIO_SODR = 0x00400000;
    g_ptPioB->qPIO_CODR = 0x0f000000;
    g_ptPioB->qPIO_MDDR = 0xff400000;
    g_ptPioB->qPIO_OER = 0x0f400000;
    g_ptPioB->qPIO_PER = 0xff400000;
    g_ptPioB->qPIO_PPUER = 0xffc00000;

    // Configure PE0-PE15 to be a PIO input (switchable to Multi-Output)
    //     Enable writes to the ODSR on PE0-PE15
    //     Writes to the ODSR effect I/O line on PE0-PE15
    //     Set PE0-PE15 to Low
    //     Enable Multi Output on PE0-PE15
    //     Disable Output on PE0-PE15
    //     Enable Pullups on PE0-PE15
    //     Enable PIO control of PE0-PE15
    g_ptPioE->qPIO_OWER = 0x0000ffff;
    g_ptPioE->qPIO_OWSR = 0x0000ffff;
    g_ptPioE->qPIO_ODSR = 0x00000000;
    g_ptPioE->qPIO_MDER = 0x0000ffff;
    g_ptPioE->qPIO_ODR = 0x0000ffff;
    g_ptPioE->qPIO_PPUER = 0x0000ffff;
    g_ptPioE->qPIO_PER = 0x0000ffff;
}

SnBool SendWordToSlave(SnQByte qData)
{
    SnQByte qStartTick = 0;

    // Set PE0-PE15 to output qData 
    g_ptPioE->qPIO_ODSR = qData & 0xffff;

    // Toggle Xmt to let the Slave know the data has been sent
    if (g_ptPioB->qPIO_PDSR & MASTER_XMT_TOGGLE) {
        g_ptPioB->qPIO_CODR = MASTER_XMT_TOGGLE;
    } else {
        g_ptPioB->qPIO_SODR = MASTER_XMT_TOGGLE;
    }

    // Wait a short time for Slave to let us know it has read the data
    while ((g_ptPioB->qPIO_PDSR & SLAVE_RCV_TOGGLE) == g_qSlaveRcvToggle) {
        if (++qStartTick >= SLAVE_WAIT_LOOP_CNT)
            break;
    }
    // If needed, wait for Slave using a ms counter
    if (qStartTick >= SLAVE_WAIT_LOOP_CNT) {
        qStartTick = GetTickCount();
        while ((g_ptPioB->qPIO_PDSR & SLAVE_RCV_TOGGLE) == g_qSlaveRcvToggle) {
            if (GetTickDelta(qStartTick) >= SLAVE_TIMEOUT_IN_MS) {
                NKDbgPrintfW(TEXT("SendWordToSlave() - Timeout\n"));
                return FALSE;
            }
        }
    }
    g_qSlaveRcvToggle ^= SLAVE_RCV_TOGGLE;

    return TRUE;
}

SnBool GetWordFromSlave(SnQByte *pqData)
{
    SnQByte qStartTick = 0;

    // Wait a short time for Slave to let us know it has sent the data
    while ((g_ptPioB->qPIO_PDSR & SLAVE_XMT_TOGGLE) == g_qSlaveXmtToggle) {
        if (++qStartTick >= SLAVE_WAIT_LOOP_CNT)
            break;
    }
    // If needed, wait for Slave using a ms counter
    if (qStartTick >= SLAVE_WAIT_LOOP_CNT) {
        qStartTick = GetTickCount();
        while ((g_ptPioB->qPIO_PDSR & SLAVE_XMT_TOGGLE) == g_qSlaveXmtToggle) {
             if (GetTickDelta(qStartTick) >= SLAVE_TIMEOUT_IN_MS) {
                NKDbgPrintfW(TEXT("GetWordFromSlave() - Timeout\n"));
                return FALSE;
            }
        }
    }
    g_qSlaveXmtToggle ^= SLAVE_XMT_TOGGLE;

	*pqData = g_ptPioE->qPIO_PDSR & 0xffff;

	// Toggle Master Rcv to let the Slave know the data has been received
	if (g_ptPioB->qPIO_PDSR & MASTER_RCV_TOGGLE) {
		g_ptPioB->qPIO_CODR = MASTER_RCV_TOGGLE;
	} else {
		g_ptPioB->qPIO_SODR = MASTER_RCV_TOGGLE;
	}

    return TRUE;
}

SnBool SetMasterDataOuput(SnBool yOutput)
{
    SnQByte qStartTick = 0;

    if (yOutput) {
        // Wait a short time for Slave to set direction to Rcv
        while ((g_ptPioB->qPIO_PDSR & SLAVE_OUTPUT)) {
            if (++qStartTick >= SLAVE_WAIT_LOOP_CNT) {
                break;
            }
        }
        // If needed, wait for Slave using a ms counter
        if (qStartTick >= SLAVE_WAIT_LOOP_CNT) {
            qStartTick = GetTickCount();
            while ((g_ptPioB->qPIO_PDSR & SLAVE_OUTPUT)) {
                if (GetTickDelta(qStartTick)>= SLAVE_TIMEOUT_IN_MS) {
                    NKDbgPrintfW(TEXT("SetMasterDataOuput() - Timeout\n"));
                    return FALSE;
                }
            }
        }
		// Enable Xmt
        g_ptPioB->qPIO_SODR = MASTER_OUTPUT;
        g_ptPioE->qPIO_OER = 0x0000ffff;
    } else {
        // Enable Rcv
        g_ptPioE->qPIO_ODR = 0x0000ffff;
        g_ptPioB->qPIO_CODR = MASTER_OUTPUT;
    }

    return TRUE;
}

SnBool SndCmd(SnWord *pwXmtWords, SnQByte qXmtWords,
                       SnWord *pwRcvWords, SnQByte qRcvWords,
                       SnQByte *pqActualOut)
{
    SnBool yValidTransfer = FALSE;
    SnQByte qAttempt = 0;

    SnByte bTxCrc, bRxCrc;
    SnQByte qRcvCnt;
    SnQByte qXmtCnt;
    SnWord *pwXmtPtr;
    SnQByte qData;
    SnBool ySlowCommand = FALSE;

    // Determine what command to send
    qData = *pwXmtWords;
    switch (qData >> 12) {
    case CMD_RD_VAR_WORD:
        qRcvCnt = 1;
        break;
    case CMD_RD_VAR_WORDS:
        qRcvCnt = (qData & 0x3FF);
        break;
    case CMD_WR_VAR_WORD:
    case CMD_WR_VAR_WORDS:
        break;
    case CMD_WR_FLASH_BLK:
        ySlowCommand = TRUE;
        break;
    case CMD_SERIAL:
        ySlowCommand = TRUE;
        break;
    default:
        return FALSE;
    }

    // Check to make sure there is enough room for the read commands
    if (qRcvWords) {
        if (qRcvCnt > qRcvWords)
            return FALSE;
        qRcvWords = qRcvCnt;
    }

    do {
        bTxCrc = 0;
        bRxCrc = 0;
        qXmtCnt = 0;
        qRcvCnt = 0;
        pwXmtPtr = pwXmtWords;

        // Interrupt the Slave
        g_ptPioB->qPIO_CODR = MASTER_IRQ_SLAVE;

        g_qSlaveXmtToggle = g_ptPioB->qPIO_PDSR & SLAVE_XMT_TOGGLE;
        g_qSlaveRcvToggle = g_ptPioB->qPIO_PDSR & SLAVE_RCV_TOGGLE;

        // Set data direction to Xmt
        if (!SetMasterDataOuput(TRUE))
            goto TransferFailed;

        // Send Command/Data and CRC
        do {
            if (qXmtCnt == qXmtWords) {
                if (!SendWordToSlave(CMD_CRC << 12 | bTxCrc))
                    goto TransferFailed;
            } else {
                qData = *pwXmtPtr++;
                if (!SendWordToSlave(qData))
                    goto TransferFailed;

                bTxCrc = g_pbCrcTable[bTxCrc ^ (qData & 0xFF)];
                bTxCrc = g_pbCrcTable[bTxCrc ^ (qData >> 8)];
            }
        } while (++qXmtCnt <= qXmtWords);

        // Set data direction to Rcv
        if (!SetMasterDataOuput(FALSE))
            goto TransferFailed;
        
        // Rcv Data
        while (qRcvCnt < qRcvWords) {
            if (!GetWordFromSlave(&qData))
                goto TransferFailed;

            pwRcvWords[qRcvCnt++] = (SnWord)qData;
            bRxCrc = g_pbCrcTable[bRxCrc ^ (qData & 0xFF)];
            bRxCrc = g_pbCrcTable[bRxCrc ^ (qData >> 8)];
        }

        // Enable Wait for Ready Flag for slow transfers before that CRC is read
        // in case the Dsp Interrupts before we wait
        g_yWaitingForReady = ySlowCommand;

        // Read the last word and check the CRC
        if (GetWordFromSlave(&qData) && qData == ((CMD_ACK<<12)|(SnQByte)bRxCrc)) {
            // Transfer was good, so clear the IRQ and break out of loop
            g_ptPioB->qPIO_SODR = MASTER_IRQ_SLAVE;
            yValidTransfer = TRUE;
            break;
        }

        NKDbgPrintfW(TEXT("Bad Rx CRC: (0x%04x != 0x%04x)\n"), qData, (CMD_ACK<<12)|(SnQByte)bRxCrc);

        // Disable the Wait for Ready Flag, CRC was bad
	    if (ySlowCommand) {
			// Remove the interrupt from the Slave
			g_ptPioB->qPIO_SODR = MASTER_IRQ_SLAVE;
			qData = WaitForSingleObject(g_hReadyCmd, 1000);
			g_yWaitingForReady = FALSE;
			continue;
        }

TransferFailed:
        // Remove the interrupt from the Slave
        g_ptPioB->qPIO_SODR = MASTER_IRQ_SLAVE;

        // Transfer failed, make sure the Slave has time to exit the IRQ 
        Sleep(1);

        // Give up after MAX_MSG_ATTEMPTS tries
    } while (++qAttempt < MAX_MSG_ATTEMPTS);

    // Wait for ready from slave if long cmd was sent
    if (ySlowCommand) {
        qData = WaitForSingleObject(g_hReadyCmd, 1000);
        g_yWaitingForReady = FALSE;
        if (qData != WAIT_OBJECT_0) {
            NKDbgPrintfW(qData == WAIT_TIMEOUT ? TEXT("Wait Timeout\n") : TEXT("Wait Error\n"));
            yValidTransfer = FALSE;
        }
        if (g_yFaultOnLastCmd == TRUE) {
            NKDbgPrintfW(TEXT("Fault Returned\n"));
            g_yFaultOnLastCmd = FALSE;
            yValidTransfer = FALSE;
        }
    }

    if (pqActualOut && yValidTransfer) {
        *pqActualOut = (qRcvCnt * sizeof(SnWord));
    }

    return yValidTransfer;
}

ULONG MsgInterruptThread(PVOID pContext)
{
	NKDbgPrintfW(TEXT("MsgInterruptThread(0x%x)\n\r"), pContext);

    while (!g_yShutdown) {
		// Block on Dsp interrupt event
        WaitForSingleObject(g_hMsgInterrupt, INFINITE);

        if (g_yWaitingForReady) {
            if (!(g_ptPioB->qPIO_PDSR & SLAVE_READY)) {
	            NKDbgPrintfW(TEXT("MsgInterrupt - Fault\n"));
                g_yFaultOnLastCmd = TRUE;
            }
            g_yWaitingForReady = FALSE;
            SetEvent(g_hReadyCmd);
        } else {
	        NKDbgPrintfW(TEXT("Spurious MsgInterrupt\n"));
        }

        // Notify the system the interrupt is finished
		InterruptDone(g_dwDspSysIntr);
    }

	NKDbgPrintfW(TEXT("MsgInterruptThread(0x%x) - Shutdown!\n\r"), pContext);

    // Should never get here unless the driver is unloading
    return 0;
}
