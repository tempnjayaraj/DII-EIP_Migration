// ConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestApp.h"
#include "ConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg dialog
CConfigDlg::CConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigDlg::IDD, pParent)
{	
	// Initialize structures
	m_NewConfig.bits = BITS_16;
	m_NewConfig.name = _T("");
	m_NewConfig.offset = 0;
	m_NewConfig.type = TYPE_READ_ONLY;
    m_NewConfig.format = FORMAT_UNSIGNED;

	
	//{{AFX_DATA_INIT(CConfigDlg)
	//}}AFX_DATA_INIT
}

CConfigDlg::~CConfigDlg()
{

}	

void CConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigDlg)
	DDX_Control(pDX, IDC_LIST_FORMAT, m_ListFormat);
	DDX_Control(pDX, IDC_LIST_BITS, m_ListBits);
	DDX_Control(pDX, IDC_LIST_READ_WRITE, m_ListReadWrite);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CConfigDlg)
	ON_EN_SETFOCUS(IDC_EDIT_NAME, OnSetfocusEditName)
	ON_EN_SETFOCUS(IDC_EDIT_OFFSET, OnSetfocusEditOffset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CConfigDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	int status;

	// Setup Screen
	status = m_ListReadWrite.InsertString(0, _T("Read Only")); // index 0
	status  =m_ListReadWrite.InsertString(1, _T("Write Only")); // index 1

	status = m_ListBits.InsertString(0, _T("16 Bit"));      // index 0
	status = m_ListBits.InsertString(1, _T("32 Bit"));      // index 1

	status = m_ListFormat.InsertString(0, _T("Unsigned")); // index 0
	status = m_ListFormat.InsertString(1, _T("Signed"));   // index 1
	status = m_ListFormat.InsertString(2, _T("Hex"));      // index 2
	status = m_ListFormat.InsertString(3, _T("Float"));      // index 3

    SetDlgItemText( IDC_EDIT_NAME, m_CurrentConfig.name);
	SetDlgItemInt( IDC_EDIT_OFFSET, m_CurrentConfig.offset);

	status = m_ListReadWrite.SetCurSel(m_CurrentConfig.type);
	status = m_ListBits.SetCurSel(m_CurrentConfig.bits);
	status = m_ListFormat.SetCurSel(m_CurrentConfig.format);
	
	return TRUE;  // return TRUE unless you set the focus to a control           
}


/////////////////////////////////////////////////////////////////////////////
// CConfigDlg message handlers

SnQByte CConfigDlg::GetDlgItemNum(int iID)
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

void CConfigDlg::OnOK() 
{
	CDialog::OnOK();
	
	// get the selected config
	GetDlgItemText(IDC_EDIT_NAME, m_NewConfig.name);

	m_NewConfig.offset = GetDlgItemNum(IDC_EDIT_OFFSET);

	m_NewConfig.type = m_ListReadWrite.GetCurSel();
	m_NewConfig.bits = m_ListBits.GetCurSel();
	m_NewConfig.format = m_ListFormat.GetCurSel();
    if (m_NewConfig.format == FORMAT_FLOAT) {
        m_NewConfig.bits = BITS_32;
    }
}

void CConfigDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CConfigDlg::GetNewConfig( CONFIG* NewConfig)
{
	NewConfig->bits = m_NewConfig.bits;
	NewConfig->name = m_NewConfig.name;
	NewConfig->offset = m_NewConfig.offset;
	NewConfig->type = m_NewConfig.type;
	NewConfig->format = m_NewConfig.format;
}

void CConfigDlg::SetCurrentConfig( CONFIG* CurrentConfig)
{
	m_CurrentConfig.bits = CurrentConfig->bits;
	m_CurrentConfig.name = CurrentConfig->name;
	m_CurrentConfig.offset = CurrentConfig->offset;
	m_CurrentConfig.type = CurrentConfig->type;
	m_CurrentConfig.format = CurrentConfig->format;
}

void CConfigDlg::OnSetfocusEditName() 
{
	CAlphaKeypadDlg dlg(IDC_EDIT_NAME, this);
	int nResponse = dlg.DoModal();
}

void CConfigDlg::OnSetfocusEditOffset() 
{
	CKeypadDlg dlg(IDC_EDIT_OFFSET, this);
	int nResponse = dlg.DoModal();
}
