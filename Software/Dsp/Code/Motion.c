/*
  Motion.c contains structures and functions for creating and
  manipulating various move profiles for servo motor control
  KK051106
*/

#include "Controller.h"
#include <stdlib.h>

/* Create new profile from source buffer */
Profile* NewProfile(SnWord NumSeq, SnWord* Data)
{
  SnWord x, *temp;
  SnIndex i;
  Profile *Shape = NULL;

  if ((Shape = malloc((x = ((NumSeq + 1) * 2 + 1) * 2) * 2)) != NULL) {
    temp = (SnWord*) Shape;
    for (i = 0; i < x; i++)
      temp[i] = Data[i];        // copy count and coordinates from buffer
    temp[0] = NumSeq;           // set count in case buffer is bogus
  }
  return Shape;                 // return address or NULL if error
}


/* Remove profile from memory */
SnWord DeleteProfile(Profile* Shape)
{
  if (Shape) {
    free(Shape);
    return OK;
  }
  else
    return UNDEFINED;
}


/* Load position profiles from main application */
SnWord GetPositionProfile(Async* Motor)
{
  /* Test for new Bldc profile data available */
  if (Motor->tEx.wProfileCmd & PCON_RX_DATA_READY) {    // check data set ready
    static SnWord wWordCount, wNumWords, *pwBLDCbuffer;
    SnWord *pwBuffer, c;

    /* Test for end of new profile data */
    if (Motor->tEx.wProfileCmd & PCON_PROFILE_READY) {  // check profile ready
      Motor->tEx.wProfileCmd &= ~PCON_PROFILE_READY;    // clear profile ready

      // Number of words for array is the value for the number of sequences
      // plus 1 for the last vertex, times 2 plus 1 for the count, times
      // the size of a float, divided by the size of a word.

      wNumWords = g_tVarArray.wBuffer[0] * 4 + 6;
      wWordCount = 0;

      pwBLDCbuffer = (SnWord*)&Motor->tMotorProfile;
    }

    /* Transfer PROFILE_BUFFER_SIZE word maximum data block from buffer to ptProfile buffer */
    pwBuffer = g_tVarArray.wBuffer;
    c = wWordCount + PROFILE_BUFFER_SIZE;
    while (wWordCount++ < (c > wNumWords ? wNumWords : c))
      *pwBLDCbuffer++ = *pwBuffer++;

    if (wWordCount > wNumWords)
      Motor->tEx.wProfileCmd |= PCON_PROFILE_READY;     // set profile ready
    Motor->tEx.wProfileCmd &= ~PCON_RX_DATA_READY;      // clear data set ready
  }

  return OK;
}


/* Motor shaft position profiler */
float CalculateTargetPosition(Async* ptBldc, Profile* Shape, float fPeriod, float fTacPerRev)
{
  float fV;

  /* Reset all variables and return */
  if (ptBldc->tEx.wProfileCmd & PCON_RESET) {
    ptBldc->tIn.wProIndex = -1;
    ptBldc->tIn.fT = ptBldc->tIn.wSeqIndex = ptBldc->tIn.wSeqCount = 0;
    ptBldc->tIn.lEndCnt = fTacPerRev * Shape->P[Shape->Count].Y;
  }

  /* Initialize sequence */
  else if (ptBldc->tEx.wProfileCmd & PCON_PROFILE) {
    if (++ptBldc->tIn.wSeqIndex >= ptBldc->tIn.wSeqCount) {
      ptBldc->tIn.wSeqIndex = 0;

      /* Initialize profile */
      if (++ptBldc->tIn.wProIndex >= Shape->Count) {
        if (ptBldc->tEx.wProfileCmd & PCON_HALT) {
          ptBldc->tEx.wProfileCmd &= ~(PCON_RESET | PCON_PROFILE);
          ptBldc->tIn.wProIndex = Shape->Count;
          ptBldc->tIn.wSeqIndex = ptBldc->tIn.wSeqCount;
          return Shape->P[Shape->Count].Y;
        }
        else {
          Disable_Commutation_Interrupts;
          ptBldc->tIn.lTacCnt -= ptBldc->tIn.lEndCnt;
          Enable_Commutation_Interrupts;
        }
        ptBldc->tIn.fT = ptBldc->tIn.fV = ptBldc->tIn.fS = ptBldc->tIn.wProIndex = 0;
      }

      /* Calculate sequence initial conditions (tac/sec) */
      ptBldc->tIn.fdT = (Shape->P[ptBldc->tIn.wProIndex + 1].X - Shape->P[ptBldc->tIn.wProIndex].X) * fPeriod;
      ptBldc->tIn.fdS = (Shape->P[ptBldc->tIn.wProIndex + 1].Y - Shape->P[ptBldc->tIn.wProIndex].Y) * fTacPerRev;
      ptBldc->tIn.wSeqCount = (ptBldc->tIn.fdT + .00001) / ptBldc->tEx.fLoopRate;
      ptBldc->tIn.fA = ptBldc->tIn.fdT ? 2 * (ptBldc->tIn.fdS / ptBldc->tIn.fdT - ptBldc->tIn.fV) / ptBldc->tIn.fdT : 0;
    }

    /* Integrate target time, position and instantaneous velocity (tac/sec) */
    fV = ptBldc->tIn.fV;
    ptBldc->tIn.fT += ptBldc->tEx.fLoopRate;
    ptBldc->tIn.fV += ptBldc->tIn.fA * ptBldc->tEx.fLoopRate;
    ptBldc->tIn.fS += (ptBldc->tIn.fV + fV) / 2 * ptBldc->tEx.fLoopRate;
  }

  ptBldc->tEx.wProfileCmd &= ~(PCON_RESET | PCON_PROFILE | PCON_HALT);
  return ptBldc->tIn.fS;
}


/* Stop blade on WindowLock position */
float StopOnPosition(Async* ptBldc)
{
  if (++ptBldc->tIn.wSeqIndex < ptBldc->tIn.wSeqCount) {
    float fV = ptBldc->tIn.fV;

    /* Integrate target time, position and instantaneous velocity (tac/sec) */
    ptBldc->tIn.fV += ptBldc->tIn.fA * ptBldc->tEx.fLoopRate;
    ptBldc->tIn.fS += (ptBldc->tIn.fV + fV) / 2 * ptBldc->tEx.fLoopRate;
  }
  else
    ptBldc->tEx.wMode = MODE_OFF;
  
  /* Return target velocity required to achieve position (rpm) */
  return (ptBldc->tIn.fS - ptBldc->tIn.lTacCnt) * 60 / (ptBldc->tEx.fLoopRate * ptBldc->tIn.fTacPerRev);
}

static float    fTacCntATmp = 0;
static float		fTacCntA = 0;
static SnWord   wIntervalA = 0;

static float    fTacCntBTmp = 0;
static float		fTacCntB = 0;
static SnWord   wIntervalB = 0;

/* Get actual velocity */
void GetActualVelocity(Async* ptBldc)
{
	/* Disable commutation timer interrupts */
	Disable_Commutation_Interrupts;

	if (ptBldc == &g_tVarArray.tBldcA) {
	  fTacCntATmp = ptBldc->tIn.lTacCnt - ptBldc->tIn.lTacCntLast;
	}
	else {
	  /* Calculate actual velocity using count */
	  fTacCntBTmp = ptBldc->tIn.lTacCnt - ptBldc->tIn.lTacCntLast;
	}
  ptBldc->tIn.lTacCntLast = ptBldc->tIn.lTacCnt;

  /* Enable commutation timer interrupts */
  Enable_Commutation_Interrupts;
  
  if (ptBldc == &g_tVarArray.tBldcA) {
	  /* Calculate actual velocity using count */
	  ptBldc->tIn.fVelAct = (fTacCntATmp) * 60 / (ptBldc->tEx.fLoopRate * ptBldc->tIn.fTacPerRev);

  	fTacCntA += fTacCntATmp;
		wIntervalA += 1;

   /* Update brushless motor absolute value of actual velocity in RPM every 60 ms*/ 
  	if (wIntervalA >= 0.06/ptBldc->tEx.fLoopRate) {
  		g_tVarArray.tBldcA.tIn.wVelAbs = (fabs(fTacCntA) * 60 / (ptBldc->tEx.fLoopRate * ptBldc->tIn.fTacPerRev))/wIntervalA;
	  	wIntervalA = 0;
	  	fTacCntA = 0;
  	}
  }
  else {
	  /* Calculate actual velocity using count */
	  ptBldc->tIn.fVelAct = (fTacCntBTmp) * 60 / (ptBldc->tEx.fLoopRate * ptBldc->tIn.fTacPerRev);

  	fTacCntB += fTacCntBTmp;
		wIntervalB += 1;

   /* Update brushless motor absolute value of actual velocity in RPM every 60 ms*/ 
  	if (wIntervalB >= 0.06/ptBldc->tEx.fLoopRate) {

  	  g_tVarArray.tBldcB.tIn.wVelAbs = (fabs(fTacCntB) * 60 / (ptBldc->tEx.fLoopRate * ptBldc->tIn.fTacPerRev))/wIntervalB;
  	  wIntervalB = 0;
	  	fTacCntB = 0;
  	}
  }
}


/* Calculate and return target velocity, calculate armature acceleration component */
float CalculateTargetVelocity(Async* ptBldc)
{
  float Accel, Decel;

  /* Scale accelerations for velocity profiler */
  if (ptBldc->tIn.fVelTar >= 0) {
    Accel = ptBldc->tEx.fAccel * ptBldc->tEx.fLoopRate;
    Decel = ptBldc->tEx.fDecel * ptBldc->tEx.fLoopRate;
  }
  else {
    Accel = -ptBldc->tEx.fDecel * ptBldc->tEx.fLoopRate;
    Decel = -ptBldc->tEx.fAccel * ptBldc->tEx.fLoopRate;
  }

  /* Velocity profiler with acceleration torques */
  if (ptBldc->tIn.fVelTar < ptBldc->tIn.fVelCmd - Accel) {
    ptBldc->tIn.fVelTar += Accel;
    ptBldc->tIn.fAccAmp = ptBldc->tEx.fAccel * ptBldc->tEx.fAccCvrt;  // acc * (2 * pi / 60 * Jm * Gx / Ktm)
  }
  else if (ptBldc->tIn.fVelTar > ptBldc->tIn.fVelCmd - Decel) {
    ptBldc->tIn.fVelTar += Decel;
    ptBldc->tIn.fAccAmp = ptBldc->tEx.fDecel * ptBldc->tEx.fAccCvrt;  // acc * (2 * pi / 60 * Jm * Gx / Ktm)
  }
  else {
    ptBldc->tIn.fVelTar = ptBldc->tIn.fVelCmd;
    ptBldc->tIn.fAccAmp = 0;
  }

  return ptBldc->tIn.fVelTar;
}