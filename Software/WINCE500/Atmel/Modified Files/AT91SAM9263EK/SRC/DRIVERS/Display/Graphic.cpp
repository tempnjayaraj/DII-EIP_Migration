//-----------------------------------------------------------------------------
//! \addtogroup DRIVERS
//! @{
//
//
//  All rights reserved ADENEO SAS 2005
//!
//-----------------------------------------------------------------------------
//! \file		graphic.cpp
//!
//! \brief		Platform specific intialisations for GraphicChip
//!
//! \if subversion
//!   $URL: http://centaure/svn/interne-ce_bsp_atmel/TRUNK50/PLATFORM/AT91SAM9263EK/SRC/DRIVERS/Display/Graphic.cpp $
//!   $Author: Woodlandk $
//!   $Revision: 1 $
//!   $Date: 1/29/09 3:26p $
//! \endif
//! 
//! This is the last file added to the driver libs, after we generate a dll
//! This file contains all board specific parameters like register settings
//! and timings
//! 
//-----------------------------------------------------------------------------
//! \addtogroup	LCDC
//! @{
#include <windows.h>
#include <ceddk.h>

#include "at91sam9263ek.h"
#include "AT91SAM9263_LCDC.h"
#include "AT91SAM926x_oal_ioctl.h"
#include "AT91SAM9263_pwmc_ioctl.h"
#include "at91sam9263ek_ioctl.h"
#include "AT91SAM9263_gpio.h"

// KW - Removed unused functions
class at91sam9263ekLCD : public LCDC63hw
{
public:
	at91sam9263ekLCD(DWORD dwBaseVideoMemStartAddress, DWORD dwVideoMemHeight, DWORD dwVideoMemWidth, DWORD dwVideoMemBusWidth);
private:
	bool BoardSpecificGraphicInit();
};


at91sam9263ekLCD::at91sam9263ekLCD(DWORD dwBaseVideoMemStartAddress, DWORD dwVideoMemHeight, DWORD dwVideoMemWidth, DWORD dwVideoMemBusWidth):LCDC63hw(dwBaseVideoMemStartAddress, dwVideoMemHeight,  dwVideoMemWidth,  dwVideoMemBusWidth)
{

	BoardSpecificGraphicInit();
}
//-----------------------------------------------------------------------------
//! \brief		Initialize all board specific things, register & timings for exemple
//!
//! \return		\e TRUE When all is good
//!	\return		\e FALSE When there is an error during Init
//!
//! This function is declared in the generic driver as an extern, It must be
//! implemented here. The driver call it at init time.
//-----------------------------------------------------------------------------
bool at91sam9263ekLCD::BoardSpecificGraphicInit()
{
	// Initialize the Chip to correctly drive the screen
	ChipConfig();

	return true;
}

// Global
static GPE *gpGPE;

//-----------------------------------------------------------------------------
//! \brief		\return color mask corresponding
//!
//! \param		dhpdev
//-----------------------------------------------------------------------------
ULONG* APIENTRY DrvGetMasks(DHPDEV dhpdev)
{
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("-> DrvGetMasks\n\r")));

	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("<- DrvGetMasks\n\r")));

	return ((at91sam9263ekLCD*)gpGPE)->GetMasks();
}

//-----------------------------------------------------------------------------
//! \brief		\return a new DDGPE device driver or an existant driver.
//! 
//! The GPE class must be unique in the system, so we create her as a singleton
//-----------------------------------------------------------------------------
GPE* GetGPE()
{
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("-> GetGPE\n\r")));
	// If a GPE isn't create, we instantiate the class
	if( !gpGPE )
	{
		DWORD dwVideoMemHeight, dwVideoMemWidth, dwVideoMemStartAddress;
		DWORD dwVideoMemBusWidth, dwBpp;
		
		HKEY hKey;
		DWORD dwBufLen = sizeof(DWORD);
		DWORD dwType = REG_DWORD;
		BOOL bUseDefault = TRUE;
		
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Drivers\\Display\\LCDC", 0, 0, &hKey) != ERROR_SUCCESS)
		{
			DEBUGMSG(GPE_ZONE_ERROR, (TEXT("An error occured while attempting to read registry\n\r")));
		}
		else
		{
			if (RegQueryValueEx(hKey, L"Bpp", 0, &dwType, (LPBYTE)&dwBpp, &dwBufLen) == ERROR_SUCCESS )
			{
				if ( (RegQueryValueEx(hKey, L"VRAMWidthInPixel" , 0, &dwType, (LPBYTE)&dwVideoMemWidth , &dwBufLen) == ERROR_SUCCESS) &&
					(RegQueryValueEx(hKey, L"VRAMHeightInPixel", 0, &dwType, (LPBYTE)&dwVideoMemHeight, &dwBufLen) == ERROR_SUCCESS) &&
					(RegQueryValueEx(hKey, L"VRAMAddress", 0, &dwType, (LPBYTE)&dwVideoMemStartAddress, &dwBufLen) == ERROR_SUCCESS) &&
					(RegQueryValueEx(hKey, L"VRAMBusWidth", 0, &dwType, (LPBYTE)&dwVideoMemBusWidth, &dwBufLen)    == ERROR_SUCCESS) )
				{
					bUseDefault = FALSE;
				}		
				else
				{
					RETAILMSG(1, (TEXT("An error occured while attempting to read registry : Invalid Video Memory description. Using default settings\n\r")));
				}
			}
			else
			{
				RETAILMSG(1, (TEXT("An error occured while attempting to read registry : No Bpp format specified\n\r")));
			}		 		
		}
		
		if (bUseDefault)
		{
			dwVideoMemWidth = AT91SAM9263EK_VIDEO_MEM_WIDTH;
			dwVideoMemHeight = AT91SAM9263EK_VIDEO_MEM_HEIGHT;	
			dwVideoMemStartAddress = AT91SAM9263EK_BASE_VIDEO_MEM;
			dwVideoMemBusWidth = AT91SAM9263EK_VIDEO_MEM_BUS_WIDTH;
		}
		
		
		
		
		
		
		DEBUGMSG(GPE_ZONE_INIT, (TEXT("GetGPE::Creating a new LCDC6xhw\n\r")));
		gpGPE = (DDGPE*)new at91sam9263ekLCD(dwVideoMemStartAddress, dwVideoMemHeight, dwVideoMemWidth, dwVideoMemBusWidth);

	}
	
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("<- GetGPE\n\r")));
	
	// \return a GPE pointer
	return (GPE*)gpGPE;
}


//-----------------------------------------------------------------------------
//! \brief		Redirect Enable call to GPEEnableDriver
//-----------------------------------------------------------------------------
	// This gets around problems exporting from .lib
	extern BOOL APIENTRY GPEEnableDriver(ULONG iEngineVersion, ULONG cj, DRVENABLEDATA* pded, PENGCALLBACKS pEngCallbacks);
BOOL APIENTRY DrvEnableDriver(ULONG iEngineVersion, ULONG cj, DRVENABLEDATA* pded, PENGCALLBACKS pEngCallbacks)
{
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("-> DrvEnableDriver\n\r")));
	

	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("<- DrvEnableDriver\n\r")));

	return GPEEnableDriver( iEngineVersion, cj, pded, pEngCallbacks );
}


//! @}

////////////////////////////////////////////////////////////////////////////////
// End of $URL: http://centaure/svn/interne-ce_bsp_atmel/TRUNK50/PLATFORM/AT91SAM9263EK/SRC/DRIVERS/Display/Graphic.cpp $
////////////////////////////////////////////////////////////////////////////////
//
//! @}