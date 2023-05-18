// Logger.cpp: implementation of the CLogger class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SnTypes.h"
#include "Logger.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogger::CLogger()
{
    m_yInitialized = FALSE;
    m_pcDataBuf = NULL;
    m_pcDataBufPtr = NULL;
    m_qDataMaxSize = 0;
    m_qDataCurSize = 0;
}

CLogger::~CLogger()
{
    DeInitLogger();
}

SnBool CLogger::InitLogger(SnQByte qDataMaxSize)
{
    m_qDataMaxSize = qDataMaxSize;
    m_qDataCurSize = 0;
    m_pcDataBuf = (char *)malloc(m_qDataMaxSize);
    m_pcDataBufPtr = m_pcDataBuf;

    if (m_pcDataBuf) {
        m_yInitialized = TRUE;
    } else {
        DeInitLogger();
        return FALSE;
    }

    return TRUE;
}

void CLogger::DeInitLogger(void)
{
    m_yInitialized = FALSE;

    if (m_pcDataBuf) {
        free(m_pcDataBuf);
        m_pcDataBuf = m_pcDataBufPtr = NULL;
    }
    m_qDataMaxSize = 0;
    m_qDataCurSize = 0;
}

SnBool CLogger::AddString(const char *pcString)
{
    SnQByte qLength = strlen(pcString);

    if (m_qDataCurSize + qLength > m_qDataMaxSize) {
        return FALSE;
    }

    memcpy(m_pcDataBufPtr, pcString, qLength);
    m_qDataCurSize += qLength;
    m_pcDataBufPtr += qLength;

    return TRUE;
}

SnBool CLogger::SaveData(const TCHAR *pwFileName)
{
    HANDLE hDataCollectionFile;
	DWORD dwBytesWritten;
    SnBool yRet = TRUE;
	
    if (m_pcDataBuf) {
        // First Open the file
	    hDataCollectionFile = CreateFile( pwFileName,                			// File name and path
										    GENERIC_READ | GENERIC_WRITE,		// dwDesiredAccess
										    FILE_SHARE_READ | FILE_SHARE_WRITE,	// dwShareMode
										    NULL,								// lpSecurityAttributes
										    CREATE_ALWAYS,						// dwCreationDistribution
										    FILE_ATTRIBUTE_NORMAL,				// dwFileAndAttributes
										    NULL);								// hTemplatefile
	
	    if (hDataCollectionFile == INVALID_HANDLE_VALUE) {
            yRet = FALSE;
	    }

        // Write buf to file
	    WriteFile(hDataCollectionFile,      // Handle to the file to be written to
			      m_pcDataBuf,	            // Pointer to buffer containing data
			      m_qDataCurSize,	        // Number of bytes to write
			      &dwBytesWritten,			// Pointer to number of bytes written
			      NULL);					// Unsupported; set to NULL

        if (dwBytesWritten != m_qDataCurSize) {
            yRet = FALSE;
        }

        // Set the EOF  marker
	    SetEndOfFile( hDataCollectionFile);

	    // Close the file Handle
        CloseHandle( hDataCollectionFile);	

        m_qDataCurSize = 0;
        m_pcDataBufPtr = m_pcDataBuf;
    }

    return yRet;
}
