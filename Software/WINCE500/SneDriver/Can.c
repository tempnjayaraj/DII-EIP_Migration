//
// Includes common to all the MDD files.
//
#define WINCEOEM 1

#include <windows.h>

#include "SnTypes.h"
#include "SnIoctl.h"
#include "SneDriver.h"

// Test the CAN controller interface
SnBool CanTest(void)
{
    volatile SnQByte qExpect;
    SnQByte qMB;

    // Disable CAN and all Interrupt sources
    g_ptCan->qCAN_MR = 0x00000000;
    g_ptCan->qCAN_IDR = 0x1fffffff;

    //
    // CAN baudrate = 1Mbit/s = 1us
    //
    // Delay of the bus driver: 70ns    (SN65HVD230)
    // Delay of the receiver: 35ns      (SN65HVD230)
    // Delay of the bus line: 15ns      (~3 meters)
    //
    // The bit time is fixed at 12 quanta.
    // Tcsc = 1 quanta = 1/12us = 1000/12ns

    // BRP = (Tcsc * MCK) - 1 = (1 * 120)/12 - 1 = 9
    
    // Tprs = 2 * (70ns + 35ns + 15ns) = 2 * 120ns = 240ns = 2.88 Tcsc = ~3 Tcsc
    // PROPAG = Tprs/Tcsc - 1 = 3 - 1 = 2

    // Tphs1 + Tphs2 = bit time - Tcsc - Tprs = (12 - 1 - 3) = 8
    // Tphs1 + Tphs2 = 8 Tcsc
    // Tphs2 = Tphs1 = (8/2) Tcsc = 4 Tcsc
    // PHASE1 = PHASE2 = Tphs1/Tcsc - 1 = 3

    // Tsjw = Min(4 Tcsc,Tphs1) = 4 Tcsc
    // SJW = Tsjw/Tcsc - 1 = 3

    // BRP=9, SJW=3, PROPAG=2, PHASE1=3, PHASE2=3, Enable CAN
    g_ptCan->qCAN_BR = 0x00093233;

    // Enable CAN Controller
    g_ptCan->qCAN_MR = 0x00000001;

    // Disable all Mailboxes
    for (qMB = 0; qMB < 16; qMB++)
        g_ptCan->ptCAN_MB[qMB].qCAN_MB_MMR = 0;

    // Send test message (ID=0x1) to the Motor Controller Board
	// We should expect back a message (ID=0x2) if all went well.

    // Mailbox 0 -- Transmit Extended	
    g_ptCan->ptCAN_MB[0].qCAN_MB_MID = 0x20000001;    // Extended Message ID=0x1
    g_ptCan->ptCAN_MB[0].qCAN_MB_MDL = 0x08040201;    // Walking Ones through 8 bytes of data
    g_ptCan->ptCAN_MB[0].qCAN_MB_MDH = 0x80402010;
    g_ptCan->ptCAN_MB[0].qCAN_MB_MMR = 0x03000000;    // Transmit Mode

	// Mailbox 15 -- Receive Extended	
    g_ptCan->ptCAN_MB[15].qCAN_MB_MID = 0x20000002;   // Extended Message ID=0x2
    g_ptCan->ptCAN_MB[15].qCAN_MB_MMR = 0x01000000;   // Receive Mode
	
    // Transmit Extended Message and 8 data bytes
    g_ptCan->ptCAN_MB[0].qCAN_MB_MCR = 0x00880000;

    // Wait for Xmt and Rcv to take place
	Sleep(30);

	// Check for Mailbox 15 Event set and Mailbox 0 Event cleared
    qExpect = g_ptCan->qCAN_SR & 0xffff;
	if (qExpect != 0x00008001)
		return FALSE;

	// Check that we received a message with ID=0x2
    qExpect = g_ptCan->ptCAN_MB[15].qCAN_MB_MFID;
	if (qExpect != 0x00000002)
		return FALSE;

    // Check that we recevied reverse Walking Ones through 8 bytes
    // of data into Message Object 15
    qExpect = g_ptCan->ptCAN_MB[15].qCAN_MB_MSR & 0x00ff0000;
	if (qExpect != 0x00880000) {
		return FALSE;
	} else {
        // Check for Reverse Walking Ones through 8 bytes of data
        if (g_ptCan->ptCAN_MB[15].qCAN_MB_MDL != 0x10204080 ||
            g_ptCan->ptCAN_MB[15].qCAN_MB_MDH != 0x01020408)
			return FALSE;
	}

    return TRUE;
}
