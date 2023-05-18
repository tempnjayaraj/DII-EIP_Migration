#ifndef NVRAMFLASH_H
#define NVRAMFLASH_H

#include "SnTypes.h"


typedef struct 
{
	SnWord usBasicForwardRpm;
	SnWord usBasicReverseRpm;
	SnWord usBasicForward2Rpm;
	SnWord usBasicReverse2Rpm;
	SnWord usBasicOscillateRpm;
	SnWord usBasicOscillateSec;

	SnWord usMiniForwardRpm;
	SnWord usMiniReverseRpm;
	SnWord usMiniForward2Rpm;
	SnWord usMiniReverse2Rpm;
	SnWord usMiniOscillateRpm;
	SnWord usMiniOscillateSec;

	SnWord usHighSpeedCurvedForwardRpm;
	SnWord usHighSpeedCurvedReverseRpm;
	SnWord usHighSpeedCurvedForward2Rpm;
	SnWord usHighSpeedCurvedReverse2Rpm;
	SnWord usHighSpeedCurvedOscillateRpm;
	SnWord usHighSpeedCurvedOscillateSec;

	SnWord usHighSpeedStraightForwardRpm;
	SnWord usHighSpeedStraightReverseRpm;
	SnWord usHighSpeedStraightForward2Rpm;
	SnWord usHighSpeedStraightReverse2Rpm;
	SnWord usHighSpeedStraightOscillateRpm;
	SnWord usHighSpeedStraightOscillateSec;

	SnWord usHighSpeedBurForwardRpm;
	SnWord usHighSpeedBurReverseRpm;
	SnWord usHighSpeedBurForward2Rpm;
	SnWord usHighSpeedBurReverse2Rpm;
	SnWord usHighSpeedBurOscillateRpm;
	SnWord usHighSpeedBurOscillateSec;

	SnWord usHighSpeedFastBurForwardRpm;
	SnWord usHighSpeedFastBurReverseRpm;
	SnWord usHighSpeedFastBurForward2Rpm;
	SnWord usHighSpeedFastBurReverse2Rpm;
	SnWord usHighSpeedFastBurOscillateRpm;
	SnWord usHighSpeedFastBurOscillateSec;
	
	SnWord usDpDrillSpeed;
	SnWord usDpSawSpeed;

	SnWord usSmallJointCurvedForwardRpm;
	SnWord usSmallJointCurvedReverseRpm;
	SnWord usSmallJointCurvedForward2Rpm;
	SnWord usSmallJointCurvedReverse2Rpm;
	SnWord usSmallJointCurvedOscillateRpm;
	SnWord usSmallJointCurvedOscillateSec;

	SnWord usSmallJointStraightForwardRpm;
	SnWord usSmallJointStraightReverseRpm;
	SnWord usSmallJointStraightForward2Rpm;
	SnWord usSmallJointStraightReverse2Rpm;
	SnWord usSmallJointStraightOscillateRpm;
	SnWord usSmallJointStraightOscillateSec;

	SnWord usSmallJointBurForwardRpm;
	SnWord usSmallJointBurReverseRpm;
	SnWord usSmallJointBurForward2Rpm;
	SnWord usSmallJointBurReverse2Rpm;
	SnWord usSmallJointBurOscillateRpm;
	SnWord usSmallJointBurOscillateSec;

} DEVICE_DATA; 


typedef struct 
{
	SnByte ucBasicForwardRpm;
	SnByte ucBasicReverseRpm;
	SnByte ucBasicForward2Rpm;
	SnByte ucBasicReverse2Rpm;
	SnByte ucBasicOscillateRpm;
	SnByte ucBasicOscillateSec;

	SnByte ucMiniForwardRpm;
	SnByte ucMiniReverseRpm;
	SnByte ucMiniForward2Rpm;
	SnByte ucMiniReverse2Rpm;
	SnByte ucMiniOscillateRpm;
	SnByte ucMiniOscillateSec;

	SnByte ucHighSpeedCurvedForwardRpm;
	SnByte ucHighSpeedCurvedReverseRpm;
	SnByte ucHighSpeedCurvedForward2Rpm;
	SnByte ucHighSpeedCurvedReverse2Rpm;
	SnByte ucHighSpeedCurvedOscillateRpm;
	SnByte ucHighSpeedCurvedOscillateSec;

	SnByte ucHighSpeedStraightForwardRpm;
	SnByte ucHighSpeedStraightReverseRpm;
	SnByte ucHighSpeedStraightForward2Rpm;
	SnByte ucHighSpeedStraightReverse2Rpm;
	SnByte ucHighSpeedStraightOscillateRpm;
	SnByte ucHighSpeedStraightOscillateSec;

	SnByte ucHighSpeedBurForwardRpm;
	SnByte ucHighSpeedBurReverseRpm;
	SnByte ucHighSpeedBurForward2Rpm;
	SnByte ucHighSpeedBurReverse2Rpm;
	SnByte ucHighSpeedBurOscillateRpm;
	SnByte ucHighSpeedBurOscillateSec;

	SnByte ucHighSpeedFastBurForwardRpm;
	SnByte ucHighSpeedFastBurReverseRpm;
	SnByte ucHighSpeedFastBurForward2Rpm;
	SnByte ucHighSpeedFastBurReverse2Rpm;
	SnByte ucHighSpeedFastBurOscillateRpm;
	SnByte ucHighSpeedFastBurOscillateSec;

	SnByte ucDpDrillSpeed;
	SnByte ucDpSawSpeed;

	SnByte ucSmallJointCurvedForwardRpm;
	SnByte ucSmallJointCurvedReverseRpm;
	SnByte ucSmallJointCurvedForward2Rpm;
	SnByte ucSmallJointCurvedReverse2Rpm;
	SnByte ucSmallJointCurvedOscillateRpm;
	SnByte ucSmallJointCurvedOscillateSec;

	SnByte ucSmallJointStraightForwardRpm;
	SnByte ucSmallJointStraightReverseRpm;
	SnByte ucSmallJointStraightForward2Rpm;
	SnByte ucSmallJointStraightReverse2Rpm;
	SnByte ucSmallJointStraightOscillateRpm;
	SnByte ucSmallJointStraightOscillateSec;

	SnByte ucSmallJointBurForwardRpm;
	SnByte ucSmallJointBurReverseRpm;
	SnByte ucSmallJointBurForward2Rpm;
	SnByte ucSmallJointBurReverse2Rpm;
	SnByte ucSmallJointBurOscillateRpm;
	SnByte ucSmallJointBurOscillateSec;

} SAVE_DEVICE_DATA;

#define FLASH_PAGE_SIZE             0x20000

/*
 * Although the FLASH area 0x00000 - 0x3FFFF is reserved for the Boot Loader,
 * the actual size of the Boot Loader is ~20K (0x5000), so there is still plenty of room.
 */
#define FLASH_BOOT_OFFSET           0x00000
#define FLASH_SERIAL_NUMBER_OFFSET  0x40000
#define FLASH_DIAGNOSTIC_OFFSET     0x60000
#define FLASH_TESTAPP_OFFSET        0x80000
#define FLASH_TOUCH_DATA_OFFSET     0xA0000

#define FLASH_SAVE_BASE			    0xC0000
#define FLASH_SAVE_TOP			    0x100000
#define FLASH_SAVE_SIZE             ((sizeof(SAVE_DEVICE_DATA)*2)+sizeof(SnQByte))

// Total size 56 bits = 7 bytes
typedef struct  
{
    
    SnByte ucLanguage:			5;

	SnByte ucCustDefaultMode:   1;
    SnByte ucOscModePortA:		2;
    SnByte ucOscModePortB:		2;
	SnByte ucOscPortASec:		8;	
	SnByte ucOscPortBSec:		8;	
	SnByte ucOscPortARev:		4;	
	SnByte ucOscPortBRev:		4;
	SnByte ucFootMode:			2;
	SnByte ucFootForward:		2;
	SnByte ucFootHandCtl:		2;
	SnByte ucPortCtl:			4;
	SnByte ucShaverPktCtl:		4;
  
    // Crc over all bytes except this one, so total CRC should be 0
    SnByte ucCrc:				8;

} NVRAM_DATA;


#define NVRAM_DATA_SIZE (sizeof(NVRAM_DATA))

#endif // NVRAMFLASH_H