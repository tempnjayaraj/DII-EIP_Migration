#ifndef SNFLASH_H
#define SNFLASH_H

//----------------------------------------------------------------------
// Rev 1	KK080604	0xA000 was 0x8000

#include "SnTypes.h"

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

typedef enum {
    SN_FLASH_MEM_BOOT = 0,
    SN_FLASH_MEM_PROGRAM,
    SN_FLASH_MEM_DATA
} SnFlashMem;

typedef enum {
    SN_FLASH_ERROR_NONE = 0,
    SN_FLASH_ERROR_ACCESS,	// Flash sequencing error
    SN_FLASH_ERROR_PROTECT,	// Flash is protected from modification
    SN_FLASH_ERROR_TIMEOUT,	// Timeout waiting for flash modification
    SN_FLASH_ERROR_VERIFY	// Verification error for writes
} SnFlashError;

typedef struct {
	void (*rReadFlashPage)(SnFlashMem eFlashMem,int iSrcPage,SnAddr aDstAddr);
	SnFlashError (*rWriteFlashPage)(SnAddr aSrcAddr,SnFlashMem eFlashMem,int iDstPage);
	SnFlashError (*rEraseFlashPage)(SnFlashMem eFlashMem,int iDstPage);
	const int (*rGetFlashPageSize)(SnFlashMem eFlashMem);
	const SnBool (*rIsFlashInProgramSpace)(SnFlashMem eFlashMem);
	const int (*rGetFlashNumPages)(SnFlashMem eFlashMem);
	const SnAddr (*rGetFlashBaseAddr)(SnFlashMem eFlashMem);
	void (*rCopyCrcTable)(SnByte* pbDst);	
} SnFlashFuncTable;

// This is the number of times a polling loop waits for a bit to go high or low.
#define SN_FLASH_POLL_TIMEOUT_COUNT	0xA000		// KK080604 increased timeout by 25%

// This is the number of times the flash is written, erased or verified before the code gives up
#define SN_FLASH_RETRIES			3

// This is the address in boot flash where the flash function table is located
#define SN_FLASH_FUNC_TABLE_ADDR	0x00020084

// This is the address in boot flash where the CRC table is located
#define SN_FLASH_CRC_TABLE			0x00020004

//----------------------------------------------------------------------
// Function prototypes
void InitFlash(void);
void ProtectFlash(void);
void ReadFlashPage(SnFlashMem eFlashMem,int iSrcPage,SnAddr aDstAddr);
SnFlashError WriteFlashPage(SnAddr aSrcAddr,SnFlashMem eFlashMem,int iDstPage);
SnFlashError EraseFlashPage(SnFlashMem eFlashMem,int iDstPage);
SnWord AsmReadProgramSpaceWord(SnAddr aSrcAddr);
void AsmWriteProgramSpaceWord(SnAddr aDstAddr,SnWord wSrcData);
const int GetFlashPageSize(SnFlashMem eFlashMem);
const SnBool IsFlashInProgramSpace(SnFlashMem eFlashMem);
const int GetFlashNumPages(SnFlashMem eFlashMem);
const SnAddr GetFlashBaseAddr(SnFlashMem eFlashMem);
void CopyCrcTable(SnByte* pbDst);
void CopyFlashFuncTable(SnFlashFuncTable* ptFlashFuncTable);

//----------------------------------------------------------------------
// Assembly code to access data in program space
// This is because regular C code can only access data space

// Read one word of program flash fron indicated address
static SnWord asm AsmReadProgramSpaceWord(SnAddr aSrcAddr)
{
    move.l A10,R2;                       /* Move given address to pointer register */
    move.w p:(r2)+,y0;                   /* Read data from program memory */
    rts;
}

// Write one word of program flash to indicated address
// Note that this needs to be wrapped into a write-page function to be usable
static void asm AsmWriteProgramSpaceWord(SnAddr aDstAddr,SnWord wSrcData)
{
    move.l A10,R2;                       /* Move given address to pointer register */
    move.w Y0,p:(R2)+;                   /* Write given data to desired address in program memory */
    rts;
}

//----------------------------------------------------------------------
// Create a local copy in data space of the flash function table
static void CopyFlashFuncTable(SnFlashFuncTable* ptFlashFuncTable)
{
    int iNumWords = sizeof(SnFlashFuncTable) / 2;
    SnAddr aSrc = SN_FLASH_FUNC_TABLE_ADDR;
    SnWord* pwDst = (SnWord*)ptFlashFuncTable;
    
    while(iNumWords-- > 0) {
    	*pwDst++ = AsmReadProgramSpaceWord(aSrc++);
    }
}

//----------------------------------------------------------------------
#endif //SNFLASH_H