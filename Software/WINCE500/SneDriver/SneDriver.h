#include "..\SnIoctl.h"

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

extern AtmelPio *g_ptPioA;
extern AtmelPio *g_ptPioB;
extern AtmelPio *g_ptPioC;
extern AtmelPio *g_ptPioE;

#define LOGINTR_BASE_PIOA			64
#define LOGINTR_BASE_PIOB			96                           
#define LOGINTR_BASE_PIOC			128

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

extern AtmelPmc *g_ptPmc;

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

extern AtmelLcdc *g_ptLcdc;

typedef volatile struct {
	SnQByte qCAN_MB_MMR; 	// MailBox Mode Register
	SnQByte	qCAN_MB_MAM; 	// MailBox Acceptance Mask Register
	SnQByte	qCAN_MB_MID; 	// MailBox ID Register
	SnQByte	qCAN_MB_MFID; 	// MailBox Family ID Register
	SnQByte	qCAN_MB_MSR; 	// MailBox Status Register
	SnQByte	qCAN_MB_MDL; 	// MailBox Data Low Register
	SnQByte	qCAN_MB_MDH; 	// MailBox Data High Register
	SnQByte	qCAN_MB_MCR; 	// MailBox Control Register
} AtmelCanMB;

typedef volatile struct {
	SnQByte	qCAN_MR; 	    // Mode Register
	SnQByte	qCAN_IER; 	    // Interrupt Enable Register
	SnQByte	qCAN_IDR; 	    // Interrupt Disable Register
	SnQByte	qCAN_IMR; 	    // Interrupt Mask Register
	SnQByte	qCAN_SR; 	    // Status Register
	SnQByte	qCAN_BR; 	    // Baudrate Register
	SnQByte	qCAN_TIM; 	    // Timer Register
	SnQByte	qCAN_TIMESTP; 	// Time Stamp Register
	SnQByte	qCAN_ECR; 	    // Error Counter Register
	SnQByte	qCAN_TCR; 	    // Transfer Command Register
	SnQByte	qCAN_ACR; 	    // Abort Command Register
	SnQByte	pqReserved0[52];// 
	SnQByte	qCAN_VR; 	    // Version Register
	SnQByte	pqReserved1[64];// 
	AtmelCanMB ptCAN_MB[16];// CAN Mailboxes
} AtmelCan;

extern AtmelCan *g_ptCan;

extern void *g_pxLcdcMap;
extern void *g_pxFlashMap;
extern void *g_pxAtmelMap;

extern DWORD  g_dwFlashIrq;
extern DWORD  g_dwFlashSysIntr;
extern HANDLE g_hFlashInterrupt;
extern HANDLE g_hFlashWaitEvent;

extern DWORD  g_dwDspIrq;
extern DWORD  g_dwDspSysIntr;
extern HANDLE g_hMsgInterrupt;
extern HANDLE g_hMsgInterruptThread;
extern HANDLE g_hMsgMutex;
extern HANDLE g_hDspIrqMutex;
extern HANDLE g_hReadyCmd;

extern SnBool g_yShutdown;

extern const SnByte g_pbCrcTable[];

#define AIC_INTERRUPTS_OFF  (ptIrq->qAIC_DCR = 0x00000002)
#define AIC_INTERRUPTS_ON   (ptIrq->qAIC_DCR = 0x00000000)

// Hold RESET low for at least 266ns (16T where T=16.67 for 60MHz) to reset the DSP.
// With 4 writes, it was measured at ~400ns.
#define RESET_DSP \
{ \
    g_ptPioB->qPIO_CODR = 0x00200000; \
    g_ptPioB->qPIO_CODR = 0x00200000; \
    g_ptPioB->qPIO_CODR = 0x00200000; \
    g_ptPioB->qPIO_CODR = 0x00200000; \
    g_ptPioB->qPIO_SODR = 0x00200000; \
}

#define ENABLE_BUZZER       (g_ptPioB->qPIO_SODR = 0x00040000)
#define DISABLE_BUZZER      (g_ptPioB->qPIO_CODR = 0x00040000)

#define FLASH_ERASE_TIMEOUT_IN_MS       2000
#define FLASH_WRITE_TIMEOUT_IN_MS       500

SnBool WaitForEraseComplete(SnAddr aSectorAddr);
SnBool EraseFlashPages(SnQByte qPages);
SnBool WaitForWriteComplete(SnAddr aSectorAddr);
SnBool WriteFlashData(volatile SnQByte *pqData, SnQByte qBytes);
SnBool ReadFlashData(volatile SnQByte *pqData, SnQByte qBytes);
SnBool VerifyFlashData(volatile SnQByte *pqData, SnQByte qBytes);
SnBool CrcBufData(SnByte *pbData, SnQByte qBytes, SnByte *pbCrc);
SnBool SetFlashOffset(SnQByte qOffset);
SnBool ReadFlashStore(SnFlashStore *ptFlashStore,
                      SnByte *pbLastStore, SnQByte *pqLastStoreSize);
SnBool WriteFlashStore(SnByte *pbStoreData, SnQByte qStoreSize);
SnBool EraseFlashStore(void);
SnBool InitFlash(void);
ULONG FlashInterruptThread(PVOID pContext);

SnBool CanTest(void);

void InitMsg(void);
SnBool SendWordToDsp(SnQByte qData);
SnBool GetWordFromDsp(SnQByte *pqData);
SnBool SetMasterDataOuput(SnBool yOutput);
SnBool SndCmd(SnWord *pwXmtWords, SnQByte qXmtWords,
                       SnWord *pwRcvWords, SnQByte qRcvWords,
                       SnQByte *pqActualOut);
ULONG MsgInterruptThread(PVOID pContext);

//
// Required Function entry points for a "stream" Interface DLL
//

DWORD SNE_Init(DWORD Registry);
BOOL SNE_Deinit(DWORD DeviceContext);
DWORD SNE_Open(DWORD DeviceContext, DWORD AccessCode, DWORD ShareMode);
BOOL SNE_Close(DWORD DeviceContext);
DWORD SNE_Read(DWORD Handle, LPVOID Buffer, DWORD Count);
DWORD SNE_Write(DWORD Handle, LPCVOID Source, DWORD Count);
DWORD SNE_Seek(DWORD Handle, long Amount, WORD Type);
BOOL SNE_IOControl(DWORD Handle, DWORD Code, PBYTE BufIn, DWORD LenIn,
				   PBYTE BufOut, DWORD LenOut, PDWORD ActOut);
VOID SNE_PowerDown(DWORD DeviceContext);
VOID SNE_PowerUp(DWORD DeviceContext);

