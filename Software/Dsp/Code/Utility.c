#include "Controller.h"

/***************************************************************
 Initialize MC56F8357 peripheral registers
 ***************************************************************/
void DSP_Init(void)
{
  /* SIM registers */
  *((SnWord *)0xF358) = 0x8;              // disable JTAG pullups

  /* Initialize PWMA */
  ptPWMA->wCTL =     0b0000000000000000;

  ptPWMA->wFCTL =    0b0000000000000001;  // automatically clear fault A0
  ptPWMA->wFSA =     0b0000000000000000;
  ptPWMA->wOUT =     0b0011111100000000;  // disable output pads enable software mode w/PWM bits off
  ptPWMA->wCNT =     0x0000;
  ptPWMA->wCM =      1200;                // 25kHz for center alignment
  ptPWMA->wVAL[0] =  0;
  ptPWMA->wVAL[1] =  0;
  ptPWMA->wVAL[2] =  0;
  ptPWMA->wVAL[3] =  0;
  ptPWMA->wVAL[4] =  0;
  ptPWMA->wVAL[5] =  0;
  ptPWMA->wDEADTM =  0;
  ptPWMA->wDISMAP1 = 0b0001000100010001;  // enable fault A0 map for all pwm
  ptPWMA->wDISMAP2 = 0b0000000000010001;
  ptPWMA->wCFG =     0b0000000000001110;  // set center align independent output control
  ptPWMA->wCCR =     0b1000000000000000;  // allow enhanced hardware control writes
  ptPWMA->wCCR =     0b1000000000010000;  // enable PWM0 writes all
  ptPWMA->wPORT =    0b0000000000000000;
  ptPWMA->wICCR =    0b0000000000000000;
  
  ptPWMA->wCTL |= 2;                      // set LDOK to load in buffered values
  ptPWMA->wCTL |= 1;                      // set PWMEN to start PWM generator

  /* Initialize PWMB */
  ptPWMB->wCTL =     0b0000000000000000;

  ptPWMB->wFCTL =    0b0000000000000001;  // automatically clear fault B0
  ptPWMB->wFSA =     0b0000000000000000;
  ptPWMB->wOUT =     0b0011111100000000;  // disable output pads enable software mode w/PWM bits off
  ptPWMB->wCNT =     0x0000;
  ptPWMB->wCM =      1200;                // 25kHz for center alignment
  ptPWMB->wVAL[0] =  0;
  ptPWMB->wVAL[1] =  0;
  ptPWMB->wVAL[2] =  0;
  ptPWMB->wVAL[3] =  0;
  ptPWMB->wVAL[4] =  0;
  ptPWMB->wVAL[5] =  0;
  ptPWMB->wDEADTM =  0;
  ptPWMB->wDISMAP1 = 0b0001000100010001;  // enable fault B0 map for all pwm
  ptPWMB->wDISMAP2 = 0b0000000000010001;
  ptPWMB->wCFG =     0b0000000000001110;  // set center align independent output control
  ptPWMB->wCCR =     0b1000000000000000;  // allow enhanced hardware control writes
  ptPWMB->wCCR =     0b1000000000010000;  // enable PWM0 writes all
  ptPWMB->wPORT =    0b0000000000000000;
  ptPWMB->wICCR =    0b0000000000000000;
  
  ptPWMB->wCTL |= 2;                      // set LDOK to load in buffered values
  ptPWMB->wCTL |= 1;                      // set PWMEN to start PWM generator

  /* Initialize Port A Priority 2 Commutation Edge Trigger Timer A0 for capture interrupt */
  ptTMRA->tChan0.wCTRL =    0;            // stop TMRA0
  ptTMRA->tChan0.wCMP1 =    0;
  ptTMRA->tChan0.wCMP2 =    0;
  ptTMRA->tChan0.wCAP =     0;
  ptTMRA->tChan0.wLOAD =    0;
  ptTMRA->tChan0.wHOLD =    0;
  ptTMRA->tChan0.wCNTR =    0;
  ptTMRA->tChan0.wSCR =     0;
  ptTMRA->tChan0.wCMPLD1 =  0;
  ptTMRA->tChan0.wCMPLD2 =  0;
  ptTMRA->tChan0.wCOMSCR =  0;
  ptTMRA->tChan0.wCTRL =    0x3E00;       // count rising edges IPBus / 128

  /* Initialize Port A Priority 2 Commutation Edge Trigger Timer A1 for capture interrupt */
  ptTMRA->tChan1.wCTRL =    0;            // stop TMRA1
  ptTMRA->tChan1.wCMP1 =    0;
  ptTMRA->tChan1.wCMP2 =    0;
  ptTMRA->tChan1.wCAP =     0;
  ptTMRA->tChan1.wLOAD =    0;
  ptTMRA->tChan1.wHOLD =    0;
  ptTMRA->tChan1.wCNTR =    0;
  ptTMRA->tChan1.wSCR =     0;
  ptTMRA->tChan1.wCMPLD1 =  0;
  ptTMRA->tChan1.wCMPLD2 =  0;
  ptTMRA->tChan1.wCOMSCR =  0;
  ptTMRA->tChan1.wCTRL =    0x3E80;       // count rising edges IPBus / 128

  /* Initialize Port A Priority 2 Commutation Edge Trigger Timer A2 for capture interrupt */
  ptTMRA->tChan2.wCTRL =    0;            // stop TMRA2
  ptTMRA->tChan2.wCMP1 =    0;
  ptTMRA->tChan2.wCMP2 =    0;
  ptTMRA->tChan2.wCAP =     0;
  ptTMRA->tChan2.wLOAD =    0;
  ptTMRA->tChan2.wHOLD =    0;
  ptTMRA->tChan2.wCNTR =    0;
  ptTMRA->tChan2.wSCR =     0;
  ptTMRA->tChan2.wCMPLD1 =  0;
  ptTMRA->tChan2.wCMPLD2 =  0;
  ptTMRA->tChan2.wCOMSCR =  0;
  ptTMRA->tChan2.wCTRL =    0x3F00;       // count rising edges IPBus / 128

  /* Initialize Port A Priority 1 Timer A3 for 5 msec reloadable CMP1 interrupt */
  ptTMRA->tChan3.wCTRL =    0;            // stop TMRA3
  ptTMRA->tChan3.wCMP1 =    9375;
  ptTMRA->tChan3.wCMP2 =    0;
  ptTMRA->tChan3.wCAP =     0;
  ptTMRA->tChan3.wLOAD =    0;
  ptTMRA->tChan3.wHOLD =    0;
  ptTMRA->tChan3.wCNTR =    0;
  ptTMRA->tChan3.wSCR =     0;
  ptTMRA->tChan3.wCMPLD1 =  18749;        // reload on this value
  ptTMRA->tChan3.wCMPLD2 =  0;
  ptTMRA->tChan3.wCOMSCR =  0x41;         // enable reload interrupt on CMP1;
  ptTMRA->tChan3.wCTRL =    0x3820;       // 001 count rising clock edges
                                          // 1100 IPBus / 16
                                          // 00 external pin 0
                                          // 0 count repeatedly
                                          // 1 reinitialize on compare
                                          // 0 count up
                                          // 0 no co-channel reinitialization
                                          // 000 assert OFLAG

  /* Initialize Port B Priority 2 Commutation Edge Trigger Timer B0 for capture interrupt */
  ptTMRB->tChan0.wCTRL =    0;            // stop TMRB0
  ptTMRB->tChan0.wCMP1 =    0;
  ptTMRB->tChan0.wCMP2 =    0;
  ptTMRB->tChan0.wCAP =     0;
  ptTMRB->tChan0.wLOAD =    0;
  ptTMRB->tChan0.wHOLD =    0;
  ptTMRB->tChan0.wCNTR =    0;
  ptTMRB->tChan0.wSCR =     0;            // disable both edge compare interrupt
  ptTMRB->tChan0.wCMPLD1 =  0;
  ptTMRB->tChan0.wCMPLD2 =  0;
  ptTMRB->tChan0.wCOMSCR =  0;
  ptTMRB->tChan0.wCTRL =    0x3E00;       // count rising edges IPBus / 128

  /* Initialize Port B Priority 2 Commutation Edge Trigger Timer B1 for capture interrupt */
  ptTMRB->tChan1.wCTRL =    0;            // stop TMRB1
  ptTMRB->tChan1.wCMP1 =    0;
  ptTMRB->tChan1.wCMP2 =    0;
  ptTMRB->tChan1.wCAP =     0;
  ptTMRB->tChan1.wLOAD =    0;
  ptTMRB->tChan1.wHOLD =    0;
  ptTMRB->tChan1.wCNTR =    0;
  ptTMRB->tChan1.wSCR =     0;            // disable both edge compare interrupt
  ptTMRB->tChan1.wCMPLD1 =  0;
  ptTMRB->tChan1.wCMPLD2 =  0;
  ptTMRB->tChan1.wCOMSCR =  0;
  ptTMRB->tChan1.wCTRL =    0x3E80;       // count rising edges IPBus / 128

  /* Initialize Port B Priority 2 Commutation Edge Trigger Timer B2 for capture interrupt */
  ptTMRB->tChan2.wCTRL =    0;            // stop TMRB2
  ptTMRB->tChan2.wCMP1 =    0;
  ptTMRB->tChan2.wCMP2 =    0;
  ptTMRB->tChan2.wCAP =     0;
  ptTMRB->tChan2.wLOAD =    0;
  ptTMRB->tChan2.wHOLD =    0;
  ptTMRB->tChan2.wCNTR =    0;
  ptTMRB->tChan2.wSCR =     0;            // disable both edge compare interrupt
  ptTMRB->tChan2.wCMPLD1 =  0;
  ptTMRB->tChan2.wCMPLD2 =  0;
  ptTMRB->tChan2.wCOMSCR =  0;
  ptTMRB->tChan2.wCTRL =    0x3F00;       // count rising edges IPBus / 128

  /* Initialize Port B Priority 1 Timer B3 for 5 msec reloadable CMP1 interrupt */
  ptTMRB->tChan3.wCTRL =    0;            // stop TMRB3
  ptTMRB->tChan3.wCMP1 =    18749;
  ptTMRB->tChan3.wCMP2 =    0;
  ptTMRB->tChan3.wCAP =     0;
  ptTMRB->tChan3.wLOAD =    0;
  ptTMRB->tChan3.wHOLD =    0;
  ptTMRB->tChan3.wCNTR =    0;
  ptTMRB->tChan3.wSCR =     0;
  ptTMRB->tChan3.wCMPLD1 =  18749;        // reload CMP1 from this register
  ptTMRB->tChan3.wCMPLD2 =  0;
  ptTMRB->tChan3.wCOMSCR =  0x41;         // enable reload interrupt on CMP1
  ptTMRB->tChan3.wCTRL =    0x3820;       // 001 count rising clock edges
                                          // 1100 IPBus / 16
                                          // 00 external pin 0
                                          // 0 count repeatedly
                                          // 1 reinitialize on compare
                                          // 0 count up
                                          // 0 no co-channel reinitialization
                                          // 000 assert OFLAG

  /* Initialize Port A Timer C0 for faultA level PWM */
  ptTMRC->tChan0.wCTRL =    0;            // stop TMRC0
  ptTMRC->tChan0.wCMP1 =    0xF000;       // duty cycle (0x10000 - CMP1) / 0x10000
  ptTMRC->tChan0.wCMP2 =    0;
  ptTMRC->tChan0.wCAP =     0;
  ptTMRC->tChan0.wLOAD =    0;
  ptTMRC->tChan0.wHOLD =    0;
  ptTMRC->tChan0.wCNTR =    0;
  ptTMRC->tChan0.wSCR =     0x0001;       // output OFLAG to pin C0
  ptTMRC->tChan0.wCMPLD1 =  0;
  ptTMRC->tChan0.wCMPLD2 =  0;
  ptTMRC->tChan0.wCOMSCR =  0;
  ptTMRC->tChan0.wCTRL =    0x3006;       // 001 count rising clock edges
                                          // 1000 IPBus
                                          // 00 external pin 0
                                          // 0 count repeatedly
                                          // 0 no initialize on compare
                                          // 0 count up
                                          // 0 no co-channel reinitialization
                                          // 110 set OFLAG on compare clear on overflow

  /* Initialize Port B Timer C1 for faultB level PWM */
  ptTMRC->tChan1.wCTRL =    0;            // stop TMRC1
  ptTMRC->tChan1.wCMP1 =    0xF000;       // duty cycle (0x10000 - CMP1) / 0x10000
  ptTMRC->tChan1.wCMP2 =    0;
  ptTMRC->tChan1.wCAP =     0;
  ptTMRC->tChan1.wLOAD =    0;
  ptTMRC->tChan1.wHOLD =    0;
  ptTMRC->tChan1.wCNTR =    0;
  ptTMRC->tChan1.wSCR =     0x0001;       // output OFLAG to pin C1
  ptTMRC->tChan1.wCMPLD1 =  0;
  ptTMRC->tChan1.wCMPLD2 =  0;
  ptTMRC->tChan1.wCOMSCR =  0;
  ptTMRC->tChan1.wCTRL =    0x3086;       // 001 count rising clock edges
                                          // 1000 IPBus
                                          // 01 external pin 1
                                          // 0 count repeatedly
                                          // 0 no initialize on compare
                                          // 0 count up
                                          // 0 no co-channel reinitialization
                                          // 110 set OFLAG on compare clear on overflow

  /* Initialize Hallbus Priority 0 Timer D0 for 0.5 msec interrupt */
  ptTMRD->tChan0.wCTRL =    0;            // stop TMRD0
  ptTMRD->tChan0.wCMP1 =    1875;         // interrupt on this value
  ptTMRD->tChan0.wCMP2 =    0;
  ptTMRD->tChan0.wCAP =     0;
  ptTMRD->tChan0.wLOAD =    0;
  ptTMRD->tChan0.wHOLD =    0;
  ptTMRD->tChan0.wCNTR =    0;
  ptTMRD->tChan0.wSCR =     0x4000;       // enable compare interrupt
  ptTMRD->tChan0.wCMPLD1 =  0;
  ptTMRD->tChan0.wCMPLD2 =  0;
  ptTMRD->tChan0.wCOMSCR =  0;
  ptTMRD->tChan0.wCTRL =    0x3820;       // 001 count rising clock edges
                                          // 1100 IPBus / 16
                                          // 00 external pin 0
                                          // 0 count repeatedly
                                          // 1 reinitialize on compare
                                          // 0 count up
                                          // 0 no co-channel reinitialization
                                          // 000 assert OFLAG

  /* Initialize Timer D1 for 400us min Hall Tick */
  ptTMRD->tChan1.wCTRL =    0;            // stop TMRD1
  ptTMRD->tChan1.wCMP1 =    1500;         // stop on this value
  ptTMRD->tChan1.wCMP2 =    0;
  ptTMRD->tChan1.wCAP =     0;
  ptTMRD->tChan1.wLOAD =    0;
  ptTMRD->tChan1.wHOLD =    0;
  ptTMRD->tChan1.wCNTR =    0;
  ptTMRD->tChan1.wSCR =     0;
  ptTMRD->tChan1.wCMPLD1 =  0;
  ptTMRD->tChan1.wCMPLD2 =  0;
  ptTMRD->tChan1.wCOMSCR =  0;
  ptTMRD->tChan1.wCTRL =    0x3860;       // 001 count rising clock edges
                                          // 1100 IPBus / 16
                                          // 00 external pin 0
                                          // 1 count once
                                          // 1 reinitialize on compare
                                          // 0 count up
                                          // 0 no co-channel reinitialization
                                          // 000 assert OFLAG
  /* Initialize ADCA */
  ptADCA->wCR1 =   0b101;                 // triggered simultaneous mode
  ptADCA->wCR2 =   0b101;                 // 5 MHz sampling rate
  ptADCA->wLST1 =  0b0011001000010000;    // normal scan order 3 - 0
  ptADCA->wLST2 =  0b0111011001010100;    // normal scan order 7 - 4

  /* Initialize ADCB */
  ptADCB->wCR1 =   0b101;                 // triggered simultaneous mode
  ptADCB->wCR2 =   0b101;                 // 5 MHz sampling rate
  ptADCB->wLST1 =  0b0011001000010000;    // normal scan order 3 - 0
  ptADCB->wLST2 =  0b0111011001010100;    // normal scan order 7 - 4

  /* GPIOA Configuration */
  ptGPIOA->wPUR    = 0x01FF;              // pull-ups enabled if input
  ptGPIOA->wDR     = 0x2A40;              // data register
  ptGPIOA->wDDR    = 0x3EF0;              // data direction
  ptGPIOA->wPER    = 0x0000;              // peripheral mode enable mode
  ptGPIOA->wIAR    = 0x0000;              // no software interrupt assert
  ptGPIOA->wIENR   = 0x0100;              // enable GPIOA8 interrupt
  ptGPIOA->wIPOLR  = 0x0100;              // GPIOA8 active low edge
  ptGPIOA->wIPR    = 0x0000;              // interrupt detect flag
  ptGPIOA->wIESR   = 0x0000;              // edge detect flag
  ptGPIOA->wPPMODE = 0x3EF0;              // push/pull output enable

  /* GPIOB Configuration */
  ptGPIOB->wPUR    = 0x003F;              // pull-ups enabled if input
  ptGPIOB->wDR     = 0x00C0;              // data register
  ptGPIOB->wDDR    = 0x00C0;              // data direction
  ptGPIOB->wPER    = 0x0000;              // peripheral mode enable mode
  ptGPIOB->wIAR    = 0x0000;              // no software interrupt assert
  ptGPIOB->wIENR   = 0x0000;              // disable interrupt
  ptGPIOB->wIPOLR  = 0x0000;              // active high edge
  ptGPIOB->wIPR    = 0x0000;              // interrupt detect flag
  ptGPIOB->wIESR   = 0x0000;              // edge detect flag
  ptGPIOB->wPPMODE = 0x00C0;              // push/pull output enable

  /* GPIOC Configuration */
  ptGPIOC->wPUR    = 0x04FF;              // pull-ups enabled if input
  ptGPIOC->wDR     = 0x0000;              // data register
  ptGPIOC->wDDR    = 0x0300;              // data direction
  ptGPIOC->wPER    = 0x0077;              // peripheral mode enable mode
  ptGPIOC->wIAR    = 0x0000;              // no software interrupt assert
  ptGPIOC->wIENR   = 0x0000;              // disable interrupt
  ptGPIOC->wIPOLR  = 0x0000;              // active high edge
  ptGPIOC->wIPR    = 0x0000;              // interrupt detect flag
  ptGPIOC->wIESR   = 0x0000;              // edge detect flag
  ptGPIOC->wPPMODE = 0x0300;              // push/pull output enable

  /* GPIOD Configuration */
  ptGPIOD->wPUR    = 0x0F8F;              // pull-ups enabled if input
  ptGPIOD->wDR     = 0x0020;              // data register
  ptGPIOD->wDDR    = 0x1870;              // data direction
  ptGPIOD->wPER    = 0x00C0;              // peripheral mode enable mode
  ptGPIOD->wIAR    = 0x0000;              // no software interrupt assert
  ptGPIOD->wIENR   = 0x0000;              // disable interrupt
  ptGPIOD->wIPOLR  = 0x0000;              // active high edge
  ptGPIOD->wIPR    = 0x0000;              // interrupt detect flag
  ptGPIOD->wIESR   = 0x0000;              // edge detect flag
  ptGPIOD->wPPMODE = 0x1870;              // push/pull output enable

  /* GPIOE Configuration */
  ptGPIOE->wPUR    = 0x3CDE;              // pull-ups enabled if input
  ptGPIOE->wDR     = 0x0000;              // data register
  ptGPIOE->wDDR    = 0x0341;              // data direction
  ptGPIOE->wPER    = 0x03F3;              // peripheral mode enable mode
  ptGPIOE->wIAR    = 0x0000;              // no software interrupt assert
  ptGPIOE->wIENR   = 0x0000;              // disable interrupt
  ptGPIOE->wIPOLR  = 0x0000;              // active high edge
  ptGPIOE->wIPR    = 0x0000;              // interrupt detect flag
  ptGPIOE->wIESR   = 0x0000;              // edge detect flag
  ptGPIOE->wPPMODE = 0x0341;              // push/pull output enable

  /* GPIOF Configuration */
  ptGPIOF->wPUR    = 0xFFFF;              // pull-ups enabled if input
  ptGPIOF->wDR     = 0x0000;              // data register
  ptGPIOF->wDDR    = 0x0000;              // data direction
  ptGPIOF->wPER    = 0x0000;              // peripheral mode enable mode
  ptGPIOF->wIAR    = 0x0000;              // no software interrupt assert
  ptGPIOF->wIENR   = 0x0000;              // disable interrupt
  ptGPIOF->wIPOLR  = 0x0000;              // active high edge
  ptGPIOF->wIPR    = 0x0000;              // interrupt detect flag
  ptGPIOF->wIESR   = 0x0000;              // edge detect flag
  ptGPIOF->wPPMODE = 0x0000;              // push/pull output enable

  /* Initialize internal temperature sense diode */
  wTSENSE |= 1;                           // turn on temperature sensor
  
  /* Initialize SCI */
  SerialInit();

  /* Initialize FlexCan (derived from Processor Expert) */
  FlexCanInit();

  /* Initialize interrupt priorities */
  ptITCN->wIPR[4] |= 0x0010;              // GPIOA Level 0
  ptITCN->wIPR[5] |= 0x0F30;              // SCI1 (RCV, RERR, TIDL) level 2
  ptITCN->wIPR[6] |= 0x0040;              // TMRD0 level 0
  // Level 2 interrupts must be turned on after passing short circuit tests
  ptITCN->wIPR[7] |= 0x2000;              // TMRB3 level 1
  ptITCN->wIPR[8] |= 0xF320;              // SCI0 (RCV, RERR, TIDL) level 2 - TMRA3 level 1

  /* Allocate space for brushless motor trapezoidal move profile */

  /* Setup COP watchdog */
  ptCOP->wCOPTO   = 0x3D08;               // set timeout to 2 seconds with 8MHz oscillator
  ptCOP->wCOPCTL  = 0x0002;               // enable
  ptCOP->wCOPCTL  = 0x0003;               // write protect to lock configuration

  /* Enable all interrupt priority levels in core status register */
  asm { bfclr #0x300,SR }
}


/***************************************************************
 Update when control parameters change
 ***************************************************************/
void OnChange(void)
{
  static float fCurrentLoopRateA, fCurrentLoopRateB;

  /* Update port configuration per wPortType */
  ConfigureMotorPorts();
    
  /* Load motor position profiles from main application */
  g_tVarArray.tBldcA.tEx.wFault |= GetPositionProfile(&g_tVarArray.tBldcA);
  g_tVarArray.tBldcB.tEx.wFault |= GetPositionProfile(&g_tVarArray.tBldcB);
  
  /* Update BLDC control loop timers */
  if (g_tVarArray.tBldcA.tEx.fLoopRate != fCurrentLoopRateA) {
    fCurrentLoopRateA = g_tVarArray.tBldcA.tEx.fLoopRate;
    ptTMRA->tChan3.wCMPLD1 = fCurrentLoopRateA * 3.75E6 - 1;
  }
  if (g_tVarArray.tBldcB.tEx.fLoopRate != fCurrentLoopRateB) {
    fCurrentLoopRateB = g_tVarArray.tBldcB.tEx.fLoopRate;
    ptTMRB->tChan3.wCMPLD1 = fCurrentLoopRateB * 3.75E6 - 1;
  }

  /* External Beep access */
  if (g_tVarArray.wAuxillary & 1)
    BeepON;
  else
    BeepOFF;
}
  

/***************************************************************
 Calculate ancillary analog values
 ***************************************************************/
void UpdateAnalog(void)
{
  float V;

  /* Calculate DSP temperature data for VarArray  V = 7.45e-3T + 1.44 {-40 to 150C} */
  V = g_tVarArray.tAnalog.sAverage[1] * 805.861E-6;             // 805.861E-6 = 3.3 / 4095
  g_tVarArray.tTemperature.fDspTemp = (V - 1.44) / 7.45e-3;

  /* Handle over temperature condition */
  if (g_tVarArray.tTemperature.fDspTemp > g_tVarArray.tTemperature.fDspLimit)
    g_tVarArray.tTemperature.wEvent |= ERROR_DSP_TEMP;

  /* Calculate LM20 temperature data for VarArray  V = -11.67e-3T + 1.8583 {-40 to 85C} */
  V = g_tVarArray.tAnalog.sAverage[2] * 805.861E-6;            // 805.861E-6 = 3.3 / 4095
  g_tVarArray.tTemperature.fOnBoardTemp = (V - 1.8583) / -11.67e-3;

  /* Handle over temperature condition */
  if (g_tVarArray.tTemperature.fOnBoardTemp > g_tVarArray.tTemperature.fOnBoardHiLimit)
    g_tVarArray.tTemperature.wEvent |= ERROR_HIGH_TEMP;

  /* Handle under temperature condition */
  if (g_tVarArray.tTemperature.fOnBoardTemp < g_tVarArray.tTemperature.fOnBoardLoLimit)
    g_tVarArray.tTemperature.wEvent |= ERROR_LOW_TEMP;
}

static float fFloatCov[16] =  {
     1.0f, 1e-1f, 1e-2f, 1e-3f, 1e-4f, 1e-5f, 1e-6f, 1e-7f,
    1e-8f, 1e9f, 1e-10f, 1e-11f, 1e-12f, 1e-13f, 1e-14f, 1e-15f
};

#define W_TO_FLOAT(w) ((w & 0x0fff) * fFloatCov[w >> 12])

void LoadXfrMotorTable(DIIMotorTblXfr *ptXfr, External *ptEx)
{
  static SnWord wFlowTable[8] = {
    0x3F00, 0xBE20, 0xBB02, 0xBB20, 0xAF08, 0xBE08, 0xAF02, 0x3F00, 
  };
  static SnWord wJogTable[6] = {
    4, 6, 2, 3, 1, 5
  };
  SnWord wOffs;
  
  for (wOffs = 0; wOffs < 8; wOffs++) {
    if (ptXfr->bConfig & MT_CONF_INV_FLOW) {
      ptEx->wFCommTable[7-wOffs] = wFlowTable[wOffs];
      ptEx->wRCommTable[wOffs] = wFlowTable[wOffs];
    } else {
      ptEx->wFCommTable[wOffs] = wFlowTable[wOffs];
      ptEx->wRCommTable[7-wOffs] = wFlowTable[wOffs];
    }
  }

  ptEx->wFDirTable[0] = ptEx->wFDirTable[7] = 0;
  ptEx->wRDirTable[0] = ptEx->wRDirTable[7] = 0;
  for (wOffs = 0; wOffs < 6; wOffs++) {
    SnWord wPrevJog = (wOffs == 0) ? wJogTable[5] : wJogTable[wOffs-1];
    if (ptXfr->bConfig & MT_CONF_INV_HALL) {
      ptEx->wJogTable[5-wOffs] = wJogTable[wOffs];
      ptEx->wFDirTable[wPrevJog] = wJogTable[wOffs];
      ptEx->wRDirTable[wJogTable[wOffs]] = wPrevJog;
    } else {
      ptEx->wJogTable[wOffs] = wJogTable[wOffs];
      ptEx->wFDirTable[wJogTable[wOffs]] = wPrevJog;
      ptEx->wRDirTable[wPrevJog] = wJogTable[wOffs];
    }
  }
  
  ptEx->fResistance = W_TO_FLOAT(ptXfr->wResistance);
  ptEx->fAccCvrt = W_TO_FLOAT(ptXfr->wAccCvrt);
  ptEx->wTacPerRevN = ptXfr->wTacPerRevN;
  ptEx->wTacPerRevD = ptXfr->bTacPerRevD;
  ptEx->fAccel = (float)ptXfr->wAccDecel;
  ptEx->fDecel = -(float)ptXfr->wAccDecel;
  ptEx->sVelMax = ptXfr->wVelMax;
  ptEx->fStopTime = (ptXfr->bConfig & MT_CONF_STOP_TIME) ? 0.5f : 0.25f;
  
  ptEx->fLoopRate = ptXfr->bLoopPeriod * 0.001f;
  ptEx->fPWMFreq = ptXfr->bPWMFreq;
  
  ptEx->tKvel.fKf = 0.0f;
  ptEx->tKvel.fKp = W_TO_FLOAT(ptXfr->wVelKp);
  ptEx->tKvel.fKi = W_TO_FLOAT(ptXfr->wVelKi);
  ptEx->tKvel.fKd = W_TO_FLOAT(ptXfr->wVelKd);
  ptEx->tKvel.fKa = (ptXfr->bConfig & MT_CONF_K_ACCEL) ? 1.0f : 0.0f;
  ptEx->tKvel.fKl = 0.0f;

  ptEx->tKpos.fKf = 0.0f;
  ptEx->tKpos.fKp = W_TO_FLOAT(ptXfr->wPosKp);
  ptEx->tKpos.fKi = W_TO_FLOAT(ptXfr->wPosKi);
  ptEx->tKpos.fKd = W_TO_FLOAT(ptXfr->wPosKd);
  ptEx->tKpos.fKa = 0.0f;
  ptEx->tKpos.fKl = 0.0f;
  
  ptEx->fFrpmA = 0.0f;
  ptEx->wFrpmB = 0;
  ptEx->fTlimA = W_TO_FLOAT(ptXfr->wTlimA);
  ptEx->wTlimB = ptXfr->wTlimB;
  ptEx->fIlim = W_TO_FLOAT(ptXfr->wIlim);
  ptEx->fIhigh = W_TO_FLOAT(ptXfr->wIhigh);
  
  // external run-time application write and/or read user variables
  ptEx->wMode = 0;
  ptEx->sVelSet = 1000;
  ptEx->wDwell = 80;
  ptEx->fCycleTime = 0.10f;
  ptEx->wProfileCmd = 0x3000;
  ptEx->wAssert = 0;
  ptEx->wFault = 0;
}

/***************************************************************
 Initialize motor ports A & B per control variable PortType
 ***************************************************************/
void ConfigureMotorPorts(void)
{
  /* Configure port A for type of motor */
  if (g_tVarArray.wPortType & 0x0080) {
    if (g_yNewXfrMotorTblA) {
      LoadXfrMotorTable(&g_tXfrMotorTblA, &g_tVarArray.tBldcA.tEx);
      g_yNewXfrMotorTblA = FALSE;
    }

    g_tVarArray.tBldcA.tIn = (Internal){0}; // clear internal variables
    g_tVarArray.tBldcA.tIn.fTacPerRev = (float)g_tVarArray.tBldcA.tEx.wTacPerRevN / (float)g_tVarArray.tBldcA.tEx.wTacPerRevD;
    g_tVarArray.tBldcA.tIn.sWinLock = 0;
    g_tVarArray.tBldcA.tIn.tStop.Count = 1;
    g_tVarArray.tBldcA.tIn.tStop.P[1].X = 1.0;
    g_tVarArray.tBldcA.tIn.wProIndex = -1;

    switch (g_tVarArray.wPortType & 0x007F) {

      /* Port off */
      default:
      case 0:
        ptTMRA->tChan0.wSCR &= ~0x14C0;     // disable OF & both edge compare interrupt
        ptTMRA->tChan1.wSCR &= ~0x04C0;     // disable both edge compare interrupt
        ptTMRA->tChan2.wSCR &= ~0x04C0;     // disable both edge compare interrupt
        break;

      /* Brushless configuration */
      case 2:
        ptTMRA->tChan0.wSCR |=  0x14C0;     // enable OF & both edge compare interrupt
        ptTMRA->tChan1.wSCR |=  0x04C0;     // enable both edge compare interrupt
        ptTMRA->tChan2.wSCR |=  0x04C0;     // enable both edge compare interrupt

        /* Set BLDC current limits */
        ptTMRC->tChan0.wCMP1 = 65536 - g_tVarArray.tBldcA.tEx.fIlim * ILIM_CONVERT;  // duty cycle (0x10000 - CMP1) / 0x10000

        /*
         * (60000000Hz / (fPWMFreq * 1000Hz)) or (60000 / fPWMFreq) which is then 
         * divided by 2 for center aligned so (60000 / fPWMFreq / 2) or
         * (30000 / fPWMFreq).
         */
        ptPWMA->wCM = (SnWord)(30000.0f / g_tVarArray.tBldcA.tEx.fPWMFreq);
        break;
    } 
    ptPWMA->wOUT = FETS_OFF;                // turn off PWM
    ptPWMA->wCTL |= 2;                      // set LDOK to load in buffered values
    g_tVarArray.wPortType &= ~0x0080;       // clear start/busy flag
  }

  /* Configure port B for type of motor */
  if (g_tVarArray.wPortType & 0x8000) {
    if (g_yNewXfrMotorTblB) {
      LoadXfrMotorTable(&g_tXfrMotorTblB, &g_tVarArray.tBldcB.tEx);
      g_yNewXfrMotorTblB = FALSE;
    }

    g_tVarArray.tBldcB.tIn = (Internal){0}; // clear and initialize internal variables
    g_tVarArray.tBldcB.tIn.fTacPerRev = (float)g_tVarArray.tBldcB.tEx.wTacPerRevN / (float)g_tVarArray.tBldcB.tEx.wTacPerRevD;
    g_tVarArray.tBldcB.tIn.sWinLock = 0;
    g_tVarArray.tBldcB.tIn.tStop.Count = 1;
    g_tVarArray.tBldcB.tIn.tStop.P[1].X = 1.0;
    g_tVarArray.tBldcB.tIn.wProIndex = -1;

    switch (g_tVarArray.wPortType & 0x7F00) {

      /* Port off */
      default:
      case 0x0000:
        ptTMRB->tChan0.wSCR &= ~0x14C0;     // disable OF & both edge compare interrupt
        ptTMRB->tChan1.wSCR &= ~0x04C0;     // disable both edge compare interrupt
        ptTMRB->tChan2.wSCR &= ~0x04C0;     // disable both edge compare interrupt
        break;

      /* Brushless configuration */
      case 0x0200:
        ptTMRB->tChan0.wSCR |=  0x14C0;     // enable OF & both edge compare interrupt
        ptTMRB->tChan1.wSCR |=  0x04C0;     // enable both edge compare interrupt
        ptTMRB->tChan2.wSCR |=  0x04C0;     // enable both edge compare interrupt

        /* Set BLDC current limits */
        ptTMRC->tChan1.wCMP1 = 65536 - g_tVarArray.tBldcB.tEx.fIlim * ILIM_CONVERT;  // duty cycle (0x10000 - CMP1) / 0x10000

        /*
         * (60000000Hz / (fPWMFreq * 1000Hz)) or (60000 / fPWMFreq) which is then 
         * divided by 2 for center aligned so (60000 / fPWMFreq / 2) or
         * (30000 / fPWMFreq).
         */
        ptPWMB->wCM = (SnWord)(30000.0f / g_tVarArray.tBldcB.tEx.fPWMFreq);
        break;
    }
    ptPWMB->wOUT = FETS_OFF;                // turn off PWM
    ptPWMB->wCTL |= 2;                      // set LDOK to load in buffered values
    g_tVarArray.wPortType &= ~0x8000;       // clear start/busy flag
  }
}