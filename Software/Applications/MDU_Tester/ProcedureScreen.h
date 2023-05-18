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

#define SN_BUTTON_PORTA				10
#define SN_BUTTON_PORTB				20

#define PSB_DOWNARROW				1
#define PSB_UPARROW					2
#define PSB_WINDOWLOCK				4
#define PSB_DELTAMODE				5
#define PSB_HANDPIECECOUNT          6

#define SN_BUTTON_DOWNARROW_PORTA       (SN_BUTTON_PORTA + PSB_DOWNARROW)
#define SN_BUTTON_UPARROW_PORTA		    (SN_BUTTON_PORTA + PSB_UPARROW)
#define SN_BUTTON_WINDOWLOCK_PORTA	    (SN_BUTTON_PORTA + PSB_WINDOWLOCK)
#define SN_BUTTON_DELTAMODE_PORTA	    (SN_BUTTON_PORTA + PSB_DELTAMODE)
#define SN_BUTTON_HANDPIECECOUNT_PORTA	(SN_BUTTON_PORTA + PSB_HANDPIECECOUNT)

#define SN_BUTTON_DOWNARROW_PORTB	(SN_BUTTON_PORTB + PSB_DOWNARROW)
#define SN_BUTTON_UPARROW_PORTB		(SN_BUTTON_PORTB + PSB_UPARROW)
#define SN_BUTTON_WINDOWLOCK_PORTB	(SN_BUTTON_PORTB + PSB_WINDOWLOCK)
#define SN_BUTTON_DELTAMODE_PORTB	(SN_BUTTON_PORTB + PSB_DELTAMODE)
#define SN_BUTTON_HANDPIECECOUNT_PORTB	(SN_BUTTON_PORTB + PSB_HANDPIECECOUNT)

#define SN_TEXT_PORTA				100
#define SN_TEXT_PORTB				120

#define PST_ACTIVEWARNING_OFFSET		1


#define SN_TEXT_ACTIVEWARNING_PORTA		(SN_TEXT_PORTA + PST_ACTIVEWARNING_OFFSET)
#define SN_TEXT_ACTIVEWARNING_PORTB		(SN_TEXT_PORTB + PST_ACTIVEWARNING_OFFSET)


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

	void OnButtonDownArrowPressedPortA()	    { m_PortA.OnButtonDownArrowPressed(); }
	void OnButtonUpArrowPressedPortA()		    { m_PortA.OnButtonUpArrowPressed(); }
	void OnButtonWindowLockPressedPortA()	    { m_PortA.OnButtonWindowLockPressed(); }
	void OnButtonDeltaModePortA()			    { m_PortA.OnButtonDeltaMode(); }
	void OnStaticTextActiveWarningPortA()	    { m_PortA.OnStaticTextActiveWarning(); }
    void OnButtonHandpieceCountPressedPortA()   { m_PortA.OnButtonHandpieceCountPressedPortA(); }

	void OnButtonDownArrowPressedPortB()	{ m_PortB.OnButtonDownArrowPressed(); }
	void OnButtonUpArrowPressedPortB()		{ m_PortB.OnButtonUpArrowPressed(); }
	void OnButtonWindowLockPressedPortB()	{ m_PortB.OnButtonWindowLockPressed(); }
	void OnButtonDeltaModePortB()			{ m_PortB.OnButtonDeltaMode(); }
	void OnStaticTextActiveWarningPortB()	{ m_PortB.OnStaticTextActiveWarning(); }
    void OnButtonHandpieceCountPressedPortB()   { m_PortB.OnButtonHandpieceCountPressedPortB(); }
	
	SnBool	GetReDraw()						{ return m_bReDraw; }
	void 	SetReDraw( SnBool bReDraw)		{ m_bReDraw = bReDraw; }
	SnBool	GetDrawRect()					{ return m_bDrawRect; }
	void 	SetDrawRect( SnBool bDrawRect)	{ m_bDrawRect = bDrawRect; }

	
	void Beeper(SnWord usState);
	SN_FOOT_STATUS GetFootStatus();

    SnBool CreateSetSpeedBitmaps(CFont *pFont);

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

    CBitmap         m_ButtonOn;
    CBitmap         m_ButtonOff;

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
    static CBitmap	m_ButtonHandpieceCountPortA;
    static CBitmap	m_ButtonHandpieceCountPortB;
	static CBitmap	m_ArrowLeftGreen;
	static CBitmap	m_ArrowLeftWhite;
	static CBitmap	m_ArrowRightGreen;
	static CBitmap	m_ArrowRightWhite;

    static CBrush	m_hBrGreen;
    static CBrush	m_hBrBlack;
    static CBrush	m_hBrYellow;
	static CBitmap	m_BitmapTestDot;
	static CBitmap	m_BitmapTestDotBlue;
	static CBitmap	m_BitmapTestDotOrange;
	static CBitmap	m_BitmapTestDotYellow;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCEDURESCREEN_H__40CE15E4_E70E_4247_AB09_A61F92E10C27__INCLUDED_)
