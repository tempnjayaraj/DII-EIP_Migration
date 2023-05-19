#include "Controller.h"

/*******************************************************
 BLDCA Commutation Capture Level 2 Interrupts
 *******************************************************/
void TacA0_Interrupt(void)
{
#pragma interrupt
  Commutate(&ptTMRA->tChan0, ptPWMA, ptGPIOC, 4, &g_tVarArray.tBldcA);
}

void TacA1_Interrupt(void)
{
#pragma interrupt
  Commutate(&ptTMRA->tChan1, ptPWMA, ptGPIOC, 4, &g_tVarArray.tBldcA);
}

void TacA2_Interrupt(void)
{
#pragma interrupt
  Commutate(&ptTMRA->tChan2, ptPWMA, ptGPIOC, 4, &g_tVarArray.tBldcA);
}

/*******************************************************
 BLDCB Commutation Capture Level 2 Interrupts
 *******************************************************/
void TacB0_Interrupt(void)
{
#pragma interrupt
  Commutate(&ptTMRB->tChan0, ptPWMB, ptGPIOC, 0, &g_tVarArray.tBldcB);
}

void TacB1_Interrupt(void)
{
#pragma interrupt
  Commutate(&ptTMRB->tChan1, ptPWMB, ptGPIOC, 0, &g_tVarArray.tBldcB);
}

void TacB2_Interrupt(void)
{
#pragma interrupt
  Commutate(&ptTMRB->tChan2, ptPWMB, ptGPIOC, 0, &g_tVarArray.tBldcB);
}

/*************************************************************
 BLDC commutation routine (TOFIE is used to force commutation)
 *************************************************************/
void Commutate(Channel* ptTimer, PWM* ptPwm, DigitalPort* ptGPIO, SnWord wShift, Async* ptBldc)
{
#pragma interrupt called

  /* Commutate with debounce */
  SnWord wTacCnt = 0, wTmpTac, wCurTac;
  wCurTac = (ptGPIO->wRAWDATA >> wShift) & 7;
  while (wTacCnt++ < 5) {
    if (wCurTac != (wTmpTac = ptGPIO->wRAWDATA >> wShift & 7)) {
      wCurTac = wTmpTac;
      wTacCnt = 0;
    }
  }
  ptPwm->wOUT = *(ptBldc->tEx.wFCommTable + ptBldc->tIn.wCommIndex + wCurTac);

  /* Interrupt triggered on edge in else case */
  if (wCurTac == ptBldc->tIn.wLastTac) {
  	if (ptTimer->wSCR & 0x0800){
    	ptBldc->tEx.wFault |= ERROR_MOTOR_TAC_NOISE;      // glitch on tac inputs
  	}
  }
  else {
    /* Accumulate directional positions and track armature */
    if (ptBldc->tEx.wFDirTable[wCurTac] == ptBldc->tIn.wLastTac) {
      ++ptBldc->tIn.lTacCnt;
      ptBldc->tIn.wDirAct = FWD;
      ptBldc->tIn.sWinLock += ptBldc->tEx.wTacPerRevD;
      if (ptBldc->tIn.sWinLock >= ptBldc->tEx.wTacPerRevN) {
          ptBldc->tIn.sWinLock -= ptBldc->tEx.wTacPerRevN;
      }
    }
    else if (ptBldc->tEx.wRDirTable[wCurTac] == ptBldc->tIn.wLastTac) {
      --ptBldc->tIn.lTacCnt;
      ptBldc->tIn.wDirAct = REV;
      ptBldc->tIn.sWinLock -= ptBldc->tEx.wTacPerRevD;
      if (ptBldc->tIn.sWinLock < 0) {
          ptBldc->tIn.sWinLock += ptBldc->tEx.wTacPerRevN;
      }
    }
    else
      ptBldc->tEx.wFault |= ERROR_MOTOR_TAC;          // bad commutation pattern

    ptBldc->tIn.wLastTac = wCurTac;                   // save LastTac
  }
  ptTimer->wCNTR = 0;                                 // reset counter to prevent TOF if IEF
  ptTimer->wSCR &= ~0x2800;                           // clear TOF & IEF
}