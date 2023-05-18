#if !defined(AFX_UTIL_H__A2247BC1_38EA_4077_B169_55302FF8F077__INCLUDED_)
#define AFX_UTIL_H__A2247BC1_38EA_4077_B169_55302FF8F077__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#include "ColorFonts.h"
#include "Driver.h"

#define SERIAL_NUMBER_SIZE          12
#define FLASH_SERIAL_NUMBER_OFFSET  0x40000

// Total size 56 bits = 7 bytes
typedef struct  
{
    
    SnByte language:		5;

	SnByte mode:            1;
    SnByte OscModePortA:    2;
    SnByte OscModePortB :   2;
	SnByte OscPortASec:		8;	
	SnByte OscPortBSec:		8;	
	SnByte OscPortARev:		4;	
	SnByte OscPortBRev:		4;
	SnByte FootMode:		2;
	SnByte FootForward:		2;
	SnByte FootHandCtl:		2;
	SnByte PortCtl:			4;
	SnByte ShaverPktCtl:	4;
  
    // Crc over all bytes except this one, so total CRC should be 0
    SnByte ucCrc:            8;

} SAVED_SHAVER_INFO;

const SnByte pbCrcTable[] = 
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
//
// end
//
//
// Util.h : header file
//

class CUtil
{
public:
	CUtil();
	virtual ~CUtil();

public:
    SnByte CrcMemChunk(SnByte* pbSrc, SnQByte qLen, SnByte bCrc);
    SnByte CrcMem(SnByte *pbSrc, SnQByte qLen);
    void DrawTextOnButton(CBitmapButton* pBtn, CFont* pFont, const CString& str);
    void RestoreShaverSerialNumber(CDriver *pDriver, char *pcSerialNumber);
    SnBool SaveShaverSerialNumber(CDriver *pDriver, char *pcSerialNumber);

public:
	static HDC		m_hDisplayHDC;
	static CDC*		m_hDisplayCDC;
    static CDC      m_hMemoryCDC;
    static CRITICAL_SECTION   m_DisplayCs;

private:
	static SnSQByte	m_sqCreationCount;
    CDriver*    m_pDriver;
};

#endif // !defined(AFX_UTIL_H__A2247BC1_38EA_4077_B169_55302FF8F077__INCLUDED_)
