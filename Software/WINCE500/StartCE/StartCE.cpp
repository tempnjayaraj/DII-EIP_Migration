//
// StartCE.cpp : Defines the entry point for the console application.
//
#include <windows.h> 
#include <winsock.h>

#define PORTNUM               5000      // Port number  
#define MAX_PENDING_CONNECTS  4         // Maximum length of the queue 
					                    // of pending connections
typedef struct {
    unsigned long ulCmd;
    WCHAR pszName[64];
    union {
	    struct {
	        WCHAR pszCmdLine[256];
	    } Exec;
	    struct {
	        unsigned long ulTotalSize;
	        unsigned long ulXfrSize;
	    } Copy;
    } CMD;
} CmdPacket;

#define CMD_ACK		0x42
#define CMD_DONE	0x55
#define CMD_EXEC	0x00000001
#define CMD_COPY	0x00000002
#define CMD_LOAD	0x00000003

#if 0
/*
 * NKDbgPrintfW
 * 
 *   Function to printf for debug
 *
 * Return Value:
 */
void WINAPIV NKDbgPrintfW(LPCWSTR lpszFmt, ...) 
 
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

    static TCHAR  szDebugStr[1024] = TEXT("");
    static char   szAnsiString[2048];
    static HANDLE hComPort = INVALID_HANDLE_VALUE;
    DWORD dwRet;

    int           nLength;
    BOOL          yDefaultUsed;

	va_list ap;
    va_start(ap, lpszFmt);    
    
    if (hComPort == INVALID_HANDLE_VALUE) {
        hComPort = CreateFile(TEXT("COM4:"), GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, 0, NULL);

        if (hComPort == INVALID_HANDLE_VALUE ||
          SetCommState(hComPort, &tDCB) == FALSE ||
          SetCommTimeouts(hComPort, &tCommTimeouts) == FALSE) {
            return;
        }
    }

    wvsprintf(&szDebugStr[0], lpszFmt, ap);
    nLength = WideCharToMultiByte(
            CP_ACP, WC_COMPOSITECHECK | WC_DEFAULTCHAR,
            szDebugStr, _tcslen(szDebugStr), szAnsiString, sizeof(szAnsiString),
            "?", &yDefaultUsed);

	WriteFile(hComPort, szAnsiString, nLength, &dwRet, NULL );
	va_end (ap);
}
#endif

int ReceiveData(SOCKET ClientSock, unsigned char *pbData, DWORD qSize, DWORD qBlock)
{
    int iBytes;
    char cAck = CMD_ACK;
    char cDone = CMD_DONE;

    do {
        iBytes = recv (ClientSock, (char *)pbData, (qBlock > qSize) ? qSize : qBlock, 0);

        if (iBytes == SOCKET_ERROR) {
	        NKDbgPrintfW (TEXT("Error, no data is received, recv failed: %d\n\r"),
	        WSAGetLastError ());
	        return -1;
        } else if (iBytes == 0) {
	        continue;
        } else if (iBytes < 0 || (DWORD)iBytes > qSize) {
	        NKDbgPrintfW(TEXT("Error, iBytes: %d\n\r"), iBytes);
	        return -1;
        }

        //	NKDbgPrintfW(TEXT("ReceiveData() Read %d bytes of data\n"), iBytes);

        // Send an ACK back
        if (send (ClientSock, &cAck, 1, 0) == SOCKET_ERROR) {
	        NKDbgPrintfW (TEXT("Sending data to the client failed. Error: %d\n\r"),
		          WSAGetLastError ());
	        return -1;
        }

        //	NKDbgPrintfW(TEXT("ReceiveData() Sent ACK to client\n"));

        qSize -= iBytes;
        pbData += iBytes;

    } while (qSize);

    // Send an DONE back
    if (send (ClientSock, &cDone, 1, 0) == SOCKET_ERROR) {
	    NKDbgPrintfW (TEXT("Sending data to the client failed. Error: %d\n\r"),
		      WSAGetLastError ());
	    return -1;
    }

//    NKDbgPrintfW(TEXT("ReceiveData() Sent Done to client\n"));

    return 0;
}

int StartServer()
{
    int iReturn;                        // Return value of recv function
    CmdPacket tPkt;			            // Exe Name and Cmd Line Strings

    SOCKET WinSocket = INVALID_SOCKET,  // Window socket
    ClientSock = INVALID_SOCKET;        // Socket for communicating 
				                        // between the server and client
    SOCKADDR_IN local_sin,              // Local socket address
	            accept_sin;             // Receives the address of the 
				                        // connecting entity
    int accept_sin_len;                 // Length of accept_sin

    WSADATA WSAData;                    // Contains details of the Winsock
				                        // implementation
    HANDLE hServerThread = GetCurrentThread();
    int iOldThreadPriority = CeGetThreadPriority(hServerThread);

    CeSetThreadPriority(hServerThread, 150);

    NKDbgPrintfW(TEXT("Starting Exec Server...\n\r"));

    // Initialize Winsock.
    if (WSAStartup (MAKEWORD(1,1), &WSAData) != 0) {
	    NKDbgPrintfW (TEXT("WSAStartup failed. Error: %d\n\r"),
	        WSAGetLastError ());
	    return FALSE;
    }

    // Create a TCP/IP socket, WinSocket.
    if ((WinSocket = socket (AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
	    NKDbgPrintfW (TEXT("Allocating socket failed. Error: %d\n\r"),
	        WSAGetLastError ());
	    return FALSE;
    }

    // Fill out the local socket's address information.
    local_sin.sin_family = AF_INET;
    local_sin.sin_port = htons (PORTNUM);  
    local_sin.sin_addr.s_addr = htonl (INADDR_ANY);

    // Associate the local address with WinSocket.
    if (bind (WinSocket, 
	    (struct sockaddr *) &local_sin, 
	    sizeof (local_sin)) == SOCKET_ERROR) {
	    NKDbgPrintfW (TEXT("Binding socket failed. Error: %d\n\r"), 
	        WSAGetLastError ());
	    closesocket (WinSocket);
	    return FALSE;
    }

    // Establish a socket to listen for incoming connections.
    if (listen (WinSocket, MAX_PENDING_CONNECTS) == SOCKET_ERROR) {
	    NKDbgPrintfW (TEXT("Listening to the client failed. Error: %d\n\r"),
	        WSAGetLastError ());
	    closesocket (WinSocket);
	    return FALSE;
    }

    accept_sin_len = sizeof (accept_sin);

    for (;;) {
	    ClientSock = accept (WinSocket, 
			       (struct sockaddr *) &accept_sin, 
			       (int *) &accept_sin_len);

	    if (ClientSock == INVALID_SOCKET)  {
	        NKDbgPrintfW (TEXT("Error accepting connection with client failed: %d\n\r"),
		    WSAGetLastError ());
	        continue;
	    }

	    iReturn = ReceiveData(ClientSock, (unsigned char *)&tPkt, sizeof (CmdPacket), sizeof(CmdPacket));
	    if (iReturn < 0) {
	        continue;
	    }
	    
	    if (tPkt.ulCmd == CMD_EXEC) {
	        NKDbgPrintfW(TEXT("Exec: '%s' with args '%s'...\n\r"), tPkt.pszName,
		    tPkt.CMD.Exec.pszCmdLine);

	        CeSetThreadPriority(hServerThread, iOldThreadPriority);
	        if (CreateProcess(tPkt.pszName, tPkt.CMD.Exec.pszCmdLine,
		        NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL) == 0) {
		        NKDbgPrintfW(TEXT("Exec failed.\n"));
	        }
	        CeSetThreadPriority(hServerThread, 150);
	    } else if (tPkt.ulCmd == CMD_COPY) {
	        unsigned long ulSize = tPkt.CMD.Copy.ulTotalSize;
	        unsigned long ulBlock = tPkt.CMD.Copy.ulXfrSize;
	        HANDLE oFile;
	        DWORD dwBytes;
	        void *pxData;

	        NKDbgPrintfW(TEXT("Copy: '%s' TotalSize: %d...\n\r"), tPkt.pszName,
		    ulSize);

	        oFile = CreateFile(tPkt.pszName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		    FILE_ATTRIBUTE_NORMAL, NULL);

	        pxData = malloc(ulSize);
	        if (!pxData) {
		        NKDbgPrintfW(TEXT("Unable to allocate %d bytes of data for Xfr\n\r"), ulSize, ulBlock);
		        CloseHandle(oFile);
		        continue;
	        }

	        iReturn = ReceiveData(ClientSock, (unsigned char *)pxData, ulSize, ulBlock);
	        if (iReturn < 0) {
		        CloseHandle(oFile);
		        free(pxData);
		        continue;
	        }

	        // Write it out to the file
	        WriteFile(oFile, pxData, ulSize, &dwBytes, NULL);
	        CloseHandle(oFile);
	        free(pxData);
	    } else if (tPkt.ulCmd == CMD_LOAD) {
	        HMODULE hLib;
	        int (*pRegisterFunc)();

	        hLib = LoadLibrary(tPkt.pszName);
	        if (hLib == NULL) {
		        NKDbgPrintfW(TEXT("Can't load library '%s'\n\r"), tPkt.pszName);
	        } else {
		        NKDbgPrintfW(TEXT("Loaded library '%s'\n\r"), tPkt.pszName);
	        }
	        pRegisterFunc = GetProcAddress(hLib, TEXT("DllRegisterServer"));
	        if (pRegisterFunc == NULL) {
		        NKDbgPrintfW(TEXT("Can't find DllRegisterServer() in '%s'\n\r"), tPkt.pszName);
	        } else {
		        NKDbgPrintfW(TEXT("Found DllRegisterServer() in '%s'\n\r"), tPkt.pszName);
		        (*pRegisterFunc)();
	        }
	        FreeLibrary(hLib);
	    } else {
	        NKDbgPrintfW(TEXT("Invalid Cmd: 0x%x.\n\r"), tPkt.ulCmd);
	    }
	    shutdown (ClientSock, 0x02);
	    closesocket (ClientSock);
    }

    WSACleanup ();

    return TRUE;
}

int main(int argc, char *argv[], char *envp[])
{
    return StartServer();
}
