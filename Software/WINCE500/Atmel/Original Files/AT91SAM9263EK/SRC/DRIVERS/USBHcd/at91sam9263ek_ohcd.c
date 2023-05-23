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
//!   $Author: Tenneyd $
//!   $Revision: 2 $
//!   $Date: 2/03/09 3:35p $
//! \endif
//!
//! Description of the driver on multi lines
//-----------------------------------------------------------------------------


// System include
#include <windows.h>
#include <memory.h>
#include <nkintr.h>
#include <CEDDK.h>

// Local include
#include "atmel_gpio.h"
#include "AT91SAM9263_gpio.h"

#define ENA		AT91C_PIN_PA(24)
#define ENB		AT91C_PIN_PA(21)
#define FGLA	AT91C_PIN_PA(23)
#define FGLB	AT91C_PIN_PA(20)

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
	BOOL bRet = FALSE;

	const struct pio_desc hw_pio[] = 
    {
			{"ENA",		ENA		, 0, PIO_DEFAULT, PIO_OUTPUT},
			{"ENB",		ENB		, 0, PIO_DEFAULT, PIO_OUTPUT},
			{"FGLA",	FGLA	, 0, PIO_DEFAULT, PIO_INPUT},
			{"FGLB",	FGLB	, 0, PIO_DEFAULT, PIO_INPUT},
		};
	bRet = pio_setup(hw_pio, sizeof(hw_pio)/sizeof(struct pio_desc));
 
	return bRet;
}


// End of Doxygen group USBHost
//! @}
//! @}
//-----------------------------------------------------------------------------
// End of $RCSfile: I2C.c,v $
//-----------------------------------------------------------------------------
//
