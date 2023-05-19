#ifndef SNBOOT_H
#define SNBOOT_H

typedef enum {
    SN_BOOT_ERROR_NONE = 0,
    SN_BOOT_ERROR_ACCESS,	  // From flash functions
    SN_BOOT_ERROR_PROTECT,	// From flash functions
    SN_BOOT_ERROR_TIMEOUT,	// From flash functions
    SN_BOOT_ERROR_VERIFY,	  // From flash functions
    SN_BOOT_ERROR_NOCODE,   // No code in flash, but no one to report it to
    SN_BOOT_ERROR_ANOMALY,	// No code in lower half, copied in from upper half
    SN_BOOT_ERROR_NUMPAGES,	// Invalid num pages to copy (not in 1..120)
    SN_BOOT_ERROR_UNKNOWN	  // Unknown error
} SnBootError;
 
// Data structure to fill up reserved upper 28 bytes of data flash
// The application locates the structure and fills in qVersion
// The other fields are filled in by the buildupgrade tool
typedef struct {
		// Contains MSB to LSB: bAppType, bMajor, bMinor, bBuild
		// Treated as one number by bootstrapper
	SnQByte qVersion;	
		// 	Address of entry point. Called by bootstrapper using SnApp prototype
	SnWord  wEntryPoint;
		// CRC of program flash in boot image, over bNumProgramFlashPages
	SnByte  bProgramFlashCrc;
		// Alignment byte, set to 0
	SnByte  bAlign;
		// Number of program flash pages occupied by code
	SnByte  bNumProgramFlashPages;
		// CRC over most of data flash, from 0 to bNumProgramFlashPages.
		// Thus, a CRC of data flash from 0 to bDataFlashCRC at runtime should be 0
	SnByte  bDataFlashCrc;
		// Reserved bytes needed for hardware compatibility, must be 0xff in boot image
	SnByte  pbConfigBytes[18];
} SnBootInfo;

typedef int (*SnApp)(SnBootError eError);	// Prototype for application entry point

#endif //SNBOOT_H