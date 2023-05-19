#include "Controller.h"

#pragma interrupt alignsp
void UnexpectedInterrupt(void)
{
  asm(DEBUGHLT);                       /* Halt the core and placing it in the debug processing state */
}

#pragma interrupt alignsp
void COPReset(void)
{
  asm(DEBUGHLT);                       /* Halt the core and placing it in the debug processing state */
}

#pragma interrupt alignsp
void IllegalInstruction(void)
{
  asm(DEBUGHLT);                       /* Halt the core and placing it in the debug processing state */
}

#pragma interrupt alignsp
void StackOverflow(void)
{
  asm(DEBUGHLT);                       /* Halt the core and placing it in the debug processing state */
}

#pragma interrupt alignsp
void MisalignedAccess(void)
{
  asm(DEBUGHLT);                       /* Halt the core and placing it in the debug processing state */
}

#pragma define_section interrupt_vectors "interrupt_vectors.text"  RX
#pragma section interrupt_vectors begin
static asm void _vect(void) {
  JMP  init_MC56F835x_                 /* Interrupt no. 0 (Used) - ivINT_Reset */
  JMP  COPReset                        /* Interrupt no. 1 (Used) - ivINT_COPReset  */
  JSR  IllegalInstruction              /* Interrupt no. 2 (Unused) - ivINT_Illegal_Instruction  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 3 (Unused) - ivINT_SW3  */
  JSR  StackOverflow                   /* Interrupt no. 4 (Unused) - ivINT_HWStackOverflow  */
  JSR  MisalignedAccess                /* Interrupt no. 5 (Unused) - ivINT_MisalignedLongWordAccess  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 6 (Unused) - ivINT_OnCE_StepCounter  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 7 (Unused) - ivINT_OnCE_BU0  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 8 (Unused) - ivReserved0  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 9 (Unused) - ivINT_OnCE_TraceBuffer  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 10 (Unused) - ivINT_OnCE_TxREmpty  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 11 (Unused) - ivINT_OnCE_RxRFull  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 12 (Unused) - ivReserved1  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 13 (Unused) - ivReserved2  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 14 (Unused) - ivINT_SW2  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 15 (Unused) - ivINT_SW1  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 16 (Unused) - ivINT_SW0  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 17 (Unused) - ivINT_IRQA  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 18 (Unused) - ivINT_IRQB  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 19 (Unused) - ivReserved3  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 20 (Unused) - ivINT_LVI  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 21 (Unused) - ivINT_PLL  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 22 (Unused) - ivINT_HFM_ERR  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 23 (Unused) - ivINT_HFM_CC  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 24 (Unused) - ivINT_HFM_CBE  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 25 (Unused) - ivReserved4  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 26 (Unused) - ivINT_FlexCAN_BusOff  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 27 (Unused) - ivINT_FlexCAN_Error  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 28 (Unused) - ivINT_FlexCAN_WakeUp  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 29 (Unused) - ivINT_FlexCAN_MB  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 30 (Unused) - ivINT_GPIO_F  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 31 (Unused) - ivINT_GPIO_E  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 32 (Unused) - ivINT_GPIO_D  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 33 (Unused) - ivINT_GPIO_C  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 34 (Unused) - ivINT_GPIO_B  */
  JSR  Msg_Interrupt                   /* Interrupt no. 35 (Used) - ivINT_GPIO_A  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 36 (Unused) - ivReserved5  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 37 (Unused) - ivReserved6  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 38 (Unused) - ivINT_SPI1_RxFull  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 39 (Unused) - ivINT_SPI1_TxEmpty  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 40 (Unused) - ivINT_SPI0_RxFull  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 41 (Unused) - ivINT_SPI0_TxEmpty  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 42 (Unused) - ivINT_SCI1_TxEmpty  */
  JSR  Sci1_Transmit_Idle_Interrupt    /* Interrupt no. 43 (Used) - ivINT_SCI1_TxIdle  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 44 (Unused) - ivReserved7  */
  JSR  Sci1_Receive_Error_Interrupt    /* Interrupt no. 45 (Used) - ivINT_SCI1_RxError  */
  JSR  Sci1_Receive_Full_Interrupt     /* Interrupt no. 46 (Used) - ivINT_SCI1_RxFull  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 47 (Unused) - ivINT_DEC1_Home_Watchdog  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 48 (Unused) - ivINT_DEC1_Index  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 49 (Unused) - ivINT_DEC0_Home_Watchdog  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 50 (Unused) - ivINT_DEC0_Index  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 51 (Unused) - ivReserved8  */
  JSR  HallBus_Timer_Interrupt         /* Interrupt no. 52 (Used) - ivINT_TMRD0  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 53 (Unused) - ivINT_TMRD1  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 54 (Unused) - ivINT_TMRD2  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 55 (Unused) - ivINT_TMRD3  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 56 (Unused) - ivINT_TMRC0  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 57 (Unused) - ivINT_TMRC1  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 58 (Unused) - ivINT_TMRC2  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 59 (Unused) - ivINT_TMRC3  */
  JSR  TacB0_Interrupt                 /* Interrupt no. 60 (Used) - ivINT_TMRB0  */
  JSR  TacB1_Interrupt                 /* Interrupt no. 61 (Used) - ivINT_TMRB1  */
  JSR  TacB2_Interrupt                 /* Interrupt no. 62 (Used) - ivINT_TMRB2  */
  JSR  PortB_Timer_Interrupt           /* Interrupt no. 63 (Used) - ivINT_TMRB3  */
  JSR  TacA0_Interrupt                 /* Interrupt no. 64 (Used) - ivINT_TMRA0  */
  JSR  TacA1_Interrupt                 /* Interrupt no. 65 (Used) - ivINT_TMRA1  */
  JSR  TacA2_Interrupt                 /* Interrupt no. 66 (Used) - ivINT_TMRA2  */
  JSR  PortA_Timer_Interrupt           /* Interrupt no. 67 (Used) - ivINT_TMRA3  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 68 (Unused) - ivINT_SCI0_TxEmpty  */
  JSR  Sci0_Transmit_Idle_Interrupt    /* Interrupt no. 69 (Used) - ivINT_SCI0_TxIdle  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 70 (Unused) - ivReserved9  */
  JSR  Sci0_Receive_Error_Interrupt    /* Interrupt no. 71 (Used) - ivINT_SCI0_RxError  */
  JSR  Sci0_Receive_Full_Interrupt     /* Interrupt no. 72 (Used) - ivINT_SCI0_RxFull  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 73 (Unused) - ivINT_ADCB_Complete  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 74 (Unused) - ivINT_ADCA_Complete  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 75 (Unused) - ivINT_ADCB_ZC_LE  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 76 (Unused) - ivINT_ADCA_ZC_LE  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 77 (Unused) - ivINT_PWMB_Reload  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 78 (Unused) - ivINT_PWMA_Reload  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 79 (Unused) - ivINT_PWMB_Fault  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 80 (Unused) - ivINT_PWMA_Fault  */
  JSR  UnexpectedInterrupt             /* Interrupt no. 81 (Unused) - ivINT_LP  */
}
#pragma section interrupt_vectors end
