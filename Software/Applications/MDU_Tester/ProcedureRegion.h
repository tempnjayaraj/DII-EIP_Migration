#if !defined(AFX_PROCEDUREREGION_H__40CE15E4_E70E_4247_AB09_A61F92E10C27__INCLUDED_)
#define AFX_PROCEDUREREGION_H__40CE15E4_E70E_4247_AB09_A61F92E10C27__INCLUDED_

#include "SlidingWindowFilter.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define SN_SET_SPEED_HEIGHT		42
#define SN_SET_SPEED_WIDTH		20
#define SN_SET_SPEED_HALFWIDTH	10

#define NUM_DEVICE_BUTTONS	3
#define NUM_DEVICE_TACS		3

typedef const struct
{
	DWORD dwWarnings;
	DWORD dwIntellioShaverWarnings;
	DWORD dwStringID;
} SnWarningTable;

typedef struct {
    int iSetSpeedFiveDigitX;
    int iSetSpeedFourDigitX;
    int iSetSpeedThreeDigitX;
    int iSetSpeedTwoDigitX;
    int iSetSpeedOneDigitX;
    int iLastTenThousands;
    int iLastThousands;
    int iLastHundreds;
    int iLastTens;
    int iLastOnes;
    int iLastX;
    SnBool yForceRedraw;
    SnBool yRunning;
} SnSetSpeedDisplay;

class CProcedureScreen;

/////////////////////////////////////////////////////////////////////////////
// CProcedureRegion
class CProcedureRegion
{
public:
	CProcedureRegion(CControl* pControl, CProcedureScreen* pParent = NULL, DWORD port = PORTA);
	virtual ~CProcedureRegion(){;}
	BOOL Init();
	void DeInit(void);
	SnBool KillWindow();
	void KillThread();
	SnBool ConfigurePort();
	SnBool CreateInvalidWindow();
	SnBool CreateToolWindow();
	SnBool CreateMduWindow();
    void RedrawNonStatics(void);
	void DrawArrows();
	void UpdateStatus(int iParam);
	void UpdateDisplay();
	void OnButtonSettings(SnBool bShowWindowLock);
	void UpdateData(unsigned char mode);
	void UpdateMaxSetSpeed();
	void DisplayWarning();
	void SetWarning( DWORD dwNewWarning ) { m_dwWarning |= dwNewWarning;}
	DWORD DwWarning() { return m_dwWarning;}
	void ClearWarning( DWORD dwNewWarning ) { m_dwWarning &= ~dwNewWarning;}
	void HandleIntellioShaverCmd( int iParam, long lParam);
	void OnStaticTextActiveWarning();
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnButtonDownArrowPressed();
	void OnButtonUpArrowPressed();
	void OnButtonWindowLockPressed();
    void OnButtonHandpieceCountPressedPortA();
    void OnButtonHandpieceCountPressedPortB();
	void OnButtonDeltaMode();

	void SetParent(CProcedureScreen *pProcScreen)	{m_pParent = pProcScreen;}
	void GetPortStatus();
	void SetPortStatus();
	CStatic* CreateStaticControl( CRect tRect, DWORD dwId, DWORD dwCtlType);
	CBitButton* CreateBitButtonControl( CRect tRect, DWORD dwId);
    void DrawArrow(int iNum, CBitmap *pBitmap);
	void SetDirectionControl();
	void HideButtons( void);
	int  GetMinMaxInc(SnWord* pwMin, SnWord* pwMax, SnWord* pwIncrement);

	static DWORD WINAPI DisplayThread(LPVOID pParam);
	void	DirectionReverse( SnBool& startRunning);
	void	DirectionForward( SnBool& startRunning);
	void	DirectionOscillate( SnBool& startRunning);
	void	DirectionWindowlock( SnBool& startRunning);
	SnSWord GetPowerRating();

	void DrawArrow(void);
    void InitUpArrowRect(void);
    void DrawUpArrow(SnBool yDisplay);
    void InitDownArrowRect(void);
    void DrawDownArrow(SnBool yDisplay);
	void InitWindowLockRect(void);
	void DrawWindowLockButton(void);
    void InitHandpieceCountButtonPortARect(void);
    void DrawHandpieceCountButtonPortA(SnBool yDisplay);
    void InitHandpieceCountButtonPortBRect(void);
    void DrawHandpieceCountButtonPortB(SnBool yDisplay);
	void DrawConnector(SnBool yDisplay);
    void DrawTextMode(SnBool yDisplay);
    void DrawTextMinSpeed(SnBool yDisplay, SnBool ySmallFont);
    void DrawTextMaxSpeed(SnBool yDisplay, SnBool ySmallFont);
    void DrawSetSpeed(SnBool yDisplay);
    void InitSetSpeedDisplay(void);
    void DrawDigit(int iX, int iDigit, SnBool yRunning);
    void UpdateSetSpeedDisplay(void);
	void SetNumDeviceButtons();
	void SetDeviceLabel();
	void SetHandPieceStatus(SnBool bRefresh=FALSE);

	void DrawDeviceLabel(SnBool yDisplay);
	void DrawTacStatus(SnBool bRefresh=FALSE);
	void DrawMduDot(int iNum, CBitmap *pBitmap);
	void DrawTacDot(int iNum, CBitmap *pBitmap);
	void DrawMduButtons(SnBool bRefresh=FALSE);
	void UpdateActualSpeed(SnBool yDisplay);
	void DrawActualSpeed(SnBool yDisplay);
	void UpdateMotorStatus(SnBool bRefresh=FALSE);

	void UpdateMotorCurrent(SnBool yDisplay);
	void DrawMotorCurrent(SnBool yDisplay);
	void UpdateBladeId(SnBool yDisplay);
	void DrawBladeId(SnBool yDisplay);

	SnBool PortRunning(){ return m_tPortStatus.bRunning;}

	BOOL ArrowScrollScaler(void){return (--m_iArrowScrollScaler < 1);}
	
    inline void NoIntellioShaverHandpiecePresent() {if(m_pControl) m_pControl->NoIntellioShaverHandpiecePresent(m_dwPort);}
    inline void UpdateIntellioShaverPortUnitsAndMode(SnByte bPortUnits, SnByte bPortMode) {if(m_pControl) m_pControl->UpdateIntellioShaverPortUnitsAndMode(m_dwPort, bPortUnits, bPortMode);}
    inline void UpdateIntellioShaverPortBlade(SnWord wBladeId) {if(m_pControl) m_pControl->UpdateIntellioShaverPortBlade(m_dwPort, wBladeId);}
    inline void UpdateIntellioShaverPortArrows(SnBool yUpArrow, SnBool yDownArrow) {if(m_pControl) m_pControl->UpdateIntellioShaverPortArrows(m_dwPort, yUpArrow, yDownArrow);}
    inline void UpdateIntellioShaverPortSetSpeed(SnByte bSetSpeed) {if(m_pControl) m_pControl->UpdateIntellioShaverPortSetSpeed(m_dwPort, bSetSpeed);}
    inline void UpdateIntellioShaverPortRunState(SnBool yRunning) {if(m_pControl) m_pControl->UpdateIntellioShaverPortRunState(m_dwPort, yRunning);}
    inline void UpdateIntellioShaverPortErrWarn(SnByte bErrWarn) {if(m_pControl) m_pControl->UpdateIntellioShaverPortErrWarn(m_dwPort, bErrWarn);}
    inline void SendIntellioShaverUpdateIfChange() {if(m_pControl) m_pControl->SendIntellioShaverUpdateIfChange();}
 
    void SetupTextButtons();

public:
    RECT*           m_pUpArrowRect;
    RECT*           m_pDownArrowRect;
    RECT*           m_pWindowLockRect;
    RECT*           m_pHandpieceCountButtonPortARect;
    RECT*           m_pHandpieceCountButtonPortBRect;

    CStatic*		m_pButtonWindowLock;
	CBitButton*	    m_pButtonDeltaMode;

	SnBool			m_bDownArrowPressed;	
	SnBool			m_bUpArrowPressed;
	SnBool			m_bWindowLockPressed;

	SnBool			m_bDownArrowHidden;	
	SnBool			m_bUpArrowHidden;
	INT				m_iArrowScrollScaler;

private:

	CControl*		m_pControl;					// Pointer to a Control Layer Object
	CProcedureScreen* m_pParent;
	DWORD			m_dwPort;
	DWORD			m_dwPortType;				//
	DWORD			m_dwMode;
	SnBool			m_dwModeChange;
	SN_PORT_STATUS	m_tPortStatus;
	DWORD			m_dwWarning;
	DWORD			m_dwActiveWarning;
	SnBool			m_bKillThreads;				// TRUE = Terminate active threads
	HANDLE			m_hDisplayThread;			// Handle to thread
	DWORD			m_hDisplayThreadID;
	HANDLE			m_hDisplayThreadKilledEvent; // Handle to event
	CSnHelp			m_SnHelp;					// Smith & Nephew helper class

	int				m_DisplayOffsetX;	// X Offset for the display region 

	int				m_SnTextActiveWarning;
	int				m_SnButtonDeltaMode;
	int				m_SnButtonWindowLock;

	int				m_SnGetMcPortStatus;
	int				m_SnSetMcPortStatus;
	int				m_SnSetMcWindowLock;
    int             m_SnConnectorImageId;

    RECT            m_tUpArrowRect;
    RECT            m_tDownArrowRect;
    RECT            m_tWindowLockRect;
    RECT            m_tHandpieceCountButtonPortARect;
    RECT            m_tHandpieceCountButtonPortBRect;

    CBitmap			m_BitmapConnector;
    CString         m_csTextMode;
    CString         m_csTextMaxSpeed;
	CString         m_csTextMinSpeed;
	CString			m_csTextTargetSpeed;
	CString			m_csTextActualSpeed;
	CString			m_csTextActualRpm;
	CString			m_csTextMotorCurrent;
	CString			m_csTextBladeId;
	CString			m_csDeviceType;
    CString         m_csTextWindowLockValue;

	CStatic*		m_pStaticTextActiveWarning;

	SnBool			m_bDualRpmOn;
	SnBool			m_bWindowLockEnabled;
	SnBool			m_bActiveWarningPressed;
	SnBool			m_bDisplayArrows;

	SnWord			m_wSetSpeed;
	SnWord			m_wDirection[10];

	SnWord			m_wNumButtons;
	SnWord			m_wDeviceButtons[NUM_DEVICE_BUTTONS];
	SnWord			m_wMotorTac[NUM_DEVICE_TACS];
	SnSQByte        m_lOldVelocity;
	SnSQByte        m_lOldCurrent;
    SnSQByte        m_lOldWindowLockValue;
    SnSQByte        m_lOldBladeId;

    SnSetSpeedDisplay m_SetSpeedDisplay; 
    
    SnWord          m_wType;

    SnBool          m_yFirstPaintOccured;

	static SnWarningTable ptWarningTable[];

};
//
/////////////////////////////////////////////////////////////////////////////


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCEDUREREGION_H__40CE15E4_E70E_4247_AB09_A61F92E10C27__INCLUDED_)
