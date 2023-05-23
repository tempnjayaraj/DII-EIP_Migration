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
//!   $Author: Tenneyd $
//!   $Revision: 2 $
//!   $Date: 2/03/09 3:35p $
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

//#define USE_BACKLIGHT_PWM


class at91sam9263ekLCD : public LCDC63hw
{
public:
	at91sam9263ekLCD(DWORD dwBaseVideoMemStartAddress, DWORD dwVideoMemHeight, DWORD dwVideoMemWidth, DWORD dwVideoMemBusWidth);
private:
	bool BoardSpecificGraphicInit();
	void ContrastInit();
	void IOConfig();
//	void ChipConfig();
	void BacklightInit();
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

	// Initialize IOs and peripherals
	IOConfig();

	// Initialize the Chip to correctly drive the screen
	ChipConfig();

	// Initialize the contrast
	ContrastInit();

	BacklightInit();



	return true;
}

	
// Internal function, get pointers to registers & set PIO/Peripheral configuration
void at91sam9263ekLCD::IOConfig()
{
        /*((unsigned int) AT91C_PC1_LCDHSYNC) |
        ((unsigned int) AT91C_PC2_LCDDOTCK) |             
        ((unsigned int) AT91C_PC6_LCDD2   ) |
        ((unsigned int) AT91C_PC7_LCDD3   ) |
        ((unsigned int) AT91C_PC8_LCDD4   ) |
        ((unsigned int) AT91C_PC9_LCDD5   ) |
        ((unsigned int) AT91C_PC10_LCDD6  ) |
        ((unsigned int) AT91C_PC11_LCDD7  ) |
        ((unsigned int) AT91C_PC14_LCDD10 ) |
        ((unsigned int) AT91C_PC15_LCDD11 ) |
        ((unsigned int) AT91C_PC16_LCDD12 ) |	
        ((unsigned int) AT91C_PC18_LCDD14 ) |
        ((unsigned int) AT91C_PC19_LCDD15 ) |
        ((unsigned int) AT91C_PC22_LCDD18 ) |
        ((unsigned int) AT91C_PC23_LCDD19 ) |
        ((unsigned int) AT91C_PC24_LCDD20 ) |
        ((unsigned int) AT91C_PC26_LCDD22 ) |
        ((unsigned int) AT91C_PC27_LCDD23 ) ,
        ((unsigned int) AT91C_PC17_LCDD21B ) |
        ((unsigned int) AT91C_PC12_LCDD13B ));*/
	const struct pio_desc hw_pio[] = {
		{"RST",		AT91C_PIN_PA(30),	1, PIO_DEFAULT, PIO_OUTPUT},
		{"HSYNC",	AT91C_PIN_PC(1),	0, PIO_DEFAULT, PIO_PERIPH_A},
		{"DOTCK",	AT91C_PIN_PC(2),	0, PIO_DEFAULT, PIO_PERIPH_A},
		{"D2",		AT91C_PIN_PC(6),	0, PIO_DEFAULT, PIO_PERIPH_A},
		{"D3",		AT91C_PIN_PC(7),	0, PIO_DEFAULT, PIO_PERIPH_A},
		{"D4",		AT91C_PIN_PC(8),	0, PIO_DEFAULT, PIO_PERIPH_A},
		{"D5",		AT91C_PIN_PC(9),	0, PIO_DEFAULT, PIO_PERIPH_A},
		{"D6",		AT91C_PIN_PC(10),	0, PIO_DEFAULT, PIO_PERIPH_A},
		{"D7",		AT91C_PIN_PC(11),	0, PIO_DEFAULT, PIO_PERIPH_A},
		{"D10",		AT91C_PIN_PC(14),	0, PIO_DEFAULT, PIO_PERIPH_A},
		{"D11",		AT91C_PIN_PC(15),	0, PIO_DEFAULT, PIO_PERIPH_A},
		{"D12",		AT91C_PIN_PC(16),	0, PIO_DEFAULT, PIO_PERIPH_A},
		{"D14",		AT91C_PIN_PC(18),	0, PIO_DEFAULT, PIO_PERIPH_A},
		{"D15",		AT91C_PIN_PC(19),	0, PIO_DEFAULT, PIO_PERIPH_A},
		{"D18",		AT91C_PIN_PC(22),	0, PIO_DEFAULT, PIO_PERIPH_A},
		{"D19",		AT91C_PIN_PC(23),	0, PIO_DEFAULT, PIO_PERIPH_A},
		{"D20",		AT91C_PIN_PC(24),	0, PIO_DEFAULT, PIO_PERIPH_A},
		{"D22",		AT91C_PIN_PC(26),	0, PIO_DEFAULT, PIO_PERIPH_A},
		{"D23",		AT91C_PIN_PC(27),	0, PIO_DEFAULT, PIO_PERIPH_A},

		{"D13",		AT91C_PIN_PC(12),	0, PIO_DEFAULT, PIO_PERIPH_B},
		{"D21",		AT91C_PIN_PC(17),	0, PIO_DEFAULT, PIO_PERIPH_B},

		{"CC",		AT91C_PIN_PB(9),	0, PIO_DEFAULT, PIO_PERIPH_B},

	};
 
	pio_setup(hw_pio, sizeof(hw_pio)/sizeof(struct pio_desc));

/*	// Configure PIO controllers to periph mode for LCDC
	AT91F_PIO_CfgPeriph(
		pPIOC, // PIO controller base address
        ((unsigned int) AT91C_PC1_LCDHSYNC) |
        ((unsigned int) AT91C_PC2_LCDDOTCK) |             
        ((unsigned int) AT91C_PC6_LCDD2   ) |
        ((unsigned int) AT91C_PC7_LCDD3   ) |
        ((unsigned int) AT91C_PC8_LCDD4   ) |
        ((unsigned int) AT91C_PC9_LCDD5   ) |
        ((unsigned int) AT91C_PC10_LCDD6  ) |
        ((unsigned int) AT91C_PC11_LCDD7  ) |
        ((unsigned int) AT91C_PC14_LCDD10 ) |
        ((unsigned int) AT91C_PC15_LCDD11 ) |
        ((unsigned int) AT91C_PC16_LCDD12 ) |	
        ((unsigned int) AT91C_PC18_LCDD14 ) |
        ((unsigned int) AT91C_PC19_LCDD15 ) |
        ((unsigned int) AT91C_PC22_LCDD18 ) |
        ((unsigned int) AT91C_PC23_LCDD19 ) |
        ((unsigned int) AT91C_PC24_LCDD20 ) |
        ((unsigned int) AT91C_PC26_LCDD22 ) |
        ((unsigned int) AT91C_PC27_LCDD23 ) ,
        ((unsigned int) AT91C_PC17_LCDD21B ) |
        ((unsigned int) AT91C_PC12_LCDD13B ));

	// Configure PIO controllers to periph mode for LCDC
	AT91F_PIO_CfgPeriph(
		pPIOB, // PIO controller base address
		0, // Peripheral A
		((unsigned int) AT91C_PB9_LCDCC   )); // Peripheral B
*/
}

// Internal function
// Initialize the contrast driver if added to the OS Design
void at91sam9263ekLCD::ContrastInit()
{
	// Initialisation of the contrast 	
	// This function call the Contrast driver to initialize them with the value
	// which is written in the registry.
	
		DWORD dwValue = 0;

	//	Read The Registry Value : 

	// Description of the Registry :
		HKEY hkeyRoot = HKEY_LOCAL_MACHINE; 
		LPCTSTR lpSubkey = _T("Drivers\\BuiltIn\\Contrast");
		LPCTSTR lpValueName = _T("Contrast"); 
	//Open the registry
		HKEY hkey;
		LONG lRet = RegOpenKeyEx(hkeyRoot,lpSubkey, 0, 0, &hkey);
		if (lRet == ERROR_SUCCESS)
		{
	//Read the value of the last change: 
		DWORD dwData;
		DWORD dwBufLen = sizeof(DWORD);
		DWORD dwType = REG_DWORD;
	//Read Value in the Registry: 
		lRet = RegQueryValueEx(hkey,lpValueName, NULL, &dwType, (LPBYTE) &dwData, &dwBufLen);
		dwValue = dwData;               
	//Close
		RegCloseKey(hkey);
    
		}

	//	Send the value to the Driver Contrast : 
		HANDLE hDeviceContrast = NULL;
		hDeviceContrast = CreateFile(
							  _T("CTR1:"), 
							  GENERIC_READ | GENERIC_WRITE, 
							  0, 
							  NULL, 
							  OPEN_ALWAYS, 
							  FILE_ATTRIBUTE_NORMAL, 
							  0
							); 
		DWORD dwNumBytes = 0;
		DeviceIoControl(hDeviceContrast, 
							  IOCTL_CONTRAST_SET_LEVEL, 
							  (LPVOID) &dwValue, 
							  sizeof (dwValue), 
							  NULL, 
							  NULL, 
							  &dwNumBytes, 
							  NULL
		);
}


void at91sam9263ekLCD::BacklightInit()
{
	BOOL bUsePio = TRUE;
	
#ifdef USE_BACKLIGHT_PWM
	HANDLE hBackLightDevice= NULL;
	T_IOCTLPWC_CONFIG cfg;

	hBackLightDevice = CreateFile(
							  _T("PWC1:"), 
							  GENERIC_READ | GENERIC_WRITE, 
							  0, 
							  NULL, 
							  OPEN_ALWAYS, 
							  FILE_ATTRIBUTE_NORMAL, 
							  0
							); 
	if (hBackLightDevice != INVALID_HANDLE_VALUE)
	{
		cfg.dwFrequency = 200;
		cfg.dwDutyCycle = 90;
		
		if (DeviceIoControl(hBackLightDevice, IOCTL_PWC_CONFIG,&cfg, sizeof(cfg), NULL, 0, NULL, NULL))
		{
			bUsePio = FALSE;			
		}
	}
#endif

	if (bUsePio)
	{
		const struct pio_desc hw_pio[] = {
			{"EN",	AT91C_PIN_PC(3),	0, PIO_DEFAULT, PIO_PERIPH_A}
		};
		pio_setup(hw_pio, sizeof(hw_pio)/sizeof(struct pio_desc));
	}	
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