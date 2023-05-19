#include "Controller.h"

/******************************************************
 Read mask invert debounce and process active port data
 ******************************************************/
#define RETRY_LIMIT 20

typedef enum {
	DIGITAL_INPUT_A,
	DIGITAL_INPUT_B,
	DIGITAL_INPUT_C,
	DIGITAL_INPUT_D,
	DIGITAL_INPUT_E,
	DIGITAL_INPUT_F
} DigitalInput;

/*
  Digital inputs are intended to be read from the main program loop and can be debounced
  by requiring up to 65535 consecutive same values before being passed to the final stage
  for configuration and subsequent use. Debounce is threaded into the main process in chunks
  of 500 tight loops (500 KHz in about 1 msec) up to the number specified in wDebounce[].
*/

static SnWord ReadPortData(DigitalPort *ptPort, DigitalInput wIndex)
{
  SnWord ReturnCode = 0, wNumRetry = 0, w, i;
  static SnWord wDebounceCtr[6];

  /* Debounce specified number of times */
  for (i = 0; i < 500; i++) {
    if (++wDebounceCtr[wIndex] < g_tVarArray.tDigital.wDebounce[wIndex] && wNumRetry < RETRY_LIMIT) {
      if ((w = ((ptPort->wDR ^ g_tVarArray.tDigital.wActiveLow[wIndex]) & g_tVarArray.tDigital.wActive[wIndex])) != g_tVarArray.tDigital.wNewData[wIndex]) {
        g_tVarArray.tDigital.wNewData[wIndex] = w;
        wDebounceCtr[wIndex] = 0;

        /* Set function return code for debounce error for any of the 16 bits */
        if (++wNumRetry >= RETRY_LIMIT)
          ReturnCode |= ERROR_DIGITAL_DEBOUNCE;
      }
    }
    else
      break;
  }

  /* StateData contains debounced momentary and push-on / push-off digital state data */
  if (wDebounceCtr[wIndex] >= g_tVarArray.tDigital.wDebounce[wIndex]) {
    wDebounceCtr[wIndex] = 0;
    g_tVarArray.tDigital.wStateData[wIndex] = ((g_tVarArray.tDigital.wNewData[wIndex]
                                   ^      g_tVarArray.tDigital.wOldData[wIndex])
                                   &      g_tVarArray.tDigital.wNewData[wIndex]
                                   ^ (w = g_tVarArray.tDigital.wStateData[wIndex]))
                                   &      g_tVarArray.tDigital.wInputType[wIndex]
                                   |     (g_tVarArray.tDigital.wNewData[wIndex]
                                   &     ~g_tVarArray.tDigital.wInputType[wIndex]);

    g_tVarArray.tDigital.wOldData[wIndex] = g_tVarArray.tDigital.wNewData[wIndex];

    /* Set function return code for ON_CHANGE event for any of the 16 bits */
    if ((g_tVarArray.tDigital.wStateData[wIndex] ^ w) & g_tVarArray.tDigital.wAssert[wIndex] & g_tVarArray.tInterrupt.wEnable)
      ReturnCode |= INTERRUPT_DIGITAL;
  }

  return ReturnCode;
}


/******************************************************
 Read digital inputs to g_tVarArray one port at a time
 ******************************************************/
void GetDigitalInput(void)
{
  static DigitalInput eIndex;

  switch (eIndex) {
    case DIGITAL_INPUT_A:
      g_tVarArray.tDigital.wEvent |= ReadPortData(ptGPIOA, DIGITAL_INPUT_A);
      break;
    case DIGITAL_INPUT_B:
      g_tVarArray.tDigital.wEvent |= ReadPortData(ptGPIOB, DIGITAL_INPUT_B) << 1;
      break;
    case DIGITAL_INPUT_C:
      g_tVarArray.tDigital.wEvent |= ReadPortData(ptGPIOC, DIGITAL_INPUT_C) << 2;
      break;
    case DIGITAL_INPUT_D:
      g_tVarArray.tDigital.wEvent |= ReadPortData(ptGPIOD, DIGITAL_INPUT_D) << 3;
      break;
    case DIGITAL_INPUT_E:
      g_tVarArray.tDigital.wEvent |= ReadPortData(ptGPIOE, DIGITAL_INPUT_E) << 4;
      break;
    case DIGITAL_INPUT_F:
      g_tVarArray.tDigital.wEvent |= ReadPortData(ptGPIOF, DIGITAL_INPUT_F) << 5;
  }
  if(++eIndex > DIGITAL_INPUT_F) eIndex = DIGITAL_INPUT_A;
}