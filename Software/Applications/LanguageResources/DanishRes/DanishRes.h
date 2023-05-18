// DanishRes.h : main header file for the DANISHRES DLL
//

#if !defined(AFX_DANISHRES_H__99070A83_5F7C_4EAB_B992_8F8754B571C3__INCLUDED_)
#define AFX_DANISHRES_H__99070A83_5F7C_4EAB_B992_8F8754B571C3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDanishResApp
// See DanishRes.cpp for the implementation of this class
//

class CDanishResApp : public CWinApp
{
public:
	CDanishResApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDanishResApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CDanishResApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DANISHRES_H__99070A83_5F7C_4EAB_B992_8F8754B571C3__INCLUDED_)
