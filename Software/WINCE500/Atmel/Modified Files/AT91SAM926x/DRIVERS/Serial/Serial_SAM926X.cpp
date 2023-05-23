//-----------------------------------------------------------------------------
//! \addtogroup DRIVERS
//! @{
//!
//  All rights reserved ADENEO SAS 2005
//!
//-----------------------------------------------------------------------------
//! \file		Serial_SAM926X.cpp
//!
//! \brief		Serial driver for AT91SAM926x chipset
//!
//! \if subversion
//!   $URL: http://centaure/svn/interne-ce_bsp_atmel/TAGS/TAGS50/SAM9263EK_v100_rc4/PLATFORM/COMMON/SRC/ARM/ATMEL/AT91SAM926x/DRIVERS/Serial/Serial_SAM926X.cpp $
//!   $Author: Tenneyd $
//!   $Revision: 2 $
//!   $Date: 8/28/09 10:50a $
//! \endif
//!
//!	\remarks
//!		Defines the entry point for the DLL application and for the driver.
//!									
//! \todo 
//!		Supports the controle of the pins DTR, DCD, RI, CI 
//-----------------------------------------------------------------------------
//! \addtogroup	Serial
//! @{
//!



//------------------------------------------------------------------------------
//                                                                      Includes
//------------------------------------------------------------------------------
// Standard includes
#include <windows.h>
#include <ceddk.h>
#include <Devload.h>
#include <nkintr.h>
#include <linklist.h>
#include <pegdser.h>
#include <serdbg.h>

// Controller includes
#include "AT91SAM926x_oal_ioctl.h"

// Project include
#include "Serial_SAM926X.h"
#include "Serial_SAM926X_HW.h"



// DEBUG ZONE
#ifdef DEBUG
#include "Serial_SAM926X_DbgZones.h"
DBGPARAM dpCurSettings = {
    TEXT("Serial"), 
	{
        TEXT("Init"),
		TEXT("Open"),
		TEXT("Read"),
		TEXT("Write"),
        TEXT("Close"),
		TEXT("Ioctl"),
		TEXT("Thread"),
		TEXT("Events"),
        TEXT("CritSec"),
		TEXT("FlowCtrl"),
		TEXT("Infrared"),
		TEXT("User Read"),
        TEXT("Alloc"),
		TEXT("Function"),
		TEXT("Warning"),
		TEXT("Error")
	},
    DBG_INIT | DBG_OPEN | DBG_READ | DBG_WRITE | DBG_CLOSE | DBG_IOCTL | DBG_EVENTS | DBG_CRITSEC | DBG_ALLOC | DBG_ERROR |DBG_WARNING |DBG_FUNCTION
};
#endif

//------------------------------------------------------------------------------
//                                                               Defines & Types
//------------------------------------------------------------------------------
#define REG_SERIALPORTINDEX_VAL_NAME		TEXT("SerialPortIndex")		///< Registry key of the device ID

#define REG_SERIALPORTTXBUFFER_SIZE_NAME	TEXT("TxBufferSize")
#define REG_SERIALPORTTXBUFFER_COUNT_NAME	TEXT("TxBufferCount")

#define REG_SERIALPORTRXBUFFER_SIZE_NAME	TEXT("RxBufferSize")

//------------------------------------------------------------------------------
//                                                            Imported functions
//------------------------------------------------------------------------------

// Value of masterclock
extern DWORD g_dwMasterClock;
// Extern function
extern DWORD SerialIST( LPVOID lpvParam );

extern "C" void DisableInt(void);
extern "C" void EnableInt(void);

//------------------------------------------------------------------------------
//                                                 Internal functions prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Note : ONLY static

// Local function
static BOOL ApplyDCB (T_SERIAL_INIT_STRUCTURE* pSerialInitStructure, DCB *pDCB);
static BOOL WaitCommEvent(T_SERIAL_OPEN_STRUCTURE*   pOpenHead, PULONG	pfdwEventMask, LPOVERLAPPED	Unused );
static BOOL GetRegistryData (LPCTSTR Identifier, HANDLE pInitStructure );

//------------------------------------------------------------------------------
//                                                            Exported functions
//------------------------------------------------------------------------------
VOID EvaluateEventFlag(PVOID pHead, ULONG fdwEventMask);

//------------------------------------------------------------------------------
//! \fn		BOOL DllEntry(HINSTANCE   hinstDll, DWORD   dwReason, LPVOID  lpReserved)		 
//!
//! \brief	Dll entry point for serial driver     
//! 
//! \param	hinstDll	Instance pointer
//! \param	dwReason	Reason routine is called
//! \param	lpReserved	System parameter
//!
//!
//! \return		True when all is good
//------------------------------------------------------------------------------
BOOL DllEntry (
			  HINSTANCE   hinstDll,
			  DWORD   dwReason,
			  LPVOID  lpReserved
			  )
{
    if ( dwReason == DLL_PROCESS_ATTACH ) {
        DEBUGREGISTER(hinstDll);
        DEBUGMSG (ZONE_INIT, (TEXT("serial port process attach\r\n")));
        DisableThreadLibraryCalls((HMODULE) hinstDll);
    }

    if ( dwReason == DLL_PROCESS_DETACH ) {
        DEBUGMSG (ZONE_INIT, (TEXT("process detach called\r\n")));
    }

    return(TRUE);
}

//------------------------------------------------------------------------------
//! \fn					HANDLE COM_Init ( ULONG   Identifier )		 
//!
//! \brief				Serial device initialization     
//! 
//! \param	Identifier	Port identifier.  The device loader passes 
//!						in the registry key that contains information
//!						about the active device.
//!
//!	\remarks			This routine is called at device load time in order
//!	 					to perform any initialization.	 Typically the init
//!						routine does as little as possible, postponing memory
//!						allocation and device power-on to Open time.
//!
//! \return				Returns a pointer to the serial head which is passed into 
//!	 					the COM_OPEN and COM_DEINIT entry points as a device handle.
//! \return	\e			NULL to indicate an error. 
//------------------------------------------------------------------------------
HANDLE COM_Init (
				 ULONG   Identifier
				)
{
	HKEY hKey = NULL;
	T_SERIAL_INIT_STRUCTURE *pSerialInitStructure = NULL;
	DWORD dwBufferIndex;

    DEBUGMSG (ZONE_INIT | ZONE_FUNCTION, (TEXT("+COM_Init\r\n")));

	// Allocate Init structure
	pSerialInitStructure = (T_SERIAL_INIT_STRUCTURE *) LocalAlloc(LPTR,sizeof(T_SERIAL_INIT_STRUCTURE));
	if (pSerialInitStructure == NULL)
	{
		goto error;
	}

	if(GetRegistryData((LPCTSTR) Identifier, pSerialInitStructure) == FALSE)
	{
		goto error;
	}

	InitializeCriticalSection(&pSerialInitStructure->csSerialInitStructure);
	InitializeCriticalSection(&pSerialInitStructure->csUsartReg);
	InitializeCriticalSection(&pSerialInitStructure->csErrorFlags);
	InitializeCriticalSection(&pSerialInitStructure->csNbBytesToSend);
		
     // Initially, open list is empty.
    InitializeListHead( &pSerialInitStructure->OpenList );
    pSerialInitStructure->pReadAccessOwner = NULL;
	pSerialInitStructure->dwOpenCnt = 0;
	pSerialInitStructure->dwErrorFlags = 0;

	// Get master clock reference value;
	KernelIoControl(IOCTL_HAL_MASTERCLOCK, NULL, 0, &g_dwMasterClock, sizeof(g_dwMasterClock), 0);

	// Call hardware init
	if (HWInit(pSerialInitStructure) == FALSE)
	{
		goto error;
	}

	///---------------------------------------------------------------------------------
	/// Interrupt Init

	// Request sysintr for the device
	pSerialInitStructure->dwSysIntr = SYSINTR_UNDEFINED;
	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, (PVOID)&pSerialInitStructure->dwDeviceID, sizeof(pSerialInitStructure->dwDeviceID), (PVOID)&pSerialInitStructure->dwSysIntr, sizeof(pSerialInitStructure->dwSysIntr), NULL))
	{
		RETAILMSG(1, (TEXT("ERROR: Failed to request the serial sysintr for USART XX.\r\n")));
		pSerialInitStructure->dwSysIntr = SYSINTR_UNDEFINED;
		goto error;
	}

	// Create serial waiting thread loop
	pSerialInitStructure->bSerialISTRun = TRUE;
	pSerialInitStructure->hSerialIST = CreateThread(  NULL,							   // Security
													  0,							   // No Stack Size
													  SerialIST,					   // Interrupt Thread
													  (LPVOID)pSerialInitStructure,    // Init stucture give by parameters
													  CREATE_SUSPENDED,				   // Create Suspended
													  NULL							   // Thread Id
												   );

	CeSetThreadPriority(pSerialInitStructure->hSerialIST, 101);

	pSerialInitStructure->hSerialEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// Initialize interuption and associate it to a Event
    if ( !InterruptInitialize(pSerialInitStructure->dwSysIntr,
                              pSerialInitStructure->hSerialEvent,
                              NULL,
                              0))
	{
        DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("Error initializing interrupt\n\r")));
        goto error;
    }

	// Valid interrupt to be sure.
    InterruptDone(pSerialInitStructure->dwSysIntr);


	//---------------------------------------------------------------------------------
	// DMA Init

	// Initialize DMA Adapter object
	pSerialInitStructure->dmaAdapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
	pSerialInitStructure->dmaAdapter.InterfaceType = PCIBus;
	pSerialInitStructure->dmaAdapter.BusNumber = 0;

	//---------------------------------------------------------------------------------
	// Allocate TX DMA buffer
	pSerialInitStructure->pDmaTxBufferVA = new PVOID[pSerialInitStructure->dwTxBufferCount];
	pSerialInitStructure->pDmaTxBufferPA = new PHYSICAL_ADDRESS[pSerialInitStructure->dwTxBufferCount];

	pSerialInitStructure->pDmaTxBufferVA[0] = HalAllocateCommonBuffer  (
											&pSerialInitStructure->dmaAdapter,
											pSerialInitStructure->dwTxBufferSize * pSerialInitStructure->dwTxBufferCount,
											pSerialInitStructure->pDmaTxBufferPA,
											FALSE
											);

	// if TX buffer allocation failed, return error
	if (pSerialInitStructure->pDmaTxBufferVA[0] == NULL)
	{
		DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("Error during DMA TX buffer allocation\n\r")));

		goto error;
	}

	// if successfull, set pointer on total allocate buffer to obtain smaller DMA buffer
	for (dwBufferIndex = 1; dwBufferIndex < pSerialInitStructure->dwTxBufferCount; ++dwBufferIndex)
	{
		pSerialInitStructure->pDmaTxBufferVA[dwBufferIndex]	= ((PCHAR)pSerialInitStructure->pDmaTxBufferVA[0]) + (dwBufferIndex * pSerialInitStructure->dwTxBufferSize);
		pSerialInitStructure->pDmaTxBufferPA[dwBufferIndex].QuadPart = pSerialInitStructure->pDmaTxBufferPA[0].QuadPart + (dwBufferIndex * pSerialInitStructure->dwTxBufferSize);
	}

	//---------------------------------------------------------------------------------
	// Allocate RX DMA buffer
	
	pSerialInitStructure->pDmaRxBufferPA = (PHYSICAL_ADDRESS*)LocalAlloc(LPTR, sizeof(PHYSICAL_ADDRESS) );
	pSerialInitStructure->pDmaRxBufferStart = (UCHAR *)HalAllocateCommonBuffer  (
												&pSerialInitStructure->dmaAdapter,
												pSerialInitStructure->dwRxBufferSize,
												pSerialInitStructure->pDmaRxBufferPA,
												FALSE
											);

	if (pSerialInitStructure->pDmaRxBufferStart == NULL)
	{
		DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("Error during DMA RX buffer allocation\n\r")));
		// Free TX DMA buffer
		HalFreeCommonBuffer (
							&pSerialInitStructure->dmaAdapter,
							pSerialInitStructure->dwTxBufferSize,
							*(pSerialInitStructure->pDmaTxBufferPA),
							pSerialInitStructure->pDmaTxBufferVA,
							FALSE
							);
		goto error;
	}

	pSerialInitStructure->dwPADmaRxBufferStart  = pSerialInitStructure->pDmaRxBufferPA->LowPart;
	// Initialisation of all used pointer
	pSerialInitStructure->dwPAControllerRxData  = pSerialInitStructure->dwPADmaRxBufferStart;
	pSerialInitStructure->dwPAUserRxData		= pSerialInitStructure->dwPADmaRxBufferStart;
	pSerialInitStructure->dwPADmaRxBufferEnd	= pSerialInitStructure->dwPADmaRxBufferStart + pSerialInitStructure->dwRxBufferSize;
	pSerialInitStructure->dwPADmaRxBufferTrueEnd= pSerialInitStructure->dwPADmaRxBufferEnd;
	pSerialInitStructure->dwPADmaRxBufferPRLimit= pSerialInitStructure->dwPADmaRxBufferStart + ((pSerialInitStructure->dwRxBufferSize / 10) * 8);

	// Compute buffer number characters to active flow control
	pSerialInitStructure->dwRxFCLLimit = ((pSerialInitStructure->dwRxBufferSize / 10) * 7);	// number of characters in buffer low limit for Flow control (to stop receive)
	pSerialInitStructure->dwRxFCHLimit = ((pSerialInitStructure->dwRxBufferSize / 10) * 5);	// number of characters in buffer high limit for Flow control (to re-start receive)

	DEBUGMSG(ZONE_INIT, (TEXT("RX flow control low limit = %u buffers\r\n"), pSerialInitStructure->dwRxFCLLimit));
	DEBUGMSG(ZONE_INIT, (TEXT("RX flow control high limit = %u buffers\r\n"), pSerialInitStructure->dwRxFCHLimit));


	// Create read and write mutex  exclusion.
	pSerialInitStructure->hReadMutex = CreateMutex(NULL, FALSE, NULL);
	pSerialInitStructure->hWriteMutex = CreateMutex(NULL, FALSE, NULL);

	// If init succefull, start serial IST
	ResumeThread(pSerialInitStructure->hSerialIST);

	goto exit;

error:
	DEBUGMSG (ZONE_ERROR , (TEXT("COM_Init Failed\r\n")));

	// Release sysintr id necessary
	if (pSerialInitStructure->dwSysIntr != SYSINTR_UNDEFINED)
	{
		InterruptDisable(pSerialInitStructure->dwSysIntr);
		if (!KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, (PVOID)&pSerialInitStructure->dwSysIntr, sizeof(pSerialInitStructure->dwSysIntr), NULL, 0, NULL))
		{
			RETAILMSG(1, (TEXT("ERROR: Failed to release the serial sysintr for USART XX.\r\n")));
		}
	}
	
	// Delete Init structure if necessary
	if (pSerialInitStructure)
	{
		LocalFree(pSerialInitStructure);		
		pSerialInitStructure = NULL;
	}	

exit:
    DEBUGMSG (ZONE_INIT | ZONE_FUNCTION, (TEXT("-COM_Init\r\n")));
    return(pSerialInitStructure);

}

//------------------------------------------------------------------------------
//! \fn					HANDLE COM_Open ( HANDLE  pHead, DWORD   AccessCode, DWORD   ShareMode )		 
//!
//! \brief				Open the serial device.    
//! 
//! \param	pHead		Handle returned by COM_Init.
//! \param	AccessCode	Access code.
//! \param	ShareMode	share mode - Not used in this driver.
//!
//!	\remarks			This routine must be called by the user to open the
//!						serial device. The HANDLE returned must be used by the application in
//!						all subsequent calls to the serial driver. This routine starts the thread
//!						which handles the serial events.
//!						Exported to users.
//!
//! \return				This routine returns a HANDLE representing the device.
//! \return	\e			NULL to indicate an error. 
//------------------------------------------------------------------------------
HANDLE COM_Open (
				HANDLE  pHead,
				DWORD   AccessCode,
				DWORD   ShareMode
				)
{
    T_SERIAL_INIT_STRUCTURE *pSerialInitStructure = (T_SERIAL_INIT_STRUCTURE*)pHead;
    T_SERIAL_OPEN_STRUCTURE *pSerialOpenStructure;

    DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("+COM_Open handle x%X, access x%X, share x%X\r\n"),
                                        pHead, AccessCode, ShareMode));

    // Test if driver initialized
    if ( !pSerialInitStructure )
	{
        RETAILMSG (1 ,(TEXT("Open attempted on uninitialized device!\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);        
        return NULL;
    }

	// Protect access to init structure by a critical section
	EnterCriticalSection(&(pSerialInitStructure->csSerialInitStructure));

    // Test if someone have already open the driver in read mode
    if ( ((AccessCode & GENERIC_READ) == GENERIC_READ) && (pSerialInitStructure->pReadAccessOwner != NULL) )
	{
		DEBUGMSG (ZONE_OPEN|ZONE_ERROR, (TEXT("Read Access already open by 0x%x!\r\n"), pSerialInitStructure->pReadAccessOwner));
		SetLastError(ERROR_INVALID_ACCESS);
		LeaveCriticalSection(&(pSerialInitStructure->csSerialInitStructure));
        return NULL;
    }
	
	// If it's first open, initialize Comm Timeout and DCB with default value
	if (pSerialInitStructure->dwOpenCnt == 0)
	{
		memset(&pSerialInitStructure->commTimeouts,0,sizeof(COMMTIMEOUTS));
		pSerialInitStructure->hRxEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		pSerialInitStructure->hTxEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		memset(&pSerialInitStructure->dcb,0,sizeof(DCB));

		// Set DCB default value
		pSerialInitStructure->dcb.DCBlength  = sizeof(DCB);
		pSerialInitStructure->dcb.BaudRate   = 9600;
		pSerialInitStructure->dcb.fBinary    = TRUE;
		pSerialInitStructure->dcb.fParity    = FALSE;

		pSerialInitStructure->dcb.fOutxCtsFlow = FALSE;
		pSerialInitStructure->dcb.fOutxDsrFlow = FALSE;
		pSerialInitStructure->dcb.fDtrControl = DTR_CONTROL_DISABLE;
		pSerialInitStructure->dcb.fDsrSensitivity = FALSE;
		pSerialInitStructure->dcb.fTXContinueOnXoff = FALSE;
		pSerialInitStructure->dcb.fOutX      = FALSE;
		pSerialInitStructure->dcb.fInX       = FALSE;
		pSerialInitStructure->dcb.fErrorChar = FALSE; //NOTE: ignored
		pSerialInitStructure->dcb.fNull      = FALSE; //NOTE: ignored
		pSerialInitStructure->dcb.fRtsControl = RTS_CONTROL_DISABLE;
		pSerialInitStructure->dcb.fAbortOnError = FALSE; //NOTE: ignored

		pSerialInitStructure->dcb.XonLim     = 512;
		pSerialInitStructure->dcb.XoffLim    = 128;

		pSerialInitStructure->dcb.ByteSize   = 8;
		pSerialInitStructure->dcb.Parity     = NOPARITY;
		pSerialInitStructure->dcb.StopBits   = ONESTOPBIT;

		pSerialInitStructure->dcb.XonChar    = X_ON_CHAR;
		pSerialInitStructure->dcb.XoffChar   = X_OFF_CHAR;
		pSerialInitStructure->dcb.ErrorChar  = ERROR_CHAR;
		pSerialInitStructure->dcb.EofChar    = E_OF_CHAR;
		pSerialInitStructure->dcb.EvtChar    = EVENT_CHAR;

		
		pSerialInitStructure->dwPAControllerRxData  = pSerialInitStructure->dwPADmaRxBufferStart;
		pSerialInitStructure->dwPAUserRxData		= pSerialInitStructure->dwPADmaRxBufferStart;
		pSerialInitStructure->dwPADmaRxBufferTrueEnd= pSerialInitStructure->dwPADmaRxBufferEnd;

		// Call hardware open just when opening
		if (HWOpen(pSerialInitStructure) == FALSE)
		{
			DEBUGMSG (ZONE_OPEN|ZONE_ERROR, (TEXT("Hardware opening failure (handle= 0x%x)!\r\n"), pSerialInitStructure->pReadAccessOwner));
			LeaveCriticalSection(&(pSerialInitStructure->csSerialInitStructure));
			return NULL;
		}
	}

    // All it's ok, so lets allocate an open structure
    pSerialOpenStructure    =  (T_SERIAL_OPEN_STRUCTURE*)LocalAlloc(LPTR, sizeof(T_SERIAL_OPEN_STRUCTURE));
    if ( !pSerialOpenStructure ) 
	{
        DEBUGMSG(ZONE_INIT | ZONE_ERROR,(TEXT("Error allocating memory for pOpenHead, COM_Open failed\n\r")));
		LeaveCriticalSection(&(pSerialInitStructure->csSerialInitStructure));
        return NULL;
    }

	// If read access ask, set correct read access owner
	if ((AccessCode & GENERIC_READ) == GENERIC_READ)
	{
		pSerialInitStructure->pReadAccessOwner = pSerialOpenStructure;
	}
    DEBUGMSG(ZONE_INIT|ZONE_CLOSE,(TEXT("COM_Open: Access permission handle granted\n\r")));

    // add this open entry to list of open entries.
    InsertHeadList(&pSerialInitStructure->OpenList,
                   &pSerialOpenStructure->llist);


    // Init the serial open structure 
    pSerialOpenStructure->pSerialInit = pSerialInitStructure;  // pointer back to our parent
    pSerialOpenStructure->AccessCode = AccessCode;
    pSerialOpenStructure->ShareMode = ShareMode;
	
	pSerialOpenStructure->CommEvents.hCommEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    pSerialOpenStructure->CommEvents.fEventMask = 0;
    pSerialOpenStructure->CommEvents.fEventData = 0;
    pSerialOpenStructure->CommEvents.fAbort = 0;

	InitializeCriticalSection(&(pSerialOpenStructure->CommEvents.EventCS));

	// Increment open counter
    ++(pSerialInitStructure->dwOpenCnt);

	// Release access to init structure
    LeaveCriticalSection(&(pSerialInitStructure->csSerialInitStructure));

    DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("-COM_Open handle x%X, x%X, Ref x%X\r\n"),
                                        pSerialOpenStructure, pSerialOpenStructure->pSerialInit, pSerialInitStructure->dwOpenCnt));

    return(pSerialOpenStructure);
}


//------------------------------------------------------------------------------
//! \fn					BOOL COM_Close ( T_SERIAL_OPEN_STRUCTURE *pSerialOpenStructure )		 
//!
//! \brief				Close the serial device.   
//! 
//! \param	pSerialOpenStructure	Context pointer returned from COM_Open.
//!
//!	\remarks			This routine is called by the device manager to close the device.
//!
//! \return				TRUE if success;
//! \return	\e			FALSE if failure
//------------------------------------------------------------------------------
BOOL COM_Close (
				T_SERIAL_OPEN_STRUCTURE *pSerialOpenStructure
			   )
{
	BOOL bResult = TRUE;
	T_SERIAL_INIT_STRUCTURE *pSerialInitStructure = pSerialOpenStructure->pSerialInit;
    
    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("+COM_Close\r\n")));
    
	// Protect access to init structure var
	EnterCriticalSection(&(pSerialInitStructure->csSerialInitStructure));

	// Close hardware com port if it's the last opened
	if (pSerialInitStructure->dwOpenCnt == 1)
	{
		if (HWClose(pSerialInitStructure) == FALSE)
		{
			DEBUGMSG (ZONE_OPEN|ZONE_ERROR, (TEXT("Hardware closing failure (handle= 0x%x)!\r\n"), pSerialInitStructure->pReadAccessOwner));
			return NULL;
		}

		CloseHandle(pSerialInitStructure->hRxEvent);
		CloseHandle(pSerialInitStructure->hTxEvent);
	}

	pSerialOpenStructure->CommEvents.fAbort = 1;
    SetEvent(pSerialOpenStructure->CommEvents.hCommEvent);

	// Delete read access owner if it is the read owner
	if (pSerialOpenStructure == pSerialInitStructure->pReadAccessOwner)
	{
		pSerialInitStructure->pReadAccessOwner = NULL;
	}

    // Remove the entry from the linked list
    RemoveEntryList(&pSerialOpenStructure->llist);

	// Decrement open counter
	--(pSerialInitStructure->dwOpenCnt);

	// Liberate open structure memory
	LocalFree(pSerialOpenStructure);

	// Release access to init structure var
    LeaveCriticalSection(&(pSerialInitStructure->csSerialInitStructure));

    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("-COM_Close\r\n")));
    return(bResult);
}

//------------------------------------------------------------------------------
//! \fn					BOOL COM_Deinit(T_SERIAL_INIT_STRUCTURE* pSerialInitStructure)		 
//!
//! \brief				De-initialize serial port.   
//! 
//! \param	pSerialInitStructure	Context pointer returned from COM_Init.
//!
//! \return				TRUE if success;
//! \return	\e			FALSE if failure
//------------------------------------------------------------------------------
BOOL COM_Deinit(T_SERIAL_INIT_STRUCTURE* pSerialInitStructure)
{
    DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("+COM_Deinit\r\n")));

	// Can't do much without this
    if ( !pSerialInitStructure )
	{
        DEBUGMSG (ZONE_INIT|ZONE_ERROR, (TEXT("COM_Deinit can't find pSerialInit\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

	// Protect access to the init structure
	EnterCriticalSection(&(pSerialInitStructure->csSerialInitStructure));

	// If there is any owner, close all communication specificaly
    if ( pSerialInitStructure->dwOpenCnt ) {
        PLIST_ENTRY     pEntry;
        T_SERIAL_OPEN_STRUCTURE   *pOpenHead;

        pEntry = pSerialInitStructure->OpenList.Flink;
        while ( pEntry != &pSerialInitStructure->OpenList ) {
            pOpenHead = CONTAINING_RECORD( pEntry, T_SERIAL_OPEN_STRUCTURE, llist);
            pEntry = pEntry->Flink;  // advance to next 

            DEBUGMSG (ZONE_INIT | ZONE_CLOSE, (TEXT(" Deinit - Closing Handle 0x%X\r\n"),
                                               pOpenHead ));
            COM_Close(pOpenHead);
        }
    }

	// Stop the serial IST
	pSerialInitStructure->bSerialISTRun = FALSE;
	// And wait is death
	WaitForSingleObject(pSerialInitStructure->hSerialIST,INFINITE);

	InterruptDisable(pSerialInitStructure->dwSysIntr);


    // Call hardware deinit
	if (HWDeinit(pSerialInitStructure) == FALSE)
	{
		LeaveCriticalSection(&(pSerialInitStructure->csSerialInitStructure));
		return FALSE;
	}

    LeaveCriticalSection(&(pSerialInitStructure->csSerialInitStructure));

	CloseHandle(pSerialInitStructure->hReadMutex);
	CloseHandle(pSerialInitStructure->hWriteMutex);

    DeleteCriticalSection(&(pSerialInitStructure->csSerialInitStructure));
	DeleteCriticalSection(&(pSerialInitStructure->csUsartReg));
	DeleteCriticalSection(&(pSerialInitStructure->csNbBytesToSend));

	// Free RX DMA buffer
	HalFreeCommonBuffer (
						&pSerialInitStructure->dmaAdapter,
						pSerialInitStructure->dwRxBufferSize,
						*pSerialInitStructure->pDmaRxBufferPA,
						pSerialInitStructure->pDmaRxBufferStart,
						FALSE
						);
	

	// Free TX DMA buffer
	HalFreeCommonBuffer (
						&pSerialInitStructure->dmaAdapter,
						pSerialInitStructure->dwTxBufferSize * pSerialInitStructure->dwTxBufferCount,
						pSerialInitStructure->pDmaTxBufferPA[0],
						pSerialInitStructure->pDmaTxBufferVA[0],
						FALSE
						);


	delete[] pSerialInitStructure->pDmaTxBufferVA;
	delete[] pSerialInitStructure->pDmaTxBufferPA;

	// Try to release sysintr
	if (!KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, (PVOID)&pSerialInitStructure->dwSysIntr, sizeof(pSerialInitStructure->dwSysIntr), NULL, 0, NULL))
	{
		RETAILMSG(1, (TEXT("ERROR: Failed to release the serial sysintr for USART XX.\r\n")));
		pSerialInitStructure->dwSysIntr = SYSINTR_UNDEFINED;
	}

	// Free init structure memory
    LocalFree(pSerialInitStructure);

    DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("-COM_Deinit\r\n")));
    return(TRUE);
}








//------------------------------------------------------------------------------
//! \fn					ULONG COM_Read  ( HANDLE pHead, PUCHAR pTargetBuffer, ULONG dwBufferLength )	 
//!
//! \brief				Allows application to receive characters from serial port.   
//!
//! \param	pHead			Context pointer returned from COM_Open.
//! \param	pTargetBuffer	Pointer to valid memory.
//! \param	dwBufferLength	Size in bytes of pTargetBuffer.
//!
//!	\remarks			This routine sets the buffer and bufferlength to be used
//! 					by the reading thread. It also enables reception and controlling when
//!						to return to the user. It writes to the referent of the fourth argument
//! 					the number of bytes transacted. It returns the status of the call.
//!						Exported to users.
//! 
//! \return				number of bytes read if success;
//! \return	\e			-1 if error
//------------------------------------------------------------------------------
ULONG COM_Read  (
				HANDLE      pHead, 
		        PUCHAR      pTargetBuffer, 
				ULONG       dwBufferLength 
				)
{
    T_SERIAL_OPEN_STRUCTURE *pOpenHead = (T_SERIAL_OPEN_STRUCTURE*) pHead;
	T_SERIAL_INIT_STRUCTURE *pSerialInitStructure;
	DWORD dwBufferLengthStart;
	DWORD dwEnterTick, dwCurrentTick;
	DWORD dwTotalReadTimeout, dwCurrentReadTimeout;

    if (pOpenHead==NULL) {
        DEBUGMSG (ZONE_USR_READ|ZONE_ERROR, (TEXT("COM_READ, Wrong Handle\r\n") ));
        SetLastError (ERROR_INVALID_HANDLE);
        return(ULONG)-1;
    }

	// Get the init structure from the open structure
	pSerialInitStructure = pOpenHead->pSerialInit;

	// Reset receive event (for a correct use of WaitForSingleObject)
	ResetEvent(pSerialInitStructure->hRxEvent);

	// Reset intervall timeout 
	pOpenHead->bTimeoutInterval = FALSE;

    // KW - Special Case for No Wait
    if (pSerialInitStructure->commTimeouts.ReadIntervalTimeout == MAXDWORD &&
        pSerialInitStructure->commTimeouts.ReadTotalTimeoutConstant == 0 &&
        pSerialInitStructure->commTimeouts.ReadTotalTimeoutMultiplier == 0)
    {
	    dwBufferLengthStart = dwBufferLength;
	    dwBufferLength = ReadAvailableData(pSerialInitStructure, dwBufferLength, &pTargetBuffer);
		if (dwBufferLength > 0 && SetThresHold(pSerialInitStructure, dwBufferLength) == TRUE)
		{
			dwBufferLength = ReadAvailableData(pSerialInitStructure, dwBufferLength, &pTargetBuffer);
		}
        // Deactivate interval timeout
        HWClearCommTimeouts(pSerialInitStructure);

        DEBUGMSG (ZONE_USR_READ|ZONE_FUNCTION,
                  (TEXT("-COM_READ: returning %d, Exit Tick:%d")
                  ,0
                  ,GetTickCount()));

        // Return number of chars 
	    return (dwBufferLengthStart - dwBufferLength);
    }

	// Activate interval timeout if necessary
	if (pSerialInitStructure->commTimeouts.ReadIntervalTimeout > 0)
	{
		// Configure and activate interval timeout
		HWSetCommTimeouts(pSerialInitStructure, &pSerialInitStructure->commTimeouts);
		HWEnableTimeOutInterrupt(pSerialInitStructure);
	}

	// Compute total timeout
	dwTotalReadTimeout = pSerialInitStructure->commTimeouts.ReadTotalTimeoutConstant + (pSerialInitStructure->commTimeouts.ReadTotalTimeoutMultiplier * dwBufferLength);
	// If total timeout is 0
	if (dwTotalReadTimeout == 0)
	{	// so timeout is infinite
		dwCurrentReadTimeout = INFINITE;
	}
	else // if a total timeout is wanted
	{   // Get COM_READ function enter tick
		dwEnterTick = GetTickCount();
		dwCurrentReadTimeout = dwTotalReadTimeout;
	}

	// Save buffer length value
	dwBufferLengthStart = dwBufferLength;

	// READ DATA : Read available data
	dwBufferLength = ReadAvailableData(pSerialInitStructure, dwBufferLength, &pTargetBuffer);

	// While resting byte need by user
	while (dwBufferLength > 0)
	{
		// CONFIGURE DMA : Set new DMA register value to read the correct number of char
		if (SetThresHold(pSerialInitStructure, dwBufferLength) == TRUE)
		{
			// If some car arrived since last read, so SetThresHold return TRUE
			dwBufferLength = ReadAvailableData(pSerialInitStructure, dwBufferLength, &pTargetBuffer);
			// There are sufficent char received, so end read
			break;
		}

		// WAIT INTERRUPT : Wait Rx Interrupt or timeout
		if (WaitForSingleObject(pSerialInitStructure->hRxEvent, dwCurrentReadTimeout) == WAIT_TIMEOUT)
		{
			// If an timeout occured, so it's finish. Time out value is already set to the maximum value
			// Try to read available data
			dwBufferLength = ReadAvailableData(pSerialInitStructure, dwBufferLength, &pTargetBuffer);
			// and end received
			break;
		}

		// READ DATA : Read arrived data
		dwBufferLength = ReadAvailableData(pSerialInitStructure, dwBufferLength, &pTargetBuffer);

		// TIMOUT : If total timeout activated
		if (dwTotalReadTimeout != 0)
		{
			// Get current time
			dwCurrentTick = GetTickCount();
			// Test if wasn't timeout
			if (dwTotalReadTimeout > (dwCurrentTick - dwEnterTick))
			{
				dwCurrentReadTimeout = dwTotalReadTimeout - (dwCurrentTick - dwEnterTick);
			}
			else
			{
				// If new value compute is 0, it's out of time.
				break;
			}
		}

		// TIMOUT : If there is a Timeout interval, stop reading
		if (pOpenHead->bTimeoutInterval == TRUE)
		{
			break;
		}
	}

	// Deactivate interval timeout
	HWClearCommTimeouts(pSerialInitStructure);

	DEBUGMSG (ZONE_USR_READ|ZONE_FUNCTION,
			  (TEXT("-COM_READ: returning %d, Exit Tick:%d")
			  ,0
			  ,GetTickCount()));

	// Return number of chars 
	return (dwBufferLengthStart - dwBufferLength);
}

//------------------------------------------------------------------------------
//! \fn					ULONG COM_Write( HANDLE pHead, PUCHAR pSourceBytes, ULONG NumberOfBytes )
//!
//! \brief				Allows application to transmit bytes to the serial port.   
//!
//! \param	pHead			Context pointer returned from COM_Open.
//! \param	pSourceBytes	Pointer to bytes to be written.
//! \param	NumberOfBytes	Number of bytes to be written.
//!
//! \return				number of bytes written if success;
//! \return	\e			-1 if error
//------------------------------------------------------------------------------
ULONG COM_Write(
			HANDLE pHead, 
			PUCHAR pSourceBytes, 
			ULONG  NumberOfBytes 
         )
{
    T_SERIAL_OPEN_STRUCTURE *pOpenHead = (T_SERIAL_OPEN_STRUCTURE*) pHead;
	T_SERIAL_INIT_STRUCTURE *pInitContext;
	DWORD StartNumberOfBytes = NumberOfBytes;
	DWORD enterTickCount = GetTickCount();
	DWORD currentTickCount, writeTotalTimeoutMS, writeTimeoutMS;


	if (pOpenHead==NULL)
	{
        DEBUGMSG (ZONE_WRITE|ZONE_ERROR, (TEXT("COM_WRITE, Wrong Handle\r\n") ));
        SetLastError (ERROR_INVALID_HANDLE);
        return(ULONG)-1;
    }

	// Get init structure from open structure
	pInitContext = pOpenHead->pSerialInit;
	
	// Initialize timeouts
	writeTotalTimeoutMS = pInitContext->commTimeouts.WriteTotalTimeoutConstant + (pInitContext->commTimeouts.WriteTotalTimeoutMultiplier * StartNumberOfBytes);
	writeTimeoutMS = writeTotalTimeoutMS;
	if (writeTimeoutMS == 0)
	{
		writeTimeoutMS = INFINITE;
	}

	// Wait write mutex to write data
	if ( WaitForSingleObject(pInitContext->hWriteMutex, writeTimeoutMS) == WAIT_TIMEOUT)
	{   // If timeout occure during waiting, exit by return 0.
		ReleaseMutex(pInitContext->hWriteMutex);
		return 0;
	}

	// Init abort tx var.
	pInitContext->bTxAbort = FALSE;
	// Set the nb bytes to send value
	pInitContext->dwNbBytesToSend = NumberOfBytes;
	
	// Reset txevent
	ResetEvent(pInitContext->hTxEvent);
	
	DWORD dwCurrentBufferId;
	DWORD dwNbByteToCopy=0;
	DWORD dwNbByteSent=0;

	// Initialize first DMA Transmit buffer
	if (pInitContext->dwNbBytesToSend > pInitContext->dwTxBufferSize)
	{
		dwNbByteToCopy = pInitContext->dwTxBufferSize;
	}
	else if(pInitContext->dwNbBytesToSend != 0)
	{
		dwNbByteToCopy = pInitContext->dwNbBytesToSend;
	}

	// Critical section used to keep the dwNbBytesToSend var in relation with the dma value
	EnterCriticalSection(&pInitContext->csNbBytesToSend);
	pInitContext->dwNbBytesToSend -= dwNbByteToCopy;

	// Copy buffer in data to DMA buffer
	memcpy(pInitContext->pDmaTxBufferVA[0], pSourceBytes, dwNbByteToCopy);
	// Data copied, so byte considered as sent
	pSourceBytes += dwNbByteToCopy;

	// Initialize first time Tx DMA register
	HWSetTxDmaRegisterValue(pInitContext
							,pInitContext->pDmaTxBufferPA[0].LowPart
							,dwNbByteToCopy
							,pInitContext->pDmaTxBufferPA[1].LowPart
							,0
							);
	LeaveCriticalSection(&pInitContext->csNbBytesToSend);

	dwCurrentBufferId = 0;

	// Enable transmit
	HWEnableTransmit(pInitContext);

	// Enable interrupt
	HWEnableTxInterrupt(pInitContext);

	while (WaitForSingleObject(pInitContext->hTxEvent, writeTimeoutMS) == WAIT_OBJECT_0)
	{
		if (pInitContext->dwNbBytesToSend > 0)
		{
			// If event occured
			dwCurrentBufferId = (dwCurrentBufferId+1) % pInitContext->dwTxBufferCount;

			if (pInitContext->dwNbBytesToSend > pInitContext->dwTxBufferSize)
			{
				dwNbByteToCopy = pInitContext->dwTxBufferSize;
			}
			else if(pInitContext->dwNbBytesToSend != 0)
			{
				dwNbByteToCopy = pInitContext->dwNbBytesToSend;
			}
			pInitContext->dwNbBytesToSend -= dwNbByteToCopy;

			// Copy buffer in data to DMA buffer
			memcpy(pInitContext->pDmaTxBufferVA[dwCurrentBufferId], pSourceBytes, dwNbByteToCopy);
			// Data copied, so byte considered as sent
			pSourceBytes += dwNbByteToCopy;

			HWSetTxDmaRegisterNextValue(pInitContext
								 ,(&pInitContext->pDmaTxBufferPA[dwCurrentBufferId])->LowPart
								 ,dwNbByteToCopy);

			// Enable interrupt
			HWEnableTxInterrupt(pInitContext);
		}
		else
		{
			break;
		}

		// If write timeout wanted is not infinite
		if (writeTotalTimeoutMS != 0)
		{
			currentTickCount = GetTickCount();

			if ( (currentTickCount-enterTickCount) > writeTotalTimeoutMS)
			{
				writeTimeoutMS = 0;
			}
			else
			{
				// Calculate new timeout value
				writeTimeoutMS = writeTotalTimeoutMS - (currentTickCount-enterTickCount);
			}

			if (writeTimeoutMS == 0)
			{
				//RETAILMSG(1, (L"WriteExit on writeTimeoutMS == 0"));
				break;
			}
		}

		// If a purge abort arrived
		if (pInitContext->bTxAbort == TRUE)
		{
			break;
		}
	}

	// Disable transmit
	HWDisableTransmit(pInitContext);
	
	// Disable interrupt
	HWDisableTxEMPTYInterrupt(pInitContext);
	HWDisableTxENDInterrupt(pInitContext);
	
	// Search byte resting in PDC. This char are not sent, so decremented it from byte send value
	dwNbByteSent = StartNumberOfBytes - (pInitContext->dwNbBytesToSend + pInitContext->pUSARTReg->US_TCR + pInitContext->pUSARTReg->US_TNCR);
	
	// Purge transmit communication
	HWPurgeTxBuffers(pOpenHead);
	
	
    EvaluateEventFlag(pInitContext, EV_TXEMPTY);

	ReleaseMutex(pInitContext->hWriteMutex);

 //   DEBUGMSG (ZONE_WRITE|ZONE_FUNCTION,(TEXT("-COM_WRITE: returning %d\r\n"),0));

    return  dwNbByteSent;
}



//------------------------------------------------------------------------------
//! \fn					ULONG COM_Seek ( HANDLE pHead, LONG Position, DWORD Type )
//!
//! \brief				Seek for a postition in the flow.   
//!
//! \param	pHead		Context pointer returned from COM_Open.
//! \param	Position	Position to seek.
//! \param	Type		Type required.
//!
//!	\remarks			Not Implemented
//!
//! \return				True if success;
//! \return	\e			False if error
//------------------------------------------------------------------------------
ULONG COM_Seek  (
				HANDLE  pHead,
				LONG    Position,
				DWORD   Type
				)
{
	// Not implemented
    return(ULONG)-1;
}

//------------------------------------------------------------------------------
//! \fn					BOOL COM_PowerUp( HANDLE pHead )
//!
//! \brief				Power-up the device.   
//!
//! \param	pHead		Context pointer returned from COM_Open.
//!
//! \return				True if success;
//! \return	\e			False if error
//------------------------------------------------------------------------------
BOOL COM_PowerUp(
				HANDLE      pHead
				)
{
	// TODO : Power up controller
    return TRUE;
}

//------------------------------------------------------------------------------
//! \fn					BOOL COM_PowerDown( HANDLE pHead )
//!
//! \brief				Power-down the device.   
//!
//! \param	pHead		Context pointer returned from COM_Open.
//!
//! \return				True if success;
//! \return	\e			False if error
//------------------------------------------------------------------------------
BOOL COM_PowerDown	(
					HANDLE      pHead
					)
{   
	// TODO : Power down controller
    return TRUE;
}

//------------------------------------------------------------------------------
//! \fn					BOOL COM_IOControl( T_SERIAL_OPEN_STRUCTURE* pOpenHead, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
//!
//! \brief				Device IO control routine.   
//!
//! \param	pOpenHead		Context pointer returned from COM_Open.
//! \param	dwCode			Io control code to be performed.
//! \param	pBufIn			Input data to the device.
//! \param	dwLenIn			Number of bytes being passed in.
//! \param	pBufOut			Output data from the device.
//! \param	dwLenOut		Maximum number of bytes to receive from device.
//! \param	pdwActualOut	Actual number of bytes received from device.
//!
//!	\remarks			Routine exported by a device driver. 
//!						"COM" is the string passed in as lpszType in RegisterDevice
//!
//! \return				True if success;
//! \return	\e			False if error
//------------------------------------------------------------------------------
BOOL COM_IOControl( T_SERIAL_OPEN_STRUCTURE* pOpenHead,
					DWORD dwCode,
					PBYTE pBufIn, DWORD dwLenIn,
					PBYTE pBufOut, DWORD dwLenOut,
					PDWORD pdwActualOut)
{
    BOOL						RetVal      = TRUE;        // Initialize to success
    T_SERIAL_INIT_STRUCTURE*	pHWIHead;
    PLIST_ENTRY					pEntry;
    DWORD						dwFlags;

    if (pOpenHead==NULL) {
        SetLastError (ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    pHWIHead = pOpenHead->pSerialInit;
	if (pHWIHead == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	
    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION,
              (TEXT("+COM_IOControl(0x%X, %d, 0x%X, %d, 0x%X, %d, 0x%X)\r\n"),
               pOpenHead, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut));

    if ( !pHWIHead->dwOpenCnt ) {
        DEBUGMSG (ZONE_IOCTL|ZONE_ERROR,
                  (TEXT(" COM_IOControl - device was closed\r\n")));
        SetLastError (ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    // Make sure the caller has access permissions
    // NOTE : Pay attention here.  I hate to make this check repeatedly
    // below, so I'll optimize it here.  But as you add new ioctl's be
    // sure to account for them in this if check.
    if ( !( (dwCode == IOCTL_SERIAL_GET_WAIT_MASK) ||
            (dwCode == IOCTL_SERIAL_SET_WAIT_MASK) ||
            (dwCode == IOCTL_SERIAL_WAIT_ON_MASK) ||
            (dwCode == IOCTL_SERIAL_GET_MODEMSTATUS) ||
            (dwCode == IOCTL_SERIAL_GET_PROPERTIES) ||
            (dwCode == IOCTL_SERIAL_GET_TIMEOUTS)   ||
			(dwCode == IOCTL_SERIAL_GET_DCB)
			)) 
	{
        // If not one of the above operations, then read or write
        // access permissions are required.
        if ( !(pOpenHead->AccessCode & (GENERIC_READ | GENERIC_WRITE) ) ) {
            DEBUGMSG(ZONE_IOCTL|ZONE_ERROR,
                     (TEXT("COM_Ioctl: Ioctl %x access permission failure x%X\n\r"),
                      dwCode, pOpenHead->AccessCode));
            SetLastError (ERROR_INVALID_ACCESS);
            return(FALSE);
        }

    }

    switch ( dwCode ) {
    // ****************************************************************
    //	
    //	@func BOOL	| IOCTL_SERIAL_SET_BREAK_ON |
    //				Device IO control routine to set the break state.
    //
    //	@parm DWORD | dwOpenData | value returned from COM_Open call
    //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_BREAK_ON
    //	@parm PBYTE | pBufIn | Ignored
    //	@parm DWORD | dwLenIn | Ignored
    //	@parm PBYTE | pBufOut | Ignored
    //	@parm DWORD | dwLenOut | Ignored
    //	@parm PDWORD | pdwActualOut | Ignored
    //
    //	@rdesc		Returns TRUE for success, FALSE for failure (and
    //				sets thread error code)
    //
    //	@remark Sets the transmission line in a break state until
    //				<f IOCTL_SERIAL_SET_BREAK_OFF> is called.
    //
    case IOCTL_SERIAL_SET_BREAK_ON :
        DEBUGMSG (ZONE_IOCTL,
                  (TEXT(" IOCTL_SERIAL_SET_BREAK_ON\r\n")));
        
		HWSetBreak(pHWIHead);

        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_SET_BREAK_OFF |
        //				Device IO control routine to clear the break state.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_BREAK_OFF
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@remark		Restores character transmission for the communications
        //				device and places the transmission line in a nonbreak state
        //				(called after <f IOCTL_SERIAL_SET_BREAK_ON>).
        //
    case IOCTL_SERIAL_SET_BREAK_OFF :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_SET_BREAK_OFF\r\n")));
        
		HWClearBreak(pHWIHead);

        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_SET_DTR |
        //				Device IO control routine to set DTR high.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_DTR
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_CLR_DTR>
        //
    case IOCTL_SERIAL_SET_DTR :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_SET_DTR\r\n")));
        
		HWSetDTR(pHWIHead);

        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_CLR_DTR |
        //				Device IO control routine to set DTR low.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_CLR_DTR
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_SET_DTR>
        //
    case IOCTL_SERIAL_CLR_DTR :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_CLR_DTR\r\n")));
       
		HWClearDTR(pHWIHead);

        break;

        // ****************************************************************
        //	
        //	@func	BOOL | IOCTL_SERIAL_SET_RTS |
        //				Device IO control routine to set RTS high.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_RTS
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_CLR_RTS>
        //
    case IOCTL_SERIAL_SET_RTS :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_SET_RTS\r\n")));
        
		HWSetRTS(pHWIHead);

        break;

        // ****************************************************************
        //	
        //	@func	BOOL | IOCTL_SERIAL_CLR_RTS |
        //				Device IO control routine to set RTS low.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_CLR_RTS
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_SET_RTS>
        //
    case IOCTL_SERIAL_CLR_RTS :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_CLR_RTS\r\n")));
        
		HWClearRTS(pHWIHead);

        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_SET_XOFF |
        //				Device IO control routine to cause transmission
        //				to act as if an XOFF character has been received.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_XOFF
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_SET_XON>
        //
    case IOCTL_SERIAL_SET_XOFF :
        DEBUGMSG (ZONE_IOCTL|ZONE_FLOW, (TEXT(" IOCTL_SERIAL_SET_XOFF not supported\r\n")));
		
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_SET_XON |
        //				Device IO control routine to cause transmission
        //				to act as if an XON character has been received.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_XON
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_SET_XOFF>
        //
    case IOCTL_SERIAL_SET_XON :
        DEBUGMSG (ZONE_IOCTL|ZONE_FLOW, (TEXT(" IOCTL_SERIAL_SET_XON not supported\r\n")));
        
        break;

        // ****************************************************************
        //	
        //	@func		BOOL | IOCTL_SERIAL_GET_WAIT_MASK |
        //				Device IO control routine to retrieve the value
        //				of the event mask.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_GET_WAIT_MASK
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Points to DWORD to place event mask
        //	@parm DWORD | dwLenOut | should be sizeof(DWORD) or larger
        //	@parm PDWORD | pdwActualOut | Points to DWORD to return length
        //				of returned data (should be set to sizeof(DWORD) if no
        //				error)
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_SET_WAIT_MASK>
        //				<f IOCTL_SERIAL_WAIT_ON_MASK>
        //
    case IOCTL_SERIAL_GET_WAIT_MASK :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_GET_WAIT_MASK\r\n")));

        if ( (dwLenOut < sizeof(DWORD)) || (NULL == pBufOut) ||
             (NULL == pdwActualOut) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }

        // Set The Wait Mask
        *(DWORD *)pBufOut = pOpenHead->CommEvents.fEventMask;

        // Return the size
        *pdwActualOut = sizeof(DWORD);

        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_SET_WAIT_MASK |
        //				Device IO control routine to set the value
        //				of the event mask.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_WAIT_MASK
        //	@parm PBYTE | pBufIn | Pointer to the DWORD mask value
        //	@parm DWORD | dwLenIn | should be sizeof(DWORD)
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_GET_WAIT_MASK>
        //				<f IOCTL_SERIAL_WAIT_ON_MASK>
        //
    case IOCTL_SERIAL_SET_WAIT_MASK :
        if ( (dwLenIn < sizeof(DWORD)) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_SET_WAIT_MASK 0x%X\r\n"),
                               *(DWORD *)pBufIn));
		
        EnterCriticalSection(&(pOpenHead->CommEvents.EventCS));
        // OK, now we can actually set the mask
        pOpenHead->CommEvents.fEventMask = *(DWORD *)pBufIn;

        // NOTE: If there is an outstanding WaitCommEvent, it should
        // return an error when SET_WAIT_MASK is called.  We accomplish
        // this by generating an hCommEvent which will wake the WaitComm
        // and subsequently return error (since no event bits will be set )
        pOpenHead->CommEvents.fAbort = 1;
        SetEvent(pOpenHead->CommEvents.hCommEvent);

        // And calculate the OR of all masks for this port.  Use a temp
        // variable so that the other threads don't see a partial mask
        dwFlags = 0;
        pEntry = pHWIHead->OpenList.Flink;
        while ( pEntry != &pHWIHead->OpenList ) {
            T_SERIAL_OPEN_STRUCTURE   *pTmpOpenHead;

            pTmpOpenHead = CONTAINING_RECORD( pEntry, T_SERIAL_OPEN_STRUCTURE, llist);
            pEntry = pEntry->Flink;    // advance to next 

            DEBUGMSG (ZONE_EVENTS, (TEXT(" SetWaitMask - handle x%X mask x%X\r\n"),
                                    pTmpOpenHead, pTmpOpenHead->CommEvents.fEventMask));
            dwFlags |= pTmpOpenHead->CommEvents.fEventMask;
        }
        pHWIHead->fEventMask = dwFlags;

		// Set one car received, so IT will rise when one char arrived,
		SetThresHold(pHWIHead, 1);

        LeaveCriticalSection(&(pOpenHead->CommEvents.EventCS));
        DEBUGMSG (ZONE_EVENTS, (TEXT(" SetWaitMask - mask x%X\r\n"),pOpenHead->CommEvents.fEventMask));


        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_WAIT_ON_MASK |
        //				Device IO control routine to wait for a communications
        //				event that matches one in the event mask
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_WAIT_ON_MASK
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Points to DWORD to place event mask.
        //				The returned mask will show the event that terminated
        //				the wait.  If a process attempts to change the device
        //				handle's event mask by using the IOCTL_SERIAL_SET_WAIT_MASK
        //				call the driver should return immediately with (DWORD)0 as
        //				the returned event mask.
        //	@parm DWORD | dwLenOut | should be sizeof(DWORD) or larger
        //	@parm PDWORD | pdwActualOut | Points to DWORD to return length
        //				of returned data (should be set to sizeof(DWORD) if no
        //				error)
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_GET_WAIT_MASK>
        //				<f IOCTL_SERIAL_SET_WAIT_MASK>
        //
    case IOCTL_SERIAL_WAIT_ON_MASK :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_WAIT_ON_MASK\r\n")));
        if ( (dwLenOut < sizeof(DWORD)) || (NULL == pBufOut) ||
             (NULL == pdwActualOut) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }

        if (WaitCommEvent(pOpenHead, (DWORD *)pBufOut, NULL) == FALSE ) {
            // Device may have been closed or removed while we were waiting 
			DEBUGMSG (ZONE_IOCTL|ZONE_ERROR,(TEXT(" COM_IOControl - Error in WaitCommEvent\r\n")));
            RetVal = FALSE;
        }
        // Return the size
        *pdwActualOut = sizeof(DWORD);
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_GET_COMMSTATUS |
        //				Device IO control routine to clear any pending
        //				communications errors and return the current communication
        //				status.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_GET_COMMSTATUS
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Points to a <f SERIAL_DEV_STATUS>
        //				structure for the returned status information
        //	@parm DWORD | dwLenOut | should be sizeof(SERIAL_DEV_STATUS)
        //				or larger
        //	@parm PDWORD | pdwActualOut | Points to DWORD to return length
        //				of returned data (should be set to
        //				sizeof(SERIAL_DEV_STATUS) if no error)
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_GET_COMMSTATUS :
        {
            PSERIAL_DEV_STATUS pSerialDevStat;

            DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_GET_COMMSTATUS\r\n")));
            if ( (dwLenOut < sizeof(SERIAL_DEV_STATUS)) || (NULL == pBufOut) ||
                 (NULL == pdwActualOut) ) {
                SetLastError (ERROR_INVALID_PARAMETER);
                RetVal = FALSE;
                DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
                break;
            }

            pSerialDevStat = (PSERIAL_DEV_STATUS)pBufOut;

            // Set The Error Mask
			EnterCriticalSection(&pHWIHead->csErrorFlags);

            pSerialDevStat->Errors = pHWIHead->dwErrorFlags;
			pHWIHead->dwErrorFlags = 0;
			
			LeaveCriticalSection(&pHWIHead->csErrorFlags);
            
			// Clear the ComStat structure & get PDD related status
            memset ((char *) &(pSerialDevStat->ComStat), 0, sizeof(COMSTAT));
            
            // PDD set fCtsHold, fDsrHold, fRLSDHold, and fTXim.  The MDD then
            // needs to set fXoffHold, fXoffSent, cbInQue, and cbOutQue.
			// Update controller pointer data to RPR register value.
			pHWIHead->dwPAControllerRxData = pHWIHead->pPDCReg->PDC_RPR;
			// Compute available char
			pSerialDevStat->ComStat.cbInQue  = ComputeAvailableChar(pHWIHead);
			EnterCriticalSection(&pHWIHead->csNbBytesToSend);
			pSerialDevStat->ComStat.cbOutQue = pHWIHead->dwNbBytesToSend;
			LeaveCriticalSection(&pHWIHead->csNbBytesToSend);

			// Software Flow Control not supported
            pSerialDevStat->ComStat.fXoffHold = 0;
            pSerialDevStat->ComStat.fXoffSent = 0;

			// Read the value of the CTS pin
			if((pHWIHead->pUSARTReg->US_CSR) & AT91C_US_CTS)
			{
				// If CTS is set, the communication is not hold
				pSerialDevStat->ComStat.fCtsHold = 0;
			}
			else
			{
				pSerialDevStat->ComStat.fCtsHold = 1;
			}

			// Read the value of the DSR pin
			if((pHWIHead->pUSARTReg->US_CSR) & AT91C_US_DSR)
			{
				// If DSR is set, the communication is not hold
				pSerialDevStat->ComStat.fDsrHold = 0;
			}
			else
			{
				pSerialDevStat->ComStat.fDsrHold = 1;
			}

			// Read the value of the DCD pin
			if((pHWIHead->pUSARTReg->US_CSR) & AT91C_US_DCD)
			{
				// If DCD is set, the communication is not hold
				pSerialDevStat->ComStat.fRlsdHold = 0;
			}
			else
			{
				pSerialDevStat->ComStat.fRlsdHold = 1;
			}

			pSerialDevStat->ComStat.fEof = 0;
			pSerialDevStat->ComStat.fTxim = 0;

            // Return the size
            *pdwActualOut = sizeof(SERIAL_DEV_STATUS);

        }

        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_GET_MODEMSTATUS |
        //				Device IO control routine to retrieve current
        //				modem control-register values
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_GET_MODEMSTATUS
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Points to a DWORD for the returned
        //				modem status information
        //	@parm DWORD | dwLenOut | should be sizeof(DWORD)
        //				or larger
        //	@parm PDWORD | pdwActualOut | Points to DWORD to return length
        //				of returned data (should be set to sizeof(DWORD)
        //				if no error)
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_GET_MODEMSTATUS :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_GET_MODEMSTATUS\r\n")));
        if ( (dwLenOut < sizeof(DWORD)) || (NULL == pBufOut) ||
             (NULL == pdwActualOut) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }

        // Set the Modem Status dword
        *(DWORD *)pBufOut = 0;

		/*todo if required*/
		RETAILMSG (1, (TEXT(" IOCTL_SERIAL_GET_MODEMSTATUS not supported\r\n")));

        // Return the size
        *pdwActualOut = sizeof(DWORD);
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_GET_PROPERTIES |
        //				Device IO control routine to retrieve information
        //				about the communications properties for the device.
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_GET_PROPERTIES
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Points to a <f COMMPROP> structure
        //				for the returned information.
        //	@parm DWORD | dwLenOut | should be sizeof(COMMPROP)
        //				or larger
        //	@parm PDWORD | pdwActualOut | Points to DWORD to return length
        //				of returned data (should be set to sizeof(COMMPROP)
        //				if no error)
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_GET_PROPERTIES :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_GET_PROPERTIES\r\n")));
        if ( (dwLenOut < sizeof(COMMPROP)) || (NULL == pBufOut) || (NULL == pdwActualOut) ) 
		{
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }

		COMMPROP * pCommProp;
		pCommProp = (COMMPROP *)pBufOut;

        // Clear the ComMProp structure
        memset ((char *) pCommProp, 0,sizeof(COMMPROP));
		pCommProp->dwMaxTxQueue			= pHWIHead->dwTxBufferSize * pHWIHead->dwTxBufferCount;
		pCommProp->dwMaxRxQueue			= pHWIHead->dwRxBufferSize;
		pCommProp->dwMaxBaud			= BAUD_115200;
		pCommProp->dwProvSubType		= PST_RS232;
		pCommProp->dwProvCapabilities	= PCF_TOTALTIMEOUTS | PCF_RTSCTS | PCF_INTTIMEOUTS;
		pCommProp->dwSettableParams		= SP_BAUD | SP_DATABITS | SP_HANDSHAKING | SP_PARITY | SP_STOPBITS;
		pCommProp->dwSettableBaud		= BAUD_075 | BAUD_110 | BAUD_300 | BAUD_600 | BAUD_1200 | BAUD_2400 | BAUD_4800 | BAUD_7200 | BAUD_9600 | BAUD_14400 | BAUD_19200 | BAUD_38400 | BAUD_57600 | BAUD_115200; 
		pCommProp->dwProvSubType		= PST_RS232;
		pCommProp->wSettableData		= DATABITS_5 | DATABITS_6 | DATABITS_7 | DATABITS_8;
		pCommProp->wSettableStopParity	= PARITY_EVEN | PARITY_NONE | PARITY_ODD | STOPBITS_10 | STOPBITS_15 | STOPBITS_20;
		pCommProp->dwCurrentTxQueue		= pHWIHead->dwTxBufferSize * pHWIHead->dwTxBufferCount;
		pCommProp->dwCurrentRxQueue		= pHWIHead->dwRxBufferSize;				
		pCommProp->dwServiceMask		= SP_SERIALCOMM;

        // Return the size
        *pdwActualOut = sizeof(COMMPROP);
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_SET_TIMEOUTS |
        //				Device IO control routine to set the time-out parameters
        //				for all read and write operations on a specified
        //				communications device
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_TIMEOUTS
        //	@parm PBYTE | pBufIn | Pointer to the <f COMMTIMEOUTS> structure
        //	@parm DWORD | dwLenIn | should be sizeof(COMMTIMEOUTS)
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_GET_TIMEOUTS>
        //
    case IOCTL_SERIAL_SET_TIMEOUTS :
        if ( (dwLenIn < sizeof(COMMTIMEOUTS)) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        DEBUGMSG (ZONE_IOCTL,
                  (TEXT(" IOCTL_SERIAL_SET_COMMTIMEOUTS (%d,%d,%d,%d,%d)\r\n"),
                   ((COMMTIMEOUTS *)pBufIn)->ReadIntervalTimeout,
                   ((COMMTIMEOUTS *)pBufIn)->ReadTotalTimeoutMultiplier,
                   ((COMMTIMEOUTS *)pBufIn)->ReadTotalTimeoutConstant,
                   ((COMMTIMEOUTS *)pBufIn)->WriteTotalTimeoutMultiplier,
                   ((COMMTIMEOUTS *)pBufIn)->WriteTotalTimeoutConstant));

		HWSetCommTimeouts(pHWIHead, (LPCOMMTIMEOUTS)pBufIn);

        pHWIHead->commTimeouts.ReadIntervalTimeout =
        ((COMMTIMEOUTS *)pBufIn)->ReadIntervalTimeout;
        pHWIHead->commTimeouts.ReadTotalTimeoutMultiplier =
        ((COMMTIMEOUTS *)pBufIn)->ReadTotalTimeoutMultiplier;
        pHWIHead->commTimeouts.ReadTotalTimeoutConstant =
        ((COMMTIMEOUTS *)pBufIn)->ReadTotalTimeoutConstant;
        pHWIHead->commTimeouts.WriteTotalTimeoutMultiplier =
        ((COMMTIMEOUTS *)pBufIn)->WriteTotalTimeoutMultiplier;
        pHWIHead->commTimeouts.WriteTotalTimeoutConstant =
        ((COMMTIMEOUTS *)pBufIn)->WriteTotalTimeoutConstant;

        
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_GET_TIMEOUTS |
        //				Device IO control routine to get the time-out parameters
        //				for all read and write operations on a specified
        //				communications device
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_GET_TIMEOUTS
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Pointer to a <f COMMTIMEOUTS> structure
        //				for the returned data
        //	@parm DWORD | dwLenOut | should be sizeof(COMMTIMEOUTS)
        //	@parm PDWORD | pdwActualOut | Points to DWORD to return length
        //				of returned data (should be set to sizeof(COMMTIMEOUTS)
        //				if no error)
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //	@xref		<f IOCTL_SERIAL_GET_TIMEOUTS>
        //
    case IOCTL_SERIAL_GET_TIMEOUTS :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_GET_TIMEOUTS\r\n")));
        if ( (dwLenOut < sizeof(COMMTIMEOUTS)) || (NULL == pBufOut) || (NULL == pdwActualOut) ) 
		{
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }

         memcpy((LPCOMMTIMEOUTS)pBufOut, &(pHWIHead->commTimeouts),sizeof(COMMTIMEOUTS));

        // Return the size
        *pdwActualOut = sizeof(COMMTIMEOUTS);
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_PURGE |
        //				Device IO control routine to discard characters from the
        //				output or input buffer of a specified communications
        //				resource.  It can also terminate pending read or write
        //				operations on the resource
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_PURGE
        //	@parm PBYTE | pBufIn | Pointer to a DWORD containing the action
        //	@parm DWORD | dwLenIn | Should be sizeof(DWORD)
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_PURGE :
        if ( (dwLenIn < sizeof(DWORD)) || (NULL == pBufIn) ) 
		{
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        dwFlags = *((PDWORD) pBufIn);

        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_PURGE 0x%X\r\n"),dwFlags));

		// Received purge, just clear data in buffer, and continue reading new chars
		if ( dwFlags & (PURGE_RXCLEAR) )
		{
			// Reset hardware pdc register and controller and user pointer value
			// WARNING this function DISABLE and ENABLE all INTERRUPT (using DisableInt function)
			HWPurgeRxBuffers(pOpenHead);

			// Signal that a Rx event occurerd, to unlock the com_read function
			SetEvent(pHWIHead->hRxEvent);
		}
		else if ( dwFlags & (PURGE_RXABORT) ) // Clear buffer and stop reading
		{
			// Reset hardware pdc register and controller and user pointer value
			// WARNING this function DISABLE and ENABLE all INTERRUPT (using DisableInt function)
			HWPurgeRxBuffers(pOpenHead);

			// Use the timeout interval boolean to stop reading function
			// Set interval timeout boolean to true, so com_read function will end imediately after the hRxEvent flags will be set
			pOpenHead->bTimeoutInterval = TRUE;

			// Signal that a Rx event occurerd, to unlock the com_read function
			SetEvent(pHWIHead->hRxEvent);
		}

		
        if ( dwFlags & (PURGE_TXABORT | PURGE_TXCLEAR) ) 
		{
			HWPurgeTxBuffers(pOpenHead);
		}

        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_SET_QUEUE_SIZE |
        //				Device IO control routine to set the queue sizes of of a
        //				communications device
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_QUEUE_SIZE
        //	@parm PBYTE | pBufIn | Pointer to a <f SERIAL_QUEUE_SIZES>
        //				structure
        //	@parm DWORD | dwLenIn | should be sizeof(<f SERIAL_QUEUE_SIZES>)
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_SET_QUEUE_SIZE :
        if ( (dwLenIn < sizeof(SERIAL_QUEUE_SIZES)) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        DEBUGMSG (ZONE_IOCTL,(TEXT(" IOCTL_SERIAL_SET_QUEUE_SIZE (%d,%d,%d,%d,%d)\r\n"),((SERIAL_QUEUE_SIZES *)pBufIn)->cbInQueue,((SERIAL_QUEUE_SIZES *)pBufIn)->cbOutQueue));
		RETAILMSG (1, (TEXT(" IOCTL_SERIAL_SET_QUEUE_SIZE not supported\r\n")));

        
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_IMMEDIATE_CHAR |
        //				Device IO control routine to transmit a specified character
        //				ahead of any pending data in the output buffer of the
        //				communications device
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_IMMEDIATE_CHAR
        //	@parm PBYTE | pBufIn | Pointer to a UCHAR to send
        //	@parm DWORD | dwLenIn | should be sizeof(UCHAR)
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_IMMEDIATE_CHAR :
        if ( (dwLenIn < sizeof(UCHAR)) || (NULL == pBufIn) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        DEBUGMSG (ZONE_IOCTL, (TEXT("IOCTL_SERIAL_IMMEDIATE_CHAR\r\n")));
		RETAILMSG (1, (TEXT(" IOCTL_SERIAL_IMMEDIATE_CHAR not supported\r\n")));

        // Possible update
		//pTargetSerialInit->pRxBuffer->InsertHeadByte(*((UCHAR *)pBufIn));

        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_GET_DCB |
        //				Device IO control routine to get the device-control
        //				block from a specified communications device
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_GET_DCB
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Pointer to a <f DCB> structure
        //	@parm DWORD | dwLenOut | Should be sizeof(<f DCB>)
        //	@parm PDWORD | pdwActualOut | Pointer to DWORD to return length
        //				of returned data (should be set to sizeof(<f DCB>) if
        //				no error)
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_GET_DCB :
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_GET_DCB\r\n")));
        if ( (dwLenOut < sizeof(DCB)) || (NULL == pBufOut) ||
             (NULL == pdwActualOut) ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }

        memcpy((char *)pBufOut, (char *)&(pHWIHead->dcb), sizeof(DCB));

        // Return the size
        *pdwActualOut = sizeof(DCB);
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_SET_DCB |
        //				Device IO control routine to set the device-control
        //				block on a specified communications device
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_SET_DCB
        //	@parm PBYTE | pBufIn | Pointer to a <f DCB> structure
        //	@parm DWORD | dwLenIn | should be sizeof(<f DCB>)
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_SET_DCB :
        if ( (dwLenIn < sizeof(DCB)) || (NULL == pBufIn) ) 
		{
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
            break;
        }
        DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_SET_DCB\r\n")));
		
        if (ApplyDCB(pHWIHead, (DCB *)pBufIn) == FALSE) 
		{
            //
            // Most likely an unsupported baud rate was specified
            //
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT(" ApplyDCB failed\r\n")));
            break;
        }

        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_ENABLE_IR |
        //				Device IO control routine to set the device-control
        //				block on a specified communications device
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_ENABLE_IR
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_ENABLE_IR :
        DEBUGMSG (ZONE_IR,(TEXT("IOCTL Enable IR not supported\r\n")));
        
        break;

        // ****************************************************************
        //	
        //	@func BOOL	| IOCTL_SERIAL_DISABLE_IR |
        //				Device IO control routine to set the device-control
        //				block on a specified communications device
        //
        //	@parm DWORD | dwOpenData | value returned from COM_Open call
        //	@parm DWORD | dwCode | IOCTL_SERIAL_DISABLE_IR
        //	@parm PBYTE | pBufIn | Ignored
        //	@parm DWORD | dwLenIn | Ignored
        //	@parm PBYTE | pBufOut | Ignored
        //	@parm DWORD | dwLenOut | Ignored
        //	@parm PDWORD | pdwActualOut | Ignored
        //
        //	@rdesc		Returns TRUE for success, FALSE for failure (and
        //				sets thread error code)
        //
        //
    case IOCTL_SERIAL_DISABLE_IR :
        DEBUGMSG (ZONE_IR,(TEXT("IOCTL Disable IR not supported\r\n")));
        break;

        // KW - 6/30/2008
        // 
        // If the Process is exiting, shut down any possible WaitCommEvent
        // so the serial port can close.
    case IOCTL_PSL_NOTIFY:
        if (dwLenIn == sizeof(DEVICE_PSL_NOTIFY) &&
            ((DEVICE_PSL_NOTIFY*)pBufIn)->dwFlags == DLL_PROCESS_EXITING) {
            pOpenHead->CommEvents.fAbort = 1;
            SetEvent(pOpenHead->CommEvents.hCommEvent);
        }
        break;

    default :
        break;
    }

    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION|(RetVal == FALSE?ZONE_ERROR:0),
              (TEXT("-COM_IOControl %s Ecode=%d (len=%d)\r\n"),
               (RetVal == TRUE) ? TEXT("Success") : TEXT("Error"),
               GetLastError(), (NULL == pdwActualOut) ? 0 : *pdwActualOut));

    return(RetVal);
}


//------------------------------------------------------------------------------
//                                                            Internal functions
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//!	\fn					static BOOL ApplyDCB (T_SERIAL_INIT_STRUCTURE* pSerialInitStructure, DCB *pDCB)
//!
//! \brief				Apply the specified dcb to the serial device.    
//! 
//! \param	pSerialInitStructure	Context pointer returned from COM_Init.
//! \param	pDCB					Pointer to the DCB structure to apply.
//!
//!
//! \return				True if success;
//! \return	\e			False if error
//------------------------------------------------------------------------------
static BOOL ApplyDCB (T_SERIAL_INIT_STRUCTURE* pSerialInitStructure, DCB *pDCB)
{
	EnterCriticalSection(&pSerialInitStructure->csSerialInitStructure);

    if (HWSetDCB(pSerialInitStructure,pDCB) == FALSE)
	{
		LeaveCriticalSection(&pSerialInitStructure->csSerialInitStructure);
        return FALSE;
    }

	// If DBC succeffull hardware applied, copy it to init structure
    memcpy(&pSerialInitStructure->dcb, pDCB, sizeof(DCB));

	LeaveCriticalSection(&pSerialInitStructure->csSerialInitStructure);

	return TRUE;
}

//------------------------------------------------------------------------------
//!	\fn					static BOOL WaitCommEvent( T_SERIAL_OPEN_STRUCTURE* pOpenHead, PULONG pfdwEventMask, LPOVERLAPPED Unused )
//!
//! \brief				Waits for an event to occur for a specified communications device.    
//! 
//! \param	pOpenHead		Context pointer returned from COM_Open.
//! \param	pfdwEventMask	Pointer to ULONG to receive CommEvents.fEventMask.
//! \param	Unused			Pointer to OVERLAPPED not used.
//!
//! \return				True if success;
//! \return	\e			False if error
//------------------------------------------------------------------------------
static BOOL WaitCommEvent(
             T_SERIAL_OPEN_STRUCTURE*   pOpenHead, 
             PULONG						pfdwEventMask, 
             LPOVERLAPPED				Unused 
             )
{
    T_SERIAL_INIT_STRUCTURE*  pHWIHead = pOpenHead->pSerialInit;
    DWORD           dwEventData;

    DEBUGMSG(ZONE_FUNCTION|ZONE_EVENTS,(TEXT("+WaitCommEvent x%X x%X, pMask x%X\n\r"),
                                        pOpenHead, pHWIHead , pfdwEventMask));

    if ( !pHWIHead || !pHWIHead->dwOpenCnt ) {
        DEBUGMSG (ZONE_ERROR|ZONE_EVENTS, (TEXT("-WaitCommEvent - device not open (x%X, %d) \r\n"),
                                           pHWIHead, (pHWIHead == NULL) ? 0 : pHWIHead->dwOpenCnt));
        *pfdwEventMask = 0;
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    // We should return immediately if mask is 0
    if ( !pOpenHead->CommEvents.fEventMask ) {
        DEBUGMSG (ZONE_ERROR|ZONE_EVENTS, (TEXT("-WaitCommEvent - Mask already clear\r\n")));
        *pfdwEventMask = 0;
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

	// Show warning for the non-supported WaitCommEvents
	if((pOpenHead->CommEvents.fEventMask) & EV_RING)
	{
        RETAILMSG (1, (TEXT(" Warning!!! : WaitCommEvents EV_RING not suppported!!!\r\n")));
	}
	if((pOpenHead->CommEvents.fEventMask) & EV_DSR)
	{
        RETAILMSG (1, (TEXT(" Warning!!! : WaitCommEvents EV_DSR not suppported!!!\r\n")));
	}
	if((pOpenHead->CommEvents.fEventMask) & EV_CTS)
	{
        RETAILMSG (1, (TEXT(" Warning!!! : WaitCommEvents EV_CTS not suppported!!!\r\n")));
	}
	if((pOpenHead->CommEvents.fEventMask) & EV_POWER)
	{
        RETAILMSG (1, (TEXT(" Warning!!! : WaitCommEvents EV_POWER not suppported!!!\r\n")));
	}
	if((pOpenHead->CommEvents.fEventMask) & EV_RXFLAG)
	{
        RETAILMSG (1, (TEXT(" Warning!!! : WaitCommEvents EV_RXFLAG not suppported!!!\r\n")));
	}

    // Abort should only affect us once we start waiting.  Ignore any old aborts
    pOpenHead->CommEvents.fAbort = 0;

    while ( pHWIHead->dwOpenCnt ) {
        // Read and clear any event bits
        EnterCriticalSection(&(pOpenHead->CommEvents.EventCS));
        ResetEvent(pOpenHead->CommEvents.hCommEvent);

		dwEventData = InterlockedExchange( &(pOpenHead->CommEvents.fEventData), 0 );
        DEBUGMSG (ZONE_EVENTS, (TEXT(" WaitCommEvent - Events 0x%X, Mask 0x%X, Abort %X\r\n"),
                                dwEventData,
                                pOpenHead->CommEvents.fEventMask,
                                pOpenHead->CommEvents.fAbort ));

        // See if we got any events of interest or mask is reset to zero
        if ( (dwEventData & pOpenHead->CommEvents.fEventMask) != 0 ||
                pOpenHead->CommEvents.fEventMask == 0 ) {
            *pfdwEventMask = dwEventData & pOpenHead->CommEvents.fEventMask;
            LeaveCriticalSection(&(pOpenHead->CommEvents.EventCS));
            break;
        }
        else
            LeaveCriticalSection(&(pOpenHead->CommEvents.EventCS));

        // Wait for an event from PDD, or from SetCommMask
        WaitForSingleObject(pOpenHead->CommEvents.hCommEvent,
                            (ULONG)-1);    

        // We should return immediately if mask was set via SetCommMask.  
        if ( pOpenHead->CommEvents.fAbort ) {
            // We must have been terminated by SetCommMask()
            // Return TRUE with a mask of 0.
            DEBUGMSG (ZONE_ERROR|ZONE_EVENTS, (TEXT(" WaitCommEvent - Mask was cleared\r\n")));
            *pfdwEventMask = 0;
            break;
        }
    }

	// Erase mask set in fEventMaskSet structure init var
	pHWIHead->fEventMaskSet &= ~(*pfdwEventMask);

    // Check and see if device was closed while we were waiting
    if ( !pHWIHead->dwOpenCnt ) {
        // Device was closed.  Get out of here.
        DEBUGMSG (ZONE_EVENTS|ZONE_ERROR,
                  (TEXT("-WaitCommEvent - device was closed\r\n")));
        *pfdwEventMask = 0;
        SetLastError (ERROR_INVALID_HANDLE);
        return(FALSE);
    } else {
        // We either got an event or a SetCommMask 0.
        DEBUGMSG (ZONE_EVENTS,
                  (TEXT("-WaitCommEvent - *pfdwEventMask 0x%X\r\n"),
                   *pfdwEventMask));
        return(TRUE);
    }
}

//------------------------------------------------------------------------------
//!	\fn					VOID EvaluateEventFlag(PVOID pHead, ULONG fdwEventMask)
//!
//! \brief				Evaluate an event mask for all the users.    
//! 
//! \param	pHead			Context pointer returned from COM_Init.
//! \param	fdwEventMask	Bitmask of events
//!
//!	\remarks			This function is called by the PDD (and internally in the MDD
//!						to evaluate a COMM event.  If the user is waiting for a
//!	  					COMM event (see WaitCommEvent()) then it will signal the
//!						users thread.
//!
//! \return				No return
//------------------------------------------------------------------------------
VOID EvaluateEventFlag(PVOID pHead, ULONG fdwEventMask)
{
    T_SERIAL_INIT_STRUCTURE		*pHWIHead = (T_SERIAL_INIT_STRUCTURE *)pHead;
    PLIST_ENTRY					pEntry;
    T_SERIAL_OPEN_STRUCTURE		*pOpenHead;
    DWORD						dwTmpEvent, dwOrigEvent;
    BOOL						fRetCode;

    if ( !pHWIHead->dwOpenCnt ) {
        DEBUGMSG (ZONE_EVENTS|ZONE_ERROR,
                  (TEXT(" EvaluateEventFlag (eventMask = 0x%x) - device was closed\r\n"),fdwEventMask));
        SetLastError (ERROR_INVALID_HANDLE);
        return;
    }

 /*   DEBUGMSG (ZONE_EVENTS, (TEXT(" CommEvent - Event 0x%X, Global Mask 0x%X\r\n"),
                            fdwEventMask,
                            pHWIHead->fEventMask));
*/
    // Now that we support multiple opens, we must check mask for each open handle
    // To keep this relatively painless, we keep a per-device mask which is the
    // bitwise or of each current open mask.  We can check this first before doing
    // all the linked list work to figure out who to notify
    if ( pHWIHead->fEventMask & fdwEventMask ) {
		// Set the sum of all event mask signaled
		pHWIHead->fEventMaskSet |= fdwEventMask;
        pEntry = pHWIHead->OpenList.Flink;
        while ( pEntry != &(pHWIHead->OpenList) ) 
		{
            pOpenHead = CONTAINING_RECORD( pEntry, T_SERIAL_OPEN_STRUCTURE, llist);
            pEntry = pEntry->Flink;  // advance to next 

            EnterCriticalSection(&(pOpenHead->CommEvents.EventCS));
            // Don't do anything unless this event is of interest to the MDD.
            if ( pOpenHead->CommEvents.fEventMask & fdwEventMask ) {
                // Store the event data
                dwOrigEvent = pOpenHead->CommEvents.fEventData;                    
                do {
                    dwTmpEvent = dwOrigEvent;
                    dwOrigEvent = InterlockedExchange(&(pOpenHead->CommEvents.fEventData), 
                                                      dwTmpEvent | fdwEventMask) ;

                } while ( dwTmpEvent != dwOrigEvent );

                // Signal the MDD that new event data is available.
                fRetCode = SetEvent(pOpenHead->CommEvents.hCommEvent);
 /*               DEBUGMSG (ZONE_EVENTS, (TEXT(" CommEvent - Event 0x%X, Handle 0x%X Mask 0x%X (%X)\r\n"),
                                        dwTmpEvent | fdwEventMask,
                                        pOpenHead,
                                        pOpenHead->CommEvents.fEventMask,
                                        fRetCode));
*/
            }
            LeaveCriticalSection(&(pOpenHead->CommEvents.EventCS));
        }
    }

    return;
}

//------------------------------------------------------------------------------
//!	\fn					static BOOL GetRegistryData ( LPCTSTR Identifier, HANDLE pInitStructure )
//!
//! \brief				Get all the data from the registry base.    
//! 
//! \param	Identifier		Port identifier.
//! \param	pInitStructure	Context pointer returned from COM_Init.
//!
//! \return				True if success;
//! \return	\e			False if error
//------------------------------------------------------------------------------
static BOOL GetRegistryData ( LPCTSTR Identifier, HANDLE pInitStructure )
{
	T_SERIAL_INIT_STRUCTURE *pSerialInitStructure = (T_SERIAL_INIT_STRUCTURE *)pInitStructure;
	HKEY	hKey = NULL;
	DWORD	dwDataSize;
	BOOL	RetValue = TRUE;

    // Get settings from registry
    DEBUGMSG (ZONE_INIT,(TEXT("Try to open %s\r\n"), (LPCTSTR) Identifier));
    hKey = OpenDeviceKey((LPCTSTR)Identifier);
    if ( !hKey ) {
        DEBUGMSG (ZONE_INIT | ZONE_ERROR,(TEXT("Failed to open devkeypath, GetRegistryData failed\r\n")));
        RetValue = FALSE;
    }
    	
    // Get the UART index associated to this driver
	dwDataSize = sizeof(pSerialInitStructure->dwDeviceIndex);
	if( RegQueryValueEx(hKey, REG_SERIALPORTINDEX_VAL_NAME, NULL, NULL, (LPBYTE) &(pSerialInitStructure->dwDeviceIndex), &dwDataSize) != ERROR_SUCCESS)
	{
        DEBUGMSG (ZONE_INIT | ZONE_ERROR,(TEXT("Failed to get registry data, GetRegistryData failed\r\n")));
		RetValue = FALSE;
	}

	dwDataSize = sizeof (pSerialInitStructure->dwTxBufferSize);
	if (RegQueryValueEx(hKey, REG_SERIALPORTTXBUFFER_SIZE_NAME, NULL, NULL, (LPBYTE) &(pSerialInitStructure->dwTxBufferSize), &dwDataSize) != ERROR_SUCCESS)
	{
		pSerialInitStructure->dwTxBufferSize = DEFAULT_TX_DMA_BUFFER_BYTE_SIZE;
		DEBUGMSG(ZONE_INIT | DBG_WARNING, (TEXT("No registry value found for TX buffer size, using default value.\r\n")));
	}

	DEBUGMSG(ZONE_INIT, (TEXT("TX buffer size set to %u.\r\n"), pSerialInitStructure->dwTxBufferSize));

	// This is commented for now because currently the driver supports only 2 buffers on emission.
	// This driver is expected to evoluate to a variable count of buffers support.
	// For now the value is forced to 2.
/*	dwDataSize = sizeof (pSerialInitStructure->dwTxBufferCount);
	if (RegQueryValueEx(hKey, REG_SERIALPORTTXBUFFER_COUNT_NAME, NULL, NULL, (LPBYTE) &(pSerialInitStructure->dwTxBufferCount), &dwDataSize) != ERROR_SUCCESS)
	{
		pSerialInitStructure->dwTxBufferCount = DEFAULT_TX_DMA_BUFFER_NUMBER;
		DEBUGMSG(ZONE_INIT | DBG_WARNING, (TEXT("No registry value found for TX buffer count, using default value.\r\n")));
	}

	if (pSerialInitStructure->dwTxBufferCount < TX_DMA_BUFFER_MIN_NUMBER)
	{
		DEBUGMSG(ZONE_INIT | DBG_WARNING, (TEXT("Bad value for TX buffer count, must be >= %u, using default value.\r\n"), TX_DMA_BUFFER_MIN_NUMBER));
		pSerialInitStructure->dwTxBufferCount = DEFAULT_TX_DMA_BUFFER_NUMBER;
	}
*/
	pSerialInitStructure->dwTxBufferCount = 2;
	DEBUGMSG(ZONE_INIT, (TEXT("TX buffer count set to %u.\r\n"), pSerialInitStructure->dwTxBufferCount));


	dwDataSize = sizeof (pSerialInitStructure->dwRxBufferSize);
	if (RegQueryValueEx(hKey, REG_SERIALPORTRXBUFFER_SIZE_NAME, NULL, NULL, (LPBYTE) &(pSerialInitStructure->dwRxBufferSize), &dwDataSize) != ERROR_SUCCESS)
	{
		pSerialInitStructure->dwRxBufferSize = DEFAULT_RX_DMA_BUFFER_BYTE_SIZE;
		DEBUGMSG(ZONE_INIT | DBG_WARNING, (TEXT("No registry value found for RX buffer size, using default value.\r\n")));
	}

	if (pSerialInitStructure->dwRxBufferSize < RX_DMA_BUFFER_MIN_BYTE_SIZE)
	{
		DEBUGMSG(ZONE_INIT | DBG_WARNING, (TEXT("Bad value for RX buffer size, must be >= %u, using default value.\r\n"), RX_DMA_BUFFER_MIN_BYTE_SIZE));
		pSerialInitStructure->dwRxBufferSize = RX_DMA_BUFFER_MIN_BYTE_SIZE;
	}

	DEBUGMSG(ZONE_INIT, (TEXT("RX buffer size set to %u.\r\n"), pSerialInitStructure->dwRxBufferSize));

	if (hKey)
	{
		RegCloseKey (hKey);
	}

	return RetValue;
}


//------------------------------------------------------------------------------
//!	\fn					inline DWORD ComputeFreeSpace(T_SERIAL_INIT_STRUCTURE * pSerialInitStructure)
//!
//! \brief				Compute the free space in RX buffer.
//! 
//! \param	pSerialInitStructure	Context pointer returned from COM_Init.
//!
//! \return				The free space (in number of char)
//------------------------------------------------------------------------------
inline DWORD ComputeFreeSpace(T_SERIAL_INIT_STRUCTURE * pSerialInitStructure)
{
	// Compute free buffer space to flow control system
	if (pSerialInitStructure->dwPAControllerRxData < pSerialInitStructure->dwPAUserRxData)
	{
		return (pSerialInitStructure->dwPAUserRxData - pSerialInitStructure->dwPAControllerRxData);
	}
	else
	{
		return (pSerialInitStructure->dwRxBufferSize - (pSerialInitStructure->dwPAControllerRxData - pSerialInitStructure->dwPAUserRxData));
	}
}

//------------------------------------------------------------------------------
//!	\fn					inline DWORD ComputeAvailableChar(T_SERIAL_INIT_STRUCTURE * pSerialInitStructure)
//!
//! \brief				Compute the number chars available in the RX buffer.
//! 
//! \param	pSerialInitStructure	Context pointer returned from COM_Init.
//!
//! \return				The number of chars available
//------------------------------------------------------------------------------
inline DWORD ComputeAvailableChar(T_SERIAL_INIT_STRUCTURE * pSerialInitStructure)
{
	// Compute free buffer space to flow control system
	if (pSerialInitStructure->dwPAControllerRxData < pSerialInitStructure->dwPAUserRxData)
	{
		return ((pSerialInitStructure->dwPADmaRxBufferTrueEnd - pSerialInitStructure->dwPADmaRxBufferStart) - (pSerialInitStructure->dwPAUserRxData - pSerialInitStructure->dwPAControllerRxData));
	}
	else
	{
		return (pSerialInitStructure->dwPAControllerRxData - pSerialInitStructure->dwPAUserRxData);
	}
}

//------------------------------------------------------------------------------
//! \fn		static BOOL SetThresHold(T_SERIAL_INIT_STRUCTURE * pSerialInitStructure, DWORD dwBufferLength)
//!
//! \brief	This function made two works : first test is chars received are sufficient for finish read, if not
//!			set new dma register value to read the correct lefting char to finish read
//!
//! \param  pSerialInitStructure Serial init structure
//!
//! \param  dwBufferLength       Number of characters lefting to read
//!
//! \return TRUE  if sufficient chars are arrived
//! \return FALSE if not
//------------------------------------------------------------------------------
BOOL SetThresHold(T_SERIAL_INIT_STRUCTURE * pSerialInitStructure, DWORD dwBufferLength)
{
	DWORD dwAvailableChar;
	BOOL bRetVal;
	DWORD dwRpr, dwRcr, dwRnpr, dwRncr;
//	DWORD dwAvailableSpace;

	// \warning Disable interrupt to minimize the section where PDC is disabled.
	// If the current thread have a low priority, the following code can't be interrupt.
	DisableInt();
	// Disable received pdc
	HWDisableReceive(pSerialInitStructure);
	// PDC need to be disable, because bewteen the calcul of the available char
	// and the calcul of the new RR value, pdc value must not change !

	// Update controller read data
	// Don't need protection because, the code is always in a critical section
	pSerialInitStructure->dwPAControllerRxData = pSerialInitStructure->pPDCReg->PDC_RPR;

	// Search available space
	dwAvailableChar = ComputeAvailableChar(pSerialInitStructure);

	// Test if there are enougth chars in buffer
	if (dwAvailableChar >= dwBufferLength)
	{
		// If there are enought car in buffer, let register value to their current value
		//RETAILMSG(1,(L"(dwAvailableChar >= dwBufferLength)"));
		// Return true to indicate that there are sufficent char in buffer
		bRetVal = TRUE;
	}
	else
	{
		// Search new DMA Register Values for resting char wanted
		ComputeRR(pSerialInitStructure, (dwBufferLength-dwAvailableChar), &dwRpr, &dwRcr);
		// Compute new next register value
		ComputeRNR(pSerialInitStructure, dwRpr, dwRcr, &dwRnpr, &dwRncr);
		
		// Set new value in register
		HWSetRxDmaRegisterValue(pSerialInitStructure, dwRpr, dwRcr, dwRnpr, dwRncr);

		if (dwRncr > 0)
		{
			// Enable RX interrupt
			HWEnableRxInterrupt(pSerialInitStructure);
		}

		// Char already arrived are'nt sufficient to finish read
		bRetVal = FALSE;
	}

	// Enable received pdc
	HWEnableReceive(pSerialInitStructure);
	// \warning Enable interrupt to end critical section.
	// The code below is the shortest as possible
	EnableInt();

	return bRetVal;
}

//------------------------------------------------------------------------------
//!	\fn					BOOL ComputeRR(T_SERIAL_INIT_STRUCTURE * pSerialInitStructure, DWORD dwWantedLen, DWORD * pdwRPR, DWORD * pdwRCR)
//!
//! \brief				Compute the value for the received register (usart dma register)
//! 
//! \param	pSerialInitStructure	Context pointer returned from COM_Init.
//!
//! \param  dwWantedLen		Number of char wanted to read
//!
//! \param  pdwRPR			(out) Value for RPR register
//!
//! \param  pdwRCR			(out) Value for RCR register
//!
//! \return				Always true
//------------------------------------------------------------------------------
BOOL ComputeRR(T_SERIAL_INIT_STRUCTURE * pSerialInitStructure, DWORD dwWantedLen, DWORD * pdwRPR, DWORD * pdwRCR)
{
	DWORD dwLen;

	// RPR = controller read pointer
	*pdwRPR = pSerialInitStructure->dwPAControllerRxData;

	// Search len to read (RCR)
	// - BufferEnd
	// - UserRxData

	// - Maximum value must be (buffersize/2)

	// If UserRxData is smaller than the ControllerRxData, controller can write until the end of the buffer
	if (pSerialInitStructure->dwPAUserRxData <= pSerialInitStructure->dwPAControllerRxData)
	{
		dwLen = pSerialInitStructure->dwPADmaRxBufferEnd - pSerialInitStructure->dwPAControllerRxData;
	}
	else
	{
		dwLen = pSerialInitStructure->dwPAUserRxData - pSerialInitStructure->dwPAControllerRxData;
	}

	// Is length possible is higher than length wanted
	dwLen = MIN(dwLen, dwWantedLen);

	// If hardware hand shake activated, test if length not exceed autorized limit
	if (pSerialInitStructure->dcb.fRtsControl == RTS_CONTROL_HANDSHAKE)
	{
		DWORD dwAvailableSpace = ComputeFreeSpace(pSerialInitStructure);

		// Is length possible is higer than flow control limit
		dwLen = MIN(dwLen
				   ,(pSerialInitStructure->dwRxFCLLimit - (pSerialInitStructure->dwRxBufferSize - dwAvailableSpace))
				   );
	}
	
	// Is length is higher than buffersize / 2
	*pdwRCR = MIN(dwLen, (pSerialInitStructure->dwRxBufferSize/2) );

	
	// If event EV_RXCHAR is wanted
	if ((pSerialInitStructure->fEventMask & EV_RXCHAR) == EV_RXCHAR)
	{
		// If event EV_RXCHAR is not already set
		if ((pSerialInitStructure->fEventMaskSet & EV_RXCHAR) != EV_RXCHAR)
		{
			// Set RCR max len to 1
			*pdwRCR = MIN(*pdwRCR, 1);
		}
	}
	

	return TRUE;
}

//------------------------------------------------------------------------------
//!	\fn					BOOL ComputeRR(T_SERIAL_INIT_STRUCTURE * pSerialInitStructure, DWORD dwWantedLen, DWORD * pdwRPR, DWORD * pdwRCR)
//!
//! \brief				Compute the value for the received register (usart dma register)
//! 
//! \param	pInitStructure	Context pointer returned from COM_Init.
//!
//! \param  dwWantedLen		Number of char wanted to read
//!
//! \param  dwRPR			(in) Value of RPR register
//!
//! \param  dwRCR			(in) Value of RCR register
//!
//! \param  pdwRNPR			(out) Value for RNPR register
//!
//! \param  pdwRNCR			(out) Value for RNCR register
//!
//! \return				TRUE  if value are compute
//! \return				FALSE if no value compute for RNR
//------------------------------------------------------------------------------
BOOL ComputeRNR(T_SERIAL_INIT_STRUCTURE * pSerialInitStructure, DWORD dwRPR, DWORD dwRCR, DWORD * pdwRNPR, DWORD * pdwRNCR)
{
	DWORD dwLen;

	// Compute new RNPR
	*pdwRNPR = dwRPR + dwRCR;

	// If RNPR is higher than the limit,
	if (*pdwRNPR > pSerialInitStructure->dwPADmaRxBufferPRLimit)
	{
		if (pSerialInitStructure->dwPAUserRxData < *pdwRNPR)
		{
			// Test if user pointer is not at buffer start
			if (pSerialInitStructure->dwPAUserRxData != pSerialInitStructure->dwPADmaRxBufferStart)
			{
				// Set True End
				pSerialInitStructure->dwPADmaRxBufferTrueEnd = *pdwRNPR;

				// Set dma next pointer register at buffer start
				*pdwRNPR = pSerialInitStructure->dwPADmaRxBufferStart;
			}
		}
		else
		{
			// Return false to indicate that no rnr value have been computed
			// This is due to a full buffer
			return FALSE;
		}
	}

	// If UserRxData is smaller than the RNPR, controller can write until the end of the buffer
	if (pSerialInitStructure->dwPAUserRxData <= *pdwRNPR)
	{
		dwLen = (pSerialInitStructure->dwPADmaRxBufferEnd - *pdwRNPR);
	}
	else
	{
		// Len is until user pointer - 1
		dwLen = ((pSerialInitStructure->dwPAUserRxData-1) - *pdwRNPR);
	}

	// Is length is higher than buffersize / 2
	*pdwRNCR = MIN(dwLen, (pSerialInitStructure->dwRxBufferSize/2) );

	if (dwLen == 0)
	{
		return FALSE;
	}

	return TRUE;
}

//------------------------------------------------------------------------------
//! \fn		static DWORD ReadAvailableData(T_SERIAL_INIT_STRUCTURE * pSerialInitStructure, DWORD dwWantedByteNb, UCHAR ** pBufferDest) 
//!
//! \brief	Read available data in received buffer.
//!
//! \param  pSerialInitStructure Serial init structure
//!
//! \param  dwWantedByteNb       Number of characters wanted to read
//!
//! \param  pBufferDest          Pointer where write data. (this pointer is modified)
//!
//! \return Number of charaters resting
//------------------------------------------------------------------------------
DWORD ReadAvailableData(T_SERIAL_INIT_STRUCTURE * pSerialInitStructure, DWORD dwWantedByteNb, UCHAR ** pBufferDest)
{
	DWORD dwLen, dwWantedByteNbTmp;
	UCHAR * pucCopySrc;

	dwWantedByteNbTmp = dwWantedByteNb;

	// Update controller read data
	EnterCriticalSection(&pSerialInitStructure->csUsartReg);
	pSerialInitStructure->dwPAControllerRxData = pSerialInitStructure->pPDCReg->PDC_RPR;
	LeaveCriticalSection(&pSerialInitStructure->csUsartReg);

	// While not all char readed and there are always some chars in buffer
	while ( (dwWantedByteNb > 0) && (pSerialInitStructure->dwPAUserRxData != pSerialInitStructure->dwPAControllerRxData) )
	{
		if (pSerialInitStructure->dwPAUserRxData > pSerialInitStructure->dwPAControllerRxData)
		{
			// Search len possible to copied
			dwLen = pSerialInitStructure->dwPADmaRxBufferTrueEnd - pSerialInitStructure->dwPAUserRxData;
		}
		else // (if (pSerialInitStructure->dwPAUserRxData <= pSerialInitStructure->dwPAControllerRxData))
		{
			// Search len possible to copied
			dwLen = pSerialInitStructure->dwPAControllerRxData - pSerialInitStructure->dwPAUserRxData;
		}

		if (dwLen > dwWantedByteNb)
		{
			dwLen = dwWantedByteNb;
		}

		// Convert physical address to virtual address
		pucCopySrc = pSerialInitStructure->pDmaRxBufferStart + (pSerialInitStructure->dwPAUserRxData - pSerialInitStructure->dwPADmaRxBufferStart);

		// Copy data
		memcpy((UCHAR*)*pBufferDest, pucCopySrc , dwLen);
		
		// Increment correct pointer and counter
		pSerialInitStructure->dwPAUserRxData += dwLen;

		// If User read data pointer has attempt the buffer true end
		if (pSerialInitStructure->dwPAUserRxData == pSerialInitStructure->dwPADmaRxBufferTrueEnd)
		{
			// Reset the User start pointer
			pSerialInitStructure->dwPAUserRxData = pSerialInitStructure->dwPADmaRxBufferStart;
			// Reset the true end buffer pointer
			pSerialInitStructure->dwPADmaRxBufferTrueEnd = pSerialInitStructure->dwPADmaRxBufferEnd;
		}

		(*pBufferDest) += dwLen;
		dwWantedByteNb -= dwLen;
	}

	// Test if some chars are arrived,
	if (dwWantedByteNbTmp != dwWantedByteNb)
	{
		DWORD dwRNPR, dwRNCR, dwAvailableSpace;

		// If a part of the buffer is free, 
		// activate interrupt
		HWEnableRxInterrupt(pSerialInitStructure);

		// \warning Disable interrupt to minimize the section where PDC is disabled.
		// If the current thread have a low priority, the following code can't be interrupt.
		DisableInt();
		// Disable received pdc
		HWDisableReceive(pSerialInitStructure);
		// PDC need to be disable, because bewteen the calcul of the available char
		// and the calcul of the new RR value, pdc value must not change !
	
		// FLOW CONTROL MECHANISM: if flow control activated
		if (pSerialInitStructure->dcb.fRtsControl == RTS_CONTROL_HANDSHAKE)
		{
			dwAvailableSpace = ComputeFreeSpace(pSerialInitStructure);

			// If no enougth available data ... 
			if ((pSerialInitStructure->dwRxBufferSize - dwAvailableSpace) < pSerialInitStructure->dwRxFCHLimit)
			{
				//... set RTS line
				HWClearRTS(pSerialInitStructure);
			}
		}

		// Compute the new dma received next register
		if (ComputeRNR	(pSerialInitStructure
						,pSerialInitStructure->pPDCReg->PDC_RPR
						,pSerialInitStructure->pPDCReg->PDC_RCR
						,&dwRNPR
						,&dwRNCR
						) == TRUE)
		{
			// Apply new value to dma next buffer register
			HWSetRxDmaRegisterNextValue(pSerialInitStructure, dwRNPR, dwRNCR);
		}
		else
		{
			HWDisableRxENDInterrupt(pSerialInitStructure);
		}

		// Enable received pdc
		HWEnableReceive(pSerialInitStructure);
		// \warning Enable interrupt to end critical section.
		// The code below is the shortest as possible
		EnableInt();
	}

	return dwWantedByteNb;
}



//------------------------------------------------------------------------------
//! End of $URL: http://centaure/svn/interne-ce_bsp_atmel/TAGS/TAGS50/SAM9263EK_v100_rc4/PLATFORM/COMMON/SRC/ARM/ATMEL/AT91SAM926x/DRIVERS/Serial/Serial_SAM926X.cpp $
//------------------------------------------------------------------------------

//
//! @}
//! @}
//
