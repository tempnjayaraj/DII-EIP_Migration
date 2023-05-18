// Driver.cpp: implementation of the CDriver class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SnTypes.h"
#include "SnIoctl.h"
#include "Driver.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDriver::CDriver()
{
    m_hDriverDev = INVALID_HANDLE_VALUE;
    m_hFlashAccess = NULL;
}

CDriver::~CDriver()
{
	DeInitDriver();
}

SnBool CDriver::InitDriver(void)
{
	if (m_hDriverDev == INVALID_HANDLE_VALUE) {
        m_hDriverDev = CreateFile(TEXT("SNE1:"), GENERIC_READ|GENERIC_WRITE,
							   0,
							   NULL,
							   OPEN_EXISTING,
							   0,
							   NULL);

        if (m_hDriverDev == INVALID_HANDLE_VALUE) {
            return FALSE;
        }

        m_hFlashAccess = CreateMutex(NULL, FALSE, _T("SnDriverFlashAccess"));
	    if (m_hFlashAccess == NULL) {
			DeInitDriver();
			return FALSE;
        }
    }

	return TRUE;
}

void CDriver::DeInitDriver(void)
{
	// Close Device handle
	if(m_hDriverDev != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hDriverDev);
		m_hDriverDev = INVALID_HANDLE_VALUE;
	}
   
    if(m_hFlashAccess != NULL) {
		CloseHandle(m_hFlashAccess);
        m_hFlashAccess = NULL;
    }

    if(m_hNewCmd != NULL) {
		CloseHandle(m_hNewCmd);
        m_hNewCmd = NULL;
    }
}

SnBool CDriver::ReadNvRam(void *pxData, SnQByte qOffset, SnQByte qLen, SnQByte *pqBytesRead)
{
    if (SetFilePointer(m_hDriverDev, qOffset, NULL, FILE_BEGIN) != qOffset) {
        *pqBytesRead = 0;
        return FALSE;
    }
    return ReadFile(m_hDriverDev, pxData, qLen, pqBytesRead, NULL);
}

SnBool CDriver::WriteNvRam(void *pxData, SnQByte qOffset, SnQByte qLen, SnQByte *pqBytesWritten)
{
    if (SetFilePointer(m_hDriverDev, qOffset, NULL, FILE_BEGIN) != qOffset) {
        *pqBytesWritten = 0;
        return FALSE;
    }
    return WriteFile(m_hDriverDev, pxData, qLen, pqBytesWritten, NULL);
}

//
// Message Driver Functions
//

SnBool CDriver::ReadWordFromDevice(SnQByte qOffset, SnWord *pwData)
{
    DWORD dwBytesReturned;
    SnWord wReadWordCmd;
    SnBool yRet;

    if (qOffset > 4095) {
        return FALSE;
    }

    wReadWordCmd = (SnWord)((CMD_RD_VAR_WORD << 12) | qOffset);

    yRet = DeviceIoControl(m_hDriverDev, IOCTL_SND_CMD, &wReadWordCmd, sizeof(SnWord),
            pwData, sizeof(SnWord), &dwBytesReturned, NULL);

    if (yRet == FALSE || dwBytesReturned != sizeof(SnWord)) {
        return FALSE;
    }

    return TRUE;
}

SnBool CDriver::WriteWordToDevice(SnQByte qOffset, SnWord wData)
{
    SnWord pwWriteWordCmd[2];
    SnBool yRet;

    if (qOffset > 4095) {
        return FALSE;
    }

    pwWriteWordCmd[0] = (SnWord)((CMD_WR_VAR_WORD << 12) | qOffset);
    pwWriteWordCmd[1] = wData;

    yRet = DeviceIoControl(m_hDriverDev, IOCTL_SND_CMD, pwWriteWordCmd, sizeof(SnWord) * 2,
            NULL, 0, NULL, NULL);

    return yRet;
}

SnBool CDriver::ReadWordsFromDevice(SnQByte qOffset, SnQByte qNumWords, SnWord *pwData)
{
    DWORD dwBytesReturned;
    SnWord pwReadWordsCmd[2];
    SnBool yRet;

    if (qNumWords == 0 || (qOffset + qNumWords) > 4095 || qNumWords > 512) {
        return FALSE;
    }

    pwReadWordsCmd[0] = (SnWord)((CMD_RD_VAR_WORDS << 12) | qNumWords);
    pwReadWordsCmd[1] = (SnWord)qOffset;

    yRet = DeviceIoControl(m_hDriverDev, IOCTL_SND_CMD, pwReadWordsCmd, sizeof(SnWord) * 2,
            pwData, qNumWords * sizeof(SnWord), &dwBytesReturned, NULL);

    if (yRet == FALSE || dwBytesReturned != (qNumWords * sizeof(SnWord))) {
        return FALSE;
    }

    return TRUE;
}

SnBool CDriver::WriteWordsToDevice(SnQByte qOffset, SnQByte qNumWords, SnWord *pwData)
{
    SnQByte qBytesToWrite = qNumWords * sizeof(SnWord);
    SnWord pwWriteWordsCmd[514];
    SnBool yRet;

    if (qNumWords == 0 || (qOffset + qNumWords) > 4095 || qNumWords > 512) {
        return FALSE;
    }

    pwWriteWordsCmd[0] = (SnWord)((CMD_WR_VAR_WORDS << 12) | qNumWords);
    pwWriteWordsCmd[1] = (SnWord)qOffset;
    memcpy(&pwWriteWordsCmd[2], pwData, qBytesToWrite);

    yRet = DeviceIoControl(m_hDriverDev, IOCTL_SND_CMD, pwWriteWordsCmd,
        qBytesToWrite + (2 * sizeof(SnWord)), NULL, 0, NULL, NULL);

    return yRet;
}

SnBool CDriver::SerialCmdsToDevice(SnQByte qOffset, SnQByte qNumCmds, SnWord *pwCmds, SnWord wTimeout)
{
    SnQByte qCmdListBytes = qNumCmds * sizeof(SnWord);
    SnWord pwSerialCmds[20];
    SnBool yRet;

    if (qNumCmds == 0 || qNumCmds > 15 || wTimeout == 0 || wTimeout > 16) {
        return FALSE;
    }

    pwSerialCmds[0] = (SnWord)((CMD_SERIAL << 12) | ((wTimeout-1) << 8) | qNumCmds);
    memcpy(&pwSerialCmds[1], pwCmds, qCmdListBytes);

    yRet = DeviceIoControl(m_hDriverDev, IOCTL_SND_CMD, pwSerialCmds,
        qCmdListBytes + sizeof(SnWord), NULL, 0, NULL, NULL);

    if (yRet) {
        yRet = ReadWordsFromDevice(qOffset, qNumCmds, pwCmds);
    }

    return yRet;
}

SnBool CDriver::SerialPageToDevice(SnQByte qPageCmdSize, SnByte *pbPageCmd)
{
    SnWord pwSerialPageCmd[70];
    SnBool yRet;

    if ((qPageCmdSize & 1)) {
        return FALSE;
    }

    pwSerialPageCmd[0] = (SnWord)((CMD_SERIAL << 12) | (qPageCmdSize >> 1));
    memcpy(&pwSerialPageCmd[1], pbPageCmd, qPageCmdSize);

    yRet = DeviceIoControl(m_hDriverDev, IOCTL_SND_CMD, pwSerialPageCmd,
        qPageCmdSize + sizeof(SnWord), NULL, 0, NULL, NULL);

    return yRet;
}

SnBool CDriver::FlashBlockToDevice(SnByte bPage, SnWord* pwData)
{
    SnWord pwFlashBlockCmd[513];

    if (bPage >= 128) {
        return FALSE;
    }
    pwFlashBlockCmd[0] = (SnWord)((CMD_WR_FLASH_BLK << 12) | bPage);
    memcpy(&pwFlashBlockCmd[1], pwData, 512 * sizeof(SnWord));

    return DeviceIoControl(m_hDriverDev, IOCTL_SND_CMD, pwFlashBlockCmd,
        (512 * sizeof(SnWord)) + sizeof(SnWord), NULL, 0, NULL, NULL);
}

//
// FLASH Driver Functions
//

SnBool CDriver::ReadFlashData(SnQByte qAddr, SnByte *pbData, SnQByte qBytes)
{
    DWORD dwBytesReturned;
    SnBool yRet;

    WaitForSingleObject(m_hFlashAccess, INFINITE);
    yRet = DeviceIoControl(m_hDriverDev, IOCTL_SET_FLASH_OFFS, &qAddr, sizeof(SnQByte),
            NULL, 0, NULL, NULL);
    if (yRet) {
        yRet = DeviceIoControl(m_hDriverDev, IOCTL_READ_FLASH, NULL, 0,
                pbData, qBytes, &dwBytesReturned, NULL);
    }
    ReleaseMutex(m_hFlashAccess);

	return yRet;
}

SnBool CDriver::EraseFlashPages(SnQByte qAddr, SnQByte qPages)
{
    SnBool yRet;

    WaitForSingleObject(m_hFlashAccess, INFINITE);
    yRet = DeviceIoControl(m_hDriverDev, IOCTL_SET_FLASH_OFFS, &qAddr, sizeof(SnQByte),
            NULL, 0, NULL, NULL);
    if (yRet) {
        yRet = DeviceIoControl(m_hDriverDev, IOCTL_ERASE_FLASH, &qPages, sizeof(SnQByte),
                NULL, 0, NULL, NULL);
    }
    ReleaseMutex(m_hFlashAccess);

	return yRet;
}

SnBool CDriver::WriteFlashData(SnQByte qAddr, SnByte *pbData, SnQByte qBytes)
{
    SnBool yRet;

    WaitForSingleObject(m_hFlashAccess, INFINITE);
    yRet = DeviceIoControl(m_hDriverDev, IOCTL_SET_FLASH_OFFS, &qAddr, sizeof(SnQByte),
            NULL, 0, NULL, NULL);
    if (yRet) {
        yRet = DeviceIoControl(m_hDriverDev, IOCTL_WRITE_FLASH, pbData, qBytes,
                NULL, 0, NULL, NULL);
    }
    ReleaseMutex(m_hFlashAccess);

	return yRet;
}

SnBool CDriver::VerifyFlashData(SnQByte qAddr, SnByte *pbData, SnQByte qBytes)
{
    SnBool yRet;

    WaitForSingleObject(m_hFlashAccess, INFINITE);
    yRet = DeviceIoControl(m_hDriverDev, IOCTL_SET_FLASH_OFFS, &qAddr, sizeof(SnQByte),
            NULL, 0, NULL, NULL);
    if (yRet) {
        yRet = DeviceIoControl(m_hDriverDev, IOCTL_VERIFY_FLASH, pbData, qBytes,
                NULL, 0, NULL, NULL);
    }
    ReleaseMutex(m_hFlashAccess);

	return yRet;
}

SnBool CDriver::CrcBufData(SnByte *pbData, SnQByte qBytes, SnByte *pbCrc)
{
    static const SnByte s_pbCrcTable[] = 
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

    SnByte bCrc = (pbCrc) ? *pbCrc : 0;

    while (qBytes-- > 0) {
        bCrc = s_pbCrcTable[bCrc ^ *pbData++];
    }

    if (pbCrc) {
        *pbCrc = bCrc;
    }

    return TRUE;
}

SnBool CDriver::ReadFlashStore(SnAddr aFlashStoreBase, SnAddr aFlashStoreTop, SnQByte qMaxStoreBufSize,
                               SnByte *pbLastStore, SnQByte *pqLastStoreSize)
{
    SnFlashStore tFlashStore;
    SnBool yRet;

    tFlashStore.aFlashStoreBase = aFlashStoreBase;
    tFlashStore.aFlashStoreTop = aFlashStoreTop;
    tFlashStore.qMaxStoreBufSize = qMaxStoreBufSize;

    WaitForSingleObject(m_hFlashAccess, INFINITE);
    yRet = DeviceIoControl(m_hDriverDev, IOCTL_READ_FLASH_STORE, &tFlashStore, sizeof(SnFlashStore),
            pbLastStore, qMaxStoreBufSize, pqLastStoreSize, NULL);
    ReleaseMutex(m_hFlashAccess);

    return yRet;
}

SnBool CDriver::WriteFlashStore(SnByte *pbStoreData, SnQByte qStoreSize)
{
    SnBool yRet;

    WaitForSingleObject(m_hFlashAccess, INFINITE);
    yRet = DeviceIoControl(m_hDriverDev, IOCTL_WRITE_FLASH_STORE, pbStoreData, qStoreSize,
            NULL, 0, NULL, NULL);
    ReleaseMutex(m_hFlashAccess);

    return yRet;
}

SnBool CDriver::EraseFlashStore(void)
{
    SnBool yRet;

    WaitForSingleObject(m_hFlashAccess, INFINITE);
    yRet = DeviceIoControl(m_hDriverDev, IOCTL_ERASE_FLASH_STORE, NULL, 0,
            NULL, 0, NULL, NULL);
    ReleaseMutex(m_hFlashAccess);

    return yRet;
}

//
// Misc Driver Functions
//

SnBool CDriver::ResetDisplayBase(void)
{
    return DeviceIoControl(m_hDriverDev, IOCTL_RESET_DISP_BASE, NULL, 0,
							        NULL, 0, NULL, NULL);
}

SnBool CDriver::SetSystemBuzzer(SnBool yEnable)
{
    return DeviceIoControl(m_hDriverDev, IOCTL_SET_BUZZER,
           &yEnable, sizeof(SnBool), NULL, 0, NULL, NULL);
}

SnBool CDriver::ResetDsp(void)
{
    return DeviceIoControl(m_hDriverDev, IOCTL_RESET_DSP, NULL, 0,
							        NULL, 0, NULL, NULL);
}

SnBool CDriver::CheckNvRamBattery(void)
{
    DWORD dwBytesReturned;
    SnBool yBatteryOK = FALSE;
    SnBool yRet;
    
    yRet = DeviceIoControl(m_hDriverDev, IOCTL_CHECK_NVRAM_BATTERY, NULL, 0,
							        &yBatteryOK, sizeof(SnBool), &dwBytesReturned, NULL);
    if (yRet && dwBytesReturned == sizeof(SnBool) && yBatteryOK) {
        return TRUE;
    }

    return FALSE;
}

SnBool CDriver::CanTest(void)
{
    DWORD dwBytesReturned;
    SnBool yCanTestOK = FALSE;
    SnBool yRet;
    
    yRet = DeviceIoControl(m_hDriverDev, IOCTL_CAN_TEST, NULL, 0,
							        &yCanTestOK, sizeof(SnBool), &dwBytesReturned, NULL);
    if (yRet && dwBytesReturned == sizeof(SnBool) && yCanTestOK) {
        return TRUE;
    }

    return FALSE;
}

SnBool CDriver::CheckBootStatus(SnQByte *pqBootStatus)
{
    DWORD dwBytesReturned;
    SnBool yRet;
    
    yRet = DeviceIoControl(m_hDriverDev, IOCTL_CHECK_BOOT, NULL, 0,
							        pqBootStatus, sizeof(SnBool), &dwBytesReturned, NULL);
    if (yRet && dwBytesReturned == sizeof(SnQByte)) {
        return TRUE;
    }

    return FALSE;
}

