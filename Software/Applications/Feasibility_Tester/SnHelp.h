// SnHelp.h: interface for the CSnHelp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SNHELP_H__CEA52790_EE90_475B_9EF4_12A033A1206A__INCLUDED_)
#define AFX_SNHELP_H__CEA52790_EE90_475B_9EF4_12A033A1206A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CommonDefines.h"
#include "ColorsFonts.h"
#include "SnTypes.h"
#include "BitButton.h"

typedef struct 
{
    int x;
    int y;
    int width;
	int height;

} SN_RECT;

class CSnHelp : public CDialog  
{
public:
	CSnHelp();
	virtual ~CSnHelp();
	
public: // Smith and Nephew GUI helper functions
	void DrawRectEmpty( COLORREF refColor, SN_RECT rect); 
	void DrawLine( COLORREF refColor, POINT startPoint, POINT endPoint);
	void DrawTextOnButton( CBitButton* pBtn, CFont* font, CString str, COLORREF refColor = SN_WHITE);
	void DrawTextOnStaticMulti( CStatic* pStatic, CFont* font, CString str, COLORREF textColor, DWORD yCenterPos = 12);
	void DrawTextOnStaticSingle( CStatic* pStatic, CFont* font, CString str, COLORREF textColor);
    void DrawTextOnDisplay(RECT *ptRect, CFont* pFont, const CString csStr);
    void LoadBitmap(CBitmap *pDstBmp, UINT nBitmapID);
	inline void SetLineWidth( int width){ m_LineWidth = width;}
	inline int GetLineWidth(){return m_LineWidth;}

	void UnicodeToAscii(const wchar_t *pSourceStr, char *pTargetStr, long nSize);
	CString GetString(int resourceID);
	void GetString(int resourceID, CString& str);
	void  GetBuffer( char* dest, CString csSource);
	CString FixedFloatToCString( SnWord wFixedFloat);
	SnWord	CStringToFixedFloat( CString csText);

private: // local member variables
	static SnSQByte	m_sqCreationCount;
	int m_LineWidth;
    SnBool m_yCreatedDisplay;

public:
	static HDC		m_hDisplayHDC;
	static CDC*		m_hDisplayCDC;
    static CDC      m_hMemoryCDC;
    static CRITICAL_SECTION   m_DisplayCs;
};

#endif // !defined(AFX_SNHELP_H__CEA52790_EE90_475B_9EF4_12A033A1206A__INCLUDED_)
