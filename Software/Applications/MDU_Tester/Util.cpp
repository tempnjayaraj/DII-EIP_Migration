// Util.cpp : implementation file
//

#include "stdafx.h"
#include "SnTypes.h"
#include "SoftwareUpgrade.h"
#include "SnIoctl.h"
#include "Driver.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

SnSQByte			CUtil::m_sqCreationCount = 0;
HDC                 CUtil::m_hDisplayHDC = NULL;
CDC *               CUtil::m_hDisplayCDC;
CDC                 CUtil::m_hMemoryCDC;
CRITICAL_SECTION    CUtil::m_DisplayCs;

/////////////////////////////////////////////////////////////////////////////
// Util

CUtil::CUtil()
{
    if (m_sqCreationCount == 0)
    {
	    InitializeCriticalSection(&m_DisplayCs);
        m_hDisplayHDC = ::GetDC(NULL);
		m_hDisplayCDC = CDC::FromHandle(m_hDisplayHDC);
	    m_hMemoryCDC.CreateCompatibleDC(m_hDisplayCDC);
    }

	m_sqCreationCount++;
}

CUtil::~CUtil()
{
}

SnByte CUtil::CrcMemChunk(SnByte* pbSrc, SnQByte qLen, SnByte bCrc)
{
    SnQByte *pqSrc = (SnQByte *)pbSrc;
    SnQByte qData;

	if (((SnAddr)pbSrc & 3) == 0) {
		do {
			qData = *pqSrc++;
			bCrc = pbCrcTable[bCrc ^ (qData & 0xFF)];
			bCrc = pbCrcTable[bCrc ^ ((qData >> 8) & 0xFF)];
			bCrc = pbCrcTable[bCrc ^ ((qData >> 16) & 0xFF)];
			bCrc = pbCrcTable[bCrc ^ (qData >> 24)];
			qLen -= 4;
		} while (qLen > 3);
	}

    if (qLen > 0) {
        pbSrc = (SnByte *)pqSrc;
        do {
            bCrc = pbCrcTable[bCrc ^ *pbSrc++];
        } while (--qLen > 0);
    }

	return bCrc;
}

SnByte CUtil::CrcMem(SnByte *pbSrc, SnQByte qLen)
{
    return CrcMemChunk(pbSrc, qLen, 0);
}

//
// This function draws a single line of text, on top of a device context, to the screen
//
void CUtil::DrawTextOnButton(CBitmapButton* pBtn, CFont* pFont, const CString& str)
{
	RECT wndRect;
	CDC* dcDisplay;

	// Get the Device Context for the OwnerDraw Button
	dcDisplay = pBtn->GetDC();
	
	// Set background and forground colors
	dcDisplay->SetTextColor(WHITE);
	dcDisplay->SetBkMode(TRANSPARENT);   
	
	/// Set the font
	CFont* pOldFont = dcDisplay->SelectObject(pFont);

	// Get the button dimensions
    pBtn->GetClientRect(&wndRect);
	
	// Draw text to the display
	dcDisplay->DrawText(str, &wndRect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	// Select back the old font
	dcDisplay->SelectObject(pOldFont);

	// Release the Device Context
	pBtn->ReleaseDC(dcDisplay);
}

void CUtil::RestoreShaverSerialNumber(CDriver *pDriver, char *pcSerialNumber)
{
    SnByte pbSerialNumber[SERIAL_NUMBER_SIZE+1] = { 0 };

    if (pDriver->ReadFlashData(FLASH_SERIAL_NUMBER_OFFSET, pbSerialNumber, SERIAL_NUMBER_SIZE)) {
        // Serial Number for DYONICS POWER II must begin with "AAX"
        if (strncmp((char *)pbSerialNumber, "AAX", 3) != 0 || atoi((char *)pbSerialNumber+3) < 5001) {
            return;
        }

        SnByte bTmp = 0;
        SnByte bCrc = 0;

        while (bTmp < (SERIAL_NUMBER_SIZE - 1)) {
            SnByte bCh = pbSerialNumber[bTmp];
            if (!((bCh >= '0' && bCh <= '9') || bCh == 'A' || bCh == 'X' || bCh == 0)) {
                return;
            }
            bCrc = pbCrcTable[bCrc ^ bCh];
            bTmp++;
        }

        // Only copy over valid Serial Number Data
        if (bCrc == pbSerialNumber[SERIAL_NUMBER_SIZE - 1]) {
            memcpy(pcSerialNumber, pbSerialNumber, SERIAL_NUMBER_SIZE);
        }
    }
}

SnBool CUtil::SaveShaverSerialNumber(CDriver *pDriver, char *pcSerialNumber)
{
    SnByte pbSerialNumberStore[SERIAL_NUMBER_SIZE] = { 0 };
    SnByte bTmp = 0;
    SnByte bCrc = 0;
    SnBool yRet = FALSE;

    // Valid strings are "AXX5001" and above, since that is the first EIP serial number
    if (pcSerialNumber == NULL || strlen(pcSerialNumber) < 7 || strlen(pcSerialNumber) > 10 ||
        strncmp(pcSerialNumber, "AAX", 3) != 0 || atoi(pcSerialNumber+3) < 5001) {
        return FALSE;
    }
    strcpy((char *)pbSerialNumberStore, pcSerialNumber);

    // Calculate Crc
    while (bTmp < SERIAL_NUMBER_SIZE - 1) {
        bCrc = pbCrcTable[bCrc ^ pbSerialNumberStore[bTmp]];
        bTmp++;
    }

    // Add Crc to help verify the Serial Number is valid
    pbSerialNumberStore[SERIAL_NUMBER_SIZE - 1] = bCrc;

    if (pDriver->EraseFlashPages(FLASH_SERIAL_NUMBER_OFFSET, 1) &&
        pDriver->WriteFlashData(FLASH_SERIAL_NUMBER_OFFSET, pbSerialNumberStore, SERIAL_NUMBER_SIZE)) {
        yRet = TRUE;
    }

    return yRet;
}
