#include "Controller.h"

//
// SCISR
//
#define SCI_TDRE    0x8000
#define SCI_TIDLE   0x4000
#define SCI_RDRF    0x2000

//
// SCICR
//
#define SCI_TIIE    0x0040
#define SCI_RFIE    0x0020
#define SCI_REIE    0x0010
#define SCI_TE      0x0008
#define SCI_RE      0x0004

//
// RX0/TX0 Selection Sources
//
#define PORTA_FOOT_DE           0x0400
#define PORTA_FOOT_RE           0x0800
#define PORTA_HAND_DE           0x1000
#define PORTA_HAND_RE           0x2000
#define PORTA_HAND_FOOT_MASK    0x3c00

//
// RX1/TX1 Selection Sources
//
#define PORTD_FOOT_DE           0x0004
#define PORTD_FOOT_RE           0x0008
#define PORTD_HAND_DE           0x0010
#define PORTD_HAND_RE           0x0020
#define PORTD_HAND_FOOT_MASK    0x003c

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

void SetSciConfig(Sci *ptSci, SnWord wConfig);
void SetPortASciMux(SnWord wSrc);
void SetPortBSciMux(SnWord wSrc);
void ProcessNextSerialCmd(void);

static volatile SnWord sg_wCmdIndex = 0;
static volatile SnWord *sg_pwCmdRequest = 0;
static volatile SnWord sg_wSerial0Status = 0;
static volatile SnWord sg_wSerial1Status = 0;
static volatile SnBool sg_wResultByteCnt0 = 0;
static volatile SnWord sg_wResult0 = 0;
static volatile SnBool sg_wResultByteCnt1 = 0;
static volatile SnWord sg_wResult1 = 0;
static volatile SnByte *sg_pbFlashPageCmd = 0;
static volatile SnBool sg_wFlashPageCmdByteCnt = 0;
static volatile SnWord sg_wWaitForCmdInMs = 0;

volatile SnWord g_wLoadSerialTimeout = 0;

DIIMotorTblXfr g_tXfrMotorTblA, g_tXfrMotorTblB;
SnBool g_yNewXfrMotorTblA = FALSE;
SnBool g_yNewXfrMotorTblB = FALSE;
static volatile SnByte *sg_pbRcvMotorTbl0 = 0;
static volatile SnByte *sg_pbRcvMotorTbl1 = 0;
static volatile SnWord sg_wRcvMotorTbl0Offs = 0;
static volatile SnWord sg_wRcvMotorTbl1Offs = 0;
static volatile SnByte sg_bRcvMotorTbl0Crc = 0;
static volatile SnByte sg_bRcvMotorTbl1Crc = 0;

static SnByte sg_pbXfrSerialNumberA[SERIAL_NUMBER_STORE];
static SnByte sg_pbXfrSerialNumberB[SERIAL_NUMBER_STORE];
static volatile SnByte *sg_pbRcvSerialNumber0 = 0;
static volatile SnByte *sg_pbRcvSerialNumber1 = 0;
static volatile SnWord sg_wRcvSerialNumber0Offs = 0;
static volatile SnWord sg_wRcvSerialNumber1Offs = 0;
static volatile SnByte sg_bRcvSerialNumber0Crc = 0;
static volatile SnByte sg_bRcvSerialNumber1Crc = 0;

void SetSciConfig(Sci *ptSci, SnWord wConfig)
{
#pragma interrupt called
  volatile SnWord wDummy;

  // Baud Rate
  ptSci->wSCIBR = wConfig & 0x1fff;
  
  // Clear out any previous Receive Errors
  wDummy = ptSci->wSCISR;
  ptSci->wSCISR = 0;

  // Enable TX / RX
  ptSci->wSCICR = (SCI_TE|SCI_RE);
}

void SetPortASciMux(SnWord wSrc)
{
#pragma interrupt called
  ptGPIOA->wDR &= ~PORTA_HAND_FOOT_MASK;
  switch (wSrc) {
  case MUX_RCV_HAND:
    ptGPIOA->wDR |=
      0 |               // A_Footswitch_DE  is Disabled
      PORTA_FOOT_RE |   // nA_Footswitch_RE is Disabled
      0 |               // A_Handpiece_DE   is Disabled
      0;                // nA_Handpiece_RE  is Enabled
    break;
  case MUX_XMT_HAND:
    ptGPIOA->wDR |=
      0 |               // A_Footswitch_DE  is Disabled
      PORTA_FOOT_RE |   // nA_Footswitch_RE is Disabled
      PORTA_HAND_DE |   // A_Handpiece_DE   is Enabled
      PORTA_HAND_RE;    // nA_Handpiece_RE  is Disabled
    break;
  case MUX_RCV_FOOT:
    ptGPIOA->wDR |=
      0 |               // A_Footswitch_DE  is Disabled
      0 |               // nA_Footswitch_RE is Enabled
      0 |               // A_Handpiece_DE   is Disabled
      PORTA_HAND_RE;    // nA_Handpiece_RE  is Disabled
    break;
  case MUX_XMT_FOOT:
    ptGPIOA->wDR |=
      PORTA_FOOT_DE |   // A_Footswitch_DE  is Enabled
      PORTA_FOOT_RE |   // nA_Footswitch_RE is Disabled
      0 |               // A_Handpiece_DE   is Disabled
      PORTA_HAND_RE;    // nA_Handpiece_RE  is Disabled
    break;
  default:                // Disable RS485, (GPIO Operation)
    ptGPIOA->wDR |=
      0 |               // A_Footswitch_DE  is Disabled
      PORTA_FOOT_RE |   // nA_Footswitch_RE is Disabled
      0 |               // A_Handpiece_DE   is Disabled
      PORTA_HAND_RE;    // nA_Handpiece_RE  is Disabled
    break;
  }
}

void SetPortBSciMux(SnWord wSrc)
{
#pragma interrupt called
  ptGPIOD->wDR &= ~PORTD_HAND_FOOT_MASK;
  switch (wSrc) {
  case MUX_RCV_HAND:
    ptGPIOD->wDR |=
      0 |               // B_Footswitch_DE  is Disabled
      PORTD_FOOT_RE |   // nB_Footswitch_RE is Disabled
      0 |               // B_Handpiece_DE   is Disabled
      0;                // nB_Handpiece_RE  is Enabled
    break;
  case MUX_XMT_HAND:
    ptGPIOD->wDR |=
      0 |               // B_Footswitch_DE  is Disabled
      PORTD_FOOT_RE |   // nB_Footswitch_RE is Disabled
      PORTD_HAND_DE |   // B_Handpiece_DE   is Enabled
      PORTD_HAND_RE;    // nB_Handpiece_RE  is Disabled
    break;
  case MUX_RCV_FOOT:
    ptGPIOD->wDR |=
      0 |               // B_Footswitch_DE  is Disabled
      0 |               // nB_Footswitch_RE is Enabled
      0 |               // B_Handpiece_DE   is Disabled
      PORTD_HAND_RE;    // nB_Handpiece_RE  is Disabled
    break;
  case MUX_XMT_FOOT:
    ptGPIOD->wDR |=
      PORTD_FOOT_DE |   // B_Footswitch_DE  is Enabled
      PORTD_FOOT_RE |   // nB_Footswitch_RE is Disabled
      0 |               // B_Handpiece_DE   is Disabled
      PORTD_HAND_RE;    // nB_Handpiece_RE  is Disabled
    break;
  default:                // Disable RS485, (GPIO Operation)
    ptGPIOD->wDR |=
      0 |               // B_Footswitch_DE  is Disabled
      PORTD_FOOT_RE |   // nB_Footswitch_RE is Disabled
      0 |               // B_Handpiece_DE   is Disabled
      PORTD_HAND_RE;    // nB_Handpiece_RE  is Disabled
    break;
  }
}

// Level 2 IRQ
void Sci0_Transmit_Idle_Interrupt(void)
{
#pragma interrupt
  if (sg_wFlashPageCmdByteCnt) {
    volatile SnWord wForceRead;
    wForceRead = ptSci0->wSCISR;
    ptSci0->wSCIDR = *sg_pbFlashPageCmd++;
    if (--sg_wFlashPageCmdByteCnt == 0)
      g_wLoadSerialTimeout = 500;
  } else {
    ptSci0->wSCICR &= ~SCI_TIIE;
    sg_wSerial0Status &= ~CMD_REQUEST_XMT;
    SetPortASciMux((sg_wSerial0Status & MUX_MASK) >> MUX_SHIFT);
  }
}

// Level 2 IRQ
void Sci1_Transmit_Idle_Interrupt(void)
{
#pragma interrupt
  if (sg_wFlashPageCmdByteCnt) {
    volatile SnWord wForceRead;
    wForceRead = ptSci1->wSCISR;
    ptSci1->wSCIDR = *sg_pbFlashPageCmd++;
    if (--sg_wFlashPageCmdByteCnt == 0)
      g_wLoadSerialTimeout = 500;
  } else {
    ptSci1->wSCICR &= ~SCI_TIIE;
    sg_wSerial1Status &= ~CMD_REQUEST_XMT;
    SetPortBSciMux((sg_wSerial1Status & MUX_MASK) >> MUX_SHIFT);
  }
}

// Level 2 IRQ
void Sci0_Receive_Full_Interrupt(void)
{
#pragma interrupt
  volatile SnWord wForceRead;
  wForceRead = ptSci0->wSCISR;
  wForceRead = ptSci0->wSCIDR;
  if (sg_pbRcvMotorTbl0) {
    if (sg_wRcvMotorTbl0Offs < sizeof(DIIMotorTblXfr)) {
      sg_pbRcvMotorTbl0[sg_wRcvMotorTbl0Offs++] = (SnByte)wForceRead;
      sg_bRcvMotorTbl0Crc = g_pbCrcTable[sg_bRcvMotorTbl0Crc ^ (SnByte)wForceRead];
    } else {
      sg_pbRcvMotorTbl0 = 0;
      if (sg_wRcvMotorTbl0Offs == sizeof(DIIMotorTblXfr) &&
       (SnByte)wForceRead == sg_bRcvMotorTbl0Crc) {
        g_yNewXfrMotorTblA = TRUE;
        SerialCmdsComplete(FALSE);
      } else {
        SerialCmdsComplete(TRUE);        
      }
    }
  } else if (sg_pbRcvSerialNumber0) {
    if (sg_wRcvSerialNumber0Offs < SERIAL_NUMBER_XFR) {
      sg_pbRcvSerialNumber0[sg_wRcvSerialNumber0Offs++] = (SnByte)wForceRead;
      sg_bRcvSerialNumber0Crc = g_pbCrcTable[sg_bRcvSerialNumber0Crc ^ (SnByte)wForceRead];
    } else {
      if (sg_wRcvSerialNumber0Offs == SERIAL_NUMBER_XFR &&
       (SnByte)wForceRead == sg_bRcvSerialNumber0Crc) {
        SnWord wOffs;
        for (wOffs = 0; wOffs < SERIAL_NUMBER_XFR; wOffs++) {
          g_tVarArray.pbSerialNumberA[wOffs] = sg_pbRcvSerialNumber0[wOffs];
        }
        sg_pbRcvSerialNumber0 = 0;
        SerialCmdsComplete(FALSE);        
      } else {
        sg_pbRcvSerialNumber0 = 0;
        SerialCmdsComplete(TRUE);        
      }
    }
  } else {
    if (sg_wResultByteCnt0++ == 0)
      sg_wResult0 = wForceRead << 8;
    else {
      sg_wResult0 |= wForceRead;
      g_tVarArray.tSerial.wCmdResult[sg_wCmdIndex-1] = sg_wResult0;
      ProcessNextSerialCmd();
    }
  }
}

// Level 2 IRQ
void Sci1_Receive_Full_Interrupt(void)
{
#pragma interrupt
  volatile SnWord wForceRead;
  wForceRead = ptSci1->wSCISR;
  wForceRead = ptSci1->wSCIDR;
  if (sg_pbRcvMotorTbl1) {
    if (sg_wRcvMotorTbl1Offs < sizeof(DIIMotorTblXfr)) {
      sg_pbRcvMotorTbl1[sg_wRcvMotorTbl1Offs++] = (SnByte)wForceRead;
      sg_bRcvMotorTbl1Crc = g_pbCrcTable[sg_bRcvMotorTbl1Crc ^ (SnByte)wForceRead];
    } else {
      sg_pbRcvMotorTbl1 = 0;
      if (sg_wRcvMotorTbl1Offs == sizeof(DIIMotorTblXfr) &&
       (SnByte)wForceRead == sg_bRcvMotorTbl1Crc) {
        g_yNewXfrMotorTblB = TRUE;
        SerialCmdsComplete(FALSE);
      } else {
        SerialCmdsComplete(TRUE);        
      }
    }
  } else if (sg_pbRcvSerialNumber1) {
    if (sg_wRcvSerialNumber1Offs < SERIAL_NUMBER_XFR) {
      sg_pbRcvSerialNumber1[sg_wRcvSerialNumber1Offs++] = (SnByte)wForceRead;
      sg_bRcvSerialNumber1Crc = g_pbCrcTable[sg_bRcvSerialNumber1Crc ^ (SnByte)wForceRead];
    } else {
      if (sg_wRcvSerialNumber1Offs == SERIAL_NUMBER_XFR &&
       (SnByte)wForceRead == sg_bRcvSerialNumber1Crc) {
        SnWord wOffs;
        for (wOffs = 0; wOffs < SERIAL_NUMBER_XFR; wOffs++) {
          g_tVarArray.pbSerialNumberB[wOffs] = sg_pbRcvSerialNumber1[wOffs];
        }
        sg_pbRcvSerialNumber1 = 0;
        SerialCmdsComplete(FALSE);        
      } else {
        sg_pbRcvSerialNumber1 = 0;
        SerialCmdsComplete(TRUE);        
      }
    }
  } else {
    if (sg_wResultByteCnt1++ == 0)
      sg_wResult1 = wForceRead << 8;
    else {
      sg_wResult1 |= wForceRead;
      g_tVarArray.tSerial.wCmdResult[sg_wCmdIndex-1] = sg_wResult1;
      ProcessNextSerialCmd();
    }
  }
}

// Level 2 IRQ
void Sci0_Receive_Error_Interrupt(void)
{
#pragma interrupt
  volatile SnWord wForceRead;
  SnWord wErrCnt;

  wForceRead = ptSci0->wSCISR;
  ptSci0->wSCISR = 0;
  wErrCnt = g_tVarArray.tSerial.wRcvErrCnt0;
  if (wErrCnt != 0xffff) {
    wErrCnt++;
  }
  g_tVarArray.tSerial.wRcvErrCnt0 = wErrCnt;
  SerialCmdsComplete(TRUE);
}

// Level 2 IRQ
void Sci1_Receive_Error_Interrupt(void)
{
#pragma interrupt
  volatile SnWord wForceRead;
  SnWord wErrCnt;

  wForceRead = ptSci1->wSCISR;
  ptSci1->wSCISR = 0;
  wErrCnt = g_tVarArray.tSerial.wRcvErrCnt1;
  if (wErrCnt != 0xffff) {
    wErrCnt++;
  }
  g_tVarArray.tSerial.wRcvErrCnt1 = wErrCnt;
  SerialCmdsComplete(TRUE);
}

void SerialInit(void)
{
  sg_pbFlashPageCmd = 0;
  sg_wFlashPageCmdByteCnt = 0;
  g_wLoadSerialTimeout = 0;
  sg_wWaitForCmdInMs = 10;
  SetSciConfig(ptSci0, 0xC3);
  SetSciConfig(ptSci1, 0xC3);
}

void StartSerialCmdList(volatile SnWord *pwCmds)
{
#pragma interrupt called
  volatile SnWord wCmdHead;

  sg_wCmdIndex = 0;
  sg_wResult0 = 0;
  sg_wResult1 = 0;
  wCmdHead = *pwCmds++;
  sg_pwCmdRequest = pwCmds;

  if ((*pwCmds & 0xFF) == 0x9c) {         // Flash Page Command 
    sg_pbFlashPageCmd = ((SnByte *)pwCmds + 3);
    sg_wFlashPageCmdByteCnt = ((wCmdHead & 0xFF) * 2) - 3;
    g_tVarArray.tSerial.wNumCmds = 1;
  } else if ((*pwCmds & 0xFF) == 0x4e) {  // Motor Table Command
    sg_wWaitForCmdInMs = ((wCmdHead >> 8) & 0xF) + 1;
    g_tVarArray.tSerial.wNumCmds = 1;
    if (*pwCmds & CMD_REQUEST_PORT) {
      g_yNewXfrMotorTblB = FALSE;
      sg_bRcvMotorTbl1Crc = 0;
      sg_wRcvMotorTbl1Offs = 0;
      sg_pbRcvMotorTbl1 = (SnByte *)&g_tXfrMotorTblB;
    } else {
      g_yNewXfrMotorTblA = FALSE;
      sg_bRcvMotorTbl0Crc = 0;
      sg_wRcvMotorTbl0Offs = 0;
      sg_pbRcvMotorTbl0 = (SnByte *)&g_tXfrMotorTblA;
    }
  } else if ((*pwCmds & 0xFF) == 0xAA) {  // Serial Number Command
    sg_wWaitForCmdInMs = ((wCmdHead >> 8) & 0xF) + 1;
    g_tVarArray.tSerial.wNumCmds = 1;
    if (*pwCmds & CMD_REQUEST_PORT) {
      sg_bRcvSerialNumber1Crc = 0;
      sg_wRcvSerialNumber1Offs = 0;
      sg_pbRcvSerialNumber1 = sg_pbXfrSerialNumberB;
    } else {
      sg_bRcvSerialNumber0Crc = 0;
      sg_wRcvSerialNumber0Offs = 0;
      sg_pbRcvSerialNumber0 = sg_pbXfrSerialNumberA;
    }
  } else {
    // Timeout in ms for Serial Cmd List
    sg_wWaitForCmdInMs = ((wCmdHead >> 8) & 0xF) + 1;
    g_tVarArray.tSerial.wNumCmds = (wCmdHead & 0xF);
  }

  ProcessNextSerialCmd();
}

void ProcessNextSerialCmd(void)
{
#pragma interrupt called
  volatile SnWord wCmd;

  if (sg_wCmdIndex < g_tVarArray.tSerial.wNumCmds) {
    wCmd = sg_pwCmdRequest[sg_wCmdIndex++];
    if (wCmd & CMD_REQUEST_PORT) {
      if ((wCmd & CMD_REQUEST_BAUD) != (sg_wSerial1Status & CMD_REQUEST_BAUD)) {
        SetSciConfig(ptSci1, (wCmd & CMD_REQUEST_BAUD) ? 0x41 : 0xC3);
      }
      if ((wCmd & MUX_MASK) != (sg_wSerial1Status & MUX_MASK)) {
        SetPortBSciMux((wCmd & MUX_MASK) >> MUX_SHIFT);
        ptSci1->wSCICR &= ~(SCI_RFIE|SCI_REIE|SCI_TIIE);
        sg_wResultByteCnt1 = 0;
        ptSci1->wSCICR |= (SCI_RFIE|SCI_REIE);
      }
      sg_wSerial1Status = wCmd & ~CMD_REQUEST_DATA;
      if (wCmd & CMD_REQUEST_ENABLE) {
        volatile SnWord wForceRead;
      
        // Write byte and turn on IDLE IRQ.
        wForceRead = ptSci1->wSCISR;
        ptSci1->wSCIDR = (wCmd & CMD_REQUEST_DATA);
        ptSci1->wSCICR |= SCI_TIIE;
        if (!sg_wFlashPageCmdByteCnt) {
          g_wLoadSerialTimeout = sg_wWaitForCmdInMs;
        }
      } else {
        SerialCmdsComplete(FALSE);
      }
    } else {
      if ((wCmd & CMD_REQUEST_BAUD) != (sg_wSerial0Status & CMD_REQUEST_BAUD)) {
        SetSciConfig(ptSci0, (wCmd & CMD_REQUEST_BAUD) ? 0x41 : 0xC3);
      }
      if ((wCmd & MUX_MASK) != (sg_wSerial0Status & MUX_MASK)) {
        SetPortASciMux((wCmd & MUX_MASK) >> MUX_SHIFT);
        ptSci0->wSCICR &= ~(SCI_RFIE|SCI_REIE|SCI_TIIE);
        sg_wResultByteCnt0 = 0;
        ptSci0->wSCICR |= (SCI_RFIE|SCI_REIE);
      }
      sg_wSerial0Status = wCmd & ~CMD_REQUEST_DATA;
      if (wCmd & CMD_REQUEST_ENABLE) {
        volatile SnWord wForceRead;

        // Write byte and turn on IDLE IRQ.
        wForceRead = ptSci0->wSCISR;
        ptSci0->wSCIDR = (wCmd & CMD_REQUEST_DATA);
        ptSci0->wSCICR |= SCI_TIIE;
        if (!sg_wFlashPageCmdByteCnt) {
          g_wLoadSerialTimeout = sg_wWaitForCmdInMs;
        }
      } else {
        SerialCmdsComplete(FALSE);
      }
    }
  } else {
    SerialCmdsComplete(FALSE);
  }
}
