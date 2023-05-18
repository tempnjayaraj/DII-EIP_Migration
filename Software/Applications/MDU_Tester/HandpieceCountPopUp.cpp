// HandpieceCountPopUp.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "HandpieceCountPopUp.h"
#include "MessageBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLanguagePopUp dialog


CHandpieceCountPopUp::CHandpieceCountPopUp(CControl* pControl, CWnd* pParent /*=NULL*/)
	: CDialog(CHandpieceCountPopUp::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHandpieceCountPopUp)
	//}}AFX_DATA_INIT
	m_pControl = pControl;

	// create solid brush for background color
	m_hbrWhite = CreateSolidBrush(SN_WHITE);
	m_hbrYellow = CreateSolidBrush(SN_YELLOW);
}

void CHandpieceCountPopUp::DeInit()
{
	// Delete solid brush object
	DeleteObject(m_hbrWhite);
	DeleteObject(m_hbrYellow);
}

CHandpieceCountPopUp::~CHandpieceCountPopUp()
{
	DeInit();
}

void CHandpieceCountPopUp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHandpieceCountPopUp)
	DDX_Control(pDX, IDC_BUTTON_YES, m_BtnYes);
	DDX_Control(pDX, IDC_STATIC_TITLE, m_StaticTitle);
	DDX_Control(pDX, IDC_STATIC_TEXT, m_StaticText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHandpieceCountPopUp, CDialog)
	//{{AFX_MSG_MAP(CHandpieceCountPopUp)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_YES, OnButtonYes)
    //ON_MESSAGE(WM_UPDATE_STATUS, UpdateStatus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHandpieceCountPopUp message handlers

HBRUSH CHandpieceCountPopUp::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
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

/*LRESULT CHandpieceCountPopUp::UpdateStatus(WPARAM iParam, LPARAM lParam)
{
    switch(iParam)
    {
        case MSG_UPDATE_PORT_STATUS:
            break;
        default: 
            break;
    }

    return 0;
}*/

/*DWORD WINAPI CHandpieceCountPopUp::UpdateStatus(LPVOID pParam)
{
    CHandpieceCountPopUp *pClass = (CHandpieceCountPopUp*)pParam;
    
    while(!pClass->m_bKillThreads)
    {
        pClass->UpdateMotorRunTime(TRUE);
        pClass->UpdateShaverPowerOnCount(TRUE);
        Sleep(50);
    }

    return 0;
}*/

BOOL CHandpieceCountPopUp::OnInitDialog() 
{
	CDialog::OnInitDialog();	
	
    m_pControl->GetCmdState(GET_MC_PORTA_STATUS, &m_tPortAStatus, sizeof(SN_PORT_STATUS));

	SetupBitmaps();
	SetupFonts();
    UpdateMotorRunTime(TRUE);
    UpdateShaverPowerOnCount(TRUE);
	SetupDefaults();

    //m_hDisplayThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL, 0, UpdateStatus, this, 0, &m_hDisplayThreadID);
	
    //if(m_hDisplayThread == NULL)
    //{
    //    return FALSE;
    //}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHandpieceCountPopUp::SetupBitmaps()
{
	// Setup Yes button
	m_BtnYes.LoadBitmaps(IDB_BITMAP_BTN_CHECK_MARK, IDB_BITMAP_GRAY_BUTTON_ROUND_PRESSED_WHITEBK);
}

void CHandpieceCountPopUp::SetupFonts()
{
	CSharedMemory mem;

	m_StaticTitle.SetFont(mem.m_Font30Normal, TRUE);
	m_StaticText.SetFont(mem.m_Font20Normal, TRUE);
}	


void CHandpieceCountPopUp::SetupDefaults()
{
	SetDlgItemText( IDC_STATIC_TITLE, TEXT("Handpiece Data"));
	SetDlgItemText(IDC_STATIC_TEXT, m_csTextMotorRunTime + m_csTextShaverPowerOnCount);
}

void CHandpieceCountPopUp::OnButtonYes() 
{
	CDialog::OnOK();
}

void CHandpieceCountPopUp::UpdateShaverPowerOnCount(SnBool yDisplay)
{
    SnQByte lCurrentShaverPowerOnCount = m_tPortAStatus.qShaverPowerOnCount;

	if (yDisplay || m_lOldShaverPowerOnCount != lCurrentShaverPowerOnCount)
	{
		m_lOldShaverPowerOnCount = lCurrentShaverPowerOnCount;
	    m_csTextShaverPowerOnCount.Format(_T("ShaverPowerOnCount= %d"), m_lOldShaverPowerOnCount);
	}

}

void CHandpieceCountPopUp::UpdateMotorRunTime(SnBool yDisplay)
{
    SnQByte lCurrentMotorRunTime = m_tPortAStatus.qMotorRunTime;

	if (yDisplay || m_lOldMotorRunTime != lCurrentMotorRunTime)
	{
		m_lOldMotorRunTime = lCurrentMotorRunTime;
	    m_csTextMotorRunTime.Format(_T("MotorRuntimeCount= %d\n"), m_lOldMotorRunTime);
	}

}
