// TestMode.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "TestMode.h"
#include "ColorsFonts.h"
#include "SharedMemory.h"
#include "MessageBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestMode dialog


CTestMode::CTestMode(CControl* pControl, CWnd* pParent /*=NULL*/)
	: CDialog(CTestMode::IDD, pParent)
{
	m_pControl = pControl;
	
	// create brushes 
	m_hbrBlack = CreateSolidBrush(SN_BKGND_COLOR);
	m_hbrYellow = CreateSolidBrush(SN_YELLOW);


	m_pStaticShortWarningPortA = NULL;
	m_pStaticShortWarningPortB = NULL;

	m_hResLib = NULL;

	m_dwFatalErrorNum = 0;
	
	//{{AFX_DATA_INIT(CTestMode)
	//}}AFX_DATA_INIT
}
CTestMode::~CTestMode()
{
	DeInit();
}
void CTestMode::DeInit()
{
	if(m_pStaticShortWarningPortA)
	{
		delete(m_pStaticShortWarningPortA);
		m_pStaticShortWarningPortA = NULL;
	}

	if(m_pStaticShortWarningPortB)
	{
		delete(m_pStaticShortWarningPortB);
		m_pStaticShortWarningPortB = NULL;
	}
	
	// delete brush objects
	DeleteObject( m_hbrBlack);
	DeleteObject( m_hbrYellow);

	// Free Language resources
	if( m_hResLib != NULL)
		FreeLibrary( m_hResLib);
}

void CTestMode::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestMode)
	DDX_Control(pDX, IDC_STATIC5, m_StaticBox5);
	DDX_Control(pDX, IDC_STATIC4, m_StaticBox4);
	DDX_Control(pDX, IDC_STATIC3, m_StaticBox3);
	DDX_Control(pDX, IDC_STATIC2, m_StaticBox2);
	DDX_Control(pDX, IDC_STATIC1, m_StaticBox1);
	DDX_Control(pDX, IDC_FATAL_ERROR_TEXT, m_StaticFatalError);
	DDX_Control(pDX, IDC_LIST2, m_ListBox2);
	DDX_Control(pDX, IDC_LIST1, m_ListBox1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTestMode, CDialog)
	//{{AFX_MSG_MAP(CTestMode)
	ON_WM_CTLCOLOR()
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelchangeList1)
	ON_BN_CLICKED(SN_SHORT_WARNING_PORTA, OnStaticShortWarningPortA)
	ON_BN_CLICKED(SN_SHORT_WARNING_PORTB, OnStaticShortWarningPortB)
	ON_LBN_SELCHANGE(IDC_LIST2, OnSelchangeList2)
	ON_MESSAGE(WM_INTELLIO_SHAVER_CMD, HandleIntellioShaverCmd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestMode message handlers
HBRUSH CTestMode::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr;
	
	// Intercept the paint call to change the dialog control colors
	
	// Call the base class implementation first! Otherwise, it may
	// undo what we are trying to accomplish .
	hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	int nCtrlID = pWnd->GetDlgCtrlID();
	switch(nCtrlID)
	{
		case SN_SHORT_WARNING_PORTA:

				pDC->SetBkColor(SN_YELLOW);
				pDC->SetTextColor(SN_BKGND_COLOR);
				hbr = m_hbrYellow;
	
			break;

		case SN_SHORT_WARNING_PORTB:
				pDC->SetBkColor(SN_YELLOW);
				pDC->SetTextColor(SN_BKGND_COLOR);
				hbr = m_hbrYellow;
				
			break;
		
		default :
			pDC->SetBkColor(SN_BKGND_COLOR);
			pDC->SetTextColor(WHITE);
			hbr = m_hbrBlack;
			break;

	}
	
	return hbr;	
		
}

BOOL CTestMode::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CSharedMemory mem;
	
	// Create Static control
	m_pStaticShortWarningPortA = CreateStaticControl(CRect(90, 285, 310, 335),SN_SHORT_WARNING_PORTA,CTL_TEXT);
	if(!m_pStaticShortWarningPortA)
	{
		// Failed to create window control
		DeInit();
		return FALSE;
	}

	m_pStaticShortWarningPortB = CreateStaticControl(CRect(490, 285, 710, 335),SN_SHORT_WARNING_PORTB,CTL_TEXT);
	if(!m_pStaticShortWarningPortB)
	{
		// Failed to create window control
		DeInit();
		return FALSE;
	}
	
	// Initialize language to English
	SnBool bStatus = LoadLanguageDll(INDEX_ENGLISH);
	if( !bStatus)
	{
		// Failed to load the language resources
		DeInit();
		return FALSE;
	}

	// Set fonts for static boxes
	m_pStaticShortWarningPortA->SetFont(mem.m_Font14Normal, TRUE);
	m_pStaticShortWarningPortB->SetFont(mem.m_Font14Normal, TRUE);

	m_iCurrentStringIdPortA = SN_TEMPERATURE_FAULT;
	m_iCurrentStringIdPortB = SN_TEMPERATURE_FAULT;

	// Set default text
	m_dwIntellioShaverShortWarningNum = SCD_PW1;
	DrawShortWarnings();
	
	// Setup List Box 1
	m_ListBox1.InsertString(INDEX_ENGLISH, _T("ENGLISH"));
	m_ListBox1.InsertString(INDEX_DEUTSCH, _T("DEUTSCH"));
	m_ListBox1.InsertString(INDEX_ITALIANO, _T("ITALIANO"));
	m_ListBox1.InsertString(INDEX_ESPANOL, _T("ESPANOL"));
	m_ListBox1.InsertString(INDEX_FRANCAIS, _T("FRANCAIS"));
	m_ListBox1.InsertString(INDEX_DANSK, _T("DANSK"));
	m_ListBox1.InsertString(INDEX_NEDERLANDS, _T("NEDERLANDS"));
	m_ListBox1.InsertString(INDEX_NORSK, _T("NORSK"));
	m_ListBox1.InsertString(INDEX_PORTUGUES, _T("PORTUGUES"));
	m_ListBox1.InsertString(INDEX_SVENSKA, _T("SVENSKA"));

	m_ListBox1.SetFont(mem.m_Font20Normal, TRUE);
	m_ListBox1.SetCurSel(INDEX_ENGLISH);

	
	// Setup List Box 2
	m_ListBox2.InsertString(INDEX_PW1, _T("PW1"));
	m_ListBox2.InsertString(INDEX_PW2, _T("PW2"));
	m_ListBox2.InsertString(INDEX_PW3, _T("PW3"));
	m_ListBox2.InsertString(INDEX_PW4, _T("PW4"));
	m_ListBox2.InsertString(INDEX_PW5, _T("PW5"));
	m_ListBox2.InsertString(INDEX_PW6, _T("PW6"));
	m_ListBox2.InsertString(INDEX_PW7, _T("PW7"));
	m_ListBox2.InsertString(INDEX_PW8, _T("PW8"));
	m_ListBox2.InsertString(INDEX_PW9, _T("PW9"));
	m_ListBox2.InsertString(INDEX_PW10, _T("PW10"));
	m_ListBox2.InsertString(INDEX_PW11, _T("PW11"));
	m_ListBox2.InsertString(INDEX_PW12, _T("PW12"));
	m_ListBox2.InsertString(INDEX_PW13, _T("PW13"));
	m_ListBox2.InsertString(INDEX_PW14, _T("PW14"));
	m_ListBox2.InsertString(INDEX_PW15, _T("PW15"));
	m_ListBox2.InsertString(INDEX_PW16, _T("PW16"));
	m_ListBox2.InsertString(INDEX_PU1, _T("PU1"));
	m_ListBox2.InsertString(INDEX_PU2, _T("PU2"));
	m_ListBox2.InsertString(INDEX_PU3, _T("PU3"));
	m_ListBox2.InsertString(INDEX_PU4, _T("PU4"));
	m_ListBox2.InsertString(INDEX_FE2, _T("FE2"));
	m_ListBox2.InsertString(INDEX_FE3, _T("FE3"));
	m_ListBox2.InsertString(INDEX_FE4, _T("FE4"));


	m_ListBox2.SetFont(mem.m_Font20Normal, TRUE);
	m_ListBox2.SetCurSel(INDEX_PW1);
		

	m_StaticFatalError.SetFont(mem.m_Font25Bold, TRUE);


	m_StaticBox1.SetFont(mem.m_Font20Normal, TRUE);
	m_StaticBox2.SetFont(mem.m_Font20Normal, TRUE);
	m_StaticBox3.SetFont(mem.m_Font20Normal, TRUE);
	m_StaticBox4.SetFont(mem.m_Font20Normal, TRUE);
	m_StaticBox5.SetFont(mem.m_Font20Normal, TRUE);

	SetDlgItemText(IDC_STATIC1, _T("Language"));
	SetDlgItemText(IDC_STATIC2, _T("Warning/Error"));
	SetDlgItemText(IDC_STATIC3, _T("Port A"));
	SetDlgItemText(IDC_STATIC4, _T("Port B"));
	SetDlgItemText(IDC_STATIC5, SN_CLEAR_TEXT);
	SetDlgItemText(IDC_FATAL_ERROR_TEXT, SN_CLEAR_TEXT);

	// Tell the control layer to send WM messages to this screen.
	if( m_pControl)
		m_pControl->SetMessageHandler(this->m_hWnd);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CStatic* CTestMode::CreateStaticControl( CRect tRect, DWORD dwId, DWORD dwCtlType)
{
	CStatic* pStatic = NULL;
	SnBool bStatus;
	DWORD dwStyle;

	if( dwCtlType == CTL_TEXT)
		dwStyle = WS_VISIBLE | SS_CENTER | SS_NOTIFY;
	else if( dwCtlType == CTL_BITMAP)
		dwStyle = WS_VISIBLE | SS_BITMAP ;
	else if( dwCtlType == CTL_BUTTON)
		dwStyle = WS_VISIBLE | SS_BITMAP | SS_NOTIFY ;
	else
		return NULL;

	//
	// Create Static window control
	//
	pStatic = new CStatic;
	if(!pStatic)
		return pStatic; // Failed to allocate resourses
		
		
	bStatus = pStatic->Create( NULL,		// Window Name 
							   dwStyle,		// Window Style
							   tRect,		// Window size and location
							   this,		// Parent Window
					           dwId);		// Window ID
	
	if(!bStatus)
	{
		// Failed to create window control
		if( pStatic)
		{
			delete pStatic;
			pStatic = NULL;
		}
	}

	return pStatic;
}

SnBool CTestMode::LoadLanguageDll(int index)
{
	CSharedMemory mem;
    SnWord usLanguage;

	if( m_hResLib != NULL)
		FreeLibrary( m_hResLib);


	// Load Language dll
	CString csDll;
	switch(index)
	{
		
		case INDEX_ENGLISH:
			csDll = _T("EnglishRes");
            usLanguage = LANGUAGE_ENGLISH;
			break;

		case INDEX_DEUTSCH:
			csDll = _T("GermanRes");
            usLanguage = LANGUAGE_GERMAN;
			break;

		case INDEX_ITALIANO:
			csDll = _T("ItalianRes");
            usLanguage = LANGUAGE_ITALIAN;
			break;
	
		case INDEX_ESPANOL:
			csDll = _T("SpanishRes");
            usLanguage = LANGUAGE_SPANISH;
			break;

		case INDEX_FRANCAIS:
			csDll = _T("FrenchRes");
            usLanguage = LANGUAGE_FRENCH;
			break;

		case INDEX_DANSK:
			csDll = _T("DanishRes");
            usLanguage = LANGUAGE_DANISH;
			break;

		case INDEX_NEDERLANDS:
			csDll = _T("DutchRes");
            usLanguage = LANGUAGE_DUTCH;
			break;

		case INDEX_NORSK:
			csDll = _T("NorwegianRes");
            usLanguage = LANGUAGE_NORWEGIAN;
			break;

		case INDEX_PORTUGUES:
			csDll = _T("PortugueseRes");
            usLanguage = LANGUAGE_PORTUGUESE;
			break;

		case INDEX_SVENSKA:
			csDll = _T("SwedishRes");
            usLanguage = LANGUAGE_SWEDISH;
			break;

		default:
			csDll = _T("EnglishRes");		
            usLanguage = LANGUAGE_ENGLISH;
			break;

	}
	
	m_hResLib = LoadLibrary( csDll) ;

	if(m_hResLib == NULL)
	{
			
		if (index != INDEX_ENGLISH)
		{
			// Try to load the English Dll as a last resort
			csDll = _T("EnglishRes");	
			m_hResLib = LoadLibrary (csDll); 
		
			if( m_hResLib == NULL)
				return FALSE;
		}

	}

    mem.LoadLanguageDll (usLanguage);

	return TRUE;
}

CString CTestMode::GetString(int resourceID)
{
	// Returns text extracted from the String Table resource
	TCHAR  pwText[1024];
	
	if( m_hResLib != NULL)
	{
		if (!LoadString( m_hResLib, resourceID, pwText,  sizeof(pwText)))
		    pwText[0] = 0;   // failure
	}
	else
		pwText[0] = 0;       // failure

	return (CString )pwText;
}

void CTestMode::OnStaticShortWarningPortA()
{
	DisplayWarningDetails(m_iCurrentStringIdPortA);
}

void CTestMode::OnStaticShortWarningPortB()
{
	DisplayWarningDetails(m_iCurrentStringIdPortB);
}

void CTestMode::OnSelchangeList1() 
{
	// Get the current list box selection
	int index = m_ListBox1.GetCurSel();
	
	LoadLanguageDll(index);

	DrawShortWarnings();

	if( m_dwFatalErrorNum != 0)
	{
		CString csTemp, csError;

		csTemp = GetString(SN_SYSTEM_ERROR);
		csError.Format(_T("%s %d"), csTemp, m_dwFatalErrorNum);
		SetDlgItemText(IDC_STATIC5, _T("Fatal Error"));
		SetDlgItemText( IDC_FATAL_ERROR_TEXT, csError);
	}
}

WarningTable CTestMode::ptWarningTable[] = 
{
	{INDEX_PW1,  SCD_PW1,  SN_TEMPERATURE_FAULT},
	{INDEX_PW2,  SCD_PW2,  SN_INVALID_BLADE_ID},
	{INDEX_PW3,  SCD_PW3,  SN_INVALID_DEVICE_ID},
	{INDEX_PW4,  SCD_PW4,  SN_INVALID_HALL_PATTERN},
	{INDEX_PW5,  SCD_PW5,  SN_MOTOR_STALL_AND_CURRENT_LIMIT},
	{INDEX_PW6,  SCD_PW6,  SN_MOTOR_STALL},
	{INDEX_PW7,  SCD_PW7,  SN_MOTOR_TAC_FAULT},
	{INDEX_PW8,  SCD_PW8,  SN_SHORT_CIRCUIT_DETECTED},
	{INDEX_PW9,  SCD_PW8,  SN_SHORT_CIRCUIT_DETECTED},
	{INDEX_PW10, SCD_PW10, SN_MOTOR_CURRENT_LIMIT},
	{INDEX_PW11, SCD_PW11, SN_MOTOR_CURRENT_LIMIT_TIMEOUT},
	{INDEX_PW12, SCD_PW12, SN_HAND_STUCK_BUTTON},
	{INDEX_PW13, SCD_PW13, SN_UNKNOWN_FOOT_ID},
	{INDEX_PW14, SCD_PW14, SN_FOOT_STUCK_PEDAL},
	{INDEX_PW15, SCD_PW15, SN_FOOT_LOW_BATTERY},
	{INDEX_PW16, SCD_PW16, SN_FOOTSWITCH_REQUIRED},
};

void CTestMode::OnSelchangeList2() 
{
	int iStringId = 0;
	SnBool bPopup = FALSE;
	SnBool bFatalError = FALSE;
	SnByte ucIntellioShaverError = 0;

	m_dwFatalErrorNum = 0;
	
	SetDlgItemText(IDC_STATIC5, SN_CLEAR_TEXT);
	SetDlgItemText(IDC_FATAL_ERROR_TEXT, SN_CLEAR_TEXT);
	
	// Get the current list box selection
	int index = m_ListBox2.GetCurSel();
	
	if( index <= INDEX_PW16)
	{
		iStringId = ptWarningTable[index].iStringID;
		m_dwIntellioShaverShortWarningNum = ptWarningTable[index].iIntellioShaverWarning;

		m_iCurrentStringIdPortA = iStringId;
		m_iCurrentStringIdPortB = iStringId;

		DrawShortWarnings();
	}
	else
	{
		int	detailedTextId = 0;

		ClearShortWarnings();
		m_iCurrentStringIdPortA = SN_STRING_INVALID;
		m_iCurrentStringIdPortB = SN_STRING_INVALID;

		switch(index)
		{
			case INDEX_PU1:
				detailedTextId = SN_CUSTOM_SETTINGS_SAVE_FAILURE;
				bPopup = TRUE;
				ucIntellioShaverError = SCD_PU1;
				break;

			case INDEX_PU2:
				detailedTextId = SN_SET_SPEEDS_SAVE_FAILURE;
				bPopup = TRUE;
				ucIntellioShaverError = SCD_PU2;
				break;

			case INDEX_PU3:
				detailedTextId = SN_CUSTOM_SETTINGS_RETRIEVE_FAILURE;
				bPopup = TRUE;
				ucIntellioShaverError = SCD_PU3;
				break;

			case INDEX_PU4:
				detailedTextId = SN_SET_SPEEDS_RETRIEVE_FAILURE;
				bPopup = TRUE;
				ucIntellioShaverError = SCD_PU4;
				break;

			case INDEX_FE2:
				m_dwFatalErrorNum = 2;
				bFatalError = TRUE;
				ucIntellioShaverError = 2;		
				break;

			case INDEX_FE3:
				m_dwFatalErrorNum = 3;
				bFatalError = TRUE;
				ucIntellioShaverError = 3;		
				break;

			case INDEX_FE4:
				m_dwFatalErrorNum = 4;
				bFatalError = TRUE;
				ucIntellioShaverError = 4;		
				break;

			default:
				m_dwFatalErrorNum = 0;
				break;
		}
		
		if( bPopup)
		{
			if( m_pControl)
			{
                m_pControl->UpdateIntellioShaverPortErrWarn(PORTA, 0);
                m_pControl->UpdateIntellioShaverPortErrWarn(PORTB, 0);
                m_pControl->SendIntellioShaverUpdateIfChange();
			}
			
			CMessageBox dlg( m_pControl, SN_SYSTEM_ERROR, detailedTextId);
			dlg.DoModal();

			if( m_pControl)
			{
				m_pControl->SetMessageHandler(this->m_hWnd);
			}
		}
		else if( bFatalError)
		{
			if( m_pControl)
			{
                m_pControl->UpdateIntellioShaverPortErrWarn(PORTA, 0);
                m_pControl->UpdateIntellioShaverPortErrWarn(PORTB, 0);
                m_pControl->SendIntellioShaverUpdateIfChange();
			}
			
			CString csTemp, csError;

			csTemp = GetString(SN_SYSTEM_ERROR);

			csError.Format(_T("%s %d"), csTemp, m_dwFatalErrorNum);

			SetDlgItemText(IDC_STATIC5, _T("Fatal Error"));
			SetDlgItemText( IDC_FATAL_ERROR_TEXT, csError);
		}
	}
}

void CTestMode::ClearShortWarnings()
{
	// Draw windows
	if(m_pStaticShortWarningPortA)
		SetDlgItemText( SN_SHORT_WARNING_PORTA,SN_CLEAR_TEXT);
	if(m_pStaticShortWarningPortB)
		SetDlgItemText( SN_SHORT_WARNING_PORTB,SN_CLEAR_TEXT);

}

void CTestMode::DrawShortWarnings()
{
	if( m_pControl)
	{
        m_pControl->UpdateIntellioShaverPortErrWarn(PORTA, (SnByte)m_dwIntellioShaverShortWarningNum);
        m_pControl->UpdateIntellioShaverPortErrWarn(PORTB, (SnByte)m_dwIntellioShaverShortWarningNum);
        m_pControl->SendIntellioShaverUpdateIfChange();
	}
	
	// Draw windows
	if(m_pStaticShortWarningPortA && m_iCurrentStringIdPortA != SN_STRING_INVALID)
		SetDlgItemText( SN_SHORT_WARNING_PORTA,GetString(m_iCurrentStringIdPortA));
	if(m_pStaticShortWarningPortB && m_iCurrentStringIdPortA != SN_STRING_INVALID)
		SetDlgItemText( SN_SHORT_WARNING_PORTB,GetString(m_iCurrentStringIdPortB));
}

void CTestMode::DisplayWarningDetails(int iStringId)
{
	if( iStringId != SN_STRING_INVALID)
	{
		int	detailedTextId = 0;

		switch(iStringId)
		{
		case SN_TEMPERATURE_FAULT:
			detailedTextId = SN_TEMPERATURE_FAULT_DETAIL;
			break;
		case SN_INVALID_BLADE_ID:
			detailedTextId = SN_INVALID_BLADE_ID_DETAIL;
			break;
		case  SN_INVALID_DEVICE_ID:
			detailedTextId = SN_INVALID_DEVICE_ID_DETAIL;
			break;
		case  SN_MOTOR_STALL:
			detailedTextId = SN_MOTOR_STALL_DETAIL;
			break;
		case  SN_MOTOR_STALL_AND_CURRENT_LIMIT:
			detailedTextId = SN_MOTOR_STALL_AND_CURRENT_LIMIT_DETAIL;
			break;
		case  SN_MOTOR_TAC_FAULT:
			detailedTextId = SN_MOTOR_TAC_FAULT_DETAIL;
			break;
		case  SN_SHORT_CIRCUIT_DETECTED:
			detailedTextId = SN_SHORT_CIRCUIT_DETAIL;
			break;
		case  SN_UNKNOWN_FOOT_ID:
			detailedTextId = SN_UNKNOWN_FOOT_ID_DETAIL;
			break;
		case  SN_HAND_STUCK_BUTTON:
			detailedTextId = SN_HAND_STUCK_BUTTON_DETAIL;
			break;
		case  SN_FOOT_STUCK_PEDAL:
			detailedTextId = SN_FOOT_STUCK_PEDAL_DETAIL;
			break;
		case  SN_MOTOR_CURRENT_LIMIT_TIMEOUT:
			detailedTextId = SN_MOTOR_CURRENT_LIMIT_TIMEOUT_DETAIL;
			break;
		case  SN_MOTOR_CURRENT_LIMIT:
			detailedTextId = SN_MOTOR_CURRENT_LIMIT_DETAIL;
			break;
		case  SN_INVALID_HALL_PATTERN:
			detailedTextId = SN_INVALID_HALL_PATTERN_DETAIL;
			break;
		case  SN_FOOT_LOW_BATTERY:
			detailedTextId = SN_FOOT_LOW_BATTERY_DETAIL;
			break;
		case  SN_FOOTSWITCH_REQUIRED:
			detailedTextId = SN_FOOTSWITCH_REQUIRED_DETAIL;
			break;
		}
		
		CMessageBox dlg( m_pControl, SN_WARNING, detailedTextId);
		dlg.DoModal();
	}
}

LRESULT CTestMode::HandleIntellioShaverCmd( WPARAM iParam, LPARAM lParam)
{
	int index;
	if( m_pControl != NULL)
	{
		
		switch(iParam)
		{
		case KEY_UP_PORTA:
		case KEY_UP_PORTB:
			// Get the current list box selection
			index = m_ListBox2.GetCurSel();
			if (index > INDEX_PW1)
			{
				index -=1;
				m_ListBox2.SetCurSel(index);
				OnSelchangeList2();
			}
			break;
			
		case KEY_DOWN_PORTA:
		case KEY_DOWN_PORTB:
			// Get the current list box selection
			index = m_ListBox2.GetCurSel();
			if (index < INDEX_FE4)
			{
				index +=1;
				m_ListBox2.SetCurSel(index);
				OnSelchangeList2();
			}
			break;
		default:
			break;
		}
		
	}
	
	return 0L;
}