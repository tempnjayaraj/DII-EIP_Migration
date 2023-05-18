// SetDateTimeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestApp.h"
#include "SetDateTimeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetDateTimeDlg dialog


CSetDateTimeDlg::CSetDateTimeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetDateTimeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetDateTimeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSetDateTimeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetDateTimeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetDateTimeDlg, CDialog)
	//{{AFX_MSG_MAP(CSetDateTimeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetDateTimeDlg message handlers

void CSetDateTimeDlg::GetNewSystemTime(SYSTEMTIME *ptTime)
{
    *ptTime = m_tTime;
}

void CSetDateTimeDlg::OnOK() 
{
	CDialog::OnOK();
    SnWord pwBuf[64];
    SnQByte qMonth, qDay, qYear;
    SnQByte qHour, qMinute, qSecond;
    char cHalf;

    // Get the new Date / Time
	GetDlgItemText(IDC_DATETIMEPICKER_DATE, (LPTSTR)pwBuf, 64);
    swscanf((const wchar_t *)pwBuf, TEXT("%d/%d/%d"), &qMonth, &qDay, &qYear);
    m_tTime.wYear = (SnWord)(2000 + qYear);
    m_tTime.wMonth = (SnWord)qMonth;
    m_tTime.wDay = (SnWord)qDay;

    // Let SetSytemTime figure this out later
    m_tTime.wDayOfWeek = 0;
    
	GetDlgItemText(IDC_DATETIMEPICKER_TIME, (LPTSTR)pwBuf, 64);
    swscanf((const wchar_t *)pwBuf, TEXT("%d:%d:%d %cM"), &qHour, &qMinute, &qSecond, &cHalf);
    m_tTime.wHour = (SnWord)((cHalf == 'A') ? qHour : qHour + 12);
    m_tTime.wMinute = (SnWord)qMinute;
    m_tTime.wSecond = (SnWord)qSecond;
    m_tTime.wMilliseconds = 0;

    SetSystemTime(&m_tTime);
    GetSystemTime(&m_tTime);
}

void CSetDateTimeDlg::OnCancel() 
{
	CDialog::OnCancel();
}
