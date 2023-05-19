/* Reference MC56F8357.pdf */
#ifndef MC56F8357_H
#define MC56F8357_H

#include "SnTypes.h"

/* Define general purpose I/O port structures */
typedef struct {
  SnWord wPUR;       // RW pull-up enabled by 1 (default) in input mode
  SnWord wDR;        // RW data register used by mcu (0 default)
  SnWord wDDR;       // RW data direction 0 input (default) 1 output
  SnWord wPER;       // RW peripheral controls port 1 (default) wDDR does not
  SnWord wIAR;       // RW interrupt assert 0 no interrupt (default)
  SnWord wIENR;      // RW interrupt enable 0 disabled (default)
  SnWord wIPOLR;     // RW interrupt polarity 0 high (default)
  SnWord wIPR;       // RO interrupt pending 1 (0 default)
  SnWord wIESR;      // RW interrupt edge sensitive 1 indicates edge (0 default)
  SnWord wPPMODE;    // RW push-pull enabled 1 (default)
  SnWord wRAWDATA;   // RO unstable data
} volatile DigitalPort;
/* Define GPIO base addresses */
#define ptGPIOA ((DigitalPort*)0x00F2E0)
#define ptGPIOB ((DigitalPort*)0x00F300)
#define ptGPIOC ((DigitalPort*)0x00F310)
#define ptGPIOD ((DigitalPort*)0x00F320)
#define ptGPIOE ((DigitalPort*)0x00F330)
#define ptGPIOF ((DigitalPort*)0x00F340)
/* End general purpose I/O port structures */

/* Define PWM structures */
typedef struct {
  SnWord wCTL;       // RW control register
  SnWord wFCTL;      // RW fault control register
  SnWord wFSA;       // RW fault status and acknowledge register
  SnWord wOUT;       // RW output control register
  SnWord wCNT;       // RO count  register
  SnWord wCM;        // RW counter modulo register
  SnWord wVAL[6];    // RW pwm value register 0 - 5
  SnWord wDEADTM;    // RW deadtime register
  SnWord wDISMAP1;   // RW disable fault mapping register 1
  SnWord wDISMAP2;   // RW disable fault mapping register 2
  SnWord wCFG;       // configure resister
  SnWord wCCR;       // channel control register
  SnWord wPORT;      // port register
  SnWord wICCR;      // internal correction control register
} volatile PWM;
/* Define PWM base addresses */
#define ptPWMA ((PWM*)0x00F140)
#define ptPWMB ((PWM*)0x00F160)
/* End PWM structures */

/* Define analog to digital converter structures */
typedef struct {
  SnWord wCR1;       // RW control register 1
  SnWord wCR2;       // RW control register 2
  SnWord wZCC;       // RW zero crossing control register
  SnWord wLST1;      // RW channel list register 1
  SnWord wLST2;      // RW channel list register 2
  SnWord wSDIS;      // RW sample disable register
  SnWord wSTAT;      // RW status register
  SnWord wLSTAT;     // RW limit status register
  SnWord wZCSTAT;    // RW zero crossing status register
  SnWord wRSLT[8];   // RW result register 0 - 7
  SnWord wLLMT[8];   // RW low limit register 0 - 7
  SnWord wHLMT[8];   // RW high limit register 0 - 7
  SnWord wOFS[8];    // RW offset register 0 - 7
  SnWord wPOWER;     // RW power control register
  SnWord wCAL;       // RW calibration register
} volatile ADC;
/* Define ADC base addresses */
#define ptADCA ((ADC*)0x00F200)
#define ptADCB ((ADC*)0x00F240)
/* End analog to digital converter structures */

/* Define 4x4 Timer structures
   access example: SnWord value = ptTMRA->tChan0.wLOAD */
   
/* Define Channel structures */
typedef struct {
  SnWord wCMP1;      // RW compare register 1
  SnWord wCMP2;      // RW compare register 2
  SnWord wCAP;       // RW capture registers
  SnWord wLOAD;      // RW load registers
  SnWord wHOLD;      // RW hold registers
  SnWord wCNTR;      // RW counter registers
  SnWord wCTRL;      // RW control registers
  SnWord wSCR;       // RW status and control registers
  SnWord wCMPLD1;    // RW comparator load registers 1
  SnWord wCMPLD2;    // RW comparator load registers 2
  SnWord wCOMSCR;    // RW comparator status and control registers
  SnWord wUNUSED[5]; // unused
} volatile Channel;
/* Define Timer as structure of 4 channel structures */
typedef struct {
  Channel tChan0;
  Channel tChan1;
  Channel tChan2;
  Channel tChan3;
} volatile Timer;
/* Timer base addresses */
#define ptTMRA ((Timer*)0x00F040)
#define ptTMRB ((Timer*)0x00F080)
#define ptTMRC ((Timer*)0x00F0C0)
#define ptTMRD ((Timer*)0x00F100)
/* End Timer structures */

/* FlexCan interface structures */
typedef struct {
	SnWord wFCMB_CTL;
	SnWord wFCMB_ID_H;
	SnWord wFCMB_ID_L;
	SnByte pbFCMB_DATA[8];
	SnWord wReserved;	
} volatile FlexCanMB;

typedef struct {
	SnWord wFCMCR;
	SnWord pwUnused1[2];
	SnWord wFCCTL0;
	SnWord wFCCTL1;
	SnWord wFCTMR;
	SnWord wFCMAXMB;
	SnWord wUnused2;
	SnWord wFCRnGMASK_H;
	SnWord wFCRnGMASK_L;
	SnWord wFCRn14MASK_H;
	SnWord wFCRn14MASK_L;
	SnWord wFCRn15MASK_H;
	SnWord wFCRn15MASK_L;
	SnWord pwUnused3[2];
	SnWord wFCSTATUS;
	SnWord wFCIMASK1;
	SnWord wFCIFLAG1;
	SnWord wFC_ERR_CNTRS;
	SnWord pwUnused4[0x40 - 0x13 - 1];
	FlexCanMB ptMB[16];
} volatile FlexCan;
/* Define FlexCan base address */
#define ptFlexCan ((FlexCan *)0x00F800)
/* End FlexCan structure */

/* Define interrupt control structure */
typedef struct {
  SnWord wIPR[10];   // interrupt priority register 0 - 9
  SnWord wVBA;       // vector base address register
  SnWord wFIM0;      // fast interrupt 0 match register
  SnWord wFIVAL0;    // fast interrupt 0 vector address low register
  SnWord wFIVAH0;    // fast interrupt 0 vector address high register
  SnWord wFIM1;      // fast interrupt 1 match register
  SnWord wFIVAL1;    // fast interrupt 1 vector address low register
  SnWord wFIVAH1;    // fast interrupt 1 vector address high register
  SnWord wIRQP[6];   // interrupt pending register 0 - 5
  SnWord wUNUSED[6]; // unused
  SnWord wICTL;      // interrupt control register
} volatile Interrupt;
/* Define interrupt control register block base address */
#define ptITCN ((Interrupt*)0x00F1A0)
/* End interupt control structure */

/* Define address of internal temperature sensor configuration register */
#define wTSENSE (*(volatile SnWord*)0x00F270)

/* Define COP watchdog structure */
typedef struct {
  SnWord wCOPCTL;
  SnWord wCOPTO;
  SnWord wCOPCTR;
} volatile COP;
/* Define COP watchdog base address */
#define ptCOP ((COP*)0x00F2C0)

/* Define SCI serial port structures */
typedef struct {
    SnWord wSCIBR;
    SnWord wSCICR;
    SnWord wReserved;
    SnWord wSCISR;
    SnWord wSCIDR;
} volatile Sci;
/* Serial port base addresses */
#define ptSci0 ((Sci*)0x00F280)
#define ptSci1 ((Sci*)0x00F290)
/* End SCI structures */

#endif  // MC56F8357_H