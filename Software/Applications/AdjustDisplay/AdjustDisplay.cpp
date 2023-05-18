//
// AdjustDisplay.cpp : Defines the entry point for the console application.
//
#include <windows.h> 
#include "SnTypes.h"
#include "SnIoctl.h"
#include "AdjustDisplay.h"

HANDLE hComPort = INVALID_HANDLE_VALUE;
HANDLE hDriverDev = INVALID_HANDLE_VALUE;
AtmelLcdc *ptLcdc = (AtmelLcdc *)0x00000000;
AtmelPio *ptPioC = (AtmelPio *)0x00000000;

void InitCOM(void)
{
    //
    // Setup COM port for no HW/SW handshake, COMM_BAUD baud, 8 bit, no parity
    // with 1 stop bit
    //
    static DCB tDCB = {
        sizeof(DCB),            // DCBlength
        CBR_115200,             // BaudRate
        TRUE,                   // fBinary - Binary Mode (Always TRUE)
        FALSE,                  // fParity - No Parity check
        FALSE,                  // fOutxCtsFlow - No CTS control
        FALSE,                  // fOutxDsrFlow - No DSR control
        DTR_CONTROL_DISABLE,    // fDtrControl - No DTR control
        FALSE,                  // fDsrSensitivity - No DSR sensitivity
        TRUE,                   // fTXContinueOnXoff - NA
        FALSE,                  // fOutX - No XOFF flow control
        FALSE,                  // fInX - No XON flow control
        FALSE,                  // fErrorChar - No parity error replacement
        FALSE,                  // fNull - Null bytes are not discarded
        RTS_CONTROL_DISABLE,    // fRtsControl - No RTS control
        FALSE,                  // fAbortOnError - Error does not stop read/writes
        0,                      // fDummy2 - NA
        0,                      // wReserved - NA
        0,                      // XonLim - NA
        0,                      // XoffLim - NA
        8,                      // ByteSize
        NOPARITY,               // Parity
        ONESTOPBIT,             // StopBits
        0,                      // XonChar - NA
        0,                      // XoffChar - NA
        0,                      // ErrorChar - NA
        0,                      // EofChar - NA
        0,                      // EvtChar - NA
        0                       // wReserved1
    };

    //
    // Disable all timeouts for COM port
    //
    static COMMTIMEOUTS tCommTimeouts = {
        0,          // ReadIntervalTimeout = 0
        0,          // ReadTotalTimeoutMultiplier = 0
        0,          // ReadTotalTimeoutConstant = 0
        0,          // WriteTotalTimeoutMultiplier = 0
        0           // WriteTotalTimeoutConstant = 0
    };
    
    if (hComPort == INVALID_HANDLE_VALUE) {
        hComPort = CreateFile(TEXT("COM2:"), GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, 0, NULL);

        if (hComPort == INVALID_HANDLE_VALUE ||
          SetCommState(hComPort, &tDCB) == FALSE ||
          SetCommTimeouts(hComPort, &tCommTimeouts) == FALSE) {
            return;
        }
    }
}

SnBool InitDriver(void)
{
    SnBaseAddress tBaseAddress;
    DWORD dwActualOut = 0;

    hDriverDev = CreateFile(TEXT("SNE1:"), GENERIC_READ|GENERIC_WRITE,
						   0,
						   NULL,
						   OPEN_EXISTING,
						   0,
						   NULL);

    if (hDriverDev == INVALID_HANDLE_VALUE) {      
        NKDbgPrintfW(TEXT("Failed to open Driver.\n\r"));
        return FALSE;
    }

    if (DeviceIoControl(hDriverDev, IOCTL_GET_BASE_REGS, NULL, 0,
        &tBaseAddress, sizeof(SnBaseAddress), &dwActualOut, NULL) == FALSE ||
        dwActualOut != sizeof(SnBaseAddress)) {
        NKDbgPrintfW(TEXT("Could not get register base from Driver.\n\r"));
		CloseHandle(hDriverDev);
		hDriverDev = INVALID_HANDLE_VALUE;
		return FALSE;
    }

    ptLcdc = (AtmelLcdc *)((SnAddr)tBaseAddress.pxAddrBase + 0x00700000);
    ptPioC = (AtmelPio *)((SnAddr)tBaseAddress.pxAtmelRegBase + 0xFFF600);

    return TRUE;
}

/*
 * NKDbgPrintfW
 * 
 *   Function to printf for debug
 *
 * Return Value:
 */
void WINAPIV NKDbgPrintfW(LPCWSTR lpszFmt, ...) 
 
{
    static TCHAR  szDebugStr[1024] = TEXT("");
    static char   szAnsiString[2048];
    DWORD dwRet;

    int           nLength;
    BOOL          yDefaultUsed;

	va_list ap;
    va_start(ap, lpszFmt);    
    
    wvsprintf(&szDebugStr[0], lpszFmt, ap);
    nLength = WideCharToMultiByte(
            CP_ACP, WC_COMPOSITECHECK | WC_DEFAULTCHAR,
            szDebugStr, _tcslen(szDebugStr), szAnsiString, sizeof(szAnsiString),
            "?", &yDefaultUsed);

	WriteFile(hComPort, szAnsiString, nLength, &dwRet, NULL );
	va_end (ap);
}

typedef struct {
    SnQByte qVBP;
    SnQByte qVFP;
    SnQByte qVPW;
    SnQByte qHBP;
    SnQByte qHFP;
    SnQByte qHPW;
    SnQByte qPC;
} SnVideo;

#define MASTER_CLOCK    120000000.0

#define PC_IN_MHZ(qPC)  (MASTER_CLOCK/(double)(((qPC)+1)*2))
#define MHZ_TO_PC(qMHz) (((SnQByte)(MASTER_CLOCK/(double)(2*(qMHz))))-1)

// Defaults for Sharp display
SnVideo tDef = {
    13,
    2,
    10,
    2,
    2,
    41,
    MHZ_TO_PC(30000000)
};

SnVideo tAdj;

int main(int argc, char *argv[], char *envp[])
{
	DWORD dwNumBytesRead;
    SnBool yUpdateLcdc = TRUE;
    SnByte bVal;

    InitCOM();

    if (InitDriver() == FALSE) {
        return 0;
    }

    tAdj = tDef;

    NKDbgPrintfW(TEXT("\n\r\n\rVideo Timing Adjustment...\n\r\n\r"));

    NKDbgPrintfW(TEXT("\n\rVBP: %3d{%3d}  VFP: %3d{%3d}  VPW: %3d{%3d}\n\r"), tAdj.qVBP, tDef.qVBP,
        tAdj.qVFP, tDef.qVFP, tAdj.qVPW, tDef.qVPW);
    NKDbgPrintfW(TEXT("HBP: %3d{%3d}  HFP: %3d{%3d}  HPW: %3d{%3d}\n\r"),  tAdj.qHBP, tDef.qHBP,
        tAdj.qHFP, tDef.qHFP, tAdj.qHPW, tDef.qHPW);
    NKDbgPrintfW(TEXT("PixelClock: %.2f MHz{%.2f MHz}\n\r"),
        PC_IN_MHZ(tAdj.qPC) / 1000000.0, PC_IN_MHZ(tDef.qPC) / 1000000.0);

    do {
        yUpdateLcdc = FALSE;
        bVal = 0;

	    ReadFile(hComPort, &bVal,1, &dwNumBytesRead, NULL);

        switch(bVal) {
        case 'q':
            break;
        case 'a':
            if (tAdj.qVBP <= 0xff) {
                tAdj.qVBP++;
                yUpdateLcdc = TRUE;
            }
            break;
        case 's':
            if (tAdj.qVBP > 1) {
                tAdj.qVBP--;
                yUpdateLcdc = TRUE;
            }
            break;
        case 'd':
            if (tAdj.qVFP <= 0xff) {
                tAdj.qVFP++;
                yUpdateLcdc = TRUE;
            }
            break;
        case 'f':
            if (tAdj.qVFP > 1) {
                tAdj.qVFP--;
                yUpdateLcdc = TRUE;
            }
            break;
        case 'g':
            if (tAdj.qVPW <= 0x3f) {
                tAdj.qVPW++;
                yUpdateLcdc = TRUE;
            }
            break;
        case 'h':
            if (tAdj.qVPW > 1) {
                tAdj.qVPW--;
                yUpdateLcdc = TRUE;
            }
            break;
        case 'z':
            if (tAdj.qHBP <= 0xff) {
                tAdj.qHBP++;
                yUpdateLcdc = TRUE;
            }
            break;
        case 'x':
            if (tAdj.qHBP > 1) {
                tAdj.qHBP--;
                yUpdateLcdc = TRUE;
            }
            break;
        case 'c':
            if (tAdj.qHFP <= 0xff) {
                tAdj.qHFP++;
                yUpdateLcdc = TRUE;
            }
            break;
        case 'v':
            if (tAdj.qHFP > 1) {
                tAdj.qHFP--;
                yUpdateLcdc = TRUE;
            }
            break;
        case 'b':
            if (tAdj.qHPW <= 0x3f) {
                tAdj.qHPW++;
                yUpdateLcdc = TRUE;
            }
            break;
        case 'n':
            if (tAdj.qHPW > 1) {
                tAdj.qHPW--;
                yUpdateLcdc = TRUE;
            }
            break;
        case '[':
            if (tAdj.qPC < 0x1ff) {
                tAdj.qPC++;
                yUpdateLcdc = TRUE;
            }
            break;
        case ']':
            if (tAdj.qPC > 0) {
                tAdj.qPC--;
                yUpdateLcdc = TRUE;
            }
            break;
        case ' ':
            tAdj = tDef;
            yUpdateLcdc = TRUE;
            break;
        case '1':
       		ptPioC->qPIO_CODR = 0x00000008;
            break;
        case '2':
       		ptPioC->qPIO_SODR = 0x00000008;
            break;
        }

        if (yUpdateLcdc) {
            ptLcdc->qLCDC_LCDCON1 = tAdj.qPC << 12;
            ptLcdc->qLCDC_TIM1 = ((tAdj.qVPW-1) << 16) | ((tAdj.qVBP-1) << 8) | (tAdj.qVFP-1);
            ptLcdc->qLCDC_TIM2 = ((tAdj.qHFP-1) << 21) | ((tAdj.qHPW-1) << 8) | (tAdj.qHBP-1);

            NKDbgPrintfW(TEXT("\n\rVBP: %3d{%3d}  VFP: %3d{%3d}  VPW: %3d{%3d}\n\r"), tAdj.qVBP, tDef.qVBP,
                tAdj.qVFP, tDef.qVFP, tAdj.qVPW, tDef.qVPW);
            NKDbgPrintfW(TEXT("HBP: %3d{%3d}  HFP: %3d{%3d}  HPW: %3d{%3d}\n\r"),  tAdj.qHBP, tDef.qHBP,
                tAdj.qHFP, tDef.qHFP, tAdj.qHPW, tDef.qHPW);
            NKDbgPrintfW(TEXT("PixelClock: %.2f MHz{%.2f MHz}\n\r"),
                PC_IN_MHZ(tAdj.qPC) / 1000000.0, PC_IN_MHZ(tDef.qPC) / 1000000.0);
        }
    } while (bVal != 'q');


    CloseHandle(hComPort);

    return 0;
}
