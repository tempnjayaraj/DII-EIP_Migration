// SerialNumber.cpp : implementation file
//

#include "stdafx.h"
#include "SharedMemory.h"
#include "SnIoctl.h"
#include "Util.h"
#include "SerialNumberPopUp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SerialNumber dialog


CSerialNumberPopUp::CSerialNumberPopUp(CDriver* pDriver, CWnd* pParent /*=NULL*/)
	: CDialog(CSerialNumberPopUp::IDD, pParent)
{

    m_pParent = pParent;
	m_pDriver = pDriver;

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
	CSharedMemory mem;
    CUtil tUtil;
	
    tUtil.RestoreShaverSerialNumber(m_pDriver, m_pcSerialNumber);
    
    if (strncmp(m_pcSerialNumber, "AAX", 3) != 0) {
        strcpy(m_pcSerialNumber, "AAX");
    }

	// Setup the screen
    SetupTextButtons();
	SetupFonts();

	// Setup static text boxes
	SetDlgItemText(IDC_STATIC_TITLE, mem.GetString(SN_ENTER_SERIAL_NUMBER));
	SetDlgItemText(IDC_STATIC_MESSAGE_TEXT, mem.GetString(SN_SERIAL_NUMBER_INSTRUCTIONS));	

    CString SerialNumber = m_pcSerialNumber;
    SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, SerialNumber);

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
    int len = strlen(m_pcSerialNumber);

    if (len < 10) {
        m_pcSerialNumber[len] = cDigit;
        m_pcSerialNumber[len+1] = 0;

        CString SerialNumber = m_pcSerialNumber;
        SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, SerialNumber);	
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
    int len = strlen(m_pcSerialNumber);

    if (len > 3) {
        m_pcSerialNumber[len - 1] = 0;

        CString SerialNumber = m_pcSerialNumber;
        SetDlgItemText(IDC_STATIC_MESSAGE_TEXT2, SerialNumber);	
    }
}

void CSerialNumberPopUp::OnButtonNumpadEnter() 
{
    CUtil tUtil;

    if (tUtil.SaveShaverSerialNumber(m_pDriver, m_pcSerialNumber)) {
	    CDialog::OnOK();
    }
    // FIXME Error
}

