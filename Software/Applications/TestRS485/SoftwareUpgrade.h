#define HDR_WINCE_NEXTGEN   0
#define HDR_MOTOR_NEXTGEN   1
#define HDR_RS485_FOOT      2
#define HDR_WINCE_BASIC     3
#define HDR_MOTOR_BASIC     4
#define HDR_RS485_MDU_SJ	5
#define HDR_RS485_MDU_FAST	6
#define HDR_RS485_FOOT_1    7

#define COPY_ZERO           0
#define COPY_UNCOMPRESSED   1
#define COPY_RL_COMPRESSED  2

typedef struct {
    SnQByte qType;
    SnQByte qSplashRamStart;
    SnQByte qSplashRamLen;
    SnQByte qSplashRomLen;
    SnQByte qNkRamStart;
    SnQByte qNkRamLen;
    SnQByte qNkRomLen;
    SnQByte qNkRamBoot;
    SnByte bSplashCopy;
    SnByte bSplashCrc;
    SnByte bNkCopy;
    SnByte bNkCrc;
    SnByte bMajorVers;
    SnByte bMinorVers;
    SnByte bBuildVers;
    SnByte bHdrCrc;
} SnWinCeHdr;

typedef struct {
    SnQByte qType;
    SnByte bAlign;
    SnByte bNumProgramFlashPages; // 1 <= x <= 120
    SnByte bProgramFlashCrc;    // To page boundary
    SnByte bDataFlashCrc;   // over whole data flash
    SnByte bMajorVers;
    SnByte bMinorVers;
    SnByte bBuildVers;
    SnByte bHdrCrc;
} SnMotorHdr;

typedef struct {
    SnQByte qType;
    SnWord wVersion;
    SnByte bProgramFlashCrc;
    SnByte bHdrCrc;
} SnRs485Hdr;

// Define this to support Software Upgrade of the RS485 Wired Footswitch
#define RS485_FOOT_UPGRADE		1

// Define this to support Software Upgrade of the RS485 MDUs
#define RS485_MDU_UPGRADE       1

#define FACTORY_MODE_FILE_DISK1		_T("\\Hard Disk\\FactoryMode.txt")	 
#define FACTORY_MODE_FILE_DISK2		_T("\\Hard Disk2\\FactoryMode.txt")	 
#define UPGRADE_FILE_DISK1			_T("\\Hard Disk\\NextGen.upg")
#define UPGRADE_FILE_DISK2			_T("\\Hard Disk2\\NextGen.upg")

#define LOWER_FLASH_ADDR            0x100000
#define UPPER_FLASH_ADDR            0x880000
#define OFFSET_OF_CE_SPLASH         (sizeof(SnWinCeHdr))
#define OFFSET_OF_CE_ROM(tHdr)      (OFFSET_OF_CE_SPLASH + ((tHdr.qSplashRomLen + 3) & ~3))
#define TOTAL_SIZE_OF_CE_DATA(tHdr) (OFFSET_OF_CE_ROM(tHdr) + ((tHdr.qNkRomLen + 3) & ~3))
