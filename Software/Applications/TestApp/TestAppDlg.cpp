// TestAppDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "SnTypes.h"
#include "SnIoctl.h"
#include "TestApp.h"
#include "TestAppDlg.h"
#include "ControllerTypes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestAppDlg dialog

CTestAppDlg::CTestAppDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestAppDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestAppDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pParent = pParent;
	m_SaveData = FALSE;
	m_FileSetup = FALSE;
	m_SaveCount = 0;
	m_Sleep = 50; // default sleep time, 50 ms
	m_qMotorRev = 0xffffffff;

    m_hUpdateDisplayThread = NULL;
    m_hShutDownAppEvent = NULL;
}

CTestAppDlg::~CTestAppDlg()
{
	// Kill threads if running
	m_KillUpdateThread = TRUE;
	m_ShutDown = TRUE;
	
	if( m_hDriver)
	{
		m_hDriver->DeInitDriver();
		delete m_hDriver;
		m_hDriver = NULL;
	}

	if( m_hUpdateDisplayThread)
	{
		CloseHandle( m_hUpdateDisplayThread);
		m_hUpdateDisplayThread = NULL;
	}

	if( m_hShutDownAppEvent)
	{
		CloseHandle( m_hShutDownAppEvent);
		m_hShutDownAppEvent = NULL;
	}

	if( m_hLogger)
	{
		m_hLogger->DeInitLogger();
		delete m_hLogger;
		m_hLogger = NULL;
	}
}	

void CTestAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestAppDlg)
	DDX_Control(pDX, IDC_EDIT_READ_WRITE_CONFIG9, m_EditReadWriteConfig9);
	DDX_Control(pDX, IDC_EDIT_READ_WRITE_CONFIG8, m_EditReadWriteConfig8);
	DDX_Control(pDX, IDC_EDIT_READ_WRITE_CONFIG7, m_EditReadWriteConfig7);
	DDX_Control(pDX, IDC_EDIT_READ_WRITE_CONFIG12, m_EditReadWriteConfig12);
	DDX_Control(pDX, IDC_EDIT_READ_WRITE_CONFIG11, m_EditReadWriteConfig11);
	DDX_Control(pDX, IDC_EDIT_READ_WRITE_CONFIG10, m_EditReadWriteConfig10);
	DDX_Control(pDX, IDC_BUTTON_WRITE_WINDOW9, m_ButtonWriteWindow9);
	DDX_Control(pDX, IDC_BUTTON_WRITE_WINDOW8, m_ButtonWriteWindow8);
	DDX_Control(pDX, IDC_BUTTON_WRITE_WINDOW7, m_ButtonWriteWindow7);
	DDX_Control(pDX, IDC_BUTTON_WRITE_WINDOW12, m_ButtonWriteWindow12);
	DDX_Control(pDX, IDC_BUTTON_WRITE_WINDOW11, m_ButtonWriteWindow11);
	DDX_Control(pDX, IDC_BUTTON_WRITE_WINDOW10, m_ButtonWriteWindow10);
	DDX_Control(pDX, IDC_BUTTON_STOP_DATA_COLLECTION, m_StopDataCollection);
	DDX_Control(pDX, IDC_BUTTON_START_DATA_COLLECTION, m_StartDataCollection);
	DDX_Control(pDX, IDC_EDIT_READ_WRITE_CONFIG6, m_EditReadWriteConfig6);
	DDX_Control(pDX, IDC_EDIT_CONFIG_6_STATIC, m_EditConfig6Static);
	DDX_Control(pDX, IDC_EDIT_READ_WRITE_CONFIG5, m_EditReadWriteConfig5);
	DDX_Control(pDX, IDC_EDIT_CONFIG_5_STATIC, m_EditConfig5Static);
	DDX_Control(pDX, IDC_EDIT_READ_WRITE_CONFIG4, m_EditReadWriteConfig4);
	DDX_Control(pDX, IDC_EDIT_CONFIG_4_STATIC, m_EditConfig4Static);
	DDX_Control(pDX, IDC_EDIT_READ_WRITE_CONFIG3, m_EditReadWriteConfig3);
	DDX_Control(pDX, IDC_EDIT_CONFIG_3_STATIC, m_EditConfig3Static);
	DDX_Control(pDX, IDC_BUTTON_WRITE_WINDOW6, m_ButtonWriteWindow6);
	DDX_Control(pDX, IDC_BUTTON_WRITE_WINDOW5, m_ButtonWriteWindow5);
	DDX_Control(pDX, IDC_BUTTON_WRITE_WINDOW4, m_ButtonWriteWindow4);
	DDX_Control(pDX, IDC_BUTTON_WRITE_WINDOW3, m_ButtonWriteWindow3);
	DDX_Control(pDX, IDC_EDIT_READ_WRITE_CONFIG2, m_EditReadWriteConfig2);
	DDX_Control(pDX, IDC_EDIT_CONFIG_2_STATIC, m_EditConfig2Static);
	DDX_Control(pDX, IDC_BUTTON_WRITE_WINDOW2, m_ButtonWriteWindow2);
	DDX_Control(pDX, IDC_BUTTON_WRITE_WINDOW1, m_ButtonWriteWindow1);
	DDX_Control(pDX, IDC_EDIT_READ_WRITE_CONFIG1, m_EditReadWriteConfig1);
	DDX_Control(pDX, IDC_EDIT_CONFIG_1_STATIC, m_EditConfig1Static);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTestAppDlg, CDialog)
	//{{AFX_MSG_MAP(CTestAppDlg)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG_1, OnButtonConfig1)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_WINDOW1, OnButtonWriteWindow1)
	ON_BN_CLICKED(IDC_BUTTON_CAL, OnButtonCal)
	ON_EN_SETFOCUS(IDC_EDIT_READ_WRITE_CONFIG1, OnSetfocusEditReadWriteConfig1)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG_2, OnButtonConfig2)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_WINDOW2, OnButtonWriteWindow2)
	ON_EN_SETFOCUS(IDC_EDIT_READ_WRITE_CONFIG2, OnSetfocusEditReadWriteConfig2)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG_3, OnButtonConfig3)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_WINDOW3, OnButtonWriteWindow3)
	ON_EN_SETFOCUS(IDC_EDIT_READ_WRITE_CONFIG3, OnSetfocusEditReadWriteConfig3)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG_4, OnButtonConfig4)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_WINDOW4, OnButtonWriteWindow4)
	ON_EN_SETFOCUS(IDC_EDIT_READ_WRITE_CONFIG4, OnSetfocusEditReadWriteConfig4)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG_5, OnButtonConfig5)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_WINDOW5, OnButtonWriteWindow5)
	ON_EN_SETFOCUS(IDC_EDIT_READ_WRITE_CONFIG5, OnSetfocusEditReadWriteConfig5)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG_6, OnButtonConfig6)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_WINDOW6, OnButtonWriteWindow6)
	ON_EN_SETFOCUS(IDC_EDIT_READ_WRITE_CONFIG6, OnSetfocusEditReadWriteConfig6)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_CONFIGURATION, OnButtonSaveConfiguration)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG_7, OnButtonConfig7)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG_8, OnButtonConfig8)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG_9, OnButtonConfig9)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG_10, OnButtonConfig10)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG_11, OnButtonConfig11)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG_12, OnButtonConfig12)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_WINDOW7, OnButtonWriteWindow7)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_WINDOW8, OnButtonWriteWindow8)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_WINDOW9, OnButtonWriteWindow9)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_WINDOW10, OnButtonWriteWindow10)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_WINDOW11, OnButtonWriteWindow11)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_WINDOW12, OnButtonWriteWindow12)
	ON_EN_SETFOCUS(IDC_EDIT_READ_WRITE_CONFIG7, OnSetfocusEditReadWriteConfig7)
	ON_EN_SETFOCUS(IDC_EDIT_READ_WRITE_CONFIG8, OnSetfocusEditReadWriteConfig8)
	ON_EN_SETFOCUS(IDC_EDIT_READ_WRITE_CONFIG9, OnSetfocusEditReadWriteConfig9)
	ON_EN_SETFOCUS(IDC_EDIT_READ_WRITE_CONFIG10, OnSetfocusEditReadWriteConfig10)
	ON_EN_SETFOCUS(IDC_EDIT_READ_WRITE_CONFIG11, OnSetfocusEditReadWriteConfig11)
	ON_EN_SETFOCUS(IDC_EDIT_READ_WRITE_CONFIG12, OnSetfocusEditReadWriteConfig12)
	ON_BN_CLICKED(IDC_BUTTON_START_DATA_COLLECTION, OnButtonStartDataCollection)
	ON_BN_CLICKED(IDC_BUTTON_STOP_DATA_COLLECTION, OnButtonStopDataCollection)
	ON_EN_SETFOCUS(IDC_EDIT_DOWN_TIME, OnSetFocusChangeEditDownTime)
	ON_BN_CLICKED(IDC_BUTTON_SET_TIME, OnButtonSetTime)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestAppDlg message handlers

BOOL CTestAppDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
//	SetIcon(m_hIcon, TRUE);			// Set big icon
//	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	CenterWindow(GetDesktopWindow());	// center to the hpc screen

	m_hDriver = NULL;
	m_hUpdateDisplayThread = NULL;
	m_ShutDown = FALSE;
	
	m_hUpdateDisplayThread = NULL;

	m_WindowsDirty = FALSE;

	m_KillUpdateThread = FALSE;

	// Initialize Logger, use a 12 MB buffer
	m_hLogger = (CLogger*)new(CLogger);
    if( m_hLogger) {
		m_hLogger->InitLogger(12 * 0x100000);
    }

    // Initialize Driver
	m_hDriver = (CDriver*)new(CDriver);
    if( m_hDriver) {
        SnQByte qMotorRev = 0;

		m_hDriver->InitDriver();

	    // Turn off the system Heartbeat
        m_hDriver->WriteWordToDevice((offsetof(Status_Control, wHeartCount))/2, 0);

        m_hDriver->ResetDisplayBase();
    }
	
	SetDlgItemInt(IDC_EDIT_DOWN_TIME, m_Sleep);

	ShowTime();

	// get the configuration info from the TestApp.ini file
	RecallConfigurationInfo();
	
	// Setup the initial screen
	SetupScreen();
	
	// Hide buttons
	m_StopDataCollection.ShowWindow( SW_HIDE);
	
	// Create event
	m_hShutDownAppEvent = CreateEvent(NULL, FALSE, FALSE, _T("SnShutDownAppEvent"));
	if( m_hShutDownAppEvent == NULL)
		return FALSE;
	
	// Create thread to update the display with read data
	m_hUpdateDisplayThread = CreateThread(
						 (LPSECURITY_ATTRIBUTES)NULL,
					     0,
					     UpdateDisplayThread,
					     this,
					     0,
					     NULL);
    if (!m_hUpdateDisplayThread) 
        return FALSE;

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTestAppDlg::UpdateTitle(SnBool yForceUpdate)
{
	SnQByte qMotorRev = 0xffffffff;
	CString csTitle;

	// Get the revision numbers from the Motor Control Board
	if(m_hDriver) {
		m_hDriver->ReadWordsFromDevice(0, 2, (SnWord *)&qMotorRev);
	}

	if (yForceUpdate || (qMotorRev != m_qMotorRev)) {
		m_qMotorRev = qMotorRev;
		if (m_qMotorRev != 0xffffffff) {
			csTitle.Format(TEXT("TestApp: %d.%02d.%02d,   Motor Board: %d.%02d.%02d"),
				TESTAPP_VERS_MAJOR, TESTAPP_VERS_MINOR, TESTAPP_VERS_BUILD,
				(qMotorRev >> 16) & 0xff, (qMotorRev >> 8) & 0xff, qMotorRev & 0xff);
		} else {
			csTitle.Format(TEXT("TestApp: %d.%02d.%02d,   Motor Board: -.--.--"),
				TESTAPP_VERS_MAJOR, TESTAPP_VERS_MINOR, TESTAPP_VERS_BUILD);
		}
		SetWindowText(csTitle);
	}
}

void CTestAppDlg::SetupScreen(void)
{
	UpdateTitle(TRUE);

	// Setup Window 1
	if( m_ConfigOne.type == TYPE_WRITE_ONLY)
	{
		m_ButtonWriteWindow1.ShowWindow(SW_SHOW);
		m_EditReadWriteConfig1.SetReadOnly( FALSE);
	}
	else
	{
		m_ButtonWriteWindow1.ShowWindow(SW_HIDE);
	}

	SetDlgItemText(IDC_EDIT_CONFIG_1_STATIC, m_ConfigOne.name);

	// Setup Window 2
	if( m_ConfigTwo.type == TYPE_WRITE_ONLY)
	{
		m_ButtonWriteWindow2.ShowWindow(SW_SHOW);
		m_EditReadWriteConfig2.SetReadOnly( FALSE);
	}
	else
	{
		m_ButtonWriteWindow2.ShowWindow(SW_HIDE);
	}

	SetDlgItemText(IDC_EDIT_CONFIG_2_STATIC, m_ConfigTwo.name);

	// Setup Window 3
	if( m_ConfigThree.type == TYPE_WRITE_ONLY)
	{
		m_ButtonWriteWindow3.ShowWindow(SW_SHOW);
		m_EditReadWriteConfig3.SetReadOnly( FALSE);
	}
	else
	{
		m_ButtonWriteWindow3.ShowWindow(SW_HIDE);
	}

	SetDlgItemText(IDC_EDIT_CONFIG_3_STATIC, m_ConfigThree.name);

	// Setup Window 4
	if( m_ConfigFour.type == TYPE_WRITE_ONLY)
	{
		m_ButtonWriteWindow4.ShowWindow(SW_SHOW);
		m_EditReadWriteConfig4.SetReadOnly( FALSE);
	}
	else
	{
		m_ButtonWriteWindow4.ShowWindow(SW_HIDE);
	}

	SetDlgItemText(IDC_EDIT_CONFIG_4_STATIC, m_ConfigFour.name);

	// Setup Window 5
	if( m_ConfigFive.type == TYPE_WRITE_ONLY)
	{
		m_ButtonWriteWindow5.ShowWindow(SW_SHOW);
		m_EditReadWriteConfig5.SetReadOnly( FALSE);
	}
	else
	{
		m_ButtonWriteWindow5.ShowWindow(SW_HIDE);
	}

	SetDlgItemText(IDC_EDIT_CONFIG_5_STATIC, m_ConfigFive.name);
	
	// Setup Window 6
	if( m_ConfigSix.type == TYPE_WRITE_ONLY)
	{
		m_ButtonWriteWindow6.ShowWindow(SW_SHOW);
		m_EditReadWriteConfig6.SetReadOnly( FALSE);
	}
	else
	{
		m_ButtonWriteWindow6.ShowWindow(SW_HIDE);
	}

	SetDlgItemText(IDC_EDIT_CONFIG_6_STATIC, m_ConfigSix.name);

	// Setup Window 7
	if( m_ConfigSeven.type == TYPE_WRITE_ONLY)
	{
		m_ButtonWriteWindow7.ShowWindow(SW_SHOW);
		m_EditReadWriteConfig7.SetReadOnly( FALSE);
	}
	else
	{
		m_ButtonWriteWindow7.ShowWindow(SW_HIDE);
	}

	SetDlgItemText(IDC_EDIT_CONFIG_7_STATIC, m_ConfigSeven.name);

	// Setup Window 8
	if( m_ConfigEight.type == TYPE_WRITE_ONLY)
	{
		m_ButtonWriteWindow8.ShowWindow(SW_SHOW);
		m_EditReadWriteConfig8.SetReadOnly( FALSE);
	}
	else
	{
		m_ButtonWriteWindow8.ShowWindow(SW_HIDE);
	}

	SetDlgItemText(IDC_EDIT_CONFIG_8_STATIC, m_ConfigEight.name);

	// Setup Window 9
	if( m_ConfigNine.type == TYPE_WRITE_ONLY)
	{
		m_ButtonWriteWindow9.ShowWindow(SW_SHOW);
		m_EditReadWriteConfig9.SetReadOnly( FALSE);
	}
	else
	{
		m_ButtonWriteWindow9.ShowWindow(SW_HIDE);
	}

	SetDlgItemText(IDC_EDIT_CONFIG_9_STATIC, m_ConfigNine.name);

	// Setup Window 10
	if( m_ConfigTen.type == TYPE_WRITE_ONLY)
	{
		m_ButtonWriteWindow10.ShowWindow(SW_SHOW);
		m_EditReadWriteConfig10.SetReadOnly( FALSE);
	}
	else
	{
		m_ButtonWriteWindow10.ShowWindow(SW_HIDE);
	}

	SetDlgItemText(IDC_EDIT_CONFIG_10_STATIC, m_ConfigTen.name);

	// Setup Window 11
	if( m_ConfigEleven.type == TYPE_WRITE_ONLY)
	{
		m_ButtonWriteWindow11.ShowWindow(SW_SHOW);
		m_EditReadWriteConfig11.SetReadOnly( FALSE);
	}
	else
	{
		m_ButtonWriteWindow11.ShowWindow(SW_HIDE);
	}

	SetDlgItemText(IDC_EDIT_CONFIG_11_STATIC, m_ConfigEleven.name);

	// Setup Window 12
	if( m_ConfigTwelve.type == TYPE_WRITE_ONLY)
	{
		m_ButtonWriteWindow12.ShowWindow(SW_SHOW);
		m_EditReadWriteConfig12.SetReadOnly( FALSE);
	}
	else
	{
		m_ButtonWriteWindow12.ShowWindow(SW_HIDE);
	}

	SetDlgItemText(IDC_EDIT_CONFIG_12_STATIC, m_ConfigTwelve.name);
}

void CTestAppDlg::ConvertDlgItemNum(CONFIG *ptConfig, int iID)
{
    SnWord pwBuf[TMP_BUF_SIZE];
    SnBool yWasFloat = FALSE;
	SnQByte qData = 0;
    float fData;

    if (GetDlgItemText(iID, (LPTSTR)pwBuf, TMP_BUF_SIZE-1) > 0) {
        if (wcsncmp((const wchar_t *)pwBuf, TEXT("0x"), 2) == 0) {
            swscanf((const wchar_t *)pwBuf, TEXT("0x%x"), &qData);
        } else if (wcschr((const wchar_t *)pwBuf, '.') != NULL) {
            swscanf((const wchar_t *)pwBuf, TEXT("%f"), &fData);
            yWasFloat = TRUE;
        } else {
            swscanf((const wchar_t *)pwBuf, TEXT("%d"), &qData);
        }
    }
    if (ptConfig->format == FORMAT_FLOAT) {
        if (!yWasFloat) {
            fData = (float)qData;
        }
        qData = *(SnQByte *)&fData;
    } else if (yWasFloat) {
        qData = (SnQByte)fData;
    }

    UpdateControlInt(iID, qData, ptConfig, TRUE, TRUE);
}

void CTestAppDlg::OnButtonConfig1() 
{
	CConfigDlg	dlg;

	dlg.SetCurrentConfig( &m_ConfigOne);
	
	if (dlg.DoModal() == IDOK)
	{
		dlg.GetNewConfig( &m_ConfigOne);
		
		if( m_ConfigOne.type == TYPE_WRITE_ONLY)
		{
			m_ButtonWriteWindow1.ShowWindow(SW_SHOW);
			m_EditReadWriteConfig1.SetReadOnly( FALSE);
		}
		else
		{
			m_ButtonWriteWindow1.ShowWindow(SW_HIDE);
		}
		
		SetDlgItemText(IDC_EDIT_CONFIG_1_STATIC, m_ConfigOne.name);
        ConvertDlgItemNum(&m_ConfigOne, IDC_EDIT_READ_WRITE_CONFIG1);
		m_WindowsDirty = TRUE;	
	}		
}

void CTestAppDlg::OnButtonConfig2() 
{
	CConfigDlg	dlg;

	dlg.SetCurrentConfig( &m_ConfigTwo);
	
	if (dlg.DoModal() == IDOK)
	{
		dlg.GetNewConfig( &m_ConfigTwo);
		
		if( m_ConfigTwo.type == TYPE_WRITE_ONLY)
		{
			m_ButtonWriteWindow2.ShowWindow(SW_SHOW);
			m_EditReadWriteConfig2.SetReadOnly( FALSE);
		}
		else
		{
			m_ButtonWriteWindow2.ShowWindow(SW_HIDE);
		}
			
	    SetDlgItemText(IDC_EDIT_CONFIG_2_STATIC, m_ConfigTwo.name);
        ConvertDlgItemNum(&m_ConfigTwo, IDC_EDIT_READ_WRITE_CONFIG2);
	    m_WindowsDirty = TRUE;
	}	
}

void CTestAppDlg::OnButtonConfig3() 
{
	CConfigDlg	dlg;

	dlg.SetCurrentConfig( &m_ConfigThree);
	
	if (dlg.DoModal() == IDOK)
	{
		dlg.GetNewConfig( &m_ConfigThree);
		
		if( m_ConfigThree.type == TYPE_WRITE_ONLY)
		{
			m_ButtonWriteWindow3.ShowWindow(SW_SHOW);
			m_EditReadWriteConfig3.SetReadOnly( FALSE);
		}
		else
		{
			m_ButtonWriteWindow3.ShowWindow(SW_HIDE);
		}
			
		SetDlgItemText(IDC_EDIT_CONFIG_3_STATIC, m_ConfigThree.name);
        ConvertDlgItemNum(&m_ConfigThree, IDC_EDIT_READ_WRITE_CONFIG3);
		m_WindowsDirty = TRUE;		
	}	
}

void CTestAppDlg::OnButtonConfig4() 
{
	CConfigDlg	dlg;

	// Set the current configuration for window 4
	dlg.SetCurrentConfig( &m_ConfigFour);
	
	if (dlg.DoModal() == IDOK)
	{
		// get the new configuration for window 4
		dlg.GetNewConfig( &m_ConfigFour);
		
		if( m_ConfigFour.type == TYPE_WRITE_ONLY)
		{
			m_ButtonWriteWindow4.ShowWindow(SW_SHOW);
			m_EditReadWriteConfig4.SetReadOnly( FALSE);
		}
		else
		{
			m_ButtonWriteWindow4.ShowWindow(SW_HIDE);
		}

		SetDlgItemText(IDC_EDIT_CONFIG_4_STATIC, m_ConfigFour.name);
        ConvertDlgItemNum(&m_ConfigFour, IDC_EDIT_READ_WRITE_CONFIG4);
		m_WindowsDirty = TRUE;
	}
}

void CTestAppDlg::OnButtonConfig5() 
{
	CConfigDlg	dlg;

	// Set the current configuration for window 5
	dlg.SetCurrentConfig( &m_ConfigFive);
	
	if (dlg.DoModal() == IDOK)
	{
		// get the new configuration for window 5
		dlg.GetNewConfig( &m_ConfigFive);
		
		if( m_ConfigFive.type == TYPE_WRITE_ONLY)
		{
			m_ButtonWriteWindow5.ShowWindow(SW_SHOW);
			m_EditReadWriteConfig5.SetReadOnly( FALSE);
		}
		else
		{
			m_ButtonWriteWindow5.ShowWindow(SW_HIDE);
		}
			
		SetDlgItemText(IDC_EDIT_CONFIG_5_STATIC, m_ConfigFive.name);
        ConvertDlgItemNum(&m_ConfigFive, IDC_EDIT_READ_WRITE_CONFIG5);
		m_WindowsDirty = TRUE;
	}
}

void CTestAppDlg::OnButtonConfig6() 
{
	CConfigDlg	dlg;

	// Set the current configuration for window 6
	dlg.SetCurrentConfig( &m_ConfigSix);
	
	if (dlg.DoModal() == IDOK)
	{
		// get the new configuration for window 6
		dlg.GetNewConfig( &m_ConfigSix);
		
		if( m_ConfigSix.type == TYPE_WRITE_ONLY)
		{
			m_ButtonWriteWindow6.ShowWindow(SW_SHOW);
			m_EditReadWriteConfig6.SetReadOnly( FALSE);
		}
		else
		{
			m_ButtonWriteWindow6.ShowWindow(SW_HIDE);
		}
			
		SetDlgItemText(IDC_EDIT_CONFIG_6_STATIC, m_ConfigSix.name);
        ConvertDlgItemNum(&m_ConfigSix, IDC_EDIT_READ_WRITE_CONFIG6);
		m_WindowsDirty = TRUE;	
	}
}

void CTestAppDlg::OnButtonConfig7() 
{
	CConfigDlg	dlg;

	// Set the current configuration for window 7
	dlg.SetCurrentConfig( &m_ConfigSeven);
	
	if (dlg.DoModal() == IDOK)
	{
		// get the new configuration for window 7
		dlg.GetNewConfig( &m_ConfigSeven);
		
		if( m_ConfigSeven.type == TYPE_WRITE_ONLY)
		{
			m_ButtonWriteWindow7.ShowWindow(SW_SHOW);
			m_EditReadWriteConfig7.SetReadOnly( FALSE);
		}
		else
		{
			m_ButtonWriteWindow7.ShowWindow(SW_HIDE);
		}
		
		SetDlgItemText(IDC_EDIT_CONFIG_7_STATIC, m_ConfigSeven.name);		
        ConvertDlgItemNum(&m_ConfigSeven, IDC_EDIT_READ_WRITE_CONFIG7);
		m_WindowsDirty = TRUE;	
	}
}

void CTestAppDlg::OnButtonConfig8() 
{
	CConfigDlg	dlg;

	// Set the current configuration for window 8
	dlg.SetCurrentConfig( &m_ConfigEight);
	
	if (dlg.DoModal() == IDOK)
	{
		// get the new configuration for window 8
		dlg.GetNewConfig( &m_ConfigEight);
		
		if( m_ConfigEight.type == TYPE_WRITE_ONLY)
		{
			m_ButtonWriteWindow8.ShowWindow(SW_SHOW);
			m_EditReadWriteConfig8.SetReadOnly( FALSE);
		}
		else
		{
			m_ButtonWriteWindow8.ShowWindow(SW_HIDE);
		}
		
		SetDlgItemText(IDC_EDIT_CONFIG_8_STATIC, m_ConfigEight.name);
        ConvertDlgItemNum(&m_ConfigEight, IDC_EDIT_READ_WRITE_CONFIG8);
		m_WindowsDirty = TRUE;	
	}
}

void CTestAppDlg::OnButtonConfig9() 
{
	CConfigDlg	dlg;

	// Set the current configuration for window 9
	dlg.SetCurrentConfig( &m_ConfigNine);
	
	if (dlg.DoModal() == IDOK)
	{
		// get the new configuration for window 9
		dlg.GetNewConfig( &m_ConfigNine);
		
		if( m_ConfigNine.type == TYPE_WRITE_ONLY)
		{
			m_ButtonWriteWindow9.ShowWindow(SW_SHOW);
			m_EditReadWriteConfig9.SetReadOnly( FALSE);
		}
		else
		{
			m_ButtonWriteWindow9.ShowWindow(SW_HIDE);
		}
		
		SetDlgItemText(IDC_EDIT_CONFIG_9_STATIC, m_ConfigNine.name);
        ConvertDlgItemNum(&m_ConfigNine, IDC_EDIT_READ_WRITE_CONFIG9);
		m_WindowsDirty = TRUE;	
	}
}

void CTestAppDlg::OnButtonConfig10() 
{
	CConfigDlg	dlg;

	// Set the current configuration for window 10
	dlg.SetCurrentConfig( &m_ConfigTen);
	
	if (dlg.DoModal() == IDOK)
	{
		// get the new configuration for window 10
		dlg.GetNewConfig( &m_ConfigTen);
		
		if( m_ConfigTen.type == TYPE_WRITE_ONLY)
		{
			m_ButtonWriteWindow10.ShowWindow(SW_SHOW);
			m_EditReadWriteConfig10.SetReadOnly( FALSE);
		}
		else
		{
			m_ButtonWriteWindow10.ShowWindow(SW_HIDE);
		}
		
		SetDlgItemText(IDC_EDIT_CONFIG_10_STATIC, m_ConfigTen.name);
        ConvertDlgItemNum(&m_ConfigTen, IDC_EDIT_READ_WRITE_CONFIG10);
		m_WindowsDirty = TRUE;	
	}
}

void CTestAppDlg::OnButtonConfig11() 
{
	CConfigDlg	dlg;

	// Set the current configuration for window 11
	dlg.SetCurrentConfig( &m_ConfigEleven);
	
	if (dlg.DoModal() == IDOK)
	{
		// get the new configuration for window 11
		dlg.GetNewConfig( &m_ConfigEleven);
		
		if( m_ConfigEleven.type == TYPE_WRITE_ONLY)
		{
			m_ButtonWriteWindow11.ShowWindow(SW_SHOW);
			m_EditReadWriteConfig11.SetReadOnly( FALSE);
		}
		else
		{
			m_ButtonWriteWindow11.ShowWindow(SW_HIDE);
		}
		
		SetDlgItemText(IDC_EDIT_CONFIG_11_STATIC, m_ConfigEleven.name);
        ConvertDlgItemNum(&m_ConfigEleven, IDC_EDIT_READ_WRITE_CONFIG11);
		m_WindowsDirty = TRUE;	
	}
}

void CTestAppDlg::OnButtonConfig12() 
{
	CConfigDlg	dlg;

	// Set the current configuration for window 12
	dlg.SetCurrentConfig( &m_ConfigTwelve);
	
	if (dlg.DoModal() == IDOK)
	{
		// get the new configuration for window 12
		dlg.GetNewConfig( &m_ConfigTwelve);
		
		if( m_ConfigTwelve.type == TYPE_WRITE_ONLY)
		{
			m_ButtonWriteWindow12.ShowWindow(SW_SHOW);
			m_EditReadWriteConfig12.SetReadOnly( FALSE);
		}
		else
		{
			m_ButtonWriteWindow12.ShowWindow(SW_HIDE);
		}
		
		SetDlgItemText(IDC_EDIT_CONFIG_12_STATIC, m_ConfigTwelve.name);
        ConvertDlgItemNum(&m_ConfigTwelve, IDC_EDIT_READ_WRITE_CONFIG12);
		m_WindowsDirty = TRUE;	
	}
}

void CTestAppDlg::OnSetfocusEditReadWriteConfig1() 
{	
	CKeypadDlg dlg(IDC_EDIT_READ_WRITE_CONFIG1, this);


    if( dlg.DoModal() == IDOK) {
        ConvertDlgItemNum(&m_ConfigOne, IDC_EDIT_READ_WRITE_CONFIG1);
		m_WindowsDirty = TRUE; // Set the window dirty flag to indicate data changed and should be saved
    }
}

void CTestAppDlg::OnSetfocusEditReadWriteConfig2() 
{	
	CKeypadDlg dlg(IDC_EDIT_READ_WRITE_CONFIG2, this);

    if( dlg.DoModal() == IDOK) {
        ConvertDlgItemNum(&m_ConfigTwo, IDC_EDIT_READ_WRITE_CONFIG2);
		m_WindowsDirty = TRUE; // Set the window dirty flag to indicate data changed and should be saved
    }
}

void CTestAppDlg::OnSetfocusEditReadWriteConfig3() 
{	
	CKeypadDlg dlg(IDC_EDIT_READ_WRITE_CONFIG3, this);

    if( dlg.DoModal() == IDOK) {
        ConvertDlgItemNum(&m_ConfigThree, IDC_EDIT_READ_WRITE_CONFIG3);
		m_WindowsDirty = TRUE; // Set the window dirty flag to indicate data changed and should be saved
    }
}

void CTestAppDlg::OnSetfocusEditReadWriteConfig4() 
{	
	CKeypadDlg dlg(IDC_EDIT_READ_WRITE_CONFIG4, this);

    if( dlg.DoModal() == IDOK) {
        ConvertDlgItemNum(&m_ConfigFour, IDC_EDIT_READ_WRITE_CONFIG4);
		m_WindowsDirty = TRUE; // Set the window dirty flag to indicate data changed and should be saved
    }
}

void CTestAppDlg::OnSetfocusEditReadWriteConfig5() 
{	
	CKeypadDlg dlg(IDC_EDIT_READ_WRITE_CONFIG5, this);

    if( dlg.DoModal() == IDOK) {
        ConvertDlgItemNum(&m_ConfigFive, IDC_EDIT_READ_WRITE_CONFIG5);
		m_WindowsDirty = TRUE; // Set the window dirty flag to indicate data changed and should be saved
    }
}

void CTestAppDlg::OnSetfocusEditReadWriteConfig6() 
{	
	CKeypadDlg dlg(IDC_EDIT_READ_WRITE_CONFIG6, this);

    if( dlg.DoModal() == IDOK) {
        ConvertDlgItemNum(&m_ConfigSix, IDC_EDIT_READ_WRITE_CONFIG6);
		m_WindowsDirty = TRUE; // Set the window dirty flag to indicate data changed and should be saved
    }
}

void CTestAppDlg::OnSetfocusEditReadWriteConfig7() 
{	
	CKeypadDlg dlg(IDC_EDIT_READ_WRITE_CONFIG7, this);

    if( dlg.DoModal() == IDOK) {
        ConvertDlgItemNum(&m_ConfigSeven, IDC_EDIT_READ_WRITE_CONFIG7);
		m_WindowsDirty = TRUE; // Set the window dirty flag to indicate data changed and should be saved
    }
}

void CTestAppDlg::OnSetfocusEditReadWriteConfig8() 
{	
	CKeypadDlg dlg(IDC_EDIT_READ_WRITE_CONFIG8, this);

    if( dlg.DoModal() == IDOK) {
        ConvertDlgItemNum(&m_ConfigEight, IDC_EDIT_READ_WRITE_CONFIG8);
		m_WindowsDirty = TRUE; // Set the window dirty flag to indicate data changed and should be saved
    }
}

void CTestAppDlg::OnSetfocusEditReadWriteConfig9() 
{	
	CKeypadDlg dlg(IDC_EDIT_READ_WRITE_CONFIG9, this);

    if( dlg.DoModal() == IDOK) {
        ConvertDlgItemNum(&m_ConfigNine, IDC_EDIT_READ_WRITE_CONFIG9);
		m_WindowsDirty = TRUE; // Set the window dirty flag to indicate data changed and should be saved
    }
}

void CTestAppDlg::OnSetfocusEditReadWriteConfig10() 
{	
	CKeypadDlg dlg(IDC_EDIT_READ_WRITE_CONFIG10, this);

    if( dlg.DoModal() == IDOK) {
        ConvertDlgItemNum(&m_ConfigTen, IDC_EDIT_READ_WRITE_CONFIG10);
		m_WindowsDirty = TRUE; // Set the window dirty flag to indicate data changed and should be saved
    }
}

void CTestAppDlg::OnSetfocusEditReadWriteConfig11() 
{	
	CKeypadDlg dlg(IDC_EDIT_READ_WRITE_CONFIG11, this);

    if( dlg.DoModal() == IDOK) {
        ConvertDlgItemNum(&m_ConfigEleven, IDC_EDIT_READ_WRITE_CONFIG11);
		m_WindowsDirty = TRUE; // Set the window dirty flag to indicate data changed and should be saved
    }
}

void CTestAppDlg::OnSetfocusEditReadWriteConfig12() 
{	
	CKeypadDlg dlg(IDC_EDIT_READ_WRITE_CONFIG12, this);

    if( dlg.DoModal() == IDOK) {
        ConvertDlgItemNum(&m_ConfigTwelve, IDC_EDIT_READ_WRITE_CONFIG12);
		m_WindowsDirty = TRUE; // Set the window dirty flag to indicate data changed and should be saved
    }
}

SnQByte CTestAppDlg::GetDlgItemNum(int iID)
{
    SnWord pwBuf[TMP_BUF_SIZE];
	SnQByte qData = 0;

    if (GetDlgItemText(iID, (LPTSTR)pwBuf, TMP_BUF_SIZE-1) > 0) {
        if (wcsncmp((const wchar_t *)pwBuf, TEXT("0x"), 2) == 0) {
            swscanf((const wchar_t *)pwBuf, TEXT("0x%x"), &qData);
        } else if (wcschr((const wchar_t *)pwBuf, '.') != NULL) {
            swscanf((const wchar_t *)pwBuf, TEXT("%f"), &qData);
        } else {
            swscanf((const wchar_t *)pwBuf, TEXT("%d"), &qData);
        }
    }
	
    return qData;
}

SnBool CTestAppDlg::WriteWindowData(CONFIG *ptConfig, int iID)
{
    SnWord pwBuf[2];
	SnQByte qData = GetDlgItemNum(iID);
	SnBool yStatus;

	if( ptConfig->bits == BITS_16)
	{
		// do 16 bit write
		pwBuf[0] = (SnWord)qData;
        if( m_hDriver != NULL) {
			yStatus = m_hDriver->WriteWordToDevice(ptConfig->offset, pwBuf[0]);
        }
	}
	else
	{
		// do 32 bit write
		pwBuf[0] = (SnWord)qData;
		qData = qData >> 16; // shift data to the right 16 times
		pwBuf[1] = (SnWord)qData;
        if( m_hDriver != NULL) {
			yStatus = m_hDriver->WriteWordsToDevice(ptConfig->offset, 2, pwBuf);
        }
	}

    return yStatus;
}

void CTestAppDlg::OnButtonWriteWindow1() 
{
    WriteWindowData(&m_ConfigOne, IDC_EDIT_READ_WRITE_CONFIG1);
}

void CTestAppDlg::OnButtonWriteWindow2() 
{
    WriteWindowData(&m_ConfigTwo, IDC_EDIT_READ_WRITE_CONFIG2);
}

void CTestAppDlg::OnButtonWriteWindow3() 
{
    WriteWindowData(&m_ConfigThree, IDC_EDIT_READ_WRITE_CONFIG3);
}

void CTestAppDlg::OnButtonWriteWindow4() 
{
    WriteWindowData(&m_ConfigFour, IDC_EDIT_READ_WRITE_CONFIG4);
}

void CTestAppDlg::OnButtonWriteWindow5() 
{
    WriteWindowData(&m_ConfigFive, IDC_EDIT_READ_WRITE_CONFIG5);
}

void CTestAppDlg::OnButtonWriteWindow6() 
{
    WriteWindowData(&m_ConfigSix, IDC_EDIT_READ_WRITE_CONFIG6);
}

void CTestAppDlg::OnButtonWriteWindow7() 
{
    WriteWindowData(&m_ConfigSeven, IDC_EDIT_READ_WRITE_CONFIG7);
}

void CTestAppDlg::OnButtonWriteWindow8() 
{
    WriteWindowData(&m_ConfigEight, IDC_EDIT_READ_WRITE_CONFIG8);
}

void CTestAppDlg::OnButtonWriteWindow9() 
{
    WriteWindowData(&m_ConfigNine, IDC_EDIT_READ_WRITE_CONFIG9);
}

void CTestAppDlg::OnButtonWriteWindow10() 
{
    WriteWindowData(&m_ConfigTen, IDC_EDIT_READ_WRITE_CONFIG10);
}

void CTestAppDlg::OnButtonWriteWindow11() 
{
    WriteWindowData(&m_ConfigEleven, IDC_EDIT_READ_WRITE_CONFIG11);
}

void CTestAppDlg::OnButtonWriteWindow12() 
{
    WriteWindowData(&m_ConfigTwelve, IDC_EDIT_READ_WRITE_CONFIG12);
}

void CTestAppDlg::OnButtonCal() 
{
    PROCESS_INFORMATION tProc;

    if (CreateProcess(TEXT("TouchCalibrate"), NULL,NULL, NULL, FALSE, 0, NULL, NULL, NULL, &tProc)) {
        DWORD dwSize, dwType, dwRet;
        SnWord pwTouchData[64];
        SnQByte pqSavedData[6];
        DWORD dwX0, dwY0;
        DWORD dwX1, dwY1;
        DWORD dwX2, dwY2;
        DWORD dwX3, dwY3;
        DWORD dwX4, dwY4;
        SnByte bCrc = 0;
        HKEY hKey;
        SnBool yFlash = FALSE;

        WaitForSingleObject(tProc.hProcess, INFINITE);

        CloseHandle(tProc.hProcess);
        CloseHandle(tProc.hThread);

        // Open the Touch key
        dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\TOUCH"), 0, 0, &hKey);
        if (dwRet != ERROR_SUCCESS)
            return;

        // Read the new Calibration Data
        dwSize = sizeof(pwTouchData);
        dwRet = RegQueryValueEx(hKey, TEXT("CalibrationData"), NULL, &dwType, (LPBYTE)pwTouchData, &dwSize);
        if (dwRet != ERROR_SUCCESS)
            return;

        swscanf(pwTouchData, TEXT("%d,%d %d,%d %d,%d %d,%d %d,%d "), &dwX0, &dwY0, &dwX1, &dwY1, &dwX2, &dwY2,
            &dwX3, &dwY3, &dwX4, &dwY4);
        
        pqSavedData[0] = (dwX0 << 16) | dwY0;
        pqSavedData[1] = (dwX1 << 16) | dwY1;
        pqSavedData[2] = (dwX2 << 16) | dwY2;
        pqSavedData[3] = (dwX3 << 16) | dwY3;
        pqSavedData[4] = (dwX4 << 16) | dwY4;
        if (m_hDriver->CrcBufData((SnByte *)pqSavedData, 20, &bCrc) &&
           m_hDriver->EraseFlashPages(FLASH_TOUCH_DATA_OFFSET, 1)) {
            pqSavedData[5] = (SnQByte)bCrc;
            if (m_hDriver->WriteFlashData(FLASH_TOUCH_DATA_OFFSET, (SnByte *)pqSavedData, 24))
                yFlash = TRUE;
        }

        MessageBox(pwTouchData, yFlash ? TEXT("Saved Touch Data") : TEXT("Tmp Touch Data"), MB_OK);

        RegCloseKey(hKey);
    }
}

void CTestAppDlg::OnButtonSaveConfiguration() 
{
	SaveConfigurationInfo();
}

void CTestAppDlg::GetBuffer( char* dest, CString csSource)
{
	CString csTmp;
	int length = 0;
	int i;
	LPTSTR  pBuffer;

	csTmp = csSource;
	length = csTmp.GetLength();
	length = length + 1; // add 1 for the null character

	pBuffer = csTmp.GetBuffer( length);
	for(i=0;i<length;i++)
	{
		dest[i] = (char)*pBuffer;
		pBuffer++;
	}
}

const SnByte pbCrcTable[] = 
{
/*0*/   0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
/*1*/ 157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
/*2*/  35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
/*3*/ 190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
/*4*/  70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
/*5*/ 219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
/*6*/ 101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
/*7*/ 248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
/*8*/ 140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
/*9*/  17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
/*a*/ 175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
/*b*/  50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
/*c*/ 202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
/*d*/  87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
/*e*/ 233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
/*f*/ 116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
//     0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f
};

SnByte CrcMem(SnByte *pbSrc,SnQByte qLen)
{
    SnByte bCrc = 0;

    do {
        bCrc = pbCrcTable[bCrc ^ *pbSrc++];
    } while (--qLen > 0);

	return bCrc;
}

void CTestAppDlg::SaveWindow(CONFIG *ptConfig, FlashConfig *ptFlashConfig, int iID)
{
	GetBuffer(ptFlashConfig->pcName, ptConfig->name); // convert cstring to char*
    ptFlashConfig->qBits = ptConfig->bits;
    ptFlashConfig->qType = ptConfig->type;
    ptFlashConfig->qOffset = ptConfig->offset;
    ptFlashConfig->qFormat = ptConfig->format;
    if (ptConfig->type == TYPE_WRITE_ONLY) {
        ptFlashConfig->qWriteData = GetDlgItemNum(iID);
    } else {
        ptFlashConfig->qWriteData = 0;
    }
}

SnBool CTestAppDlg::SaveConfigurationInfo()
{
    FlashConfig ptWindows[13];
    SnBool yRet = TRUE;

    if (m_WindowsDirty) {
        SaveWindow(&m_ConfigOne, &ptWindows[0], IDC_EDIT_READ_WRITE_CONFIG1);
        SaveWindow(&m_ConfigTwo, &ptWindows[1], IDC_EDIT_READ_WRITE_CONFIG2);
        SaveWindow(&m_ConfigThree, &ptWindows[2], IDC_EDIT_READ_WRITE_CONFIG3);
        SaveWindow(&m_ConfigFour, &ptWindows[3], IDC_EDIT_READ_WRITE_CONFIG4);
        SaveWindow(&m_ConfigFive, &ptWindows[4], IDC_EDIT_READ_WRITE_CONFIG5);
        SaveWindow(&m_ConfigSix, &ptWindows[5], IDC_EDIT_READ_WRITE_CONFIG6);
        SaveWindow(&m_ConfigSeven, &ptWindows[6], IDC_EDIT_READ_WRITE_CONFIG7);
        SaveWindow(&m_ConfigEight, &ptWindows[7], IDC_EDIT_READ_WRITE_CONFIG8);
        SaveWindow(&m_ConfigNine, &ptWindows[8], IDC_EDIT_READ_WRITE_CONFIG9);
        SaveWindow(&m_ConfigTen, &ptWindows[9], IDC_EDIT_READ_WRITE_CONFIG10);
        SaveWindow(&m_ConfigEleven, &ptWindows[10], IDC_EDIT_READ_WRITE_CONFIG11);
        SaveWindow(&m_ConfigTwelve, &ptWindows[11], IDC_EDIT_READ_WRITE_CONFIG12);

        if (m_hDriver) {
            *(SnQByte *)&ptWindows[12] = CrcMem((SnByte *)&ptWindows[0], sizeof(FlashConfig) * 12);
            yRet = m_hDriver->EraseFlashPages(FLASH_TESTAPP_OFFSET, 1);
            if (yRet)
                yRet = m_hDriver->WriteFlashData(FLASH_TESTAPP_OFFSET, (SnByte *)&ptWindows[0], (sizeof(FlashConfig) * 12) + 4);
            if (yRet) {
                m_WindowsDirty = FALSE;
            }
        } else {
            yRet = FALSE;
        }
    }

    return yRet;
}

void CTestAppDlg::RecallWindow(CONFIG *ptConfig, FlashConfig *ptFlashConfig, int iID)
{
	ptConfig->name = ptFlashConfig->pcName;
    ptConfig->bits = ptFlashConfig->qBits;
    ptConfig->type = ptFlashConfig->qType;
    ptConfig->offset = ptFlashConfig->qOffset;
    ptConfig->format = ptFlashConfig->qFormat;
    if (ptFlashConfig->qType == TYPE_WRITE_ONLY) {
        UpdateControlInt(iID, ptFlashConfig->qWriteData, ptConfig, TRUE, TRUE);
    }
}

FlashConfig ptWindowDefault[12] = {
    { "Window1", 0, 0, 0, 0, FORMAT_UNSIGNED },
    { "Window2", 0, 0, 0, 0, FORMAT_UNSIGNED },
    { "Window3", 0, 0, 0, 0, FORMAT_UNSIGNED },
    { "Window4", 0, 0, 0, 0, FORMAT_UNSIGNED },
    { "Window5", 0, 0, 0, 0, FORMAT_UNSIGNED },
    { "Window6", 0, 0, 0, 0, FORMAT_UNSIGNED },
    { "Window7", 0, 0, 0, 0, FORMAT_UNSIGNED },
    { "Window8", 0, 0, 0, 0, FORMAT_UNSIGNED },
    { "Window9", 0, 0, 0, 0, FORMAT_UNSIGNED },
    { "Window10", 0, 0, 0, 0, FORMAT_UNSIGNED },
    { "Window11", 0, 0, 0, 0, FORMAT_UNSIGNED },
    { "Window12", 0, 0, 0, 0, FORMAT_UNSIGNED }
};

SnBool CTestAppDlg::RecallConfigurationInfo()
{
    FlashConfig ptWindows[13];
    SnQByte *pqCrc = (SnQByte *)&ptWindows[12];
    SnByte bOldCrc, bNewCrc;
    SnBool yRet;

    if (m_hDriver) {
        memset(ptWindows, 0xff, sizeof(FlashConfig) * 13);
        yRet = m_hDriver->ReadFlashData(FLASH_TESTAPP_OFFSET, (SnByte *)&ptWindows[0], (sizeof(FlashConfig) * 12) + 4);
        if (yRet == FALSE) {
            return FALSE;
        }
        bOldCrc = (SnByte)*pqCrc;
        bNewCrc = CrcMem((SnByte *)&ptWindows[0], sizeof(FlashConfig) * 12);
        if (bNewCrc != bOldCrc) {
            memcpy(&ptWindows[0], &ptWindowDefault[0], sizeof(FlashConfig) * 12);
            *pqCrc = (SnQByte)bNewCrc;
            yRet = m_hDriver->EraseFlashPages(FLASH_TESTAPP_OFFSET, 1);
            yRet = m_hDriver->WriteFlashData(FLASH_TESTAPP_OFFSET, (SnByte *)&ptWindows[0], (sizeof(FlashConfig) * 12) + 4);
        }
    }

    RecallWindow(&m_ConfigOne, &ptWindows[0], IDC_EDIT_READ_WRITE_CONFIG1);
    RecallWindow(&m_ConfigTwo, &ptWindows[1], IDC_EDIT_READ_WRITE_CONFIG2);
    RecallWindow(&m_ConfigThree, &ptWindows[2], IDC_EDIT_READ_WRITE_CONFIG3);
    RecallWindow(&m_ConfigFour, &ptWindows[3], IDC_EDIT_READ_WRITE_CONFIG4);
    RecallWindow(&m_ConfigFive, &ptWindows[4], IDC_EDIT_READ_WRITE_CONFIG5);
    RecallWindow(&m_ConfigSix, &ptWindows[5], IDC_EDIT_READ_WRITE_CONFIG6);
    RecallWindow(&m_ConfigSeven, &ptWindows[6], IDC_EDIT_READ_WRITE_CONFIG7);
    RecallWindow(&m_ConfigEight, &ptWindows[7], IDC_EDIT_READ_WRITE_CONFIG8);
    RecallWindow(&m_ConfigNine, &ptWindows[8], IDC_EDIT_READ_WRITE_CONFIG9);
    RecallWindow(&m_ConfigTen, &ptWindows[9], IDC_EDIT_READ_WRITE_CONFIG10);
    RecallWindow(&m_ConfigEleven, &ptWindows[10], IDC_EDIT_READ_WRITE_CONFIG11);
    RecallWindow(&m_ConfigTwelve, &ptWindows[11], IDC_EDIT_READ_WRITE_CONFIG12);

    return TRUE;
}

void CTestAppDlg::OnButtonStartDataCollection() 
{	
	SYSTEMTIME theTime;
	char pcTimeBuf[TMP_BUF_SIZE];
	
    if (m_hLogger && m_hLogger->LoggerIsInitialized())
	{	
	    // Hide the Start button and show the stop button
	    m_StartDataCollection.ShowWindow( SW_HIDE);
	    m_StopDataCollection.ShowWindow( SW_SHOW);
	    
	    m_SaveCount = 0;	// set the number of windows to save count
	    m_SetupCount = 0;	// Set the setup count to zero
	    m_FileSetup = FALSE;

        GetSystemTime(&theTime);

        sprintf(pcTimeBuf, "Start: %d/%d/%d %02d:%02d:%02d\r\n\r\n",
          theTime.wMonth, theTime.wDay, theTime.wYear,
          theTime.wHour, theTime.wMinute, theTime.wSecond);

        m_hLogger->AddString(pcTimeBuf);

		// Set the save flags for windows that are readable
		if( m_ConfigOne.type == TYPE_READ_ONLY)
		{
			m_SaveCount++;
		}
		if( m_ConfigTwo.type == TYPE_READ_ONLY)
		{
			m_SaveCount++;
		}
		if( m_ConfigThree.type == TYPE_READ_ONLY)
		{
			m_SaveCount++;
		}
		if( m_ConfigFour.type == TYPE_READ_ONLY)
		{
			m_SaveCount++;
		}
		if( m_ConfigFive.type == TYPE_READ_ONLY)
		{
			m_SaveCount++;
		}
		if( m_ConfigSix.type == TYPE_READ_ONLY)
		{
			m_SaveCount++;
		}
		if( m_ConfigSeven.type == TYPE_READ_ONLY)
		{
			m_SaveCount++;
		}
		if( m_ConfigEight.type == TYPE_READ_ONLY)
		{
			m_SaveCount++;
		}
		if( m_ConfigNine.type == TYPE_READ_ONLY)
		{
			m_SaveCount++;
		}
		if( m_ConfigTen.type == TYPE_READ_ONLY)
		{
			m_SaveCount++;
		}
		if( m_ConfigEleven.type == TYPE_READ_ONLY)
		{
			m_SaveCount++;
		}
		if( m_ConfigTwelve.type == TYPE_READ_ONLY)
		{
			m_SaveCount++;
		}
		m_SaveData = TRUE;
	}
}

void CTestAppDlg::OnButtonStopDataCollection() 
{
    SYSTEMTIME theTime;
	char pcTimeBuf[TMP_BUF_SIZE];
	
    if (m_hLogger && m_hLogger->LoggerIsInitialized())
	{	
        m_SaveData = FALSE;	

        GetSystemTime(&theTime);

        sprintf(pcTimeBuf, "\r\n\r\nStop: %d/%d/%d %02d:%02d:%02d",
          theTime.wMonth, theTime.wDay, theTime.wYear,
          theTime.wHour, theTime.wMinute, theTime.wSecond);
		
        m_hLogger->AddString(pcTimeBuf);
        m_hLogger->SaveData(FILE_NAME_DATA_COLLECTION);

        // Hide the Stop button and show the Start button
	    m_StartDataCollection.ShowWindow( SW_SHOW);
	    m_StopDataCollection.ShowWindow( SW_HIDE);
    }
}

void CTestAppDlg::ShowTime()
{
    static SYSTEMTIME tLastTime = {0};
	char pcTimeBuf[TMP_BUF_SIZE];
    SYSTEMTIME tCurTime;

    GetSystemTime(&tCurTime);
    if (tCurTime.wSecond != tLastTime.wSecond ||
      tCurTime.wMinute != tLastTime.wMinute ||
      tCurTime.wHour != tLastTime.wHour ||
      tCurTime.wDay != tLastTime.wDay ||
      tCurTime.wMonth != tLastTime.wMonth ||
      tCurTime.wYear != tLastTime.wYear) {
        sprintf(pcTimeBuf, "%d/%d/%d %02d:%02d:%02d",
          tCurTime.wMonth, tCurTime.wDay, tCurTime.wYear,
          tCurTime.wHour, tCurTime.wMinute, tCurTime.wSecond);
	
	    CString csTime = pcTimeBuf;
	    SetDlgItemText(IDC_EDIT_TIME, csTime);

        tLastTime = tCurTime;
    }
}

void CTestAppDlg::OnSetFocusChangeEditDownTime() 
{
		
	CKeypadDlg dlg(IDC_EDIT_DOWN_TIME, this);

	int nResponse = dlg.DoModal();

	m_Sleep = GetDlgItemInt(IDC_EDIT_DOWN_TIME);
}

void CTestAppDlg::UpdateControlInt(int iID, SnQByte qData, CONFIG *ptConfig, SnBool yForceUpdate,
                                   SnBool yValid)
{
    static SnQByte pqLastData[12] = {0};
    static SnQByte pqLastBits[12] = {0};
    static SnQByte pqLastFormat[12] = {0};
    static SnBool pyLastValid[12] = {0};
    SnWord pwBuf[TMP_BUF_SIZE];
    int iOffs;

    switch(iID) {
    default:
    case IDC_EDIT_READ_WRITE_CONFIG1:
        iOffs = 0;
        break;
    case IDC_EDIT_READ_WRITE_CONFIG2:
        iOffs = 1;
        break;
    case IDC_EDIT_READ_WRITE_CONFIG3:
        iOffs = 2;
        break;
    case IDC_EDIT_READ_WRITE_CONFIG4:
        iOffs = 3;
        break;
    case IDC_EDIT_READ_WRITE_CONFIG5:
        iOffs = 4;
        break;
    case IDC_EDIT_READ_WRITE_CONFIG6:
        iOffs = 5;
        break;
    case IDC_EDIT_READ_WRITE_CONFIG7:
        iOffs = 6;
        break;
    case IDC_EDIT_READ_WRITE_CONFIG8:
        iOffs = 7;
        break;
    case IDC_EDIT_READ_WRITE_CONFIG9:
        iOffs = 8;
        break;
    case IDC_EDIT_READ_WRITE_CONFIG10:
        iOffs = 9;
        break;
    case IDC_EDIT_READ_WRITE_CONFIG11:
        iOffs = 10;
        break;
    case IDC_EDIT_READ_WRITE_CONFIG12:
        iOffs = 11;
        break;
    }

    if (pyLastValid[iOffs] == FALSE) {
        yForceUpdate = TRUE;
    }

    if (yForceUpdate || qData != pqLastData[iOffs] ||
        ptConfig->bits != pqLastBits[iOffs] ||
        ptConfig->format != pqLastFormat[iOffs]) {

        if (yValid) {
            pqLastData[iOffs] = qData;
            pqLastBits[iOffs] = ptConfig->bits;
            pqLastFormat[iOffs] = ptConfig->format;
            switch (ptConfig->format) {
            case FORMAT_UNSIGNED:
                swprintf((wchar_t *)pwBuf, TEXT("%u"), qData);
                SetDlgItemText(iID, (LPCTSTR)pwBuf);
                break;
            case FORMAT_SIGNED:
                if (ptConfig->bits == BITS_16) {
                    SetDlgItemInt(iID, (short)qData);
                } else {
                    SetDlgItemInt(iID, (int)qData);
                }
                break;
            case FORMAT_HEX:
                if (ptConfig->bits == BITS_16) {
                    swprintf((wchar_t *)pwBuf, TEXT("0x%04x"), qData);
                } else {
                    swprintf((wchar_t *)pwBuf, TEXT("0x%08x"), qData);
                }
                SetDlgItemText(iID, (LPCTSTR)pwBuf);
                break;
            case FORMAT_FLOAT:
                {
                    float fData = *(float *)&qData;

                    qData = (SnQByte)fData;
                    if (fData == (float)qData) {
                        swprintf((wchar_t *)pwBuf, TEXT("%d.0"), qData);
                    } else {
                        swprintf((wchar_t *)pwBuf, TEXT("%g"), fData);
                    }
                    SetDlgItemText(iID, (LPCTSTR)pwBuf);
                }
                break;
            default:
                yValid = FALSE;
                break;
            }
        }
        if (yValid == FALSE) {
            SetDlgItemText(iID, TEXT("----------------"));
        }
        pyLastValid[iOffs] = yValid;
    }
}

void UpdateWindow(CTestAppDlg *pClass, CONFIG *ptConfig, int iID, SnQByte *pqRecordCount)
{
	char	pcDataBuf[TMP_BUF_SIZE];
	SnQByte qData;	// 32 bit data
	SnWord	pwBuffer[2];
	SnBool  yStatus = FALSE;

    if(ptConfig->type == TYPE_READ_ONLY && !pClass->m_KillUpdateThread)
	{
		if( ptConfig->bits == BITS_16)
		{	
            if( pClass->m_hDriver != NULL) {
				yStatus = pClass->m_hDriver->ReadWordFromDevice( 
											   ptConfig->offset,
											   &pwBuffer[0]);
            }
			qData = (SnQByte)pwBuffer[0];
		}
		else
		{
            if( pClass->m_hDriver != NULL) {
				yStatus = pClass->m_hDriver->ReadWordsFromDevice(
												ptConfig->offset,
												2, pwBuffer);
            }
			qData = pwBuffer[1] << 16; 
			qData = qData | pwBuffer[0];
		}
		
        pClass->UpdateControlInt(iID, qData, ptConfig, FALSE, yStatus);

        if (pClass->m_SaveData)
        {
            if( pClass->m_SetupCount == pClass->m_SaveCount)
			    pClass->m_FileSetup = TRUE;
						    
		    (*pqRecordCount)++;

		    if( pClass->m_FileSetup == FALSE)   // write window name
		    {
			    pClass->GetBuffer( pcDataBuf, ptConfig->name); 
		    
			    if( *pqRecordCount == pClass->m_SaveCount)
			    {
				    strcat( pcDataBuf, "\r\n");   // end of record marker
				    *pqRecordCount = 0;         // reset the count to zero
			    }
			    else
			    {
				    strcat( pcDataBuf, ",");      // column marker
			    }	
			    pClass->m_SetupCount++;
		    }
		    else // write data
		    {
                CString Data;

                pClass->GetDlgItemText(iID, Data);
			    pClass->GetBuffer( pcDataBuf, Data); 
		    
			    if( *pqRecordCount == pClass->m_SaveCount)
			    {
				    strcat( pcDataBuf, "\r\n");   // end of record marker
				    *pqRecordCount = 0;           // reset the count to zero
			    }
			    else
			    {
				    strcat( pcDataBuf, ",");
			    }							    
		    }
            pClass->m_hLogger->AddString(pcDataBuf);
        }
	}
}

void AdjustedSleep(DWORD timerStart, DWORD sleepPeriod, DWORD maxAdjustment) 
{
	DWORD	timerDelta = 0;
	timerDelta = GetTickCount();
	timerDelta = (timerDelta > timerStart) ? timerDelta - timerStart : 0;
	sleepPeriod -= ( timerDelta < maxAdjustment)? timerDelta: maxAdjustment;
	Sleep (sleepPeriod);
}

DWORD WINAPI UpdateDisplayThread(LPVOID pParam)
{	
	CTestAppDlg *pClass = (CTestAppDlg*)pParam;
	SnQByte	qRecordCount = 0;
    SnQByte qStart, qDelta;

	while( !pClass->m_KillUpdateThread)
	{
        qStart = GetTickCount();

		pClass->UpdateTitle(FALSE);
        UpdateWindow(pClass, &(pClass->m_ConfigOne), IDC_EDIT_READ_WRITE_CONFIG1, &qRecordCount);
        UpdateWindow(pClass, &(pClass->m_ConfigTwo), IDC_EDIT_READ_WRITE_CONFIG2, &qRecordCount);
        UpdateWindow(pClass, &(pClass->m_ConfigThree), IDC_EDIT_READ_WRITE_CONFIG3, &qRecordCount);
        UpdateWindow(pClass, &(pClass->m_ConfigFour), IDC_EDIT_READ_WRITE_CONFIG4, &qRecordCount);
        UpdateWindow(pClass, &(pClass->m_ConfigFive), IDC_EDIT_READ_WRITE_CONFIG5, &qRecordCount);
        UpdateWindow(pClass, &(pClass->m_ConfigSix), IDC_EDIT_READ_WRITE_CONFIG6, &qRecordCount);
        UpdateWindow(pClass, &(pClass->m_ConfigSeven), IDC_EDIT_READ_WRITE_CONFIG7, &qRecordCount);
        UpdateWindow(pClass, &(pClass->m_ConfigEight), IDC_EDIT_READ_WRITE_CONFIG8, &qRecordCount);
        UpdateWindow(pClass, &(pClass->m_ConfigNine), IDC_EDIT_READ_WRITE_CONFIG9, &qRecordCount);
        UpdateWindow(pClass, &(pClass->m_ConfigTen), IDC_EDIT_READ_WRITE_CONFIG10, &qRecordCount);
        UpdateWindow(pClass, &(pClass->m_ConfigEleven), IDC_EDIT_READ_WRITE_CONFIG11, &qRecordCount);
        UpdateWindow(pClass, &(pClass->m_ConfigTwelve), IDC_EDIT_READ_WRITE_CONFIG12, &qRecordCount);		
		
		pClass->ShowTime();

        if (WaitForSingleObject(pClass->m_hShutDownAppEvent, 0) == WAIT_OBJECT_0) {
            ExitProcess(0);
        }
 
        qDelta = GetTickCount() - qStart;

        if (qDelta < pClass->m_Sleep)
		    Sleep(pClass->m_Sleep - qDelta); 
	}
	return 0;
}

void CTestAppDlg::OnButtonSetTime() 
{
    SYSTEMTIME tTime;
	CSetDateTimeDlg	dlg;
 
	if (dlg.DoModal() == IDOK)
	{
		dlg.GetNewSystemTime(&tTime);
        SetSystemTime(&tTime);
	}
}
