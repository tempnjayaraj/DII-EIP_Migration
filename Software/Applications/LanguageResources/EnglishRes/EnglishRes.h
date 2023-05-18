// EnglishRes.h : main header file for the ENGLISHRES DLL
//

#if !defined(AFX_ENGLISHRES_H__2CDF3533_EBA9_4776_BFD3_8F0708C65D67__INCLUDED_)
#define AFX_ENGLISHRES_H__2CDF3533_EBA9_4776_BFD3_8F0708C65D67__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CEnglishResApp
// See EnglishRes.cpp for the implementation of this class
//

class CEnglishResApp : public CWinApp
{
public:
	CEnglishResApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEnglishResApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CEnglishResApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ENGLISHRES_H__2CDF3533_EBA9_4776_BFD3_8F0708C65D67__INCLUDED_)
