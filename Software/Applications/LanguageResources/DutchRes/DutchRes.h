// DutchRes.h : main header file for the DUTCHRES DLL
//

#if !defined(AFX_DUTCHRES_H__3B6C63B9_5D1A_43D2_8D83_6522004EA0EE__INCLUDED_)
#define AFX_DUTCHRES_H__3B6C63B9_5D1A_43D2_8D83_6522004EA0EE__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDutchResApp
// See DutchRes.cpp for the implementation of this class
//

class CDutchResApp : public CWinApp
{
public:
	CDutchResApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDutchResApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CDutchResApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DUTCHRES_H__3B6C63B9_5D1A_43D2_8D83_6522004EA0EE__INCLUDED_)
