// SerialNumber.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "SerialNumberPopUp.h"
#include "Control.h"
#include "Driver.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDriver*    m_pDriver;

/////////////////////////////////////////////////////////////////////////////
// SerialNumber dialog


CSerialNumberPopUp::CSerialNumberPopUp(CControl* pControl, SnWord wEnteringFromScreen, CWnd* pParent /*=NULL*/)
	: CDialog(CSerialNumberPopUp::IDD, pParent)
{

    m_pParent = pParent;
	m_pControl = pControl;
    m_wEnteringFromScreen = wEnteringFromScreen;
	// create solid brush for background color
	m_hbrBlack = CreateSolidBrush(SN_BLACK);

    //{{AFX_DATA_INIT(CSerialNumberPopUp)
	//}}AFX_DATA_INIT
}

CSerialNumberPopUp::~CSerialNumberPopUp()
{
	// Delete solid brush object
	DeInit();
}

void CSerialNumberPopUp::DeInit()
{
	// delete brush objects
	DeleteObject(m_hbrBlack);
}

void CSerialNumberPopUp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSerialNumberPopUp)
	DDX_Control(pDX, IDC_STATIC_TITLE, m_StaticTitle);
	DDX_Control(pDX, IDC_STATIC_MESSAGE_TEXT2, m_StaticSerialNumber);
	DDX_Control(pDX, IDC_STATIC_MESSAGE_TEXT, m_StaticInstructions);
	DDX_Control(pDX, IDC_BUTTON_NUMPAD_ENTER, m_BtnPadEnter);
	DDX_Control(pDX, IDC_BUTTON_NUMPAD_DEL, m_BtnPadDel);
	DDX_Control(pDX, IDC_BUTTON_NUMPAD_9, m_BtnPad9);
	DDX_Control(pDX, IDC_BUTTON_NUMPAD_8, m_BtnPad8);
	DDX_Control(pDX, IDC_BUTTON_NUMPAD_7, m_BtnPad7);
	DDX_Control(pDX, IDC_BUTTON_NUMPAD_6, m_BtnPad6);
	DDX_Control(pDX, IDC_BUTTON_NUMPAD_5, m_BtnPad5);
	DDX_Control(pDX, IDC_BUTTON_NUMPAD_4, m_BtnPad4);
	DDX_Control(pDX, IDC_BUTTON_NUMPAD_3, m_BtnPad3);
	DDX_Control(pDX, IDC_BUTTON_NUMPAD_2, m_BtnPad2);
	DDX_Control(pDX, IDC_BUTTON_NUMPAD_1, m_BtnPad1);
	DDX_Control(pDX, IDC_BUTTON_NUMPAD_0, m_BtnPad0);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSerialNumberPopUp, CDialog)
	//{{AFX_MSG_MAP(CSerialNumberPopUp)
	ON_BN_CLICKED(IDC_BUTTON_NUMPAD_0, OnButtonNumpad0)
	ON_BN_CLICKED(IDC_BUTTON_NUMPAD_1, OnButtonNumpad1)
	ON_BN_CLICKED(IDC_BUTTON_NUMPAD_2, OnButtonNumpad2)
	ON_BN_CLICKED(IDC_BUTTON_NUMPAD_3, OnButtonNumpad3)
	ON_BN_CLICKED(IDC_BUTTON_NUMPAD_4, OnButtonNumpad4)
	ON_BN_CLICKED(IDC_BUTTON_NUMPAD_5, OnButtonNumpad5)
	ON_BN_CLICKED(IDC_BUTTON_NUMPAD_6, OnButtonNumpad6)
	ON_BN_CLICKED(IDC_BUTTON_NUMPAD_7, OnButtonNumpad7)
	ON_BN_CLICKED(IDC_BUTTON_NUMPAD_8, OnButtonNumpad8)
	ON_BN_CLICKED(IDC_BUTTON_NUMPAD_9, OnButtonNumpad9)
	ON_BN_CLICKED(IDC_BUTTON_NUMPAD_DEL, OnButtonNumpadDel)
	ON_BN_CLICKED(IDC_BUTTON_NUMPAD_ENTER, OnButtonNumpadEnter)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSerialNumberPopUp message handlers

HBRUSH CSerialNumberPopUp::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int nCtrlID = pWnd->GetDlgCtrlID();

    if (nCtrlID == IDC_STATIC2) {
        hbr = m_hbrBlack;
    }

    return hbr;
}


BOOL CSerialNumberPopUp::OnInitDialog()
{
	CDialog::OnInitDialog();

    if (m_pControl) {
        m_pControl->GetSerialNumber(m_pcSerialNumber);
    }

    if (strncmp(m_pcSerialNumber, "AAX", 3) != 0) {
        strcpy(m_pcSerialNumber, "AAX");
    }

	// Setup the screen
    SetupTextButtons();
	SetupFonts();

	// Setup static text boxes
    if(m_wEnteringFromScreen == 0)
    {
	    SetDlgItemText( IDC_STATIC_TITLE, TEXT("Enter Serial Number"));
	    SetDlgItemText( IDC_STATIC_MESSAGE_TEXT, TEXT("Enter the Serial Number via the keypad on the right.\n\nValid Serial Numbers start at 5001 and contain no more than 7 numeric digits.\n\nPress Enter when you are done."));

        CString SerialNumber = m_pcSerialNumber;
        SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, SerialNumber);
    }
    else if(m_wEnteringFromScreen == 1 && wReliantLongSerialNumberEnable == 0)
    {
        m_pControl->GetCmdState(GET_MC_PORTA_STATUS, &m_tPortAStatus, sizeof(SN_PORT_STATUS));

        SetDlgItemText( IDC_STATIC_TITLE, TEXT("Enter Serial Number"));
        CString PrefixSerialNumber;

        if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_CTL) {
	        SetDlgItemText( IDC_STATIC_MESSAGE_TEXT, TEXT("Enter the Serial Number via the keypad on the right.\n\nValid Serial Numbers start at GGB000001 and contain no more than 6 numeric digits.\n\nPress Enter when you are done."));
            PrefixSerialNumber = _T("GGB");
		} else if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_BF) {
			SetDlgItemText( IDC_STATIC_MESSAGE_TEXT, TEXT("Enter the Serial Number via the keypad on the right.\n\nValid Serial Numbers start at GGH000001 and contain no more than 6 numeric digits.\n\nPress Enter when you are done."));
            PrefixSerialNumber = _T("GGH");
		} else {
			SetDlgItemText( IDC_STATIC_MESSAGE_TEXT, TEXT("Enter the Serial Number via the keypad on the right.\n\nValid Serial Numbers start at XXX000001 and contain no more than 6 numeric digits.\n\nPress Enter when you are done."));
            PrefixSerialNumber = _T("XXX");
		}

        CString SerialNumberFromFlash = CString(m_tPortAStatus.pcSerialNumber,9);
        CString SerialNumber = PrefixSerialNumber + SerialNumberFromFlash.Mid(3,6);
        m_csTextPcSerialNumber = SerialNumber;
        SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, m_csTextPcSerialNumber);
    }
    else if(m_wEnteringFromScreen == 1 && wReliantLongSerialNumberEnable == 1)
    {
        m_pControl->GetCmdState(GET_MC_PORTA_STATUS, &m_tPortAStatus, sizeof(SN_PORT_STATUS));

        SetDlgItemText( IDC_STATIC_TITLE, TEXT("Enter Serial Number"));
		CString PrefixSerialNumber;

        if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_CTL) {
	        SetDlgItemText( IDC_STATIC_MESSAGE_TEXT, TEXT("Enter the Serial Number via the keypad on the right.\n\nValid Serial Numbers start at GGB000001 and contain no more than 8 numeric digits.\n\nPress Enter when you are done."));
            PrefixSerialNumber = _T("GGB");
		} else if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_BF) {
			SetDlgItemText( IDC_STATIC_MESSAGE_TEXT, TEXT("Enter the Serial Number via the keypad on the right.\n\nValid Serial Numbers start at GGH000001 and contain no more than 8 numeric digits.\n\nPress Enter when you are done."));
            PrefixSerialNumber = _T("GGH");
		} else {
			SetDlgItemText( IDC_STATIC_MESSAGE_TEXT, TEXT("Enter the Serial Number via the keypad on the right.\n\nValid Serial Numbers start at XXX000001 and contain no more than 8 numeric digits.\n\nPress Enter when you are done."));
            PrefixSerialNumber = _T("XXX");
		}

        CString SerialNumberFromFlash = CString(m_tPortAStatus.pcSerialNumber,11);
        CString SerialNumber = PrefixSerialNumber + SerialNumberFromFlash.Mid(3,8);
        m_csTextPcSerialNumber = SerialNumber;
        SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, m_csTextPcSerialNumber);
    }
    else if(m_wEnteringFromScreen == 2 && wReliantLongSerialNumberEnable == 0)
    {
        m_pControl->GetCmdState(GET_MC_PORTB_STATUS, &m_tPortBStatus, sizeof(SN_PORT_STATUS));

        SetDlgItemText( IDC_STATIC_TITLE, TEXT("Enter Serial Number"));
		CString PrefixSerialNumber;

        if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_CTL) {
	        SetDlgItemText( IDC_STATIC_MESSAGE_TEXT, TEXT("Enter the Serial Number via the keypad on the right.\n\nValid Serial Numbers start at GGB000001 and contain no more than 6 numeric digits.\n\nPress Enter when you are done."));
            PrefixSerialNumber = _T("GGB");
		} else if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_BF) {
			SetDlgItemText( IDC_STATIC_MESSAGE_TEXT, TEXT("Enter the Serial Number via the keypad on the right.\n\nValid Serial Numbers start at GGH000001 and contain no more than 6 numeric digits.\n\nPress Enter when you are done."));
            PrefixSerialNumber = _T("GGH");
		} else {
			SetDlgItemText( IDC_STATIC_MESSAGE_TEXT, TEXT("Enter the Serial Number via the keypad on the right.\n\nValid Serial Numbers start at XXX000001 and contain no more than 6 numeric digits.\n\nPress Enter when you are done."));
            PrefixSerialNumber = _T("XXX");
		}

        CString SerialNumberFromFlash = CString(m_tPortBStatus.pcSerialNumber,9);
        CString SerialNumber = PrefixSerialNumber + SerialNumberFromFlash.Mid(3,6);
        m_csTextPcSerialNumber = SerialNumber;
        SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, m_csTextPcSerialNumber);
    }
    else if(m_wEnteringFromScreen == 2 && wReliantLongSerialNumberEnable == 1)
        {
            m_pControl->GetCmdState(GET_MC_PORTB_STATUS, &m_tPortBStatus, sizeof(SN_PORT_STATUS));

            SetDlgItemText( IDC_STATIC_TITLE, TEXT("Enter Serial Number"));
			CString PrefixSerialNumber;

            if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_CTL) {
	            SetDlgItemText( IDC_STATIC_MESSAGE_TEXT, TEXT("Enter the Serial Number via the keypad on the right.\n\nValid Serial Numbers start at GGB000001 and contain no more than 8 numeric digits.\n\nPress Enter when you are done."));
                PrefixSerialNumber = _T("GGB");
		    } else if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_BF) {
			    SetDlgItemText( IDC_STATIC_MESSAGE_TEXT, TEXT("Enter the Serial Number via the keypad on the right.\n\nValid Serial Numbers start at GGH000001 and contain no more than 8 numeric digits.\n\nPress Enter when you are done."));
                PrefixSerialNumber = _T("GGH");
		    } else {
			    SetDlgItemText( IDC_STATIC_MESSAGE_TEXT, TEXT("Enter the Serial Number via the keypad on the right.\n\nValid Serial Numbers start at XXX000001 and contain no more than 8 numeric digits.\n\nPress Enter when you are done."));
                PrefixSerialNumber = _T("XXX");
		    }

            CString SerialNumberFromFlash = CString(m_tPortBStatus.pcSerialNumber,11);
            CString SerialNumber = PrefixSerialNumber + SerialNumberFromFlash.Mid(3,8);
            m_csTextPcSerialNumber = SerialNumber;
            SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, m_csTextPcSerialNumber);
        }

    return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSerialNumberPopUp::SetupFonts()
{
	CSharedMemory mem;

	// Set fonts
	m_StaticTitle.SetFont(mem.m_Font30Normal, TRUE);
	m_StaticSerialNumber.SetFont(mem.m_Font30Bold, TRUE);
	m_StaticInstructions.SetFont(mem.m_Font15Normal, TRUE);
}

void CSerialNumberPopUp::SetupTextButtons()
{
	CSharedMemory mem;

	m_BtnPad0.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED,
        mem.m_Font20Normal, TEXT("0"), SN_BLACK);
	m_BtnPad1.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED,
        mem.m_Font20Normal, TEXT("1"), SN_BLACK);
	m_BtnPad2.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED,
        mem.m_Font20Normal, TEXT("2"), SN_BLACK);
	m_BtnPad3.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED,
        mem.m_Font20Normal, TEXT("3"), SN_BLACK);
	m_BtnPad4.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED,
        mem.m_Font20Normal, TEXT("4"), SN_BLACK);
	m_BtnPad5.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED,
        mem.m_Font20Normal, TEXT("5"), SN_BLACK);
	m_BtnPad6.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED,
        mem.m_Font20Normal, TEXT("6"), SN_BLACK);
	m_BtnPad7.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED,
        mem.m_Font20Normal, TEXT("7"), SN_BLACK);
	m_BtnPad8.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED,
        mem.m_Font20Normal, TEXT("8"), SN_BLACK);
	m_BtnPad9.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED,
        mem.m_Font20Normal, TEXT("9"), SN_BLACK);
	m_BtnPadDel.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED,
        mem.m_Font20Normal, TEXT("Del"), SN_BLACK);
	m_BtnPadEnter.LoadBitmaps(IDB_BITMAP_GRAY_BUTTON_ROUND, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED,
        mem.m_Font20Normal, TEXT("Enter"), SN_BLACK);
}

void CSerialNumberPopUp::AddDigit(char cDigit)
{
    if(m_wEnteringFromScreen == 0)
    {
        int len = strlen(m_pcSerialNumber);

        if (len < 10) {
            m_pcSerialNumber[len] = cDigit;
            m_pcSerialNumber[len+1] = 0;

            CString SerialNumber = m_pcSerialNumber;
            SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, SerialNumber);
        }
    }
    else if(m_wEnteringFromScreen == 1 && wReliantLongSerialNumberEnable == 0)
    {
        int len = strlen(m_tPortAStatus.pcSerialNumber);

        if (len < 9) {
            m_tPortAStatus.pcSerialNumber[len] = cDigit;
            m_tPortAStatus.pcSerialNumber[len+1] = 0;
			CString PrefixSerialNumber;

			if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_CTL) {
				PrefixSerialNumber = _T("GGB");
			} else if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_BF) {
				PrefixSerialNumber = _T("GGH");
			} else {
				PrefixSerialNumber = _T("XXX");
			}

            CString SerialNumberFromFlash = CString(m_tPortAStatus.pcSerialNumber,9);
            CString SerialNumber = PrefixSerialNumber + SerialNumberFromFlash.Mid(3,6);
            m_csTextPcSerialNumber = SerialNumber;
            SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, m_csTextPcSerialNumber);
        }
    }
    else if(m_wEnteringFromScreen == 1 && wReliantLongSerialNumberEnable == 1)
    {
        int len = strlen(m_tPortAStatus.pcSerialNumber);

        if (len < 11) {
            m_tPortAStatus.pcSerialNumber[len] = cDigit;
            m_tPortAStatus.pcSerialNumber[len+1] = 0;
			CString PrefixSerialNumber;

			if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_CTL) {
				PrefixSerialNumber = _T("GGB");
			} else if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_BF) {
				PrefixSerialNumber = _T("GGH");
			} else {
				PrefixSerialNumber = _T("XXX");
			}

            CString SerialNumberFromFlash = CString(m_tPortAStatus.pcSerialNumber,11);
            CString SerialNumber = PrefixSerialNumber + SerialNumberFromFlash.Mid(3,8);
            m_csTextPcSerialNumber = SerialNumber;
            SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, m_csTextPcSerialNumber);
        }
    }
    else if(m_wEnteringFromScreen == 2 && wReliantLongSerialNumberEnable == 0)
    {
        int len = strlen(m_tPortBStatus.pcSerialNumber);

        if (len < 9) {
            m_tPortBStatus.pcSerialNumber[len] = cDigit;
            m_tPortBStatus.pcSerialNumber[len+1] = 0;
			CString PrefixSerialNumber;

 			if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_CTL) {
 				PrefixSerialNumber = _T("GGB");
 			} else if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_BF) {
 				PrefixSerialNumber = _T("GGH");
 			} else {
 				PrefixSerialNumber = _T("XXX");
			}

            CString SerialNumberFromFlash = CString(m_tPortBStatus.pcSerialNumber,9);
            CString SerialNumber = PrefixSerialNumber + SerialNumberFromFlash.Mid(3,6);
            m_csTextPcSerialNumber = SerialNumber;
            SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, m_csTextPcSerialNumber);
        }
    }
    else if(m_wEnteringFromScreen == 2 && wReliantLongSerialNumberEnable == 1)
    {
        int len = strlen(m_tPortBStatus.pcSerialNumber);

        if (len < 11) {
            m_tPortBStatus.pcSerialNumber[len] = cDigit;
            m_tPortBStatus.pcSerialNumber[len+1] = 0;
			CString PrefixSerialNumber;

			if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_CTL) {
				PrefixSerialNumber = _T("GGB");
			} else if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_BF) {
				PrefixSerialNumber = _T("GGH");
			} else {
				PrefixSerialNumber = _T("XXX");
			}

            CString SerialNumberFromFlash = CString(m_tPortBStatus.pcSerialNumber,11);
            CString SerialNumber = PrefixSerialNumber + SerialNumberFromFlash.Mid(3,8);
            m_csTextPcSerialNumber = SerialNumber;
            SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, m_csTextPcSerialNumber);
        }
    }
}

void CSerialNumberPopUp::OnButtonNumpad0()
{
    AddDigit('0');
}

void CSerialNumberPopUp::OnButtonNumpad1()
{
    AddDigit('1');
}

void CSerialNumberPopUp::OnButtonNumpad2()
{
    AddDigit('2');
}

void CSerialNumberPopUp::OnButtonNumpad3()
{
    AddDigit('3');
}

void CSerialNumberPopUp::OnButtonNumpad4()
{
    AddDigit('4');
}

void CSerialNumberPopUp::OnButtonNumpad5()
{
    AddDigit('5');
}

void CSerialNumberPopUp::OnButtonNumpad6()
{
    AddDigit('6');
}

void CSerialNumberPopUp::OnButtonNumpad7()
{
    AddDigit('7');
}

void CSerialNumberPopUp::OnButtonNumpad8()
{
    AddDigit('8');
}

void CSerialNumberPopUp::OnButtonNumpad9()
{
    AddDigit('9');
}

void CSerialNumberPopUp::OnButtonNumpadDel()
{
    if(m_wEnteringFromScreen == 0)
    {
        int len = strlen(m_pcSerialNumber);

        if (len > 3) {
            m_pcSerialNumber[len - 1] = 0;

            CString SerialNumber = m_pcSerialNumber;
            SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, SerialNumber);
        }
    }
    else if(m_wEnteringFromScreen == 1 && wReliantLongSerialNumberEnable == 0)
    {
        int len = strlen(m_tPortAStatus.pcSerialNumber);

        if (len > 3) {
            m_tPortAStatus.pcSerialNumber[len - 1] = 0;
			CString PrefixSerialNumber;

			if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_CTL) {
				PrefixSerialNumber = _T("GGB");
			} else if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_BF) {
				PrefixSerialNumber = _T("GGH");
			} else {
				PrefixSerialNumber = _T("XXX");
			}

            CString SerialNumberFromFlash = CString(m_tPortAStatus.pcSerialNumber,9);
            CString SerialNumber = PrefixSerialNumber + SerialNumberFromFlash.Mid(3,6);
            m_csTextPcSerialNumber = SerialNumber;
            SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, m_csTextPcSerialNumber);
        }
    }
    else if(m_wEnteringFromScreen == 1 && wReliantLongSerialNumberEnable == 1)
    {
        int len = strlen(m_tPortAStatus.pcSerialNumber);

        if (len > 3) {
            m_tPortAStatus.pcSerialNumber[len - 1] = 0;
			CString PrefixSerialNumber;

			if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_CTL) {
				PrefixSerialNumber = _T("GGB");
			} else if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_BF) {
				PrefixSerialNumber = _T("GGH");
			} else {
				PrefixSerialNumber = _T("XXX");
			}

            CString SerialNumberFromFlash = CString(m_tPortAStatus.pcSerialNumber,11);
            CString SerialNumber = PrefixSerialNumber + SerialNumberFromFlash.Mid(3,8);
            m_csTextPcSerialNumber = SerialNumber;
            SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, m_csTextPcSerialNumber);
        }
    }
    else if(m_wEnteringFromScreen == 2 && wReliantLongSerialNumberEnable == 0)
    {
        int len = strlen(m_tPortBStatus.pcSerialNumber);

        if (len > 3) {
            m_tPortBStatus.pcSerialNumber[len - 1] = 0;
			CString PrefixSerialNumber;

			if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_CTL) {
				PrefixSerialNumber = _T("GGB");
			} else if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_BF) {
				PrefixSerialNumber = _T("GGH");
			} else {
				PrefixSerialNumber = _T("XXX");
			}

            CString SerialNumberFromFlash = CString(m_tPortBStatus.pcSerialNumber,9);
            CString SerialNumber = PrefixSerialNumber + SerialNumberFromFlash.Mid(3,6);
            m_csTextPcSerialNumber = SerialNumber;
            SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, m_csTextPcSerialNumber);
        }
    }
    else if(m_wEnteringFromScreen == 2 && wReliantLongSerialNumberEnable == 1)
    {
        int len = strlen(m_tPortBStatus.pcSerialNumber);

        if (len > 3) {
            m_tPortBStatus.pcSerialNumber[len - 1] = 0;
			CString PrefixSerialNumber;

			if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_CTL) {
				PrefixSerialNumber = _T("GGB");
			} else if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_BF) {
				PrefixSerialNumber = _T("GGH");
			} else {
				PrefixSerialNumber = _T("XXX");
			}

            CString SerialNumberFromFlash = CString(m_tPortBStatus.pcSerialNumber,11);
            CString SerialNumber = PrefixSerialNumber + SerialNumberFromFlash.Mid(3,8);
            m_csTextPcSerialNumber = SerialNumber;
            SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, m_csTextPcSerialNumber);
        }
    }
}

void CSerialNumberPopUp::OnButtonNumpadEnter()
{
    CUtil tUtil;
    SnBool bStatus = FALSE;

    if(m_wEnteringFromScreen == 0)
    {
        if (m_pControl->SaveShaverSerialNumber(m_pcSerialNumber)) {
            m_pControl->RestoreShaverSerialNumber();
	        CDialog::OnOK();
        }
    }
    else if(m_wEnteringFromScreen == 1)
    {
        SnWord newPwRequests;
        newPwRequests = HAND_PORT_CMD(PORTA, SERIAL_CMD_REQ_11);

        m_pControl->SendSerialRequests(1,&newPwRequests,4);

        SnByte g_pbFlashBuf[134];
        *(SnWord*)g_pbFlashBuf = HAND_PORT_CMD(PORTA, SERIAL_CMD_REQ_12);
        g_pbFlashBuf[2] = SERIAL_CMD_REQ_12;
        g_pbFlashBuf[3] = 0x00;
        g_pbFlashBuf[4] = 0xE0;

        SnWord len = strlen(m_tPortAStatus.pcSerialNumber);

        for(SnWord i=0; i<len; i++)
        {
            if(i==0) {
				if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_CTL)
					g_pbFlashBuf[5+i] = 'G';
				else if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_BF)
					g_pbFlashBuf[5+i] = 'G';
				else
					g_pbFlashBuf[5+i] = 'X';
            } else if (i==1) {
				if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_CTL)
					g_pbFlashBuf[5+i] = 'G';
				else if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_BF)
					g_pbFlashBuf[5+i] = 'G';
				else
					g_pbFlashBuf[5+i] = 'X';
            } else if (i==2) {
                if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_CTL)
					g_pbFlashBuf[5+i] = 'B';
				else if(m_tPortAStatus.usType == TYPE_MDU_RELIANT_BF)
					g_pbFlashBuf[5+i] = 'H';
				else
					g_pbFlashBuf[5+i] = 'X';
            } else {
				if(m_tPortAStatus.pcSerialNumber[i] >= '0' && m_tPortAStatus.pcSerialNumber[i] <= '9')
					g_pbFlashBuf[5+i] = m_tPortAStatus.pcSerialNumber[i];
				else {
					g_pbFlashBuf[5+i] = '\0';
					break;
				}
            }
        }

        // (16 bit sci cmd) + (8 bit page cmd) + (16 bit page #) +
        // (128 bytes of flash data) + Crc byte = 134 bytes
        SnQByte qPageCmdSize = 2 + 1 + 2 + 128 + 1;

        g_pbFlashBuf[128+5] = tUtil.CrcMemChunk(&g_pbFlashBuf[2],128 + 3,0);

        m_pDriver = (CDriver*)new(CDriver);
        if(m_pDriver) {

            m_pDriver->InitDriver();
            if(m_pDriver->SerialPageToDevice(qPageCmdSize, g_pbFlashBuf) == FALSE)
            {
                CDialog::OnOK();
            } else {

                CDialog::OnOK();
            }

        } else {
            CDialog::OnOK();
        }

    }
    else {
        SnWord newPwRequests;
        newPwRequests = HAND_PORT_CMD(PORTB, SERIAL_CMD_REQ_11);

        m_pControl->SendSerialRequests(1,&newPwRequests,4);

        SnByte g_pbFlashBuf[134];
        *(SnWord*)g_pbFlashBuf = HAND_PORT_CMD(PORTB, SERIAL_CMD_REQ_12);
        g_pbFlashBuf[2] = SERIAL_CMD_REQ_12;
        g_pbFlashBuf[3] = 0x00;
        g_pbFlashBuf[4] = 0xE0;

        SnWord len = strlen(m_tPortBStatus.pcSerialNumber);

        for(SnWord i=0; i<len; i++)
        {
            if(i==0) {
				if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_CTL)
					g_pbFlashBuf[5+i] = 'G';
				else if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_BF)
					g_pbFlashBuf[5+i] = 'G';
				else
					g_pbFlashBuf[5+i] = 'X';
            } else if (i==1) {
				if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_CTL)
					g_pbFlashBuf[5+i] = 'G';
				else if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_BF)
					g_pbFlashBuf[5+i] = 'G';
				else
					g_pbFlashBuf[5+i] = 'X';
            } else if (i==2) {
                if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_CTL)
					g_pbFlashBuf[5+i] = 'B';
				else if(m_tPortBStatus.usType == TYPE_MDU_RELIANT_BF)
					g_pbFlashBuf[5+i] = 'H';
				else
					g_pbFlashBuf[5+i] = 'X';
            } else {
				if(m_tPortBStatus.pcSerialNumber[i] >= '0' && m_tPortBStatus.pcSerialNumber[i] <= '9')
					g_pbFlashBuf[5+i] = m_tPortBStatus.pcSerialNumber[i];
				else {
					g_pbFlashBuf[5+i] = '\0';
					break;
				}
            }
        }

        // (16 bit sci cmd) + (8 bit page cmd) + (16 bit page #) +
        // (128 bytes of flash data) + Crc byte = 134 bytes
        SnQByte qPageCmdSize = 2 + 1 + 2 + 128 + 1;

        g_pbFlashBuf[128+5] = tUtil.CrcMemChunk(&g_pbFlashBuf[2],128 + 3,0);

        m_pDriver = (CDriver*)new(CDriver);
        if(m_pDriver) {

            m_pDriver->InitDriver();
            if(m_pDriver->SerialPageToDevice(qPageCmdSize, g_pbFlashBuf) == FALSE)
            {
                CDialog::OnOK();
            } else {
                CDialog::OnOK();
            }

        } else {
            CDialog::OnOK();
        }
    }
}