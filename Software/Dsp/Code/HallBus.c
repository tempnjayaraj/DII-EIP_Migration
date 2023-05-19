#include "Controller.h"

/* HallBus port pin macros */
#define SetHallBusAddress ptGPIOC->wDR |= 0x100
#define ClrHallBusAddress ptGPIOC->wDR &= ~0x100
#define SetHallBusReset   ptGPIOC->wDR |= 0x200
#define ClrHallBusReset   ptGPIOC->wDR &= ~0x200

/* Define hall bus hysteresis requirement */
#define ZERO_OFFSET 1024                // 0.75V hall bus analog signal hysteresis
#define HIGH_LIMIT 3755                 // 2.75V hall bus high voltage fault limit
#define LOW_LIMIT 341                   // 0.25V hall bus low voltage fault limit

/* Number of times the same value of Hall Bus is read before accepting it. */
#define HALL_DEBOUNCE	3

typedef struct {
  SnWord wHallBusIq;
  SnWord wDeviceExistTmp;
  SnWord wDeviceActiveTmp;
  SnWord wDeviceLastLatch;
  SnWord wDeviceActiveLast;
  SnWord wDeviceExistLast;
  SnWord wDeviceDbncCnt;
} HallVar;

void HallDebounceAndLatch(HallBus * const ptHallBus, HallVar * const ptHallVar, SnWord wInterrupt);

static HallVar tHallA = { 0 };
static HallVar tHallB = { 0 };

void HallDebounceAndLatch(HallBus * const ptHallBus, HallVar * const ptHallVar, SnWord wInterrupt)
{
#pragma interrupt called

  SnWord w;

  /* Is there a new Active or Exist value? */
  if (ptHallVar->wDeviceExistTmp != ptHallBus->wDeviceExist ||
      ptHallVar->wDeviceActiveTmp != ptHallBus->wDeviceActive) {

#if HALL_DEBOUNCE
    /* Load debounce counter */
    if (ptHallVar->wDeviceDbncCnt == 0) {
      ptHallVar->wDeviceDbncCnt = HALL_DEBOUNCE;
      ptHallBus->wDeltaCnt++;
    } else {
      /* If same as last read, decrement debounce counter. */
      /* Otherwise reset debounce counter. */
      if (ptHallVar->wDeviceExistTmp == ptHallVar->wDeviceExistLast &&
          ptHallVar->wDeviceActiveTmp == ptHallVar->wDeviceActiveLast) {
    	  if (--ptHallVar->wDeviceDbncCnt == 0) {
          if (ptHallBus->wDeltaCnt < 0xFFFF)
            ptHallBus->wDeltaCnt+=1000;
        }
      } else {
        ptHallVar->wDeviceDbncCnt = HALL_DEBOUNCE;
        ptHallBus->wDeltaCnt++;
      }
    }
    ptHallVar->wDeviceExistLast = ptHallVar->wDeviceExistTmp;
    ptHallVar->wDeviceActiveLast = ptHallVar->wDeviceActiveTmp;
#else
    if (ptHallBus->wDeltaCnt < 0xFFFF)
      ptHallBus->wDeltaCnt++;
#endif
  } else {
    /* New value changed back before debounce period is over, so ignore it. */
    ptHallVar->wDeviceDbncCnt = 0;
  }

  /* Process the new values if not debouncing */
  if (!ptHallVar->wDeviceDbncCnt) {
    ptHallBus->wDeviceExist = ptHallVar->wDeviceExistTmp;
    ptHallBus->wDeviceActive = ptHallVar->wDeviceActiveTmp;

    /* DeviceLatch contains  push-on / push-all-off state data */
    w = (ptHallBus->wDeviceActive ^ ptHallVar->wDeviceLastLatch) & ptHallBus->wDeviceActive;

    /* Set HallBus system interrupt flag if needed */
    if (w & ptHallBus->wDeviceAssert)
      g_tVarArray.tInterrupt.wFlags |= wInterrupt;
    
    w &= HALL_MDU_CTL_MASK;
    /* Update latch press on/press any off status */
    if (w && ptHallBus->wDeviceLatch)
      ptHallBus->wDeviceLatch = 0;
    else if (w)
      ptHallBus->wDeviceLatch = w ^ ptHallBus->wDeviceLatch & w;

    /* Save for next use */
    ptHallVar->wDeviceLastLatch = ptHallVar->wDeviceActiveTmp & HALL_MDU_CTL_MASK;
  }
}

/*******************************************************
 HallBus Timer D0 0.5 msec Level 0 Interrupt

 Legacy DYONICS Power shaver sensor system

 Bit definitions:
  0 - low speed blade family sensor
  1 - high speed blade family sensor
  2 - reverse hand control button sensor
  3 - forward hand control button sensor
  4 - oscillate hand control button sensor
  5 - high torque shaver high speed blade family sensor
  6 - footswitch jog button sensor

 DeviceExist contains hall devices present pattern
 DeviceActive contains activated hall device pattern
 *******************************************************/

void HallBus_Timer_Interrupt(void)
{
#pragma interrupt saveall

  static SnWord wHallBusCounter = 1, wHallBusMask;
  static SnBool yMsToggle = FALSE;

#if UT_TEST == UT_HALL_TIMER
  DbgD11ON;
#endif

  /*
   * If the Heartbeat ms timeout counter is active (non-zero)
   * and it counts down to zero, halt the system and put it in a safe mode
   */
  if (g_wHeartbeat && --g_wHeartbeat == 0) {
    ptITCN->wICTL |= 0x0020;            // disable all interrupts
    ptPWMA->wOUT = FETS_OFF;            // reset PWMA outputs
    ptPWMB->wOUT = FETS_OFF;            // reset PWMB outputs
    BeepON;                             // turn on beeper
    for(;;) Reload_Watchdog;            // stay here
  }

  /* Read all ADC channels at 2 KHz */
  GetAnalogInput();

  /* If the 400us timer has not expired since the last HallBus Interrupt, wait for the next one */
  if (ptTMRD->tChan1.wSCR & 0x8000) {
    /* Read hall bus patterns and state into HallDeviceExist and HallDeviceActive
       This needs to be on a timer interrupt @ 0.5 msec max for 18 msec return */

    switch (wHallBusCounter) {

    /* Reset hall bus */
    case 1:
      SetHallBusReset;
      break;

    /* Clear reset */
    case 2:
      ClrHallBusReset;
      break;

    /* Initialize, qualify and save quiescent current */
    case 3:
      tHallA.wDeviceExistTmp = 0;
      tHallA.wDeviceActiveTmp = 0;
      tHallB.wDeviceExistTmp = 0;
      tHallB.wDeviceActiveTmp = 0;
      wHallBusMask = 1;

      if (g_tVarArray.tAnalog.sData[3] > HIGH_LIMIT || g_tVarArray.tAnalog.sData[3] < LOW_LIMIT)
        g_tVarArray.tInterrupt.wFlags |= INTERRUPT_HALL_BUS_A;
      tHallA.wHallBusIq = (g_tVarArray.tHallBusA.wHallBusVq = g_tVarArray.tAnalog.sData[3]) + ZERO_OFFSET;

      if (g_tVarArray.tAnalog.sData[9] > HIGH_LIMIT || g_tVarArray.tAnalog.sData[9] < LOW_LIMIT)
        g_tVarArray.tInterrupt.wFlags |= INTERRUPT_HALL_BUS_B;
      tHallB.wHallBusIq = (g_tVarArray.tHallBusB.wHallBusVq = g_tVarArray.tAnalog.sData[9]) + ZERO_OFFSET;

      ClrHallBusAddress;
      break;

    /* Build bit pattern for up to 8 detected devices */
    case 4: case 6: case 8: case 10: case 12: case 14: case 16: case 18:
      if (g_tVarArray.tAnalog.sData[3] > tHallA.wHallBusIq) tHallA.wDeviceExistTmp |= wHallBusMask;
      if (g_tVarArray.tAnalog.sData[9] > tHallB.wHallBusIq) tHallB.wDeviceExistTmp |= wHallBusMask;
      
      SetHallBusAddress;
      break;

    /* Build bit pattern for activated devices */
    case 5: case 7: case 9: case 11: case 13: case 15: case 17:
      if (g_tVarArray.tAnalog.sData[3] > tHallA.wHallBusIq) tHallA.wDeviceActiveTmp |= wHallBusMask;
      if (g_tVarArray.tAnalog.sData[9] > tHallB.wHallBusIq) tHallB.wDeviceActiveTmp |= wHallBusMask;
      wHallBusMask *= 2;

      ClrHallBusAddress;
      break;
    }

    /* Reset the 400us timer */
    ptTMRD->tChan1.wSCR &= ~0x8000;       // clear timer compare flag TCF
    ptTMRD->tChan1.wCTRL |= 0x2000;       // start timer

    /* Debounce and transfer static data to external variables */
    if (++wHallBusCounter == 19) {        // test for end of hall bus scan
      wHallBusCounter = 1;
    
      HallDebounceAndLatch(&g_tVarArray.tHallBusA, &tHallA, INTERRUPT_HALL_BUS_A);
      HallDebounceAndLatch(&g_tVarArray.tHallBusB, &tHallB, INTERRUPT_HALL_BUS_B);
    }
  }

  /* Run this block every 1 ms, (every other 0.5 ms tick), so only when yMsToggle is TRUE */
  if (yMsToggle) {
    /* Count down for RS485 communication timeout */
    static volatile SnWord wSerialTimeoutInMs = 0;

    /* Millisecond down counters */
    if (g_tVarArray.tBldcA.tIn.wMilliSec) --g_tVarArray.tBldcA.tIn.wMilliSec;
    if (g_tVarArray.tBldcB.tIn.wMilliSec) --g_tVarArray.tBldcB.tIn.wMilliSec;

    if (g_wLoadSerialTimeout) {
      if (g_wLoadSerialTimeout == CLR_SERIAL_TIMEOUT) {
        wSerialTimeoutInMs = 0;
      } else {
        wSerialTimeoutInMs = g_wLoadSerialTimeout;
      }
      g_wLoadSerialTimeout = 0;
    } else if (wSerialTimeoutInMs) {
      if (--wSerialTimeoutInMs == 0)
        SerialCmdsComplete(TRUE);
    }
  }
  yMsToggle = !yMsToggle;

  ptTMRD->tChan0.wSCR &= ~0x8000;       // clear timer compare flag TCF
#if UT_TEST == UT_HALL_TIMER
  DbgD11OFF;
#endif
}