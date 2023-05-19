// TestRS485.h : main header file for the TestRS485 application
//

#if !defined(AFX_TestRS485_H__45B1B4CC_1425_4C99_AB4B_7BE22AC1706D__INCLUDED_)
#define AFX_TestRS485_H__45B1B4CC_1425_4C99_AB4B_7BE22AC1706D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "SnTypes.h"
#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTestRS485App:
// See TestRS485.cpp for the implementation of this class
//

class CTestRS485App : public CWinApp
{
public:
	CTestRS485App();
private:
    HANDLE  m_hShutDownAppEvent;
    HANDLE  m_hShutDownAppMutex;
    SnBool  m_bDialogExited;

	friend DWORD WINAPI PowerOnSelfTestThread(LPVOID pParam);
	friend DWORD WINAPI ShutdownDetectThread(LPVOID pParam);
 
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestRS485App)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTestRS485App)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TestRS485_H__45B1B4CC_1425_4C99_AB4B_7BE22AC1706D__INCLUDED_)
