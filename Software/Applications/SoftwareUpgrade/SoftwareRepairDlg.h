//
// SoftwareRepairDlg.h : header file
//

#if !defined(AFX_SOFTWAREREPAIRDLG_H__31C37D27_DAA9_4498_9889_2A2F36703471__INCLUDED_)
#define AFX_SOFTWAREREPAIRDLG_H__31C37D27_DAA9_4498_9889_2A2F36703471__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Resource.h"
#include "ColorFonts.h"
#include "Driver.h"
#include "Util.h"

/////////////////////////////////////////////////////////////////////////////
// CSoftwareRepairDlg dialog

class CSoftwareRepairDlg : public CDialog
{
// Construction
public:
	CSoftwareRepairDlg(CDriver* pDriver, SnBool yLowerFlashValid,
        SnBool yUpperFlashValid, CWnd* pParent = NULL); // Standard constructor
    ~CSoftwareRepairDlg();                              // Standard destructor

// Dialog Data
	//{{AFX_DATA(CSoftwareRepairDlg)
	enum { IDD = IDD_SOFTWAREREPAIR_DIALOG };
	CStatic	m_StaticStatus3;
	CStatic	m_StaticStatus2;
	CStatic	m_StaticStatus1;
	CStatic	m_StaticTitle;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSoftwareRepairDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    HBRUSH m_hBrush;

	// Generated message map functions
	//{{AFX_MSG(CSoftwareRepairDlg)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonDone();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:    // Local Functions
    void SetupFonts(void);
    void SetupButtons(void);
    SnBool GetValidFlashImage(SnQByte qAddr, SnByte **ppbData, SnQByte *pqSize);
    SnBool CopyFlashImage(SnQByte qSrcAddr, SnQByte qDstAddr);
    void RepairFlashImage(void);
    friend DWORD WINAPI RepairFlashThread(LPVOID pParam);
 
private:    // Local Meber Variables
	CDriver*        m_pDriver;		// Pointer to a Driver Object
    SnBool          m_yLowerFlashValid;
    SnBool          m_yUpperFlashValid;
	CBitmapButton	m_BtnDone;		// Handle to Bitmap/Button
	BOOL			m_bDetail;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOFTWAREREPAIRDLG_H__31C37D27_DAA9_4498_9889_2A2F36703471__INCLUDED_)
