#if !defined(AFX_PROCEDURESCREEN_H__40CE15E4_E70E_4247_AB09_A61F92E10C27__INCLUDED_)
#define AFX_PROCEDURESCREEN_H__40CE15E4_E70E_4247_AB09_A61F92E10C27__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcedureScreen.h : header file
//
#include "SnHelp.h"
#include "SharedMemory.h"
#include "ProcedureRegion.h"
#include "Control.h"
#include "SettingsScreen.h"
#include "SystemErrorDlg.h"

#define BITMAP_GREEN		1
#define BITMAP_WHITE		2
#define BITMAP_FORWARD		3
#define BITMAP_REVERSE		4
#define BITMAP_OSCILLATE	5

#define CTL_BITMAP			1
#define CTL_TEXT			2
#define CTL_BUTTON			3

#define PORT_TYPE_UNKNOWN	0
#define PORT_TYPE_INVALID	1
#define PORT_TYPE_MDU		2
#define PORT_TYPE_TOOL		3

#define SN_BUTTON_DOWNARROW_PORTA	1
#define SN_BUTTON_UPARROW_PORTA		2
#define SN_BUTTON_WINDOWLOCK_PORTA	4
#define SN_BUTTON_DELTAMODE_PORTA	5

#define SN_BUTTON_DOWNARROW_PORTB	6
#define SN_BUTTON_UPARROW_PORTB		7
#define SN_BUTTON_WINDOWLOCK_PORTB	9
#define SN_BUTTON_DELTAMODE_PORTB	10

#define SN_TEXT_SETSPEED_PORTA		100
#define SN_TEXT2_PORTA		101
#define SN_TEXT_ACTIVEWARNING_PORTA		102
#define SN_TEXT4_PORTA		103

#define SN_TEXT_SETSPEED_PORTB		104
#define SN_TEXT2_PORTB		105
#define SN_TEXT_ACTIVEWARNING_PORTB		106
#define SN_TEXT4_PORTB		107
#define SN_BITMAP_DIRECTION_PORTA	108
#define SN_BITMAP_DIRECTION_PORTB	109

// Power Rating scale of 0 to 10
const SnSWord sMaxPowerRating  = 10;
const SnSWord sHalfPowerRating = 5;

/////////////////////////////////////////////////////////////////////////////
// CProcedureScreen dialog

class CProcedureScreen : public CDialog
{
// Construction
public:
	CProcedureScreen(CControl* pControl, CWnd* pParent = NULL);	// standard constructor
	virtual ~CProcedureScreen();								// destructor

// Dialog Data
	//{{AFX_DATA(CProcedureScreen)
	enum { IDD = IDD_PROCEDURE };
	CBitButton m_BtnSettings;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcedureScreen)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProcedureScreen)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnButtonSettings();
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public: // helper functions

	void DrawLines();				// draws all lines and rectangles 
    void SetupTextButtons();        // Load text buttons
    void SetupBitmaps();			// Load bitmaps
	void KillThreads();				// Terminates active threads
	void StartScrollingTimer();		// Starts timer
	void StopScrollingTimer();		// Stops timer
	void StartRemoteTimer();		// Starts timer
	void StopRemoteTimer();			// Stops timer
	void StopScrolling();
	void DeInit();					// Frees all resources
	void UpdateSpeeds();
	void DisplayWarningDetails(DWORD dwPortActiveWarning);

    inline SnBool ButtonPressed(RECT *ptButton, POINT *ptCursor)
    {
        return (ptButton && 
            ptCursor->x >= ptButton->left && ptCursor->x <= ptButton->right &&
            ptCursor->y >= ptButton->top && ptCursor->y <= ptButton->bottom);
    }

	void OnButtonDownArrowPressedPortA()	{ m_PortA.OnButtonDownArrowPressed(); }
	void OnButtonUpArrowPressedPortA()		{ m_PortA.OnButtonUpArrowPressed(); }
	void OnButtonWindowLockPressedPortA()	{ m_PortA.OnButtonWindowLockPressed(); }
	void OnButtonDeltaModePortA()			{ m_PortA.OnButtonDeltaMode(); }
	void OnStaticTextActiveWarningPortA()	{ m_PortA.OnStaticTextActiveWarning(); }

	void OnButtonDownArrowPressedPortB()	{ m_PortB.OnButtonDownArrowPressed(); }
	void OnButtonUpArrowPressedPortB()		{ m_PortB.OnButtonUpArrowPressed(); }
	void OnButtonWindowLockPressedPortB()	{ m_PortB.OnButtonWindowLockPressed(); }
	void OnButtonDeltaModePortB()			{ m_PortB.OnButtonDeltaMode(); }
	void OnStaticTextActiveWarningPortB()	{ m_PortB.OnStaticTextActiveWarning(); }
	
	SnBool	GetReDraw()						{ return m_bReDraw; }
	void 	SetReDraw( SnBool bReDraw)		{ m_bReDraw = bReDraw; }
	SnBool	GetDrawRect()					{ return m_bDrawRect; }
	void 	SetDrawRect( SnBool bDrawRect)	{ m_bDrawRect = bDrawRect; }

	
	void Beeper(SnWord usState);
	SN_FOOT_STATUS GetFootStatus();

    SnBool CreateSetSpeedBitmaps(CFont *pFont50Bold);

	// Message Handlers
	LRESULT HandleErrorConditions(WPARAM iParam, LPARAM lParam);
	LRESULT UpdateStatus(WPARAM iParam, LPARAM lParam);
	LRESULT ExitDialog(WPARAM iParam, LPARAM lParam);
	LRESULT HandleIntellioShaverCmd(WPARAM iParam, LPARAM lParam);
    LRESULT HandleFootCmd( WPARAM iParam, LPARAM lParam);

	// Threads
	friend DWORD WINAPI ButtonScrollThread(LPVOID pParam);

private:
    inline void UpdateIntellioShaverSettingsScreen(SnBool yInSettingsScreen) {if(m_pControl) m_pControl->UpdateIntellioShaverSettingsScreen(yInSettingsScreen);}
    inline void SendIntellioShaverUpdateIfChange() {if(m_pControl) m_pControl->SendIntellioShaverUpdateIfChange();}

	CControl*			m_pControl;				// Pointer to a Control Layer Object
	CWnd*				m_pParent;				// Handle to Dialog window
	CSnHelp				m_SnHelp;				// Smith & Nephew helper class
	CProcedureRegion	m_PortA;				// PortA display region support
	CProcedureRegion	m_PortB;				// PortB display region support
	
	unsigned int	m_uiScrollingTimer;			// Timer ID
	unsigned int	m_uiRemoteTimer;			// Timer ID
	SnBool			m_bKillThreads;				// TRUE = Terminate active threads
	SnBool			m_bRemoteKeyDown;
	SnBool			m_bReDraw;
	SnBool			m_bDrawRect;
	HANDLE			m_hButtonScrollThread;		// Handle to thread 
	DWORD			m_hButtonScrollThreadID;
	HANDLE			m_hButtonScrollThreadKilledEvent; // Handle to event

	DWORD			m_dwBeeperCount;

	SnBool			m_bWaitToScroll;
    SnBool          m_bFatalError;

public:
	static SnSQByte	m_sqCreationCount;
    static CBitmap  m_hIdleSetSpeedDigit[10];
    static CBitmap  m_hRunSetSpeedDigit[10];

    static CBitmap	m_WindowLock;
	static CBitmap	m_WindowLockPressed;
	static CBitmap	m_ArrowUp;					//static CBitmap Object
	static CBitmap	m_ArrowUpPressed;			//static CBitmap Object
	static CBitmap	m_ArrowDown;				//static CBitmap Object
	static CBitmap	m_ArrowDownPressed;			//static CBitmap Object
	static CBitmap	m_ArrowLeftGreen;
	static CBitmap	m_ArrowLeftWhite;
	static CBitmap	m_ArrowRightGreen;
	static CBitmap	m_ArrowRightWhite;

    static CBrush	m_hBrGreen;
    static CBrush	m_hBrBlack;
    static CBrush	m_hBrYellow;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCEDURESCREEN_H__40CE15E4_E70E_4247_AB09_A61F92E10C27__INCLUDED_)
