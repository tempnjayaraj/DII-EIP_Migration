#if !defined(AFX_BITBUTTON_H__DE6248C4_0E2E_4925_8232_12142FA29E16__INCLUDED_)
#define AFX_BITBUTTON_H__DE6248C4_0E2E_4925_8232_12142FA29E16__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BitButton.h : header file
//
#include "ColorFonts.h"

/////////////////////////////////////////////////////////////////////////////
// CBitButton window

class CBitButton : public CButton
{
public:
	CBitButton();
	virtual ~CBitButton();
    BOOL LoadBitmaps(UINT nUpID, UINT nDownID, CFont *pFont = NULL, CString csStr = SN_CLEAR_TEXT, COLORREF refColor = SN_WHITE, SnQByte qTop = 0);

	//{{AFX_VIRTUAL(CBitButton)
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

private:
    LoadBitButton(CBitmap *pDstBmp, UINT nID, CFont *pFont, CString csStr, COLORREF refColor, SnQByte qTop);

public:

	// Generated message map functions
protected:
    CBitmap m_BitmapUp;
    CBitmap m_BitmapDown;

    //{{AFX_MSG(CBitButton)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BITBUTTON_H__DE6248C4_0E2E_4925_8232_12142FA29E16__INCLUDED_)
