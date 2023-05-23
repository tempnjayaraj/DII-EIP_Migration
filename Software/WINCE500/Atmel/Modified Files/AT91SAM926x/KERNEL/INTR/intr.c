//-----------------------------------------------------------------------------
//! \addtogroup	OAL
//! @{
//!
//  All rights reserved ADENEO SAS 2005
//!
//-----------------------------------------------------------------------------
//! \file		AT91SAM926x/KERNEL/INTR/intr.c
//!
//! \brief		Implements the interrupt engine
//!
//! \if subversion
//!   $URL: http://centaure/svn/interne-ce_bsp_atmel/TAGS/TAGS50/SAM9263EK_v100_rc4/PLATFORM/COMMON/SRC/ARM/ATMEL/AT91SAM926x/KERNEL/INTR/intr.c $
//!   $Author: Woodlandk $
//!   $Revision: 1 $
//!   $Date: 1/29/09 3:26p $
//! \endif
//! 
//-----------------------------------------------------------------------------

//! \addtogroup	INTR
//! @{

#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <oal_intr.h>

// Controllers includes
#include "AT91SAM926x.h"
#include "lib_AT91SAM926x.h"


// APIs includes
#include "intr.h"
#include "AT91SAM926x_oal_intr.h"
#include "AT91SAM926x_oal_timer.h"
#include "pio_intr.h"

//------------------------------------------------------------------------------
//
//  Globals.

//Profiler interrupt handler
PFN_PROFILER_ISR g_pProfilerISR = NULL;

//Profiler IRQ
DWORD g_dwProfilerIRQ = 0xFFFFFFFF;

//Interrupts save for future restoration
static DWORD AICResumeMask;
static AT91PS_AIC pAIC = 0;
AT91PS_SYS pSYS = 0;

//-----------------------------------------------------------------------------
//! \fn			  BOOL OALIntrInit()
//!
//! \brief		This function initialize interrupt mapping, hardware and call platform specific initialization.
//!
//!
//!
//! \return 	TRUE if OALIntrInit works well, FALSE if not
//-----------------------------------------------------------------------------
BOOL OALIntrInit()
{
    BOOL result = TRUE;
    int i;
       
    RETAILMSG(1, (L"+OALIntrInit\r\n") );
	pSYS = (AT91PS_SYS) OALPAtoVA((DWORD)AT91C_BASE_SYS,FALSE);

	// Setting pAIC
	if (pAIC == 0)
	{
		pAIC = (AT91PS_AIC) OALPAtoVA((DWORD)AT91C_BASE_AIC,FALSE);

		//Sanity Check
		if (pAIC == NULL)
		{
			RETAILMSG(1, (L"+%s OALPAtoVA((DWORD)AT91C_BASE_AIC,FALSE) failed \r\n",__FUNCTION__ ));
			return FALSE;
		}
	}
	
	//Sanity Check
	if (pSYS == NULL)
	{
		RETAILMSG(1, (L"+%s OALPAtoVA((DWORD)AT91C_BASE_SYS,FALSE) failed \r\n",__FUNCTION__ ));
		return FALSE;
	}

    // Disable all interrupts
    pSYS->SYS_AIC_IDCR = -1;
    // clear all interrupts
    pSYS->SYS_AIC_ICCR = -1;
    
    // Configure the AIC to correctly manage the interrupts from the peripheral controllers.
    // Later the user would be able to change those properties by calling a kernelIoControl (IOCTL_HAL_SET_IRQ_PROPERTIES -not implemented yet-)
    // in order for example to handle interrupt on IRQ0-IRQ6
    for (i = 0; i <= LOGINTR_AIC_LAST; i++) 
    {
		// Create an IRQ descriptor for each AIC entry
		// Init each SMR AIC entry
		pSYS->SYS_AIC_SVR[i] = i;
		pSYS->SYS_AIC_SMR[i] = AT91C_AIC_PRIOR_LOWEST | AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE;

		// Perform 8 End Of Interrupt Command to make sure AIC will not Lock out nIRQ
		if (i < 8)
			pSYS->SYS_AIC_EOICR = pSYS->SYS_AIC_EOICR;

    }

    // Initialize interrupt mapping
    OALIntrMapInit();
    
    // Initialize PIO controllers
    SOCPioIntrInit();

#ifdef OAL_BSP_CALLBACKS    
	/// \note this functions gives a chance to the BSP to perform something specific here like to initializing a subordinate controller
    result = BSPIntrInit();
#endif

    RETAILMSG(1, (L"-OALIntrInit(rc = %d)\r\n", result));
	// Disable all interrupts
	 
    return result;
}


//-----------------------------------------------------------------------------
//! \fn			  BOOL SOCDisableIrq(DWORD irq,BOOL* pOldState)
//!
//! \brief		Turns the interrupt source off.
//!				Update the register on the target which masks the irq passed to the CPU.
//!
//! \param		irq IRQ to disable
//! \param		pOldState pointer to a BOOLEAN indicating if the interrupt was previously enabled
//!
//! \return 	TRUE if irq is disable, FALSE if irq hasn't be found
//-----------------------------------------------------------------------------
BOOL SOCDisableIrq(DWORD irq,BOOL* pOldState)
{
		// RETAILMSG(1, ( L"+SOCDisableIrq(%d)\r\n", irq ));
    	// Depending on IRQ number use AIC or PIO controler register
        if ((irq >= LOGINTR_AIC_FIRST) && (irq <= LOGINTR_AIC_LAST)) {
            // Use AIC interrupt register
//            RETAILMSG(1, ( L"+SOCDisableIrq(%d) AIC mask %x\r\n", irq , (1<<irq)));
			if (pOldState!=NULL)
			{
				*pOldState = pAIC->AIC_IMR & (1<<irq);
			}
            pAIC->AIC_IDCR = (1<<irq);
        } 
        else 
        {
        	//try to use PIO registers
        	if (SOCDisablePioIrq(irq, pOldState) == FALSE)
        		return FALSE;
           
        }
	return TRUE;
}


//-----------------------------------------------------------------------------
//! \fn			BOOL SOCEnableIrq(DWORD irq)
//!
//! \brief		Turns the interrupt source on
//!				Update the register on the target which masks the irq passed to the CPU.
//!
//! \param		irq IRQ to disable
//!
//! \return 	TRUE if irq is enable, FALSE if irq hasn't be found
//-----------------------------------------------------------------------------
BOOL SOCEnableIrq(DWORD irq)
{	
	// Depending on IRQ number use AIC or PIO controler register
        if ((irq >= LOGINTR_AIC_FIRST) && (irq <= LOGINTR_AIC_LAST)) {
            // Use AIC interrupt register
            //RETAILMSG(1, ( L"+SOCEnableIrq(%d) AIC mask %x\r\n", irq , (1<<irq)));
            pAIC->AIC_IECR = (1<<irq);
        } 
        else 
        {
        	//try to use PIO registers
        	if (SOCEnablePioIrq(irq) == FALSE)
        		return FALSE;
           
        }
	return TRUE;
}

//-----------------------------------------------------------------------------
//! \fn			  void SOCSaveAndDisableAllIntrBeforeSuspend()
//!
//! \brief		Save interrupts mask and turn all interrupt sources off.
//!				This function is called before entering suspend.
//!
//!
//!
//!
//-----------------------------------------------------------------------------
void SOCSaveAndDisableAllIntrBeforeSuspend()
{
	RETAILMSG(1, ( L"+SOCSaveAndDisableAllIntrBeforeSuspend()\r\n" )); 	
 	AICResumeMask = pAIC->AIC_IMR;
	pAIC->AIC_IDCR = 0xFFFFFFFF; //mask all the interrupts
	SOCPioSaveAndDisableAllIntrBeforeSuspend();
	SOCPioControllerIntrEnable(); //Leave the interrupt for the pio controllers !
}

//-----------------------------------------------------------------------------
//! \fn			void SOCRestoreAllIntrAfterSuspend()
//!
//! \brief		retore the previously backed-up interrupt mask (\sa SOCSaveAndDisableAllIntrBeforeSuspend)
//!				This function is called juste before leaving suspend mode
//!
//!
//!
//!
//!
//-----------------------------------------------------------------------------
void SOCRestoreAllIntrAfterSuspend()
{
	RETAILMSG(1, ( L"+SOCRestoreAllIntrAfterSuspend()\r\n" ));
	pAIC->AIC_IECR = AICResumeMask;
	SOCPioRestoreAllIntrAfterSuspend();	
}

//-----------------------------------------------------------------------------
//! \fn			BOOL OALIntrRequestIrqs(DEVICE_LOCATION *pDevLoc, UINT32 *pCount, UINT32 *pIrqs)
//!
//! \brief		This function returns IRQs for CPU/SoC devices based on their physical address.
//!
//! \param		pDevLoc	Pointer to device location wich request IRQ
//! \param		pCount	Number of IRQ request
//!	\param		pIrqs	Returns table of IRQs
//!
//! \return		TRUE if IRQs requested are allocated, FALSE if not
//-----------------------------------------------------------------------------
BOOL OALIntrRequestIrqs(DEVICE_LOCATION *pDevLoc, UINT32 *pCount, UINT32 *pIrqs)
{
    BOOL rc = FALSE;

   OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIntrRequestIrqs(0x%08x->%d/%d/0x%08x/%d, 0x%08x, 0x%08x)\r\n",
        pDevLoc, pDevLoc->IfcType, pDevLoc->BusNumber, pDevLoc->LogicalLoc,
        pDevLoc->Pin, pCount, pIrqs
    ));

    // This shouldn't happen
    if (*pCount >= 1)
	{
	#ifdef OAL_BSP_CALLBACKS
		/// \note this functions gives a chance to the BSP to perform something specific here
    	rc = BSPIntrRequestIrqs(pDevLoc, pCount, pIrqs);
	#endif    
	}
       
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrRequestIrqs(rc = %d)\r\n", rc));
    return rc;
}


//-----------------------------------------------------------------------------
//! \fn			BOOL OALIntrEnableIrqs(UINT32 count, const UINT32 *pIrqs)
//!
//! \brief		OAL function to enable IRQs based on their physicall interrupt number
//!
//! \param		count	size of pIrqs
//!	\param		pIrqs	table of IRQs
//!
//! \return		TRUE if all IRQs are enabled, FALSE if not
//-----------------------------------------------------------------------------
BOOL OALIntrEnableIrqs(UINT32 count, const UINT32 *pIrqs)
{
    BOOL rc = TRUE;
    UINT32 i, irq;

  //  RETAILMSG(1, (        L"+OALIntrEnableIrqs(%d, 0x%08x)\r\n", count, pIrqs ));

    for (i = 0; i < count; i++) {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
		/// \note this functions gives a chance to the BSP to perform something specific here like enabling irq on subordinate interrupt controller
        irq = BSPIntrEnableIrq(pIrqs[i]);
#endif
        if (irq == OAL_INTR_IRQ_UNDEFINED) continue;
        
        if (SOCEnableIrq(irq) == FALSE)
        	rc = FALSE;
    }        

      //RETAILMSG(1, (L"-OALIntrEnableIrqs (rc=%d) (AIC_IMR = 0x%x)\r\n",rc,AT91C_VA_BASE_SYS->AIC_IMR));
 return rc;    
}


//-----------------------------------------------------------------------------
//! \fn			VOID OALIntrDisableIrqs(UINT32 count, const UINT32 *pIrqs)
//!
//! \brief		OAL function to disable IRQs based on their physicall interrupt number
//!
//! \param		count	size of pIrqs
//! \param		pIrqs	table of IRQs
//!
//!
//-----------------------------------------------------------------------------
VOID OALIntrDisableIrqs(UINT32 count, const UINT32 *pIrqs)
{
    UINT32 i, irq;

//    RETAILMSG(1, (
//        L"+OALIntrDisableIrqs(%d, 0x%08x)\r\n", count, pIrqs
//    ));

    for (i = 0; i < count; i++) {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
		/// \note this functions gives a chance to the BSP to perform something specific here like disabling irq on subordinate interrupt controller
        irq = BSPIntrDisableIrq(pIrqs[i]);
#endif
        if (irq == OAL_INTR_IRQ_UNDEFINED) continue;
        
        SOCDisableIrq(irq, 0);
    }        
    //RETAILMSG(1, (L"-OALIntrDisableIrqs (AIC_IMR = 0x%x)\r\n",AT91C_VA_BASE_SYS->AIC_IMR));
}



//-----------------------------------------------------------------------------
//! \fn			VOID OALIntrDoneIrqs(UINT32 count, const UINT32 *pIrqs)
//!
//! \brief		OAL function to finish IRQs based on their physicall interrupt number
//!				On this architecture this is mostly the same as enabling the interrupt
//!
//! \param		count	size of pIrqs
//!	\param		pIrqs	table of IRQs
//!
//!
//-----------------------------------------------------------------------------
VOID OALIntrDoneIrqs(UINT32 count, const UINT32 *pIrqs)
{
    UINT32 i, irq;

   // RETAILMSG(1, (   L"+OALIntrDoneIrqs(%d, 0x%08x)\r\n", count, pIrqs));

    for (i = 0; i < count; i++) {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
		/// \note this functions gives a chance to the BSP to perform something specific here like finishing the irq on subordinate interrupt controller
        irq = BSPIntrDoneIrq(pIrqs[i]);
#endif    
       if (irq == OAL_INTR_IRQ_UNDEFINED) continue;
        
        // Depending on IRQ number use AIC or PIO controler register
        
        SOCEnableIrq(irq);
     }


    //RETAILMSG(1, (L"-OALIntrDoneIrqs (AIC_IMR = 0x%x)\r\n",AT91C_VA_BASE_SYS->AIC_IMR));
}


//-----------------------------------------------------------------------------
//! \fn			int OEMInterruptHandler(unsigned int ra)
//!
//! \brief		This is the main Interrupt handler
//!
//! \param		ra is reserved for future use. In case of a kernel with profiler enabled it's the location of the PC when the interrupt occured
//!
//!	\return		SYSIntr corresponding to Interrupt occured
//!
//!
//-----------------------------------------------------------------------------
int OEMInterruptHandler(unsigned int ra)
{
	static AT91PS_SYS pSys = NULL;	   	
	ULONG nSysIntr = SYSINTR_NOP;
	DWORD nIVR;
    extern volatile UINT32 g_oalLastSysIntr;
	DWORD dwTempCountSinceSysTick = OALTimerCountsSinceSysTick();

	//Sanity Check
	if (pSys == NULL)
	{
		pSys = (AT91PS_SYS) OALPAtoVA((DWORD)AT91C_BASE_SYS,FALSE);
		if (pSys == NULL)
		{
			RETAILMSG(1, (L"+%s OALPAtoVA((DWORD)AT91C_BASE_SYS,FALSE) failed \r\n",__FUNCTION__ ));
			return SYSINTR_NOP;
		}
	}

	nIVR = pSys->SYS_AIC_IVR;

    /// First aknowledge the interrupt at the AIC level
	pSys->SYS_AIC_EOICR = pSys->SYS_AIC_EOICR;

	/// Then check if it's a system controller interrupt
	if (nIVR == AT91C_ID_SYS) 
	{
		/// if a System Timer Interrupt occured, then call the timer interrupt handler
	    if (pSys->SYS_PITC_PISR & AT91C_PITC_PITS) 
	    {
//	        OALMSG(1, (L"." ));
	        nSysIntr = OALTimerIntrHandlerWithILTSupport(dwTempCountSinceSysTick);
		}
#if 0	    
	    else if (XXXXXXX) //Other system controller interrupt management
	    {
	        
	    }
#endif	    
	    
	}
	
	// if a profiler Interrupt occured, then call the profiler interrupt handler
	else if ((nIVR == g_dwProfilerIRQ) && (g_pProfilerISR))
	{
		nSysIntr = g_pProfilerISR(ra); 
	}
	//If this is another interrupt then it must be a device's interrupt
    else
    {
		DWORD oldInt = nIVR;
    	// First let the SOC-dedicated interrupt susbsytem check if this is a pio interrupt
  //  	OALMSG(1, (L"d" ));
    	nIVR = SOCIntrActiveIrq(nIVR);

#ifdef OAL_BSP_CALLBACKS
       // Then let the BSP-specific interrupt susbsytem check if it wants to say that's another interrupt number (think of secondary interrupt controller on board, then it makes sense)
        nIVR = BSPIntrActiveIrq(nIVR);
#endif
		// Mask the interrupt so that it won't occur again before being finished with SOCInterruptDone
	   	// Then find if IRQ is claimed by chain (see NKCallIntChain and LoadIntrChainHandler in PB doc)
		if (nIVR != LOGINTR_UNDEFINED)
		{
			SOCDisableIrq(nIVR, 0);
			nSysIntr = NKCallIntChain((UCHAR)nIVR);
			
			if (nSysIntr == SYSINTR_CHAIN || !NKIsSysIntrValid(nSysIntr))
			{
			SOCDisableIrq(nIVR, 0);
			// If the IRQ wasn't claimed by the chain, we use static LOGINTR -> SYSINTR mapping.
				nSysIntr = OALIntrTranslateIrq(nIVR);
			}
	
			if (nSysIntr == SYSINTR_NOP)
			{
			// If not a valid sysintr then put the inteerupt back to normal (enabled)
			// \note what's the point to enabling an interrupt that's handled ?.......
			// answer is : To do some ISR treatment only !
			// \warning Beware that this a flaw in the robustness of the kernel; If an interrupt is triggered 
			// and not acknowledged at the peripheral level, it may trigger again and again ... and again. And the system will be stopped in this loop
				EdbgOutputDebugString( "Reenabling interrupt %d (nSysIntr %d)\r\n",nIVR,nSysIntr);
				SOCEnableIrq(nIVR);
				
			} 
			else  if (nSysIntr == SYSINTR_UNDEFINED)
			{
				//EdbgOutputDebugString( "Undefined Sysintr %d (nSysIntr %d)\r\n",nIVR,nSysIntr);
				nSysIntr = SYSINTR_NOP;
			}
			else  if (!NKIsSysIntrValid(nSysIntr))
			{
				//EdbgOutputDebugString( "Invalid Sysintr %d (nSysIntr %d)\r\n",nIVR,nSysIntr);
				nSysIntr = SYSINTR_NOP;
			}
		}
	}
	
    // save the last Sysintr value. It's used by the suspend system and the fake idle routine
	g_oalLastSysIntr = nSysIntr; 
    
	return nSysIntr;
}


//! @} end of subgroup INTR

//! @} end of group OAL

////////////////////////////////////////////////////////////////////////////////
// End of $URL: http://centaure/svn/interne-ce_bsp_atmel/TAGS/TAGS50/SAM9263EK_v100_rc4/PLATFORM/COMMON/SRC/ARM/ATMEL/AT91SAM926x/KERNEL/INTR/intr.c $
////////////////////////////////////////////////////////////////////////////////
//

