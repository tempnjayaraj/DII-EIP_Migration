// TestRS485.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Control.h"
#include "SystemErrorDlg.h"
#include "TestRS485.h"
#include "TestRS485Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestRS485App

BEGIN_MESSAGE_MAP(CTestRS485App, CWinApp)
	//{{AFX_MSG_MAP(CTestRS485App)
	//}}AFX_MSG
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestRS485App construction

CTestRS485App::CTestRS485App()
	: CWinApp(),
	m_hShutDownAppEvent(NULL),
    m_hShutDownAppMutex(NULL),
	m_bDialogExited(FALSE)
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTestRS485App object

CTestRS485App theApp;

/////////////////////////////////////////////////////////////////////////////
// CTestRS485App initialization

BOOL CTestRS485App::InitInstance()
{
    SnBool		bStatus;
	CControl*	pControl;
	DWORD		hTestRS485ThreadID;
	DWORD		m_hThreadID;

	m_pMainWnd = NULL;
	pControl = NULL;

	hTestRS485ThreadID = GetCurrentThreadId();
	DEBUGMSG(TRUE, (TEXT("Footswitch Tester Application Thread: 0x%08X\n"),hTestRS485ThreadID));

	// Create Shutdown Mutex
    m_hShutDownAppMutex = CreateMutex(NULL, TRUE, _T("SnShutDownAppMutex"));
    if ( m_hShutDownAppMutex == NULL)
    {
        return FALSE;
    }
	
	// Create Shutdown Event
	m_hShutDownAppEvent = CreateEvent(NULL, FALSE, FALSE, _T("SnShutDownAppEvent"));
	if( m_hShutDownAppEvent == NULL)
	{ 
        return FALSE;
	}

	// Spawn thread to detect shutdown request and shut down the app
	::CreateThread((LPSECURITY_ATTRIBUTES)NULL,
		            0,
					ShutdownDetectThread,
					this,
					0,
					&m_hThreadID);

	DEBUGMSG(TRUE, (TEXT("Shaver ShutdownDetectThread: 0x%08X\n"),m_hThreadID));

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Standard initialization
	// Bring up the Control layer 
	pControl = (CControl*)new(CControl);
		
	if(pControl)
	{
		unsigned short usTypeFailure, usDetailFailure;
	
		bStatus = pControl->Init(&usTypeFailure, &usDetailFailure);
		pControl->ResetDisplayBase();
	
		if( usTypeFailure != CLEAR_ERROR_CONDITION)
		{
			DWORD dwErrorNum;
					
			if(usTypeFailure == COMMUNICATION_ERROR)
				dwErrorNum = 2;
			else if(usTypeFailure == SYSTEM_RESOURCE_ERROR)
				dwErrorNum = 4;
			else
				dwErrorNum = 1;

			CSystemErrorDlg errDlg(pControl, dwErrorNum, SN_CLEAR_TEXT);
			errDlg.DoModal();
		}

		// Spawn thread for Power On Self Test
		::CreateThread((LPSECURITY_ATTRIBUTES)NULL,
				    0,
					PowerOnSelfTestThread,
					pControl,
					0,
					&m_hThreadID);

		DEBUGMSG(TRUE, (TEXT("Shaver PowerOnSelfTestThread: 0x%08X\n"),m_hThreadID));

		CTestRS485Dlg dlg(pControl);
		m_pMainWnd = &dlg;
		int nResponse = dlg.DoModal();
		m_bDialogExited = TRUE;


		if (nResponse == IDOK)
		{
		}
		else if (nResponse == IDCANCEL)
		{
		}
 	}

   // Allow Software Update to go once the dialog screen is down and
    // Power On Self Test is complete. Meanwhile shut down Control.
    SnQByte qTimeout;
    for (qTimeout = 0; qTimeout < 1000 && !pControl->CheckPowerOnSelfTest(); qTimeout += 100)
        Sleep(100);
    ReleaseMutex(m_hShutDownAppMutex);

	// delete Motor board resources
	if( pControl)
	{
		pControl->DeInit(FALSE);
		delete(pControl);
		pControl = NULL;
	}

	if( m_hShutDownAppEvent)
	{
		CloseHandle( m_hShutDownAppEvent);
		m_hShutDownAppEvent = NULL;
	}
	
	if( m_hShutDownAppMutex)
	{
		CloseHandle( m_hShutDownAppMutex);
		m_hShutDownAppEvent = NULL;
	}
	
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

DWORD WINAPI PowerOnSelfTestThread(LPVOID pParam)
{
	CControl *pControl = (CControl *)pParam;

    pControl->PowerOnSelfTest();
	pControl->ResetDisplayBase();
	return(0);
}

DWORD WINAPI ShutdownDetectThread(LPVOID pParam)
{
    CTestRS485App *pTestRS485App = (CTestRS485App *)pParam;
	
    WaitForSingleObject(pTestRS485App->m_hShutDownAppEvent, INFINITE);

	while (!pTestRS485App->m_bDialogExited) {
		PostMessage(HWND_BROADCAST, WM_EXIT_DIALOG, (WPARAM)0, (LPARAM)0);
		Sleep(100);
	}
	return(0);
}

