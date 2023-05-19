#include "Controller.h"

// PIOA
#define MASTER_XMT_TOGGLE   0x0001
#define MASTER_RCV_TOGGLE   0x0002
#define MASTER_OUTPUT       0x0004
#define MASTER_RESERVED     0x0008
#define SLAVE_XMT_TOGGLE    0x0010
#define SLAVE_RCV_TOGGLE    0x0020
#define SLAVE_OUTPUT        0x0040
#define SLAVE_READY         0x0080
#define MASTER_IRQ_SLAVE    0x0100
#define SLAVE_IRQ_MASTER    0x0200

/* Msg 4 bit command set */
#define CMD_RD_VAR_WORD     0x1     // command to read one word from variable array
#define CMD_WR_VAR_WORD     0x2     // command to write one word to variable array
#define CMD_RD_VAR_WORDS    0x3     // command to read a string from variable array
#define CMD_WR_VAR_WORDS    0x4     // command to write a string to variable array
#define CMD_WR_FLASH_BLK    0x5     // command to write 512 word block to flash ROM
#define CMD_SERIAL          0x6     // command to write a string of serial cmds through the SCI

/* Msg 4 bit special character prefixes to 8 bit CRC */
#define CRC                 0xC     // control identifier for 8 bit CRC
#define ACK                 0xD     // acknowledge control word
#define NACK                0xE     // not acknowledge control word

/* Global variables */
SnByte g_pbCrcTable[256];           // 8 bit CRC table
SnWord g_wHeartbeat = 30000;        // System heartbeat timer (arm for 15 seconds on startup)

typedef enum {
  MSG_NO_WAIT,
  MSG_WAIT_FOR_FLASH,
  MSG_WAIT_FOR_SERIAL
} MsgWait;

/* Local variables */
static volatile SnWord     sg_wRxDataIn = 0;            // Index to data words into RxBuffer
static volatile SnWord*    sg_pwDataArray = 0;          // pointer to status and control data arrays
static volatile SnWord     sg_wMasterXmtToggle;
static volatile SnWord     sg_wMasterRcvToggle;
static volatile MsgWait    sg_eMsgWait = MSG_NO_WAIT;

SnBool SendWordToMaster(SnWord wData);
SnBool GetWordFromMaster(SnWord *pwData);
SnBool SetSlaveDataOutput(SnBool yOutput);

/*
 * Size of the buffer used to store Master commands.
 */
#define RX_BUF_SIZE 514
static volatile SnWord sg_pwRxBuffer[RX_BUF_SIZE];      // Incoming data receive buffer

inline SnWord CalculateCrc(SnWord wCrc, SnWord wData)
{
  wCrc = g_pbCrcTable[wCrc ^ (wData & 0x00FF)];
  wCrc = g_pbCrcTable[wCrc ^ (wData >> 8)];
  
  return wCrc;
}

inline SnBool SendWordToMaster(SnWord wData)
{
  ptGPIOF->wDR = wData;

  // Toggle Xmt to let the Master know the data has been sent
  if (ptGPIOA->wDR & SLAVE_XMT_TOGGLE) {
    ptGPIOA->wDR &= ~SLAVE_XMT_TOGGLE;
  } else {
    ptGPIOA->wDR |= SLAVE_XMT_TOGGLE;
  }

  // Wait for Master to let us know it has read the data
  while ((ptGPIOA->wRAWDATA & MASTER_RCV_TOGGLE) == sg_wMasterRcvToggle) {
    if (ptGPIOA->wRAWDATA & MASTER_IRQ_SLAVE)
      return FALSE;
  }
  sg_wMasterRcvToggle ^= MASTER_RCV_TOGGLE;

  return TRUE;
}

inline SnBool GetWordFromMaster(SnWord *pwData)
{
  // Wait for Master to let us know it has sent the data
  while ((ptGPIOA->wRAWDATA & MASTER_XMT_TOGGLE) == sg_wMasterXmtToggle) {
    if (ptGPIOA->wRAWDATA & MASTER_IRQ_SLAVE)
      return FALSE;
  }
  sg_wMasterXmtToggle ^= MASTER_XMT_TOGGLE;

  *pwData = ptGPIOF->wRAWDATA;

  // Toggle Slave Rcv to let the Master know the data has been received
  if (ptGPIOA->wDR & SLAVE_RCV_TOGGLE) {
    ptGPIOA->wDR &= ~SLAVE_RCV_TOGGLE;
  } else {
    ptGPIOA->wDR |= SLAVE_RCV_TOGGLE;
  }

  return TRUE;
}

inline SnBool SetSlaveDataOutput(SnBool yOutput)
{
  if (yOutput) {
    // Wait for Master to set direction to Rcv
    while ((ptGPIOA->wRAWDATA & MASTER_OUTPUT)) {
      if (ptGPIOA->wRAWDATA & MASTER_IRQ_SLAVE)
        return FALSE;
    }
    // Enable Xmt
    ptGPIOA->wDR |= SLAVE_OUTPUT;
    ptGPIOF->wDDR = 0xffff;
  } else {
    // Enable Rcv
    ptGPIOF->wDDR = 0x0000;
    ptGPIOA->wDR &= ~SLAVE_OUTPUT;
  }

  return TRUE;
}

/*******************************************************
 PIOA8 Message Layer Level 0 Interrupt inline coded
 *******************************************************/
void Msg_Interrupt(void)
{
#pragma interrupt
  SnWord wRcvWords;
  SnWord wXmtWords;
  SnBool yAck;
  SnWord wCrc;
  SnWord wTmp;

  // Reset IRQ
  ptGPIOA->wIESR = MASTER_IRQ_SLAVE;

  // Clear any previous READY
  ptGPIOA->wDR &= ~SLAVE_READY;

  // Capture a snapshot of the Master Toggle Bits
  sg_wMasterXmtToggle = ptGPIOA->wRAWDATA & MASTER_XMT_TOGGLE;
  sg_wMasterRcvToggle = ptGPIOA->wRAWDATA & MASTER_RCV_TOGGLE;

  // Master will not start sending the command until we have
  // disabled Slave Output
  if (!SetSlaveDataOutput(FALSE))
    goto ExitMsg;

  wCrc = 0;
  sg_wRxDataIn = 0;

  // Read Command
  if (!GetWordFromMaster(&wTmp))
    goto ExitMsg;
  wCrc = CalculateCrc(wCrc, wTmp);
  sg_pwRxBuffer[sg_wRxDataIn++] = wTmp;

  // Determine command
  switch (wTmp >> 12) {
  case CMD_RD_VAR_WORD:
    wRcvWords = 0;
    wXmtWords = 1;
    wTmp &= 0x0fff;
    if (wTmp >= (sizeof(g_tVarArray)/2))
      goto ExitMsg;
    sg_pwDataArray = (SnWord *) &g_tVarArray + wTmp;
    break;
  case CMD_RD_VAR_WORDS:
    wRcvWords = 0;
    wXmtWords = (wTmp & 0x3FF);
    if (!GetWordFromMaster(&wTmp))
      goto ExitMsg;
    wCrc = CalculateCrc(wCrc, wTmp);
    sg_pwRxBuffer[sg_wRxDataIn++] = wTmp;
    if (wXmtWords > 512 || (wTmp + wXmtWords) > (sizeof(g_tVarArray)/2))
      goto ExitMsg;
    sg_pwDataArray = (SnWord *) &g_tVarArray + wTmp;
    break;
  case CMD_WR_VAR_WORD:
    wRcvWords = 1;
    wXmtWords = 0;
    wTmp &= 0x0fff;
    if (wTmp >= (sizeof(g_tVarArray)/2))
      goto ExitMsg;
    sg_pwDataArray = (SnWord *) &g_tVarArray + wTmp;
    break;
  case CMD_WR_VAR_WORDS:
    wRcvWords = (wTmp & 0x3FF);
    wXmtWords = 0;
    if (!GetWordFromMaster(&wTmp))
      goto ExitMsg;
    wCrc = CalculateCrc(wCrc, wTmp);
    sg_pwRxBuffer[sg_wRxDataIn++] = wTmp;
    if (wRcvWords > 512 || (wTmp + wRcvWords) > (sizeof(g_tVarArray)/2))
      goto ExitMsg;
    sg_pwDataArray = (SnWord *) &g_tVarArray + wTmp;
    break;
  case CMD_WR_FLASH_BLK:
    wRcvWords = 512;
    wXmtWords = 0;
    break;
  case CMD_SERIAL:
    wRcvWords = (wTmp & 0xFF);
    wXmtWords = 0;
    break;
  default:
    goto ExitMsg;
  }
    
  // Read the rest of the words of the command
  while (wRcvWords > 0) {
    if (!GetWordFromMaster(&wTmp))
      goto ExitMsg;
    wCrc = CalculateCrc(wCrc, wTmp);
    sg_pwRxBuffer[sg_wRxDataIn++] = wTmp;
    wRcvWords--;
  }
    
  // Check Rx CRC
  if (!GetWordFromMaster(&wTmp))
    goto ExitMsg;
  yAck = (wTmp == ((CRC << 12) | wCrc));

  if (!SetSlaveDataOutput(TRUE))
    goto ExitMsg;

  // Send the rest of the words for the command
  wCrc = 0;
  while (wXmtWords > 0) {
    wTmp = *sg_pwDataArray++;
    wCrc = CalculateCrc(wCrc, wTmp);
    if (!SendWordToMaster(wTmp))
      goto ExitMsg;
    wXmtWords--;
  }
    
  // Send Ack/Nack with Tx CRC
  if (!yAck)
    SendWordToMaster((NACK << 12) | wCrc);
  else {
    SendWordToMaster((ACK << 12) | wCrc);

    // Reload heartbeat timer
    g_wHeartbeat = g_tVarArray.wHeartCount;        

    // Execute Write, Flash and Serial commands
    switch (sg_pwRxBuffer[0] >> 12) {
    case CMD_WR_VAR_WORD:
      *sg_pwDataArray++ = sg_pwRxBuffer[1];
      break;
    case CMD_WR_VAR_WORDS:
      for (wTmp = 2; wTmp < sg_wRxDataIn; wTmp++)
        *sg_pwDataArray++ = sg_pwRxBuffer[wTmp];
      break;
    case CMD_WR_FLASH_BLK:
      sg_eMsgWait = MSG_WAIT_FOR_FLASH;
      break;
    case CMD_SERIAL:
      sg_eMsgWait = MSG_WAIT_FOR_SERIAL;
      sg_pwRxBuffer[sg_wRxDataIn] = 0;
      StartSerialCmdList(sg_pwRxBuffer);
      break;
    }
  }
ExitMsg:
  // Set Slave Ouput Flag to force Master to wait until SetSlaveDataOutput() is called
  // on the next interrupt so the toggle flags stay in sync.
  ptGPIOA->wDR |= SLAVE_OUTPUT;
}

void DeferredFlashCommand(void)
{  
  SnFlashError eFlashError;
  SnBool yError = TRUE;
  int iPage;
  
  if (sg_eMsgWait != MSG_WAIT_FOR_FLASH)    
    return;
  
  if ((sg_pwRxBuffer[0] >> 12) == CMD_WR_FLASH_BLK) {
    iPage = (int)(sg_pwRxBuffer[0] & 0x7f) + 128;
    eFlashError = g_tFlashFuncTable.rEraseFlashPage(SN_FLASH_MEM_PROGRAM,iPage);
    if(eFlashError == SN_FLASH_ERROR_NONE) {
      eFlashError = g_tFlashFuncTable.rWriteFlashPage((SnAddr)&sg_pwRxBuffer[1],SN_FLASH_MEM_PROGRAM,iPage);
    }
    if(eFlashError == SN_FLASH_ERROR_NONE) {
      yError = FALSE;
    }
  }
  sg_eMsgWait = MSG_NO_WAIT;
  
  // Set READY on sucess and interrupt the Master
  if (!yError)
    ptGPIOA->wDR |= SLAVE_READY;
  SysInt;
}

void SerialCmdsComplete(SnBool yError)
{
  g_wLoadSerialTimeout = CLR_SERIAL_TIMEOUT;
  if (sg_eMsgWait != MSG_WAIT_FOR_SERIAL)     
    return;
    
  if ((sg_pwRxBuffer[0] >> 12) != CMD_SERIAL)
    yError = TRUE;
  sg_eMsgWait = MSG_NO_WAIT;
  
  // Set READY on sucess and interrupt the Master
  if (!yError)
    ptGPIOA->wDR |= SLAVE_READY;
  SysInt;
}
