// SoftwareUpgrade.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "SoftwareUpgrade.h"
#include "SoftwareUpgradeApp.h"
#include "SoftwareRepairDlg.h"
#include "SoftwareUpgradeDlg.h"
#include "SharedMemory.h"
#include "SnIoctl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSoftwareUpgradeApp

BEGIN_MESSAGE_MAP(CSoftwareUpgradeApp, CWinApp)
	//{{AFX_MSG_MAP(CSoftwareUpgradeApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
END_MESSAGE_MAP()

CDriver*        g_pDriver;		// Pointer to a Driver Object

/////////////////////////////////////////////////////////////////////////////
// CSoftwareUpgradeApp construction

CSoftwareUpgradeApp::CSoftwareUpgradeApp()
	: CWinApp()
{
    g_pDriver = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSoftwareUpgradeApp object

CSoftwareUpgradeApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSoftwareUpgradeApp initialization

BOOL CSoftwareUpgradeApp::InitInstance()
{
	HANDLE hShutDownAppEvent;
	HANDLE hWaitForAppToShutDownMutex;
	CSharedMemory mem;
	SnQByte qTries = 30;	// Poll for the USB key for 3 Seconds
	FILE* pUpgradeFile;
	FILE* pFactoryModeFile;
	char cTempBuf[MAX_PATH];

	m_pMainWnd = NULL;	// If this data member is NULL, the main window for the CWinApp 
						// object of the application will be used to determine when to terminate
						// the thread. m_pMainWnd is a public variable of type CWnd*.

	// Initialize Driver
	g_pDriver = (CDriver*)new(CDriver);
	if (g_pDriver) {
		g_pDriver->InitDriver();

	} else {
		return FALSE;
	}

	hShutDownAppEvent = CreateEvent(NULL, FALSE, FALSE, _T("SnShutDownAppEvent"));
	hWaitForAppToShutDownMutex = CreateMutex(NULL, FALSE, _T("SnShutDownAppMutex"));

	// Check the state of both the upper and lower flash images
	SnBool yLowerFlashValid;
	SnBool yUpperFlashValid;
	SnQByte qBootStatus;

	if (g_pDriver->CheckBootStatus(&qBootStatus)) {
		yLowerFlashValid = (qBootStatus & 0x0000ffff) ? TRUE : FALSE;
		yUpperFlashValid = (qBootStatus & 0xffff0000) ? TRUE : FALSE;
	} else {
		yLowerFlashValid = TRUE;
		yUpperFlashValid = TRUE;
	}

	// Repair them if needed
	if (yLowerFlashValid == FALSE || yUpperFlashValid == FALSE) {
		SetEvent(hShutDownAppEvent);
		g_pDriver->SetSystemBuzzer(FALSE);
		WaitForSingleObject(hWaitForAppToShutDownMutex, 5000);
		BitBlt(::GetDC(NULL), 0, 0, 800, 480, NULL, 0, 0, BLACKNESS);
		g_pDriver->ResetDisplayBase();
		CSoftwareRepairDlg dlg(g_pDriver, yLowerFlashValid, yUpperFlashValid);
		dlg.DoModal();
	} else {
		do {
			// Look for the Software Upgrade file in USB Disk1 and Disk2
		
			// Check Disk1
			mem.GetBuffer(cTempBuf, UPGRADE_FILE_DISK1);
			pUpgradeFile = fopen(cTempBuf, "rb"); // Try to open the file
			if( pUpgradeFile == NULL)
			{
				// Check Disk 2
				mem.GetBuffer(cTempBuf, UPGRADE_FILE_DISK2);
				pUpgradeFile = fopen(cTempBuf, "rb"); // Try to open the file
				if( pUpgradeFile != NULL)
					mem.SetUpgradeFileName( cTempBuf, sizeof( cTempBuf)); // Save the path and file name
			}
			else
				mem.SetUpgradeFileName( cTempBuf, sizeof( cTempBuf));// Save the Path and file name
	  
				
			if (pUpgradeFile == NULL)
			{
				Sleep(100); // Did not find the software upgrade file 
			}
			else
			{
				// Now that we found the Software upgrade key, look for the Factory mode key. If we find
				// the Factory mode key then detailed status will be displayed during upgrade procedure
				
				// Check Disk1
				mem.GetBuffer(cTempBuf, FACTORY_MODE_FILE_DISK1);
				pFactoryModeFile = fopen(cTempBuf, "rb"); // Try to open the file
				if( pFactoryModeFile == NULL)
				{
					// Check Disk2
					mem.GetBuffer(cTempBuf, FACTORY_MODE_FILE_DISK2);
					pFactoryModeFile = fopen(cTempBuf, "rb"); // Try to open the file
					if( pFactoryModeFile != NULL)
						mem.SetStringDetailMode( TRUE);
					else
						mem.SetStringDetailMode( FALSE);
				}
				else
					mem.SetStringDetailMode( TRUE);
			}
 
		} while (pUpgradeFile == NULL && --qTries > 0);

		if (pFactoryModeFile)
			fclose(pFactoryModeFile);

		// Launch the Software Upgrade Screen and pass in a pointer to the Software Upgrade file
		if (pUpgradeFile) {
            // Turn off the system Heartbeat
            g_pDriver->WriteWordToDevice((offsetof(Status_Control, wHeartCount))/2, 0);
			SetEvent(hShutDownAppEvent);
			g_pDriver->SetSystemBuzzer(FALSE);
			WaitForSingleObject(hWaitForAppToShutDownMutex, 5000);
			BitBlt(::GetDC(NULL), 0, 0, 800, 480, NULL, 0, 0, BLACKNESS);
			g_pDriver->ResetDisplayBase();
			CSoftwareUpgradeDlg dlg(g_pDriver, pUpgradeFile);
			dlg.DoModal();

			fclose(pUpgradeFile);
		}
	}

	g_pDriver->DeInitDriver();

	delete g_pDriver;
	g_pDriver = NULL;

	// Since the dialog has been closed, return FALSE so that we exit the
	//	application, rather than start the application's message pump.
	return FALSE;
}
