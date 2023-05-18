// BitButton.cpp : implementation file
//

#include "stdafx.h"
#include "SnTypes.h"
#include "SnHelp.h"
#include "BitButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBitButton


BEGIN_MESSAGE_MAP(CBitButton, CButton)
	//{{AFX_MSG_MAP(CBitButton)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CBitButton::CBitButton()
{
}

CBitButton::~CBitButton()
{
	m_BitmapUp.DeleteObject();
	m_BitmapDown.DeleteObject();
}

CBitButton::LoadBitButton(CBitmap *pDstBmp, UINT nID, CFont *pFont, CString csStr, COLORREF refColor, SnQByte qTop)
{
    CBitmap SrcBmp;
    SrcBmp.LoadBitmap(nID);

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

    if (pFont) {
        RECT tRect;
        tRect.left = 0;
        tRect.top = qTop;
        tRect.right = BmpInfo.bmWidth;
        tRect.bottom = BmpInfo.bmHeight;

        TmpDC.SetTextColor(refColor);
        TmpDC.SetBkMode(TRANSPARENT);
        TmpDC.SelectObject(pFont);
        TmpDC.DrawText(csStr, csStr.GetLength(), &tRect,
           qTop ? DT_CENTER : DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    }
        
    pDstBmp->CreateCompatibleBitmap(&TmpDC,BmpInfo.bmWidth,BmpInfo.bmHeight);

    CDC DstDC;
    DstDC.CreateCompatibleDC(NULL);

    CBitmap *pOldBmp3 = DstDC.SelectObject(pDstBmp);

    DstDC.BitBlt(0,0,BmpInfo.bmWidth,BmpInfo.bmHeight,&TmpDC,0,0,SRCCOPY);

    SrcDC.SelectObject(pOldBmp1);
    TmpDC.SelectObject(pOldBmp2);
    DstDC.SelectObject(pOldBmp3);
}

BOOL CBitButton::LoadBitmaps(UINT nUpID, UINT nDownID, CFont *pFont, CString csStr, COLORREF refColor, SnQByte qTop)
{
	m_BitmapUp.DeleteObject();
	m_BitmapDown.DeleteObject();

    LoadBitButton(&m_BitmapUp, nUpID, pFont, csStr, refColor, qTop);
    LoadBitButton(&m_BitmapDown, nDownID, pFont, csStr, refColor, qTop);

    // Resize the button to the size of the bitmap
	BITMAP bmInfo;
	m_BitmapUp.GetObject(sizeof(bmInfo), &bmInfo);
	SetWindowPos(NULL, -1, -1, bmInfo.bmWidth, bmInfo.bmHeight,
		SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

    return TRUE;
}

void CBitButton::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
    if(lpDIS->itemAction != ODA_FOCUS)
	{
        CBitmap *pOldBitmap;
        CRect tWinRect;

        GetWindowRect(tWinRect);

	    EnterCriticalSection(&CSnHelp::m_DisplayCs);
        pOldBitmap = CSnHelp::m_hMemoryCDC.SelectObject((lpDIS->itemState & ODS_SELECTED) ? &m_BitmapDown : &m_BitmapUp);
        CSnHelp::m_hDisplayCDC->BitBlt(tWinRect.left, tWinRect.top, tWinRect.Width(), tWinRect.Height(),
            &CSnHelp::m_hMemoryCDC, 0, 0, SRCCOPY);
        CSnHelp::m_hMemoryCDC.SelectObject(pOldBitmap);
	    LeaveCriticalSection(&CSnHelp::m_DisplayCs);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBitButton message handlers
