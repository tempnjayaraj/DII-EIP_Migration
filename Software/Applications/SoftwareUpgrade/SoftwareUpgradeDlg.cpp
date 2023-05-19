// SoftwareUpgradeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SoftwareUpgrade.h"
#include "SoftwareUpgradeApp.h"
#include "SoftwareUpgradeDlg.h"
#include "SerialNumberPopUp.h"
#include "SharedMemory.h"
#include "SnIoctl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSoftwareUpgradeDlg dialog

CSoftwareUpgradeDlg::CSoftwareUpgradeDlg(CDriver* pDriver, FILE *ptFile,
    CWnd* pParent /*=NULL*/) : CDialog(CSoftwareUpgradeDlg::IDD, pParent)
{
    m_pDriver = pDriver;
    m_ptFile = ptFile;
	m_bDetail = FALSE;

    m_hStartCancelEvent = NULL;
    m_yCancel = FALSE;
    m_ySerialNumberCheck = TRUE;

    m_tCurSysRev.bValid = FALSE;
    m_tCurMotorRev.bValid = FALSE;
#if RS485_FOOT_UPGRADE
    m_tCurFootRev.bValid = FALSE;
#endif
#if RS485_MDU_UPGRADE
    m_tCurMduRev.bValid = FALSE;
#endif
    m_tUpgSysRev.bValid = FALSE;
    m_tUpgMotorRev.bValid = FALSE;
#if RS485_FOOT_UPGRADE
    m_tUpgFootRev.bValid = FALSE;
#endif
#if RS485_MDU_UPGRADE
    m_tUpgMduRev.bValid = FALSE;
#endif

    // Create solid brush for background color
	m_hBrush = CreateSolidBrush(BLACK);
	
    memset(m_pcSerialNumber, 0, SERIAL_NUMBER_SIZE);

    //{{AFX_DATA_INIT(CSoftwareUpgradeDlg)
	//}}AFX_DATA_INIT
}

CSoftwareUpgradeDlg::~CSoftwareUpgradeDlg()
{
    // Delete solid brush object
	DeleteObject(m_hBrush);

    // Delete Event Handles
	if (m_hStartCancelEvent) {
		CloseHandle(m_hStartCancelEvent);
		m_hStartCancelEvent = NULL;
	}
}

void CSoftwareUpgradeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSoftwareUpgradeDlg)
	DDX_Control(pDX, IDC_STATIC_UPGRADE_STATUS3, m_StaticStatus3);
	DDX_Control(pDX, IDC_STATIC_UPGRADE_STATUS2, m_StaticStatus2);
	DDX_Control(pDX, IDC_STATIC_UPGRADE_STATUS1, m_StaticStatus1);
	DDX_Control(pDX, IDC_STATIC_TITLE, m_StaticTitle);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSoftwareUpgradeDlg, CDialog)
	//{{AFX_MSG_MAP(CSoftwareUpgradeDlg)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_DONE, OnButtonDone)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_BUTTON_START, OnButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	ON_MESSAGE(IDC_BUTTON_DONE, OnButtonDone)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSoftwareUpgradeDlg message handlers

BOOL CSoftwareUpgradeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0,
        SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);

    CSharedMemory mem;
	BOOL bStatus = mem.GetInitStatus();
	if( !bStatus)
		mem.Init(m_pDriver);

	m_bDetail = mem.GetStringDetailMode(); // Check to see if we need to display detailed upgrade status

	SetupFonts();
    SetupButtons();

    SetupSerialNumberDisplay();

    if (m_pcSerialNumber[0] == 0) {
        CSerialNumberPopUp dlg(m_pDriver);
        dlg.DoModal();
        SetupSerialNumberDisplay();
    }

	// Create event to indicate start of upgrade
	m_hStartCancelEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if(m_hStartCancelEvent == NULL) {
		return FALSE;
	}

    // Create the thread that updates the flash image
	CreateThread((LPSECURITY_ATTRIBUTES)NULL,
					 0,
					 UpgradeSoftwareThread,
					 this,
					 0,
					 NULL);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSoftwareUpgradeDlg::SetupFonts(void)
{
    CSharedMemory mem;

    m_StaticTitle.SetFont(mem.m_Font30Bold, TRUE);
    m_StaticStatus1.SetFont(mem.m_Font25Normal, TRUE);
    m_StaticStatus2.SetFont(mem.m_Font25Normal, TRUE);
    m_StaticStatus3.SetFont(mem.m_Font25Normal, TRUE);
}

void CSoftwareUpgradeDlg::SetupButtons(void)
{
    //
	// Setup Cancel button 
	//
	m_BtnCancel.AutoLoad(IDC_BUTTON_CANCEL, this);
	m_BtnCancel.LoadBitmaps(IDB_BITMAP_BK_BUTTON_UP, IDB_BITMAP_BK_BUTTON_DOWN, NULL, NULL);
	m_BtnCancel.SizeToContent();

    //
	// Setup Start button 
	//
	m_BtnStart.AutoLoad(IDC_BUTTON_START, this);
	m_BtnStart.LoadBitmaps(IDB_BITMAP_BK_BUTTON_UP, IDB_BITMAP_BK_BUTTON_DOWN, NULL, NULL);
	m_BtnStart.SizeToContent();

    //
	// Setup Done button 
	//
	m_BtnDone.AutoLoad(IDC_BUTTON_DONE, this);
	m_BtnDone.LoadBitmaps(IDB_BITMAP_BK_BUTTON_UP, IDB_BITMAP_BK_BUTTON_DOWN, NULL, NULL);
	m_BtnDone.SizeToContent();
}

#if RS485_FOOT_UPGRADE || RS485_MDU_UPGRADE
void CSoftwareUpgradeDlg::SetupRevisions(void)
{
    CString csUpgRev, csCurRev;
    CString csUpdate;
	CString csTemp1, csTemp2;
	CSharedMemory mem;

    GetFileRevisions();

	GetSystemRevision(&m_tCurSysRev);
	GetMotorBoardRevision(&m_tCurMotorRev);

#if RS485_FOOT_UPGRADE
	if (m_tUpgFootRev.bValid) {
		GetFootswitchRevision(&m_tCurFootRev);

        // For the footswitch, if the Major versions do not match then there is a upgrade file mis-match
        // Major 1: Linemaster Footswitch
        // Major 2: Steute Footswitch
        if (m_tUpgFootRev.bValid && m_tCurFootRev.bValid &&
            m_tUpgFootRev.bMajor != m_tCurFootRev.bMajor)
            m_tUpgFootRev.bValid = FALSE;
    }
#endif
#if RS485_MDU_UPGRADE
	if (m_tUpgMduRev.bValid) {
		GetMduRevision(&m_tCurMduRev);
    }
#endif

	if (m_tUpgSysRev.bValid || m_tUpgMotorRev.bValid) {
#if RS485_FOOT_UPGRADE
        m_tUpgFootRev.bValid = FALSE;
#endif
#if RS485_MDU_UPGRADE
        m_tUpgMduRev.bValid = FALSE;
#endif
		if (m_tCurSysRev.bValid) {
			csCurRev.Format(TEXT("%d.%02d.%02d"),
				m_tCurSysRev.bMajor, m_tCurSysRev.bMinor, m_tCurSysRev.bBuild);
		} else {
			csCurRev = mem.GetString(SN_UNKNOWN);
		}
		if (m_tUpgSysRev.bValid) {
			csUpgRev.Format(TEXT("%d.%02d.%02d"),
				m_tUpgSysRev.bMajor, m_tUpgSysRev.bMinor, m_tUpgSysRev.bBuild);
		}

 		if (m_tUpgSysRev.bValid) {
			csTemp1 = mem.GetString(SN_UPDATE_SYSTEM_SOFTWARE);
			csTemp2 = mem.GetString(SN_TO);

			csUpdate.Format(TEXT("%s%s %s %s"), csTemp1, csCurRev, csTemp2, csUpgRev);
		} else {
			csTemp1 = mem.GetString(SN_SYSTEM_SOFTWARE);
			csTemp2 = mem.GetString(SN_NO_CHANGE);

			csUpdate.Format(TEXT("%s%s %s"), csTemp1, csCurRev, csTemp2);
		}
		SetDlgItemText(IDC_STATIC_UPGRADE_STATUS1, csUpdate);

		if (m_tCurMotorRev.bValid) {
			csCurRev.Format(TEXT("%d.%02d.%02d"),
				m_tCurMotorRev.bMajor, m_tCurMotorRev.bMinor, m_tCurMotorRev.bBuild);
		} else {
			csCurRev = mem.GetString(SN_UNKNOWN);
		}
		if (m_tUpgMotorRev.bValid) {
			csUpgRev.Format(TEXT("%d.%02d.%02d"),
				m_tUpgMotorRev.bMajor, m_tUpgMotorRev.bMinor, m_tUpgMotorRev.bBuild);
		}

		if (m_tUpgMotorRev.bValid && 
			(m_tUpgMotorRev.bMajor != m_tCurMotorRev.bMajor || 
			m_tUpgMotorRev.bMinor != m_tCurMotorRev.bMinor ||
			m_tUpgMotorRev.bBuild != m_tCurMotorRev.bBuild)) {
			csTemp1 = mem.GetString(SN_UPDATE_MOTOR_CONTROLLER_SOFTWARE);
			csTemp2 = mem.GetString(SN_TO);
			csUpdate.Format(TEXT("%s%s %s %s"), csTemp1, csCurRev, csTemp2, csUpgRev);
		} else {
			csTemp1 = mem.GetString(SN_MOTOR_CONTROLLER_SOFTWARE);
			csTemp2 = mem.GetString(SN_NO_CHANGE);
			csUpdate.Format(TEXT("%s%s %s"), csTemp1,
				m_tUpgMotorRev.bValid ? csUpgRev : csCurRev, csTemp2);
		}
		SetDlgItemText(IDC_STATIC_UPGRADE_STATUS2, csUpdate);
#if RS485_MDU_UPGRADE
	} else if (m_tUpgMduRev.bValid) {
		if (m_tCurMduRev.bValid) {
			csCurRev.Format(TEXT("%d.%02d.%02d"),
				m_tCurMduRev.bMajor, m_tCurMduRev.bMinor, m_tCurMduRev.bBuild);
			csUpgRev.Format(TEXT("%d.%02d.%02d"),
				m_tUpgMduRev.bMajor, m_tUpgMduRev.bMinor, m_tUpgMduRev.bBuild);

			csTemp1 = mem.GetString(SN_UPDATE_MDU_SOFTWARE);
			csTemp2 = mem.GetString(SN_TO);
			csUpdate.Format(TEXT("%s%s %s %s"), csTemp1, csCurRev, csTemp2, csUpgRev);
			SetDlgItemText(IDC_STATIC_UPGRADE_STATUS2, csUpdate);
		} else {
			m_BtnStart.ShowWindow(SW_HIDE);
			SetDlgItemText(IDC_STATIC_UPGRADE_STATUS2, mem.GetString(SN_NO_MDU_DETECTED));
		}
		SetDlgItemText(IDC_STATIC_UPGRADE_STATUS1, SN_CLEAR_TEXT);
#endif
#if RS485_FOOT_UPGRADE
	} else if (m_tUpgFootRev.bValid) {
		if (m_tCurFootRev.bValid) {
			csCurRev.Format(TEXT("%d.%02d.%02d"),
				m_tCurFootRev.bMajor, m_tCurFootRev.bMinor, m_tCurFootRev.bBuild);
			csUpgRev.Format(TEXT("%d.%02d.%02d"),
				m_tUpgFootRev.bMajor, m_tUpgFootRev.bMinor, m_tUpgFootRev.bBuild);

			csTemp1 = mem.GetString(SN_UPDATE_FOOTSWITCH_SOFTWARE);
			csTemp2 = mem.GetString(SN_TO);
			csUpdate.Format(TEXT("%s%s %s %s"), csTemp1, csCurRev, csTemp2, csUpgRev);

			SetDlgItemText(IDC_STATIC_UPGRADE_STATUS2, csUpdate);
		} else {
			m_BtnStart.ShowWindow(SW_HIDE);
			SetDlgItemText(IDC_STATIC_UPGRADE_STATUS2, mem.GetString(SN_NO_FOOTSWITCH_DETECTED));
		}
		SetDlgItemText(IDC_STATIC_UPGRADE_STATUS1, SN_CLEAR_TEXT);
#endif
	} else {
		m_BtnStart.ShowWindow(SW_HIDE);
		SetDlgItemText(IDC_STATIC_UPGRADE_STATUS2, mem.GetString(SN_INVALID_UPGRADE_FILE));
		SetDlgItemText(IDC_STATIC_UPGRADE_STATUS1, SN_CLEAR_TEXT);
	}
}
#else
void CSoftwareUpgradeDlg::SetupRevisions(void)
{
    CString csUpgRev, csCurRev;
    CString csUpdate;
	CString csTemp1, csTemp2;
	CSharedMemory mem;

    GetSystemRevision(&m_tCurSysRev);
    GetMotorBoardRevision(&m_tCurMotorRev);
    GetFileRevisions();

	if (m_tCurSysRev.bValid) {
        csCurRev.Format(TEXT("%d.%02d.%02d"),
            m_tCurSysRev.bMajor, m_tCurSysRev.bMinor, m_tCurSysRev.bBuild);
    } else {
        csCurRev = mem.GetString(SN_UNKNOWN);
    }
    if (m_tUpgSysRev.bValid) {
        csUpgRev.Format(TEXT("%d.%02d.%02d"),
            m_tUpgSysRev.bMajor, m_tUpgSysRev.bMinor, m_tUpgSysRev.bBuild);
    }

 	if (m_tUpgSysRev.bValid) {
		csTemp1 = mem.GetString(SN_UPDATE_SYSTEM_SOFTWARE);
		csTemp2 = mem.GetString(SN_TO);

        csUpdate.Format(TEXT("%s%s %s %s"), csTemp1, csCurRev, csTemp2, csUpgRev);
    } else {
		csTemp1 = mem.GetString(SN_SYSTEM_SOFTWARE);
		csTemp2 = mem.GetString(SN_NO_CHANGE);

        csUpdate.Format(TEXT("%s%s %s"), csTemp1, csCurRev, csTemp2);
    }
    SetDlgItemText(IDC_STATIC_UPGRADE_STATUS1, csUpdate);

	if (m_tCurMotorRev.bValid) {
        csCurRev.Format(TEXT("%d.%02d.%02d"),
            m_tCurMotorRev.bMajor, m_tCurMotorRev.bMinor, m_tCurMotorRev.bBuild);
    } else {
        csCurRev = mem.GetString(SN_UNKNOWN);
    }
    if (m_tUpgMotorRev.bValid) {
        csUpgRev.Format(TEXT("%d.%02d.%02d"),
            m_tUpgMotorRev.bMajor, m_tUpgMotorRev.bMinor, m_tUpgMotorRev.bBuild);
    }

	if (m_tUpgMotorRev.bValid && 
        (m_tUpgMotorRev.bMajor != m_tCurMotorRev.bMajor || 
        m_tUpgMotorRev.bMinor != m_tCurMotorRev.bMinor ||
        m_tUpgMotorRev.bBuild != m_tCurMotorRev.bBuild)) {
		csTemp1 = mem.GetString(SN_UPDATE_MOTOR_CONTROLLER_SOFTWARE);
		csTemp2 = mem.GetString(SN_TO);
        csUpdate.Format(TEXT("%s%s %s %s"), csTemp1, csCurRev, csTemp2, csUpgRev);
    } else {
		csTemp1 = mem.GetString(SN_MOTOR_CONTROLLER_SOFTWARE);
		csTemp2 = mem.GetString(SN_NO_CHANGE);
        csUpdate.Format(TEXT("%s%s %s"), csTemp1,
            m_tUpgMotorRev.bValid ? csUpgRev : csCurRev, csTemp2);
    }
    SetDlgItemText(IDC_STATIC_UPGRADE_STATUS2, csUpdate);
}
#endif

void CSoftwareUpgradeDlg::SetupUpgradeDisplay()
{
	CSharedMemory mem;

    m_BtnDone.ShowWindow(SW_HIDE);
    m_BtnStart.RedrawWindow();
    m_BtnCancel.RedrawWindow();
    SetupRevisions();
	SetDlgItemText(IDC_STATIC_TITLE, mem.GetString(SN_SOFTWARE_UPGRADE));
}

void CSoftwareUpgradeDlg::SetupSerialNumberDisplay()
{
	CSharedMemory mem;
	CString csTemp;
    CUtil tUtil;
	
    tUtil.RestoreShaverSerialNumber(m_pDriver, m_pcSerialNumber);

    SetDlgItemText(IDC_STATIC_TITLE, mem.GetString(SN_CHECK_SERIAL_NUMBER));
    csTemp = mem.GetString(SN_SERIAL_NUMBER);
    csTemp += m_pcSerialNumber;
    SetDlgItemText(IDC_STATIC_UPGRADE_STATUS1, csTemp);
    SetDlgItemText(IDC_STATIC_UPGRADE_STATUS2, mem.GetString(SN_VERIFY_SERIAL_NUMBER));
}

HBRUSH CSoftwareUpgradeDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hBrush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// Set the Background color
	pDC->SetBkColor(BLACK);
	pDC->SetTextColor(WHITE);
	
	return m_hBrush;
}

SnBool CSoftwareUpgradeDlg::GetMotorBoardRevision(SN_REVISION* ptRevision)
{
	SnBool yStatus;
	SnWord pwRevision[2];

	// Get the revision numbers from the Motor Control Board
	yStatus = m_pDriver->ReadWordsFromDevice(MC_REVISION, 2, pwRevision);
	if(!yStatus)
		return FALSE;

    ptRevision->bMajor = (pwRevision[1] & 0xff);
	ptRevision->bMinor = pwRevision[0] >> 8;
	ptRevision->bBuild = (pwRevision[0] & 0xff);
	ptRevision->bValid = TRUE;

    return TRUE;
}

SnBool CSoftwareUpgradeDlg::GetSystemRevision(SN_REVISION* ptRevision)
{
	SnWinCeHdr tWinCeHdr;
	SnBool yStatus;
    CUtil tUtil;

    // Get the system Revision Numbers

    // First try the lower image at offset
	yStatus =  m_pDriver->ReadFlashData(LOWER_FLASH_ADDR, (SnByte*)&tWinCeHdr,
        sizeof(SnWinCeHdr));
	if(!yStatus)
		return FALSE;

    // If the Header data is corrupt, try the upper offset
    if (tUtil.CrcMem((SnByte *)&tWinCeHdr, sizeof(SnWinCeHdr)) != 0) {
	    yStatus =  m_pDriver->ReadFlashData(UPPER_FLASH_ADDR, (SnByte*)&tWinCeHdr,
            sizeof(SnWinCeHdr));
	    if(!yStatus)
		    return FALSE;

        // If both images are corrupt, then return an error
        if (tUtil.CrcMem((SnByte *)&tWinCeHdr, sizeof(SnWinCeHdr)) != 0) {
            return FALSE;
        }
    }

	ptRevision->bMajor = tWinCeHdr.bMajorVers;
	ptRevision->bMinor = tWinCeHdr.bMinorVers;
	ptRevision->bBuild = tWinCeHdr.bBuildVers;
	ptRevision->bValid = TRUE;

	return TRUE;
}

#if RS485_FOOT_UPGRADE || RS485_MDU_UPGRADE
SnBool CSoftwareUpgradeDlg::ErrorCorrectSerialResponse(SnWord *pwResponse)
{
    SnWordFlavors twResponse;
    SnByte bECC;
    int iBit;

    twResponse.w = *pwResponse;

    // Compute ECC

    // Physical bits:      B15 B14 B13 B12 B11 B10  B9 B8 B7 B6 B5 B4 B3 B2 B1 B0
    // Physical mapping is:  X  E4  E3  E2  E1 D11 D10 D9 D8 D7 D6 D5 D4 D3 D2 D1
    // Logical address is:    15  14 13 12 11 10  9  8  7  6  5  4  3  2  1
    //                         1   1  1  1  1  1  1  1  0  0  0  0  0  0  0
    //                         1   1  1  1  0  0  0  0  1  1  1  1  0  0  0
    //                         1   1  0  0  1  1  0  0  1  1  0  0  1  1  0
    //                         1   0  1  0  1  0  1  0  1  0  1  0  1  0  1
    // Logical mapping is:   D11 D10 D9 D8 D7 D6 D5 e8 D4 D3 D2 e4 D1 e2 e1
    // e1 = d1 ^ d2 ^ d4 ^ d5 ^ d7 ^ d9 ^ d11;
    // e2 = d1 ^ d3 ^ d4 ^ d6 ^ d7 ^ d10 ^ d11; 
    // e4 = d2 ^ d3 ^ d4 ^ d8 ^ d9 ^ d10 ^ d11;
    // e8 = d5 ^ d6 ^ d7 ^ d8 ^ d9 ^ d10 ^ d11;
    bECC =
        (twResponse.B0 ^ twResponse.B1 ^ twResponse.B3 ^ twResponse.B4
         ^ twResponse.B6 ^ twResponse.B8 ^ twResponse.B10 ^ twResponse.B11)       // E1
        |
        ((twResponse.B0 ^ twResponse.B2 ^ twResponse.B3 ^ twResponse.B5
         ^ twResponse.B6 ^ twResponse.B9 ^ twResponse.B10 ^ twResponse.B12) << 1) // E2
        |
        ((twResponse.B1 ^ twResponse.B2 ^ twResponse.B3 ^ twResponse.B7
         ^ twResponse.B8 ^ twResponse.B9 ^ twResponse.B10 ^ twResponse.B13) << 2) // E4
        |
        ((twResponse.B4 ^ twResponse.B5 ^ twResponse.B6 ^ twResponse.B7
         ^ twResponse.B8 ^ twResponse.B9 ^ twResponse.B10 ^ twResponse.B14) << 3) // E8
        ;

    for (iBit = 0; iBit < 15; iBit++) {
        twResponse.yParity ^= (twResponse.w >> iBit) & 1;
    }

    // Perform error detection / correction
    // Parity   ECC     Action
    // 0        0       No errors
    // 0        1-15    Double error detected
    // 1        0       Parity bit wrong
    // 1        1-15    Single error, corrected

    if (!twResponse.yParity && bECC) {
        // Double error detected, fail
        twResponse.yAckNak = FALSE;
    } else if (twResponse.yParity && bECC) {
        // Perform error correction
        switch(bECC) {
        default:    //FallThrough - values other than 0-7 cannot happen
        case 0:
            // No error
            break;

        case 1:
            // Fix E1, ignore
            break;
        case 2:
            // Fix E2, ignore
            break;
        case 3:
            // Fix D1
            twResponse.B0 ^= 1;
            break;
        case 4:
            // Fix E4, ignore
            break;
        case 5:
            // Fix D2
            twResponse.B1 ^= 1;
            break;
        case 6:
            // Fix D3
            twResponse.B2 ^= 1;
            break;
        case 7:
            // Fix D4
            twResponse.B3 ^= 1;
            break;
        case 8:
            // Fix E8, ignore
            break;
        case 9:
            // Fix D5
            twResponse.B4 ^= 1;
            break;
        case 10:
            // Fix D6
            twResponse.B5 ^= 1;
            break;
        case 11:
            // Fix D7
            twResponse.B6 ^= 1;
            break;
        case 12:
            // Fix D8
            twResponse.B7 ^= 1;
            break;
        case 13:
            // Fix D9
            twResponse.B8 ^= 1;
            break;
        case 14:
            // Fix D10
            twResponse.B9 ^= 1;
            break;
        case 15:
            // Fix D11
            twResponse.B10 ^= 1;
            break;
        }
    }

    *pwResponse = twResponse.w;

    return twResponse.yAckNak;
}

SnBool CSoftwareUpgradeDlg::SendSerialRequests(SnQByte qNumRequests, SnWord *pwResults, SnWord wTimeout)
{
    SnQByte qRetrys = 0;
    SnBool yStatus;

    do {
        SnQByte qCnt;

        yStatus = m_pDriver->SerialCmdsToDevice(MC_SERIAL_CMD_RESULTS, qNumRequests, pwResults, wTimeout);

        if (yStatus) {
            for (qCnt = 0; qCnt < qNumRequests; qCnt++) {
                if (ErrorCorrectSerialResponse(&pwResults[qCnt]) == FALSE)
                    yStatus = FALSE;
            }
        }
    } while (!yStatus && ++qRetrys < 3);

    return yStatus;
}
#endif

#if RS485_FOOT_UPGRADE
SnBool CSoftwareUpgradeDlg::GetFootswitchRevision(SN_REVISION* ptRevision)
{
	SnWord wFootSwVers;
	SnWord wDevType;

    wFootSwVers = FOOT_FRONT_CMD(SERIAL_CMD_VERS);
    if (SendSerialRequests(1, &wFootSwVers, 4) == FALSE) {
		return FALSE;
    }

    wDevType = FOOT_FRONT_CMD(SERIAL_CMD_DEV_TYPE);
    if (SendSerialRequests(1, &wDevType, 4) == FALSE) {
		return FALSE;
    }

	if ((wDevType & 0x3ff) != 1) {
		return FALSE;
	}

    ptRevision->bMajor = ((wFootSwVers >> 8) & 3);
	ptRevision->bMinor = (wFootSwVers >> 4) & 0xf;
	ptRevision->bBuild = wFootSwVers & 0xf;
	ptRevision->bValid = TRUE;

    return TRUE;
}
#endif

#if RS485_MDU_UPGRADE
SnBool CSoftwareUpgradeDlg::GetMduRevision(SN_REVISION* ptRevision)
{
	SnWord wMduSwVers;
	SnWord wDevType;

    wMduSwVers = HAND_PORTA_CMD(SERIAL_CMD_VERS);
    if (SendSerialRequests(1, &wMduSwVers, 4) == FALSE) {
		return FALSE;
    }

    wDevType = HAND_PORTA_CMD(SERIAL_CMD_DEV_TYPE);
    if (SendSerialRequests(1, &wDevType, 4) == FALSE) {
		return FALSE;
    }
    wDevType &= 0x3ff;

    //
    // This contains the qType for the MDU
    //
    switch (m_tUpgMduRev.bValid) {
    case HDR_RS485_MDU_SJ:
	    if (wDevType != 2 && wDevType != 5 && wDevType != 6) {
		    return FALSE;
	    }
        break;
    case HDR_RS485_MDU_RL:
	    if (wDevType != 3 && wDevType != 4 && wDevType != 7) {
		    return FALSE;
	    }
        break;
    }

    ptRevision->bMajor = ((wMduSwVers >> 8) & 3);
	ptRevision->bMinor = (wMduSwVers >> 4) & 0xf;
	ptRevision->bBuild = wMduSwVers & 0xf;
	ptRevision->bValid = TRUE;

    return TRUE;
}
#endif

SnBool CSoftwareUpgradeDlg::GetFileRevisions(void)
{
    SnWinCeHdr tWinCeHdr;
    SnMotorHdr tMotorHdr;
#if RS485_FOOT_UPGRADE
	SnRs485Hdr tFootHdr;
#endif
#if RS485_MDU_UPGRADE
	SnRs485Hdr tMduHdr;
#endif
    SnQByte qTmp, qSize;
    SnBool yRet = TRUE;
    CUtil tUtil;
	
    while (yRet && !feof(m_ptFile) && fread(&qTmp, sizeof(SnQByte), 1, m_ptFile) == 1)
	{
        switch (qTmp) {
        case HDR_WINCE_BASIC:
            tWinCeHdr.qType = qTmp;
            if (fread(&tWinCeHdr.qSplashRamStart, sizeof(SnWinCeHdr) - sizeof(SnQByte), 1, m_ptFile) != 1) {
		        yRet = FALSE;
                break;
	        }
            if (tUtil.CrcMem((SnByte *)&tWinCeHdr, sizeof(SnWinCeHdr)) != 0) {
		        yRet = FALSE;
                break;
            }
            qSize = TOTAL_SIZE_OF_CE_DATA(tWinCeHdr) - sizeof(SnWinCeHdr);
            if (fseek(m_ptFile, qSize, SEEK_CUR) != 0) {
		        yRet = FALSE;
                break;
            }
            m_tUpgSysRev.bMajor = tWinCeHdr.bMajorVers;
            m_tUpgSysRev.bMinor = tWinCeHdr.bMinorVers;
            m_tUpgSysRev.bBuild = tWinCeHdr.bBuildVers;
        	m_tUpgSysRev.bValid = TRUE;
            break;

        case HDR_MOTOR_BASIC:
            tMotorHdr.qType = qTmp;
            if (fread(&tMotorHdr.bAlign, sizeof(SnMotorHdr) - sizeof(SnQByte), 1, m_ptFile) != 1) {
		        yRet = FALSE;
                break;
	        }
            if (tUtil.CrcMem((SnByte *)&tMotorHdr, sizeof(SnMotorHdr)) != 0) {
		        yRet = FALSE;
                break;
            }
            qSize = tMotorHdr.bNumProgramFlashPages * 1024;
            qSize += 8 * 1024;
            if (fseek(m_ptFile, qSize, SEEK_CUR) != 0) {
		        yRet = FALSE;
                break;
            }

            m_tUpgMotorRev.bMajor = tMotorHdr.bMajorVers;
            m_tUpgMotorRev.bMinor = tMotorHdr.bMinorVers;
            m_tUpgMotorRev.bBuild = tMotorHdr.bBuildVers;
        	m_tUpgMotorRev.bValid = TRUE;
            break;

#if RS485_FOOT_UPGRADE
        case HDR_RS485_FOOT:
        case HDR_RS485_FOOT_1:
            tFootHdr.qType = qTmp;
            if (fread(&tFootHdr.wVersion, sizeof(SnRs485Hdr) - sizeof(SnQByte), 1, m_ptFile) != 1) {
		        yRet = FALSE;
                break;
	        }
            if (tUtil.CrcMem((SnByte *)&tFootHdr, sizeof(SnRs485Hdr)) != 0) {
		        yRet = FALSE;
                break;
            }
            qSize = (qTmp == HDR_RS485_FOOT) ? (56 * 128) : (64 * 128);
            if (fseek(m_ptFile, qSize, SEEK_CUR) != 0) {
		        yRet = FALSE;
                break;
            }

            m_tUpgFootRev.bMajor = (tFootHdr.wVersion >> 8) & 3;
            m_tUpgFootRev.bMinor = (tFootHdr.wVersion >> 4) & 0xf;
            m_tUpgFootRev.bBuild = tFootHdr.wVersion & 0xf;
        	m_tUpgFootRev.bValid = TRUE;
            break;
#endif

#if RS485_MDU_UPGRADE
        case HDR_RS485_MDU_SJ:
        case HDR_RS485_MDU_RL:
            tMduHdr.qType = qTmp;
            if (fread(&tMduHdr.wVersion, sizeof(SnRs485Hdr) - sizeof(SnQByte), 1, m_ptFile) != 1) {
		        yRet = FALSE;
                break;
	        }
            if (tUtil.CrcMem((SnByte *)&tMduHdr, sizeof(SnRs485Hdr)) != 0) {
		        yRet = FALSE;
                break;
            }
			qSize = 53 * 128;
            if (fseek(m_ptFile, qSize, SEEK_CUR) != 0) {
		        yRet = FALSE;
                break;
            }

            m_tUpgMduRev.bMajor = (tMduHdr.wVersion >> 8) & 3;
            m_tUpgMduRev.bMinor = (tMduHdr.wVersion >> 4) & 0xf;
            m_tUpgMduRev.bBuild = tMduHdr.wVersion & 0xf;
        	m_tUpgMduRev.bValid = (SnByte)qTmp;
            break;
#endif
        default:
		    yRet = FALSE;
            break;
        }
	}

    if (fseek(m_ptFile, 0L, SEEK_SET) != 0) {
        return FALSE;
    }
    
    return TRUE;
}

SnBool CSoftwareUpgradeDlg::WriteVerifyFlashData(SnQByte qAddr, SnByte *pbData, SnQByte qBytes)
{
    SnBool yLower = (qAddr == LOWER_FLASH_ADDR);
    SnBool yRet;
	CSharedMemory mem;

    if (yLower) {
        StatusMsg(mem.GetString(SN_WRITING_LOWER_FLASH_DATA));
    } else {
        StatusMsg(mem.GetString(SN_WRITING_UPPER_FLASH_DATA));
    }
    yRet = m_pDriver->EraseFlashPages(qAddr, FLASH_BYTES_TO_PAGES(UPPER_FLASH_ADDR-LOWER_FLASH_ADDR));
    if (yRet)
        yRet = m_pDriver->WriteFlashData(qAddr, pbData, qBytes);
    if (yRet) {
        if (yLower) {
            if( m_bDetail)
				StatusMsg(mem.GetString(SN_VERIFYING_LOWER_FLASH_DATA));
        } else {
            if( m_bDetail)
				StatusMsg(mem.GetString(SN_VERIFYING_UPPER_FLASH_DATA));
        }
        yRet = m_pDriver->VerifyFlashData(qAddr, pbData, qBytes);
        if (!yRet) {
            if (yLower) {
				if( m_bDetail)
					StatusMsg(mem.GetString(SN_VERIFYING_LOWER_FLASH_DATA_FAILED));
            } else {
                if( m_bDetail)
					StatusMsg(mem.GetString(SN_VERIFYING_UPPER_FLASH_DATA_FAILED));
            }
        }
    } else {
        if (yLower) {
            if( m_bDetail)
				StatusMsg(mem.GetString(SN_WRITING_LOWER_FLASH_DATA_FAILED));
        } else {
            if( m_bDetail)
				StatusMsg(mem.GetString(SN_WRITING_UPPER_FLASH_DATA_FAILED));
        }
    }

    return yRet;
}

SnBool CSoftwareUpgradeDlg::SoftwareUpgrade(FILE *ptIn)
{
    SnWinCeHdr tWinCeHdr;
    SnMotorHdr tMotorHdr;
#if RS485_FOOT_UPGRADE
	SnRs485Hdr tFootHdr;
#endif
#if RS485_MDU_UPGRADE
	SnRs485Hdr tMduHdr;
#endif
    SnByte g_pbFlashBuf[1024];
    SnByte bStartPage, bEndPage;
    SnQByte qTmp, qSize;
    SnByte *pbData = 0;
    SnQByte qOffs;
    SnByte bPage;
    SnByte bCrc;
    CUtil tUtil;
	SnBool yUpg;
	CSharedMemory mem;

    while (!feof(ptIn) && fread(&qTmp, sizeof(SnQByte), 1, ptIn) == 1) {
        switch (qTmp) {
        case HDR_WINCE_BASIC:
            tWinCeHdr.qType = qTmp;
            if( m_bDetail)
				StatusMsg(mem.GetString(SN_READING_CE_IMAGE_HEADER));
            if (fread(&tWinCeHdr.qSplashRamStart, sizeof(SnWinCeHdr) - sizeof(SnQByte), 1, ptIn) != 1) {
                if( m_bDetail)
					StatusMsg(mem.GetString(SN_READING_CE_HEADER_FAILED));
		        goto SoftwareUpgradeError;
	        }
            if (tUtil.CrcMem((SnByte *)&tWinCeHdr, sizeof(SnWinCeHdr)) != 0) {
                if( m_bDetail)
					StatusMsg(mem.GetString(SN_INVALID_CE_HEADER_CRC));
		        goto SoftwareUpgradeError;
            }
            
			StatusMsg(mem.GetString(SN_READING_CE_IMAGE_DATA));
            qSize = TOTAL_SIZE_OF_CE_DATA(tWinCeHdr);
            pbData = (SnByte *)malloc(qSize);
	        if (pbData == NULL) {
                if( m_bDetail)
					StatusMsg(mem.GetString(SN_ERROR_ALLOCATING_MEM_CE_IMAGE));
		        goto SoftwareUpgradeError;
	        }
            *(SnWinCeHdr *)pbData = tWinCeHdr;
            if (fread(pbData + sizeof(SnWinCeHdr), qSize - sizeof(SnWinCeHdr), 1, ptIn) != 1) {
                if( m_bDetail)
					StatusMsg(mem.GetString(SN_READING_CE_IMAGE_DATA_FAILED));
		        goto SoftwareUpgradeError;
	        }
            qOffs = OFFSET_OF_CE_SPLASH;
            if (tUtil.CrcMem(pbData + qOffs, tWinCeHdr.qSplashRomLen) != tWinCeHdr.bSplashCrc) {
                if( m_bDetail)
					StatusMsg(mem.GetString(SN_INVALID_SPLASH_DATA_CRC));
		        goto SoftwareUpgradeError;
            }
            qOffs = OFFSET_OF_CE_ROM(tWinCeHdr);
            if (tUtil.CrcMem(pbData + qOffs, tWinCeHdr.qNkRomLen) != tWinCeHdr.bNkCrc) {
                if( m_bDetail)
					StatusMsg(mem.GetString(SN_INVALID_CE_DATA_CRC));
		        goto SoftwareUpgradeError;
            }
            if (WriteVerifyFlashData(LOWER_FLASH_ADDR, pbData, qSize) == FALSE) {
                goto SoftwareUpgradeError;
            }
            if (WriteVerifyFlashData(UPPER_FLASH_ADDR, pbData, qSize) == FALSE) {
                goto SoftwareUpgradeError;
            }
            free(pbData);
            pbData = 0;
            break;

        case HDR_MOTOR_BASIC:
			yUpg = m_tUpgMotorRev.bValid && 
							(m_tUpgMotorRev.bMajor != m_tCurMotorRev.bMajor || 
							m_tUpgMotorRev.bMinor != m_tCurMotorRev.bMinor ||
							m_tUpgMotorRev.bBuild != m_tCurMotorRev.bBuild);

            tMotorHdr.qType = qTmp;
			if(yUpg) {
				if( m_bDetail)
					StatusMsg(mem.GetString(SN_READING_MOTOR_CONTROLLER_HEADER));
			} else {
				if( m_bDetail)
					StatusMsg(mem.GetString(SN_SKIPPING_MOTOR_CONTROLLER_HEADER));
			}
            if (fread(&tMotorHdr.bAlign, sizeof(SnMotorHdr) - sizeof(SnQByte), 1, ptIn) != 1) {
                if( m_bDetail)
					StatusMsg(mem.GetString(SN_READING_MOTOR_CONTROLLER_HEADER_FAILED));
		        goto SoftwareUpgradeError;
	        }
            if (tUtil.CrcMem((SnByte *)&tMotorHdr, sizeof(SnMotorHdr)) != 0) {
                if( m_bDetail)
					StatusMsg(mem.GetString(SN_INVALID_MOTOR_CONTROLLER_HEADER_CRC));
		        goto SoftwareUpgradeError;
            }

			if(yUpg) {
				StatusMsg(mem.GetString(SN_UPDATING_MOTOR_CONTROLLER_PROGRAM_FLASH));
				Sleep(75); // hold so text will remain on screen for a few mills
			}
			bCrc = 0;
			for(bPage = 0; bPage < tMotorHdr.bNumProgramFlashPages; bPage++) {
				if(fread(&g_pbFlashBuf[0],1024,1,ptIn) != 1) {
					if( m_bDetail)
						StatusMsg(mem.GetString(SN_READING_MOTOR_CONTROLLER_PROGRAM_FLASH_FAILED));
					goto SoftwareUpgradeError;
				}
				bCrc = tUtil.CrcMemChunk(&g_pbFlashBuf[0],1024,bCrc);
				if(yUpg) {
					if(m_pDriver->FlashBlockToDevice(bPage,(SnWord*)&g_pbFlashBuf[0])
							== FALSE) {
						if( m_bDetail)
							StatusMsg(mem.GetString(SN_WRITING_MOTOR_CONTROLLER_PROGRAM_FLASH_FAILED));
						goto SoftwareUpgradeError;
					}
				}
			}
			if(bCrc != tMotorHdr.bProgramFlashCrc) {
				if( m_bDetail)
					StatusMsg(mem.GetString(SN_INVALED_MOTOR_CONTROLLER_PROGRAM_FLASH_CRC));
				goto SoftwareUpgradeError;
			}

			if(yUpg) {
				if( m_bDetail)
					StatusMsg(mem.GetString(SN_UPDATING_MOTOR_CONTROLLER_DATA_FLASH));
			}
			bCrc = 0;
			for(bPage = 0; bPage < 8; bPage++) {
				if(fread(&g_pbFlashBuf[0],1024,1,ptIn) != 1) {
					if( m_bDetail)
						StatusMsg(mem.GetString(SN_READING_MOTOR_CONTROLLER_DATA_FLASH_FAILED));
					goto SoftwareUpgradeError;
				}
				bCrc = tUtil.CrcMemChunk(&g_pbFlashBuf[0],1024,bCrc);
				if(yUpg) {
					if(m_pDriver->FlashBlockToDevice(bPage+120,(SnWord*)&g_pbFlashBuf[0])
							== FALSE) {
						if( m_bDetail)
							StatusMsg(mem.GetString(SN_WRITING_MOTOR_CONTROLLER_DATA_FLASH_FAILED));
						goto SoftwareUpgradeError;
					}
				}
			}
			if(bCrc != tMotorHdr.bDataFlashCrc) {
				if( m_bDetail)
					StatusMsg(mem.GetString(SN_INVALID_MOTOR_CONTROLLER_DATA_FLASH_CRC));
				goto SoftwareUpgradeError;
			}
            break;

#if RS485_FOOT_UPGRADE
        case HDR_RS485_FOOT:
        case HDR_RS485_FOOT_1:
			yUpg = m_tUpgFootRev.bValid;

            tFootHdr.qType = qTmp;
			if(yUpg) {
				if( m_bDetail)
					StatusMsg(mem.GetString(SN_READING_FOOTSWITCH_HEADER));
			} else {
				if( m_bDetail)
					StatusMsg(mem.GetString(SN_SKIPPING_FOOTSWITCH_HEADER));
			}
            if (fread(&tFootHdr.wVersion, sizeof(SnRs485Hdr) - sizeof(SnQByte), 1, ptIn) != 1) {
                if( m_bDetail)
					StatusMsg(mem.GetString(SN_READING_FOOTSWITCH_HEADER_FAILED));
		        goto SoftwareUpgradeError;
	        }
            if (tUtil.CrcMem((SnByte *)&tFootHdr, sizeof(SnRs485Hdr)) != 0) {
                if( m_bDetail)
					StatusMsg(mem.GetString(SN_INVALID_FOOTSWITCH_HEADER_CRC));
		        goto SoftwareUpgradeError;
            }

			if(yUpg) {
				StatusMsg(mem.GetString(SN_UPDATING_FOOTSWITCH_PROGRAM_FLASH));
                if (qTmp == HDR_RS485_FOOT_1) {
				    SnWord wRequest = FOOT_FRONT_CMD(SERIAL_CMD_REQ_11);
				    SendSerialRequests(1, &wRequest, 4);
                }
			}	

			bCrc = 0;
			*(SnWord*)g_pbFlashBuf = FOOT_FRONT_CMD(SERIAL_CMD_REQ_12);
			g_pbFlashBuf[2] = SERIAL_CMD_REQ_12;
			g_pbFlashBuf[3] = 0;
            if (qTmp == HDR_RS485_FOOT) {
                bStartPage = 56;
                bEndPage = 112;
            } else {
                bStartPage = 0;
                bEndPage = 64;
            }

			for(bPage = bStartPage; bPage < bEndPage; bPage++) {
				g_pbFlashBuf[4] = bPage;
				if(fread(&g_pbFlashBuf[5],128,1,ptIn) != 1) {
					if( m_bDetail)
						StatusMsg(mem.GetString(SN_READING_FOOTSWITCH_PROGRAM_FLASH_FAILED));
					goto SoftwareUpgradeError;
				}
				bCrc = tUtil.CrcMemChunk(&g_pbFlashBuf[5],128,bCrc);
				if (yUpg) {
                    // (16 bit Sci Cmd) + (8 bit Page Cmd) + (16 bit page #) +
                    // (128 bytes of Flash Data) + Crc byte = 134 bytes
                    SnQByte qPageCmdSize = 2 + 1 + 2 + 128 + 1;

					g_pbFlashBuf[128 + 5] = tUtil.CrcMemChunk(&g_pbFlashBuf[2],128 + 3,0);
					if(m_pDriver->SerialPageToDevice(qPageCmdSize, g_pbFlashBuf) == FALSE) {
						if( m_bDetail)
							StatusMsg(mem.GetString(SN_WRITING_FOOTSWITCH_PROGRAM_FLASH_FAILED));
						goto SoftwareUpgradeError;
					}
				}
			}
			if(bCrc != tFootHdr.bProgramFlashCrc) {
				if( m_bDetail)
					StatusMsg(mem.GetString(SN_INVALID_FOOTSWITCH_PROGRAM_FLASH_CRC));
				goto SoftwareUpgradeError;
			}
			if (yUpg) {
				SnWord wRequest = FOOT_FRONT_CMD(SERIAL_CMD_REQ_13);

				StatusMsg(mem.GetString(SN_RESETTING_FOOTSWITCH));
				if (SendSerialRequests(1, &wRequest, 4) == FALSE) {
					StatusMsg(mem.GetString(SN_RESETTING_FOOTSWITCH_FAILED));
					goto SoftwareUpgradeError;
				}
				Sleep(5000);
			}
            break;
#endif
#if RS485_MDU_UPGRADE
        case HDR_RS485_MDU_SJ:
        case HDR_RS485_MDU_RL:
			yUpg = m_tUpgMduRev.bValid;
			
            tMduHdr.qType = qTmp;
			if(yUpg) {
				if( m_bDetail)
					StatusMsg(mem.GetString(SN_READING_MDU_HEADER));
			} else {
				if( m_bDetail) 
					StatusMsg(mem.GetString(SN_SKIPPING_MDU_HEADER));
			}
            if (fread(&tMduHdr.wVersion, sizeof(SnRs485Hdr) - sizeof(SnQByte), 1, ptIn) != 1) {
                if( m_bDetail)
					StatusMsg(mem.GetString(SN_READING_MDU_HEADER_FAILED));
				goto SoftwareUpgradeError;
			}
            if (tUtil.CrcMem((SnByte *)&tMduHdr, sizeof(SnRs485Hdr)) != 0) {
                if( m_bDetail)
					StatusMsg(mem.GetString(SN_INVALID_MDU_HEADER_CRC));
				goto SoftwareUpgradeError;
            }
			
			if(yUpg) {
				SnWord wRequest = HAND_PORTA_CMD(SERIAL_CMD_REQ_11);
				
				StatusMsg(mem.GetString(SN_UPDATING_MDU_PROGRAM_FLASH));
				SendSerialRequests(1, &wRequest, 4);
			}
			
			bCrc = 0;
			*(SnWord*)g_pbFlashBuf = HAND_PORTA_CMD(SERIAL_CMD_REQ_12);
			g_pbFlashBuf[2] = SERIAL_CMD_REQ_12;
			g_pbFlashBuf[3] = 0;

            if (qTmp == HDR_RS485_MDU_SJ) {
                bStartPage = 69;
                bEndPage = 122;
            } else {
                bStartPage = 128;
                bEndPage = 224;
            }
			for(bPage = bStartPage; bPage < bEndPage; bPage++) {
				g_pbFlashBuf[4] = bPage;
				if(fread(&g_pbFlashBuf[5],128,1,ptIn) != 1) {
					if( m_bDetail)
						StatusMsg(mem.GetString(SN_READING_MDU_PROGRAM_FLASH_FAILED));
					goto SoftwareUpgradeError;
				}
				bCrc = tUtil.CrcMemChunk(&g_pbFlashBuf[5],128,bCrc);
				if (yUpg) {
                    // (16 bit Sci Cmd) + (8 bit Page Cmd) + (16 bit page #) +
                    // (128 bytes of Flash Data) + Crc byte = 134 bytes
                    SnQByte qPageCmdSize = 2 + 1 + 2 + 128 + 1;
					
                    g_pbFlashBuf[128 + 5] = tUtil.CrcMemChunk(&g_pbFlashBuf[2],128 + 3,0);
					if(m_pDriver->SerialPageToDevice(qPageCmdSize, g_pbFlashBuf) == FALSE) {
						if( m_bDetail)
							StatusMsg(mem.GetString(SN_WRITING_MDU_PROGRAM_FLASH_FAILED));
						goto SoftwareUpgradeError;
					}
				}
			}
			if(bCrc != tMduHdr.bProgramFlashCrc) {
				if( m_bDetail)
					StatusMsg(mem.GetString(SN_INVALID_MDU_PROGRAM_FLASH_CRC));
				goto SoftwareUpgradeError;
			}
			if (yUpg) {
				SnWord wRequest = HAND_PORTA_CMD(SERIAL_CMD_REQ_13);
				
				StatusMsg(mem.GetString(SN_RESETTING_MDU));
				if (SendSerialRequests(1, &wRequest, 4) == FALSE) {
					StatusMsg(mem.GetString(SN_RESETTING_MDU_FAILED));
					goto SoftwareUpgradeError;
				}
				Sleep(5000);
			}
            break;
#endif
        default:
		    if( m_bDetail)
				StatusMsg(mem.GetString(SN_UNKNOWN_IMAGE_HEADER_TYPE));
		    goto SoftwareUpgradeError;
        }
	}

    return TRUE;

SoftwareUpgradeError:
    if (pbData) {
        free(pbData);
    }

    return FALSE;
}

void CSoftwareUpgradeDlg::UpgradeSoftware(void)
{
    CString csStatus;
	CSharedMemory mem;

    WaitForSingleObject(m_hStartCancelEvent, INFINITE);

    if (m_yCancel == FALSE) {
        if (SoftwareUpgrade(m_ptFile) == FALSE) {
            csStatus = mem.GetString(SN_UPDATE_FAILED);
        } else {
            csStatus = mem.GetString(SN_UPDATE_COMPLETE);
        }
    } else {
        csStatus = m_ySerialNumberCheck ? _T("") : mem.GetString(SN_UPDATE_CANCELLED);
        m_ySerialNumberCheck = FALSE;
    }
    fclose(m_ptFile);

    StatusMsg(csStatus + mem.GetString(SN_PLEASE_REMOVE_USB_DRIVE));

	// Get the Software path and filename
	char pFileName[MAX_PATH]; 
	mem.GetUpgradeFileName( pFileName, sizeof( pFileName));

	for (;;) {
    	m_ptFile = fopen(pFileName, "rb");

		if (m_ptFile == NULL) {
			StatusMsg(csStatus + mem.GetString(SN_POWER_DOWN_SYSTEM));
			break;
		}
		fclose(m_ptFile);
        Sleep(100);
    }
}

DWORD WINAPI UpgradeSoftwareThread(LPVOID pParam)
{
    CSoftwareUpgradeDlg *pClass = (CSoftwareUpgradeDlg *)pParam;

    pClass->UpgradeSoftware();
	Sleep(500);
	PostMessage(pClass->m_hWnd, IDC_BUTTON_DONE, (WPARAM)0, (LPARAM)0);
    return 0;
}

void CSoftwareUpgradeDlg::OnButtonDone()
{
    if (m_ySerialNumberCheck) {
        m_ySerialNumberCheck = FALSE;
        SetupUpgradeDisplay();
    } else {
    	CDialog::OnOK();
    }
}

void CSoftwareUpgradeDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
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
	    case IDC_BUTTON_CANCEL:
				tUtil.DrawTextOnButton(&m_BtnCancel,
										  mem.m_Font20Normal,
										  (m_ySerialNumberCheck ? mem.GetString(SN_DONE) : mem.GetString(SN_CANCEL)));
            break;
	    case IDC_BUTTON_START:	
				tUtil.DrawTextOnButton(&m_BtnStart,
										  mem.m_Font20Normal,
                                          (m_ySerialNumberCheck ? mem.GetString(SN_CHANGE) : mem.GetString(SN_START_LOWER_CASE)));
 			break;
	    case IDC_BUTTON_DONE:	
				tUtil.DrawTextOnButton(&m_BtnDone,
										  mem.m_Font20Normal,
										 (m_ySerialNumberCheck ? mem.GetString(SN_CONTINUE) : mem.GetString(SN_DONE)));
			break;
        }
    }
}

void CSoftwareUpgradeDlg::OnButtonStart() 
{
    if (m_ySerialNumberCheck) {
        CSerialNumberPopUp dlg(m_pDriver);
        dlg.DoModal();
        SetupSerialNumberDisplay();
    } else {
        SetEvent(m_hStartCancelEvent);

        m_BtnStart.ShowWindow(SW_HIDE);
        m_BtnCancel.ShowWindow(SW_HIDE);
    }
}

void CSoftwareUpgradeDlg::OnButtonCancel() 
{
    m_yCancel = TRUE;

    SetEvent(m_hStartCancelEvent);

    m_BtnStart.ShowWindow(SW_HIDE);
    m_BtnCancel.ShowWindow(SW_HIDE);
    m_BtnDone.ShowWindow(SW_HIDE);
}

void CSoftwareUpgradeDlg::StatusMsg(CString csMsg)
{
    SetDlgItemText(IDC_STATIC_UPGRADE_STATUS3, csMsg);
}

