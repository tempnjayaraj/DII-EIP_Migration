//-----------------------------------------------------------------------------
//! \addtogroup	OAL
//! @{
//!
//  All rights reserved ADENEO SAS 2005
//!
//-----------------------------------------------------------------------------
//! \file		init.c
//!
//! \brief		This file implements the OEMInit function
//!
//! \if subversion
//!   $URL: http://centaure/svn/interne-ce_bsp_atmel/TRUNK50/PLATFORM/AT91SAM9263EK/SRC/KERNEL/OAL/init.c $
//!   $Author: Tenneyd $
//!   $Revision: 2 $
//!   $Date: 2/03/09 3:35p $
//! \endif
//-----------------------------------------------------------------------------

#include <windows.h>
#include <oal.h>
#include <oal_intr.h>
#include <oal_memory.h>
#include <oal_timer.h>
#include <oal_flash.h>
#include <oal_perreg.h>
#include <blcommon.h>
#include "AT91SAM926x.h"
#include "AT91SAM926x_interface.h"
#include "AT91SAM926x_oal_intr.h"
#include "AT91SAM926x_oal_power.h"
#include "AT91SAM926x_oal_ioctl.h"
#include "AT91SAM926x_oal_watchdog.h"
#include "AT91SAM9263_oal_hmatrix.h"


#include "at91sam9263EK.h"
#include "oal_args.h"
#include "bsp_cfg.h"
#include "drv_glob.h"
#include "at91sam9263ek_HMatrix.h"

#include "lib_AT91SAM926x.h"

#include "..\..\version.h"

#define MAX_CORE_FREQUENCY		240
#define MAX_BUS_DIVIDER			4

#define DEFAULT_CORE_FREQUENCY	200
#define DEFAULT_BUS_DIVIDER		4

extern DWORD SC_GetTickCount();
extern BOOL BSPIoCtlHalShutdown (UINT32 code, VOID *pInpBuffer,UINT32 inpSize, VOID *pOutBuffer,UINT32 outSize, UINT32 *pOutSize);

UINT32 OALGetTickCount()
{
    return SC_GetTickCount();
}

//Here we give the list of our SDRAM areas to the kernel
DWORD OEMEnumExtensionDRAM(PMEMORY_SECTION pMemSections,DWORD cMemSections)
{
#ifndef IMGEBOOT
	//For the moment we only get back the space reserved for eboot.
	MEMORY_SECTION SDRAMExtensionTable[] = {
		{0,AT91SAM9263EK_BASE_BLDRIMAGE,AT91SAM9263EK_BLDR_RESERVEDAREA_SIZE}, //Section reserved for Eboot
	};
	DWORD toCopy = sizeof(SDRAMExtensionTable);
		

	if ((cMemSections*sizeof(MEMORY_SECTION)) < sizeof(SDRAMExtensionTable))
	{
		RETAILMSG(1,(TEXT("OEMEnumExtensionDRAM : Unable to give every memory sections. not enough space in pMemSections\r\n")));
		toCopy = cMemSections*sizeof(MEMORY_SECTION);
	}
	memcpy(pMemSections,SDRAMExtensionTable,toCopy);
	return toCopy/sizeof(MEMORY_SECTION);
#else
	return 0;
#endif
}


DWORD GetResetCause()
{
	AT91PS_RSTC pResetCtrl = (AT91PS_RSTC) OALPAtoVA((DWORD)AT91C_BASE_RSTC,FALSE);
	
	return pResetCtrl->RSTC_RSR & AT91C_RSTC_RSTTYP;
}

extern BOOL fForceCleanBoot;
extern void OEMProfileTimerEnable(DWORD);
extern DWORD g_dwSoftRebootMagic;
extern DWORD GetPLLAFreq(AT91PS_PMC);
extern DWORD GetPLLBFreq(AT91PS_PMC);

extern unsigned int ConfigureBaudrate (const unsigned int main_clock,const unsigned int baud_rate);

/// Initialize Hardware Interfaces needed by the kernel
///
/// OEMInit is called by the kernel after it has performed minimal
/// initialization. Interrupts are disabled and the kernel is not
/// ready to handle exceptions. 
void OEMInit(void)
{
    //extern UINT32 idleconv;
	PDRIVER_GLOBALS pDrvGlobalArea;	
	int i=0;
	AT91PS_PMC pPMC = 0;
	AT91PS_MATRIX	pMatrix = (AT91PS_MATRIX)OALPAtoVA((DWORD)AT91C_BASE_MATRIX, FALSE);
  	AT91PS_SMC pSMC0 = (AT91PS_SMC)OALPAtoVA((DWORD)AT91C_BASE_SMC0, FALSE);
	AT91PS_SMC pSMC1 = (AT91PS_SMC)OALPAtoVA((DWORD)AT91C_BASE_SMC1, FALSE);
	AT91PS_RTTC pRTTC0 = (AT91PS_RTTC) OALPAtoVA((DWORD) AT91C_BASE_RTTC0, FALSE);
 	AT91PS_RTTC pRTTC1 = (AT91PS_RTTC) OALPAtoVA((DWORD) AT91C_BASE_RTTC1, FALSE);
	AT91PS_RSTC pRSTC = (AT91PS_RSTC) OALPAtoVA((DWORD) AT91C_BASE_RSTC, FALSE);
	AT91PS_PIO pPIOC = (AT91PS_PIO) OALPAtoVA((DWORD) AT91C_BASE_PIOC, FALSE);
	AT91PS_PIO pPIOE = (AT91PS_PIO) OALPAtoVA((DWORD) AT91C_BASE_PIOE, FALSE);

	AT91PS_USART VirtualAddr_DBGU;
	extern DWORD dwOEMWatchDogPeriod;
	DWORD dwCoreFrequency,dwBusFreqDivider;

#ifdef FAKE_STRONGARM
    extern DWORD CEProcessorType;
    CEProcessorType=PROCESSOR_STRONGARM;
#endif

	// A reset of the external chip is necessary to fix the ethernet PHY's addresses
	pPIOC->PIO_PER = AT91C_PIO_PC25;
	pPIOC->PIO_OER = AT91C_PIO_PC25;
	pPIOC->PIO_CODR = AT91C_PIO_PC25;
	pPIOE->PIO_PER = (AT91C_PIO_PE25 | AT91C_PIO_PE26 | AT91C_PIO_PE22);
	pPIOE->PIO_OER = (AT91C_PIO_PE25 | AT91C_PIO_PE26 | AT91C_PIO_PE22);
	pPIOE->PIO_CODR = (AT91C_PIO_PE25 | AT91C_PIO_PE26 | AT91C_PIO_PE22);

	pRSTC->RSTC_RCR = (0xA5 << 24 ) | AT91C_RSTC_EXTRST;

	RETAILMSG(1,(TEXT("Windows CE Firmware Init\r\n")));	
	RETAILMSG(1,(TEXT("BSP %d.%d.%d for the AT91SAM9263EK board (built %s)\r\nAdaptation performed by ADENEO (c) 2005\r\n"),BSP_VERSION_MAJOR, BSP_VERSION_MINOR, BSP_VERSION_RELEASE, TEXT(__DATE__)));

	pPMC = (AT91PS_PMC) OALPAtoVA((DWORD) AT91C_BASE_PMC,FALSE);
	if (pPMC == NULL)
	{
		return;
	}

	VirtualAddr_DBGU = (AT91PS_USART) OALPAtoVA((DWORD) AT91C_BASE_DBGU,FALSE);
	if (VirtualAddr_DBGU == NULL)
	{
		return;
	}


     // Initialize interrupts.
    //
    if (!OALIntrInit())
    {
        OALMSG(OAL_ERROR, (L"ERROR: OEMInit: failed to initialize interrupts.\r\n"));
    }
    // Initializes the global variables for the use of cache
    // 	
	OALCacheGlobalsInit();
    
	// Initialize driver globals area
    //    
    pDrvGlobalArea = OALPAtoVA(DRIVER_GLOBALS_PHYSICAL_MEMORY_START,TRUE);	

 	if (pDrvGlobalArea)
    {
		RETAILMSG(1,(TEXT("Initialize driver globals Zeros area...\r\npDrvGlobalArea 0x%x  size 0x%x (0x%x -0x%x)\r\n"),		
		pDrvGlobalArea,(((DWORD)(&pDrvGlobalArea->FirstNonZeroVariable)) - (DWORD)pDrvGlobalArea),(&pDrvGlobalArea->FirstNonZeroVariable),pDrvGlobalArea));
        memset((PVOID)pDrvGlobalArea, 0,((DWORD)(&pDrvGlobalArea->FirstNonZeroVariable)) - (DWORD)pDrvGlobalArea);
    }
    else
    {
        RETAILMSG(1,(TEXT("Initialize driver globals area...OALPAtoVA FAILED !\r\n")));
    }
	RETAILMSG(1,(TEXT("Initialize driver globals Zeros area...done\r\n")));


	if (pDrvGlobalArea != NULL)
	{
		dwCoreFrequency = pDrvGlobalArea->BSPArgs.dwCoreFrequency;
		dwBusFreqDivider = pDrvGlobalArea->BSPArgs.dwBusFreqDivider;
		if ( (pDrvGlobalArea->BSPArgs.dwCoreFrequency > MAX_CORE_FREQUENCY) || (pDrvGlobalArea->BSPArgs.dwBusFreqDivider > MAX_BUS_DIVIDER))
		{
			RETAILMSG(1,(TEXT("CAUTION : Invalid PLL parameters given by the bootloader. Using it without warantly\r\n")));
		}
	}
	else
	{
		dwCoreFrequency = DEFAULT_CORE_FREQUENCY;
		dwBusFreqDivider = DEFAULT_BUS_DIVIDER;
		RETAILMSG(1,(TEXT("DrvGlobalArea not defined, using default PLL value")));
	}
		

	// Configure the PLLA frequency, the processor clock and the master clock...	
	AT91SAM926x_SetProcessorAndMasterClocks(pPMC, dwCoreFrequency, dwBusFreqDivider);
	// Reset DBGU baudrate so that we keep having readable debug messages
	VirtualAddr_DBGU->US_BRGR = ConfigureBaudrate(AT91SAM926x_GetMasterClock(FALSE), 115200);
	// PLLB configuration : PLL B is running at 48 MHz and there's no further divider for the USBs	
	AT91SAM926x_SetPLLBFreq(pPMC, 96 , AT91C_CKGR_USBDIV_1);	

	{
		// Small tempo to let the Chip a bit time to settle.
		// Someow it's no good to write characters on the serial before a bit.
		volatile DWORD dwTempo = 100000;
		while (dwTempo--);
	}
	RETAILMSG(1,(TEXT("-------------------\r\n")));
	RETAILMSG(1,(TEXT("|PLLA : %d Hz|\r\n"),GetPLLAFreq(pPMC)));
	RETAILMSG(1,(TEXT("|PLLB : %d Hz|\r\n"),GetPLLBFreq(pPMC)));
	RETAILMSG(1,(TEXT("--------------------\r\n")));
	
	
	/* JJH todo : check if the frequency change affects EMACB	*/


	RETAILMSG(1,(TEXT("Configuring EBI1 CS0 for PSRAM...\r\n")));
	// Setup bus config for the PSRAM on EBI1
	AT91SAM926x_SetChipSelectTimingIn_ns(	pSMC1							,	// pSMC0
											0							,		// Chip Select
											AT91SAM926x_GetMasterClock(FALSE),
											AT91C_PSRAM_EBI1_NWE_SETUP		,		// dwNWE_SETUP	
											AT91C_PSRAM_EBI1_NCS_WR_SETUP	,		// dwNCS_WR_SETUP
											AT91C_PSRAM_EBI1_NRD_SETUP		,		// dwNRD_SETUP	
											AT91C_PSRAM_EBI1_NCS_RD_SETUP	,		// dwNCS_RD_SETUP
											AT91C_PSRAM_EBI1_NWE_PULSE		,		// dwNWE_PULSE	
											AT91C_PSRAM_EBI1_NCS_WR_PULSE	,		// dwNCS_WR_PULSE
											AT91C_PSRAM_EBI1_NRD_PULSE		,		// dwNRD_PULSE	
											AT91C_PSRAM_EBI1_NCS_RD_PULSE	,		// dwNCS_RD_PULSE
											AT91C_PSRAM_EBI1_NRD_CYCLE		,		// dwNRD_CYCLE	
											AT91C_PSRAM_EBI1_NWE_CYCLE		);		// dwNWE_CYCLE

		// Configure SMC Mode Register
	pSMC1->SMC_CTRL0 = (AT91C_SMC_READMODE					|
						AT91C_SMC_WRITEMODE					|
						AT91C_SMC_NWAITM_NWAIT_DISABLE		|						
						AT91C_SMC_DBW_WIDTH_SIXTEEN_BITS	| 						
						AT91C_PSRAM_EBI1_TDF_CYCLES			);

    // Initialize system clocks and timers
    //
	RETAILMSG(1,(TEXT("OALTimerInit\r\n")));

	pRTTC0->RTTC_RTMR &= ~(AT91C_RTTC_ALMIEN | AT91C_RTTC_RTTINCIEN); 
	pRTTC1->RTTC_RTMR &= ~(AT91C_RTTC_ALMIEN | AT91C_RTTC_RTTINCIEN); 

	if (!OALTimerInit(OEM_DEFAULT_TICKS, -1/*we don't support this parameter*/, OEM_TICK_COUNT_MARGIN))
     {
        OALMSG(OAL_ERROR, (
            L"ERROR: OEMInit: failed to initialize the timer sub-system.\r\n"
        ));
    }

	// @@FDC To delete in the future but how run timer with interrupt without enable the
	// interrupts here????????????
	// Enable interrupts TO 
	INTERRUPTS_ON();
	

	//JJH : The following should be changed according to the boot strategy
	// if boot from eboot, a cold reboot may have occured.
	if (pDrvGlobalArea->bEboot == TRUE)
	{
#ifndef IMGEBOOT
		pDrvGlobalArea->bForceCleanBoot = TRUE;
#endif
		pDrvGlobalArea->bEboot=FALSE;
	}

	if (g_dwSoftRebootMagic == SOFT_REBOOT_MAGIC)
	{
		RETAILMSG(1,(TEXT("reset : Software Reset\r\n")));
	}
	else
	{		
		//Depending of the cause of the reset we may want to force a clean boot.
		switch (GetResetCause())
		{
			case AT91C_RSTC_RSTTYP_GENERAL:
				RETAILMSG(1,(TEXT("reset : General Reset (PowerOn)\r\n")));
				pDrvGlobalArea->bForceCleanBoot = TRUE;
				break;
			case AT91C_RSTC_RSTTYP_WAKEUP:
				RETAILMSG(1,(TEXT("reset : Wakeup Reset\r\n")));
				pDrvGlobalArea->bForceCleanBoot = TRUE;
				break;
			case AT91C_RSTC_RSTTYP_WATCHDOG:
				RETAILMSG(1,(TEXT("reset : Watchdog Reset\r\n")));
				break;
			case AT91C_RSTC_RSTTYP_SOFTWARE:
				RETAILMSG(1,(TEXT("reset : warm or cold Reset\r\n")));
				break;
			case AT91C_RSTC_RSTTYP_USER:
				RETAILMSG(1,(TEXT("reset : User Reset\r\n")));
				break;
			default:
				RETAILMSG(1,(TEXT("reset : Unknown cause\r\n")));
				pDrvGlobalArea->bForceCleanBoot = TRUE;
				break;
		}
	}
	g_dwSoftRebootMagic = 0;
		
	if (pDrvGlobalArea->bForceCleanBoot == TRUE)
	{		
		RETAILMSG(1,(TEXT("pDrvGlobalArea->bEboot == TRUE. Forcing Clean Object store\r\n")));
		NKForceCleanBoot();
		pDrvGlobalArea->bForceCleanBoot = FALSE;
	}
	
	// Intialize optional kernel functions.
    //
	pOEMIsProcessorFeaturePresent = OALIsProcessorFeaturePresent;
    
	
	// Provide the board-specific SHUTDOWN handler
	//
	g_pfnBSPIoCtlHalShutdown = BSPIoCtlHalShutdown;

	//initialization of the resources used by the kernel's suspend code
	//	

    OEMPowerManagerInit();

    // Initialize the KITL connection if required.
    //
    RETAILMSG(1,(TEXT("OALKitlStart\r\n")));
    OALKitlStart();

    RETAILMSG(1,(TEXT("Firmware Init Done.\r\n")));
    
	//Setup the hardware watchdog... The hardware watchdog would fire after DEFAULT_WATCHDOG_PERIOD ms.
	//We set the refreshing thread period to be half the hardware watchdog period (so that it's not to sensitive to a heavy load of the CPU).
	dwOEMWatchDogPeriod = OEMInitWatchDogTimer(DEFAULT_WATCHDOG_PERIOD) / 2;
	
	//Tell the kernel to enumerate our other SDRAM areas
	pNKEnumExtensionDRAM = OEMEnumExtensionDRAM;

	// Let default value for the HMatrix
	// You can easily modify the HMatrix register value by deleting this comment
	// and editing the g_MatrixSlaveConfig and g_MatrixMasterConfig tables
	// in file SRC/INC/at91sam9263ek_hmaxtrix.h
	//RETAILMSG(1,(TEXT("Configuring HMATRIX\r\n")));
	//AT91SAM926x_ConfigureHMatrix(pMatrix,g_MatrixSlaveConfig,AT91SAM9263_NB_SLAVES,g_MatrixMasterConfig,AT91SAM9263_NB_MASTER);
}



//! @}
//-----------------------------------------------------------------------------
// End of $URL: http://centaure/svn/interne-ce_bsp_atmel/TRUNK50/PLATFORM/AT91SAM9263EK/SRC/KERNEL/OAL/init.c $
//-----------------------------------------------------------------------------
//
