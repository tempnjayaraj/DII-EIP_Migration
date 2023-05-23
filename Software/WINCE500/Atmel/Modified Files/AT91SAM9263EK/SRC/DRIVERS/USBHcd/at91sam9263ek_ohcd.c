//-----------------------------------------------------------------------------
//! \addtogroup DRIVERS
//! @{
//
//! \addtogroup	USBHost
//! @{
//!
//  All rights reserved ADENEO SAS 2006
//!
//-----------------------------------------------------------------------------
//! \file		at91sam9263ek_ohcd.c
//!
//! \brief		USB Host driver for AT91SAM9263ek board
//!
//! \if cvs
//!   $RCSfile: I2C.c,v $
//!   $Author: Woodlandk $
//!   $Revision: 1 $
//!   $Date: 1/29/09 3:26p $
//! \endif
//!
//! Description of the driver on multi lines
//-----------------------------------------------------------------------------


// System include
#include <windows.h>
#include <memory.h>
#include <nkintr.h>
#include <CEDDK.h>

//-----------------------------------------------------------------------------
//! \fn			BOOL HWUSBBoardSpecificInit()
//!
//! \brief		This function intialiase USB Host Power PIOs via the SP2526A-2 chip
//!
//! \return		\e TRUE when all is good
//!	\return		\e FALSE when all is bad
//!
//! This function intialiase USB Host Power PIOs via the SP2526A-2 chip
//-----------------------------------------------------------------------------
BOOL HWUSBBoardSpecificInit()
{
    return TRUE;
}


// End of Doxygen group USBHost
//! @}
//! @}
//-----------------------------------------------------------------------------
// End of $RCSfile: I2C.c,v $
//-----------------------------------------------------------------------------
//
