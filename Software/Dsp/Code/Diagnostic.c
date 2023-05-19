#include "Controller.h"

/*
  Check for 24 volt short circuits by turning off bottom FETs
  and ramping PWM to top FETs while monitoring current. This is
  called incrementally from the .005 sec brushless motor control
  loop in state wMode 8 (MODE_DIAGNOSE). If current measures greater
  than 1.0A then power and commutation are disabled and a fault
  bit is set before returning to wMode 0 (MODE_OFF).

  Applications must call this whenever a change is detected on
  motor ports A or B to prevent 24V from being applied through
  a short to ground. This can be done automatically by initializing
  newly detected motors with wMode = 8 (MODE_DIAGNOSE). Test wFault
  after the motor board returns to state wMode 0 (MODE_OFF) to obtain
  the result of the test.
  
  Calling procedure: wMode = 8 (MODE_DIAGNOSE)
  Read wFault when wMode = 0: ERROR_SHORT_24 (wFault bit 6 = 1)
                                          OK (wFault bit 6 = 0)
*/

SnWord TestForShort(Async* ptBldc, PWM* ptPwm)
{
  SnWord Status = BUSY;

  /* Disable commutation timer interrupts */
  Disable_Commutation_Interrupts;

  ptPwm->wOUT = PWM_TOP_BOT_OFF;  // turn off bottom FETs & PWM top

  /* Test sequence: step PWM from 0 to 25% in 100 steps */
  if ((ptPwm->wVAL[0] += 3) <= 300) {
    ptPwm->wCTL |= 2;

#if UT_TEST == UT_DIAGNOSE
    DbgD11ON;
#endif

    /* Fail if current measures greater than 1.0 amp */
    if (fabs(ptBldc->tIn.fCurrent) > 1.0) {
      ptPwm->wOUT = FETS_OFF;     // turn off all FETs
      ResetPWM;
      ptBldc->tEx.wFault |= Status = ERROR_SHORT_24;
#if UT_TEST == UT_DIAGNOSE
    DbgD11OFF;
#endif
    }
  }
  /* Test sequence is over */
  else {
    Status = OK;
    ResetPWM;
    ptBldc->tEx.wFault &= ~ERROR_SHORT_24;
#if UT_TEST == UT_DIAGNOSE
    DbgD11OFF;
#endif

    /* Enable commutation timer interrupts */
    Enable_Commutation_Interrupts;
  }

  return Status;
}