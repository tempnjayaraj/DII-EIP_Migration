// HandpieceCountPopUpPortA.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "HandpieceCountPopUpPortA.h"
#include "MessageBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLanguagePopUp dialog

CHandpieceCountPopUpPortA::CHandpieceCountPopUpPortA(CControl* pControl, CWnd* pParent /*=NULL*/)
	: CDialog(CHandpieceCountPopUpPortA::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHandpieceCountPopUpPortA)
	//}}AFX_DATA_INIT
	m_pControl = pControl;

	// create solid brush for background color
	m_hbrWhite = CreateSolidBrush(SN_WHITE);
	m_hbrYellow = CreateSolidBrush(SN_YELLOW);
}

void CHandpieceCountPopUpPortA::DeInit()
{
	// Delete solid brush object
	DeleteObject(m_hbrWhite);
	DeleteObject(m_hbrYellow);
}

CHandpieceCountPopUpPortA::~CHandpieceCountPopUpPortA()
{
	DeInit();
}

void CHandpieceCountPopUpPortA::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHandpieceCountPopUpPortA)
	DDX_Control(pDX, IDC_BUTTON_YES, m_BtnYes);
	DDX_Control(pDX, IDC_STATIC_TITLE, m_StaticTitle);
	DDX_Control(pDX, IDC_STATIC_TEXT, m_StaticText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHandpieceCountPopUpPortA, CDialog)
	//{{AFX_MSG_MAP(CHandpieceCountPopUpPortA)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_YES, OnButtonYes)
    ON_MESSAGE(WM_UPDATE_STATUS, UpdateStatus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHandpieceCountPopUp message handlers

HBRUSH CHandpieceCountPopUpPortA::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr;
	
	// Intercept the paint call to change the dialog control colors
	
	// Call the base class implementation first! Otherwise, it may
	// undo what we are trying to accomplish .
	hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	int nCtrlID = pWnd->GetDlgCtrlID();
	switch(nCtrlID)
	{
		case IDC_STATIC_TITLE:
			pDC->SetBkColor(SN_YELLOW);
			pDC->SetTextColor(SN_BKGND_COLOR);
			hbr = m_hbrYellow;
			break;
		
		default :
			pDC->SetBkColor(SN_WHITE);
			pDC->SetTextColor(SN_BLACK);
			hbr = m_hbrWhite;
			break;

	}
	
	return hbr;

}

LRESULT CHandpieceCountPopUpPortA::UpdateStatus(WPARAM iParam, LPARAM lParam)
{
    switch(iParam)
    {
        case MSG_UPDATE_MODE_STATUS:
            UpdateData();
            break;

        default: 
            break;
    }

    return 0;
}

BOOL CHandpieceCountPopUpPortA::OnInitDialog() 
{
	CDialog::OnInitDialog();	
	
    m_pControl->GetCmdState(GET_MC_PORTA_STATUS, &m_tPortAStatus, sizeof(SN_PORT_STATUS));

	SetupBitmaps();
	SetupFonts();
    UpdateMotorRunTime(TRUE);
    UpdateShaverPowerOnCount(TRUE);
    UpdateWindowLockMagnetPositionValue(TRUE);
	SetupDefaults();

    wHandpieceCountPopUpPortAActive = 1;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHandpieceCountPopUpPortA::UpdateData()
{
    m_pControl->GetCmdState(GET_MC_PORTA_STATUS, &m_tPortAStatus, sizeof(SN_PORT_STATUS));  
    UpdateShaverPowerOnCount(FALSE);
    UpdateMotorRunTime(FALSE);
    UpdateWindowLockMagnetPositionValue(FALSE);
    SetupDefaults();
}

void CHandpieceCountPopUpPortA::SetupBitmaps()
{
	// Setup Yes button
	m_BtnYes.LoadBitmaps(IDB_BITMAP_BTN_CHECK_MARK, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED_WHITEBK);
}

void CHandpieceCountPopUpPortA::SetupFonts()
{
	CSharedMemory mem;

	m_StaticTitle.SetFont(mem.m_Font20Normal, TRUE);
	m_StaticText.SetFont(mem.m_Font14Normal, TRUE);
}	


void CHandpieceCountPopUpPortA::SetupDefaults()
{
	SetDlgItemText( IDC_STATIC_TITLE, TEXT("Handpiece Data"));
	SetDlgItemText(IDC_STATIC_TEXT, m_csTextMotorRunTime + m_csTextShaverPowerOnCount + m_csTextWindowLockMagnetPositionValue);
}

void CHandpieceCountPopUpPortA::OnButtonYes() 
{
    wHandpieceCountPopUpPortAActive = 0;
	CDialog::OnOK();
}

void CHandpieceCountPopUpPortA::UpdateShaverPowerOnCount(SnBool yDisplay)
{
    SnQByte lCurrentShaverPowerOnCount = m_tPortAStatus.qShaverPowerOnCount;

	if (yDisplay || m_lOldShaverPowerOnCount != lCurrentShaverPowerOnCount)
	{
		m_lOldShaverPowerOnCount = lCurrentShaverPowerOnCount;
	    m_csTextShaverPowerOnCount.Format(_T("ShaverPowerOnCount = %d\n"), m_lOldShaverPowerOnCount);
	}

}

void CHandpieceCountPopUpPortA::UpdateMotorRunTime(SnBool yDisplay)
{
    SnQByte lCurrentMotorRunTime = m_tPortAStatus.qMotorRunTime;

	if (yDisplay || m_lOldMotorRunTime != lCurrentMotorRunTime)
	{
		m_lOldMotorRunTime = lCurrentMotorRunTime;
	    m_csTextMotorRunTime.Format(_T("MotorRuntimeCount = %d Minutes\n"), m_lOldMotorRunTime);
	}

}

void CHandpieceCountPopUpPortA::UpdateWindowLockMagnetPositionValue(SnBool yDisplay)
{
    SnQByte lCurrentWindowLockMagnetPositionValue = m_tPortAStatus.wWindowLockMagnetPositionValue;

	if (yDisplay || m_lOldWindowLockMagnetPositionValue != lCurrentWindowLockMagnetPositionValue)
	{
		m_lOldWindowLockMagnetPositionValue = lCurrentWindowLockMagnetPositionValue;
	    m_csTextWindowLockMagnetPositionValue.Format(_T("WindowLockPosition = %d"), m_lOldWindowLockMagnetPositionValue);
	}

}