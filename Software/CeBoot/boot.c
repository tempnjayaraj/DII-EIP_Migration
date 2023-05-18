#include <SnTypes.h>
#include <SoftwareUpgrade.h>
#include <printf.h>
 
#define TRUE    1
#define FALSE   0

#define DISPLAY_WIDTH		800
#define DISPLAY_HEIGHT		480
#define VIDMEM_WIDTH		2048
#define VIDMEM_HEIGHT		2048

#define VIDMEM_PAD_BYTES		((VIDMEM_WIDTH - DISPLAY_WIDTH)*2)
#define VIDMEM_PAD_QBYTES		(VIDMEM_PAD_BYTES/sizeof(SnQByte))
#define DISPLAY_WIDTH_QBYTES	((DISPLAY_WIDTH*2)/sizeof(SnQByte))

#define ENABLE_BUZZER \
{\
    ptPioB->qPIO_SODR = 0x00140000;\
    ptPioB->qPIO_MDDR = 0x00140000;\
    ptPioB->qPIO_OER = 0x00140000;\
    ptPioB->qPIO_PER = 0x00140000;\
    ptPioB->qPIO_PPUDR = 0x00140000;\
}

#define DISPLAY_SIZE            (480 * 800 * sizeof(SnWord))
 
extern const SnByte bBootErrorScreenCrc;
extern const SnQByte qBootErrorScreenLen;
extern const SnQByte pqBootErrorScreen[];

typedef volatile struct {
	volatile SnQByte    qPIO_PER; 	// PIO Enable Register
	volatile SnQByte    qPIO_PDR; 	// PIO Disable Register
	volatile SnQByte    qPIO_PSR; 	// PIO Status Register
	volatile SnQByte    pqReserved0[1]; 	// 
	volatile SnQByte    qPIO_OER; 	// Output Enable Register
	volatile SnQByte    qPIO_ODR; 	// Output Disable Registerr
	volatile SnQByte    qPIO_OSR; 	// Output Status Register
	volatile SnQByte    pqReserved1[1]; 	// 
	volatile SnQByte    qPIO_IFER; 	// Input Filter Enable Register
	volatile SnQByte    qPIO_IFDR; 	// Input Filter Disable Register
	volatile SnQByte    qPIO_IFSR; 	// Input Filter Status Register
	volatile SnQByte    pqReserved2[1]; 	// 
	volatile SnQByte    qPIO_SODR; 	// Set Output Data Register
	volatile SnQByte    qPIO_CODR; 	// Clear Output Data Register
	volatile SnQByte    qPIO_ODSR; 	// Output Data Status Register
	volatile SnQByte    qPIO_PDSR; 	// Pin Data Status Register
	volatile SnQByte    qPIO_IER; 	// Interrupt Enable Register
	volatile SnQByte    qPIO_IDR; 	// Interrupt Disable Register
	volatile SnQByte    qPIO_IMR; 	// Interrupt Mask Register
	volatile SnQByte    qPIO_ISR; 	// Interrupt Status Register
	volatile SnQByte    qPIO_MDER; 	// Multi-driver Enable Register
	volatile SnQByte    qPIO_MDDR; 	// Multi-driver Disable Register
	volatile SnQByte    qPIO_MDSR; 	// Multi-driver Status Register
	volatile SnQByte    pqReserved3[1]; 	// 
	volatile SnQByte    qPIO_PPUDR; 	// Pull-up Disable Register
	volatile SnQByte    qPIO_PPUER; 	// Pull-up Enable Register
	volatile SnQByte    qPIO_PPUSR; 	// Pull-up Status Register
	volatile SnQByte    pqReserved4[1]; 	// 
	volatile SnQByte    qPIO_ASR; 	// Select A Register
	volatile SnQByte	qPIO_BSR; 	// Select B Register
	volatile SnQByte    qPIO_ABSR; 	// AB Select Status Register
	volatile SnQByte    pqReserved5[9]; 	// 
	volatile SnQByte    qPIO_OWER; 	// Output Write Enable Register
	volatile SnQByte    qPIO_OWDR; 	// Output Write Disable Register
	volatile SnQByte    qPIO_OWSR; 	// Output Write Status Register
} AtmelPio;

AtmelPio * const ptPioA = (AtmelPio *)0xfffff200;
AtmelPio * const ptPioB = (AtmelPio *)0xfffff400;
AtmelPio * const ptPioC = (AtmelPio *)0xfffff600;
AtmelPio * const ptPioD = (AtmelPio *)0xfffff800;

typedef struct {
	volatile SnQByte	qPMC_SCER; 	// System Clock Enable Register
	volatile SnQByte	qPMC_SCDR; 	// System Clock Disable Register
	volatile SnQByte	qPMC_SCSR; 	// System Clock Status Register
	volatile SnQByte	pqReserved0[1]; 	// 
	volatile SnQByte	qPMC_PCER; 	// Peripheral Clock Enable Register
	volatile SnQByte	qPMC_PCDR; 	// Peripheral Clock Disable Register
	volatile SnQByte	qPMC_PCSR; 	// Peripheral Clock Status Register
	volatile SnQByte	pqReserved1[1]; 	// 
	volatile SnQByte	qPMC_MOR; 	// Main Oscillator Register
	volatile SnQByte	qPMC_MCFR; 	// Main Clock  Frequency Register
	volatile SnQByte	qPMC_PLLAR; 	// PLL A Register
	volatile SnQByte	qPMC_PLLBR; 	// PLL B Register
	volatile SnQByte	qPMC_MCKR; 	// Master Clock Register
	volatile SnQByte	pqReserved2[3]; 	// 
	volatile SnQByte	qPMC_PCKR[8]; 	// Programmable Clock Register
	volatile SnQByte	qPMC_IER; 	// Interrupt Enable Register
	volatile SnQByte	qPMC_IDR; 	// Interrupt Disable Register
	volatile SnQByte	qPMC_SR; 	// Status Register
	volatile SnQByte	qPMC_IMR; 	// Interrupt Mask Register
} AtmelPmc;

AtmelPmc * const ptPmc = (AtmelPmc *)0xfffffc00;

typedef struct {
	volatile SnQByte	qWDTC_WDCR; 	// Watchdog Control Register
	volatile SnQByte	qWDTC_WDMR; 	// Watchdog Mode Register
	volatile SnQByte	qWDTC_WDSR; 	// Watchdog Status Register
} AtmelWdt;

AtmelWdt * const ptWdt = (AtmelWdt *)0xfffffd40;

typedef struct {
	volatile SnQByte	qSDRAMC_MR; 	// SDRAM Controller Mode Register
	volatile SnQByte	qSDRAMC_TR; 	// SDRAM Controller Refresh Timer Register
	volatile SnQByte	qSDRAMC_CR; 	// SDRAM Controller Configuration Register
	volatile SnQByte	qSDRAMC_HSR; 	// SDRAM Controller High Speed Register
	volatile SnQByte	qSDRAMC_LPR; 	// SDRAM Controller Low Power Register
	volatile SnQByte	qSDRAMC_IER; 	// SDRAM Controller Interrupt Enable Register
	volatile SnQByte	qSDRAMC_IDR; 	// SDRAM Controller Interrupt Disable Register
	volatile SnQByte	qSDRAMC_IMR; 	// SDRAM Controller Interrupt Mask Register
	volatile SnQByte	qSDRAMC_ISR; 	// SDRAM Controller Interrupt Mask Register
	volatile SnQByte	qSDRAMC_MDR; 	// SDRAM Memory Device Register
} AtmelSdram;

AtmelSdram * const ptSdram0 = (AtmelSdram *)0xffffe200;

typedef struct {
	volatile SnQByte	qLCDC_BA1; 	// DMA Base Address Register 1
	volatile SnQByte	qLCDC_BA2; 	// DMA Base Address Register 2
	volatile SnQByte	qLCDC_FRMP1; 	// DMA Frame Pointer Register 1
	volatile SnQByte	qLCDC_FRMP2; 	// DMA Frame Pointer Register 2
	volatile SnQByte	qLCDC_FRMA1; 	// DMA Frame Address Register 1
	volatile SnQByte	qLCDC_FRMA2; 	// DMA Frame Address Register 2
	volatile SnQByte	qLCDC_FRMCFG; 	// DMA Frame Configuration Register
	volatile SnQByte	qLCDC_DMACON; 	// DMA Control Register
	volatile SnQByte	qLCDC_DMA2DCFG; 	// DMA 2D addressing configuration
	volatile SnQByte	pqReserved0[503]; 	// 
	volatile SnQByte	qLCDC_LCDCON1; 	// LCD Control 1 Register
	volatile SnQByte	qLCDC_LCDCON2; 	// LCD Control 2 Register
	volatile SnQByte	qLCDC_TIM1; 	// LCD Timing Config 1 Register
	volatile SnQByte	qLCDC_TIM2; 	// LCD Timing Config 2 Register
	volatile SnQByte	qLCDC_LCDFRCFG; 	// LCD Frame Config Register
	volatile SnQByte	qLCDC_FIFO; 	// LCD FIFO Register
	volatile SnQByte	qLCDC_MVAL; 	// LCD Mode Toggle Rate Value Register
	volatile SnQByte	qLCDC_DP1_2; 	// Dithering Pattern DP1_2 Register
	volatile SnQByte	qLCDC_DP4_7; 	// Dithering Pattern DP4_7 Register
	volatile SnQByte	qLCDC_DP3_5; 	// Dithering Pattern DP3_5 Register
	volatile SnQByte	qLCDC_DP2_3; 	// Dithering Pattern DP2_3 Register
	volatile SnQByte	qLCDC_DP5_7; 	// Dithering Pattern DP5_7 Register
	volatile SnQByte	qLCDC_DP3_4; 	// Dithering Pattern DP3_4 Register
	volatile SnQByte	qLCDC_DP4_5; 	// Dithering Pattern DP4_5 Register
	volatile SnQByte	qLCDC_DP6_7; 	// Dithering Pattern DP6_7 Register
	volatile SnQByte	qLCDC_PWRCON; 	// Power Control Register
	volatile SnQByte	qLCDC_CTRSTCON; 	// Contrast Control Register
	volatile SnQByte	qLCDC_CTRSTVAL; 	// Contrast Value Register
	volatile SnQByte	qLCDC_IER; 	// Interrupt Enable Register
	volatile SnQByte	qLCDC_IDR; 	// Interrupt Disable Register
	volatile SnQByte	qLCDC_IMR; 	// Interrupt Mask Register
	volatile SnQByte	qLCDC_ISR; 	// Interrupt Enable Register
	volatile SnQByte	qLCDC_ICR; 	// Interrupt Clear Register
	volatile SnQByte	qLCDC_GPR; 	// General Purpose Register
	volatile SnQByte	qLCDC_ITR; 	// Interrupts Test Register
	volatile SnQByte	qLCDC_IRR; 	// Interrupts Raw Status Register
	volatile SnQByte	pqReserved1[230]; 	// 
	volatile SnQByte	pqLCDC_LUT_ENTRY[256]; 	// LUT Entries Register
} AtmelLcdc;

AtmelLcdc * const ptLcdc = (AtmelLcdc *)0x00700000;

AtmelDbgUart * const ptDbgU = (AtmelDbgUart *)0xffffee00;

SnWinCeHdr tHdr = { 0 };
SnByte *pbNkData = 0;
SnByte *pbSplashData = 0;

SnQByte qBootAddr = 0;

SnBool yDebugBoot = FALSE;

SnBool SetupDisplay(void);
void Fault(void);
void ZeroOutMem(SnQByte *pqDst, SnQByte qLen);
SnBool CopyWithCrcCheck(SnQByte *pqSrc, SnQByte *pqDst, SnQByte qLen, SnByte bCrc);
SnBool RunLengthDecompressNK(SnQByte *pqSrc, SnQByte *pqDst, SnQByte qSrcLen, SnQByte qDstLen, SnByte bCrc);
SnBool RunLengthDecompressSplash(SnQByte *pqSrc, SnQByte *pqDst, SnQByte qSrcLen, SnQByte qDstLen, SnByte bCrc);
SnBool CopySplashScreen(void);
SnBool BootWindowsCE(void);
SnBool ReadRomHeader(SnQByte qAddr);
SnBool TryBoot(SnQByte qAddr);
extern void *InitMmu(void);
extern void EnableMmu(void);

const SnByte pbCrcTable[] = 
{
/*0*/   0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
/*1*/ 157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
/*2*/  35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
/*3*/ 190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
/*4*/  70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
/*5*/ 219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
/*6*/ 101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
/*7*/ 248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
/*8*/ 140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
/*9*/  17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
/*a*/ 175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
/*b*/  50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
/*c*/ 202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
/*d*/  87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
/*e*/ 233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
/*f*/ 116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
//     0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f
};

SnBool SetupDisplay(void)
{
	SnBool yStatus;
    volatile SnQByte qDelay;
    volatile SnQByte qTmp;
	
	// Enable clock for LCDC
	ptPmc->qPMC_PCER = 0x04000000;

    // Set PB9 to be LCDCC (Contrast Control)
    //     Configure PB9 to Function B
    //     Disable Multi Output on PB9
    //     Disable PIO control of PB9
    //     Disable Pullup on PB9
	ptPioB->qPIO_BSR = 0x00000200;
	ptPioB->qPIO_MDDR = 0x00000200;
	ptPioB->qPIO_PDR = 0x00000200;
	ptPioB->qPIO_PPUDR = 0x00000200;

	// Configure LCD function pins
    //     Configure LCD to Function A & B
    //     Disable Multi Output on LCD
    //     Disable PIO control of LCD
    //     Disable Pullup on LCD
	ptPioC->qPIO_ASR = 0x0dcdcfcf;
	ptPioC->qPIO_BSR = 0x00021000;
	ptPioC->qPIO_MDDR = 0x0dcfdfcf;
	ptPioC->qPIO_PDR = 0x0dcfdfcf;
	ptPioC->qPIO_PPUDR = 0x0dcfdfcf;

	/* Turn off the LCD controller, DMA controller and Disable IRQs */
	ptLcdc->qLCDC_PWRCON = 0x00000000;
	while (ptLcdc->qLCDC_PWRCON & 0x80000000);
	ptLcdc->qLCDC_DMACON = 0x00000000;
	ptLcdc->qLCDC_IDR = 0x00000077;
	
	/* Reset LCDC DMA */
	ptLcdc->qLCDC_DMACON = 0x00000002;

	// Setup Display for a Hantronix HDA700L LCD
	ptLcdc->qLCDC_LCDCON1 = 0x00001000;		// Dot Clock = 30.0 MHz
	ptLcdc->qLCDC_LCDCON2 = 0x80008692;		// Little Endian, LCDDOTCK always active,
											// LCDDEN Active High, LCDHSYNC Active Low, LCDVSYNC Active Low,
											// LCDD Normal, 16 bits per pixel, Single Scan, TFT
	ptLcdc->qLCDC_TIM1 = 0x0000002c;		// VHDLY = 0, VPW = 0, VBP = 0, VFP = 45
	ptLcdc->qLCDC_TIM2 = 0x1fe00000;		// HFP = 256, HPW = 0, HBP = 0
	ptLcdc->qLCDC_LCDFRCFG = 0x63e001df;	// 800 x 480 display
	ptLcdc->qLCDC_FIFO = 0x000001f5;

	ptLcdc->qLCDC_BA1 = tHdr.qSplashRamStart;
	ptLcdc->qLCDC_FRMCFG = 0x0302ee00;
	ptLcdc->qLCDC_DMA2DCFG = VIDMEM_PAD_BYTES;

	// Turn on the Display timing
	ptLcdc->qLCDC_DMACON = 0x00000019;
	ptLcdc->qLCDC_PWRCON = 0x0000001f;

	// Copy the Splash Screen over and delay a bit to give time for the LCD display to settle
	// Warning - This depends on the 240MHz clock speed, any change in clock speed and this should
	// be adjusted. This is currently ~200ms.
	yStatus = CopySplashScreen();
	for (qDelay = 0; qDelay < 800000; qDelay++);
        qTmp = *(SnQByte *)0x10000000;

	// Turn on the Backlight
	ptLcdc->qLCDC_CTRSTCON = 0x0000000f;
	ptLcdc->qLCDC_CTRSTVAL = 0x000000da;

    return yStatus;
}

void Fault(void)
{
    tHdr.qSplashRamStart = 0x23B10000;
    tHdr.bSplashCopy = COPY_RL_COMPRESSED;
    tHdr.qSplashRomLen = qBootErrorScreenLen;
    tHdr.bSplashCrc = bBootErrorScreenCrc;
    pbSplashData = (SnByte *)pqBootErrorScreen;
    SetupDisplay();
    
    ENABLE_BUZZER;
    
    for (;;) {
        // Loop Forever
    }
}

void ZeroOutMem(SnQByte *pqDst, SnQByte qLen)
{
    do {
        *pqDst++ = 0;
        qLen -= 4;
    } while (qLen > 3);
    
    if (qLen > 0) {
	 	SnByte *pbDst = (SnByte *)pqDst;
		do *pbDst++ = 0;
		while (--qLen > 0);
    }    
}

SnBool CopyWithCrcCheck(SnQByte *pqSrc, SnQByte *pqDst, SnQByte qLen, SnByte bCrc)
{
    SnByte bNewCrc = 0;
    SnQByte qData;

    // Convert Length from Bytes to SnQByte
    qLen >>= 2;

    while (qLen > 0) {
        qData = *pqDst++ = *pqSrc++;
        bNewCrc = pbCrcTable[bNewCrc ^ (qData & 0xFF)];
        bNewCrc = pbCrcTable[bNewCrc ^ ((qData >> 8) & 0xFF)];
        bNewCrc = pbCrcTable[bNewCrc ^ ((qData >> 16) & 0xFF)];
        bNewCrc = pbCrcTable[bNewCrc ^ (qData >> 24)];
        qLen--;
    }
        
    return (bNewCrc == bCrc) ? TRUE : FALSE;
}

SnBool CrcFlash(SnQByte qAddr, SnQByte qBytes, SnByte bCrc)
{
    volatile SnQByte *pqFlash = (SnQByte *)qAddr;
    SnQByte qWords = qBytes >> 2;
    SnByte bNewCrc = 0;
    SnQByte qData;

    do {
        qData = *pqFlash++;
        bNewCrc = pbCrcTable[bNewCrc ^ (qData & 0xFF)];
        bNewCrc = pbCrcTable[bNewCrc ^ ((qData >> 8) & 0xFF)];
        bNewCrc = pbCrcTable[bNewCrc ^ ((qData >> 16) & 0xFF)];
        bNewCrc = pbCrcTable[bNewCrc ^ (qData >> 24)];
    } while (--qWords);

    return (bNewCrc == bCrc) ? TRUE : FALSE;
}

SnBool RunLengthDecompressNK(SnQByte *pqSrc, SnQByte *pqDst, SnQByte qSrcLen, SnQByte qDstLen, SnByte bCrc)
{
    SnByte bNewCrc = 0;
    SnQByte qRunLen;
    SnQByte qData;
    
    // Convert Lengths from Bytes to SnQByte
    qSrcLen >>= 2;
    qDstLen >>= 2;

    while (qSrcLen > 0) {
        qRunLen = *pqSrc++;
        qSrcLen--;
        bNewCrc = pbCrcTable[bNewCrc ^ (qRunLen & 0xFF)];
        bNewCrc = pbCrcTable[bNewCrc ^ ((qRunLen >> 8) & 0xFF)];
        bNewCrc = pbCrcTable[bNewCrc ^ ((qRunLen >> 16) & 0xFF)];
        bNewCrc = pbCrcTable[bNewCrc ^ (qRunLen >> 24)];
        
		// Fill block with value
        if (qRunLen & 0x80000000) {
            qRunLen &= 0x7FFFFFFF;
            if (qRunLen > qDstLen) {
               return FALSE;
            }
            qData = *pqSrc++;
            qSrcLen--;
            bNewCrc = pbCrcTable[bNewCrc ^ (qData & 0xFF)];
            bNewCrc = pbCrcTable[bNewCrc ^ ((qData >> 8) & 0xFF)];
            bNewCrc = pbCrcTable[bNewCrc ^ ((qData >> 16) & 0xFF)];
            bNewCrc = pbCrcTable[bNewCrc ^ (qData >> 24)];
            do {
                *pqDst++ = qData;
                qDstLen--;
            } while (--qRunLen > 0);
        } else {
			// Copy block of data
            if (qRunLen > qDstLen) {
                return FALSE;
            }
            do {
                qData = *pqDst++ = *pqSrc++;
                bNewCrc = pbCrcTable[bNewCrc ^ (qData & 0xFF)];
                bNewCrc = pbCrcTable[bNewCrc ^ ((qData >> 8) & 0xFF)];
                bNewCrc = pbCrcTable[bNewCrc ^ ((qData >> 16) & 0xFF)];
                bNewCrc = pbCrcTable[bNewCrc ^ (qData >> 24)];
                qSrcLen--;
                qDstLen--;
            } while (--qRunLen > 0);
        }
    }

    return (bNewCrc == bCrc) ? TRUE : FALSE;
}

SnBool RunLengthDecompressSplash(SnQByte *pqSrc, SnQByte *pqDst, SnQByte qSrcLen, SnQByte qDstLen, SnByte bCrc)
{
    SnByte bNewCrc = 0;
    SnQByte qRunLen;
    SnQByte qData;
	SnQByte qX = 0;
	SnQByte qPad = VIDMEM_PAD_QBYTES;
    
    // Convert Lengths from Bytes to SnQByte
    qSrcLen >>= 2;
    qDstLen >>= 2;

    while (qSrcLen > 0) {
        qRunLen = *pqSrc++;
        qSrcLen--;
        bNewCrc = pbCrcTable[bNewCrc ^ (qRunLen & 0xFF)];
        bNewCrc = pbCrcTable[bNewCrc ^ ((qRunLen >> 8) & 0xFF)];
        bNewCrc = pbCrcTable[bNewCrc ^ ((qRunLen >> 16) & 0xFF)];
        bNewCrc = pbCrcTable[bNewCrc ^ (qRunLen >> 24)];
        
		// Fill block with value
        if (qRunLen & 0x80000000) {
            qRunLen &= 0x7FFFFFFF;
            if (qRunLen > qDstLen) {
               return FALSE;
            }
            qData = *pqSrc++;
            qSrcLen--;
            bNewCrc = pbCrcTable[bNewCrc ^ (qData & 0xFF)];
            bNewCrc = pbCrcTable[bNewCrc ^ ((qData >> 8) & 0xFF)];
            bNewCrc = pbCrcTable[bNewCrc ^ ((qData >> 16) & 0xFF)];
            bNewCrc = pbCrcTable[bNewCrc ^ (qData >> 24)];
            do {
                *pqDst++ = qData;
				if (++qX == DISPLAY_WIDTH_QBYTES) {
					pqDst += qPad;
					qX = 0;
				}
                qDstLen--;
            } while (--qRunLen > 0);
        } else {
			// Copy block of data
            if (qRunLen > qDstLen) {
                return FALSE;
            }
            do {
                qData = *pqDst++ = *pqSrc++;
                bNewCrc = pbCrcTable[bNewCrc ^ (qData & 0xFF)];
                bNewCrc = pbCrcTable[bNewCrc ^ ((qData >> 8) & 0xFF)];
                bNewCrc = pbCrcTable[bNewCrc ^ ((qData >> 16) & 0xFF)];
                bNewCrc = pbCrcTable[bNewCrc ^ (qData >> 24)];
                qSrcLen--;
                qDstLen--;
  				if (++qX == DISPLAY_WIDTH_QBYTES) {
					pqDst += qPad;
					qX = 0;
				}
          } while (--qRunLen > 0);
        }
    }

    return (bNewCrc == bCrc) ? TRUE : FALSE;
}

SnBool CopySplashScreen(void)
{
    SnQByte *pqSrc = (SnQByte *)pbSplashData;
    SnQByte *pqDst;
    SnBool yStatus = TRUE;
    SnQByte qLen = tHdr.qSplashRomLen;

	switch (tHdr.bSplashCopy) {
    case COPY_ZERO:
        ZeroOutMem((SnQByte *)tHdr.qSplashRamStart, tHdr.qSplashRamLen);
        break;

    case COPY_UNCOMPRESSED:
        pqDst = (SnQByte *)tHdr.qSplashRamStart;
        yStatus = CopyWithCrcCheck(pqSrc, pqDst, qLen, tHdr.bSplashCrc);
        break;

    case COPY_RL_COMPRESSED:
        pqDst = (SnQByte *)tHdr.qSplashRamStart;
        yStatus = RunLengthDecompressSplash(pqSrc, pqDst, qLen, tHdr.qSplashRamLen,
          tHdr.bSplashCrc);
        break;

    default:
        yStatus = FALSE;
        break;
    }
    
    if (yStatus == FALSE) {
        ZeroOutMem((SnQByte *)tHdr.qSplashRamStart, tHdr.qSplashRamLen);
    }

    return yStatus;
}

SnBool LoadWindowsCE(void)
{
    SnBool yStatus = TRUE;
    SnQByte *pqSrc = (SnQByte *)pbNkData;
    SnQByte *pqDst;
    SnQByte qLen;

    qLen = tHdr.qNkRomLen;

	tHdr.qNkRamStart -= 0x60000000;
	tHdr.qNkRamBoot -= 0x60000000;

	switch (tHdr.bNkCopy) {
    case COPY_UNCOMPRESSED:
        pqDst = (SnQByte *)tHdr.qNkRamStart;
        yStatus = CopyWithCrcCheck(pqSrc, pqDst, qLen, tHdr.bNkCrc);
        break;

    case COPY_RL_COMPRESSED:
        pqDst = (SnQByte *)(tHdr.qNkRamStart);
        yStatus = RunLengthDecompressNK(pqSrc, pqDst, qLen, 
          tHdr.qNkRamLen, tHdr.bNkCrc);
        break;

    case COPY_ZERO:
    default:
        yStatus = FALSE;
        break;
    }

	return yStatus;
}

SnBool ReadRomHeader(SnQByte qAddr)
{
    SnByte *pbRomHdr = (SnByte *)qAddr;
    SnBool yStatus;

    yStatus = CopyWithCrcCheck((SnQByte *)pbRomHdr, (SnQByte *)&tHdr,
      sizeof(SnWinCeHdr), 0);
    
    if (yStatus) {
        pbSplashData = (pbRomHdr + sizeof(SnWinCeHdr));
        pbNkData = pbSplashData + ((tHdr.qSplashRomLen + 3) & ~0x3);
    }
  
    return yStatus;
}

void InitHardware(void)
{
	int iDelay;

	/* Disable watchdog */
	ptWdt->qWDTC_WDMR = (1 << 15);

	// ----------------------------- PMC ----------------------------

	/* At this stage the main oscillator is supposed to be enabled
	 * and PLLA is configured, so just configure PLLB */

	/* Configure PLLB */
	ptPmc->qPMC_PLLBR = 0x10283f07;		// PLLB = PCLK / 2.5 = 96MHz
	while (!(ptPmc->qPMC_SR & (1 << 2)));

	// ---------------------------- DBGU ----------------------------

	/* Set Port C PIO (Debug Rx / Tx) to Peripheral A */
	ptPioC->qPIO_IDR = 0xc0000000;
	ptPioC->qPIO_PPUDR = 0xc0000000;
	ptPioC->qPIO_ASR = 0xc0000000;
	ptPioC->qPIO_PDR = 0xc0000000;

	/* Disable interrupts, Reset Tx/Rx, 115200 baud, async */
	ptDbgU->qDBGU_IDR = 0xffffffff;
	ptDbgU->qDBGU_CR = 0x000000ac;
    // BAUD = MCK/(16*BRGR) = 120000000/(16*65) = ~115384 baud
	ptDbgU->qDBGU_BRGR = 0x00000041;

	ptDbgU->qDBGU_MR = 0x00000e00;
	ptDbgU->qDBGU_CR = 0x00000050;

	// ---------------------------- SDRAM ---------------------------

	/* VDDIOMSEL = 1 -> Memories are 3.3V powered */
	*(volatile SnQByte *)0xffffed20 = 0x00010002;

	/* Set Port D PIO (EBI0 D16-D31, A23) to Peripheral A */
	ptPioD->qPIO_IDR = 0xffff1000;
	ptPioD->qPIO_PPUDR = 0xffff1000;
	ptPioD->qPIO_ASR = 0xffff1000;
	ptPioD->qPIO_PDR = 0xffff1000;

	ptSdram0->qSDRAMC_CR = 0x85227259;						// CFG Control Register

	for (iDelay = 0; iDelay < 1000; iDelay++);

	ptSdram0->qSDRAMC_MR = 0x00000002;						// Set PRCHG AL
	*(volatile SnQByte *)0x20000000 = 0x00000000;			// Perform PRCHG

	for (iDelay = 0; iDelay < 1000; iDelay++);

	ptSdram0->qSDRAMC_MR = 0x00000004;						// Set 1st CBR
	*(volatile SnQByte *)0x20000010 = 0x00000001;			// Perform CBR
	ptSdram0->qSDRAMC_MR = 0x00000004;						// Set 2 CBR
	*(volatile SnQByte *)0x20000020 = 0x00000002;			// Perform CBR
	ptSdram0->qSDRAMC_MR = 0x00000004;						// Set 3 CBR
	*(volatile SnQByte *)0x20000030 = 0x00000003;			// Perform CBR
	ptSdram0->qSDRAMC_MR = 0x00000004;						// Set 4 CBR
	*(volatile SnQByte *)0x20000040 = 0x00000004;			// Perform CBR
	ptSdram0->qSDRAMC_MR = 0x00000004;						// Set 5 CBR
	*(volatile SnQByte *)0x20000050 = 0x00000005;			// Perform CBR
	ptSdram0->qSDRAMC_MR = 0x00000004;						// Set 6 CBR
	*(volatile SnQByte *)0x20000060 = 0x00000006;			// Perform CBR
	ptSdram0->qSDRAMC_MR = 0x00000004;						// Set 7 CBR
	*(volatile SnQByte *)0x20000070 = 0x00000007;			// Perform CBR
	ptSdram0->qSDRAMC_MR = 0x00000004;						// Set 8 CBR
	*(volatile SnQByte *)0x20000080 = 0x00000008;			// Perform CBR
	ptSdram0->qSDRAMC_MR = 0x00000003;						// Set LMR operation
	*(volatile SnQByte *)0x20000090 = 0xcafedede;			// Perform LMR burst=1, lat=2

	ptSdram0->qSDRAMC_TR = 0x000002bb; 						// Set Refresh Timer

	ptSdram0->qSDRAMC_MR = 0x00000000;						// Set Normal mode
	*(volatile SnQByte *)0x20000000 = 0x00000000;			// Perform Normal mode
}

SnBool CheckBoot(qAddr)
{
    if (!ReadRomHeader(qAddr)) {
        return FALSE;
    }

	qAddr += sizeof(SnWinCeHdr);

    if (!CrcFlash(qAddr, tHdr.qSplashRomLen, tHdr.bSplashCrc))
    	return FALSE;
    
	qAddr += ((tHdr.qSplashRomLen + 3) & ~0x3);

    return CrcFlash(qAddr, tHdr.qNkRomLen, tHdr.bNkCrc);
}

SnBool LoadBoot(SnQByte qAddr)
{
    SnBool yStatus;
    
    yStatus = ReadRomHeader(qAddr);
    if (yStatus == FALSE) {
        return FALSE;
    }
    yStatus = SetupDisplay();
    if (yStatus == FALSE) {
        return FALSE;
    }

    yStatus = LoadWindowsCE();
    if (yStatus == FALSE) {
        return FALSE;
    }
    
    qBootAddr = tHdr.qNkRamBoot;

    return TRUE;
}

int main(int argc, char *argv[])
{
	SnBool yLowerValid;
	SnBool yUpperValid;

	InitHardware();

	printf("\n\n*** Boot Version 1.00 ***\n\n");

	//
    // Validate both Lower and Upper Flash.
    // Boot the first valid one found.
    // If neither are valid then fault.
    //
    yLowerValid = LoadBoot(0x10000000+LOWER_FLASH_ADDR);
    if (yLowerValid) {
    	yUpperValid = CheckBoot(0x10000000+UPPER_FLASH_ADDR);
    } else {
    	yUpperValid = LoadBoot(0x10000000+UPPER_FLASH_ADDR);
    }
    
    ptLcdc->pqLCDC_LUT_ENTRY[0] = yLowerValid ? 1 : 0;
    ptLcdc->pqLCDC_LUT_ENTRY[1] = yUpperValid ? 1 : 0;
    
    if (!yLowerValid && !yUpperValid)
        Fault();
    
	/* Zero out boot paramters and jump to CE boot */
    ZeroOutMem((SnQByte *)0x2006b800, 0x800);
    ((void(*)())qBootAddr)();
    
	return 0;
}
