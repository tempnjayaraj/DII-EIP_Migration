/*
 *   Errata Number 5.1
 *   Description:
 *       Command conflict when setting CCIF in the Flash Module
 *   Impact:
 *       If the CCIF is set at the exact same time the CBEIF bit is cleared,
 *       only the set will occur. This gives the impression that a command is
 *       completed when in fact it is active. This can only occur on the very
 *       last cycle of a pipeline operation.
 *   Workaround:
 *       Use driver software to avoid issue.
 *
 *   Errata Number 6.1
 *   Description:
 *       Flash program / erase operations can cause other peripheral register
 *       access to be dup(l)icated.
 *   Impact:
 *       This condition can cause issues with the transmit / receive registers
 *       and quadrature decoder hold registers.
 *   Workaround:
 *       Avoid peripheral I/O to any peripheral except the Flash Module for 2
 *       CPU cycles prior to writing to a flash memory over its system bus
 *       interface.
 *
 *   Errata Number 14.3
 *   Description:
 *       The CodeWarrior debugger is not sensitive enough to the operation
 *       frequency of the device. Memory code may be corrupted when setting
 *       / clearing.
 *   Impact:
 *       Once the PLL is engaged, the device may be under erased / programmed
 *       when setting and clearing breakpoints.
 *   Workaround:
 *       flash.cfg file should contain the following entry:
 *           set_hfmclkd 0x14
 *       This sets the on-chip flash interface unit to use the maximum program
 *       time at 4MHz system rate. At 60Mhz, program/erase times will be less
 *       than desired, but appear operational under otherwise normal conditions.
 *       Use of hardware breakpoints also eliminates this issue.
 */
#include <SnTypes.h>
#include <SnFlash.h>

//----------------------------------------------------------------------
// The pragmas force the code to be in the boot flash
#pragma define_section startup "startup.text"  RX
#pragma section startup begin

//----------------------------------------------------------------------
// Local typedef
typedef enum {
	SN_FLASH_CMD_WRITE = 0,
	SN_FLASH_CMD_PAGE_ERASE
} SnFlashCmd;

//----------------------------------------------------------------------
// This at a fixed address, enforced in the linker command file
// The address of this table should be dialed into SN_FLASH_TABLE_ADDR
const SnFlashFuncTable g_tFlashFuncTable = 
{
	ReadFlashPage,
	WriteFlashPage,
	EraseFlashPage,
	GetFlashPageSize,
	IsFlashInProgramSpace,
	GetFlashNumPages,
	GetFlashBaseAddr,
	CopyCrcTable
};

//----------------------------------------------------------------------
// Register and bitfield defines for flash module

#define DefineBit(i)               ((SnWord)(1 << (i)))
#define DefineBFld(iMs,iLs) \
    ((SnWord)(((SnQByte)1 << ((iMs) + 1)) - ((SnQByte)1 << (iLs))))

#define FM_BASE ((volatile SnWord*)0x00F400)

#define REG_FMCLKD         (FM_BASE + 0x0)
#define BFLD_DIVLD         DefineBit(7)
#define BFLD_PRDIV8        DefineBit(6)
#define BFLD_DIV           DefineBFld(5,0)

// The following register is also referred to as FMMCR
#define REG_FMCR           (FM_BASE + 0x1)
#define BFLD_LOCK          DefineBit(10)
#define BFLD_AEIE          DefineBit(8)
#define BFLD_CBEIE         DefineBit(7)
#define BFLD_CCIE          DefineBit(6)
#define BFLD_KEYACC        DefineBit(5)
#define BFLD_BKSEL         DefineBFld(1,0)
#define BFLD_BKSEL_PROGRAM_OR_BOOT	(0x0 << 0)
#define BFLD_BKSEL_DATA				(0x1 << 0)

#define REG_FMSECH         (FM_BASE + 0x3)
#define BFLD_KEYEN         DefineBit(15)
#define BFLD_SECSTAT       DefineBit(14)

#define REG_FMSECL         (FM_BASE + 0x4)
#define BFLD_SEC           DefineBFld(15,0)

#define REG_FMMNTR         (FM_BASE + 0x5)

#define REG_FMPROT         (FM_BASE + 0x10)     //banked
#define BFLD_PROTECT       DefineBFld(15,0)

#define REG_FMPROTB        (FM_BASE + 0x11)     //banked
// manual uses duplicate name PROTECT for this bitfield
#define BFLD_PROTECTB      DefineBFld(3,0)

#define REG_FMUSTAT        (FM_BASE + 0x13)     //banked
    //NOTE: Clear CBEIF bit by writing ONE to it! Writing 0 aborts a sequence
#define BFLD_CBEIF         DefineBit(7)
#define BFLD_CCIF          DefineBit(6)
#define BFLD_PVIOL         DefineBit(5)
#define BFLD_ACCERR        DefineBit(4)
#define BFLD_BLANK         DefineBit(2)

#define REG_FMCMD          (FM_BASE + 0x14)		//banked
#define BFLD_CMD           DefineBFld(6,0)
#define BFLD_CMD_RDARY1    (0x05 << 0)			// Erase Verify (All ones)
#define BFLD_CMD_PGM       (0x20 << 0)			// Word Program
#define BFLD_CMD_PGERS     (0x40 << 0)			// Page Erase
#define BFLD_CMD_MASERS    (0x41 << 0)			// Mass Erase

#define REG_FMCTL          (FM_BASE + 0x15)     //banked

// The following registers are also referred to as FMIFROPTn
#define REG_FMOPT0         (FM_BASE + 0x1A)       
#define REG_FMOPT1         (FM_BASE + 0x1B)
#define REG_FMOPT2         (FM_BASE + 0x1C)

//----------------------------------------------------------------------
// Utility functions to access registers

// Write a register with a value
static void SetReg(volatile SnWord* pwReg,SnWord wVal)
{
    *pwReg = wVal;
}

// Read the value of a register
static volatile SnWord InqReg(volatile const SnWord* pwReg)
{
    return *pwReg;
}

// Set a bitfield in a register with the indicated value
static void SetBFld(volatile SnWord* pwReg,SnWord wBFld,SnWord wVal)
{
    SnWord wTmp;

    wTmp = *pwReg;
    wTmp &= ~wBFld;
    wTmp |= wVal;
    *pwReg = wTmp;
}

// Set a bitfield in a register with all ones
static void SetBFldOnes(volatile SnWord* pwReg,SnWord wBFld)
{
    SnWord wTmp;

    wTmp = *pwReg;
    wTmp |= wBFld;
    *pwReg = wTmp;
}

// Set a bitfield in a register with all zeroes (i.e. clear bitfield)
static void SetBFldZeroes(volatile SnWord* pwReg,SnWord wBFld)
{
    SnWord wTmp;

    wTmp = *pwReg;
    wTmp &= ~wBFld;
    *pwReg = wTmp;
}

//----------------------------------------------------------------------
// Flash module functions

// Initialize flash. Must be called exactly once before accessing flash memory
void InitFlash(void)
{  
	SnWord wFMCR;
	
    // Write the clock divider (FMCLKD) register
    if(0 == (InqReg(REG_FMCLKD) & BFLD_DIVLD)) {
        // The input_clock is system_clock / 2 = 60MHz / 2 = 30MHz
        // For input clock frequencies > 12.8MHz, PRDIV8 must be set in FMCLKD
        //      to pre-divide the input clock by 8 (i.e. ICLK = input_clock / 8)
        //      Thus, ICLK = 30MHz / 8 = 3.75 MHz (which is not divisible by 200kHz)
        // If ICLK is divisible by 200kHz, then DIV = (ICLK / 200kHz) - 1, else
        // DIV = INT(ICLK / 200kHz)
        //      = INT(3.75MHz / 200 kHz) = INT(18.75) = 18 = 0x12
        SetReg(REG_FMCLKD,BFLD_PRDIV8|0x12);
        //SetReg(REG_FMCLKD,0x14);
    }
    
    // Protect boot flash from being overwritten
    // Make sure FMCR::LOCK is 0
    wFMCR = InqReg(REG_FMCR);
    if(wFMCR & BFLD_LOCK) {
    	// Can only happen if InitFlash() is called multiple times, in this case, just return
    	return;
    }
    
    	// Set FMCR::BKSEL = (program or boot)
    wFMCR &= ~BFLD_BKSEL;
//    wFMCR |= BFLD_BKSEL_PROGRAM_OR_BOOT;	// Compiler complains about lack of side effect
    SetReg(REG_FMCR,wFMCR);
    
    	// Unprotect program flash
    	// Clear all bits of FMPROT:PROTECT
    SetReg(REG_FMPROT,0x0000);
    
    	// Protect boot flash from being overwritten
    	// Set FMPROTB:PROTECTB to all ones (0xf)
    SetReg(REG_FMPROTB,0xf);
    
    	// Set FMCR::BKSEL = data
    wFMCR &= ~BFLD_BKSEL;
    wFMCR |= BFLD_BKSEL_DATA;
    SetReg(REG_FMCR,wFMCR);
    	// Clear all bits of FMPROT:PROTECT
    SetReg(REG_FMPROT,0x0000);        
}

// This should be called after boot code is done setting up, but before calling application
void ProtectFlash(void)
{
	SnWord wFMCR;
	
    // Protect boot flash from being overwritten
    // Make sure FMCR::LOCK is 0
    wFMCR = InqReg(REG_FMCR);
    if(wFMCR & BFLD_LOCK) {
    	// Can only happen if ProtectFlash() is called multiple times, in this case, just return
    	return;
    }
    
    	// Set FMCR::BKSEL = 0 (program or boot)
    wFMCR &= ~BFLD_BKSEL;
//    wFMCR |= BFLD_BKSEL_PROGRAM_OR_BOOT;	// Compiler complains about lack of side effect
    SetReg(REG_FMCR,wFMCR);
    
    // Protect lower half of program flash (where running application resides)
    	// Set lower 8 bits of FMPROT:PROTECT
    SetReg(REG_FMPROT,0x00ff);
    
    // Protect boot flash from being overwritten
    	// Set FMPROTB:PROTECTB to all ones (0xf)
    SetReg(REG_FMPROTB,0xf);
    
    // Leave data flash unprotected
    
    // Lock flash protections
    	// Set FMCR::LOCK = 1
    wFMCR |= BFLD_LOCK;
    SetReg(REG_FMCR,wFMCR);	
}

const int GetFlashPageSize(SnFlashMem eFlashMem)
{
    return (eFlashMem == SN_FLASH_MEM_PROGRAM) ? 512 : 256;
}

const SnBool IsFlashInProgramSpace(SnFlashMem eFlashMem)
{
    return (eFlashMem == SN_FLASH_MEM_DATA) ? FALSE : TRUE;
}

const int GetFlashNumPages(SnFlashMem eFlashMem)
{
    switch(eFlashMem) {
    default:
        //FallThrough
        ;
    case SN_FLASH_MEM_BOOT:
        return 32;
    case SN_FLASH_MEM_PROGRAM:
        return 256;
    case SN_FLASH_MEM_DATA:
        return 16;
    }
}

const SnAddr GetFlashBaseAddr(SnFlashMem eFlashMem)
{
    switch(eFlashMem) {
    default:
        //FallThrough
        ;
    case SN_FLASH_MEM_BOOT:
        return 0x020000;
    case SN_FLASH_MEM_PROGRAM:
        return 0x000000;
    case SN_FLASH_MEM_DATA:
        return 0x002000;
    }
}

// Read a page of flash memory
void ReadFlashPage(SnFlashMem eFlashMem,int iSrcPage,SnAddr aDstAddr)
{
    SnBool yPgmSpace;
    int iPageSize = GetFlashPageSize(eFlashMem);
    SnWord *pwDst = (SnWord*)aDstAddr;

    yPgmSpace = IsFlashInProgramSpace(eFlashMem);

    if(yPgmSpace) {
        SnAddr aSrcAddr = GetFlashBaseAddr(eFlashMem) + (SnAddr)((SnAddr)iSrcPage * (SnAddr)iPageSize);
    
        while(iPageSize-- > 0) {
            *pwDst++ = AsmReadProgramSpaceWord(aSrcAddr++);
        }
    } else {
    	SnWord* pwSrc = (SnWord*)(GetFlashBaseAddr(eFlashMem) + ((SnAddr)iSrcPage * (SnAddr)iPageSize));
    	
        while(iPageSize-- > 0) {
            *pwDst++ = *pwSrc++;
        }
    }
}

// Modify flash memory, accessed only through layered functions that follow.
static SnFlashError DoFlashCmd(SnFlashCmd eFlashCmd, SnAddr aSrcAddr, SnFlashMem eFlashMem, int iDstPage)
{
    SnBool yPgmSpace;
    SnWord wTmp;
    SnBool yDoubleWrite;
    SnWord wCmd;
    SnWord wBkSel;
    SnQByte qTimeout;
    int iPageSize = GetFlashPageSize(eFlashMem);
    const SnWord* pwSrc = (SnWord*)aSrcAddr;
    SnAddr aDst = GetFlashBaseAddr(eFlashMem) + ((SnAddr)iDstPage * (SnAddr)iPageSize);
    SnWord* pwDst = (SnWord*)aDst;
    SnQByte qTimeoutInit;

    yPgmSpace = IsFlashInProgramSpace(eFlashMem);
    switch(eFlashMem) {
    default:
        //FallThrough
        ;
    case SN_FLASH_MEM_BOOT:
    	return SN_FLASH_ERROR_PROTECT;	// Disallow writing boot flash
        //yDoubleWrite = FALSE;
        //wBkSel = 0x0;
        break;
    case SN_FLASH_MEM_PROGRAM:
        yDoubleWrite = TRUE;
        iPageSize /= 2;
        wBkSel = 0x0;
        break;
    case SN_FLASH_MEM_DATA:
        yDoubleWrite = FALSE;
        wBkSel = 0x1;
        break;
    }
    qTimeoutInit = SN_FLASH_POLL_TIMEOUT_COUNT;
    switch(eFlashCmd) {
    default:
    	//FallThrough
    	;
    case SN_FLASH_CMD_WRITE:
    	wCmd = BFLD_CMD_PGM;
    	break;
	case SN_FLASH_CMD_PAGE_ERASE:
        pwSrc = &wTmp;
        iPageSize = 1;
        wCmd = BFLD_CMD_PGERS;
    	break;
    }
        // Errata 6.1 above: Insert two NOPs
    asm 
    {
    	NOP;
    	NOP;
    }

        // For each word in page
    while(iPageSize-- > 0) {
            // Address, Data, Command Buffer Empty Check
            // Wait for bit FMUSTAT:CBEIF to be set
        qTimeout = qTimeoutInit;
        while((qTimeout > 0) && ((InqReg(REG_FMUSTAT) & BFLD_CBEIF) == 0)) {
            qTimeout--;
        }
        if(qTimeout == 0) {
            return SN_FLASH_ERROR_TIMEOUT;
        }

            // Write FMCR:BKSEL
        SetBFld(REG_FMCR,BFLD_BKSEL,wBkSel);

            // Write array address and data
            // If program flash, write array odd address and data
        if(yPgmSpace) {
            AsmWriteProgramSpaceWord(aDst++,*pwSrc++);
            if(yDoubleWrite) {
                AsmWriteProgramSpaceWord(aDst++,*pwSrc++);
            }
        } else {
            *pwDst++ = *pwSrc++;
        }

            // Write FMCMD:CMD
        SetBFld(REG_FMCMD,BFLD_CMD,wCmd);

            // Clear bit FMUSTAT:CBEIF (by writing a 1 to it)
        SetBFldOnes(REG_FMUSTAT,BFLD_CBEIF);

            // If FMUSTAT:PVIOL is set, clear FMUSTAT:PVIOL, issue protection violation error and exit
        wTmp = InqReg(REG_FMUSTAT);
        if(wTmp & BFLD_PVIOL) {
            wTmp &= ~BFLD_PVIOL;
            SetReg(REG_FMUSTAT,wTmp);
            return SN_FLASH_ERROR_PROTECT;
        }

            // If FMUSTAT:ACCERR is set, clear FMUSTAT:ACCERR, issue access error and exit
        if(wTmp & BFLD_ACCERR) {
            wTmp &= ~BFLD_ACCERR;
            SetReg(REG_FMUSTAT,wTmp);
            return SN_FLASH_ERROR_ACCESS;
        }
        
            // Loop if next write
    }

    // Bit Polling for Command Completion Check
    // Wait for bit FMUSTAT:CCIF to be set
    qTimeout = qTimeoutInit;
    while((qTimeout > 0) && ((InqReg(REG_FMUSTAT) & BFLD_CCIF) == 0)) {
        qTimeout--;
    }
    if(qTimeout == 0) {
        return SN_FLASH_ERROR_TIMEOUT;
    }

    return SN_FLASH_ERROR_NONE;
}

//----------------------------------------------------------------------
//
// Wrapper for flash functions with retries
//
SnFlashError EraseFlashPage(SnFlashMem eFlashMem, int iDstPage)
{
	int iRetry;	
	SnFlashError eFlashError;
	
	for(iRetry = SN_FLASH_RETRIES; iRetry-- > 0; ) {
		eFlashError = DoFlashCmd(SN_FLASH_CMD_PAGE_ERASE,0,eFlashMem,iDstPage);
		if(eFlashError == SN_FLASH_ERROR_NONE) {
			break;
		}
	}
	return eFlashError;
}

static SnBool VerifyPageData(SnWord wCount,SnWord* pwSrc,SnWord* pwDst)
{
    while(wCount-- > 0) {
        if(*pwSrc++ != *pwDst++) {
            return FALSE;
        }
    }
    return TRUE;
}

SnFlashError WriteFlashPage(SnAddr aSrcAddr,SnFlashMem eFlashMem,int iDstPage)
{
	int iRetry;	
	SnFlashError eFlashError;
	SnWord pwBuf[512];
	
	for(iRetry = SN_FLASH_RETRIES; iRetry-- > 0; ) {
		eFlashError = DoFlashCmd(SN_FLASH_CMD_WRITE,aSrcAddr,eFlashMem,iDstPage);
		if(eFlashError == SN_FLASH_ERROR_NONE) {
		    ReadFlashPage(eFlashMem,iDstPage,(SnAddr)&pwBuf[0]);
		    if(VerifyPageData(GetFlashPageSize(eFlashMem),(SnWord*)aSrcAddr,&pwBuf[0]) == FALSE) {
		        eFlashError = SN_FLASH_ERROR_VERIFY;
		        if(iRetry == 0) {
		            break;
		        }
		    } else {
		        break;
		    }
		}
		
		// If a write failed, one should probably erase the page first before trying again
		// If the erase failed too, then give up
		eFlashError = EraseFlashPage(eFlashMem,iDstPage);
		if(eFlashError != SN_FLASH_ERROR_NONE) {
			break;
		}
	}
	return eFlashError;
}

//----------------------------------------------------------------------
// CRC table in boot rom, always at fixed address SN_FLASH_CRC_TABLE
const SnByte g_pbCrcTable[] = 
{
/*0*/   0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
/*1*/ 157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
/*2*/  35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
/*3*/ 190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
/*4*/  70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
/*5*/ 219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
/*6*/ 101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
/*7*/ 248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
/*8*/ 140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
/*9*/  17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
/*a*/ 175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
/*b*/  50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
/*c*/ 202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
/*d*/  87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
/*e*/ 233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
/*f*/ 116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
//     0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f
};

// Copy CRC table from boot flash area into working copy for speed
void CopyCrcTable(SnByte* pbDst)
{
    SnAddr aSrc;
    int i;
    SnWord* pwDst = (SnWord*)pbDst;
	
	for(aSrc = SN_FLASH_CRC_TABLE, i = 128; i-- > 0; ) {
		*pwDst++ = AsmReadProgramSpaceWord(aSrc++);
	}
}

//----------------------------------------------------------------------

#pragma section startup end

//----------------------------------------------------------------------