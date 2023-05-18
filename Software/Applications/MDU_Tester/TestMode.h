#if !defined(AFX_TESTMODE_H__A6317A75_83A5_435B_B731_5D598E98D642__INCLUDED_)
#define AFX_TESTMODE_H__A6317A75_83A5_435B_B731_5D598E98D642__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TestMode.h : header file
//

#define SN_SHORT_WARNING_PORTA		100
#define SN_SHORT_WARNING_PORTB		101

#define CTL_BITMAP			1
#define CTL_TEXT			2
#define CTL_BUTTON			3

// List Index #'s
#define INDEX_ENGLISH		0
#define INDEX_DEUTSCH		1
#define INDEX_ITALIANO		2
#define INDEX_ESPANOL		3
#define INDEX_FRANCAIS		4
#define INDEX_DANSK			5
#define INDEX_NEDERLANDS	6
#define INDEX_NORSK			7
#define INDEX_PORTUGUES		8
#define INDEX_SVENSKA		9

// Warning/Errors
#define INDEX_PW1			0
#define INDEX_PW2			1
#define INDEX_PW3			2
#define INDEX_PW4			3
#define INDEX_PW5			4
#define INDEX_PW6			5
#define INDEX_PW7			6
#define INDEX_PW8			7
#define INDEX_PW9			8
#define INDEX_PW10			9
#define INDEX_PW11			10
#define INDEX_PW12			11
#define INDEX_PW13			12
#define INDEX_PW14			13
#define INDEX_PW15			14
#define INDEX_PW16			15

#define INDEX_PU1			16
#define INDEX_PU2			17
#define INDEX_PU3			18
#define INDEX_PU4			19

#define INDEX_FE2			20
#define INDEX_FE3			21
#define INDEX_FE4			22


#define SN_CLEAR_STRING		255
#define	SN_STRING_INVALID	0xffffffff

/////////////////////////////////////////////////////////////////////////////
// CTestMode dialog
typedef const struct
{
	int iIndex;
	int iIntellioShaverWarning;
	int iStringID;
} WarningTable;

class CTestMode : public CDialog
{
// Construction
public:
	CTestMode(CControl* pControl, CWnd* pParent = NULL);	// standard constructor
	virtual ~CTestMode();									// destructor


// Dialog Data
	//{{AFX_DATA(CTestMode)
	enum { IDD = IDD_TEST_MODE };
	CStatic	m_StaticBox5;
	CStatic	m_StaticBox4;
	CStatic	m_StaticBox3;
	CStatic	m_StaticBox2;
	CStatic	m_StaticBox1;
	CStatic	m_StaticFatalError;
	CListBox	m_ListBox2;
	CListBox	m_ListBox1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestMode)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTestMode)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeList1();
	afx_msg void OnSelchangeList2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


private: 
	
	void DeInit();
	CStatic* CreateStaticControl( CRect tRect, DWORD dwId, DWORD dwCtlType);
	SnBool LoadLanguageDll(int index);
	CString GetString(int resourceID);
	void OnStaticShortWarningPortA();
	void OnStaticShortWarningPortB();
	void DrawShortWarnings();
	void DisplayWarningDetails(int iStringId);
	void ClearShortWarnings();
	LRESULT HandleIntellioShaverCmd(WPARAM iParam, LPARAM lParam);

private:
	CControl*		m_pControl;		// Pointer to the Control Layer Object

	HBRUSH			m_hbrBlack;	
	HBRUSH			m_hbrYellow;
	DWORD			m_dwFatalErrorNum;

	HINSTANCE		m_hResLib;

	CStatic*		m_pStaticShortWarningPortA;
	CStatic*		m_pStaticShortWarningPortB;

	int				m_iCurrentStringIdPortA;
	int				m_iCurrentStringIdPortB;

	DWORD			m_dwIntellioShaverShortWarningNum;

	static WarningTable	ptWarningTable[];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTMODE_H__A6317A75_83A5_435B_B731_5D598E98D642__INCLUDED_)
