#include "Controller.h"

/******************************************************
 Read 8 ADCs and store in sData. Result is 12 bit
 value x 8 called by GetAnalogInput 
 ******************************************************/
void ReadADC(ADC *ptADC, SnSWord wADCbuffer[])
{
  SnWord w, wCnt = 50000;                               // set time out for 50000 decrements
  volatile SnWord *pwADC = &ptADC->wRSLT[0];            // set pointer to base of ADCA results

  /* Read ADC bank */
  while (ptADC->wSTAT & 0x8000 && wCnt--) ;             // wait for end of scan CIP == 0

  if (wCnt) {
    for (w = 0; w < 8; ++w)
      wADCbuffer[w] = *pwADC++ >> 3;                    // copy shifted results to buffer
  }
  else
    g_tVarArray.tInterrupt.wFlags |= INTERRUPT_ADC_EOC; // set ADC failure to convert fault
}


/******************************************************
 Puts formatted analog data in g_tVarArray
 ADCs are read during the HallBus 0.5 ms timer interrupt
 ******************************************************/
void GetAnalogInput(void)
{
  SnSWord w, x;
  static SnSQByte lSum[16];
  static SnWord wCnt[16];

  ptADCA->wCR1 |= 0x2000;                               // start scan ADCA START = 1
  ptADCB->wCR1 |= 0x2000;                               // start scan ADCB START = 1
  ReadADC(ptADCA, g_tVarArray.tAnalog.sData);           // read ANA
  ReadADC(ptADCB, g_tVarArray.tAnalog.sData + 8);       // read ANB
  
  /* Scale and average ADCs */
  for (w = 0; w < 16; ++w) {
    switch (g_tVarArray.tAnalog.wCount[w]) {
  	  case 0:                                           // ADC is unused (1 us)
  	    x = 0;
  	    break;

  	  case 1:                                           // 1 sample no averaging (10us)
        x = (g_tVarArray.tAnalog.wInvert[w] ?
          g_tVarArray.tAnalog.sOffset[w] - g_tVarArray.tAnalog.sData[w] :
          g_tVarArray.tAnalog.sData[w] - g_tVarArray.tAnalog.sOffset[w]) * g_tVarArray.tAnalog.fGain[w];
        break;

  	  default:                                          // average result (1 or 15 us)
        lSum[w] += g_tVarArray.tAnalog.sData[w];
        if (++wCnt[w] >= g_tVarArray.tAnalog.wCount[w]) {
          x = (g_tVarArray.tAnalog.wInvert[w] ?
            g_tVarArray.tAnalog.sOffset[w] - lSum[w] / wCnt[w] :
            lSum[w] / wCnt[w] - g_tVarArray.tAnalog.sOffset[w]) * g_tVarArray.tAnalog.fGain[w];
          lSum[w] = wCnt[w] = 0;
        }
        else
          x = g_tVarArray.tAnalog.sAverage[w];
        break;
    }
    /* Derivative trigger */
    if (abs(x - g_tVarArray.tAnalog.sAverage[w]) > g_tVarArray.tAnalog.wAssert[w])
      g_tVarArray.tInterrupt.wFlags |= g_tVarArray.tAnalog.wAssert[w] ? INTERRUPT_ANALOG : 0;

    /* Save result */
    g_tVarArray.tAnalog.sAverage[w] = x;
  }
}