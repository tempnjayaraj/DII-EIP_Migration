// Shaver.h : main header file for the SHAVER application
//

#if !defined(AFX_SHAVER_H__C51B1231_DCF4_40B1_A4AC_5BDF428AEFE9__INCLUDED_)
#define AFX_SHAVER_H__C51B1231_DCF4_40B1_A4AC_5BDF428AEFE9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "SnTypes.h"
#include "Control.h"
#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CShaverApp:
// See Shaver.cpp for the implementation of this class
//

class CShaverApp : public CWinApp
{
public:
	CShaverApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShaverApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CShaverApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    // Thread
	friend DWORD WINAPI PowerOnSelfTestThread(LPVOID pParam);
	friend DWORD WINAPI ServiceDetectThread(LPVOID pParam);
	friend DWORD WINAPI ShutdownDetectThread(LPVOID pParam);

private:
    SnBool  m_bRun;
    DWORD   m_dwMode;
    HANDLE  m_hShutDownAppEvent;
    HANDLE  m_hShutDownAppMutex;
    SnBool  m_bDialogExited;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHAVER_H__C51B1231_DCF4_40B1_A4AC_5BDF428AEFE9__INCLUDED_)
