//
// Includes common to all the MDD files.
//
#define WINCEOEM 1
#define DEBUG 1

#include <windows.h>
#include <types.h>
#include <memory.h>
#include <string.h>
#include <nkintr.h>
#include <excpt.h>

#include "SnTypes.h"
#include "SneDriver.h"

void *g_pxLcdcMap = 0;
void *g_pxFlashMap = 0;
void *g_pxAtmelMap = 0;

AtmelLcdc *g_ptLcdc = 0;
AtmelCan *g_ptCan = 0;
AtmelPio *g_ptPioA = 0;
AtmelPio *g_ptPioB = 0;
AtmelPio *g_ptPioC = 0;
AtmelPio *g_ptPioE = 0;
AtmelPmc *g_ptPmc = 0;

SnQByte g_qNvRamOffs = 0;
SnQByte *g_pqNvRam = 0;
SnBool g_yNvRamBattery = FALSE;

SnQByte g_qBootStatus = 0;

SnBool g_yShutdown = FALSE;

DWORD  g_dwDspIrq = LOGINTR_BASE_PIOB + 23;
DWORD  g_dwDspSysIntr;
HANDLE g_hMsgInterrupt;
HANDLE g_hMsgInterruptThread;
HANDLE g_hMsgMutex;

DWORD  g_dwFlashIrq = LOGINTR_BASE_PIOA + 22;
DWORD  g_dwFlashSysIntr;
HANDLE g_hFlashInterrupt;
HANDLE g_hFlashInterruptThread;
HANDLE g_hFlashWaitEvent;

HANDLE g_hNvRamMutex;
HANDLE g_hReadyCmd;

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

/*
 * Map in an area of Physical Space. Once mapped in this area will be available to
 * everything under Windows CE so the same pointer can be used by the Driver and
 * User space.
 *
 * This function assumes that the Physical Area is page aligned.
 */
PVOID MapIoSpace (SnAddr aPhysicalAddress, ULONG NumberOfBytes)
{
    int     iFlags = PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE;
    SnByte  *pbVirtualAddress;
    SnBool   ySuccess;

	pbVirtualAddress = VirtualAlloc(0, NumberOfBytes, MEM_RESERVE, PAGE_NOACCESS);
	if (pbVirtualAddress != NULL) {
	    ySuccess = VirtualCopy((PVOID)pbVirtualAddress,
				    (PVOID)(aPhysicalAddress >> 8), NumberOfBytes,
				    iFlags);
		if (!ySuccess) {
		    VirtualFree((PVOID)pbVirtualAddress, NumberOfBytes, MEM_RELEASE);
		    pbVirtualAddress = NULL;
		}
	}
    return ((PVOID)pbVirtualAddress);
}

VOID UnMapIoSpace (PVOID pVirtualAddress, ULONG NumberOfBytes)
{
	VirtualFree(pVirtualAddress, NumberOfBytes, MEM_RELEASE);
}

BOOL __stdcall
SNE_DllEntry (HANDLE hinstDLL, DWORD Op, LPVOID lpvReserved)
{
    switch (Op) {
    case DLL_PROCESS_ATTACH :
        break;
    case DLL_PROCESS_DETACH :
        break;
    case DLL_THREAD_DETACH :
        break;
    case DLL_THREAD_ATTACH :
        break;
    default :
        break;
    }

    return TRUE;
}

typedef struct {
    SnWord *pwName;
    SnQByte qValue;
} SnRegDword;

const SnRegDword ptHantronixDisplay[] = {
    TEXT("Width"),                  800,
    TEXT("Height"),                 480,
    TEXT("Bpp"),                    16,
    TEXT("VRAMWidthInPixel"),       2048,
    TEXT("VRAMHeightInPixel"),      2048,
    TEXT("VRAMAddress"),            0x23800000,
    TEXT("VRAMBusWidth"),           32,
    TEXT("UpperMargin"),            0,              // VBP = 0 (VSYNC Back Porch - 1)
    TEXT("LowerMargin"),            44,             // VFP = 44 (VSYNC Front Porch - 1)
    TEXT("LeftMargin"),             0,              // HBP = 0 (HSYNC Back Porch - 1)
    TEXT("RightMargin"),            255,            // HFP = 255 (HSYNC Front Porch - 1)
    TEXT("Vsync"),                  0x80,           // VPW = 0, INVFRAME=1 (VSYNC Pulse Width - 1, Active Low)           // VSync = 0x00, Inverted
    TEXT("Hsync"),                  0x80,           // HPW = 0, INVLINE=1 (HSYNC Pulse Width - 1, Active Low)
    TEXT("PixelClock"),             30000000,       // 30 MHz
    0,                              0
};

SnBool SetCalibrationRegistry(void)
{
    SnWord pwTouchData[64];
    SnQByte pqSavedData[6];
    DWORD dwSize, dwRet;
    HKEY hKey;
    SnByte bCrc = 0;

    // Read saved Touch Data. If no valid Touch Data, set it to Default and write it out to Flash.
    if (!SetFlashOffset(FLASH_TOUCH_DATA_OFFSET) || !ReadFlashData(pqSavedData, 24) ||
        !CrcBufData((SnByte *)pqSavedData, 20, &bCrc) || (SnQByte)bCrc != pqSavedData[5]) {

        NKDbgPrintfW(TEXT("Setting Touch Data to Defaults.\n"));

        // No valid Touch Data so set to Defaults
        pqSavedData[0] = (2082 << 16) | 2034;
        pqSavedData[1] = (3204 << 16) | 3086;
        pqSavedData[2] = (3201 << 16) | 977;
        pqSavedData[3] = (962 << 16) | 989;
        pqSavedData[4] = (971 << 16) | 3072;
        bCrc = 0;
        if (CrcBufData((SnByte *)pqSavedData, 20, &bCrc) && EraseFlashPages(1)) {
            pqSavedData[5] = (SnQByte)bCrc;
            WriteFlashData(pqSavedData, 24);
        }
    }
    // Reset the Flash offset to Default
    SetFlashOffset(FLASH_DEFAULT_OFFSET);

    wsprintf(pwTouchData, TEXT("%d,%d %d,%d %d,%d %d,%d %d,%d "), pqSavedData[0] >> 16, pqSavedData[0] & 0xffff,
      pqSavedData[1] >> 16, pqSavedData[1] & 0xffff, pqSavedData[2] >> 16, pqSavedData[2] & 0xffff,
      pqSavedData[3] >> 16, pqSavedData[3] & 0xffff, pqSavedData[4] >> 16, pqSavedData[4] & 0xffff);

    NKDbgPrintfW(TEXT("Touch Data: %s\n"), pwTouchData);

	// Open the Touch key
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\TOUCH"), 0, 0, &hKey);
    if (dwRet != ERROR_SUCCESS) {
        return FALSE;
    }

    // Store the new Calibration Data
    dwSize = (wcslen(pwTouchData) + 1) * sizeof(SnWord);
    dwRet = RegSetValueEx(hKey, TEXT("CalibrationData"), 0, REG_SZ, (PUCHAR)pwTouchData, dwSize);
    RegCloseKey(hKey);
    if (dwRet != ERROR_SUCCESS) {
        return FALSE;
    }

    return TRUE;
}

SnBool SetLCDCRegistry(void)
{
    const SnRegDword *ptLCDC;
    DWORD dwSize, dwRet;
    HKEY hKey;

    ptLCDC = ptHantronixDisplay;

    // Open the Display key
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Drivers\\Display\\LCDC"), 0, 0, &hKey);
    if (dwRet != ERROR_SUCCESS) {
        return FALSE;
    }

    // Store the Display Parameters
    while (ptLCDC->pwName) {
        dwSize = sizeof(DWORD);
        dwRet = RegSetValueEx(hKey, ptLCDC->pwName, 0, REG_DWORD, (PUCHAR)&ptLCDC->qValue, dwSize);
        if (dwRet != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return FALSE;
        }
        ptLCDC++;
    }

    RegCloseKey(hKey);

    return TRUE;
}

DWORD SNE_Init(DWORD Index)
{
	NKDbgPrintfW(TEXT("SNE_Init(%d)\n\r"), Index);

    g_pxLcdcMap  = MapIoSpace(0x00600000, 0x200000);
    g_pxFlashMap = MapIoSpace(0x10000000, 0x1000000);
    g_pxAtmelMap = MapIoSpace(0xFFE00000, 0x200000);

    NKDbgPrintfW(TEXT("g_pxLcdcMap  = 0x%x\n"), g_pxLcdcMap);
    NKDbgPrintfW(TEXT("g_pxFlashMap = 0x%x\n"), g_pxFlashMap);
    NKDbgPrintfW(TEXT("g_pxAtmelMap = 0x%x\n"), g_pxAtmelMap);

    if (g_pxLcdcMap == NULL || g_pxFlashMap == NULL || g_pxAtmelMap == NULL) {
        goto Init_Error;
    }

    g_ptLcdc  = (AtmelLcdc*)((SnAddr)g_pxLcdcMap  + 0x100000);
    g_ptCan   = (AtmelCan*) ((SnAddr)g_pxAtmelMap + 0x1AC000);
    g_ptPioA  = (AtmelPio*) ((SnAddr)g_pxAtmelMap + 0x1FF200);
    g_ptPioB  = (AtmelPio*) ((SnAddr)g_pxAtmelMap + 0x1FF400);
    g_ptPioC  = (AtmelPio*) ((SnAddr)g_pxAtmelMap + 0x1FF600);
    g_ptPioE  = (AtmelPio*) ((SnAddr)g_pxAtmelMap + 0x1FFA00);
    g_ptPmc   = (AtmelPmc*) ((SnAddr)g_pxAtmelMap + 0x1FFC00);
    g_pqNvRam = (SnQByte*)  ((SnAddr)g_pxAtmelMap + 0x1FFD6C);

    // The first two LUT entries are combined to get a 32 bit status from the bootstrap.
    g_qBootStatus = (g_ptLcdc->pqLCDC_LUT_ENTRY[1] << 16) | g_ptLcdc->pqLCDC_LUT_ENTRY[0];
    NKDbgPrintfW(TEXT("g_qBootStatus = 0x%08x\n"), g_qBootStatus);

    if (g_pqNvRam[16] == 0x55555555)
        g_yNvRamBattery = TRUE;
    g_pqNvRam[16] = 0x55555555;

    // Configure PA13 to be CANTX
    // Configure PA14 to be CANRX
    // Configure PA22 to be a PIO input (Flash Busy)
    //     Disable Multi Output on PA13
    //     Disable Output on PA14/PA22
    //     Enable Pullup on PA14/PA22
    //     Configure PA13/PA14 to Function A, (CANTX/CANRX)
    //     Disable PIO control of PA13/PA14
    //     Enable PIO control of PA22
    g_ptPioA->qPIO_MDDR = 0x00002000;
    g_ptPioA->qPIO_ODR = 0x00404000;
    g_ptPioA->qPIO_PPUER = 0x00404000;
    g_ptPioA->qPIO_ASR = 0x00006000;
    g_ptPioA->qPIO_PDR = 0x00006000;
    g_ptPioA->qPIO_PER = 0x00400000;

    // Turn on Clock for the CAN controller
    g_ptPmc->qPMC_PCER = 0x00001000;

    // Configure PB21 to be a PIO output (Dsp Reset)
    // Configure PB18 to be a PIO output (Buzzer)
    //     Set PB21 to High
    //     Set PB18 to Low
    //     Enable Output on PB21, PB18
    //     Enable PIO control of PB21, PB18
    //     Disable Pullup on PB21, PB18
    g_ptPioB->qPIO_SODR = 0x00200000;
    g_ptPioB->qPIO_CODR = 0x00040000;
    g_ptPioB->qPIO_MDDR = 0x00240000;
    g_ptPioB->qPIO_OER = 0x00240000;
    g_ptPioB->qPIO_PER = 0x00240000;
    g_ptPioB->qPIO_PPUDR = 0x00240000;

    // Configure PC0, PC1 to be PIO Output for Debug
    //     Set PC0, PC1 to Low
    //     Enable Output on PC0, PC1
    //     Enable PIO control of PC0, PC1
    //     Disable Pullup on PC0, PC1
    g_ptPioC->qPIO_CODR = 0x00000003;
    g_ptPioC->qPIO_MDDR = 0x00000003;
    g_ptPioC->qPIO_OER = 0x00000003;
    g_ptPioC->qPIO_PER = 0x00000003;
    g_ptPioC->qPIO_PPUDR = 0x00000003;

    SetLCDCRegistry();

    g_hMsgInterrupt = CreateEvent(0, FALSE, FALSE, NULL);
	if (!g_hMsgInterrupt) {
        goto Init_Error;
    }

    g_hFlashInterrupt = CreateEvent(0, FALSE, FALSE, NULL);
	if (!g_hFlashInterrupt) {
        goto Init_Error;
    }

    g_hFlashWaitEvent = CreateEvent(0, FALSE, FALSE, NULL);
	if (!g_hFlashWaitEvent) {
        goto Init_Error;
    }

    g_hMsgMutex = CreateMutex(NULL, FALSE, NULL);
	if (!g_hMsgMutex) {
        goto Init_Error;
    }

    g_hNvRamMutex = CreateMutex(NULL, FALSE, NULL);
	if (!g_hNvRamMutex) {
        goto Init_Error;
    }

    g_hReadyCmd = CreateEvent(0, FALSE, FALSE, NULL);
	if (!g_hReadyCmd) {
        goto Init_Error;
    }

    KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR,
				(VOID *)(&g_dwDspIrq), sizeof(g_dwDspIrq),
				(VOID *)(&g_dwDspSysIntr), sizeof(g_dwDspSysIntr), NULL);

	if (g_dwDspSysIntr == SYSINTR_UNDEFINED) {
        goto Init_Error;
	}

    if (!InterruptInitialize(g_dwDspSysIntr, g_hMsgInterrupt, NULL, 0)) {
        goto Init_Error;
	}
	InterruptDone(g_dwDspSysIntr);

    g_hMsgInterruptThread =
		CreateThread((LPSECURITY_ATTRIBUTES)NULL,		// create the IST
					 0,
					 (LPTHREAD_START_ROUTINE)MsgInterruptThread,
					 NULL,
					 0,
					 NULL);
	if (!g_hMsgInterruptThread) {
        goto Init_Error;
	}

    // Bump up the priority since the interrupt notifies us of activity
	// on the hardware
	CeSetThreadPriority (g_hMsgInterruptThread, 0);

	KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR,
				(VOID *)(&g_dwFlashIrq), sizeof(g_dwFlashIrq),
				(VOID *)(&g_dwFlashSysIntr), sizeof(g_dwFlashSysIntr), NULL);

	if (g_dwFlashSysIntr == SYSINTR_UNDEFINED) {
        goto Init_Error;
	}

    if (!InterruptInitialize(g_dwFlashSysIntr, g_hFlashInterrupt, NULL, 0)) {
        goto Init_Error;
	}
	InterruptDone(g_dwFlashSysIntr);

    g_hFlashInterruptThread =
		CreateThread((LPSECURITY_ATTRIBUTES)NULL,		// create the IST
					 0,
					 (LPTHREAD_START_ROUTINE)FlashInterruptThread,
					 NULL,
					 0,
					 NULL);
	if (!g_hFlashInterruptThread) {
        goto Init_Error;
	}

    // Bump up the priority since the interrupt notifies us of activity
	// on the hardware
	CeSetThreadPriority (g_hFlashInterruptThread, 0);
    InitMsg();

    if (InitFlash() == FALSE) {
	    NKDbgPrintfW(TEXT("SNE_Init(): Init Flash failed\n\r"));
        goto Init_Error;
    }
    SetCalibrationRegistry();

	// Return Non-Zero for success, will be passed to Deinit and Open
    return 1;

Init_Error:
    SNE_Deinit(0);
    return 0L;
}

BOOL SNE_Deinit(DWORD dwData)
{
	NKDbgPrintfW(TEXT("SNE_Denit(%d)\n\r"), dwData);

    g_yShutdown = TRUE;
    if (g_pxLcdcMap) {
        UnMapIoSpace((PVOID)g_pxLcdcMap, 0x200000);
        g_pxLcdcMap = 0;
        g_ptLcdc = 0;
    }
    if (g_pxFlashMap) {
        UnMapIoSpace((PVOID)g_pxFlashMap, 0x1000000);
        g_pxFlashMap = 0;
    }
    if (g_pxAtmelMap) {
        UnMapIoSpace((PVOID)g_pxAtmelMap, 0x200000);
        g_pxAtmelMap = 0;
        g_ptCan = 0;
        g_ptPioA = 0;
        g_ptPioB = 0;
        g_ptPioC = 0;
        g_ptPioE = 0;
        g_pqNvRam = 0;
    }
    if (g_hMsgInterrupt) {
		CloseHandle(g_hMsgInterrupt);
		g_hMsgInterrupt = NULL;
	}
    if (g_hFlashInterrupt) {
		CloseHandle(g_hFlashInterrupt);
		g_hFlashInterrupt = NULL;
	}
    if (g_hMsgMutex) {
		CloseHandle(g_hMsgMutex);
		g_hMsgMutex = NULL;
	}
    if (g_hNvRamMutex) {
		CloseHandle(g_hNvRamMutex);
		g_hNvRamMutex = NULL;
	}
    if (g_hReadyCmd) {
		CloseHandle(g_hReadyCmd);
		g_hReadyCmd = NULL;
	}

    return TRUE;
}

DWORD
SNE_Open(DWORD dwData, DWORD dwAccess, DWORD dwShareMode)
{
	NKDbgPrintfW(TEXT("SNE_Open(%d, 0x%x, %d)\n\r"), dwData, dwAccess, dwShareMode);

    return 1;
}

BOOL
SNE_Close(DWORD dwData)
{
	NKDbgPrintfW(TEXT("SNE_Close(%d)\n\r"), dwData);
    return TRUE;
}

DWORD
SNE_Read(DWORD dwData, LPVOID pBuf, DWORD Len)
{
    SnQByte *pqSrc, *pqDst;
    SnQByte qCnt;

    WaitForSingleObject(g_hNvRamMutex, INFINITE);

    if (g_qNvRamOffs >= 64 || (g_qNvRamOffs + Len) > 64 || Len & 3) {
        return (SnQByte)-1;
    }

    pqSrc = &g_pqNvRam[g_qNvRamOffs >> 2];
    pqDst = pBuf;
    qCnt = Len >> 2;

    while (qCnt-- > 0) {
	    *pqDst++ = *pqSrc++;
    }

    ReleaseMutex(g_hNvRamMutex);

    return Len;
}

DWORD
SNE_Write(DWORD dwData, LPCVOID pBuf, DWORD Len)
{
    SnQByte *pqSrc, *pqDst;
    SnQByte qCnt;


    WaitForSingleObject(g_hNvRamMutex, INFINITE);

    if (g_qNvRamOffs >= 64 || (g_qNvRamOffs + Len) > 64 || Len & 3) {
        return (SnQByte)-1;
    }

    pqSrc = (SnQByte *)pBuf;
    pqDst = &g_pqNvRam[g_qNvRamOffs >> 2];
    qCnt = Len >> 2;

    while (qCnt-- > 0) {
	    *pqDst++ = *pqSrc++;
    }

    ReleaseMutex(g_hNvRamMutex);

    return Len;
}

DWORD
SNE_Seek(DWORD dwData, long pos, WORD type)
{
    long lOffs;

    WaitForSingleObject(g_hNvRamMutex, INFINITE);

    switch (type) {
    case FILE_BEGIN:
        lOffs = 0;
        break;
    case FILE_CURRENT:
        lOffs = g_qNvRamOffs;
        break;
    case FILE_END:
        lOffs = 64;
        break;
    default:
        return -1;
    }

    lOffs += pos;

    if (lOffs < 0 || lOffs >= 64 || lOffs & 3) {
        return -1;
    }

    g_qNvRamOffs = lOffs;

    ReleaseMutex(g_hNvRamMutex);

    return g_qNvRamOffs;
}

VOID
SNE_PowerUp(DWORD DeviceContext)
{
}

VOID
SNE_PowerDown(DWORD DeviceContext)
{
}

BOOL
SNE_IOControl(DWORD dwOpenData, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn,
		PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    SnBool yStatus = TRUE;

//	NKDbgPrintfW(TEXT("SNE_IOControl(%d, 0x%x, In={0x%x, %d}, Out={0x%x, %d}, 0x%x)\n\r"),
//        dwOpenData, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);

    switch (dwCode) {
    case IOCTL_SND_CMD:
        if ((dwLenIn & 1) == 0 && (dwLenOut & 1) == 0) {
            WaitForSingleObject(g_hMsgMutex, INFINITE);
            yStatus = SndCmd((SnWord *)pBufIn, dwLenIn >> 1, (SnWord *)pBufOut, dwLenOut >> 1,
                pdwActualOut);
            ReleaseMutex(g_hMsgMutex);
        } else {
            yStatus = FALSE;
        }
	    break;
    case IOCTL_SET_FLASH_OFFS:
        if (dwLenIn == sizeof(SnQByte)) {
           	yStatus = SetFlashOffset(*(SnQByte *)pBufIn);
        } else {
            yStatus = FALSE;
        }
	    break;
    case IOCTL_READ_FLASH:
        yStatus = ReadFlashData((volatile SnQByte *)pBufOut, dwLenOut);
        if (pdwActualOut) {
            *pdwActualOut = yStatus ? dwLenOut : 0;
        }
	    break;
    case IOCTL_WRITE_FLASH:
        yStatus = WriteFlashData((volatile SnQByte *)pBufIn, dwLenIn);
	    break;
    case IOCTL_VERIFY_FLASH:
        yStatus = VerifyFlashData((volatile SnQByte *)pBufIn, dwLenIn);
	    break;
    case IOCTL_ERASE_FLASH:
        if (dwLenIn == sizeof(SnQByte)) {
           	yStatus = EraseFlashPages(*(SnQByte *)pBufIn);
        } else {
            yStatus = FALSE;
        }
	    break;
    case IOCTL_CHECK_BOOT:
        if (dwLenOut == sizeof(SnQByte)) {
           	*(SnQByte *)pBufOut = g_qBootStatus;
            if (pdwActualOut) {
                *pdwActualOut = sizeof(SnQByte);
            }
        } else {
            yStatus = FALSE;
        }
	    break;
    case IOCTL_READ_FLASH_STORE:
        if (dwLenIn == sizeof(SnFlashStore)) {
            yStatus = ReadFlashStore((SnFlashStore *)pBufIn, pBufOut, pdwActualOut);
        } else {
            yStatus = FALSE;
        }
	    break;
    case IOCTL_WRITE_FLASH_STORE:
        yStatus = WriteFlashStore(pBufIn, dwLenIn);
	    break;
    case IOCTL_ERASE_FLASH_STORE:
		yStatus = EraseFlashStore();
	    break;
    case IOCTL_RESET_DISP_BASE:
        if (g_ptLcdc) {
            // Set new display base address and tell the LCDC to update
            g_ptLcdc->qLCDC_BA1 = 0x23800000;
            g_ptLcdc->qLCDC_DMACON |= 0x00000008;
        }
	    break;
    case IOCTL_CHECK_NVRAM_BATTERY:
        if (dwLenOut == sizeof(SnBool)) {
           	*(SnBool *)pBufOut = g_yNvRamBattery;
            if (pdwActualOut) {
                *pdwActualOut = sizeof(SnBool);
            }
        } else {
            yStatus = FALSE;
        }
	    break;
    case IOCTL_SET_BUZZER:
        if (dwLenIn == sizeof(SnBool)) {
           	if (*(SnBool *)pBufIn) {
                ENABLE_BUZZER;
            } else {
                DISABLE_BUZZER;
            }
        } else {
            yStatus = FALSE;
        }
	    break;
    case IOCTL_RESET_DSP:
        WaitForSingleObject(g_hMsgMutex, INFINITE);
        RESET_DSP;
        ReleaseMutex(g_hMsgMutex);
	    break;
    case IOCTL_CAN_TEST:
        if (dwLenOut == sizeof(SnBool)) {
           	*(SnBool *)pBufOut = CanTest();
            if (pdwActualOut) {
                *pdwActualOut = sizeof(SnBool);
            }
        } else {
            yStatus = FALSE;
        }
        break;
    default:
        yStatus = FALSE;
    }

    return yStatus;
}

