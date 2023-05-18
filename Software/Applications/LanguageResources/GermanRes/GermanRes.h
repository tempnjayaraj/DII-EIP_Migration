// GermanRes.h : main header file for the GERMANRES DLL
//

#if !defined(AFX_GERMANRES_H__54B7B044_8738_46D9_AF36_C83AAD734711__INCLUDED_)
#define AFX_GERMANRES_H__54B7B044_8738_46D9_AF36_C83AAD734711__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CGermanResApp
// See GermanRes.cpp for the implementation of this class
//

class CGermanResApp : public CWinApp
{
public:
	CGermanResApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGermanResApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CGermanResApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GERMANRES_H__54B7B044_8738_46D9_AF36_C83AAD734711__INCLUDED_)
