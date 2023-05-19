#include "Controller.h"

/*******************************************************
 Brushless Motor State and PID Controller
 *******************************************************/
void ControlBldc(Async *ptBldc, PWM *ptPwm)
{
  float fProTerm, fPidTar, fPidAct, fPwmOut;
  SnSWord sTlim, sTacs, sWinLock;
  Kgain tK;
  GetActualVelocity(ptBldc);
  
/*
  wMode state controller converts mode and velocity commands to signed velocity target
*/

  switch (ptBldc->tEx.wMode) {

    case MODE_OFF:                                                      // 0 reset motor
      ptBldc->tEx.wFault &= ERROR_SHORT_24;
      ptBldc->tIn.sDirFaultCnt = ptBldc->tIn.wDirFaultCtr = 0;
      ptBldc->tIn.fVelCmd = ptBldc->tIn.sOverload = ptBldc->tIn.wMilliSec = 0;
      fPidTar = CalculateTargetVelocity(ptBldc);
      fPidAct = ptBldc->tIn.fVelAct;
      tK = ptBldc->tEx.tKvel;                                           // use velocity coefficients
      if (fabs(fPidTar) < 1.0) {
        ResetPWM;
        ptBldc->tIn.wCtrl = MODE_OFF;
      }
      break;

    case MODE_REVERSE:                                                  // 1 reverse
      if (ptBldc->tIn.wCtrl != MODE_REVERSE) {
        ResetPID;
        ptBldc->tIn.wDirCmd = ptBldc->tIn.wCtrl = MODE_REVERSE;
      }
      ptBldc->tIn.fVelCmd = -ptBldc->tEx.sVelSet;
      fPidTar = CalculateTargetVelocity(ptBldc);
      fPidAct = ptBldc->tIn.fVelAct;
      tK = ptBldc->tEx.tKvel;                                           // use velocity coefficients
      break;

    case MODE_FORWARD:                                                  // 2 forward
      if (ptBldc->tIn.wCtrl != MODE_FORWARD) {
        ResetPID;
        ptBldc->tIn.wDirCmd = ptBldc->tIn.wCtrl = MODE_FORWARD;
      }
      ptBldc->tIn.fVelCmd = ptBldc->tEx.sVelSet;
      fPidTar = CalculateTargetVelocity(ptBldc);
      fPidAct = ptBldc->tIn.fVelAct;
      tK = ptBldc->tEx.tKvel;                                           // use velocity coefficients
      break;

    case MODE_OSCILLATE:                                                // 3 traditional oscillate
      if (ptBldc->tIn.wCtrl != MODE_OSCILLATE) {
        ResetPID;
        ptBldc->tIn.wDwellCnt = ptBldc->tEx.wDwell;                     // set counter
        if (ptBldc->tIn.wDirCmd == MODE_REVERSE) {
          ptBldc->tIn.fVelCmd = -ptBldc->tEx.sVelSet;
          ptBldc->tIn.wDirCmd = MODE_FORWARD;
        }
        else {
          ptBldc->tIn.fVelCmd = ptBldc->tEx.sVelSet;
          ptBldc->tIn.wDirCmd = MODE_REVERSE;
        }
        ptBldc->tIn.wCtrl = MODE_OSCILLATE;
      }
      else if (ptBldc->tIn.wDwellCnt)
        --ptBldc->tIn.wDwellCnt;
      else {
        ptBldc->tIn.wDwellCnt = ptBldc->tEx.wDwell;                     // reset counter and reverse direction
        if (ptBldc->tIn.fVelCmd < 0)
          ptBldc->tIn.fVelCmd = ptBldc->tEx.sVelSet;
        else
          ptBldc->tIn.fVelCmd = -ptBldc->tEx.sVelSet;
        ptBldc->tIn.wDirCmd ^= MODE_OSCILLATE;                          // toggle last direction
      }
      fPidTar = CalculateTargetVelocity(ptBldc);
      fPidAct = ptBldc->tIn.fVelAct;
      tK = ptBldc->tEx.tKvel;                                           // use velocity coefficients
      break;

    case MODE_POSITION:                                                 // 4 position profile
      if (ptBldc->tIn.wCtrl != MODE_POSITION) {
        ptBldc->tIn.wCtrl = MODE_POSITION;
        ResetPID;
        ptBldc->tIn.fVelTar = ptBldc->tIn.fVelAct = ptBldc->tIn.fV = ptBldc->tIn.fS = ptBldc->tIn.lTacCnt = 0;
        ptBldc->tEx.wProfileCmd |= PCON_RESET;
      }
      ptBldc->tEx.wProfileCmd |= PCON_PROFILE;
      fPidTar = CalculateTargetPosition(ptBldc, (Profile *)&ptBldc->tMotorProfile, ptBldc->tEx.fCycleTime, ptBldc->tIn.fTacPerRev);
      fPidAct = ptBldc->tIn.lTacCnt;
      ptBldc->tIn.fAccAmp = (ptBldc->tIn.fA * ptBldc->tEx.fAccCvrt * 60) / ptBldc->tIn.fTacPerRev;
      tK = ptBldc->tEx.tKpos;                                           // use position coefficients
      break;

    case MODE_JOG:                                                      // 5 jog armature (must stop with mode 0)
      if (ptBldc->tIn.wCtrl != MODE_JOG) {
	    Disable_Commutation_Interrupts;
        ptBldc->tIn.fVelTar = ptBldc->tIn.wMilliSec = 0;
	    ptBldc->tIn.wCtrl = MODE_JOG;
      }
      if (!(ptBldc->tIn.wMilliSec || ptBldc->tEx.wFault & ERROR_SHORT_24)) {
        ptBldc->tIn.wMilliSec =(SnWord)(7500.0/ptBldc->tIn.fTacPerRev); // 7.5 seconds per revolution
        ptPwm->wVAL[0] = ptBldc->tEx.tKpos.fKp * (ptBldc->tEx.fResistance + 10);           // jog current
        ptPwm->wCTL |= 2;
        ptPwm->wOUT = *(ptBldc->tEx.wFCommTable + (ptBldc->tIn.wLastTac = ptBldc->tEx.wJogTable[ptBldc->tIn.wJogIndex]));
        if (++ptBldc->tIn.wJogIndex > 5) ptBldc->tIn.wJogIndex = 0;
        ptBldc->tIn.sWinLock = 0;
      }
      return;

    case MODE_LOCK:                                                     // 6 lock armature
      ptBldc->tIn.wCtrl = MODE_LOCK;
      fPidTar = ptBldc->tIn.tStop.P[1].Y;
      fPidAct = ptBldc->tIn.lTacCnt;
      tK = ptBldc->tEx.tKpos;                                           // use position coefficients
      if (!ptBldc->tIn.wMilliSec || ptBldc->tIn.sOverload > 20)
        ptBldc->tEx.wMode = MODE_OFF;
      break;

    case MODE_STOP:                                                     // 7 WindowLock stop
      if (ptBldc->tIn.wCtrl != MODE_STOP) {
      	if(ptBldc->tIn.wCtrl == MODE_POSITION)
      		ptBldc->tIn.fV = ptBldc->tIn.fV * 60 / ptBldc->tIn.fTacPerRev;
      	else if (ptBldc->tIn.fVelTar <= -100 || ptBldc->tIn.fVelTar >= 100)
      		ptBldc->tIn.fV = ptBldc->tIn.fVelTar;
      	else
      		ptBldc->tIn.fV = (ptBldc->tIn.fVelTar >= 0)? 100: -100;
      	
        ptBldc->tIn.fProLast = ptBldc->tIn.fVelTar = ptBldc->tIn.fS = ptBldc->tIn.lTacCnt = 0;

        /* Calculate stopping distance in tacs */
        sWinLock = ptBldc->tIn.sWinLock;
        sTacs = (SnSWord)(ptBldc->tIn.fV * ptBldc->tIn.fV / (ptBldc->tEx.fDecel * 120)) * ptBldc->tIn.fTacPerRev;
        if (ptBldc->tIn.fV < 0)
          sTacs -= (sWinLock / ptBldc->tEx.wTacPerRevD);
        else if (ptBldc->tIn.fV > 0)
          sTacs = ((ptBldc->tEx.wTacPerRevN - sWinLock) / ptBldc->tEx.wTacPerRevD) - sTacs;
        else {
          ptBldc->tIn.tStop.P[1].Y = sTacs;
          ptBldc->tEx.wMode = MODE_LOCK;
          ptBldc->tIn.wMilliSec = 2000;
          return;
        }
          
        ptBldc->tIn.fV = ptBldc->tIn.fV / 60 * ptBldc->tIn.fTacPerRev;  // scale to tacs per sec
        ptBldc->tEx.fStopTime = fabs(2 * sTacs / ptBldc->tIn.fV);       // fV won't be zero

        /* Set stop position */
        if (ptBldc->tIn.wCtrl == MODE_OFF)
          ptBldc->tIn.tStop.P[1].Y = 0;
        else
          ptBldc->tIn.tStop.P[1].Y = sTacs;

        ptBldc->tEx.wProfileCmd |= PCON_RESET;
        ptBldc->tIn.wCtrl = MODE_STOP;
      }
      ptBldc->tEx.wProfileCmd |= PCON_PROFILE | PCON_HALT;
      if ((fPidTar = CalculateTargetPosition(ptBldc, &ptBldc->tIn.tStop, ptBldc->tEx.fStopTime, 1)) == ptBldc->tIn.tStop.P[1].Y) {
        ptBldc->tEx.wMode = MODE_LOCK;
        ptBldc->tIn.wMilliSec = 2000;
        return;
      }
      fPidAct = ptBldc->tIn.lTacCnt;
      ptBldc->tIn.fAccAmp = (ptBldc->tIn.fA * ptBldc->tEx.fAccCvrt * 60) / ptBldc->tIn.fTacPerRev;
      tK = ptBldc->tEx.tKpos;                                           // use position coefficients
      break;

    case MODE_DIAGNOSE:                                                 // 8 diagnostics
      ResetPWM;
      ptBldc->tEx.wMode = MODE_CONT;
      // fall through

  	case MODE_CONT:
      if (TestForShort(ptBldc, ptPwm) == BUSY)
        return;
      else
        ptBldc->tEx.wMode = MODE_OFF;
        // fall thru default

    default:
      ResetPID;
      ResetPWM;
      fPidTar = fPidAct = 0;
      return;
  }

/*
  PID controller with feed forward, motor acceleration and current compensation
*/

  /* Proportional term */
  fProTerm = fPidTar - fPidAct;                                         // velocity or position error

  /* Integral term */
  ptBldc->tIn.fIntTerm = tK.fKi * fProTerm + ptBldc->tIn.fIntTerm;      // scaled integral of proportional error

  /* Derivative term */
  ptBldc->tIn.fDerTerm = fProTerm - ptBldc->tIn.fProLast;               // derivative of proportional error
  ptBldc->tIn.fProLast = fProTerm;

  /* Feed forward PWM = f(rpm) */
  fPwmOut = ptBldc->tEx.fFrpmA * ptBldc->tIn.fVelTar;
  if (ptBldc->tIn.fVelTar > 0)
    fPwmOut += ptBldc->tEx.wFrpmB;
  else if (ptBldc->tIn.fVelTar < 0)
    fPwmOut -= ptBldc->tEx.wFrpmB;

  /* F + P + I + D + A + L */
  fPwmOut = tK.fKf * fPwmOut + tK.fKp * fProTerm + ptBldc->tIn.fIntTerm + tK.fKd * ptBldc->tIn.fDerTerm +
           (tK.fKa * ptBldc->tIn.fAccAmp + tK.fKl * (ptBldc->tIn.fCurrent - ptBldc->tIn.fAccAmp)) *
            ptBldc->tEx.fResistance * ptPwm->wCM / VCC;

  /* Signed fPwmOut to PWM and direction control */
  if (ptBldc->tIn.fVelTar > 0) {
    if (ptBldc->tIn.wCommIndex == REV) {
      ptBldc->tIn.wCommIndex = FWD;                                     // run forward
      ptBldc->tIn.fIntTerm = -ptBldc->tIn.fIntTerm;                     // reverse integral term on zero crossing
    }
    if (fPwmOut < 0)
      fPwmOut = 0;
  }
  else if (ptBldc->tIn.fVelTar < 0) {
    if (ptBldc->tIn.wCommIndex == FWD) {
      ptBldc->tIn.wCommIndex = REV;                                     // run reverse
      ptBldc->tIn.fIntTerm = -ptBldc->tIn.fIntTerm;                     // reverse integral term on zero crossing
    }
    if (fPwmOut > 0)
      fPwmOut = 0;
    else
      fPwmOut = -fPwmOut;
  }
  /* WindowLock happens here */
  else if (fPwmOut > 0)
    ptBldc->tIn.wCommIndex = FWD;                                       // torque forward
  else if (fPwmOut < 0) {
    ptBldc->tIn.wCommIndex = REV;                                       // torque reverse
    fPwmOut = -fPwmOut;
  }

  /* Torque limit */
  sTlim = ptBldc->tEx.fTlimA * fabs(ptBldc->tIn.fVelAct) + ptBldc->tEx.wTlimB;
  sTlim = sTlim >= ptPwm->wCM ? ptPwm->wCM : sTlim;

  /* Brushless motor output */
  if (ptBldc->tIn.wCtrl) {
    if (fPwmOut > sTlim) {
      ptPwm->wVAL[0] = sTlim;
      if (sTlim < ptPwm->wCM)
        ptBldc->tEx.wFault |= ERROR_MOTOR_TLIM;                         // instantaneous flag - motor is in torque limit
      if (fabs(ptBldc->tIn.fVelAct) < 1)
        ptBldc->tEx.wFault |= ERROR_MOTOR_STALL;                        // instantaneous flag - motor is stalled
    }
    else {
      ptPwm->wVAL[0] = fPwmOut;
      ptBldc->tEx.wFault &= ~(ERROR_MOTOR_STALL | ERROR_MOTOR_TLIM);
    }

    /* Indicate high current operation */
    if (fabs(ptBldc->tIn.fCurrent) > ptBldc->tEx.fIhigh)
      ptBldc->tEx.wFault |= ERROR_MOTOR_IHIGH;                          // instantaneous flag - motor in current operation
    else
      ptBldc->tEx.wFault &= ~ERROR_MOTOR_IHIGH;
    
    /* Check for DSP current limit */
    if (ptPwm->wFSA & 0x0100) {
      ptBldc->tEx.wFault |= ERROR_MOTOR_ILIM;                           // instantaneous flag - motor in DSP current limit
      ptPwm->wFSA |= 0x0001;
    }

    /* Bound integral term and indicate saturation */
    if (ptBldc->tIn.fIntTerm > sTlim) {
      ptBldc->tIn.fIntTerm = sTlim;
      ptBldc->tEx.wFault |= ERROR_MOTOR_INT_SAT;
    }
    else if (ptBldc->tIn.fIntTerm < -sTlim) {
      ptBldc->tIn.fIntTerm = -sTlim;
      ptBldc->tEx.wFault |= ERROR_MOTOR_INT_SAT;
    }
    else
      ptBldc->tEx.wFault &= ~ERROR_MOTOR_INT_SAT;

    /* Check for and indicate wrong motor direction */
    if (ptBldc->tIn.wDirFaultCtr++ < 100) {
      if (ptBldc->tIn.fVelAct && (ptBldc->tIn.wDirAct != ptBldc->tIn.wCommIndex))
        ++ptBldc->tIn.sDirFaultCnt;
      else
        --ptBldc->tIn.sDirFaultCnt;
    }
    else {
      if (ptBldc->tIn.sDirFaultCnt > 60)
        ptBldc->tEx.wFault |= ERROR_MOTOR_DIRECTION;
      else
        ptBldc->tEx.wFault &= ~ERROR_MOTOR_DIRECTION;

      ptBldc->tIn.sDirFaultCnt = ptBldc->tIn.wDirFaultCtr = 0;
    }
  }
  else {                                                                // motor is disabled
    ResetPID;
    ptPwm->wVAL[0] = 0;
  }
  ptPwm->wCTL |= 2;                                                     // set LDOK to load in PWM values

  /* Reset maximum allowable set speed if set out of range */
  if (ptBldc->tEx.sVelSet > ptBldc->tEx.sVelMax) {
    ptBldc->tEx.sVelSet = ptBldc->tEx.sVelMax;
    ptBldc->tEx.wFault |= ERROR_MOTOR_SET_SPEED;
  }
  else if (ptBldc->tEx.sVelSet < -ptBldc->tEx.sVelMax) {
    ptBldc->tEx.sVelSet = -ptBldc->tEx.sVelMax;
    ptBldc->tEx.wFault |= ERROR_MOTOR_SET_SPEED;
  }
  else
    ptBldc->tEx.wFault &= ~ERROR_MOTOR_SET_SPEED;

 /* Holey bucket motor overload accumulator */
  if (ptBldc->tEx.wFault & (ERROR_MOTOR_STALL | ERROR_MOTOR_ILIM) && ptBldc->tIn.sOverload < 200)
    ptBldc->tIn.sOverload += 2;
  else if (ptBldc->tIn.sOverload > 0)
    --ptBldc->tIn.sOverload;

  /* Set system interrupt flag */
  if (ptBldc->tEx.wFault & ptBldc->tEx.wAssert) {
    if (ptBldc == &g_tVarArray.tBldcA)
      g_tVarArray.tInterrupt.wFlags |= INTERRUPT_MOTOR_A;
    else
      g_tVarArray.tInterrupt.wFlags |= INTERRUPT_MOTOR_B;
  }
}