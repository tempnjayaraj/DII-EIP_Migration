// Shaver.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Shaver.h"

#include "Control.h"
#include "IconStatusBar.h"
#include "ProcedureScreen.h"
#include "SystemErrorDlg.h"
#include "FactoryScreen.h"
#include "MessageBox.h"
#include "TestMode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShaverApp

BEGIN_MESSAGE_MAP(CShaverApp, CWinApp)
	//{{AFX_MSG_MAP(CShaverApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShaverApp construction

CShaverApp::CShaverApp()
	: CWinApp()
{
    m_bRun = TRUE;
    m_dwMode = 0;
    m_hShutDownAppEvent = NULL;
    m_hShutDownAppMutex = NULL;	
    m_bDialogExited = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CShaverApp object

CShaverApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CShaverApp initialization

BOOL CShaverApp::InitInstance()
{
    SnBool		bStatus;
	CControl*	pControl;
	int			nResponse;
	CSnHelp		SnHelp;
	DWORD		hShaverThreadID;
	DWORD		m_hThreadID;


    m_pMainWnd = NULL;
	pControl = NULL;

	hShaverThreadID = GetCurrentThreadId();
	DEBUGMSG(TRUE, (TEXT("Shaver Application Thread: 0x%08X\n"),hShaverThreadID));

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

	// Bring up the Control layer 
	pControl = (CControl*)new(CControl);
		
	if(pControl)
	{
		unsigned short usTypeFailure, usDetailFailure;
	
		bStatus = pControl->Init(&usTypeFailure, &usDetailFailure);
		
		if( usTypeFailure != CLEAR_ERROR_CONDITION)
		{
			// Initialize shared memory in order to load language dlls.
			CSharedMemory mem;
			SnBool bStatus = mem.GetInitStatus();
			if( !bStatus)
				mem.Init( NULL, pControl);
			
			DWORD dwErrorNum;
					
			if(usTypeFailure == COMMUNICATION_ERROR)
				dwErrorNum = 2;
			else if(usTypeFailure == SYSTEM_RESOURCE_ERROR)
				dwErrorNum = 4;
			else
				dwErrorNum = 1;
				
			// Launch the "System Error" Screen
			// This call will never return. User must reboot
			CSystemErrorDlg errDlg(pControl, dwErrorNum, SN_CLEAR_TEXT);
			errDlg.DoModal();
		}
		else
		{
			// Init Passed	

			// Initialize shared memory in order to load language dlls.
			CSharedMemory mem;
			SnBool bStatus = mem.GetInitStatus();
			if( !bStatus)
				mem.Init( NULL, pControl);


            // Check the status of persistent storage
			if(pControl)
			{
				if(pControl->RecallFlashFailed())
				{
	                pControl->ResetDisplayBase();
					CMessageBox dlg( pControl, SN_SYSTEM_ERROR, SN_SET_SPEEDS_RETRIEVE_FAILURE);
					dlg.DoModal();
				}
			
				if(pControl->RecallNvRamFailed())
				{
	                pControl->ResetDisplayBase();
					CMessageBox dlg( pControl, SN_SYSTEM_ERROR, SN_CUSTOM_SETTINGS_RETRIEVE_FAILURE);
					dlg.DoModal();
				}
			}
		}
	}
	
	// Spawn thread for Power On Self Test
    ::CreateThread((LPSECURITY_ATTRIBUTES)NULL,
				    0,
					PowerOnSelfTestThread,
					pControl,
					0,
					&m_hThreadID);

	DEBUGMSG(TRUE, (TEXT("Shaver PowerOnSelfTestThread: 0x%08X\n"),m_hThreadID));

	// Spawn thread for Biomed and Factory Mode Detection
	::CreateThread((LPSECURITY_ATTRIBUTES)NULL,
		            0,
					ServiceDetectThread,
					this,
					0,
					&m_hThreadID);

	DEBUGMSG(TRUE, (TEXT("Shaver ServiceDetectThread: 0x%08X\n"),m_hThreadID));
	
	
	// Create and display the common status bar  
	CIconStatusBar *pIconStatusBar;
    pIconStatusBar = new CIconStatusBar(pControl, 0);
    pIconStatusBar->Create(IDD_ICON_STATUS_BAR);


	while( m_bRun)
	{
        m_bDialogExited = FALSE;

        if (m_dwMode == TEST_MODE)
		{
		    // Found Test key run diagnositcs
		    CTestMode dlg(pControl);
		    nResponse = dlg.DoModal();
            m_bDialogExited = TRUE;
        }
		else if (m_dwMode == FACTORY_MODE || m_dwMode == SERVICE_MODE || m_dwMode == RESET_SN_MODE)
		{
            if (m_dwMode == RESET_SN_MODE) {
                pControl->EraseShaverSerialNumber();
                pControl->RestoreShaverSerialNumber();
                m_dwMode = FACTORY_MODE;
            }

			// Found Factory key run diagnositcs
		    CFactoryScreen dlg(pControl, m_dwMode);
		    nResponse = dlg.DoModal();
            m_bDialogExited = TRUE;
        } 
		else
		{
			CProcedureScreen dlg( pControl);
			nResponse = dlg.DoModal();
            m_bDialogExited = TRUE;
		}
	
		if (nResponse == IDOK)
		{
			// Don't do anything
		}
		else if (nResponse == IDCANCEL)
		{
			// Don't do anything	
		}
	}
	
    // Allow Software Update to go once the dialog screen is down and
    // Power On Self Test is complete. Meanwhile shut down Control.
    SnQByte qTimeout;
    for (qTimeout = 0; qTimeout < 1000 && !pControl->CheckPowerOnSelfTest(); qTimeout += 100)
        Sleep(100);
    ReleaseMutex(m_hShutDownAppMutex);

    // Delete the Shared Memory resources
	CSharedMemory mem;
	bStatus = mem.GetInitStatus();
	if( bStatus)
		mem.DeInit();
	
	// delete Motor board resources
	if( pControl)
	{
		pControl->DeInit(FALSE);
		delete(pControl);
		pControl = NULL;
	}

	// De-allocate the common status bar
	if( pIconStatusBar)
		delete(pIconStatusBar);
	
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

DWORD WINAPI ServiceDetectThread(LPVOID pParam)
{
    CShaverApp *pShaverApp = (CShaverApp *)pParam;
    SnQByte qTries = 40;    // Poll for the USB key for 4 Seconds

    do {
        Sleep(100);

        // Take a look at the USB drive to see if any special keys are there
        if (GetFileAttributes(FILE_NAME_RESETSN) != 0xFFFFFFFF) {
            pShaverApp->m_dwMode = RESET_SN_MODE;           // Reset Shaver Serial Number before starting Factory Mode
        } else if (GetFileAttributes(FILE_NAME_TEST) != 0xFFFFFFFF) {
		    pShaverApp->m_dwMode = TEST_MODE;               // Found the Test key (aka. file name)
        } else if (GetFileAttributes(FILE_NAME_FACTORY) != 0xFFFFFFFF) {
		    pShaverApp->m_dwMode = FACTORY_MODE;            // Found the Factory key (aka. file name)
        } else if (GetFileAttributes(FILE_NAME_SERVICE) != 0xFFFFFFFF) {
			pShaverApp->m_dwMode = SERVICE_MODE;            // Found the Service key (aka. file name)
        } else {
		    pShaverApp->m_dwMode = 0;
        }
    } while (pShaverApp->m_dwMode == 0 && --qTries > 0);

    if (pShaverApp->m_dwMode) {
        PostMessage(HWND_BROADCAST, WM_EXIT_DIALOG, (WPARAM)0, (LPARAM)0);
    }
	return(0);
}

DWORD WINAPI ShutdownDetectThread(LPVOID pParam)
{
    CShaverApp *pShaverApp = (CShaverApp *)pParam;
	
    WaitForSingleObject(pShaverApp->m_hShutDownAppEvent, INFINITE);

	pShaverApp->m_bRun = FALSE;
	while (!pShaverApp->m_bDialogExited) {
		PostMessage(HWND_BROADCAST, WM_EXIT_DIALOG, (WPARAM)0, (LPARAM)0);
		Sleep(100);
	}
	return(0);
}