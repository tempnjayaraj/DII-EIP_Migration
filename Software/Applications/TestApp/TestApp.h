// TestApp.h : main header file for the TESTAPP application
//

#if !defined(AFX_TESTAPP_H__1D63923A_A91D_4BA1_A67D_9AC15411ECBF__INCLUDED_)
#define AFX_TESTAPP_H__1D63923A_A91D_4BA1_A67D_9AC15411ECBF__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTestAppApp:
// See TestApp.cpp for the implementation of this class
//

class CTestAppApp : public CWinApp
{
public:
	CTestAppApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestAppApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTestAppApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTAPP_H__1D63923A_A91D_4BA1_A67D_9AC15411ECBF__INCLUDED_)
