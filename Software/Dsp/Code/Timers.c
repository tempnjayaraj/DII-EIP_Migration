#include "Controller.h"

/*******************************************************
 Motor Port A Timer A3 Level 1 Interrupt
 *******************************************************/
void PortA_Timer_Interrupt(void)
{
#pragma interrupt saveall
#if UT_TEST == UT_CTRL_A_TIMER
  DbgD11ON;
#endif
  /* Measure motor current and commutate brushless A motor */
  g_tVarArray.tBldcA.tIn.fCurrent = (g_tVarArray.tBldcA.tIn.wCommIndex == REV ? -AMP_CONVERT : AMP_CONVERT) * g_tVarArray.tAnalog.sAverage[0];
  ControlBldc(&g_tVarArray.tBldcA, ptPWMA);

  ptTMRA->tChan3.wCOMSCR &= ~0x0010;  // clear timer compare interrupt flag TCF1
#if UT_TEST == UT_CTRL_A_TIMER
  DbgD11OFF;
#endif
}

/*******************************************************
 Motor Port B Timer B3 Level 1 Interrupt
 *******************************************************/
void PortB_Timer_Interrupt(void)
{
#pragma interrupt saveall
#if UT_TEST == UT_CTRL_B_TIMER
  DbgD11ON;
#endif
  /* Measure motor current and commutate brushless B motor */
  g_tVarArray.tBldcB.tIn.fCurrent = (g_tVarArray.tBldcB.tIn.wCommIndex == REV ? -AMP_CONVERT : AMP_CONVERT) * g_tVarArray.tAnalog.sAverage[8];
  ControlBldc(&g_tVarArray.tBldcB, ptPWMB);

  ptTMRB->tChan3.wCOMSCR &= ~0x0010;  // clear timer compare interrupt flag TCF1
#if UT_TEST == UT_CTRL_B_TIMER
  DbgD11OFF;
#endif
}