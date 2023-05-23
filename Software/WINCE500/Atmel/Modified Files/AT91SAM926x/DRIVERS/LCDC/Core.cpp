//-----------------------------------------------------------------------------
//! \addtogroup DRIVERS
//! @{
//!
//  All rights reserved ADENEO SAS 2005
//
//-----------------------------------------------------------------------------
//! \file		AT91SAM926x\DRIVERS\LCDC\Core.cpp
//! \bug The Mouse pointer is not properly handled. Because we don't have overlay support in the chip, we must draw manually the pointer in the frame buffer. But this doesn't work as well as it should with Direct Draw.
//! \brief		Implementation of LCDC61hw class, heritating from DDGPE abstract class
//
//! \if subversion
//!   $URL: http://centaure/svn/interne-ce_bsp_atmel/TAGS/TAGS50/SAM9263EK_v100_rc4/PLATFORM/COMMON/SRC/ARM/ATMEL/AT91SAM926x/DRIVERS/LCDC/Core.cpp $
//!   $Author: Woodlandk $
//!   $Revision: 1 $
//!   $Date: 1/29/09 3:26p $
//! \endif
//-----------------------------------------------------------------------------
//! \addtogroup	LCDC
//! @{
//

//! \addtogroup Core
//! @{

#include "LCDC\precomp.h"
#include "LCDC\mode.h"
#include "AT91SAM926x_oal_ioctl.h"
#include "nkintr.h"

#define LCDC_REGISTRY_KEY L"Drivers\\Display\\LCDC"

// If there is no data in registry, we load 'mode 1'
// Modes are defined in mode.h
#define DEFAULT_MODE 5


#define ISTERROR_TIMEOUT 1000 //ms

// The dpCurSettings structure
INSTANTIATE_GPE_ZONES(0x3,"DDI_9262","unused1","unused2")
#define GPE_ZONE_UNUSED1 DEBUGZONE(14)
#define GPE_ZONE_UNUSED2 DEBUGZONE(15)

/*
GPE_ZONE_ERROR    Errors
GPE_ZONE_WARNING  Warnings
GPE_ZONE_PERF     Performance
GPE_ZONE_TEMP     Temporary tests
GPE_ZONE_ENTER    Enter,Exit
GPE_ZONE_INIT     Initialize
GPE_ZONE_BLT_HI   Blt Calls
GPE_ZONE_BLT_LO   Blt Verbose
GPE_ZONE_CREATE   Surface Create
GPE_ZONE_FLIP     Flip
GPE_ZONE_LINE     Line
GPE_ZONE_HW       Hardware
GPE_ZONE_POLY     Polygon
GPE_ZONE_CURSOR   Cursor  
*/

// Blt & Line callback format
#define GPEBLT (SCODE (GPE::* )(GPEBltParms *))
#define GPELINE (SCODE (GPE::* )(GPELineParms *))

extern void LCDCProcSpecificInitPMC(void);
extern void LCDCProcSpecificActivatePMC(void);
extern void LCDCProcSpecificDeactivatePMC(void);
extern DWORD LCDCProcSpecificGetLCDCBaseAddress(void);
extern DWORD LCDCProcSpecificGetLCDCID(void);

static BOOL bLcdcErrorISTRun;

//-----------------------------------------------------------------------------
//! \brief		\return a pointer to physical memory
//-----------------------------------------------------------------------------
void LCDC6xhw::GetPhysicalVideoMemory(DWORD *pPhysicalMemoryBase, DWORD *pVideoMemorySize)
{
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("-> GetPhysicalVideoMemory\n\r")));
	
	*pPhysicalMemoryBase = (DWORD)m_pPhyVidMemAddr;
	*pVideoMemorySize    = (m_VidMemSizeInPxl*m_VidMemBpp)/8;
	
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("<- GetPhysicalVideoMemory\n\r")));
}

//-----------------------------------------------------------------------------
//! \brief		Error management for LCD Controller. It only manages fifo underflow.
//! Fifo underflow can cause shiting on display.
//!  
//-----------------------------------------------------------------------------
int LCDC6xhw::LcdlErrorHandler()
{
  DWORD dwStatus;
  static counter = 0;
  
  while( bLcdcErrorISTRun)
  {
    dwStatus = WaitForSingleObject(hEvent, ISTERROR_TIMEOUT);

	// Check to see if we are finished
    if(!bLcdcErrorISTRun) 
	{
		return 1;
	}
	
    // Make sure we have the object
    if( dwStatus == WAIT_OBJECT_0 )
    {
		//TRAITEMENT !!!
		if ( m_pLCDC->LCDC_ISR & AT91C_LCDC_UFLWI)
		{
            // KW - This should never happen, make sure we know if it does
        	RETAILMSG(1, (TEXT("Lcd Fifo Underflow\n")));
			counter++;

			m_pLCDC->LCDC_DMACON &= ~AT91C_LCDC_DMAEN;
			m_pLCDC->LCDC_PWRCON = 0;						
			while(m_pLCDC->LCDC_PWRCON & AT91C_LCDC_BUSY );
			m_pLCDC->LCDC_CTRSTCON&=~AT91C_LCDC_ENA_PWMGEMENABLED;
			LCDCProcSpecificDeactivatePMC();

			LCDCProcSpecificActivatePMC();
			m_pLCDC->LCDC_CTRSTCON |= AT91C_LCDC_ENA_PWMGEMENABLED ;
			m_pLCDC->LCDC_DMACON |= AT91C_LCDC_DMAEN;
			m_pLCDC->LCDC_PWRCON = AT91C_LCDC_PWR | (0x0F<<1);
		}
		// Finish the interrupt
		m_pLCDC->LCDC_ICR = m_pLCDC->LCDC_ISR;
		InterruptDone( dwSysIntr );

	}
  }
	return 0;
}

int LcdlErrorIST(LPVOID lpvParam)
{
	LCDC6xhw *pLCDC = (LCDC6xhw*)lpvParam;
	return pLCDC->LcdlErrorHandler();
}

//-----------------------------------------------------------------------------
//! \brief		Constructor
//! 
//! Initialize graphic mode, VideoMem and cursor. 
//-----------------------------------------------------------------------------
LCDC6xhw::LCDC6xhw(DWORD dwVideoMemStartAddress,DWORD dwVideoMemHeight, DWORD dwVideoMemWidth)
{
	DWORD dwDeviceId;
	
	DEBUGMSG(GPE_ZONE_ENTER | GPE_ZONE_INIT, (TEXT("-> LCDC6xhw Constructor\n\r")));
	
	m_bForceRGB = FALSE;
	m_bFBisCached = FALSE;
	// Allocation of member variables
	m_pMode = new GPEMode();
	// Mode
	// This mode should be set by reading reg.ini & choosing the closest mode the user want.
	DWORD ModeNB = QueryMode(DEFAULT_MODE);
	
	// Initialize power state mode
	m_Dx = D0;

/*	
	// Initialisation of protected GPE members variables
	m_pMode->modeId		= 0                       ;
	m_pMode->width		= aModes[ModeNB].width    ;
	m_pMode->height		= aModes[ModeNB].height   ;
	m_pMode->Bpp		= aModes[ModeNB].Bpp      ;
	m_pMode->frequency	= aModes[ModeNB].frequency;
	m_pMode->format		= aModes[ModeNB].format   ;
*/	
	DWORD dwVideoMemSize = dwVideoMemHeight * dwVideoMemWidth * m_pMode->Bpp / 8;
	RETAILMSG(1, (TEXT("Display mode #%d, %dx%dx%dbpp @ %dhz"), m_pMode->modeId, m_pMode->width, m_pMode->height, m_pMode->Bpp, m_pMode->frequency));
	
	m_nScreenWidth		= m_pMode->width ;
	m_nScreenHeight		= m_pMode->height;
	
    // Set up the software cursor state.
    m_CursorVisible   = FALSE;
    m_CursorDisabled  = TRUE; 
    m_CursorForcedOff = FALSE;
	
	// Cursor Init
	InitCursor();
	
	m_iRotate = GetRotateModeFromReg();
    SetRotateParams();
	m_pMode->width		= m_nScreenWidth    ;
	m_pMode->height		= m_nScreenHeight   ;
	
	
	m_pPhyVidMemAddr = dwVideoMemStartAddress;

	m_pVirtVidMemAddr = VirtualAlloc(0, dwVideoMemSize, MEM_RESERVE, PAGE_NOACCESS);

    if ( m_pVirtVidMemAddr )
    {
        if (VirtualCopy((void *)m_pVirtVidMemAddr, (void *)(m_pPhyVidMemAddr >> 8), dwVideoMemSize, PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL))
        {
            CeSetMemoryAttributes ((void *)m_pVirtVidMemAddr, (void *)(m_pPhyVidMemAddr >> 8), dwVideoMemSize, PAGE_WRITECOMBINE);
        }
    }
	
	/*
	// We are mapping framebuffer into a public memory location
	PUCHAR hLAWMapping = (PUCHAR)CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, dwVideoMemSize, NULL);
	
	if (hLAWMapping != NULL)
	{
		m_pVirtVidMemAddr = (PUCHAR)MapViewOfFile(hLAWMapping, FILE_MAP_WRITE, 0, 0, 0);
	} else {
		m_pVirtVidMemAddr = NULL;
		DEBUGMSG(GPE_ZONE_HW, (TEXT("ERROR : Video memory can't be reserved. Display can't initialize.")));
		return;
	}
	
	if (!VirtualCopy(m_pVirtVidMemAddr, (void*)(dwVideoMemStartAddress>>8), dwVideoMemSize, PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL))
	{
		DWORD myerror = GetLastError();
		DEBUGMSG(GPE_ZONE_ERROR, (TEXT("LCDC6xhw::VirtualCopy failed with (%d) code, please check memory alignements\n\r"), myerror));
		return;
	}
*/	
	
	
	m_VidMemBpp = m_pMode->Bpp;
	m_VidMemSizeInPxl = (dwVideoMemSize*8) / m_VidMemBpp;
	
	m_VidMemStrideByte = (dwVideoMemWidth * m_VidMemBpp)/8;
	
	DEBUGMSG(GPE_ZONE_ERROR, (TEXT("LCDC6xhw::Video Memory will be (%dw | %dh) in pxl"), dwVideoMemWidth, dwVideoMemSize/m_VidMemStrideByte));
	
	// Creating a new Node2D and allocate a primary surface
	m_pAllVidMem = new Node2D(dwVideoMemWidth, dwVideoMemSize/m_VidMemStrideByte);
	
	
	
	AllocSurface(&m_pPrimarySurface, m_nScreenWidthSave, m_nScreenHeightSave,m_pMode->format, GPE_REQUIRE_VIDEO_MEMORY);
	
	
	if (m_pPrimarySurface)
	{
		((GPESurf *)m_pPrimarySurface)->SetRotation(m_nScreenWidth, m_nScreenHeight, m_iRotate);
	}
	
	if (!m_pPrimarySurface)
	{
		DEBUGMSG(GPE_ZONE_ERROR, (TEXT("LCDC6xhw::AllocSurface failed, maybe there isn't enough VideoMem\n\r")));
		return;
	}
	
	LCDCProcSpecificInitPMC();
	
	PHYSICAL_ADDRESS LCDCBasePhysical;  
	LCDCBasePhysical.LowPart = LCDCProcSpecificGetLCDCBaseAddress();
	LCDCBasePhysical.HighPart = 0;
	RETAILMSG(1,(TEXT("\n\rFrame buffer (0x%x) is %s\r\n"),dwVideoMemStartAddress,m_bFBisCached ? L"Cached" : L"Uncached"));
	m_pLCDC = (AT91PS_LCDC) MmMapIoSpace(LCDCBasePhysical,sizeof(*m_pLCDC),m_bFBisCached ? TRUE : FALSE);

   	ChangeFramePointer(dwVideoMemStartAddress);
	
	pSurfCursorColor = NULL;
	pSurfCursorMask = NULL;
	pSurfCursorXOR = NULL;
	pSurfBackingStore = NULL;
	

	dwDeviceId = LCDCProcSpecificGetLCDCID();
	if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwDeviceId, sizeof(dwDeviceId), &dwSysIntr, sizeof(dwSysIntr), NULL))
	{
		RETAILMSG(1, (TEXT("ERROR: Failed to request the serial sysintr for PIOA.\r\n")));
		dwSysIntr = SYSINTR_UNDEFINED;
	}

	bLcdcErrorISTRun = TRUE;

	HANDLE hThread = CreateThread(  NULL,								// Security
									0,									// No Stack Size
									(LPTHREAD_START_ROUTINE)LcdlErrorIST,							// Interrupt Thread
									this,		// Init stucture give by parameters
									CREATE_SUSPENDED,					// Create Suspended
									NULL								// Thread Id
									);

	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// Initialize interuption and associate it to a Event
    if ( !InterruptInitialize(dwSysIntr,
                              hEvent,
                              NULL,
                              0))
	{
		RETAILMSG(1, (TEXT("ERROR: failed initializing interrupt\n\r")));
    }

	// Valid interrupt to be sure.
    InterruptDone(dwSysIntr);

	ResumeThread(hThread);	


	// Notify the system that we are a power-manageable device
	AdvertisePowerInterface(g_hmodDisplayDll);
	
	DEBUGMSG(GPE_ZONE_ENTER | GPE_ZONE_INIT, (TEXT("<- LCDC6xhw Constructor\n\r")));
}

//-----------------------------------------------------------------------------
//! \brief		Destructor, don't do any specific operations 'cause driver can't be unload
//-----------------------------------------------------------------------------
LCDC6xhw::~LCDC6xhw()
{
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("-> LCDC6xhw::~LCDC6xhw\n\r")));
	
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("<- LCDC6xhw::~LCDC6xhw\n\r")));
}
void LCDC6xhw::ChipConfig()
{
    // KW -  Configure only if chip has been configured by the bootstrap.
    BOOL bConfigure = (m_pLCDC->LCDC_DMACON == 0);

    if (bConfigure)
    {
        RETAILMSG(1, (TEXT("ChipConfig()\n\r")));

        ReadRegistryChipConfig();

        // Stop and reset GraphicChip & DMA
	    m_pLCDC->LCDC_PWRCON	= 0x0c;
	    m_pLCDC->LCDC_DMACON	= DISABLE_DMA;
	    m_pLCDC->LCDC_DMACON	= DMA_RESET;

	    // Select the right BPP value
	    switch(m_pMode->Bpp)
	    {
	    /*
	    case 32:
	    m_pLCDC->LCDC_LCDCON2 = PS32BPP;
	    break;
		    */
		    
	    case 24:
		    m_pLCDC->LCDC_LCDCON2 = PS24BPP;
		    break;
		    
	    case 16:
		    m_pLCDC->LCDC_LCDCON2 = PS16BPP;
		    break;
		    
	    case 8:
		    m_pLCDC->LCDC_LCDCON2 = PS8BPP;
		    break;
		    
	    case 4:
		    m_pLCDC->LCDC_LCDCON2 = PS4BPP;
		    break;
	    }
	    
	    
	    // Compute PxlClock from MasterClock
	    DWORD dwMasterClock;
	    KernelIoControl(IOCTL_HAL_MASTERCLOCK, 0, 0, &dwMasterClock, sizeof(dwMasterClock), 0);
	    
	    DWORD value = dwMasterClock / m_dwPixelClock;
	    
	    if (dwMasterClock % m_dwPixelClock)
		    value++;
	    
	    value = value/2;
	    
	    if (!value)
		    m_pLCDC->LCDC_LCDCON1 = AT91C_LCDC_BYPASS;
	    else
	    {
		    value -= 1;
		    m_pLCDC->LCDC_LCDCON1   = (value << CLKVAL_SHFT);
	    }

	    // KW - Check if the Hsync or Vsync is inverted
	    value = DISPPARAM;
	    if (m_dwHsync & 0x80)
		    value |= INVERT_LINE;
	    if (m_dwVsync & 0x80)
		    value |= INVERT_FRM;

	    // KW - Mask off values to correct sizes
	    m_dwUpperMargin &= 0xff;
	    m_dwLowerMargin &= 0xff;
	    m_dwVsync &= 0x3f;
        m_dwLeftMargin &= 0xff;
	    m_dwRightMargin &= 0x7ff;
	    m_dwHsync &= 0x3f;

	    // Set up Display parameters
	    m_pLCDC->LCDC_LCDCON2   |= value;
	    
	    // Horizontal & Vertical timings
	    m_pLCDC->LCDC_TIM1   = (m_dwUpperMargin << VBP_SHFT) | ((m_dwVsync) << VPW_SHFT) | (m_dwLowerMargin<< VFP_SHFT);
	    m_pLCDC->LCDC_TIM2   = ((m_dwLeftMargin) << HBP_SHFT) | ((m_dwHsync) << HPW_SHFT) | ((m_dwRightMargin) << HFP_SHFT) ;
	    
	    // Some other configurations
	    m_pLCDC->LCDC_LCDFRCFG = (((m_nScreenWidthSave-1)<<21) | (m_nScreenHeightSave-1));	
	    m_pLCDC->LCDC_FIFO   = FIFO_MAX_SIZE;
	    m_pLCDC->LCDC_MVAL   = MVAL;
	    m_pLCDC->LCDC_DP1_2     = 0xA5;
	    m_pLCDC->LCDC_DP4_7     = 0x05AF0FA5;
	    m_pLCDC->LCDC_DP3_5     = 0x000A5A5F;
	    m_pLCDC->LCDC_DP2_3     = 0x00000A5F;
	    m_pLCDC->LCDC_DP5_7     = 0x0FAF5FA5;
	    m_pLCDC->LCDC_DP3_4     = 0x0000FAF5;
	    m_pLCDC->LCDC_DP4_5     = 0x000FAF5F;
	    m_pLCDC->LCDC_DP6_7     = 0x0F5FFAFF;
    }

	// Disable IT's
	m_pLCDC->LCDC_IDR   = 0xFFFFFFFF;
	m_pLCDC->LCDC_GPR   = 0x00000000; 
	
	//Enable FIFO underflow IT, LCDC lost synchronisation with DMA when it happen. LCDC must be reseted
	m_pLCDC->LCDC_IER   = AT91C_LCDC_UFLWI;  

    if (bConfigure)
    {
	    // Enable Contrast
	    m_pLCDC->LCDC_CTRSTCON = CONTRAST;
	    m_pLCDC->LCDC_CTRSTVAL = CONTRASTVAL;
	    
	    m_pLCDC->LCDC_DMACON = DMA_RESET;
	    m_pLCDC->LCDC_FRMCFG = (BRSTLN<<BRSTLN_SHFT) | ((m_nScreenWidthSave * m_nScreenHeightSave * m_pMode->Bpp)/32); 	
	    m_pLCDC->LCDC_DMACON |= AT91C_LCDC_DMAEN;
	    m_pLCDC->LCDC_PWRCON    = AT91C_LCDC_PWR | 0x0F;	

        Config2DEngine();
    }
}


//-----------------------------------------------------------------------------
//! \brief		\return the bit mask for specific display mode
//-----------------------------------------------------------------------------
ULONG* LCDC6xhw::GetMasks()
{
	ULONG* masks;
	
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("-> GetMasks\n\r")));
	
	// TODO : modifiying masks attribution system, actually the mask system
	// is independant of the display mode. It should be specified with the display
	// mode, or in registry
	if (m_bForceRGB == FALSE)
	{
		switch(m_pMode->Bpp)
		{
		case 4:
			masks = gBitMasks4Bpp;
			break;
		case 8:
			masks = gBitMasks8Bpp;
			break;
		case 16:
			masks = gBitMasks16Bpp1555XBGR;
			break;
		case 24:
			masks = gBitMasks24Bpp888BGR;
			break;
		case 32:
			masks = gBitMasks32Bpp888BGR;
			break;
		}
	}
	else
	{
		switch(m_pMode->Bpp)
		{
		case 4:
			masks = gBitMasks4Bpp;
			break;
		case 8:
			masks = gBitMasks8Bpp;
			break;
		case 16:
			masks = gBitMasks16Bpp1555XRGB;
			break;
		case 24:
			masks = gBitMasks24Bpp888RGB;
			break;
		case 32:
			masks = gBitMasks32Bpp888RGB;
			break;
		}
	}
	
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("<- GetMasks\n\r")));
	
	return masks;
}

//-----------------------------------------------------------------------------
//! \brief		\return informations about a specific display mode
//-----------------------------------------------------------------------------
SCODE LCDC6xhw::GetModeInfo(GPEMode* pMode, int modeNo)
{
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("-> GetModeInfo\n\r")));
	
	// \return current mode
	if (pMode)
	{
		pMode->modeId		= m_pMode->modeId;
		pMode->width		= m_pMode->width;
		pMode->height		= m_pMode->height;
		pMode->Bpp			= m_pMode->Bpp;
		pMode->frequency	= m_pMode->frequency;
		pMode->format		= m_pMode->format;
	}
	
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("<- GetModeInfo\n\r")));
	
	return S_OK;
}

//-----------------------------------------------------------------------------
//! \brief		Choose a display mode, first try to read registry, then use dwDefaultMode
//-----------------------------------------------------------------------------
DWORD LCDC6xhw::QueryMode(DWORD dwDefaultMode)
{
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("-> QueryMode\n\r")));
	
	DWORD dwMode = dwDefaultMode;
	DWORD dwTemp, dwTemp2;
	DWORD dwForceRGB, dwForceCachedFrameBuffer;
	HKEY hKey;
	
	
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, LCDC_REGISTRY_KEY, 0, 0, &hKey) != ERROR_SUCCESS)
	{
		DEBUGMSG(GPE_ZONE_ERROR, (TEXT("QueryMode::An error occured while attempting to read registry, defautlt mode (%d) is selected\n\r"), dwMode));
		goto exit;
	}
	
	dwTemp2 = sizeof(DWORD);
	
	// If we can read Registry values
	if ( RegQueryValueEx(hKey, L"Width" , 0, &dwTemp, (LPBYTE)&(m_pMode->width) , &dwTemp2) == ERROR_SUCCESS &&
		RegQueryValueEx(hKey, L"Height", 0, &dwTemp, (LPBYTE)&(m_pMode->height), &dwTemp2) == ERROR_SUCCESS &&
		RegQueryValueEx(hKey, L"Bpp"   , 0, &dwTemp, (LPBYTE)&(m_pMode->Bpp)   , &dwTemp2) == ERROR_SUCCESS )
	{
		dwMode = 0;
		switch(m_pMode->Bpp)
		{
		case 8:
			m_pMode->format = gpe8Bpp;
			break;
		case 16:
			m_pMode->format = gpe16Bpp;
			break;
		case 24:
			m_pMode->format = gpe24Bpp;
			break;
		case 32:
			m_pMode->format = gpe32Bpp;
			break;
		default:
			m_pMode->format = gpe16Bpp;
			break;	
		}

	} else {
		DEBUGMSG(GPE_ZONE_ERROR, (TEXT("QueryMode::An error occured while attempting to read registry, defautlt mode (%d) is selected\n\r"), dwMode));
	}
	
	dwTemp2 = sizeof(DWORD);
	
	if (RegQueryValueEx(hKey, L"ForceRGB", 0, &dwTemp, (LPBYTE)&dwForceRGB   , &dwTemp2) == ERROR_SUCCESS )
	{
		m_bForceRGB = dwForceRGB ? TRUE: FALSE;
	}
	
	dwTemp2 = sizeof(DWORD);
	
	if (RegQueryValueEx(hKey, L"Cached", 0, &dwTemp, (LPBYTE)&dwForceCachedFrameBuffer  , &dwTemp2) == ERROR_SUCCESS )
	{
		m_bFBisCached = dwForceCachedFrameBuffer ? TRUE: FALSE;
	}
	
	
	
	
	RegCloseKey(hKey);
	
exit:
	
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("<- QueryMode\n\r")));
	return dwMode;
}

//-----------------------------------------------------------------------------
//! \brief		Read registry configuration for Chip Config
//-----------------------------------------------------------------------------
void LCDC6xhw::ReadRegistryChipConfig()
{
	HKEY hKey;
	DWORD dwBufLen = sizeof(DWORD);
	DWORD dwType = REG_DWORD;
	
	
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, LCDC_REGISTRY_KEY, 0, 0, &hKey) != ERROR_SUCCESS)
	{
		DEBUGMSG(GPE_ZONE_ERROR, (TEXT("An error occured while attempting to read registry\n\r")));
	}
	else
	{
		dwType = REG_DWORD;
		dwBufLen = sizeof(DWORD);
		
		if((RegQueryValueEx(hKey, L"UpperMargin", 0, &dwType, (LPBYTE)&m_dwUpperMargin, &dwBufLen) != ERROR_SUCCESS) || 
			(RegQueryValueEx(hKey, L"LowerMargin", 0, &dwType, (LPBYTE)&m_dwLowerMargin, &dwBufLen) != ERROR_SUCCESS) ||
			(RegQueryValueEx(hKey, L"RightMargin", 0, &dwType, (LPBYTE)&m_dwRightMargin, &dwBufLen) != ERROR_SUCCESS) ||
			(RegQueryValueEx(hKey, L"LeftMargin", 0, &dwType, (LPBYTE)&m_dwLeftMargin, &dwBufLen) != ERROR_SUCCESS) ||
			(RegQueryValueEx(hKey, L"Vsync", 0, &dwType, (LPBYTE)&m_dwVsync, &dwBufLen) != ERROR_SUCCESS) ||
			(RegQueryValueEx(hKey, L"Hsync", 0, &dwType, (LPBYTE)&m_dwHsync, &dwBufLen) != ERROR_SUCCESS) ||
			(RegQueryValueEx(hKey, L"PixelClock", 0, &dwType, (LPBYTE)&m_dwPixelClock, &dwBufLen)!= ERROR_SUCCESS) )
		{
			m_dwUpperMargin = 0;
			m_dwLowerMargin = 0;
			m_dwRightMargin = 0;
			m_dwLeftMargin = 0;
			m_dwPixelClock = 6000000;
			m_dwVsync = 1;
			m_dwHsync = 1;
		}
	}
	RegCloseKey(hKey);
}

//-----------------------------------------------------------------------------
//! \brief		\return number of different display mode avaiable
//-----------------------------------------------------------------------------
int LCDC6xhw::NumModes()
{
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("-> NumModes\n\r")));
	
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("<- NumModes\n\r")));
	
	// We support just one mode
	return 1;
}

//-----------------------------------------------------------------------------
//! \brief		Select a specific display mode
//-----------------------------------------------------------------------------
SCODE LCDC6xhw::SetMode(int modeId, HPALETTE* pPaletteHandle)
{
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("-> SetMode\n\r")));
	
	
	
	
		// Create a palette
		switch (m_pMode->Bpp)
		{
			// We use windows helper to create a 4Bpp  or a 8bpp palette
		case 4:
			*pPaletteHandle = EngCreatePalette(PAL_INDEXED, 16, (ULONG*)g_LUT4Bpp, 0, 0, 0);
			break;
		case 8:
			{				
				*pPaletteHandle = EngCreatePalette(PAL_INDEXED, 256, (ULONG*)rgbPalette, 0, 0, 0);
				InitLut();
			}
			break;
			
			// We doesn't need a palette
		case 16:
		case 24:
		case 32:
			{
				ULONG* pMask = GetMasks();
				// TODO : Create smthg good here, when you change the system of mask attribution, you'd to
				// rewrite this line
				*pPaletteHandle = EngCreatePalette(PAL_BITFIELDS, 0, 0, pMask[0], pMask[1], pMask[2]);
				*pPaletteHandle = EngCreatePalette(PAL_BITFIELDS, 0, 0, pMask[0], pMask[1], pMask[2]);
			}
			break;
		}
	
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("<- SetMode\n\r")));
	
	return S_OK;
}

//-----------------------------------------------------------------------------
//! \brief		Here for debug use only, to generate stats on blt drawing
//-----------------------------------------------------------------------------
#ifdef __CALLSTATS__
void BltStats(GPEBltParms* pBltParms)
{
	static WORD *aRop4Code = 0;
	static DWORD *aTotalArea = 0;
	int offset = 0;
	
	// First call, allocate memory
	if (!aRop4Code)
	{
		aRop4Code = new WORD[0x20000];
		memset(aRop4Code, 0, 0x20000 * sizeof(WORD));
		
		aTotalArea = new DWORD[0x20000];
		memset(aTotalArea, 0, 0x20000 * sizeof(DWORD));
	}
	
	// Check if there is only VideoMem, or SystemMem
	if (!pBltParms->pDst->InVideoMemory())
	{
		offset = 0x10000;
	}
	
	if (pBltParms->pSrc && !pBltParms->pSrc->InVideoMemory())
	{
		offset = 0x10000;
	}
	
	if (pBltParms->pMask && !pBltParms->pMask->InVideoMemory())
	{
		offset = 0x10000;
	}
	
	if (pBltParms->pBrush && !pBltParms->pBrush->InVideoMemory())
	{
		offset = 0x10000;
	}
	
	// Create stats array, for Video or System memory
	aRop4Code[offset+pBltParms->rop4]++;
	
	if (pBltParms->prclDst)
	{
		aTotalArea[offset+pBltParms->rop4] += (abs(pBltParms->prclDst->right - pBltParms->prclDst->left)) * (abs(pBltParms->prclDst->bottom - pBltParms->prclDst->left));
	}
	
	// To display results, place a breakpoint here and jump into lower code:
	int Display = false;
	if (Display)
	{
		int i=0;
		
		for (; i<0x20000; i++)
		{
			if (aRop4Code[i]!=0)
			{
				DEBUGMSG(1, (TEXT("pBltParms->rop4[%04X] ; VideoMem[%s] ; Called[%u] ; NbPxl[%u]"), 0x0000FFFF&i, (i<0x10000?L"true":L"false"), aRop4Code[i], aTotalArea[i]));
			}
		}
		
		memset(aRop4Code, 0, 0x20000 * sizeof(WORD));
		memset(aTotalArea, 0, 0x20000 * sizeof(DWORD));
	}
}
#endif //__CALLSTATS__

//-----------------------------------------------------------------------------
//! \brief		Prepare a bitblit 
//-----------------------------------------------------------------------------
SCODE LCDC6xhw::BltPrepare(GPEBltParms* pBltParms)
{
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("-> BltPrepare")));
	
#ifdef __CALLSTATS__
	BltStats(pBltParms);
#endif
	
	pBltParms->pBlt = GPEBLT &LCDC6xhw::DoBlt;
	
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("<- BltPrepare")));
	
	return S_OK;
}


ULONG  LCDC6xhw::DrvEscape(
						   SURFOBJ *pso,
						   ULONG    iEsc,
						   ULONG    cjIn,
						   PVOID    pvIn,
						   ULONG    cjOut,
						   PVOID    pvOut)
{
	ULONG ulResult = -1;
	
	switch (iEsc)
	{
	case QUERYESCSUPPORT:
		if ( (cjIn == sizeof(DWORD)) && (pvIn != NULL) )
		{
			DWORD SupportChk = *(LPDWORD)pvIn;
			
			switch(SupportChk)
			{
			case DRVESC_GETSCREENROTATION:
			case DRVESC_SETSCREENROTATION:		
			case IOCTL_POWER_CAPABILITIES:
			case IOCTL_POWER_QUERY:
			case IOCTL_POWER_SET:
			case IOCTL_POWER_GET:				
				return 1;
				break;
			default : 
				SetLastError (ERROR_INVALID_PARAMETER);
				return -1;
				break;
			}
		}
		else 
		{
			SetLastError (ERROR_INVALID_PARAMETER);
			return -1;			 
		}
		break;
		
	case DRVESC_GETSCREENROTATION:
		{
			*(int *)pvOut = ((DMDO_0 | DMDO_90 | DMDO_180 | DMDO_270) << 8) | ((BYTE)m_iRotate);
			return DISP_CHANGE_SUCCESSFUL; 
		}
		break;
	case DRVESC_SETSCREENROTATION:
		{
			if ((cjIn == DMDO_0) ||
				(cjIn == DMDO_90) ||
				(cjIn == DMDO_180) ||
				(cjIn == DMDO_270) )
			{
				return DynRotate(cjIn);
			}
			return DISP_CHANGE_BADMODE;
		}
		break;
		
	case IOCTL_POWER_CAPABILITIES:
		// tell the power manager about ourselves
		DEBUGMSG(1, (L"LCDC6xhw: IOCTL_POWER_CAPABILITIES\r\n"));
		if (pvOut != NULL && cjOut == sizeof(POWER_CAPABILITIES))
		{
			PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pvOut;
			memset(ppc, 0, sizeof(*ppc));
			ppc->DeviceDx = (1<<D0) | (1<<D1)| (1<<D2)| (1<<D3)| (1<<D4);	// support D0,D1,D2,D3 and D4
			return 1;			
		}
		break;
		
	case IOCTL_POWER_QUERY:
		if(pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE))
		{
			int RetVal = -1;
			// return a good status on any valid query, since we are always ready to
			// change power states.
			CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE) pvOut;
			if(VALID_DX(NewDx))
			{
				// this is a valid Dx state so return a good status
				RetVal = 1;
			}
			DEBUGMSG(1, (L"LCDC6xhw: IOCTL_POWER_QUERY %u %s\r\n", 
				NewDx, (RetVal == 1) ? L"succeeded" : L"failed"));		
			return RetVal;
		}
		break;
		
	case IOCTL_POWER_SET:
		if(pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE))
		{
			int RetVal;
			CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE) pvOut;				
			if(VALID_DX(NewDx))
			{
				ApplyPowerState(NewDx);	
				m_Dx = NewDx;					
				DEBUGMSG(0, (L"LCDC6xhw: IOCTL_POWER_SET %u\r\n", NewDx));
				RetVal = 1;
			}
			else
			{
				DEBUGMSG(0, (L"LCDC6xhw IOCTL_POWER_SET: invalid state request %u\r\n", NewDx));
				RetVal = -1;
			}


		

			return RetVal;
			
		}
		break;
		
	case IOCTL_POWER_GET:
		if(pvOut != NULL && cjOut == sizeof(CEDEVICE_POWER_STATE))
		{
						
			*(PCEDEVICE_POWER_STATE) pvOut = m_Dx;
			DEBUGMSG(1, (L"LCDC6xhw: IOCTL_POWER_GET: passing back %u\r\n",m_Dx));
			return 1;
			
		}
		break;
		
	default:
		break;
	}
    return 0;
}

void LCDC6xhw::ApplyPowerState(CEDEVICE_POWER_STATE dx)
{
    // KW - This should never be called, make sure we know if it does
    RETAILMSG(1, (TEXT("ApplyPowerState(%d)\n"), dx));
		switch(dx) 
			{
					case D0:  // turn the display on
						if (m_Dx != D0)
						{
							LCDCProcSpecificActivatePMC();
							m_pLCDC->LCDC_CTRSTCON |= AT91C_LCDC_ENA_PWMGEMENABLED ;
							m_pLCDC->LCDC_DMACON &= ~AT91C_LCDC_DMAEN;
							m_pLCDC->LCDC_DMACON |= AT91C_LCDC_DMAEN;
							m_pLCDC->LCDC_PWRCON = AT91C_LCDC_PWR | (0x0F<<1);
						}
						break;
					case D1:		// D1->D4 turn the display off
					case D2:
					case D3:
					case D4:
						if (m_Dx == D0)
						{
							m_pLCDC->LCDC_DMACON &= ~AT91C_LCDC_DMAEN;
							m_pLCDC->LCDC_PWRCON = 0;						
							//Sleep(10);
							while(m_pLCDC->LCDC_PWRCON & AT91C_LCDC_BUSY );
							m_pLCDC->LCDC_CTRSTCON&=~AT91C_LCDC_ENA_PWMGEMENABLED;
							LCDCProcSpecificDeactivatePMC();
						}
						break;
			}
}

int
LCDC6xhw::GetRotateModeFromReg()
{
    HKEY hKey;
	
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\GDI\\ROTATION"),0,0, &hKey))
    {
        DWORD dwSize, dwAngle, dwType = REG_DWORD;
        dwSize = sizeof(DWORD);
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, 
			TEXT("ANGLE"), 
			NULL, 
			&dwType,
			(LPBYTE)&dwAngle,
			&dwSize))
        {
            switch (dwAngle)
            {
            case 0:
                return DMDO_0;
            case 90: 
                return DMDO_90;
            case 180:
                return DMDO_180;
            case 270:
                return DMDO_270;
            default:
                return DMDO_0;
            }
        }
		
        RegCloseKey(hKey);
    }
    
    return DMDO_0;
}

void
LCDC6xhw::SetRotateParams()
{
    int iswap;
    switch(m_iRotate)
    {
    case DMDO_0:
        m_nScreenHeightSave = m_nScreenHeight;
        m_nScreenWidthSave = m_nScreenWidth;
        break;
		
    case DMDO_180:
        m_nScreenHeightSave = m_nScreenHeight;
        m_nScreenWidthSave = m_nScreenWidth;
        break;
		
    case DMDO_90:
    case DMDO_270:
        iswap = m_nScreenHeight;
        m_nScreenHeight = m_nScreenWidth;
        m_nScreenWidth = iswap;
        m_nScreenHeightSave = m_nScreenWidth;
        m_nScreenWidthSave = m_nScreenHeight;
        break;
		
    default:
        m_nScreenHeightSave = m_nScreenHeight;
        m_nScreenWidthSave = m_nScreenWidth;
        break;
    }
	
    return;
}


LONG LCDC6xhw::DynRotate(int angle)
{
	GPESurfRotate *pSurf = (GPESurfRotate *)m_pPrimarySurface;
	if (angle == m_iRotate)
		return DISP_CHANGE_SUCCESSFUL;
	
	m_iRotate = angle;
	
	switch(m_iRotate)
    {
    case DMDO_0:
    case DMDO_180:
		m_nScreenHeight = m_nScreenHeightSave;
		m_nScreenWidth = m_nScreenWidthSave;
		break;
	case DMDO_90:
	case DMDO_270:
		m_nScreenHeight = m_nScreenWidthSave;
		m_nScreenWidth = m_nScreenHeightSave;
		break;
    }
	
	m_pMode->width = m_nScreenWidth;
	m_pMode->height = m_nScreenHeight;
	pSurf->SetRotation(m_nScreenWidth, m_nScreenHeight, angle);
	
	return DISP_CHANGE_SUCCESSFUL;
}

//-----------------------------------------------------------------------------
//! \brief		Check to see if the software cursor should be disabled.
//-----------------------------------------------------------------------------
void LCDC6xhw::CheckDisableCursor(GPEBltParms* pBltParms)
{
	RECTL rectl;
	
    if (pBltParms->pDst)
	{
		if (pBltParms->pDst->InVideoMemory())  // only care if dest is main display surface
		{
			if (m_CursorVisible && !m_CursorDisabled)
			{
				if (pBltParms->prclDst != NULL)        // make sure there is a valid prclDst
				{
					rectl = *pBltParms->prclDst;       // if so, use it
					
					// There is no guarantee of a well ordered rect in pParms
					// due to flipping and mirroring.
					if(rectl.top > rectl.bottom)
					{
						int iSwapTmp     = rectl.top;
						rectl.top    = rectl.bottom;
						rectl.bottom = iSwapTmp;
					}
					
					if(rectl.left > rectl.right)
					{
						int iSwapTmp    = rectl.left;
						rectl.left  = rectl.right;
						rectl.right = iSwapTmp;
					}
				}
				else
				{
					rectl = m_CursorRect;                   // if not, use the Cursor rect - this forces the cursor to be turned off in this case.
				}
				
				if (m_CursorRect.top <= rectl.bottom && m_CursorRect.bottom >= rectl.top &&
					m_CursorRect.left <= rectl.right && m_CursorRect.right >= rectl.left)
				{
					CursorOff();
					m_CursorForcedOff = TRUE;
				}
			}
		}
	}
	
    // check for source overlap with cursor and turn off cursor if overlaps
    if (pBltParms->pSrc)
	{
		if (pBltParms->pSrc->InVideoMemory())  // only care if source is main display surface
		{
			if (m_CursorVisible && !m_CursorDisabled)
			{
				if (pBltParms->prclSrc != NULL)        // make sure there is a valid prclSrc
				{
					rectl = *pBltParms->prclSrc;       // if so, use it
				}
				else
				{
					rectl = m_CursorRect;                   // if not, use the Cursor rect - this forces the cursor to be turned off in this case.
				}
				
				if (m_CursorRect.top < rectl.bottom && m_CursorRect.bottom > rectl.top && 
					m_CursorRect.left < rectl.right && m_CursorRect.right > rectl.left)
				{
					CursorOff();
					m_CursorForcedOff = TRUE;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
//! \brief		Last function called during a bitblt
//-----------------------------------------------------------------------------
SCODE LCDC6xhw::BltComplete(GPEBltParms* pBltParms)
{
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("-> BltComplete")));
	
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("<- BltComplete")));
	
	return S_OK;
}

//-----------------------------------------------------------------------------
//! \brief		Here for debug use only, to generate stats on line drawing
//-----------------------------------------------------------------------------
#ifdef __CALLSTATS__
void LineStats(GPELineParms* pLineParms)
{
	static WORD *aMixCode = 0;
	int offset = 0;
	
	// First call, allocate memory
	if (!aMixCode)
	{
		aMixCode = new WORD[0x20000];
		memset(aMixCode, 0, 0x20000 * sizeof(WORD));
	}
	
	// Create stats array, for Video or System memory
	(pLineParms->pDst->InVideoMemory()?offset=0:offset=0x10000);
	aMixCode[offset+pLineParms->mix]++;
	
	// To display results, place a breakpoint here and jump into lower code:
	int Display = false;
	if (Display)
	{
		int i=0;
		
		for (; i<0x20000; i++)
		{
			if (aMixCode[i]!=0)
			{
				DEBUGMSG(1, (TEXT("pLineParms->mix[%04X] ; VideoMem[%s] ; Called[%u]"), 0x0000FFFF&i, (i<0x10000?L"true":L"false"), aMixCode[i]));
			}
		}
		
		memset(aMixCode, 0, 0x20000 * sizeof(WORD));
	}
}
#endif

//-----------------------------------------------------------------------------
//! \brief		Implementing a line drawing method
//!
//! If we can do it in HW, then we just call HwLine, else SwLine
//-----------------------------------------------------------------------------
SCODE LCDC6xhw::Line(GPELineParms* pLineParms, EGPEPhase phase)
{
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("-> Line")));
	
#ifdef __CALLSTATS__
	LineStats(pLineParms);
#endif
	
    if(phase == gpeSingle || phase == gpePrepare) 
    {
		pLineParms->pLine = GPELINE &LCDC6xhw::DrawLine;
		
        if(pLineParms->pDst)
		{
			if(pLineParms->pDst->InVideoMemory())
			{
				pLineParms->pLine = (SCODE (GPE::*)(struct GPELineParms *))&LCDC6xhw::CursorsWrappedDrawLine;				
			}
        }
		
    }
	
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("<- Line")));
	
    return S_OK;
}

SCODE LCDC6xhw::CursorsWrappedDrawLine(GPELineParms *pParms)
{
    SCODE retval;
    RECT  bounds;
    int   N_plus_1;  // Minor length of bounding rect + 1
	
    // calculate the bounding-rect to determine overlap with cursor
    if (pParms->dN)   // The line has a diagonal component (we'll refresh the bounding rect)
    {
        N_plus_1 = 2 + ((pParms->cPels * pParms->dN) / pParms->dM);
    }
    else
    {
        N_plus_1 = 1;
    }
	
    switch(pParms->iDir)
    {
	case 0:
		bounds.left   = pParms->xStart;
		bounds.top    = pParms->yStart;
		bounds.right  = pParms->xStart + pParms->cPels + 1;
		bounds.bottom = bounds.top + N_plus_1;
		break;
		
	case 1:
		bounds.left   = pParms->xStart;
		bounds.top    = pParms->yStart;
		bounds.bottom = pParms->yStart + pParms->cPels + 1;
		bounds.right  = bounds.left + N_plus_1;
		break;
		
	case 2:
		bounds.right  = pParms->xStart + 1;
		bounds.top    = pParms->yStart;
		bounds.bottom = pParms->yStart + pParms->cPels + 1;
		bounds.left   = bounds.right - N_plus_1;
		break;
		
	case 3:
		bounds.right  = pParms->xStart + 1;
		bounds.top    = pParms->yStart;
		bounds.left   = pParms->xStart - pParms->cPels;
		bounds.bottom = bounds.top + N_plus_1;
		break;
		
	case 4:
		bounds.right  = pParms->xStart + 1;
		bounds.bottom = pParms->yStart + 1;
		bounds.left   = pParms->xStart - pParms->cPels;
		bounds.top    = bounds.bottom - N_plus_1;
		break;
		
	case 5:
		bounds.right  = pParms->xStart + 1;
		bounds.bottom = pParms->yStart + 1;
		bounds.top    = pParms->yStart - pParms->cPels;
		bounds.left   = bounds.right - N_plus_1;
		break;
		
	case 6:
		bounds.left   = pParms->xStart;
		bounds.bottom = pParms->yStart + 1;
		bounds.top    = pParms->yStart - pParms->cPels;
		bounds.right  = bounds.left + N_plus_1;
		break;
		
	case 7:
		bounds.left   = pParms->xStart;
		bounds.bottom = pParms->yStart + 1;
		bounds.right  = pParms->xStart + pParms->cPels + 1;
		bounds.top    = bounds.bottom - N_plus_1;
		break;
		
	default:
		DEBUGMSG(GPE_ZONE_ERROR,(TEXT("Invalid direction: %d\r\n"), pParms->iDir));
		return E_INVALIDARG;
    }
	
    // check for line overlap with cursor and turn off cursor if overlaps
    if (m_CursorVisible && !m_CursorDisabled)
    {
        RotateRectl(&m_CursorRect);
		
        if (m_CursorRect.top < bounds.bottom && m_CursorRect.bottom > bounds.top &&
            m_CursorRect.left < bounds.right && m_CursorRect.right > bounds.left)
        {
            RotateRectlBack(&m_CursorRect);
            CursorOff();
            m_CursorForcedOff = TRUE;
        }
        else
        {
            RotateRectlBack(&m_CursorRect);
        }
    }
	
    // do emulated line
    retval = DrawLine(pParms);
	
    // see if cursor was forced off because of overlap with line bouneds and turn back on.
    if (m_CursorForcedOff)
    {
        m_CursorForcedOff = FALSE;
        CursorOn();
    }
	
    return retval;
}


//-----------------------------------------------------------------------------
//! \brief		Allocate a video-surface used by GPE
//!
//! Here is made a difficult choice, where do we allocate memory ?
//! If the caller require VideoMemory (GPE_REQUIRE_VIDEO_MEMORY) then we allocate
//! in VideoMem, if there isn't enough place we return an error E_OUTOFMEMORY
//! If the caller doen't require anything, and because we get a lot
//! of VideoMem, we can't try to allocate in VideoMem, but if there isn't enough
//! place we allocate in system memory.
//!
//! You should remember that only VideoMem access could be HW accelerated
//-----------------------------------------------------------------------------
SCODE LCDC6xhw::AllocSurface(GPESurf **ppSurf, int width, int height, EGPEFormat format, int surfaceFlags)
{
	DEBUGMSG(GPE_ZONE_ENTER | GPE_ZONE_HW, (TEXT("-> AllocSurface for GPE")));
	
	// If VideoMemory is required & desired pxlFormat is the same of frameBuffer
	if (surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY)
	{
		// Check if required format can be allocated in VidMem
		if (EGPEFormatToBpp[format]==m_VidMemBpp)
		{
			// Try to allocate in VideoMem
			Node2D *pRect = m_pAllVidMem->Alloc(width, height);
			
			if (!pRect)
			{
				DEBUGMSG(GPE_ZONE_ERROR, (TEXT("AllocSurfaceGPE::Error, can't allocate in VideoMem!")));
				return E_OUTOFMEMORY;
			}
			
			// Compute VidMemVirtualAddr
			DWORD RectAddr = (DWORD)m_pVirtVidMemAddr + m_VidMemStrideByte*pRect->Top() + (m_VidMemBpp*pRect->Left())/8;
			
			*ppSurf = new hwSurf(width, height, RectAddr-(DWORD)m_pVirtVidMemAddr, (void*)RectAddr, m_VidMemStrideByte, format, pRect);
			
			// Little msg
			DEBUGMSG(GPE_ZONE_HW, (TEXT("AllocSurfaceGPE::Memory correctly allocate in VideoPool")));
			
			return S_OK;
		}
		
		DEBUGMSG(GPE_ZONE_ERROR, (TEXT("AllocSurfaceGPE::Error, can't allocate this format of Surface in VidMem")));
		return E_OUTOFMEMORY;
		
	} else { // Else we try as much as possible to allocate into VideoMem
		
		// Check if required format can be allocated in VidMem
		if (EGPEFormatToBpp[format]==m_VidMemBpp)
		{
			// Check if required surface isn't too big, Node2D will check that, it's just
			// for optimum responsetime
			if ((DWORD)width<m_VidMemStrideByte/m_VidMemBpp*8)
			{
				// Try to allocate in VideoMem
				Node2D *pRect = m_pAllVidMem->Alloc(width, height);
				
				if (pRect)
				{
					DWORD RectAddr = (DWORD)m_pVirtVidMemAddr + m_VidMemStrideByte*pRect->Top() + (m_VidMemBpp*pRect->Left())/8;
					
					*ppSurf = new hwSurf(width, height, RectAddr-(DWORD)m_pVirtVidMemAddr, (void*)RectAddr, m_VidMemStrideByte, format, pRect);
					
					// Little msg
					DEBUGMSG(GPE_ZONE_HW, (TEXT("AllocSurfaceGPE::Memory correctly allocate in VideoPool")));
					
					return S_OK;
				}
			}
		}
		
		// Can't allocate in VidMem, no problem, just allocate in SysMem
		*ppSurf = new GPESurf(width, height, format);
		
		// Little msg
		DEBUGMSG(GPE_ZONE_HW, (TEXT("AllocSurfaceGPE::Memory correctly allocate in SystemPool")));
		
	}
	
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("<- AllocSurface for GPE")));
	
	return	S_OK;
}

//-----------------------------------------------------------------------------
//! \brief		Allocate a video-surface used by DirectDraw
//!
//! Here is made a difficult choice, where do we allocate memory ?
//! If the caller require VideoMemory (GPE_REQUIRE_VIDEO_MEMORY) then we allocate
//! in VideoMem, if there isn't enough place we return an error E_OUTOFMEMORY
//! If the caller doen't require anything, and because we get a lot
//! of VideoMem, we can't try to allocate in VideoMem, but if there isn't enough
//! place we allocate in system memory.
//!
//! You should remember that only VideoMem access could be HW accelerated
//-----------------------------------------------------------------------------
SCODE LCDC6xhw::AllocSurface(DDGPESurf **ppSurf, int width, int height, EGPEFormat format, EDDGPEPixelFormat pixelFormat, int surfaceFlags)
{
	DEBUGMSG(GPE_ZONE_ENTER | GPE_ZONE_HW, (TEXT("-> AllocSurface for DDGPE")));
	
	// If VideoMemory is required & desired pxlFormat is the same of frameBuffer
	if (surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY)
	{
		// Check if required format can be allocated in VidMem
		if (EGPEFormatToBpp[format]==m_VidMemBpp)
		{
			// Try to allocate in VideoMem
			Node2D *pRect = m_pAllVidMem->Alloc(width, height);
			
			if (!pRect)
			{
				DEBUGMSG(GPE_ZONE_ERROR, (TEXT("AllocSurfaceDDGPE::Error, can't allocate in VideoMem!")));
				return E_OUTOFMEMORY;
			}
			
			// Compute VidMemVirtualAddr
			DWORD RectAddr = (DWORD)m_pVirtVidMemAddr + m_VidMemStrideByte*pRect->Top() + (m_VidMemBpp*pRect->Left())/8;
			
			*ppSurf = new hwSurf(width, height, RectAddr-(DWORD)m_pVirtVidMemAddr, (void*)RectAddr, m_VidMemStrideByte, format, pRect);
			
			// Little msg
			DEBUGMSG(GPE_ZONE_HW, (TEXT("AllocSurfaceDDGPE::Memory correctly allocate in VideoPool")));
			
			return S_OK;
		}
		
		DEBUGMSG(GPE_ZONE_ERROR, (TEXT("AllocSurfaceDDGPE::Error, can't allocate this format of Surface in VidMem")));
		return E_OUTOFMEMORY;
		
	} else { // Else we try as much as possible to allocate into VideoMem
		
		// Check if required format can be allocated in VidMem
		if (EGPEFormatToBpp[format]==m_VidMemBpp)
		{
			// Check if required surface isn't too big, Node2D will check that, it's just
			// for optimum responsetime
			if ((DWORD)width<=m_VidMemStrideByte/m_VidMemBpp*8)
			{
				// Try to allocate in VideoMem
				Node2D *pRect = m_pAllVidMem->Alloc(width, height);
				
				if (pRect)
				{
					DWORD RectAddr = (DWORD)m_pVirtVidMemAddr + m_VidMemStrideByte*pRect->Top() + (m_VidMemBpp*pRect->Left())/8;
					
					*ppSurf = new hwSurf(width, height, RectAddr-(DWORD)m_pVirtVidMemAddr, (void*)RectAddr, m_VidMemStrideByte, format, pRect);
					
					// Little msg
					DEBUGMSG(GPE_ZONE_HW, (TEXT("AllocSurfaceDDGPE::Memory correctly allocate in VideoPool")));
					
					return S_OK;
				}
			}
		}
		
		// Can't allocate in VidMem, no problem, just allocate in SysMem
		*ppSurf = new DDGPESurf(width, height, format);
		
		// Little msg
		DEBUGMSG(GPE_ZONE_HW, (TEXT("AllocSurfaceDDGPE::Memory correctly allocate in SystemPool")));
		
	}
	
	DEBUGMSG(GPE_ZONE_ENTER, (TEXT("<- AllocSurface for DDGPE")));
	
	return	S_OK;
}

//-----------------------------------------------------------------------------
//! \brief		Select a VideoMem surface to be displayed
//!
//! Called by the GDI & DDraw to change the front buffer, the call is redirected
//! to a low level platform specific function who change the frame buffer
//-----------------------------------------------------------------------------
void LCDC6xhw::SetVisibleSurface(GPESurf *pTempSurf)
{
	DWORD displayStart = pTempSurf->OffsetInVideoMemory();
	
	RETAILMSG(1,(TEXT("SetVisibleSurface(0x%x)\r\n"), displayStart + m_pPhyVidMemAddr));
	ChangeFramePointer(displayStart + m_pPhyVidMemAddr);
}

//-----------------------------------------------------------------------------
//! \brief		Select a VideoMem surface to be displayed
//!
//! Same as upper function, but with VSync
//-----------------------------------------------------------------------------
void LCDC6xhw::SetVisibleSurface(GPESurf *pTempSurf, BOOL bWaitForVBlank)
{
	DWORD displayStart = pTempSurf->OffsetInVideoMemory();
	
	RETAILMSG(1,(TEXT("SetVisibleSurface(0x%x, %d)\r\n"), displayStart + m_pPhyVidMemAddr, bWaitForVBlank));
	ChangeFramePointer(displayStart + m_pPhyVidMemAddr);
}

//-----------------------------------------------------------------------------
//! \brief		This method permit to change the pointer shape
//!
//! Change the pointer shape, We need to create a VideoMem surface for the cursor
//! and his backsurface. 
//-----------------------------------------------------------------------------
SCODE LCDC6xhw::SetPointerShape( GPESurf * pMask, GPESurf * pColorSurf, int xHot, int yHot, int cX, int cY )
{
	EnterCriticalSection(&m_CursorCs);
	/*    int        row;
    int        colByte;
    BYTE     * pAND;
    BYTE     * pXOR;
    BYTE       bitMask;
    unsigned   i;
	*/
    CursorOff();
	
    if (!pMask)
	{
        m_CursorDisabled = TRUE;
    }
    else 
    {
		
		if ((m_CursorSize.x   != cX) || (m_CursorSize.y != cY))
		{
			if (pSurfCursorColor)
			{
				delete pSurfCursorColor;
				pSurfCursorColor = NULL;
			}
			
			if (pSurfCursorMask)
			{
				delete pSurfCursorMask;
				pSurfCursorMask = NULL;
			}
			
			if (pSurfCursorXOR)
			{
				delete pSurfCursorXOR;
				pSurfCursorXOR = NULL;
			}
			
			if (pSurfBackingStore)
			{
				delete pSurfBackingStore;
				pSurfBackingStore = NULL;
			}
			
			if (pColorSurf)
			{
				if ( AllocSurface(&pSurfCursorColor,cX,cY,m_pMode->format, GPE_PREFER_VIDEO_MEMORY) != S_OK)
				{
					DEBUGMSG(GPE_ZONE_ERROR, (L"SetPointerShape : Error during AllocSurface of pSurfCursorColor"));
				}
			}
			
			if( AllocSurface(&pSurfCursorMask,cX,cY,m_pMode->format, GPE_PREFER_VIDEO_MEMORY) != S_OK)
			{
				DEBUGMSG(GPE_ZONE_ERROR, (L"SetPointerShape : Error during AllocSurface of pSurfCursorMask"));
			}
			if( AllocSurface(&pSurfCursorXOR,cX,cY,m_pMode->format, GPE_PREFER_VIDEO_MEMORY) != S_OK)
			{
				DEBUGMSG(GPE_ZONE_ERROR, (L"SetPointerShape : Error during AllocSurface of pSurfCursorXOR"));
			}
			if( AllocSurface(&pSurfBackingStore,cX,cY,m_pMode->format, GPE_PREFER_VIDEO_MEMORY) != S_OK)
			{
				DEBUGMSG(GPE_ZONE_ERROR, (L"SetPointerShape : Error during AllocSurface of pSurfBackingStore"));
			}
		}
		
		{
			// Create a LUT to convert from 1Bpp to nBpp
			DWORD LUT[2];
			RECTL DstRect,SrcRect;
			GPEBltParms BltParms;
			LUT[0] = 0x00000000;
			LUT[1] = 0xFFFFFFFF;	
			
			DstRect.top					= 0;
			DstRect.bottom				= cY-1;
			DstRect.left				= 0;
			DstRect.right				= cX-1;
			SrcRect.top					= 0;
			SrcRect.bottom				= cY-1;
			SrcRect.left				= 0;
			SrcRect.right				= cX-1;
			
			
			BltParms.pBlt				= GPEBLT &LCDC6xhw::EmulatedBlt;
			BltParms.pMask				= 0;
			BltParms.pBrush				= 0;
			BltParms.prclDst			= &DstRect;
			BltParms.prclSrc			= &SrcRect;
			BltParms.prclClip			= 0;
			BltParms.solidColor			= 0;
			BltParms.bltFlags			= 0;	
			BltParms.prclMask			= 0;
			BltParms.pptlBrush			= 0;
			BltParms.xPositive			= 1;
			BltParms.yPositive			= 1;
			BltParms.pLookup			= LUT;
			BltParms.pConvert			= 0;
			BltParms.pColorConverter	= 0;
			BltParms.blendFunction		= g_NullBlendFunction;
			
			
			if (pColorSurf)
			{
				BltParms.pDst				= pSurfCursorColor;
				BltParms.pSrc				= pColorSurf;
				BltParms.rop4				= 0xCCCC; //Dest = Src . SRCCOPY;
				EmulatedBlt(&BltParms);
			}
			
			BltParms.pDst				= pSurfCursorMask;
			BltParms.pSrc				= pMask;
			BltParms.rop4				= 0xCCCC; //Dest = Src . SRCCOPY;
			EmulatedBlt(&BltParms);
			
			SrcRect.top					= cY;
			SrcRect.bottom				= 2*cY-1;
			SrcRect.left				= 0;
			SrcRect.right				= cX-1;
			
			BltParms.pDst				= pSurfCursorXOR;
			BltParms.pSrc				= pMask;
			BltParms.rop4				= 0xCCCC; //Dest = Src . SRCCOPY;
			EmulatedBlt(&BltParms);
		}
		
		
		
        m_CursorDisabled = FALSE;
        m_CursorSize.x   = cX;
        m_CursorSize.y   = cY;
        m_xHot            = xHot;
        m_yHot            = yHot;
    }
	
	LeaveCriticalSection(&m_CursorCs);
    return S_OK;
}

//-----------------------------------------------------------------------------
//! \brief		This method is called every time the mouse pointer moves
//!
//! This method just shut down the cursor, move X & Y and refresh the display
//-----------------------------------------------------------------------------
SCODE LCDC6xhw::MovePointer( int xPosition, int yPosition )
{
	EnterCriticalSection(&m_CursorCs);
    CursorOff();
	
    if(xPosition != -1 && yPosition != -1)
    {
        m_CursorRect.left   = (xPosition - m_xHot);
        m_CursorRect.right  = (m_CursorRect.left + m_CursorSize.x -1);
        m_CursorRect.top    = yPosition - m_yHot;
        m_CursorRect.bottom = m_CursorRect.top + m_CursorSize.y -1;	
		
		//m_CursorDisabled =  FALSE;
        CursorOn(); // Enable the cursor
    }
	else
	{
		//m_CursorDisabled =  TRUE;
	}
	
	LeaveCriticalSection(&m_CursorCs);
	
    return S_OK;
}

//-----------------------------------------------------------------------------
//! \brief		Initialization of variables used to display the mouse cursor
//!
//-----------------------------------------------------------------------------
void LCDC6xhw::InitCursor()
{
	InitializeCriticalSection(&m_CursorCs);
	m_CursorSize.x = m_CursorSize.y = 0;
    m_xHot = m_yHot = 0;
}


SCODE
ClipBlt(
		GPE         * pGPE,
		GPEBltParms * pBltParms
		);

//-----------------------------------------------------------------------------
//! \brief		Display the mouse cursor - SW version
//!				Saves screen part under cursor into m_CursorBackingStore,
//!				and display mouse cursor using gCursorMask and gCursorData
//-----------------------------------------------------------------------------
void LCDC6xhw::CursorOn()
{
	static BOOL bInit = FALSE;
	EnterCriticalSection(&m_CursorCs);
	/*
	
	  WORD * ptrScreen = (WORD *) m_pPrimarySurface->Buffer();
	  WORD * ptrLine;
	  WORD * cbsLine;
	*/
    if (!m_CursorForcedOff && !m_CursorDisabled && !m_CursorVisible)
    {
		
		static RECTL SmallRect;
		static RECTL ClipRect;
		static RECTL CursorOnScreenRect;
		static GPEBltParms BltParms;
		
		CursorOnScreenRect.left = MAX(m_CursorRect.left,0);
		CursorOnScreenRect.right = MIN(m_CursorRect.right,m_nScreenWidth-1);
		CursorOnScreenRect.top = MAX(m_CursorRect.top,0);
		CursorOnScreenRect.bottom = MIN(m_CursorRect.bottom,m_nScreenHeight-1);
		
		if (bInit == FALSE)
		{
		ClipRect.top				= 0;
		ClipRect.left				= 0;
		BltParms.prclClip			= 0;
			BltParms.pBlt				= GPEBLT &LCDC6xhw::DrawBlt;
		BltParms.pMask				= 0;
		BltParms.pBrush				= 0;			
		BltParms.solidColor			= 0;
		BltParms.bltFlags			= 0;	
		BltParms.prclMask			= 0;
		BltParms.pptlBrush			= 0;
		BltParms.xPositive			= 1;
		BltParms.yPositive			= 1;
		BltParms.pLookup			= NULL;
		BltParms.pConvert			= 0;
		BltParms.pColorConverter	= 0;
		BltParms.blendFunction		= g_NullBlendFunction;
			BltParms.prclClip			= &ClipRect;
			
			bInit = TRUE;
		}

		ClipRect.bottom				= m_nScreenHeight;		
		ClipRect.right				= m_nScreenWidth;
		
		SmallRect.top				= CursorOnScreenRect.top - m_CursorRect.top;
		SmallRect.bottom			= m_CursorSize.y-1 - (m_CursorRect.bottom - CursorOnScreenRect.bottom);
		SmallRect.left				= CursorOnScreenRect.left - m_CursorRect.left;
		SmallRect.right				= m_CursorSize.x-1 - (m_CursorRect.right - CursorOnScreenRect.right);
		
		
		
		
		
		
		BltParms.prclSrc			= &CursorOnScreenRect;
		BltParms.pSrc				= m_pPrimarySurface;			
		BltParms.prclDst			= &SmallRect;			
		BltParms.pDst				= pSurfBackingStore;			
		BltParms.rop4				= 0xCCCC; //Dest = Src . SRCCOPY;
		
											  /*
											  RETAILMSG(1,(TEXT("CursorOn	(backing store): (%d,%d)/(%d,%d) -> (%d,%d)/(%d,%d)\r\n"),
											  CursorOnScreenRect.left,
											  CursorOnScreenRect.top,
											  CursorOnScreenRect.right,
											  CursorOnScreenRect.bottom,
											  SmallRect.left,
											  SmallRect.top,
											  SmallRect.right,
											  SmallRect.bottom
											  ));
		*/
		ClipBlt(this,&BltParms);
		
		
		/*	RETAILMSG(1,(TEXT("CursorOn	(affichage curseur): (%d,%d)/(%d,%d) -> (%d,%d)/(%d,%d)\r\n"),
		SmallRect.left,
		SmallRect.top,
		SmallRect.right,
		SmallRect.bottom,
		CursorOnScreenRect.left,
		CursorOnScreenRect.top,
		CursorOnScreenRect.right,
		CursorOnScreenRect.bottom
		));
		*/
		
		BltParms.prclSrc			= &SmallRect;
		BltParms.prclDst			= &CursorOnScreenRect;
		BltParms.pDst				= m_pPrimarySurface;			
		BltParms.pSrc				= pSurfCursorMask;			
		BltParms.rop4				= 0x8888; //Dest = Src AND Dest
		ClipBlt(this,&BltParms);
		
		BltParms.pSrc				= pSurfCursorXOR;			
		BltParms.rop4				= 0x6666; //Dest = Src XOR Dest
		ClipBlt(this,&BltParms);
		
		
		if (pSurfCursorColor)
		{
			BltParms.pSrc				= pSurfCursorColor;			
			BltParms.rop4				= 0xEEEE; //Dest = src OR dest
			ClipBlt(this,&BltParms);
		}
		
		/*
		
		  int   iRotate;
		  
			for (int y = m_CursorRect.top; y < m_CursorRect.bottom; y++)
			{
            if (y < 0)
            {
			continue;
            }
            if (y >= m_nScreenHeightSave)
            {
			break;
            }
			
			  ptrLine = &ptrScreen[y * m_pPrimarySurface->Stride() / 2]; // 2 because ptrScreen points to WORD
			  cbsLine = &m_CursorBackingStore[(y - m_CursorRect.top) * m_CursorSize.x];
			  
				for (int x = m_CursorRect.left; x < m_CursorRect.right; x++)
				{
                if (x < 0)
                {
				continue;
                }
                if (x >= m_nScreenWidthSave)
                {
				break;
                }
				
				  iRotate = (y - m_CursorRect.top)*m_CursorSize.x + x - m_CursorRect.left; // no rotation
				  
					cbsLine[x - m_CursorRect.left] = ptrLine[x];  //
					ptrLine[x] &= gCursorMask[iRotate]; // 2*x/2 = x   (2 because 16bpp, 2 because WORD)
					ptrLine[x] ^= gCursorData[iRotate]; //
					}
					}
		*/
        m_CursorVisible = TRUE;
    }
	LeaveCriticalSection(&m_CursorCs);
}

//-----------------------------------------------------------------------------
//! \brief		Hide the mouse cursor
//!				Replace the screen part under cursor by m_CursorBackingStore
//!
//-----------------------------------------------------------------------------
void LCDC6xhw::CursorOff()
{
	static BOOL bInit = FALSE;

	EnterCriticalSection(&m_CursorCs);
	/*
	WORD * ptrScreen = (WORD *) m_pPrimarySurface->Buffer();
	
	  WORD * ptrLine;
	  WORD * cbsLine;
	*/
	
    if (!m_CursorForcedOff && !m_CursorDisabled && m_CursorVisible)
    {
	/*
	   RECTL rSave = m_CursorRect;
	   
		 for (int y = m_CursorRect.top; y < m_CursorRect.bottom; y++)
		 {
		 // clip to displayable screen area (top/bottom)
		 if (y < 0)
		 {
		 continue;
		 }
		 if (y >= m_nScreenHeightSave)
		 {
		 break;
		 }
		 
		   ptrLine = &ptrScreen[y * m_pPrimarySurface->Stride() / 2]; // 2 because ptrScreen points to WORD
		   cbsLine = &m_CursorBackingStore[(y - m_CursorRect.top) * m_CursorSize.x];
		   
			 for (int x = m_CursorRect.left; x < m_CursorRect.right; x++)
			 {
			 // clip to displayable screen area (left/right)
			 if (x < 0)
			 {
			 continue;
			 }
			 if (x >= (int)m_nScreenWidthSave)
			 {
			 break;
			 }
			 
			   ptrLine[x] = cbsLine[x - m_CursorRect.left]; // 2*x/2 = x   (2 because 16bpp, 2 because WORD)
			   }
			   }
			   
				 m_CursorRect = rSave;
		*/
		static RECTL SmallRect;
		static GPEBltParms BltParms;			
		static RECTL ClipRect;
		static RECTL CursorOnScreenRect;
		
		
		CursorOnScreenRect.left = MAX(m_CursorRect.left,0);
		CursorOnScreenRect.right = MIN(m_CursorRect.right,m_nScreenWidth-1);
		CursorOnScreenRect.top = MAX(m_CursorRect.top,0);
		CursorOnScreenRect.bottom = MIN(m_CursorRect.bottom,m_nScreenHeight-1);
		
		if (bInit == FALSE)
		{
		ClipRect.top				= 0;
		ClipRect.left				= 0;
			BltParms.pBlt				= GPEBLT &LCDC6xhw::DrawBlt;
		BltParms.pMask				= 0;
		BltParms.pBrush				= 0;
		BltParms.prclClip			= &ClipRect;
		BltParms.solidColor			= 0;
		BltParms.bltFlags			= 0;	
		BltParms.prclMask			= 0;
		BltParms.pptlBrush			= 0;
		BltParms.xPositive			= 1;
		BltParms.yPositive			= 1;
		BltParms.pLookup			= NULL;
		BltParms.pConvert			= 0;
		BltParms.pColorConverter	= 0;
		BltParms.blendFunction		= g_NullBlendFunction;
		
			bInit = TRUE;
		}
		
		ClipRect.bottom				= m_nScreenHeight;		
		ClipRect.right				= m_nScreenWidth;
		
		SmallRect.top				= CursorOnScreenRect.top - m_CursorRect.top;
		SmallRect.bottom			= m_CursorSize.y-1 - (m_CursorRect.bottom - CursorOnScreenRect.bottom);
		SmallRect.left				= CursorOnScreenRect.left - m_CursorRect.left;
		SmallRect.right				= m_CursorSize.x-1 - (m_CursorRect.right - CursorOnScreenRect.right);
		
		
		
		
		BltParms.prclDst			= &CursorOnScreenRect;
		BltParms.pDst				= m_pPrimarySurface;			
		BltParms.prclSrc			= &SmallRect;			
		BltParms.pSrc				= pSurfBackingStore;			
		BltParms.rop4				= 0xCCCC; //Dest = Src . SRCCOPY;
											  /*
											  RETAILMSG(1,(TEXT("CursorOff	(backing store): (%d,%d)/(%d,%d) -> (%d,%d)/(%d,%d)\r\n"),
											  SmallRect.left,
											  SmallRect.top,
											  SmallRect.right,
											  SmallRect.bottom,
											  CursorOnScreenRect.left,
											  CursorOnScreenRect.top,
											  CursorOnScreenRect.right,
											  CursorOnScreenRect.bottom
											  ));
		*/
		ClipBlt(this,&BltParms);
		
		
        m_CursorVisible = FALSE;
    }
	LeaveCriticalSection(&m_CursorCs);
}

//-----------------------------------------------------------------------------
//! \brief		This method permit to change the color palette used on the display
//-----------------------------------------------------------------------------
SCODE LCDC6xhw::SetPalette(const PALETTEENTRY* src, unsigned short firstEntry, unsigned short numEntries)
{
	//	DEBUGMSG( 1,(TEXT("CAT91SAM9261GraphicChip::SetPalette()\n\r")));
	
	// valid only for 256 color palette ! mustbedone for other paletized modes.....
	int i;
	for(i=firstEntry;i<firstEntry+numEntries;i++)
	{
		WORD w;
		w = 0;
		w |= (src[i].peRed >> 3)   << 0;
		w |= (src[i].peGreen >> 3) << 5;
		w |= (src[i].peBlue >> 3)  << 10;
		m_pLCDC->LCDC_LUT_ENTRY[i] = w;
		
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
//! \brief		\return current VBlank status: InVBlank or NotInVBlank
//!
//! A fake implementation of VBlank
//-----------------------------------------------------------------------------
int LCDC6xhw::InVBlank()
{
	static bool myVBlank = true;
	
	myVBlank = !(myVBlank);
	
	return myVBlank;
}

//-----------------------------------------------------------------------------
//! \brief		Waits for the vertical blank
//!
//-----------------------------------------------------------------------------
void  LCDC6xhw::WaitForVBlank(BOOL bWaitForendOfVBlank)
{
}


//-----------------------------------------------------------------------------
//! \brief		\return the number of bit per pixel of current settings
//-----------------------------------------------------------------------------
int LCDC6xhw::GetBpp()
{
	//	DEBUGMSG( 1,(TEXT("CAT91SAM9261GraphicChip::GetBpp()\n\r")));
	
	return m_pMode->Bpp;
}

//-----------------------------------------------------------------------------
//! \brief		Create a new color palette
//-----------------------------------------------------------------------------
void LCDC6xhw::InitLut(void)
{
	
	SetPalette((const PALETTEENTRY *) rgbPalette,0,256);
}

//-----------------------------------------------------------------------------
/// \brief This function is called every time you want to change frame buffer
///
/// Example : for fullscreen doublebuffered application
///
/// \param PhysicalAddress New Physicall address of the frame buffer
//-----------------------------------------------------------------------------
void LCDC6xhw::ChangeFramePointer(DWORD PhysicalAddress)
{
	// DMA Base address, ie physical address of FrameBuffer
    // KW - Change only if bootstrap has not initialized it yet
    if (m_pLCDC->LCDC_BA1 == 0)
	    m_pLCDC->LCDC_BA1  = (DWORD)(PhysicalAddress);
}


//-----------------------------------------------------------------------------
/// \brief Implements SW Blt operation
//-----------------------------------------------------------------------------
SCODE LCDC6xhw::DrawBlt(GPEBltParms *pBltParms)
{	
	// Call GPE emulated blt engine
	if ((pBltParms->pDst == m_pPrimarySurface && m_iRotate) || (pBltParms->pSrc == m_pPrimarySurface && m_iRotate))
    {
        return EmulatedBltRotate(pBltParms);
    }
	else
	{
		return EmulatedBlt(pBltParms);
	}
}

//-----------------------------------------------------------------------------
/// \brief Implements Blt operation
//-----------------------------------------------------------------------------
SCODE LCDC6xhw::DoBlt(GPEBltParms *pBltParms)
{	
	SCODE S_Result;
	EnterCriticalSection(&m_CursorCs);
	CheckDisableCursor(pBltParms);
	// Call GPE emulated blt engine
	S_Result = DrawBlt(pBltParms);
	
	// Reactivate cursor after the blt.
    if (m_CursorForcedOff)
    {
        m_CursorForcedOff = FALSE;
        CursorOn();
    }
	LeaveCriticalSection(&m_CursorCs);
	return S_Result;
}
//-----------------------------------------------------------------------------
/// \brief Implements HW Line drawing operation
/// actually it's a software one if not overloaded by a child class
//-----------------------------------------------------------------------------
SCODE LCDC6xhw::DrawLine(GPELineParms *pLineParms)
{	
	// Call GPE emulated line engine
	return EmulatedLine(pLineParms);
}



//-----------------------------------------------------------------------------
/// \brief this routine converts a string into a GUID and returns TRUE if the conversion was successful.
//-----------------------------------------------------------------------------
BOOL  LCDC6xhw::ConvertStringToGuid (LPCTSTR pszGuid, GUID *pGuid)
{
	UINT Data4[8];
	int  Count;
	BOOL fOk = FALSE;
	TCHAR *pszGuidFormat = _T("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}");
	
	DEBUGCHK(pGuid != NULL && pszGuid != NULL);
	__try
	{
		if (_stscanf(pszGuid, pszGuidFormat, &pGuid->Data1, 
			&pGuid->Data2, &pGuid->Data3, &Data4[0], &Data4[1], &Data4[2], &Data4[3], 
			&Data4[4], &Data4[5], &Data4[6], &Data4[7]) == 11)
		{
			for(Count = 0; Count < (sizeof(Data4) / sizeof(Data4[0])); Count++)
			{
				pGuid->Data4[Count] = (UCHAR) Data4[Count];
			}
		}
		fOk = TRUE;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
	
	return fOk;
}

//-----------------------------------------------------------------------------
/// \brief This routine notifies the OS that we support the Power Manager IOCTLs (through ExtEscape(), which calls DrvEscape()).
//-----------------------------------------------------------------------------
BOOL LCDC6xhw::AdvertisePowerInterface(HMODULE hInst)
{
	BOOL fOk = FALSE;
	HKEY hk;
	DWORD dwStatus;
	TCHAR szTemp[MAX_PATH];
	GUID gClass;
	
	
	// check for an override in the registry
	dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, LCDC_REGISTRY_KEY, 0, 0, &hk);
	if(dwStatus == ERROR_SUCCESS)
	{
		DWORD dwType, dwSize;
		dwSize = sizeof(szTemp);
		dwStatus = RegQueryValueEx(hk, _T("DisplayPowerClass"), NULL, &dwType, (LPBYTE) szTemp, &dwSize);
		if(dwStatus == ERROR_SUCCESS && dwType == REG_SZ)
		{
			// got a guid string, convert it to a guid
			GUID gTemp;
			fOk = ConvertStringToGuid(szTemp, &gTemp);
			DEBUGCHK(fOk);
			if(fOk)
			{
				gClass = gTemp;
			}
		}
		
		// release the registry key
		RegCloseKey(hk);
	}
	
	// if not override settings in registry assume we are advertising the default class
	if (fOk == FALSE)
		fOk = ConvertStringToGuid(PMCLASS_DISPLAY, &gClass);
	DEBUGCHK(fOk);
	
	
	// figure out what device name to advertise
	if(fOk)
	{
		fOk = GetModuleFileName(hInst, szTemp, sizeof(szTemp) / sizeof(szTemp[0]));
		DEBUGCHK(fOk);
	}
	
	// now advertise the interface
	if(fOk)
	{
		fOk = AdvertiseInterface(&gClass, szTemp, TRUE);
		DEBUGCHK(fOk);
	}
    
	return fOk;
}

//! @}

//! @}


////////////////////////////////////////////////////////////////////////////////
// End of $URL: http://centaure/svn/interne-ce_bsp_atmel/TAGS/TAGS50/SAM9263EK_v100_rc4/PLATFORM/COMMON/SRC/ARM/ATMEL/AT91SAM926x/DRIVERS/LCDC/Core.cpp $
////////////////////////////////////////////////////////////////////////////////
//
//! @}
