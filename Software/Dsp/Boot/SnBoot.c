#include <SnTypes.h>
#include <SnFlash.h>
#include <SnBoot.h>

//----------------------------------------------------------------------
// The pragmas force the code to be in the boot flash
#pragma define_section startup "startup.text"  RX
#pragma section startup begin

//----------------------------------------------------------------------
// Local defines

// The beginning of the data flash image
#define SN_APP_DATA_FLASH_OFFSET	((SnAddr)0xF000)

// This is the word offset in program flash where the version is located
#define SN_APP_VERSION_OFFSET		((SnAddr)0xFFF2)

// This is the location in program flash that contains the application entry point
#define SN_APP_ENTRYPOINT			((SnAddr)0xFFF4)

// This is the location in program flash that contains the program flash crc
#define SN_APP_PROGRAM_FLASH_CRC_OFFSET	((SnAddr)0xFFF5)

// This location contains the number of program flash pages and the data flash crc
#define SN_APP_NUM_PROGRAM_FLASH_PAGES_OFFSET	((SnAddr)0xFFF6)

// Local enumeration to make code more readable
typedef enum {
	SN_FLASH_HALF_FIRST = 0,
	SN_FLASH_HALF_SECOND
} SnFlashHalf;

//----------------------------------------------------------------------
// Error conversion routine from flash errors to boot errors
static SnBootError ConvertFlashErrorToBootError(SnFlashError eFlashError)
{
    switch(eFlashError) {
    case SN_FLASH_ERROR_NONE:
        return SN_BOOT_ERROR_NONE;
    case SN_FLASH_ERROR_ACCESS:
        return SN_BOOT_ERROR_ACCESS;
    case SN_FLASH_ERROR_PROTECT:
        return SN_BOOT_ERROR_PROTECT;
    case SN_FLASH_ERROR_TIMEOUT:
        return SN_BOOT_ERROR_TIMEOUT;
    case SN_BOOT_ERROR_VERIFY:
    	return SN_BOOT_ERROR_VERIFY;
    default:
    	return SN_BOOT_ERROR_UNKNOWN;
    }
}

//----------------------------------------------------------------------
//
// Compute validity of given half of the flash via computation of checksum.
// If a valid checksum is detected, return the computed checksum, otherwise
// return 0.
//
static SnQByte ComputeFlashVersion(const SnByte* pbCrcTable,SnFlashHalf eHalf)
{    
	SnByte bCrc;
    SnAddr aAddr;
	const int iNumPages = GetFlashNumPages(SN_FLASH_MEM_PROGRAM);
    const int iPageSizeInWords = GetFlashPageSize(SN_FLASH_MEM_PROGRAM);
    SnQByte qByteIndex;
    SnWord wVal;
    SnByte bVal;
    SnAddr aApp;
    SnQByte qNumCrcBytes;
    SnByte bNumProgramFlashPages;
	
	aApp =
    	GetFlashBaseAddr(SN_FLASH_MEM_PROGRAM)
    	+
    	(
    		(SnAddr)(((eHalf == SN_FLASH_HALF_FIRST)?0:1) * iNumPages / 2)
    		*
    		(SnAddr)iPageSizeInWords
    	)
    	;
    
    // First verify data flash area
    aAddr = aApp + SN_APP_DATA_FLASH_OFFSET;
    bCrc = 0;    
    qNumCrcBytes = ((SnAddr)0xFF7 * 2);
    for(qByteIndex = 0; qByteIndex < qNumCrcBytes; qByteIndex++) {
    	if(!(qByteIndex & 1)) {
    		wVal = AsmReadProgramSpaceWord(aAddr++);
    		bVal = (SnByte)(wVal & 0xff);
    	} else {
    		bVal = (SnByte)(wVal >> 8);
    	}
        bCrc = pbCrcTable[bCrc ^ bVal];
    }
    if(bCrc) {
        // No valid checksum for data flash
    	return 0;
    }
    
    // Now verify program flash area
    bNumProgramFlashPages = (SnByte)AsmReadProgramSpaceWord(aApp + SN_APP_NUM_PROGRAM_FLASH_PAGES_OFFSET);
    if(bNumProgramFlashPages == 0) {
    	// No program!
    	return 0;
    }
    // Compute program flash crc
    aAddr = aApp;
    bCrc = 0;    
    qNumCrcBytes = (SnQByte)bNumProgramFlashPages * (SnQByte)iPageSizeInWords * 2;
    for(qByteIndex = 0; qByteIndex < qNumCrcBytes; qByteIndex++) {
    	if(!(qByteIndex & 1)) {
    		wVal = AsmReadProgramSpaceWord(aAddr++);
    		bVal = (SnByte)(wVal & 0xff);
    	} else {
    		bVal = (SnByte)(wVal >> 8);
    	}
        bCrc = pbCrcTable[bCrc ^ bVal];
    }
    if(bCrc != (SnByte)AsmReadProgramSpaceWord(aApp + SN_APP_PROGRAM_FLASH_CRC_OFFSET)) {
    	// No valid checksum for program flash
    	return 0;
    }
    
   
	// Detected valid checksums, now extract version (little endian)
	return
	    ((SnQByte)AsmReadProgramSpaceWord(aApp + SN_APP_VERSION_OFFSET))
	    |
	    (((SnQByte)AsmReadProgramSpaceWord(aApp + SN_APP_VERSION_OFFSET + 1)) << 16)
	    ;
}

//----------------------------------------------------------------------
//
// Copy indicated source half of flash onto other half with retries.
// If performing a copy in, then update data flash as well from the
// bottom portion of the half being copied
// Return appropriate error code
//
static SnBootError CopyFlashHalf(SnFlashHalf eSrcHalf)
{
	int iPDstPage;	// First dst page to copy into
	int iPSrcPage;	// First src page to copy from
	int iPXSrcPage;	// First page number in program flash to copy data flash from
	int iPXDstPage;	// First page number in program flash to copy data flash into
    int iPNumPages = GetFlashNumPages(SN_FLASH_MEM_PROGRAM);	// Number of program flash pages
    int iPHalfNumPages = iPNumPages / 2;	// Half the number of program flash pages
    SnAddr aXBuf;	// Temp to get address of data flash page from pwBuf (either &pwBuf[0] or &pwBuf[256])
    int iNumCopyPages;	// Read in from flash image; # program flash pages to copy
    SnAddr aNumCopyPagesAddr;	// Address in program flash that contains the number of program flash pages to copy
    int iPage;		// Generic variable
    int iNumDataFlashPages = GetFlashNumPages(SN_FLASH_MEM_DATA);
    SnBootError eBootError;	// Return value
    SnWord pwBuf[512];	// Buffer big enough to read any flash page, program or data
    
    aNumCopyPagesAddr = GetFlashBaseAddr(SN_FLASH_MEM_PROGRAM) + SN_APP_NUM_PROGRAM_FLASH_PAGES_OFFSET;
                    
    if(eSrcHalf == SN_FLASH_HALF_FIRST) {
    	// Copy out, no update to data flash
        iPSrcPage = 0;
        iPDstPage = iPHalfNumPages;
        iPXSrcPage = iPHalfNumPages - (iNumDataFlashPages / 2);
        iPXDstPage = iPXSrcPage + iPHalfNumPages;
    } else {
    	// Copy in, update data flash
    	iPSrcPage = iPHalfNumPages;
    	iPDstPage = 0;    		
        	// Each source program flash page corresponds to two data flash pages
    	iPXSrcPage = iPNumPages - (iNumDataFlashPages / 2);
        iPXDstPage = iPXSrcPage - iPHalfNumPages;
    	aNumCopyPagesAddr += (SnAddr)iPHalfNumPages * (SnAddr)GetFlashPageSize(SN_FLASH_MEM_PROGRAM);
    }   
    iNumCopyPages = (int)(SnByte)AsmReadProgramSpaceWord(aNumCopyPagesAddr);
    if((iNumCopyPages <= 0) ||  (iNumCopyPages > 120)) {
    	return SN_BOOT_ERROR_NUMPAGES;
    }
	
	// Copy program area into destination half
    for(iPage = 0; iPage < iNumCopyPages; iPage++) {
        int iDstPage = (iPDstPage + iPage);

        // Read source page
        ReadFlashPage(SN_FLASH_MEM_PROGRAM,iPSrcPage + iPage,(SnAddr)&pwBuf[0]);
        
        // Erase destination page
        eBootError = ConvertFlashErrorToBootError(EraseFlashPage(SN_FLASH_MEM_PROGRAM,iDstPage));
        if(eBootError != SN_BOOT_ERROR_NONE) {
        	return eBootError;
        }

        // Copy source data into destination page
        eBootError = ConvertFlashErrorToBootError(WriteFlashPage((SnAddr)&pwBuf[0],SN_FLASH_MEM_PROGRAM,iDstPage));

        if(eBootError != SN_BOOT_ERROR_NONE) {
        	return eBootError;
        }
    }
        
    // Copy data flash area into destination half, and if copying in, update the actual data flash
    aXBuf = (SnAddr)&pwBuf[0];
    for(iPage = 0; iPage < iNumDataFlashPages; iPage++) {
    	if((iPage & 1) == 0) {
	        // Read source page
	        ReadFlashPage(SN_FLASH_MEM_PROGRAM,iPXSrcPage + iPage/2,(SnAddr)&pwBuf[0]);
	        
	        // Erase destination page
	        eBootError = ConvertFlashErrorToBootError(EraseFlashPage(SN_FLASH_MEM_PROGRAM,iPXDstPage + iPage/2));
	        if(eBootError != SN_BOOT_ERROR_NONE) {
	        	return eBootError;
	        }

	        // Copy source data into destination page
	        eBootError = ConvertFlashErrorToBootError(WriteFlashPage((SnAddr)&pwBuf[0],SN_FLASH_MEM_PROGRAM,iPXDstPage + iPage/2));
	        if(eBootError != SN_BOOT_ERROR_NONE) {
	        	return eBootError;
	        }    		
    	}
        
        if(eSrcHalf == SN_FLASH_HALF_SECOND) {
            // Erase data flash page
            eBootError = ConvertFlashErrorToBootError(EraseFlashPage(SN_FLASH_MEM_DATA,iPage));
	        if(eBootError != SN_BOOT_ERROR_NONE) {
	        	return eBootError;
	        }
	        
    		// Write data flash page
        	eBootError = ConvertFlashErrorToBootError(WriteFlashPage(aXBuf,SN_FLASH_MEM_DATA,iPage));
	        if(eBootError != SN_BOOT_ERROR_NONE) {
	        	return eBootError;
	        }
	        
	        if(aXBuf == (SnAddr)&pwBuf[0]) {
	        	aXBuf = (SnAddr)&pwBuf[256];
	        } else {
	        	aXBuf = (SnAddr)&pwBuf[0];
	        }
        }
    }
       
    return SN_BOOT_ERROR_NONE;
}

//----------------------------------------------------------------------
//
// Top level boot code
//
// See associated document for details of operation
//
static SnApp Boot(SnBootError* peBootError)
{
    SnQByte qFirstVersion;
    SnQByte qSecondVersion;
    SnByte pbCrcTable[256];

	InitFlash();
	CopyCrcTable(&pbCrcTable[0]);
		
    qFirstVersion = ComputeFlashVersion(&pbCrcTable[0],SN_FLASH_HALF_FIRST);
    qSecondVersion = ComputeFlashVersion(&pbCrcTable[0],SN_FLASH_HALF_SECOND);
         
    if(qSecondVersion > 0) {
    	if(qFirstVersion == qSecondVersion) {
    		// No copying to do
    		*peBootError = SN_BOOT_ERROR_NONE;
    	} else if(qFirstVersion == 0) {
    		// Anomalous situation, copy in
    		*peBootError = CopyFlashHalf(SN_FLASH_HALF_SECOND);
    		if(*peBootError == SN_BOOT_ERROR_NONE) {
    			// Notify app of anomalous situation
    			*peBootError = SN_BOOT_ERROR_ANOMALY;
    		} else {
    			// Error copying in, silent error
    			return (SnApp)0xFFFFFFFF;
    		}
    	} else {
    		// Copy in    	
    		*peBootError = CopyFlashHalf(SN_FLASH_HALF_SECOND);
    		if(*peBootError != SN_BOOT_ERROR_NONE) {
    			// Error copying in, silent error
    			return (SnApp)0xFFFFFFFF;
    		}
    	}
    } else {
    	if(qFirstVersion > 0) {
    		// Copy out
    		*peBootError = CopyFlashHalf(SN_FLASH_HALF_FIRST);
    	} else {
    	    // Check to see if app has been downloaded without the build upgrade tool
    	    // by looking for magic values in data flash
    	    SnBootInfo *ptBootInfo = (SnBootInfo*)0x2FF2;
    	    
    	    if(
    	    		(ptBootInfo->qVersion == 0xFEEDC0DE)
    	    		&&
    	    		(ptBootInfo->wEntryPoint == 0x6174)
    	    		&&
    	    		(ptBootInfo->bProgramFlashCrc == 0x42)
    	    		&&
    	    		(ptBootInfo->bAlign == 0x61)
    	    		&&
    	    		(ptBootInfo->bNumProgramFlashPages == 0x74)
    	    		&&
    	    		(ptBootInfo->bDataFlashCrc == 0x42)
    	    		) {
    	    	*peBootError = SN_BOOT_ERROR_NONE;
    	    	return (SnApp)0;
    	    } else {
		        // No code available, silent error
		        *peBootError = SN_BOOT_ERROR_NOCODE;
		        return (SnApp)0xFFFFFFFF;    	    	
    	    }
    	}
    }
	
	// Pick up app entry point and return it
	// NOTE: Since an app is limited to 120KB (60KW), a 16-bit word address is sufficient to address it
	return (SnApp)AsmReadProgramSpaceWord(GetFlashBaseAddr(SN_FLASH_MEM_PROGRAM) + SN_APP_ENTRYPOINT);
}

// The actual work is done in the Boot routine, because this frees up most of the stack for the application
int main(void)
{
	SnApp rApp;
	SnBootError eBootError;
	
	rApp = Boot(&eBootError);
        
    // Protect the boot flash and the lower half of program flash before calling app	
	ProtectFlash();
	
	switch(eBootError) {
	    case SN_BOOT_ERROR_NONE:
	    case SN_BOOT_ERROR_ANOMALY:
	        return rApp(eBootError);
	    
	    default:
		    return (int)eBootError;
	}	
}

//----------------------------------------------------------------------

#pragma section startup end

//----------------------------------------------------------------------