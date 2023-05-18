#include <stdafx.h>
#include "SnTypes.h"
#include "cserialport.h"

//------------------------------------------------------------------------------------------------

#if CSERIALPORT_LOG_BYTES
typedef enum {
	WR_U,
	WR_W,
	WR_R
} WR;

typedef struct WRBuf {
	WR eWR;
	SnByte b;
	int iTime;
	SnBool yRet;
	int iTrace;
} WRBuf;

WRBuf ptWRBuf[32];
WRBuf* ptWR = ptWRBuf;
HANDLE hMutex;

void Log(WR eWR,SnByte bVal,SnBool yRet,int iTrace)
{
	SYSTEMTIME tSystemTime;

	WaitForSingleObject(hMutex,1);
	if(ptWR == &ptWRBuf[32]) {
		ptWR = ptWRBuf;
	}
	ptWR->eWR = eWR;
	ptWR->b = bVal;
	GetSystemTime(&tSystemTime);
	ptWR->iTime = tSystemTime.wSecond * 1000 + tSystemTime.wMilliseconds;
	ptWR->yRet = yRet;
	ptWR++->iTrace = iTrace;
	ReleaseMutex(hMutex);
}
#endif

// FALSE means failure
SnBool CSerialPort::Open(LPCTSTR lpComPort, DWORD BaudRate)
{
	DWORD dwError;
	DCB tDCB;
	COMMTIMEOUTS tCommTimeouts;

	m_hSerialPortIn = CreateFile(
					lpComPort,					    //LPCTSTR lpFileName
					GENERIC_READ,					//DWORD dwDesiredAccess
					FILE_SHARE_WRITE,				//DWORD dwShareMode
					NULL,							//LPSECURITY_ATTRIBUTES lpSecurityAttributes
					OPEN_EXISTING,					//DWORD dwCreationDisposition
					0,								//DWORD dwFlagsAndAttributes,
					NULL							//HANDLE hTemplateFile
					);
	if(m_hSerialPortIn == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	m_hSerialPortOut = CreateFile(
					lpComPort,					    //LPCTSTR lpFileName
					GENERIC_WRITE,					//DWORD dwDesiredAccess
					0,								//DWORD dwShareMode
					NULL,							//LPSECURITY_ATTRIBUTES lpSecurityAttributes
					OPEN_EXISTING,					//DWORD dwCreationDisposition
					0,								//DWORD dwFlagsAndAttributes,
					NULL							//HANDLE hTemplateFile
					);
	if(m_hSerialPortOut == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	if(!GetCommState(m_hSerialPortIn,&tDCB)) {
		// Failure
		return FALSE;
	}


	tDCB.DCBlength = sizeof(DCB);
	tDCB.BaudRate = BaudRate;					
	tDCB.fBinary = TRUE;
	tDCB.fParity = FALSE;
	tDCB.fOutxCtsFlow = FALSE;
	tDCB.fOutxDsrFlow = FALSE;
	tDCB.fDtrControl = DTR_CONTROL_DISABLE;
	tDCB.fDsrSensitivity = FALSE;
	tDCB.fTXContinueOnXoff = FALSE;	// Check
	tDCB.fOutX = FALSE;
	tDCB.fInX = FALSE;
	tDCB.fErrorChar = FALSE;
	tDCB.fNull = FALSE;
	tDCB.fRtsControl = RTS_CONTROL_DISABLE;
	tDCB.fAbortOnError = FALSE;
	//tDCB.fDummy2 = do-not-use;
	tDCB.wReserved = 0;
	tDCB.XonLim = 0;
	tDCB.XoffLim = 0;
	tDCB.ByteSize = 8;
	tDCB.Parity = NOPARITY;
	tDCB.StopBits = ONESTOPBIT;
	tDCB.XonChar = 0;
	tDCB.XoffChar = 1;	// Must be different from XonChar for SetCommState
	tDCB.ErrorChar = 0;
	tDCB.EofChar = 0;
	tDCB.EvtChar = 0;
	//tDCB.wReserved1 = do-not-use;

	if(!SetCommState(m_hSerialPortIn,&tDCB)) {
		dwError = GetLastError();
		return FALSE;
	}

	if(!SetCommState(m_hSerialPortOut,&tDCB)) {
		dwError = GetLastError();
		return FALSE;
	}

	if(!GetCommTimeouts(m_hSerialPortIn,&tCommTimeouts)) {
		// Failure
		return FALSE;
	}
	// Timeouts for both reads and writes
	tCommTimeouts.ReadIntervalTimeout = 0;
	tCommTimeouts.ReadTotalTimeoutMultiplier = 100;
	tCommTimeouts.ReadTotalTimeoutConstant = 0;
	tCommTimeouts.WriteTotalTimeoutMultiplier = 0;
	tCommTimeouts.WriteTotalTimeoutConstant = 0;
	if(!SetCommTimeouts(m_hSerialPortIn,&tCommTimeouts)) {
		// Failure
		return FALSE;
	}

	if(!SetCommTimeouts(m_hSerialPortOut,&tCommTimeouts)) {
		// Failure
		return FALSE;
	}

	// Setup in and out buffer sizes
	if(!SetupComm(m_hSerialPortIn,12,6)) {
		// Failure
		return FALSE;
	}

	if(!SetupComm(m_hSerialPortOut,12,6)) {
		// Failure
		return FALSE;
	}

	PurgeComm( m_hSerialPortIn, PURGE_RXCLEAR | PURGE_RXABORT );
	PurgeComm( m_hSerialPortOut, PURGE_TXCLEAR | PURGE_TXABORT );


#if CSERIALPORT_LOG_BYTES
	hMutex = CreateMutex(NULL,FALSE,NULL);
	for(int i = 0; i < 32; i++) {
		ptWRBuf[i].eWR = WR_U;
	}
#endif

	return TRUE;
}

SnBool CSerialPort::Close(void)
{
	SnBool result = TRUE;
	if(m_hSerialPortIn != INVALID_HANDLE_VALUE) {
		if(!CloseHandle(m_hSerialPortIn))
			result = FALSE;
		else
			m_hSerialPortIn = INVALID_HANDLE_VALUE;
	}

	if(m_hSerialPortOut != INVALID_HANDLE_VALUE) {
		if(!CloseHandle(m_hSerialPortOut))
			result = FALSE;
		else
			m_hSerialPortOut = INVALID_HANDLE_VALUE;
	}

#if CSERIALPORT_LOG_BYTES
	if( hMutex )
	{
		if(!CloseHandle(hMutex))
			result = FALSE;
		else
			hMutex = INVALID_HANDLE_VALUE;
	}
#endif
	
	return result;
}

CSerialPort::~CSerialPort(void)
{
	Close();
}

SnBool CSerialPort::Write(SnByte * bVal, SnByte bSize
#if CSERIALPORT_LOG_BYTES
						  ,int iTrace
#endif
						  )
{
	DWORD dwNumBytesWritten;
	SnBool yRet;

	yRet = WriteFile(m_hSerialPortOut,bVal,bSize,&dwNumBytesWritten,NULL) && (dwNumBytesWritten == bSize);
	if (!yRet)
		FlushFileBuffers (m_hSerialPortOut);

#if CSERIALPORT_LOG_BYTES
	DWORD ii = 0;
	do
	{
		Log(WR_W,bVal[ii],yRet,iTrace);
		ii++; 
	}while (ii < dwNumBytesWritten);
#endif

	return yRet;
}

SnBool CSerialPort::Write(SnByte bVal
#if CSERIALPORT_LOG_BYTES
						  ,int iTrace
#endif
						  )
{
	DWORD dwNumBytesWritten;
	BYTE b = (BYTE)bVal;
	SnBool yRet;

	yRet = WriteFile(m_hSerialPortOut,&b,1,&dwNumBytesWritten,NULL) && (dwNumBytesWritten == 1);
	if (!yRet)
		FlushFileBuffers (m_hSerialPortOut);

#if CSERIALPORT_LOG_BYTES
	Log(WR_W,bVal,yRet,iTrace);
#endif

	return yRet;
}

SnBool CSerialPort::Read(SnByte* pbVal
#if CSERIALPORT_LOG_BYTES
						 ,int iTrace
#endif
						 )
{
	DWORD dwNumBytesRead;
	BYTE b;
	SnBool yRet;

	yRet = ReadFile(m_hSerialPortIn,&b,1,&dwNumBytesRead,NULL) && (dwNumBytesRead == 1);
	*pbVal = (SnByte)b;

#if CSERIALPORT_LOG_BYTES
	Log(WR_R,*pbVal,yRet,iTrace);
#endif

	return yRet;
}
