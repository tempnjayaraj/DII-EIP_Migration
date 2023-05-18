// AdjustDisplay.h

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
