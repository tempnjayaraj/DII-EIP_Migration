// DIICondorVerif.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>

typedef unsigned char SnBool;
typedef unsigned char SnByte;
typedef unsigned short SnWord;
typedef unsigned long SnQByte;

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE 1
#endif

HANDLE hCondorSerial = NULL;
SnByte pbPacket[128];
int iPacketIndex = 0;
FILE* ptLog = NULL;

CRITICAL_SECTION xmtCriticalSection;

void XmtPacket(const SnByte* pbData,int iNumBytes);
SnBool RcvPacket(void);
void InterpretPacket(void);

void vOutput(const char* pcFmt,va_list pxArgs)
{
    vfprintf_s(stdout,pcFmt,pxArgs);
    if(ptLog) {
		vfprintf_s(ptLog,pcFmt,pxArgs);
		fflush(ptLog);
    }
}

void Output(const char* pcFmt,...)
{
    va_list pxArgs;

    va_start(pxArgs,pcFmt);
    vOutput(pcFmt,pxArgs);
    va_end(pxArgs);
}

void vError(const char* pcFmt,va_list pxArgs)
{
    vfprintf_s(stderr,pcFmt,pxArgs);
    if(ptLog) {
        vfprintf_s(ptLog,pcFmt,pxArgs);
 		fflush(ptLog);
   }
}

void Error(const char* pcFmt,...)
{
    va_list pxArgs;

    va_start(pxArgs,pcFmt);
    vError(pcFmt,pxArgs);
    va_end(pxArgs);
}

void XmtPacket(const SnByte* pbData,int iNumBytes)
{
	EnterCriticalSection(&xmtCriticalSection);
	DWORD dwNumberOfBytesWritten = 0;
	SnByte pbXmtBuf[64];
	static SnByte bSegRes = 0;

	memcpy(pbXmtBuf, pbData, iNumBytes);
	if (iNumBytes > 2) {
		pbXmtBuf[2] = bSegRes;
		pbXmtBuf[iNumBytes-2] += bSegRes;
		bSegRes += 0x10;
	}
    if((WriteFile(hCondorSerial,pbXmtBuf,iNumBytes,&dwNumberOfBytesWritten,(LPOVERLAPPED)NULL)
                == FALSE) ||
            (dwNumberOfBytesWritten != iNumBytes)) {
        Error("Error: Failed to write packet with %d bytes\n",iNumBytes);
    }
	LeaveCriticalSection(&xmtCriticalSection);
}

SnBool RcvPacket(void)
{
    DWORD dwNumberOfBytesRead = 0;
    SnBool yInPacket;
    SnBool yEscape;
    SnByte bByte;
    static const SnByte bACK = 0x06;

    yInPacket = FALSE;
    yEscape = FALSE;
    iPacketIndex = 0;
    for(;;) {
        if(!ReadFile(hCondorSerial,&bByte,1,&dwNumberOfBytesRead,(LPOVERLAPPED)NULL) ||
                (dwNumberOfBytesRead != 1)) {
            return FALSE;
        }

        if(!yInPacket) {
            if(bByte == 0x05) {
                yInPacket = TRUE;
                iPacketIndex = -3;
            } else if(bByte == 0x15) {
                pbPacket[iPacketIndex] = bByte;
                return TRUE;
            } else {
                ; // ignore byte
            }
        } else {
            if(bByte == 0x04) {
                iPacketIndex--;
                break;
            } else if(bByte == 0x17) {
                yEscape = TRUE;
            } else {
                if(yEscape) {
                    yEscape = FALSE;
                    bByte &= 0x1f;
                }
                if(++iPacketIndex >= 0) {
                    pbPacket[iPacketIndex] = bByte;
                }
            }
        }
    }

    XmtPacket(&bACK,1);
    return TRUE;
}


SnByte bMDUModePrior[3] = { 7, 7, 7 };	// Initialize all Previous Modes to Unknown 2

void InterpretStatus(void)
{
    SnWord wTmp;
    SnByte bTmp;
    SnByte bNumPorts;
    SnByte bPort;
    SnByte bMDUMode;
    int iIndex;
    SnByte bPortType;
    static char* ppcMDUModes[] = {
        "Forward    ",
        "Reverse    ",
        "Oscillate1 ",
        "Oscillate2 ",
        "WindowLock1",
        "WindowLock2",
        "Unknown_1  ",
        "Unknown_2  ",
    };

    Output("-----------------------------------------------\n");
    if(pbPacket[0] != 0xC1) {
        Error("Error: Not SRV_UPD_STATUS packet\n");
        return;
    }
    if(pbPacket[1] & 0x80) {
        bNumPorts = 3;
    } else {
        bNumPorts = 2;
    }
    Output("NumPorts: %d ",bNumPorts);

    Output("ScreenType: ");
    bTmp = pbPacket[1] & 0x7f;
    if(bTmp == 0) {
        Output("Normal\n");
    } else if(bTmp <= 0x0F) {
        Output("Fatal FE%d\n",bTmp);
    } else if(bTmp <= 0x1F) {
        Output("Expanded Popup Warning PU%d with OK button\n",bTmp - 0x0F);
    } else {
        Output("Expanded Port Warning PW%d with OK button\n",bTmp - 0x1F);
    }

    iIndex = 2;
    for(bPort = 0; bPort < bNumPorts; bPort++) {
        bTmp = pbPacket[iIndex++];
        bPortType = bTmp & 0x3;
        Output("\nPort %C: ",bPort + 'A');
        switch(bPortType) {
        case 0x0: Output("None\n"); break;
        case 0x1: Output("MDU\n"); break;
        case 0x2: Output("Powered Instrument\n"); break;
        case 0x3: Output("RF(future expansion)\n"); break;
        }

        Output("Icons : ");
        switch(bTmp & 0xc0) {
        case 0x0 << 6: Output("Pump(off)            "); break;
        case 0x1 << 6: Output("Pump(stationary)     "); break;
        case 0x2 << 6: Output("Pump(animated)       "); break;
        case 0x3 << 6: Output("Pump(Error: unknown) "); break;
        }
        switch(bTmp & 0x30) {
        case 0x0 << 4: Output("Footswitch(off)            "); break;
        case 0x1 << 4: Output("Footswitch(wired)          "); break;
        case 0x2 << 4: Output("Footswitch(wireless)       "); break;
        case 0x3 << 4: Output("Footswitch(wired+wireless) "); break;
        }
        switch(bTmp & 0x0c) {
        case 0x0 << 2: Output("Handpiece(off)\n"); break;
        case 0x1 << 2: Output("Handpiece(MDU)\n"); break;
        case 0x2 << 2: Output("Handpiece(Saw)\n"); break;
        case 0x3 << 2: Output("Handpiece(Drill)\n"); break;
        }

        bTmp = pbPacket[iIndex++];
        Output("Status: ");
        if(bTmp & 0x80) {
            Output("Running    ");
        } else {
            Output("NotRunning ");
        }
        bTmp &= 0x7f;
        Output("          PortWarning: ");
        if(bTmp == 0) {
            Output("None\n");
        } else if(bTmp <= 0x0F) {
            Output("ERROR: Fatal FE%d\n",bTmp);
        } else if(bTmp <= 0x1F) {
            Output("ERROR: Popup Warning PU%d\n",bTmp - 0x0F);
        } else {
            Output("Port Warning PW%d\n",bTmp - 0x1F);
        }

        switch(bPortType) {
        case 0:     //None
			bMDUModePrior[bPort] = 7;	// Reset the Previous MDU mode for the current port to Unknown 2
            break;
        case 1:     //MDU
            bTmp = pbPacket[iIndex++];
            bMDUMode = (bTmp >> 3) & 0x7;
            Output("UP button: %-17s DOWN button: %s\nUP1000(voice): %-13s DOWN1000(voice): %-9s DeltaMode: %s\nMode: %s\n",
                    (bTmp & 0x80)?" ON":"OFF",
                    (bTmp & 0x40)?" ON":"OFF",
                    (bTmp & 0x04)?"ENABLED":"DISABLED",
                    (bTmp & 0x02)?"ENABLED":"DISABLED",
                    (bTmp & 0x01)?"ENABLED":"DISABLED",
                    ppcMDUModes[bMDUMode]
                  );
            bTmp = pbPacket[iIndex++];
            wTmp = (SnWord)bTmp << 8;
            bTmp = pbPacket[iIndex++];
            wTmp += (SnWord)bTmp;
            Output("CurrentDisplay: %4d         ",wTmp);
 
			bTmp = pbPacket[iIndex++];
            wTmp = (SnWord)bTmp << 8;
            bTmp = pbPacket[iIndex++];
            wTmp += (SnWord)bTmp;
			
			bTmp = bMDUMode;
			if (bTmp == 0x4 || bTmp == 0x5)		// If WindowLock 1 or 2 use Prior Mode for MaxDisplay
				bTmp = bMDUModePrior[bPort];
			else								// Otherwise update the prior Mode
				bMDUModePrior[bPort] = bTmp;

			switch (bTmp){
			case 0x0:	// Forward
			case 0x1:	// Reverse
                Output("MaxDisplay:  RPM    %4d\n",wTmp);
				break;
			case 0x2:	// Oscillate 1
                Output("MaxDisplay: Mode 1   RPM\n");
				break;
			case 0x3:	// Oscillate 2
                Output("MaxDisplay: Mode 2  RATE\n");
				break;
			default:
				Output ("MaxDisplay: %d\n",bMDUMode);
				break;
			}
            break;
        case 2:     //PoweredInstrument
			bTmp = pbPacket[iIndex++];
			switch((bTmp >> 5) & 0x7) {
            case 0:
                Output("Drill");
				Output("(%s):",(bTmp & 0x10)?"Reverse":"Forward");
                break;
            case 1:
				Output("Saw:");
                break;
   
			case 2:
				Output("Combo:");
				break;

			default:
				Output("Unknown:");
                break;
            }
            Output(" %d%%\n",10*(bTmp & 0xf));
            break;
        case 3:
            Output("RF - not supported\n");
            iIndex += 3;
            break;
        }
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
	//
	// Setup COM port for no HW/SW handshake, 19200 baud, 8 bit, no parity
	// with 1 stop bit
	//
	static DCB tDCB = {
		sizeof(DCB),            // DCBlength
		CBR_19200,              // BaudRate
		TRUE,                   // fBinary - Binary Mode(Always TRUE)
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
		0,    // ReadIntervalTimeout = 0
		100,          // ReadTotalTimeoutMultiplier = 100
		0,          // ReadTotalTimeoutConstant = 0
		0,          // WriteTotalTimeoutMultiplier = 0
		0           // WriteTotalTimeoutConstant = 0
	};
	static const SnByte pbSRV_DEV[] = {0x05,0x00,0x00,0x60,0x60,0x04};
	static const SnByte pbSRV_UPD[] = {0x05,0x00,0x00,0x40,0x40,0x04};
	static const SnByte pbSRV_UPD_DEMAND[] = {0x05,0x00,0x00,0x41,0x41,0x04};
	static SnByte pbSRV_TRIG[] = {0x05,0x00,0x00,0x50,0,0x50,0x04};
	int iChar;
	SnByte bPort;
	SnBool ySrvUpdDemand = TRUE;
	SnBool noQuit = TRUE;

	InitializeCriticalSection(&xmtCriticalSection);

	if(fopen_s(&ptLog,"D2CVerif.txt","w") != NULL) {
		Error("WARNING: Could not open log file D2CVerif.txt for writing, no log file will be created\n");
	}

	Output("Attempting to communicate with DyonicsII shaver...\n");

	hCondorSerial = CreateFile(
		TEXT("COM1:"),				// lpFileName
		GENERIC_READ|GENERIC_WRITE,	// dwDesiredAccess
		0,							// dwShareMode
		NULL,						// lpSecurityAttributes
		OPEN_EXISTING,				// dwCreationDisposition
		0,							// dwFlagsAndAttributes
		NULL						// hTemplateFile
		);

	if((hCondorSerial == INVALID_HANDLE_VALUE) ||
		(SetCommState(hCondorSerial, &tDCB) == FALSE) ||
		(SetCommTimeouts(hCondorSerial, &tCommTimeouts) == FALSE)) {
			Error("Error: Failed to open COM1\n");
			return 1;
	}
	while(noQuit) {
		for(;;) {
			XmtPacket(pbSRV_DEV,sizeof(pbSRV_DEV));
			if(!RcvPacket()) {
				Error("No Dyonics II shaver detected, retrying (Hit any key to quit)...\n");
				Sleep(1000);
				if(_kbhit()) {
					noQuit = FALSE;
					goto L_END;
				}
				continue;
			} else if(
				(pbPacket[0] != 0xE0)
				||
				(pbPacket[1] != 0x3B)
				||
				(pbPacket[2] != 0x34)
				||
				(pbPacket[3] != 0x32)
				||
				(pbPacket[4] < 0x10)
				) {
					Error("Error: Not Dyonics II shaver or old s/w, got 0x[%02x,%02x,%02x,%02x,%02x], retrying (Hit any key to quit)...\n",
						pbPacket[0],pbPacket[1],pbPacket[2],pbPacket[3],pbPacket[4]);
					Sleep(1000);
					if(_kbhit()) {
						noQuit = FALSE;
						goto L_END;
					}
					continue;
			}
			break;
		}
		Output("Dyonics II shaver detected, software version is 0x%02x\n",pbPacket[4]);

		bPort = 0x40;   // port A
		for(;;) {
			if(_kbhit()) {
				iChar = _getch();
				if(iChar == 0x1A) {
					break;
				} else {
					Output("INPUT: Got char '%c'\n",(char)iChar);
					switch((char)iChar) {
					case 'a':
					case 'A':
						bPort = 0x40;
						break;

					case 'b':
					case 'B':
						bPort = 0x80;
						break;

					case 'u':     // up (0x01)
						pbSRV_TRIG[4] = bPort | 0x01;
						pbSRV_TRIG[5] = pbSRV_TRIG[3] + pbSRV_TRIG[4];
						XmtPacket(pbSRV_TRIG,sizeof(pbSRV_TRIG));
						break;

					case 'd':     // down (0x02)
						pbSRV_TRIG[4] = bPort | 0x02;
						pbSRV_TRIG[5] = pbSRV_TRIG[3] + pbSRV_TRIG[4];
						XmtPacket(pbSRV_TRIG,sizeof(pbSRV_TRIG));
						break;

					case 'U':     // up 1000 (0x03)
						pbSRV_TRIG[4] = bPort | 0x03;
						pbSRV_TRIG[5] = pbSRV_TRIG[3] + pbSRV_TRIG[4];
						XmtPacket(pbSRV_TRIG,sizeof(pbSRV_TRIG));
						break;

					case 'D':     // down 1000 (0x04)
						pbSRV_TRIG[4] = bPort | 0x04;
						pbSRV_TRIG[5] = pbSRV_TRIG[3] + pbSRV_TRIG[4];
						XmtPacket(pbSRV_TRIG,sizeof(pbSRV_TRIG));
						break;

					case 'o':     // OK (0x00)
					case 'O':     // OK (0x00)
						pbSRV_TRIG[4] = 0x00;
						pbSRV_TRIG[5] = pbSRV_TRIG[3] + pbSRV_TRIG[4];
						XmtPacket(pbSRV_TRIG,sizeof(pbSRV_TRIG));
						break;

					case 'w':     // port warning
						pbSRV_TRIG[4] = bPort | 0x05;
						pbSRV_TRIG[5] = pbSRV_TRIG[3] + pbSRV_TRIG[4];
						XmtPacket(pbSRV_TRIG,sizeof(pbSRV_TRIG));
						break;

					case 'm':     // delta mode (0x06)
					case 'M':     // delta mode (0x06)
						pbSRV_TRIG[4] = bPort | 0x06;
						pbSRV_TRIG[5] = pbSRV_TRIG[3] + pbSRV_TRIG[4];
						XmtPacket(pbSRV_TRIG,sizeof(pbSRV_TRIG));
						break;

					default:
						break;
					}
					while(RcvPacket() && pbPacket[0] == 0x15) {
						XmtPacket(pbSRV_TRIG,sizeof(pbSRV_TRIG));
					}
				}
			}
			if(ySrvUpdDemand) {
				ySrvUpdDemand = FALSE;
				XmtPacket(pbSRV_UPD_DEMAND,sizeof(pbSRV_UPD_DEMAND));
			} else {
				XmtPacket(pbSRV_UPD,sizeof(pbSRV_UPD));
			}
			if(RcvPacket() && pbPacket[0] != 0x15) {
				InterpretStatus();
			}
		}
	}
L_END:
    CloseHandle(hCondorSerial);
	DeleteCriticalSection(&xmtCriticalSection);

    Output("-----------------------------------------------\n");
    Output("Closing application\n");
    Output("-----------------------------------------------\n");

    if(ptLog) {
        fclose(ptLog);
        ptLog = NULL;
    }

	return 0;
}
