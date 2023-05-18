// SnHelp.cpp: implementation of the CSnHelp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SnHelp.h"
#include "SharedMemory.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

SnSQByte			CSnHelp::m_sqCreationCount = 0;
HDC                 CSnHelp::m_hDisplayHDC = NULL;
CDC *               CSnHelp::m_hDisplayCDC;
CDC                 CSnHelp::m_hMemoryCDC;
CRITICAL_SECTION    CSnHelp::m_DisplayCs;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSnHelp::CSnHelp()
{
	m_LineWidth = 1; // initialize the line width to 1
    
    if (m_sqCreationCount == 0)
    {
	    InitializeCriticalSection(&m_DisplayCs);
        m_hDisplayHDC = ::GetDC(NULL);
		m_hDisplayCDC = CDC::FromHandle(m_hDisplayHDC);
	    m_hMemoryCDC.CreateCompatibleDC(m_hDisplayCDC);
        m_yCreatedDisplay = TRUE;
    }
    else
        m_yCreatedDisplay = FALSE;

	m_sqCreationCount++;
}

CSnHelp::~CSnHelp()
{
	m_sqCreationCount--;
    if (m_sqCreationCount == 0)
    {
        DeleteCriticalSection(&m_DisplayCs);
		m_hMemoryCDC.DeleteDC();
		m_hDisplayCDC->DeleteDC();
    }
}

//
// This function draws an unfilled rectangle to the screen
//
void CSnHelp::DrawRectEmpty( COLORREF refColor, SN_RECT rect)
{
	int	 iAdj = m_LineWidth/2;

    // Top
    m_hDisplayCDC->FillSolidRect(rect.x - iAdj, rect.y - iAdj,
        rect.width + m_LineWidth, m_LineWidth, refColor);
 
    // Bottom
    m_hDisplayCDC->FillSolidRect(rect.x - iAdj, rect.y + rect.height - iAdj,
        rect.width + m_LineWidth, m_LineWidth, refColor);

    // Left
    m_hDisplayCDC->FillSolidRect(rect.x - iAdj, rect.y + iAdj,
        m_LineWidth, rect.height - m_LineWidth, refColor);

    // Right
    m_hDisplayCDC->FillSolidRect(rect.x + rect.width - iAdj, rect.y + iAdj,
        m_LineWidth, rect.height - m_LineWidth, refColor);
}

//
// This function draws a single line to the screen
//
void CSnHelp::DrawLine( COLORREF refColor, POINT startPoint, POINT endPoint)
{
	int iAdj = m_LineWidth/2;
    int iTmp;

    if (startPoint.x > endPoint.x) {
        iTmp = endPoint.x;
        endPoint.x = startPoint.x;
        startPoint.x = iTmp;
    }
    if (startPoint.y > endPoint.y) {
        iTmp = endPoint.y;
        endPoint.y = startPoint.y;
        startPoint.y = iTmp;
    }

    m_hDisplayCDC->FillSolidRect(startPoint.x - iAdj, startPoint.y - iAdj,
        endPoint.x - startPoint.x + m_LineWidth,
        endPoint.y - startPoint.y + m_LineWidth, refColor);
}

//
// This function draws a single line of text, on top of a device context, to the screen
//
void CSnHelp::DrawTextOnButton( CBitButton* pBtn, CFont* font,CString str, COLORREF refColor)
{
	RECT tRect;
    pBtn->GetWindowRect(&tRect);

    EnterCriticalSection(&CSnHelp::m_DisplayCs);
 
    // Setup the Colors for Text Draw (White on Black)
    CSnHelp::m_hDisplayCDC->SetTextColor(refColor);
    CSnHelp::m_hDisplayCDC->SetBkMode(TRANSPARENT);
	
    // Select the Font
    CSnHelp::m_hDisplayCDC->SelectObject(font->operator HFONT());

    // Draw the string
    CSnHelp::m_hDisplayCDC->DrawText(str, str.GetLength(), &tRect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	LeaveCriticalSection(&CSnHelp::m_DisplayCs);
}

//
// This function draws multiple lines of text, on top of a device context, to the screen
//
void CSnHelp::DrawTextOnStaticMulti( CStatic* pStatic, CFont* font,CString str, COLORREF textColor, DWORD yCenterPos)
{
	RECT tRect;
    pStatic->GetWindowRect(&tRect);
	tRect.top = tRect.top + yCenterPos;

    EnterCriticalSection(&CSnHelp::m_DisplayCs);
 
    // Setup the Colors for Text Draw (White on Black)
    CSnHelp::m_hDisplayCDC->SetTextColor(textColor);
    CSnHelp::m_hDisplayCDC->SetBkMode(TRANSPARENT);
	
    // Select the Font
    CSnHelp::m_hDisplayCDC->SelectObject(font);

    // Draw the string
    CSnHelp::m_hDisplayCDC->DrawText(str, str.GetLength(), &tRect, DT_CENTER);

	LeaveCriticalSection(&CSnHelp::m_DisplayCs);
}

//
// This function draws a single line of text, on top of a device context, to the screen
//
void CSnHelp::DrawTextOnStaticSingle( CStatic* pStatic, CFont* font,CString str, COLORREF textColor)
{
	RECT tRect;
    pStatic->GetWindowRect(&tRect);

    EnterCriticalSection(&CSnHelp::m_DisplayCs);
 
    // Setup the Colors for Text Draw (White on Black)
    CSnHelp::m_hDisplayCDC->SetTextColor(textColor);
    CSnHelp::m_hDisplayCDC->SetBkMode(TRANSPARENT);
	
    // Select the Font
    CSnHelp::m_hDisplayCDC->SelectObject(font);

    // Draw the string
    CSnHelp::m_hDisplayCDC->DrawText(str, str.GetLength(), &tRect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	LeaveCriticalSection(&CSnHelp::m_DisplayCs);
}

void CSnHelp::DrawTextOnDisplay(RECT *ptRect, CFont* pFont, CString csStr)
{
	EnterCriticalSection(&CSnHelp::m_DisplayCs);
 
    // Setup the Colors for Text Draw (White on Black)
    CSnHelp::m_hDisplayCDC->SetTextColor(SN_WHITE);
    CSnHelp::m_hDisplayCDC->SetBkColor(SN_BLACK);
	
    // Select the Font
    CSnHelp::m_hDisplayCDC->SelectObject(pFont);

    // Draw the string
	CSnHelp::m_hDisplayCDC->DrawText( csStr, csStr.GetLength(), ptRect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	LeaveCriticalSection(&CSnHelp::m_DisplayCs);
}

void CSnHelp::LoadBitmap(CBitmap *pDstBmp, UINT nBitmapID)
{
    CBitmap SrcBmp;
    SrcBmp.LoadBitmap(nBitmapID);

    BITMAP BmpInfo;
    SrcBmp.GetBitmap(&BmpInfo);

    CDC SrcDC;
    SrcDC.CreateCompatibleDC(NULL);

    CBitmap *pOldBmp1 = SrcDC.SelectObject(&SrcBmp);

    CBitmap TmpBmp;
    TmpBmp.CreateBitmap(BmpInfo.bmWidth,BmpInfo.bmHeight,1,16, NULL);

    CDC TmpDC;
    TmpDC.CreateCompatibleDC(NULL);

    CBitmap *pOldBmp2 = TmpDC.SelectObject(&TmpBmp);

    TmpDC.BitBlt(0,0,BmpInfo.bmWidth,BmpInfo.bmHeight,&SrcDC,0,0,SRCCOPY);
        
    pDstBmp->CreateCompatibleBitmap(&TmpDC,BmpInfo.bmWidth,BmpInfo.bmHeight);

    CDC DstDC;
    DstDC.CreateCompatibleDC(NULL);

    CBitmap *pOldBmp3 = DstDC.SelectObject(pDstBmp);

    DstDC.BitBlt(0,0,BmpInfo.bmWidth,BmpInfo.bmHeight,&TmpDC,0,0,SRCCOPY);

    SrcDC.SelectObject(pOldBmp1);
    TmpDC.SelectObject(pOldBmp2);
    DstDC.SelectObject(pOldBmp3);

	TmpDC.DeleteDC();
	DstDC.DeleteDC();

	TmpBmp.DeleteObject();
}

CString CSnHelp::GetString(int resourceID)
{
	CSharedMemory mem;
	// Returns text extracted from the String Table resource
	CString str = _T("");
	TCHAR pwText[1024];
	
	// Get the handle of the resource dll
	HINSTANCE hResLib = mem.GetResourceHandle();

	if( hResLib != NULL)
	{
		if (LoadString( hResLib, resourceID, pwText,  sizeof(pwText)))
		    str = pwText;
		else if ((resourceID) && wcslen((TCHAR *)resourceID) < 1024)
		{
			str = (TCHAR *)(resourceID);
		}
	}

	return str;
}

void CSnHelp::GetString(int resourceID, CString& str)
{
	CSharedMemory mem;
	// Returns text extracted from the String Table resource
	TCHAR pwText[1024];
	str = _T("");   // failure

	// Get the handle of the resource dll
	HINSTANCE hResLib = mem.GetResourceHandle();

	if( hResLib != NULL)
	{
		if (LoadString( hResLib, resourceID, pwText,  sizeof(pwText)))
			str = pwText;
		else if ((resourceID) && wcslen((TCHAR *)resourceID) < 1024)
		{
			str = (TCHAR *)(resourceID);
		}		

	}
}

// Function:	Get Buffer
// Purpose:		convert a CString type to a char buffer
// Return:		void
//
void CSnHelp::GetBuffer( char* dest, CString csSource)
{
	CString csTmp;
	int length = 0;
	int i;
	LPTSTR  pBuffer;

	csTmp = csSource;
	length = csTmp.GetLength();
	length = length + 1; // add 1 for the null character

	pBuffer = csTmp.GetBuffer( length);
	for(i=0;i<length;i++)
	{
		dest[i] = (char)*pBuffer;
		pBuffer++;
	}
	csTmp.ReleaseBuffer();
}

// Function:	UnicodeToAscii
// Purpose:		convert a CString type to a unicode ascii string
// Return:		void
//
void CSnHelp::UnicodeToAscii(const wchar_t *pSourceStr, char *pTargetStr, long nSize)
{
	long i;
	long bufSize;

	bufSize = nSize - 1;

	for (i = 0; i < bufSize; i++)
	{
		if (pSourceStr[i] == '\0')
		{
			break;
		}
		else
		{
			pTargetStr[i] = (char)pSourceStr[i];
		}
	}
	pTargetStr[i] = 0;
}

CString CSnHelp::FixedFloatToCString( SnWord wFixedFloat)
{
	CSharedMemory mem;
	CString csFlow;
    CString csSep;

	if( mem.GetCurrentLanguage() != LANGUAGE_ENGLISH)
        csSep = _T(",");
    else
        csSep = _T(".");

    if (wFixedFloat > 99)
	    csFlow.Format(_T("1%s00"), csSep);
    else
        csFlow.Format(_T("0%s%02d"), csSep, wFixedFloat);

	return csFlow;
}

SnWord CSnHelp::CStringToFixedFloat( CString csText)
{
	SnWord wFixedFloat;
	char tempBuf[TMP_BUF_SIZE];

    if (csText[0] != '0')
        return 100;

	// Convert CString to char string
	UnicodeToAscii((LPCTSTR)csText.Right(2), tempBuf, TMP_BUF_SIZE);

	// convert ascii to integer
	wFixedFloat = atoi( tempBuf);

	return wFixedFloat;
}