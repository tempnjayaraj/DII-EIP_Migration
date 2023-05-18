#if !defined(AFX_SETDATETIMEDLG_H__E5E16E56_2C16_4D2C_B353_2981068295FA__INCLUDED_)
#define AFX_SETDATETIMEDLG_H__E5E16E56_2C16_4D2C_B353_2981068295FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetDateTimeDlg.h : header file
//

#include "SnTypes.h"

/////////////////////////////////////////////////////////////////////////////
// CSetDateTimeDlg dialog

class CSetDateTimeDlg : public CDialog
{
// Construction
public:
	CSetDateTimeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetDateTimeDlg)
	enum { IDD = IDD_DIALOG_DATE_TIME };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetDateTimeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetDateTimeDlg)
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
    void GetNewSystemTime(SYSTEMTIME *ptTime);

private:
    SYSTEMTIME m_tTime;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETDATETIMEDLG_H__E5E16E56_2C16_4D2C_B353_2981068295FA__INCLUDED_)
