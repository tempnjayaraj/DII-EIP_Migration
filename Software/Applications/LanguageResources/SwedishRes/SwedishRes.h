// SwedishRes.h : main header file for the SWEDISHRES DLL
//

#if !defined(AFX_SWEDISHRES_H__D27CC42A_BCAE_4526_86F8_1B2BBCD1BAB3__INCLUDED_)
#define AFX_SWEDISHRES_H__D27CC42A_BCAE_4526_86F8_1B2BBCD1BAB3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSwedishResApp
// See SwedishRes.cpp for the implementation of this class
//

class CSwedishResApp : public CWinApp
{
public:
	CSwedishResApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSwedishResApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CSwedishResApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SWEDISHRES_H__D27CC42A_BCAE_4526_86F8_1B2BBCD1BAB3__INCLUDED_)
