#if !defined(AFX_CONFIGDLG_H__22FAE3E0_0859_4552_9DB9_E922E2F8DFDF__INCLUDED_)
#define AFX_CONFIGDLG_H__22FAE3E0_0859_4552_9DB9_E922E2F8DFDF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigDlg.h : header file
//

#include "SnTypes.h"
#include "KeypadDlg.h"
#include "AlphaKeypadDlg.h"

#define TMP_BUF_SIZE			1024

#define TYPE_READ_ONLY	0
#define TYPE_WRITE_ONLY	1
#define BITS_16			0
#define BITS_32			1

#define FORMAT_UNSIGNED 0
#define FORMAT_SIGNED   1
#define FORMAT_HEX      2
#define FORMAT_FLOAT    3

typedef struct {
	CString name;		// Name of Window
	SnQByte bits;			// 16/32 bit Read/Write. 0 = 16 bits, 1 = 32 bits
	SnQByte type;			// Specifies Window type, Read or Write. 0 = Read Only, 1 = Write Only
	SnQByte offset;
    SnQByte format;	
} CONFIG;

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg dialog
class CConfigDlg : public CDialog 
{
// Construction
public:
	
	CConfigDlg(CWnd* pParent = NULL);   // standard constructor
	
	virtual ~CConfigDlg();	// default destructor
	
	// Dialog Data
	//{{AFX_DATA(CConfigDlg)
	enum { IDD = IDD_DIALOG_CONFIG };
	CListBox	m_ListFormat;
	CListBox	m_ListBits;
	CListBox	m_ListReadWrite;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSetfocusEditName();
	afx_msg void OnSetfocusEditOffset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    SnQByte GetDlgItemNum(int iID);
	void GetNewConfig( CONFIG* NewConfig);
	void SetCurrentConfig( CONFIG* CurrentConfig);

private:
    CONFIG m_CurrentConfig;
    CONFIG m_NewConfig;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGDLG_H__22FAE3E0_0859_4552_9DB9_E922E2F8DFDF__INCLUDED_)
