// FrenchRes.h : main header file for the FRENCHRES DLL
//

#if !defined(AFX_FRENCHRES_H__BAFECFE3_1C52_40DA_ADCF_4924AD8F32A6__INCLUDED_)
#define AFX_FRENCHRES_H__BAFECFE3_1C52_40DA_ADCF_4924AD8F32A6__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CFrenchResApp
// See FrenchRes.cpp for the implementation of this class
//

class CFrenchResApp : public CWinApp
{
public:
	CFrenchResApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFrenchResApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CFrenchResApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FRENCHRES_H__BAFECFE3_1C52_40DA_ADCF_4924AD8F32A6__INCLUDED_)
