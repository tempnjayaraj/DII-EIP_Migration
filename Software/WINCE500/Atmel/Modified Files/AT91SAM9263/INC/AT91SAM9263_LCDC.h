//-----------------------------------------------------------------------------
//! \addtogroup DRIVERS
//! @{
//
//  All rights reserved ADENEO SAS 2005
//!
//-----------------------------------------------------------------------------
//! \file		AT91SAM9263/INC/AT91SAM9263_LCDC.h
//!
//! \brief		Declaration of __LCDC63hw__ class, heritating from __LCDC6xhw abstract class
//!
//! \if subversion
//!   $URL: http://centaure/svn/interne-ce_bsp_atmel/TAGS/TAGS50/SAM9263EK_v100_rc4/PLATFORM/COMMON/SRC/ARM/ATMEL/AT91SAM9263/INC/AT91SAM9263_LCDC.h $
//!   $Author: Woodlandk $
//!   $Revision: 1 $
//!   $Date: 1/29/09 3:26p $
//! \endif
//-----------------------------------------------------------------------------
//! \addtogroup	LCDC
//! @{
//
//! \addtogroup LCDC63
//! @{

#ifndef __LCDC63hw__
#define __LCDC63hw__

#include "LCDC\precomp.h"

#define USE_HW_ACCELERATION
#define USE_HW_BLT_ACCELERATION
// KW - Not used if main application and causes draw errors for TestApp
//#define USE_HW_LINE_ACCELERATION

class LCDC2DGE;

//! \addtogroup Core
//! @{

// Class
class LCDC63hw : public LCDC6xhw
{
public:
	// Constructor / Destructor
	LCDC63hw(DWORD dwVideoMemStartAddress,DWORD dwVideoMemWidth, DWORD dwVideoMemHeight, DWORD dwVideoMemBusWidth);
	virtual	~LCDC63hw();	

	virtual void ApplyPowerState(CEDEVICE_POWER_STATE dx);
	void Config2DEngine(void);
#ifdef USE_HW_ACCELERATION		
	virtual void WaitForNotBusy(BOOL bUseInterrupt=TRUE);
#ifdef USE_HW_BLT_ACCELERATION
	virtual SCODE DrawBlt(GPEBltParms *pBltParms);
#endif //USE_HW_BLT_ACCELERATION

#ifdef USE_HW_LINE_ACCELERATION
	virtual SCODE DrawLine(GPELineParms *pLineParms );
#endif //USE_HW_LINE_ACCELERATION
	
	virtual SCODE SetMode(int modeId, HPALETTE* pPaletteHandle);
#endif //USE_HW_ACCELERATION	

private:
	CRITICAL_SECTION	m_cs;
	DWORD	m_dwVideoMemBusWidth;	
	DWORD	m_dwPixelSize;
	LCDC2DGE *    m_p2DGE;
};

#endif //__LCDC63hw__

// Doxygen End of group Core
//! @}
// Doxygen End of group LCDC63
//! @} 
// Doxygen End of group LCDC
//! @}
// Doxygen End of group Driver
//! @}


////////////////////////////////////////////////////////////////////////////////
// End of $URL: http://centaure/svn/interne-ce_bsp_atmel/TAGS/TAGS50/SAM9263EK_v100_rc4/PLATFORM/COMMON/SRC/ARM/ATMEL/AT91SAM9263/INC/AT91SAM9263_LCDC.h $
////////////////////////////////////////////////////////////////////////////////
//
