// SoftwareRepairDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SoftwareUpgrade.h"
#include "SoftwareUpgradeApp.h"
#include "SoftwareRepairDlg.h"
#include "SharedMemory.h"
#include "SnIoctl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSoftwareRepairDlg dialog


CSoftwareRepairDlg::CSoftwareRepairDlg(CDriver* pDriver, SnBool yLowerFlashValid,
        SnBool yUpperFlashValid, CWnd* pParent /*=NULL*/)
	: CDialog(CSoftwareRepairDlg::IDD, pParent)
{
	m_pDriver = pDriver;
    m_yLowerFlashValid = yLowerFlashValid;
    m_yUpperFlashValid = yUpperFlashValid;
	m_bDetail = FALSE;

    // Create solid brush for background color
	m_hBrush = CreateSolidBrush(BLACK);

	//{{AFX_DATA_INIT(CSoftwareRepairDlg)
	//}}AFX_DATA_INIT
}

CSoftwareRepairDlg::~CSoftwareRepairDlg()
{
    // Delete solid brush object
	DeleteObject(m_hBrush);
}

void CSoftwareRepairDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSoftwareRepairDlg)
	DDX_Control(pDX, IDC_STATIC_REPAIR_STATUS3, m_StaticStatus3);
	DDX_Control(pDX, IDC_STATIC_REPAIR_STATUS2, m_StaticStatus2);
	DDX_Control(pDX, IDC_STATIC_REPAIR_STATUS1, m_StaticStatus1);
	DDX_Control(pDX, IDC_STATIC_TITLE, m_StaticTitle);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSoftwareRepairDlg, CDialog)
	//{{AFX_MSG_MAP(CSoftwareRepairDlg)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_DONE, OnButtonDone)
	ON_WM_DRAWITEM()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSoftwareRepairDlg message handlers

HBRUSH CSoftwareRepairDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hBrush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// Set the Background color
	pDC->SetBkColor(BLACK);
	pDC->SetTextColor(WHITE);
	
	return m_hBrush;
}

BOOL CSoftwareRepairDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

    SetWindowPos(CWnd::FromHandle(HWND_TOPMOST), 0, 0, 0, 0,
        SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);

    CSharedMemory mem;
	BOOL bStatus = mem.GetInitStatus();
	if( !bStatus)
		mem.Init(m_pDriver);

	m_bDetail = mem.GetStringDetailMode(); // Check to see if we need to display detailed upgrade status

	SetupFonts();
    SetupButtons();
	

	SetDlgItemText(IDC_STATIC_TITLE, mem.GetString(SN_FLASH_DATA_REPAIR));

	// Create the thread that repairs the flash image
	CreateThread((LPSECURITY_ATTRIBUTES)NULL,
					 0,
					 RepairFlashThread,
					 this,
					 0,
					 NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSoftwareRepairDlg::SetupFonts(void)
{
    CSharedMemory mem;

    m_StaticTitle.SetFont(mem.m_Font30Bold, TRUE);
    m_StaticStatus1.SetFont(mem.m_Font25Normal, TRUE);
    m_StaticStatus2.SetFont(mem.m_Font25Normal, TRUE);
    m_StaticStatus3.SetFont(mem.m_Font25Normal, TRUE);
}

void CSoftwareRepairDlg::SetupButtons(void)
{
    //
	// Setup Done button 
	//
	m_BtnDone.AutoLoad(IDC_BUTTON_DONE, this);
	m_BtnDone.LoadBitmaps(IDB_BITMAP_BK_BUTTON_UP, IDB_BITMAP_BK_BUTTON_DOWN, NULL, NULL);
	m_BtnDone.SizeToContent();
    m_BtnDone.ShowWindow(SW_HIDE);
}

SnBool CSoftwareRepairDlg::GetValidFlashImage(SnQByte qAddr, SnByte **ppbData, SnQByte *pqSize)
{
    SnQByte qOffs, qSize;
    SnWinCeHdr tHdr;
    SnByte *pbData;
    CUtil tUtil;

    if (m_pDriver->ReadFlashData(qAddr, (SnByte *)&tHdr, sizeof(SnWinCeHdr)) == FALSE) {
		return FALSE;
	}

    if (tUtil.CrcMem((SnByte *)&tHdr, sizeof(SnWinCeHdr)) != 0) {
        return FALSE;
    }

    qSize = TOTAL_SIZE_OF_CE_DATA(tHdr);
    pbData = *ppbData = (SnByte *)malloc(qSize);
	if (pbData == NULL) {
		return FALSE;
	}
    if (m_pDriver->ReadFlashData(qAddr, pbData, qSize) == FALSE) {
		return FALSE;
	}

    qOffs = OFFSET_OF_CE_SPLASH;
    if (tUtil.CrcMem(pbData + qOffs, tHdr.qSplashRomLen) != tHdr.bSplashCrc) {
		return FALSE;
    }

    qOffs = OFFSET_OF_CE_ROM(tHdr);
    if (tUtil.CrcMem(pbData + qOffs, tHdr.qNkRomLen) != tHdr.bNkCrc) {
		return FALSE;
    }
    
    *pqSize = qSize;

    return TRUE;
}

SnBool CSoftwareRepairDlg::CopyFlashImage(SnQByte qSrcAddr, SnQByte qDstAddr)
{
    SnByte *pbData = NULL;
    SnQByte qBytes = 0;
    SnBool yRet;
	CSharedMemory mem;

    if (m_bDetail) {
		SetDlgItemText(IDC_STATIC_REPAIR_STATUS2, mem.GetString(SN_READING_FLASH_DATA));
    } else {
        SetDlgItemText(IDC_STATIC_REPAIR_STATUS2, mem.GetString(SN_WRITING_FLASH_DATA));
    }
	yRet = GetValidFlashImage(qSrcAddr, &pbData, &qBytes);
    if (yRet) {
        if (m_bDetail)
            SetDlgItemText(IDC_STATIC_REPAIR_STATUS2, mem.GetString(SN_WRITING_FLASH_DATA));
        yRet = m_pDriver->EraseFlashPages(qDstAddr, FLASH_BYTES_TO_PAGES(qBytes));
        if (yRet)
            yRet = m_pDriver->WriteFlashData(qDstAddr, pbData, qBytes);
        if (yRet) {
            if (m_bDetail)
				SetDlgItemText(IDC_STATIC_REPAIR_STATUS2, mem.GetString(SN_VERIFYING_FLASH_DATA));
            yRet = m_pDriver->VerifyFlashData(qDstAddr, pbData, qBytes);
            if (!yRet) {
                if (m_bDetail)
					SetDlgItemText(IDC_STATIC_REPAIR_STATUS2, mem.GetString(SN_VERIFYING_FLASH_DATA_FAILED));
            }
        } else {
            if (m_bDetail)
				SetDlgItemText(IDC_STATIC_REPAIR_STATUS2, mem.GetString(SN_WRITING_FLASH_DATA_FAILED));
        }
    } else {
        if (m_bDetail)
			SetDlgItemText(IDC_STATIC_REPAIR_STATUS2, mem.GetString(SN_READING_FLASH_DATA_FAILED));
    }

    if (yRet) {
        SetDlgItemText(IDC_STATIC_REPAIR_STATUS2, mem.GetString(SN_REPAIR_COMPLETE));
    } else {
        if (m_bDetail == FALSE)
		    SetDlgItemText(IDC_STATIC_REPAIR_STATUS2, mem.GetString(SN_WRITING_FLASH_DATA_FAILED));
    }

    if (pbData) {
        free(pbData);
        pbData = NULL;
    }

    return yRet;
}

void CSoftwareRepairDlg::RepairFlashImage(void)
{
   CSharedMemory mem;

	if (m_yLowerFlashValid == FALSE && m_yUpperFlashValid == FALSE) {
		SetDlgItemText(IDC_STATIC_REPAIR_STATUS1, mem.GetString(SN_ALL_FLASH_DATA_CORRUPT_REPAIR_NOT_POSSIBLE));
    } else {
        if (m_yLowerFlashValid == FALSE && m_yUpperFlashValid) {
            SetDlgItemText(IDC_STATIC_REPAIR_STATUS1, mem.GetString(SN_LOWER_FLASH_DATA_CORRUPT));
            CopyFlashImage(UPPER_FLASH_ADDR, LOWER_FLASH_ADDR);
        } else if (m_yLowerFlashValid && m_yUpperFlashValid == FALSE) {
            SetDlgItemText(IDC_STATIC_REPAIR_STATUS1, mem.GetString(SN_UPPER_FLASH_DATA_CORRUPT));
            CopyFlashImage(LOWER_FLASH_ADDR, UPPER_FLASH_ADDR);
        }
    }

    SetDlgItemText(IDC_STATIC_REPAIR_STATUS3, mem.GetString(SN_POWER_DOWN_SYSTEM));

    for (;;) {
        Sleep(INFINITE);
    }
}

DWORD WINAPI RepairFlashThread(LPVOID pParam)
{
    CSoftwareRepairDlg *pClass = (CSoftwareRepairDlg *)pParam;

    pClass->RepairFlashImage();

    return 0;
}

void CSoftwareRepairDlg::OnButtonDone() 
{
    OnOK();
}

void CSoftwareRepairDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CSharedMemory mem;
    CUtil tUtil;

    if( (lpDrawItemStruct->itemAction == ODA_FOCUS))
	{
		// Don't do anything, we don't want a button redrawn
		// when it gains or loses focus
	}
	else
	{
		// Call the base class implementation first! Otherwise, it may
		// undo what we are trying to accomplish .
		CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);

		switch( nIDCtl) {
	    case IDC_BUTTON_DONE:	
				tUtil.DrawTextOnButton(&m_BtnDone,
										  mem.m_Font20Normal,
										  mem.GetString(SN_DONE));
			break;
        }
    }
}
