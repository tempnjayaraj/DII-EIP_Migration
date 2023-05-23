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
//!   $Author: Woodlandk $
//!   $Revision: 1 $
//!   $Date: 1/29/09 3:26p $
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

extern DWORD SC_GetTickCount();
extern BOOL BSPIoCtlHalShutdown (UINT32 code, VOID *pInpBuffer,UINT32 inpSize, VOID *pOutBuffer,UINT32 outSize, UINT32 *pOutSize);

UINT32 OALGetTickCount()
{
    return SC_GetTickCount();
}

// Here we give the list of our SDRAM areas to the kernel
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

extern DWORD GetPLLAFreq(AT91PS_PMC);
extern DWORD GetPLLBFreq(AT91PS_PMC);

/// Initialize Hardware Interfaces needed by the kernel
///
/// OEMInit is called by the kernel after it has performed minimal
/// initialization. Interrupts are disabled and the kernel is not
/// ready to handle exceptions. 
void OEMInit(void)
{
	PDRIVER_GLOBALS pDrvGlobalArea;	
	AT91PS_RTTC pRTTC0 = (AT91PS_RTTC) OALPAtoVA((DWORD) AT91C_BASE_RTTC0, FALSE);
 	AT91PS_RTTC pRTTC1 = (AT91PS_RTTC) OALPAtoVA((DWORD) AT91C_BASE_RTTC1, FALSE);

	extern DWORD dwOEMWatchDogPeriod;

// Denotes TestApp build of kernel
#ifdef FAKE_STRONGARM
	AT91PS_PIO pPIOC = (AT91PS_PIO) OALPAtoVA((DWORD) AT91C_BASE_PIOC, FALSE);
	AT91PS_PIO pPIOE = (AT91PS_PIO) OALPAtoVA((DWORD) AT91C_BASE_PIOE, FALSE);
	AT91PS_RSTC pRSTC = (AT91PS_RSTC) OALPAtoVA((DWORD) AT91C_BASE_RSTC, FALSE);

    // Needed for eVc 4.0 debug
    extern DWORD CEProcessorType;
    CEProcessorType=PROCESSOR_STRONGARM;

    // A reset of the external chip is necessary to fix the ethernet PHY's addresses
	pPIOC->PIO_PER = AT91C_PIO_PC25;
	pPIOC->PIO_OER = AT91C_PIO_PC25;
	pPIOC->PIO_CODR = AT91C_PIO_PC25;
	pPIOE->PIO_PER = (AT91C_PIO_PE25 | AT91C_PIO_PE26 | AT91C_PIO_PE22);
	pPIOE->PIO_OER = (AT91C_PIO_PE25 | AT91C_PIO_PE26 | AT91C_PIO_PE22);
	pPIOE->PIO_CODR = (AT91C_PIO_PE25 | AT91C_PIO_PE26 | AT91C_PIO_PE22);

	pRSTC->RSTC_RCR = (0xA5 << 24 ) | AT91C_RSTC_EXTRST;
#else
    // Turn off Serial Debug Ouput from this point on
    g_pDBGU = NULL;
#endif

	RETAILMSG(1,(TEXT("Windows CE Firmware Init\r\n")));	
	RETAILMSG(1,(TEXT("BSP %d.%d.%d for the AT91SAM9263EK board (built %s)\r\nAdaptation performed by ADENEO (c) 2005\r\n"),BSP_VERSION_MAJOR, BSP_VERSION_MINOR, BSP_VERSION_RELEASE, TEXT(__DATE__)));

    // Initialize interrupts.
    if (!OALIntrInit())
    {
        OALMSG(OAL_ERROR, (L"ERROR: OEMInit: failed to initialize interrupts.\r\n"));
    }

    // Initializes the global variables for the use of cache
	OALCacheGlobalsInit();
    
	// Initialize driver globals area
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

    // Always force a clean boot, reset not supported
	NKForceCleanBoot();
	
	// Intialize optional kernel functions.
	pOEMIsProcessorFeaturePresent = OALIsProcessorFeaturePresent;
	
	// Provide the board-specific SHUTDOWN handler
	g_pfnBSPIoCtlHalShutdown = BSPIoCtlHalShutdown;

	// Initialization of the resources used by the kernel's suspend code
    OEMPowerManagerInit();

    // Initialize the KITL connection if required.
    OALKitlStart();

    RETAILMSG(1,(TEXT("Firmware Init Done.\r\n")));
    
	// Setup the hardware watchdog... The hardware watchdog would fire after DEFAULT_WATCHDOG_PERIOD ms.
	// We set the refreshing thread period to be half the hardware watchdog period (so that it's not to sensitive to a heavy load of the CPU).
	dwOEMWatchDogPeriod = OEMInitWatchDogTimer(DEFAULT_WATCHDOG_PERIOD) / 2;
	
	// Tell the kernel to enumerate our other SDRAM areas
	pNKEnumExtensionDRAM = OEMEnumExtensionDRAM;
}



//! @}
//-----------------------------------------------------------------------------
// End of $URL: http://centaure/svn/interne-ce_bsp_atmel/TRUNK50/PLATFORM/AT91SAM9263EK/SRC/KERNEL/OAL/init.c $
//-----------------------------------------------------------------------------
//
