// ProcedureScreen.cpp : implementation file
//

#include "stdafx.h"
#include "Shaver.h"
#include "ProcedureScreen.h"
#include "MessageBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProcedureRegion

CProcedureRegion::CProcedureRegion(CControl* pControl, CProcedureScreen* pParent, DWORD port):
	m_dwPort(port),
	m_dwPortType(PORT_TYPE_UNKNOWN),
	m_pControl(pControl),
	m_pParent(pParent),

	m_dwWarning(0),
	m_dwActiveWarning(0),
	m_bDualRpmOn(FALSE),
	m_bDisplayArrows(FALSE),
	m_dwMode(MOTOR_OSCILLATE_1),
	m_dwModeChange(TRUE),
	m_wSetSpeed(0),

	m_pDownArrowRect(NULL),
	m_pUpArrowRect(NULL),
	m_pWindowLockRect(NULL),

    m_pButtonWindowLock(NULL),
	m_pButtonDeltaMode(NULL),

	m_bDownArrowPressed(FALSE),
	m_bUpArrowPressed(FALSE),
	m_bWindowLockPressed(FALSE),

	m_bDownArrowHidden(TRUE),	
    m_bUpArrowHidden(TRUE),
	m_iArrowScrollScaler(1),

    m_csTextMode(""),
    m_csTextMaxSpeed(""),
    m_pStaticTextActiveWarning(NULL),

	m_bActiveWarningPressed(FALSE),

	m_bKillThreads(FALSE),
	m_hDisplayThread(NULL),
	m_hDisplayThreadKilledEvent(NULL),
    m_wType(TYPE_ERROR),
    m_yFirstPaintOccured(FALSE)
{ 
	int ii;
	for( ii = 0; ii < sMaxPowerRating; ii ++)
	{
		m_wDirection[ii] = NULL;	
	}

	if (m_dwPort == PORTA)
	{
		m_DisplayOffsetX = 0;

		m_SnTextActiveWarning = SN_TEXT_ACTIVEWARNING_PORTA;
		m_SnButtonDeltaMode = SN_BUTTON_DELTAMODE_PORTA;
		m_SnButtonWindowLock = SN_BUTTON_WINDOWLOCK_PORTA;

		m_SnGetMcPortStatus = GET_MC_PORTA_STATUS;
		m_SnSetMcPortStatus = SET_MC_PORTA_STATUS;
		m_SnSetMcWindowLock = SET_MC_WINDOW_LOCK_PORTA;
        m_SnConnectorImageId = IDB_BITMAP_CONNECTOR_PORTA;
	}
	else
	{
		m_DisplayOffsetX = 400;

		m_SnTextActiveWarning = SN_TEXT_ACTIVEWARNING_PORTB;
		m_SnButtonDeltaMode = SN_BUTTON_DELTAMODE_PORTB;
		m_SnButtonWindowLock = SN_BUTTON_WINDOWLOCK_PORTB;

		m_SnGetMcPortStatus = GET_MC_PORTB_STATUS;
		m_SnSetMcPortStatus = SET_MC_PORTB_STATUS;
		m_SnSetMcWindowLock = SET_MC_WINDOW_LOCK_PORTB;
        m_SnConnectorImageId = IDB_BITMAP_CONNECTOR_PORTB;
	}
}

BOOL CProcedureRegion::Init()
{
	// Get Port status from the control layer
	GetPortStatus();

    // Setup Set Speed Display
    InitSetSpeedDisplay();

    InitUpArrowRect();
    InitDownArrowRect();
    InitWindowLockRect();

    m_SnHelp.LoadBitmap(&m_BitmapConnector,m_SnConnectorImageId);

    // Initialize display to Invalid Window
	CreateInvalidWindow();

	m_hDisplayThreadKilledEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if(m_hDisplayThreadKilledEvent == NULL)
	{
		return FALSE;
	}

	//Create thread that handles displaying port A running indicators
	m_hDisplayThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
					 0,
					 DisplayThread,
					 this,
					 0,
					 &m_hDisplayThreadID);
	
	if (m_hDisplayThread == NULL)
	{
 		return FALSE;
	}
	DEBUGMSG(TRUE, (TEXT("Region %d DisplayThread: 0x%08X\n"),m_dwPort,m_hDisplayThreadID));

	// Get Port status from the control layer in case
	// it changed in the background
	// Check for a device that might have connected before the display thread
	// was created
	GetPortStatus();
	if (m_tPortStatus.usPrevType != TYPE_INVALID)
		PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
               (WPARAM)MSG_UPDATE_PORT_STATUS, (LPARAM)m_dwPort);
    return TRUE;
}

void CProcedureRegion::DeInit(void)
{
	// Terminate all running threads
	m_bKillThreads = TRUE;

	// Close Event Handles
	if( m_hDisplayThreadKilledEvent)
	{
		CloseHandle(m_hDisplayThreadKilledEvent);
		m_hDisplayThreadKilledEvent = NULL;
	}

	// Close Thread Handles

	if( m_hDisplayThread)
	{
		CloseHandle(m_hDisplayThread);
		m_hDisplayThread = NULL;
	}

	m_dwPortType = PORT_TYPE_UNKNOWN;

	// Close Window
	KillWindow();
}

void CProcedureRegion::KillThread()
{
	DWORD waitStatus;
	DWORD exitCode;

	// Terminate all running threads
	m_bKillThreads = TRUE;

	if( m_hDisplayThread)
	{
		// wait for the thread to terminate, time out after a certain period of time
		if( m_hDisplayThreadKilledEvent)
			waitStatus = WaitForSingleObject( m_hDisplayThreadKilledEvent,THREAD_TERMINATION_WAIT );

		if( waitStatus == WAIT_TIMEOUT) 
		{
			GetExitCodeThread( m_hDisplayThread, &exitCode);
		
			TerminateThread( m_hDisplayThread, exitCode);
	
		}
		
		// Close the thread handle
		CloseHandle( m_hDisplayThread);
		m_hDisplayThread = NULL;
	}
}

SnBool CProcedureRegion::KillWindow()
{
    m_pUpArrowRect = NULL;
    m_pDownArrowRect= NULL;
    m_pWindowLockRect= NULL;

    if( m_pStaticTextActiveWarning)
	{
		delete( m_pStaticTextActiveWarning);
		m_pStaticTextActiveWarning = NULL;
	}
	
	if( m_pButtonWindowLock)
	{
		// Disable Window Lock
		if( m_pControl && m_bWindowLockPressed)
		{
			SnWord usTemp = WINDOW_LOCK_OFF;
			// Save the settings
			m_pControl->SetCmdState(m_SnSetMcWindowLock, &usTemp, sizeof(usTemp));
		}
		m_bWindowLockPressed = FALSE;

		delete( m_pButtonWindowLock);
		m_pButtonWindowLock = NULL;
	}
	
	if( m_pButtonDeltaMode)
	{
		delete( m_pButtonDeltaMode);
		m_pButtonDeltaMode = NULL;
	}
	
	switch( m_dwPortType)
	{
	case PORT_TYPE_TOOL:
        DrawSetSpeed(FALSE);
        DrawTextMode(FALSE);
        DrawUpArrow(FALSE);
        DrawDownArrow(FALSE);
        m_bUpArrowHidden = TRUE;
        m_bDownArrowHidden = TRUE;
        break;
	case PORT_TYPE_MDU:
        DrawSetSpeed(FALSE);
        DrawTextMode(FALSE);
        DrawTextMaxSpeed(FALSE, FALSE);
        DrawUpArrow(FALSE);
        DrawDownArrow(FALSE);
        m_bUpArrowHidden = TRUE;
        m_bDownArrowHidden = TRUE;
		break;	
		
	case PORT_TYPE_INVALID:
        DrawConnector(FALSE);
		break;
    }

	return TRUE;
}

void CProcedureRegion::RedrawNonStatics(void)
{
    m_yFirstPaintOccured = TRUE;

    switch( m_dwPortType)
	{
	case PORT_TYPE_TOOL:
        DrawSetSpeed(TRUE);
        DrawTextMode(TRUE);
        if (!m_bUpArrowHidden)
            DrawUpArrow(TRUE);
        if (!m_bDownArrowHidden)
            DrawDownArrow(TRUE);
        break;
	case PORT_TYPE_MDU:
        DrawSetSpeed(TRUE);
        DrawTextMode(TRUE);
        UpdateMaxSetSpeed();
        if (!m_bUpArrowHidden)
            DrawUpArrow(TRUE);
        if (!m_bDownArrowHidden)
            DrawDownArrow(TRUE);
		break;	
		
	case PORT_TYPE_INVALID:
        DrawConnector(TRUE);
		break;

	default:
		break;
	}
}

void CProcedureRegion::UpdateMaxSetSpeed(void)
{
    if (m_dwPortType == PORT_TYPE_MDU)
    {
	    SnBool ySmallFont = FALSE;

	    if( m_dwMode == MOTOR_FORWARD)
		    m_csTextMaxSpeed.Format(_T("%d"),m_tPortStatus.usForwardMax);
	    else if( m_dwMode == MOTOR_REVERSE)
		    m_csTextMaxSpeed.Format(_T("%d"),m_tPortStatus.usReverseMax);
	    else if( m_dwMode == MOTOR_OSCILLATE_1)
	    {
		    SnWord usLanguage;
		    
		    // Get the current language
		    if( m_pControl) 
		    {
			    // Get the language selection
			    SnBool bStatus = m_pControl->GetCmdState(GET_SYSTEM_LANGUAGE, &usLanguage, sizeof(usLanguage));
			    if(!bStatus)
				    usLanguage = LANGUAGE_ENGLISH;
		    }
		    else
			    usLanguage = LANGUAGE_ENGLISH;		
			    
		    if( usLanguage == LANGUAGE_SWEDISH)
                ySmallFont = TRUE;

    	    m_SnHelp.GetString(SN_RPM, m_csTextMaxSpeed);
	    }
	    else if( m_dwMode == MOTOR_OSCILLATE_2)
		     m_SnHelp.GetString(SN_RATE, m_csTextMaxSpeed);

        DrawTextMaxSpeed(TRUE, ySmallFont);
    }
}

int CProcedureRegion::GetMinMaxInc(SnWord* pwMin, SnWord* pwMax, SnWord* pwIncrement)
{
	int ii;
	
	if(m_dwPortType == PORT_TYPE_MDU)
	{
        switch( m_dwMode)
		{
		case MOTOR_REVERSE:
			*pwMax = m_tPortStatus.usReverseMax;
			*pwMin = m_tPortStatus.usReverseMin;
			*pwIncrement = m_tPortStatus.usReverseIncrement;
			if(m_bDualRpmOn)
				ii = m_tPortStatus.usReverse2;
			else
				ii = m_tPortStatus.usReverse;
			break;
		case MOTOR_FORWARD:
			*pwMax = m_tPortStatus.usForwardMax;
			*pwMin = m_tPortStatus.usForwardMin;
			*pwIncrement = m_tPortStatus.usForwardIncrement;
			if(m_bDualRpmOn)
				ii = m_tPortStatus.usForward2;
			else
				ii = m_tPortStatus.usForward;
			break;
		case MOTOR_OSCILLATE_2:
			*pwMax = m_tPortStatus.wOscillateSecondsMax;
			*pwMin = m_tPortStatus.wOscillateSecondsMin;
			*pwIncrement = m_tPortStatus.wOscillateSecondsIncrement;
 			ii = m_tPortStatus.wOscillateSeconds;
			break;
		case MOTOR_OSCILLATE_1:
		default:
			*pwMax = m_tPortStatus.usOscillateRpmMax;
			*pwMin = m_tPortStatus.usOscillateRpmMin;
			*pwIncrement = m_tPortStatus.usOscillateRpmIncrement;
			ii = m_tPortStatus.usOscillateRpm;
			break;
		}

	}
	else if(m_dwPortType == PORT_TYPE_TOOL)
	{
		*pwMax = m_tPortStatus.usPercentMax;
		*pwMin = m_tPortStatus.usPercentMin;
		*pwIncrement = m_tPortStatus.sPercentIncrement;
		ii = m_tPortStatus.usPercent;

	}
	if (ii > *pwMax)
	{
		ii = *pwMax;
	}
	else if (ii < *pwMin)
	{
		ii = *pwMin;
	}	
	return ii;
}

CStatic* CProcedureRegion::CreateStaticControl( CRect tRect, DWORD dwId, DWORD dwCtlType)
{
	CStatic* pStatic = NULL;
	SnBool bStatus;
	DWORD dwStyle;

	if( dwCtlType == CTL_TEXT)
		dwStyle = WS_VISIBLE | SS_CENTER | SS_NOTIFY;
	else if( dwCtlType == CTL_BITMAP)
		dwStyle = WS_VISIBLE | SS_BITMAP;
	else if( dwCtlType == CTL_BUTTON)
		dwStyle = WS_VISIBLE | SS_BITMAP | SS_NOTIFY ;
	else
		return NULL;

	//
	// Create Static window control
	//
	pStatic = new CStatic;
	if(!pStatic)
		return pStatic; // Failed to allocate resourses
		
		
	bStatus = pStatic->Create( NULL,		// Window Name 
							   dwStyle,		// Window Style
							   tRect,		// Window size and location
							   m_pParent,		// Parent Window
					           dwId);		// Window ID
	
	if(!bStatus)
	{
		// Failed to create window control
		if( pStatic)
		{
			delete pStatic;
			pStatic = NULL;
		}
	}

	return pStatic;
}

CBitButton* CProcedureRegion::CreateBitButtonControl( CRect tRect, DWORD dwId)
{
	CBitButton* pBitButton = NULL;
	SnBool bStatus;
	DWORD dwStyle;


	dwStyle = WS_VISIBLE | BS_OWNERDRAW | WS_TABSTOP;

	//
	// Create button window control
	//
	pBitButton = new CBitButton;
	if(!pBitButton)
		return pBitButton; // Failed to allocate resourses
		
		
	bStatus = pBitButton->Create( NULL,		// Window Name 
									 dwStyle,		// Window Style
							         tRect,		// Window size and location
							         m_pParent,		// Parent Window
					                 dwId);		// Window ID
	
	if(!bStatus)
	{
		// Failed to create window control
		if( pBitButton)
		{
			delete pBitButton;
			pBitButton = NULL;
		}
	}

	return pBitButton;
}

void CProcedureRegion::DrawArrows()
{
	int ii;
	DWORD dwDirection;
	
    dwDirection = m_dwMode;
	if(m_pParent->GetDrawRect())
	{
		if(m_bDisplayArrows)
		{
			switch( dwDirection)
			{
			case MOTOR_FORWARD:
				for( ii = (sMaxPowerRating-1); ii >= 0; ii--)
				{
					DrawArrow(ii, &CProcedureScreen::m_ArrowRightWhite);
					m_wDirection[ii] = IDB_BITMAP_ARROW_RIGHT;
				}
				break;
				
			case MOTOR_REVERSE:
				for( ii = 0; ii < sMaxPowerRating; ii++)
				{
					DrawArrow(ii, &CProcedureScreen::m_ArrowLeftWhite);
					m_wDirection[ii] = IDB_BITMAP_ARROW_LEFT;
				}
				break;
				
				
			case MOTOR_OSCILLATE_1:
			case MOTOR_OSCILLATE_2:
				for( ii = 0; ii < sHalfPowerRating; ii++)
				{
					DrawArrow(ii, &CProcedureScreen::m_ArrowLeftWhite);
					m_wDirection[ii] = IDB_BITMAP_ARROW_LEFT;			
				}
				
				for( ii = sHalfPowerRating; ii < sMaxPowerRating; ii++)
				{
					DrawArrow(ii, &CProcedureScreen::m_ArrowRightWhite);
					m_wDirection[ii] = IDB_BITMAP_ARROW_RIGHT;		
				}
				break;
			}
		}
		else
		{

			//
			// Erase the arrows
			//

			for( ii = 0; ii < sMaxPowerRating; ii++)
				DrawArrow(ii, NULL);
		}
	}
}

void CProcedureRegion::SetDirectionControl()
{
	switch( m_dwPortType)
	{
	case PORT_TYPE_TOOL:
	case PORT_TYPE_MDU:
	    DrawArrows();
        if (m_wSetSpeed)
            UpdateSetSpeedDisplay();
		break;	
		
	case PORT_TYPE_INVALID:
		break;
    }
	
	m_dwModeChange = TRUE;
}

void CProcedureRegion::HideButtons( void) 
{
	SnWord wMax,wMin,wIncrement;
    SnBool yUpArrowHidden = FALSE;
    SnBool yDownArrowHidden = FALSE;
	SnWord w;
	int iOscillateSeconds = 0;

	// Get Minimum, Maximum and Increment values
	iOscillateSeconds = GetMinMaxInc(&wMin, &wMax, &wIncrement);	
	
	if( m_dwMode == MOTOR_OSCILLATE_2)
	{	
		w = m_tPortStatus.wOscillateSeconds;
		if(w == wMax)
		{
            yDownArrowHidden = TRUE;
		}
		else if( w == wMin)
		{
            yUpArrowHidden = TRUE;
		}
	}
	else
	{
		w = m_wSetSpeed;
		if(w == wMin)
		{
            yDownArrowHidden = TRUE;
		}
		else if( w == wMax)
		{
            yUpArrowHidden = TRUE;
		}
	}

    UpdateIntellioShaverPortArrows(!yUpArrowHidden, !yDownArrowHidden);

    if (m_pDownArrowRect && yDownArrowHidden != m_bDownArrowHidden)
    {
        m_bDownArrowHidden = yDownArrowHidden;
        DrawDownArrow(TRUE);
    }
    if (m_pUpArrowRect && yUpArrowHidden != m_bUpArrowHidden)
    {
        m_bUpArrowHidden = yUpArrowHidden;
        DrawUpArrow(TRUE);
    }
}

void CProcedureRegion::UpdateDisplay()
{	
	SnWord usRpm = 0;
	SnWord usSeconds = 0;
	SnWord usMax,usMin,usIncrement;

	if(m_dwPortType == PORT_TYPE_MDU)
	{
		if( m_dwMode == MOTOR_FORWARD)
		{
			if(m_bDualRpmOn)
			{
                if (m_wSetSpeed != m_tPortStatus.usForward2) {
                    m_wSetSpeed = m_tPortStatus.usForward2;
                    UpdateSetSpeedDisplay();
                }
				usRpm = m_wSetSpeed;
			}
			else
			{
                if (m_wSetSpeed != m_tPortStatus.usForward) {
                    m_wSetSpeed = m_tPortStatus.usForward;
                    UpdateSetSpeedDisplay();
                }
				usRpm = m_wSetSpeed;
			}
			
			if( m_csTextMode != m_SnHelp.GetString(SN_RPM))
            {
                m_SnHelp.GetString(SN_RPM, m_csTextMode);
				DrawTextMode(TRUE);
            }
		}
		else if( m_dwMode == MOTOR_REVERSE)
		{
			if(m_bDualRpmOn)
			{
                if (m_wSetSpeed != m_tPortStatus.usReverse2) {
                    m_wSetSpeed = m_tPortStatus.usReverse2;
                    UpdateSetSpeedDisplay();
                }
				usRpm = m_wSetSpeed;
			}
			else
			{
                if (m_wSetSpeed != m_tPortStatus.usReverse) {
                    m_wSetSpeed = m_tPortStatus.usReverse;
                    UpdateSetSpeedDisplay();
                }
				usRpm = m_wSetSpeed;
			}
			
			if( m_csTextMode != m_SnHelp.GetString(SN_RPM))
            {
                m_SnHelp.GetString(SN_RPM, m_csTextMode);
				DrawTextMode(TRUE);
            }
		}
		else if( m_dwMode == MOTOR_OSCILLATE_1)
		{
            if (m_wSetSpeed != m_tPortStatus.usOscillateRpm) {
                m_wSetSpeed = m_tPortStatus.usOscillateRpm;
                UpdateSetSpeedDisplay();
            }
			usRpm = m_wSetSpeed;
			
			if( m_csTextMode != m_SnHelp.GetString(SN_MODE1))
            {
                m_SnHelp.GetString(SN_MODE1, m_csTextMode);
				DrawTextMode(TRUE);
            }
		}
		else if( m_dwMode == MOTOR_OSCILLATE_2)
		{
			// Get Minimum, Maximum and Increment values
			GetMinMaxInc(&usMin, &usMax, &usIncrement);
			
			usSeconds = m_tPortStatus.wOscillateSeconds;
			
			SnWord wSetSpeed = 0;
			if (usIncrement)
				wSetSpeed = ((usMax - usSeconds) / usIncrement) + 1;
			else
				wSetSpeed = m_wSetSpeed;
 
			if (m_wSetSpeed != wSetSpeed) {
                m_wSetSpeed = wSetSpeed;
                UpdateSetSpeedDisplay();
            }
			
			if( m_csTextMode != m_SnHelp.GetString(SN_MODE2))
            {
                m_SnHelp.GetString(SN_MODE2, m_csTextMode);
				DrawTextMode(TRUE);
            }
		}
	}
	else if(m_dwPortType == PORT_TYPE_TOOL)
	{
        if (m_wSetSpeed != m_tPortStatus.usPercent) {
            m_wSetSpeed = m_tPortStatus.usPercent;
            UpdateSetSpeedDisplay();
        }
		if( m_csTextMode != m_SnHelp.GetString(SN_SPEED))
        {
            m_SnHelp.GetString(SN_SPEED, m_csTextMode);
			DrawTextMode(TRUE);
        }
	}
	
    SnByte bSetSpeed;

	if(m_dwPortType == PORT_TYPE_MDU)
	{
		if( m_dwMode == MOTOR_OSCILLATE_2)
		{
			bSetSpeed = (SnByte)m_wSetSpeed;
		}
		else
		{
            bSetSpeed = (SnByte)(usRpm / 100);
		}
	}
	else if(m_dwPortType == PORT_TYPE_TOOL)
	{
        bSetSpeed = (SnByte)m_tPortStatus.usPercent;
	}

    UpdateIntellioShaverPortSetSpeed(bSetSpeed);
 	
	HideButtons();
}

void CProcedureRegion::OnButtonSettings(SnBool bShowWindowLock) 
{
	// Get PortB status from the control layer
	GetPortStatus();

	// Check to see if the window lock button should be displayed
	if( m_pButtonWindowLock)
	{
		if( bShowWindowLock)
		{
			m_pButtonWindowLock->ShowWindow(SW_SHOW);	// Display WindowLock Button
		}
		else
		{
			m_pButtonWindowLock->ShowWindow(SW_HIDE);	// Hide WindowLock Button
		}
	}
	// Get the port Status and update the screen
	GetPortStatus();
	
	// If the last mode for Port B was Oscillate mode make sure it gets updated in case a System Reset occured.
	if( m_dwMode == MOTOR_OSCILLATE_2 && m_tPortStatus.usOscMode == OSC_MODE1)
	{
		m_dwMode = MOTOR_OSCILLATE_1;
		UpdateIntellioShaverPortUnitsAndMode(SCD_UNIT_RPM, SCD_MODE_OSCILLATE_1);
	}

	if(m_dwWarning != 0)
		DisplayWarning();
    else
        UpdateIntellioShaverPortErrWarn(0);
	
	UpdateDisplay();
	UpdateMaxSetSpeed();

    SendIntellioShaverUpdateIfChange();
}

SnWarningTable CProcedureRegion::ptWarningTable[] = 
{
	
	//
	// Prioritized list of all Possible Warnings, from highest to lowest
	//
	{ TEMPERATURE_WARNING,											   SCD_PW1,  SN_TEMPERATURE_FAULT },
	{ MOTORA_SHORT_CIRCUIT | MOTORB_SHORT_CIRCUIT |
	  MOTORA_SHORT_CIRCUIT_TIMEOUT | MOTORB_SHORT_CIRCUIT_TIMEOUT,	   SCD_PW8,  SN_SHORT_CIRCUIT_DETECTED },
	{ MOTORA_CURRENT_LIMIT_TIMEOUT | MOTORB_CURRENT_LIMIT_TIMEOUT,	   SCD_PW11, SN_MOTOR_CURRENT_LIMIT_TIMEOUT },
	{ MOTORA_CURRENT_LIMIT | MOTORB_CURRENT_LIMIT,					   SCD_PW10, SN_MOTOR_CURRENT_LIMIT },
	{ MOTORA_STALL_AND_CURRENT_LIMIT | MOTORB_STALL_AND_CURRENT_LIMIT, SCD_PW5,  SN_MOTOR_STALL_AND_CURRENT_LIMIT },
	{ MOTORA_STALL | MOTORB_STALL,									   SCD_PW6,	 SN_MOTOR_STALL },
	{ MOTORA_TAC_FAULT | MOTORB_TAC_FAULT,							   SCD_PW7,  SN_MOTOR_TAC_FAULT },
	{ HALL_PATTERN_FAULT_PORTA | HALL_PATTERN_FAULT_PORTB,			   SCD_PW4,	 SN_INVALID_HALL_PATTERN },
	{ UNKNOWN_DEVICE_ID_PORTA | UNKNOWN_DEVICE_ID_PORTB,			   SCD_PW3,	 SN_INVALID_DEVICE_ID },
	{ UNKNOWN_BLADE_ID_PORTA | UNKNOWN_BLADE_ID_PORTB,				   SCD_PW2,	 SN_INVALID_BLADE_ID },
	{ UNKNOWN_FOOTSWITCH_ID,										   SCD_PW13, SN_UNKNOWN_FOOT_ID },
	{ FOOTSWITCH_STUCK_PEDAL,										   SCD_PW14, SN_FOOT_STUCK_PEDAL },
	{ HANDPIECE_STUCK_BUTTON_PORTA | HANDPIECE_STUCK_BUTTON_PORTB,	   SCD_PW12, SN_HAND_STUCK_BUTTON },
	{ FOOTSWITCH_LOW_BATTERY,										   SCD_PW15, SN_FOOT_LOW_BATTERY },
	{ FOOTSWITCH_REQUIRED_PORTA | FOOTSWITCH_REQUIRED_PORTB,		   SCD_PW16, SN_FOOTSWITCH_REQUIRED},
	{ 0 }
	
};


void CProcedureRegion::DisplayWarning()
{
	CString csWarning = SN_CLEAR_TEXT;
	DWORD dwPortWarning= m_dwWarning;
	DWORD dwIntellioShaverWarning = 0;
	DWORD dwPortActiveWarning = 0;
	int iCnt = 0;

	// If there are warnings on this port, display the highest priority one
	if (dwPortWarning)
	{
		for (iCnt = 0; ptWarningTable[iCnt].dwWarnings; iCnt++)
		{
			if (dwPortWarning & ptWarningTable[iCnt].dwWarnings)
			{
				dwPortActiveWarning = ptWarningTable[iCnt].dwStringID;
				m_SnHelp.GetString(dwPortActiveWarning, csWarning);
				dwIntellioShaverWarning = ptWarningTable[iCnt].dwIntellioShaverWarnings;
				break;
			}
		}
	}
    
    UpdateIntellioShaverPortErrWarn((SnByte)dwIntellioShaverWarning);

    if( m_pStaticTextActiveWarning)
		m_pParent->SetDlgItemText(m_SnTextActiveWarning, csWarning);
	m_dwActiveWarning = dwPortActiveWarning;
}

void CProcedureRegion::HandleIntellioShaverCmd( int iParam, long lParam)
{
	if( m_pControl != NULL)
	{
		switch(iParam)
		{
			case KEY_DOWN_PORTA:
			case KEY_DOWN_PORTB:
				if(m_pDownArrowRect && !m_bDownArrowPressed && !m_bUpArrowPressed)
				{
                    m_bDownArrowPressed = TRUE;
                    DrawDownArrow(TRUE);
					UpdateData(DOWN);
                    m_bDownArrowPressed = FALSE;
                    DrawDownArrow(TRUE);
				}		
				break;
			
			case KEY_UP_PORTA:
			case KEY_UP_PORTB:
				if(m_pUpArrowRect && !m_bDownArrowPressed && !m_bUpArrowPressed)
				{
                    m_bUpArrowPressed = TRUE;
                    DrawUpArrow(TRUE);
					UpdateData(UP);
                    m_bUpArrowPressed = FALSE;
                    DrawUpArrow(TRUE);
				}		
				break;

			case KEY_DELTA_MODE_PORTA:
			case KEY_DELTA_MODE_PORTB:
                if (m_pButtonDeltaMode && m_pButtonDeltaMode->IsWindowVisible()) {
					OnButtonDeltaMode();
                }
				break;

			default:
				break;
		}
	}
}

void CProcedureRegion::OnStaticTextActiveWarning()
{
	if( !m_bActiveWarningPressed && m_pStaticTextActiveWarning)
	{
		m_bActiveWarningPressed = TRUE;
		
		m_pParent->DisplayWarningDetails(m_dwActiveWarning);
		
		m_bActiveWarningPressed = FALSE;
	}
}

void CProcedureRegion::OnLButtonUp(UINT nFlags, CPoint point)
{
	if( m_pButtonWindowLock && m_bWindowLockPressed)
	{
		// Disable Window Lock
		if( m_pControl)
		{
			SnWord usTemp = WINDOW_LOCK_OFF;
			// Save the settings
			m_pControl->SetCmdState(m_SnSetMcWindowLock, &usTemp, sizeof(usTemp));
		}

		m_bWindowLockPressed = FALSE;

		DrawWindowLockButton();
	}
}

void CProcedureRegion::OnButtonDownArrowPressed()
{
	if( m_pDownArrowRect && !m_bDownArrowPressed && !m_bUpArrowPressed)
	{
        m_bDownArrowPressed = TRUE;
        DrawDownArrow(TRUE);

		// Start the timer a button has been pressed
		m_pParent->StartScrollingTimer();
	
		// Decrement value
		UpdateData(DOWN); 
	}
}

void CProcedureRegion::OnButtonUpArrowPressed()
{
	if( m_pUpArrowRect  && !m_bDownArrowPressed && !m_bUpArrowPressed)
	{
        m_bUpArrowPressed = TRUE;
        DrawUpArrow(TRUE);

		// Start the timer a button has been pressed
		m_pParent->StartScrollingTimer();
	
		// Increment value
		UpdateData(UP); 
	}
}

void CProcedureRegion::OnButtonWindowLockPressed()
{
	// Enable Window Lock for Port
	if( m_pControl)
	{
		// Draw the bitmap button
		m_pButtonWindowLock->SetBitmap(CProcedureScreen::m_WindowLockPressed);
		
		m_bWindowLockPressed = TRUE;
		
		SnWord usTemp = WINDOW_LOCK_ON;
		// Write to control layer
		m_pControl->SetCmdState(m_SnSetMcWindowLock, &usTemp, sizeof(usTemp));

	}
}

void CProcedureRegion::OnButtonDeltaMode()
{
	
	Sleep(50); // sleep to capture button press
	GetPortStatus();
	
	// Toggle the Oscillate mode
	if( m_tPortStatus.usOscMode == OSC_MODE1)
	{
		m_tPortStatus.usOscMode = OSC_MODE2;
		m_dwMode = MOTOR_OSCILLATE_2;
	}
	else
	{
		m_tPortStatus.usOscMode = OSC_MODE1;
		m_dwMode = MOTOR_OSCILLATE_1;
	}
	
	SetPortStatus(); // Update the control layer
	GetPortStatus(); // get the new status

    // Update Intellio Shaver
	if( m_tPortStatus.usOscMode == OSC_MODE2)
	{
        UpdateIntellioShaverPortUnitsAndMode(SCD_UNIT_RATE, SCD_MODE_OSCILLATE_2);
	}
	else
	{
        UpdateIntellioShaverPortUnitsAndMode(SCD_UNIT_RPM, SCD_MODE_OSCILLATE_1);
	}
	
	UpdateDisplay();	  // Update display
	UpdateMaxSetSpeed(); // Update display
	
    if (m_pControl)
    {
		SnBool bStatus;
		SnWord usTemp = SAVE_NVRAM;

        // Save the settings
		bStatus = m_pControl->SetCmdState(SET_SYSTEM_PARAMETERS, &usTemp, sizeof(usTemp));
		if( !bStatus)
		{
			m_pParent->SetDrawRect( FALSE );
			CMessageBox dlg( m_pControl, SN_SYSTEM_ERROR, SN_CUSTOM_SETTINGS_SAVE_FAILURE);
			dlg.DoModal();
			m_pParent->SetDrawRect( TRUE );
			m_pParent->SetReDraw( TRUE );
		}
    }
}

void CProcedureRegion::GetPortStatus()
{
	if( m_pControl)
	{
		m_pControl->GetCmdState(m_SnGetMcPortStatus, &m_tPortStatus, sizeof(SN_PORT_STATUS));
	}
}

void CProcedureRegion::SetPortStatus()
{
	if( m_pControl)
	{
		m_pControl->SetCmdState(m_SnSetMcPortStatus, &m_tPortStatus, sizeof(SN_PORT_STATUS));
	}
}

void CProcedureRegion::InitWindowLockRect(void)
{
	long windowlockLocX;

	if( m_dwPort == PORTA)
	{
		windowlockLocX = 15;
	}
	else
	{
		windowlockLocX = 650;
	}

    m_tWindowLockRect.left = windowlockLocX;
    m_tWindowLockRect.top = 416;
    m_tWindowLockRect.right = windowlockLocX+134;
    m_tWindowLockRect.bottom = 416 + 59;
}

void CProcedureRegion::DrawWindowLockButton()
{

	if( m_pButtonWindowLock)
	{
		m_pButtonWindowLock->SetBitmap(CProcedureScreen::m_WindowLock);
	}
}

void CProcedureRegion::UpdateData(unsigned char mode) 
{
	int ii, iOrg;
	SnWord usMax,usMin,usIncrement;
	
	// Get Minimum, Maximum and Increment values
    ii = GetMinMaxInc(&usMin, &usMax, &usIncrement);
	iOrg = ii;
	
	
	if (m_dwMode == MOTOR_OSCILLATE_2)
	{
		// Get the current selection
		
		if( mode == UP)
		{
			if( (ii <= usMax) && (ii > usMin))
			{
				ii = ii - usIncrement; // decrement 
			}
		}
		else if( mode == DOWN)
		{
			if( (ii < usMax) && (ii >= usMin))
			{
				ii = ii + usIncrement; // increment 
			}
		}
	}
	else
	{
		// Get the current selection
		
		if( mode == DOWN)
		{
			if( (ii <= usMax) && (ii > usMin))
			{
				ii = ii - usIncrement; // decrement 
			}
		}
		else if( mode == UP)
		{
			if( (ii < usMax) && (ii >= usMin))
			{
				ii = ii + usIncrement; // increment 
			}
		}
	}
	
	SnWord usTemp = ii;
	
	if(m_dwPortType == PORT_TYPE_MDU)
	{
		m_iArrowScrollScaler = 1;
		switch (m_dwMode)
		{
		case MOTOR_REVERSE:
			if(m_bDualRpmOn)
				m_tPortStatus.usReverse2 = ii;
			else
				m_tPortStatus.usReverse = ii;
			break;
		case MOTOR_FORWARD:
			if(m_bDualRpmOn)
				m_tPortStatus.usForward2 = ii;
			else
				m_tPortStatus.usForward = ii;
			break;
		case MOTOR_OSCILLATE_1:
			m_tPortStatus.usOscillateRpm = ii;
			break;
		case MOTOR_OSCILLATE_2:
			m_tPortStatus.wOscillateSeconds = ii;
			m_iArrowScrollScaler = 4;
			if(usIncrement)
				usTemp = ((usMax - ii) / usIncrement) + 1;
			else
				usTemp = m_wSetSpeed;
			break;
		}
		
	}	
	else if(m_dwPortType == PORT_TYPE_TOOL)
	{
		m_iArrowScrollScaler = 4;
		m_tPortStatus.usPercent = ii;
		
	}

	m_wSetSpeed = usTemp;

    if( m_pControl)
	{
		if(m_dwPortType == PORT_TYPE_MDU && m_dwMode != MOTOR_OSCILLATE_2)
		{
            UpdateIntellioShaverPortSetSpeed((SnByte)(m_wSetSpeed / 100));
		}	
		else
		{
			UpdateIntellioShaverPortSetSpeed((SnByte)m_wSetSpeed);
		}
	}

	
	if (ii != iOrg)
	{
        UpdateSetSpeedDisplay();
    }

	SetPortStatus();
	HideButtons();
	
	if( m_pControl)
	{
		SnWord usTemp = SAVE_FLASH;
		// Save the settings
		m_pControl->SetCmdState(SET_SYSTEM_PARAMETERS, &usTemp, sizeof(usTemp));
	}

    SendIntellioShaverUpdateIfChange();
}

SnBool CProcedureRegion::ConfigurePort()
{
	SnBool result = TRUE;

	// Save the device type away locally in case it changes
	// while the display is being updated.
	SnWord          wType = m_tPortStatus.usType;

	switch( m_tPortStatus.usType)
	{
	
		case TYPE_DP_DRILL:
		case TYPE_DP_SAW:
			result = CreateToolWindow();
			break;
	
		case TYPE_INVALID:
			result = CreateInvalidWindow();
			break;

		case TYPE_MDU_UTLRA_IUR:
		case TYPE_MDU_STANDARD:
		case TYPE_MDU_STANDARD_CTL:
		case TYPE_MDU_MINI:
		case TYPE_MDU_RELIANT:
		case TYPE_MDU_RELIANT_CTL:
		case TYPE_MDU_RELIANT_BF:
		case TYPE_MDU_POWERMINI:
		case TYPE_MDU_POWERMINI_CTL:
		case TYPE_MDU_POWERMINI_BF:
			result = CreateMduWindow();
			break;

		default:
			result = FALSE;
			break;
	}

    if (result)
    {
		m_wType = wType;
        if (IS_TYPE_MDU(wType))
            UpdateIntellioShaverPortBlade(m_tPortStatus.usShaverBladeId);
    }
	else
    {
		m_wType = TYPE_ERROR;
        NoIntellioShaverHandpiecePresent();
    }

    SendIntellioShaverUpdateIfChange();

	return result;
}

SnBool CProcedureRegion::CreateInvalidWindow()
{
	CSharedMemory mem;
	
	m_bDisplayArrows = FALSE;

	// Make sure previous resources are deallocated before new window is created
	KillWindow();

    m_dwPortType = PORT_TYPE_INVALID;

    // Draw the Connector
    DrawConnector(TRUE);
	
	// Create Static control
	m_pStaticTextActiveWarning = CreateStaticControl(CRect(90+m_DisplayOffsetX, 285, 310+m_DisplayOffsetX, 335),m_SnTextActiveWarning,CTL_TEXT);
	if(!m_pStaticTextActiveWarning)
	{
		// Failed to create window control
		KillWindow();
		return FALSE;
	}

	m_pStaticTextActiveWarning->SetFont(mem.m_Font14Normal, TRUE);
    if(m_dwWarning != 0)
		DisplayWarning();
    else
        UpdateIntellioShaverPortErrWarn(0);
	
    NoIntellioShaverHandpiecePresent();

	return TRUE;
}


SnBool CProcedureRegion::CreateToolWindow( )
{
	CSharedMemory mem;
	
	m_bDisplayArrows = TRUE;
		
	// Make sure resources are deallocated
	KillWindow();
	
	m_dwPortType = PORT_TYPE_TOOL;

	// Create Static control
	m_pStaticTextActiveWarning = CreateStaticControl(CRect(90+m_DisplayOffsetX, 285, 310+m_DisplayOffsetX, 335),m_SnTextActiveWarning,CTL_TEXT);
	if(!m_pStaticTextActiveWarning)
	{
		// Failed to create window control
		KillWindow();
		return FALSE;
	}
	if(m_dwWarning != 0)
		DisplayWarning();
    else
        UpdateIntellioShaverPortErrWarn(0);

    // Setup the buttons
    m_pDownArrowRect = &m_tDownArrowRect;
    m_pUpArrowRect = &m_tUpArrowRect;

	// Setup the fonts
	m_pStaticTextActiveWarning->SetFont(mem.m_Font14Normal, TRUE);
	
	// Get Port status from the control layer
	GetPortStatus();

	// Find out what direction the hand tool is in
	m_dwMode = m_tPortStatus.usDisplayMode;

    UpdateDisplay();
    DrawSetSpeed(TRUE);
	
    UpdateIntellioShaverPortBlade(BLADE_ID_OTHER);
    UpdateIntellioShaverPortSetSpeed((SnByte)m_tPortStatus.usPercent);
	UpdateIntellioShaverPortUnitsAndMode(SCD_UNIT_PERCENT, m_tPortStatus.usType == TYPE_DP_DRILL ? SCD_MODE_REVERSE:SCD_MODE_OSCILLATE_1);
	
	return TRUE;
}

void CProcedureRegion::SetupTextButtons()
{
	CSharedMemory mem;

    if (m_pButtonDeltaMode)
    {
	    m_pButtonDeltaMode->LoadBitmaps(IDB_BITMAP_BUTTON_DELTA_MODE, IDB_BITMAP_BUTTON_DELTA_MODE_PRESSED,
		    mem.m_Font14Normal, m_SnHelp.GetString(SN_MODE));
    }
}

SnBool CProcedureRegion::CreateMduWindow()
{
	CSharedMemory mem;

	int windowlockLocX;
	int deltaModeLocX;
	
	m_bDisplayArrows = TRUE;
	
	if( m_dwPort == PORTA)
	{
		windowlockLocX = 16;
		deltaModeLocX = windowlockLocX + 149;
	}
	else
	{
		windowlockLocX = 650;
		deltaModeLocX = windowlockLocX - 149;
	}
	
	// Make sure resources are deallocated
	KillWindow();
	
	m_dwPortType = PORT_TYPE_MDU;
	
    // Setup the buttons
    m_pDownArrowRect = &m_tDownArrowRect;
    m_pUpArrowRect = &m_tUpArrowRect;
    m_pWindowLockRect = &m_tWindowLockRect;

	// Create Static control
	m_pStaticTextActiveWarning = CreateStaticControl(CRect(90+m_DisplayOffsetX, 285, 310+m_DisplayOffsetX, 335),m_SnTextActiveWarning,CTL_TEXT);
	if(!m_pStaticTextActiveWarning)
	{
		// Failed to create window control
		KillWindow();
		return FALSE;
	}
	if(m_dwWarning != 0)
		DisplayWarning();
    else
        UpdateIntellioShaverPortErrWarn(0);
	
	// Create static button control
	m_pButtonWindowLock = CreateStaticControl(CRect(windowlockLocX, 350, windowlockLocX+10, 360),m_SnButtonWindowLock, CTL_BUTTON);
	if( !m_pButtonWindowLock)
	{
		// Failed to allocate resources
		KillWindow();
		return FALSE;
	}

	// Create Bitmap Button
	m_pButtonDeltaMode = CreateBitButtonControl(CRect(deltaModeLocX, 350, deltaModeLocX+10, 360),m_SnButtonDeltaMode);
	if( !m_pButtonDeltaMode)
	{
		// Failed to allocate resources
		KillWindow();
		return FALSE;
	}
	
	if( m_pButtonWindowLock)
		m_pButtonWindowLock->SetBitmap( CProcedureScreen::m_WindowLock);
	
	SetupTextButtons();
	
	// Setup the fonts
	m_pStaticTextActiveWarning->SetFont(mem.m_Font14Normal, TRUE);
	
	// Get Port status from the control layer
	GetPortStatus();

	// Default MDU to display Oscillate Mode
	if(m_tPortStatus.usOscMode == OSC_MODE2 && !m_tPortStatus.yForceOscMode1)
		m_dwMode = MOTOR_OSCILLATE_2;
	else
		m_dwMode = MOTOR_OSCILLATE_1;

#if WINDOWLOCK_BUTTON_DIGITAL_FOOT
	// See if we need to display the window lock button on the GUI
	SN_FOOT_STATUS tFootStatus = m_pParent->GetFootStatus();
	SnBool bFootAssigned;

	bFootAssigned = (m_dwPort == tFootStatus.usFootAssignedPort);

	if( bFootAssigned && tFootStatus.usType == TYPE_DIGITAL_FOOTPEDAL && m_pButtonWindowLock)
		m_pButtonWindowLock->ShowWindow(SW_SHOW);
	else
		m_pButtonWindowLock->ShowWindow(SW_HIDE);
#else
	m_pButtonWindowLock->ShowWindow(SW_SHOW);
#endif
	
	// Hide the Delta Mode Button if the MDU does not support Mode 2
	if( m_tPortStatus.yForceOscMode1)
		m_pButtonDeltaMode->ShowWindow(SW_HIDE);

    UpdateDisplay();
    DrawSetSpeed(TRUE);
	UpdateMaxSetSpeed();

    // Update Intellio Shaver
	if( m_pControl)
	{
        UpdateIntellioShaverPortBlade((SnByte)m_tPortStatus.usShaverBladeId);
	
		if( m_dwMode == MOTOR_OSCILLATE_2)
		{
			UpdateIntellioShaverPortUnitsAndMode(SCD_UNIT_RATE, SCD_MODE_OSCILLATE_2);			
		}
		else
		{
			UpdateIntellioShaverPortUnitsAndMode(SCD_UNIT_RPM, SCD_MODE_OSCILLATE_1);			
		}			
	}
	
	return TRUE;
}

void CProcedureRegion::UpdateStatus(int iParam)
{
	// Something has changed get PortA and PortB status from the control layer
	GetPortStatus();
	
	switch( iParam)
	{
		
	case MSG_UPDATE_FOOT_STATUS:
		if( m_pControl && m_pButtonWindowLock)
		{
#if WINDOWLOCK_BUTTON_DIGITAL_FOOT
			SN_FOOT_STATUS tFootStatus = m_pParent->GetFootStatus();
			// See if we need to display the Window Lock button
			if( tFootStatus.usType == TYPE_DIGITAL_FOOTPEDAL)
			{
				if( m_dwPort == tFootStatus.usFootAssignedPort)
				{
					m_pButtonWindowLock->ShowWindow(SW_SHOW);
				}
				else
				{
					m_pButtonWindowLock->ShowWindow(SW_HIDE);
				}
			}
			else
			{
				// Hide window lock buttons not required
				m_pButtonWindowLock->ShowWindow(SW_HIDE);
			}
#else
		    m_pButtonWindowLock->ShowWindow(SW_SHOW);
#endif
		}		
		break;
		
	case MSG_UPDATE_RUNNING_STATUS:
		
		if( PortRunning())
		{	
#if !WINDOWLOCK_BUTTON_DIGITAL_FOOT
			if( m_dwPortType == PORT_TYPE_MDU && !m_bWindowLockPressed)
    			m_pButtonWindowLock->ShowWindow(SW_HIDE);
#endif
			if( m_pButtonDeltaMode)
			{
				m_pButtonDeltaMode->ShowWindow(SW_HIDE);
			}

            
		}
		else
		{
#if !WINDOWLOCK_BUTTON_DIGITAL_FOOT
			if( m_dwPortType == PORT_TYPE_MDU)
    			m_pButtonWindowLock->ShowWindow(SW_SHOW);
#endif
			if( m_pButtonDeltaMode && !m_tPortStatus.yForceOscMode1 && 
				(m_dwMode == MOTOR_OSCILLATE_1 || m_dwMode == MOTOR_OSCILLATE_2))
			{
				m_pButtonDeltaMode->ShowWindow(SW_SHOW);
			}
		}
		
		SetDirectionControl();
		
		UpdateIntellioShaverPortRunState(PortRunning());
		break;

	case MSG_UPDATE_HANDPIECE_DUAL_RPM_ON:
	    {
		    if( m_dwPortType == PORT_TYPE_MDU)
		    {
			    m_bDualRpmOn = TRUE;			
			    UpdateDisplay();
		    }
	    }
	    break;
	
	case MSG_UPDATE_HANDPIECE_DUAL_RPM_OFF:
		{
			if( m_dwPortType == PORT_TYPE_MDU)
			{
				m_bDualRpmOn = FALSE;			
				UpdateDisplay();
			}
		}
		break;
		
		
		
	case MSG_UPDATE_BLADE_STATUS:
		{
			if( m_dwPortType == PORT_TYPE_MDU)
			{
				UpdateDisplay();
				UpdateMaxSetSpeed();
                UpdateIntellioShaverPortBlade(m_tPortStatus.usShaverBladeId);
			}
		}
		
		break;
		
	case MSG_UPDATE_MODE_STATUS:
		// Mode has changed
		{
			switch(m_tPortStatus.usDisplayMode)
			{
				
			case MOTOR_OFF:
				break;
				
			case MOTOR_REVERSE:
				m_dwMode = MOTOR_REVERSE;
				
				// Set direction indicators
				SetDirectionControl(); 
				UpdateDisplay();
				UpdateMaxSetSpeed();
				
				if( m_dwPortType == PORT_TYPE_MDU)
				{
                    UpdateIntellioShaverPortUnitsAndMode(SCD_UNIT_RPM, SCD_MODE_REVERSE);
				}
				else if( m_dwPortType == PORT_TYPE_TOOL)
				{
                    UpdateIntellioShaverPortUnitsAndMode(SCD_UNIT_PERCENT, SCD_MODE_REVERSE);
				}	
				break;
				
			case MOTOR_FORWARD:
				m_dwMode = MOTOR_FORWARD;
				
				// Set direction indicators
				SetDirectionControl(); 
				UpdateDisplay();
				UpdateMaxSetSpeed();
				
				if( m_dwPortType == PORT_TYPE_MDU)
				{
                    UpdateIntellioShaverPortUnitsAndMode(SCD_UNIT_RPM, SCD_MODE_FORWARD);
				}
				else if( m_dwPortType == PORT_TYPE_TOOL)
				{
                    UpdateIntellioShaverPortUnitsAndMode(SCD_UNIT_PERCENT, SCD_MODE_FORWARD);
				}	
				break;
				
			case MOTOR_OSCILLATE_1:
				m_dwMode = MOTOR_OSCILLATE_1;
				
				// Set direction indicators
				SetDirectionControl(); 
				UpdateDisplay();
				UpdateMaxSetSpeed();
				
				if( m_dwPortType == PORT_TYPE_MDU)
				{
                    UpdateIntellioShaverPortUnitsAndMode(SCD_UNIT_RPM, SCD_MODE_OSCILLATE_1);
				}
				else if( m_dwPortType == PORT_TYPE_TOOL)
				{
                    UpdateIntellioShaverPortUnitsAndMode(SCD_UNIT_PERCENT, SCD_MODE_OSCILLATE_1);
				}	
				break;
				
			case MOTOR_OSCILLATE_2:
				m_dwMode = MOTOR_OSCILLATE_2;
				
				// Set direction indicators
				SetDirectionControl(); 
				UpdateDisplay();
				UpdateMaxSetSpeed();
				UpdateIntellioShaverPortUnitsAndMode(SCD_UNIT_RATE, SCD_MODE_OSCILLATE_2);
				break;
				
			case MOTOR_WINDOW_LOCK:
				break;
				
			default:
				break;
			} // end switch
		}
		break;
			
	case MSG_UPDATE_PORT_STATUS:
		// Update Port Configuration
		{
			if (m_wType != m_tPortStatus.usType)
                ConfigurePort();
		}
		break;
		
    case MSG_UPDATE_ALL_SETTINGS:
        SetupTextButtons();
        if (m_pButtonDeltaMode)
            m_pButtonDeltaMode->RedrawWindow();
		UpdateDisplay();
		UpdateMaxSetSpeed();
        break;

	default:
		break;
		
	}// end switch
	
    SendIntellioShaverUpdateIfChange();
}

DWORD WINAPI CProcedureRegion::DisplayThread(LPVOID pParam)
{
	// Thread used to display bitmaps that indicate direction of running instrument.
	// Forward, Reverse or Oscillate 
	CProcedureRegion *pClass = (CProcedureRegion*)pParam;
	SnBool startRunning = TRUE;
	DWORD usDisplayModePrev = pClass->m_dwMode;
	
	while(!pClass->m_bKillThreads)
	{
		if (usDisplayModePrev != pClass->m_dwMode || pClass->m_dwModeChange)
		{
			usDisplayModePrev = pClass->m_dwMode;
			pClass->m_dwModeChange = FALSE;
			startRunning = TRUE;
		}

		if(pClass->PortRunning())
		{
			switch (pClass->m_tPortStatus.usDisplayMode)
			{
			case MOTOR_FORWARD:
				pClass->DirectionForward( startRunning);
				break;
			case MOTOR_REVERSE:
				pClass->DirectionReverse( startRunning);
				break;
			case MOTOR_OSCILLATE_1:
			case MOTOR_OSCILLATE_2:
				pClass->DirectionOscillate( startRunning);
				break;
			case MOTOR_WINDOW_LOCK:
				pClass->DirectionWindowlock( startRunning);
				break;
			}
		}
		else
		{
			startRunning = TRUE;
			Sleep(50);
		}

	}
	
	// Set the Thread Killed Event
	SetEvent( pClass->m_hDisplayThreadKilledEvent);
	
	return 0;

}

SnSWord CProcedureRegion::GetPowerRating()
{
	SnWord usSetSpeed;
	SnWord usActualVelocity;	
	SnSWord sPower;
	DWORD dwMode;
	SnWord usOscillateSeconds;
	SnWord usPeriod;
	SnWord usMinVelocity = 0;
	
	// Get the current port status
	
	GetPortStatus();
	dwMode = m_dwMode;
	
	// Get Target Speed from the display, that way we don't have to figure out the direction
	if(IS_TYPE_MDU(m_tPortStatus.usType))
	{
		if( dwMode == MOTOR_OSCILLATE_2)
		{
			usOscillateSeconds = m_tPortStatus.wOscillateSeconds;
			usPeriod = m_tPortStatus.wPeriod;
		}
		else
			usSetSpeed = m_wSetSpeed;
		
	}
	else if(IS_TYPE_POWER_INSTR(m_tPortStatus.usType))
	{
		float fPercent = (float)m_tPortStatus.usPercent/100.0f; 
		usSetSpeed = (SnWord)((m_tPortStatus.usForwardMax - m_tPortStatus.usForwardMin) * fPercent + m_tPortStatus.usForwardMin);
		usMinVelocity = m_tPortStatus.usForwardMin;
	}
	
	if( m_pControl)
		usActualVelocity = m_tPortStatus.usActualVelocity;
	else 
		usActualVelocity = usSetSpeed;
	
	if( dwMode == MOTOR_OSCILLATE_2)
	{
		if (55 - usOscillateSeconds)
		{
			DWORD dwPower = sMaxPowerRating * (usPeriod - usOscillateSeconds);
			dwPower <<= 4;
			dwPower /= (55 - usOscillateSeconds);
			dwPower >>= 4;
			sPower = (short)( dwPower);
		}
		else
			sPower = 0;

	}
	else 
	{
		if (usSetSpeed && (usSetSpeed - usMinVelocity))
			sPower = (short)(sMaxPowerRating * (1.0 - ((float)(usActualVelocity - usMinVelocity))/ (usSetSpeed - usMinVelocity)));
		else
			sPower = 0;
	}

	if (sPower < 0)
        sPower = 0;
	else if (dwMode == MOTOR_OSCILLATE_1 || dwMode == MOTOR_OSCILLATE_2)
	{
		sPower = (sPower +1)/2;
		if (sPower >= sHalfPowerRating)
			sPower = sHalfPowerRating -1;
	}
	else
	{
		if (sPower >= sMaxPowerRating)
			sPower = sMaxPowerRating -1;
	}
    
	return (SnWord)sPower;
}

void CProcedureRegion::DirectionReverse( SnBool& startRunning)
{
	int ii;
	SnSWord sPowerRating;

    sPowerRating = GetPowerRating();

	if (startRunning)
	{
		// Move Direction Arrows Reverse
		for( ii = 0; ii < sMaxPowerRating; ii++)
		{
			if(!PortRunning())
				break; // get out
            DrawArrow(ii, &CProcedureScreen::m_ArrowLeftGreen);
			m_wDirection[ii]=IDB_BITMAP_GREEN_ARROW_LEFT;
		}
		startRunning = FALSE;
	}
	
	// Move Direction Arrows Reverse
	for( ii = (sMaxPowerRating-1); ii >= sPowerRating; ii--)
	{
		Sleep(10);
		
		if(!PortRunning())
			break; // get out
        DrawArrow(ii, NULL);
		m_wDirection[ii]=IDB_BITMAP_BLANK;
		
		Sleep(10);
		
		if(!PortRunning())
			break; // get out
        DrawArrow(ii, &CProcedureScreen::m_ArrowLeftGreen);
		m_wDirection[ii]=IDB_BITMAP_GREEN_ARROW_LEFT;
		sPowerRating = GetPowerRating();
	}
	
	SnBool bNeedSleep = FALSE;
	// Setup the bitmaps
	for( ii = 0; ii < sPowerRating ; ii++)
	{
		bNeedSleep = FALSE;
		// Set the white arrows
		if(!PortRunning())
			break; // get out
		if( m_wDirection[ii] != IDB_BITMAP_ARROW_LEFT)
		{
			bNeedSleep = TRUE;
            DrawArrow(ii, &CProcedureScreen::m_ArrowLeftWhite);
			m_wDirection[ii] = IDB_BITMAP_ARROW_LEFT;
		}
		if(bNeedSleep)
		{
			sPowerRating = GetPowerRating();
		}
	}
	
}

void CProcedureRegion::DirectionForward( SnBool& startRunning)
{
    int ii;
	SnSWord sPowerRating;

    sPowerRating = (sMaxPowerRating-1) - GetPowerRating(); // invert the power level

    if (startRunning)
	{
		// Move Direction Arrows Forward
		for( ii = (sMaxPowerRating-1); ii >= 0; ii--)
		{
			if(!PortRunning())
				break; // get out
            DrawArrow(ii, &CProcedureScreen::m_ArrowRightGreen);
			m_wDirection[ii] = IDB_BITMAP_GREEN_ARROW_RIGHT;			
		}	
		startRunning = FALSE;
	}
	
	// Move Direction Arrows forward
	for( ii = 0; ii <= sPowerRating; ii++)
	{	
		Sleep(10);
		
		if(!PortRunning())
			break; // get out
        DrawArrow(ii, NULL);
		m_wDirection[ii] = IDB_BITMAP_BLANK;
		
		Sleep(10);
		
		if(!PortRunning())
			break; // get out
        DrawArrow(ii, &CProcedureScreen::m_ArrowRightGreen);
		m_wDirection[ii] = IDB_BITMAP_GREEN_ARROW_RIGHT;
		sPowerRating = (sMaxPowerRating-1) - GetPowerRating(); // invert the power level
		
	}

    SnBool bNeedSleep = FALSE;
	// Setup the bitmaps
	for( ii = (sMaxPowerRating-1); ii > sPowerRating; ii--)
	{
		bNeedSleep = FALSE;
		// Set the white arrows
		if(!PortRunning())
			break; // get out
		if( m_wDirection[ii] != IDB_BITMAP_ARROW_RIGHT)
		{
			bNeedSleep = TRUE;
            DrawArrow(ii, &CProcedureScreen::m_ArrowRightWhite);
			m_wDirection[ii] = IDB_BITMAP_ARROW_RIGHT;
		}
		if(bNeedSleep)
		{
			sPowerRating = (sMaxPowerRating-1) - GetPowerRating(); // invert the power level
		}
	}
}

void CProcedureRegion::DirectionOscillate( SnBool& startRunning)
{
	int ii, jj;
	
	SnSWord sPowerRating;
	
	sPowerRating = GetPowerRating();
	
	if(startRunning)
	{
		for( ii = 0, jj = (sMaxPowerRating-1); ii < sHalfPowerRating; ii++, jj--)
		{
			if(!PortRunning())
				break; // get out
            DrawArrow(ii, &CProcedureScreen::m_ArrowLeftGreen);
			m_wDirection[ii] = IDB_BITMAP_GREEN_ARROW_LEFT;
			if(!PortRunning())
				break; // get out
            DrawArrow(jj, &CProcedureScreen::m_ArrowRightGreen);
			m_wDirection[jj] = IDB_BITMAP_GREEN_ARROW_RIGHT;
		}
		startRunning = FALSE;
	}
	
	// Move Direction Arrows Reverse
	for( jj = sHalfPowerRating, ii = (jj-1); ii >= sPowerRating; ii--, jj++)
	{
		Sleep(15);
		
		if(!PortRunning())
			break; // get out
        DrawArrow(ii, NULL);
		m_wDirection[ii] = IDB_BITMAP_BLANK;
		if(!PortRunning())
			break; // get out
        DrawArrow(jj, NULL);
		m_wDirection[jj] = IDB_BITMAP_BLANK;
		
		Sleep(15);
		
		if(!PortRunning())
			break; // get out
        DrawArrow(ii, &CProcedureScreen::m_ArrowLeftGreen);
		m_wDirection[ii] = IDB_BITMAP_GREEN_ARROW_LEFT;
		if(!PortRunning())
			break; // get out
        DrawArrow(jj, &CProcedureScreen::m_ArrowRightGreen);
		m_wDirection[jj] = IDB_BITMAP_GREEN_ARROW_RIGHT;
		
		SnSWord sNewPowerRating = GetPowerRating();
		if (sNewPowerRating	< sPowerRating)
			sPowerRating = sNewPowerRating;
	}
	
	SnBool bNeedSleep = FALSE;
	// Setup the bitmaps
	for( ii = 0, jj = (sMaxPowerRating-1); ii < sPowerRating; ii++, jj--)
	{
		bNeedSleep = FALSE;
		// Set the white arrows
		if(!PortRunning())
			break; // get out
		if(m_wDirection[ii] != IDB_BITMAP_ARROW_LEFT) 
		{
			bNeedSleep = TRUE;
            DrawArrow(ii, &CProcedureScreen::m_ArrowLeftWhite);
			m_wDirection[ii] = IDB_BITMAP_ARROW_LEFT;
		}
		if	(m_wDirection[jj] != IDB_BITMAP_ARROW_RIGHT)
		{
			bNeedSleep = TRUE;
            DrawArrow(jj, &CProcedureScreen::m_ArrowRightWhite);
			m_wDirection[jj] = IDB_BITMAP_ARROW_RIGHT;
		}
		if (bNeedSleep)
		{
			sPowerRating = GetPowerRating();
		}
	}
}

void CProcedureRegion::DirectionWindowlock( SnBool& startRunning)
{
	int ii, jj;
	
	if(startRunning)
	{
		for( ii = 0, jj = (sMaxPowerRating-1); ii < sHalfPowerRating; ii++,jj--)
		{
			if(!PortRunning())
				break; // get out
            DrawArrow(ii, &CProcedureScreen::m_ArrowRightGreen);
			m_wDirection[ii] = IDB_BITMAP_GREEN_ARROW_RIGHT;
			if(!PortRunning())
				break; // get out
            DrawArrow(jj, &CProcedureScreen::m_ArrowLeftGreen);
			m_wDirection[jj] = IDB_BITMAP_GREEN_ARROW_LEFT;
		}
		startRunning = FALSE;
	}
	
	// Move Direction Arrows Reverse
	for( ii = 0, jj = (sMaxPowerRating-1); ii < sHalfPowerRating; ii++, jj--)
	{
		Sleep(15);
		
		if(!PortRunning())
			break; // get out
        DrawArrow(ii, NULL);
		m_wDirection[ii] = IDB_BITMAP_BLANK;
		if(!PortRunning())
			break; // get out
        DrawArrow(jj, NULL);
		m_wDirection[jj] = IDB_BITMAP_BLANK;
		
		Sleep(15);
		
		if(!PortRunning())
			break; // get out
        DrawArrow(ii, &CProcedureScreen::m_ArrowRightGreen);
		m_wDirection[ii] = IDB_BITMAP_GREEN_ARROW_RIGHT;
		if(!PortRunning())
			break; // get out
        DrawArrow(jj, &CProcedureScreen::m_ArrowLeftGreen);
		m_wDirection[jj] = IDB_BITMAP_GREEN_ARROW_LEFT;
	}
}

//
/////////////////////////////////////////////////////////////////////////////

void CProcedureRegion::DrawSetSpeed(SnBool yDisplay)
{
    if (m_pParent->GetDrawRect() && m_yFirstPaintOccured)
    {
        if (yDisplay)
        {
            DrawArrows();

            SN_RECT tRect = {100 + m_DisplayOffsetX, 131, 200, 93};
			    
		    m_SnHelp.SetLineWidth( SN_LINE_WIDTH_10);
            m_SnHelp.DrawRectEmpty( m_dwPort == PORTA ? SN_BLUE : SN_YELLOW, tRect);

            if (m_wSetSpeed)
            {
				m_SetSpeedDisplay.yForceRedraw = TRUE;
				UpdateSetSpeedDisplay();
			}
	    }
	    else
        {
            // Blank out the Arrows
            CSnHelp::m_hDisplayCDC->BitBlt( 100+m_DisplayOffsetX, 83, 20*10, 34,
                NULL, 0, 0, BLACKNESS);

            // Blank out the Set Speed Box
            CSnHelp::m_hDisplayCDC->BitBlt( 100-5+m_DisplayOffsetX, 126, 200+10, 93+10,
                NULL, 0, 0, BLACKNESS);
        }
    }
}

void CProcedureRegion::DrawArrow(int iNum, CBitmap *pBitmap)
{
    if (m_pParent->GetDrawRect())
    {
        if (pBitmap)
        {
            CBitmap *pOldBitmap;
	        EnterCriticalSection(&CSnHelp::m_DisplayCs);
            pOldBitmap = CSnHelp::m_hMemoryCDC.SelectObject(pBitmap);
            CSnHelp::m_hDisplayCDC->BitBlt((100+m_DisplayOffsetX)+(iNum*20), 83, 20, 34,
                &CSnHelp::m_hMemoryCDC, 0, 0, SRCCOPY);
            CSnHelp::m_hMemoryCDC.SelectObject( pOldBitmap);
	        LeaveCriticalSection(&CSnHelp::m_DisplayCs);
        }
        else 
        {
            CSnHelp::m_hDisplayCDC->BitBlt( (100+m_DisplayOffsetX)+(iNum*20), 83, 20, 34,
                NULL, 0, 0, BLACKNESS);
        }
    }
}

void CProcedureRegion::InitUpArrowRect(void)
{
    m_tUpArrowRect.left = 214 + m_DisplayOffsetX;
    m_tUpArrowRect.top = 266;
    m_tUpArrowRect.right = 214 + 72 + m_DisplayOffsetX;
    m_tUpArrowRect.bottom = 266 + 62;
}

void CProcedureRegion::DrawUpArrow(SnBool yDisplay)
{
    if (m_pParent->GetDrawRect())
    {
        if (yDisplay && !m_bUpArrowHidden)
        {
            CBitmap *pOldBitmap;
	        EnterCriticalSection(&CSnHelp::m_DisplayCs);
            if (m_bUpArrowPressed)
                pOldBitmap = CSnHelp::m_hMemoryCDC.SelectObject(&CProcedureScreen::m_ArrowUpPressed);
            else
                pOldBitmap = CSnHelp::m_hMemoryCDC.SelectObject(&CProcedureScreen::m_ArrowUp);
			CSnHelp::m_hDisplayCDC->BitBlt(214+m_DisplayOffsetX, 266, 72, 62,
                &CSnHelp::m_hMemoryCDC, 0, 0, SRCCOPY);
            CSnHelp::m_hMemoryCDC.SelectObject(pOldBitmap);
	        LeaveCriticalSection(&CSnHelp::m_DisplayCs);
        }
        else
        {
            CSnHelp::m_hDisplayCDC->BitBlt( 214+m_DisplayOffsetX, 266, 72, 62,
                NULL, 0, 0, BLACKNESS);
        }
    }
}

void CProcedureRegion::InitDownArrowRect(void)
{
    m_tDownArrowRect.left = 114 + m_DisplayOffsetX;
    m_tDownArrowRect.top = 266;
    m_tDownArrowRect.right = 114 + 72 + m_DisplayOffsetX;
    m_tDownArrowRect.bottom = 266 + 62;
}


void CProcedureRegion::DrawDownArrow(SnBool yDisplay)
{
    if (m_pParent->GetDrawRect())
    {
        if (yDisplay && !m_bDownArrowHidden)
        {
            CBitmap * pOldBitmap;
	        EnterCriticalSection(&CSnHelp::m_DisplayCs);
            if (m_bDownArrowPressed)
                pOldBitmap = CSnHelp::m_hMemoryCDC.SelectObject( &CProcedureScreen::m_ArrowDownPressed);
            else
                pOldBitmap = CSnHelp::m_hMemoryCDC.SelectObject( &CProcedureScreen::m_ArrowDown);
            CSnHelp::m_hDisplayCDC->BitBlt( 114+m_DisplayOffsetX, 266, 72, 62,
                &CSnHelp::m_hMemoryCDC, 0, 0, SRCCOPY);
            CSnHelp::m_hMemoryCDC.SelectObject(pOldBitmap);
	        LeaveCriticalSection(&CSnHelp::m_DisplayCs);
        }
        else
        {
            CSnHelp::m_hDisplayCDC->BitBlt( 114+m_DisplayOffsetX, 266, 72, 62,
                NULL, 0, 0, BLACKNESS);
        }
    }
}

void CProcedureRegion::DrawConnector(SnBool yDisplay)
{
    if (m_pParent->GetDrawRect() && m_yFirstPaintOccured)
    {
        if (yDisplay)
        {
            CBitmap *pOldBitmap;
	        EnterCriticalSection(&CSnHelp::m_DisplayCs);
            pOldBitmap = CSnHelp::m_hMemoryCDC.SelectObject( & m_BitmapConnector);
            CSnHelp::m_hDisplayCDC->BitBlt( 148+m_DisplayOffsetX, 121, 104, 171,
                &CSnHelp::m_hMemoryCDC, 0, 0, SRCCOPY);
            CSnHelp::m_hMemoryCDC.SelectObject( pOldBitmap);
	        LeaveCriticalSection(&CSnHelp::m_DisplayCs);
        }
        else
        {
            CSnHelp::m_hDisplayCDC->BitBlt( 148+m_DisplayOffsetX, 121, 104, 171,
                NULL, 0, 0, BLACKNESS);
        }
    }
}

void CProcedureRegion::DrawTextMode(SnBool yDisplay)
{
    if (m_pParent->GetDrawRect() && m_yFirstPaintOccured)
    {
	    CSharedMemory mem;
        CFont *pFont;
        RECT tRect;
		// Device context

        if (m_dwPortType == PORT_TYPE_TOOL)
        {
            tRect.left = 100 + m_DisplayOffsetX;
            tRect.top = 233;
            tRect.right = 300 + m_DisplayOffsetX;
            tRect.bottom = 259;
            pFont = mem.m_Font16Normal;
        }
        else
        {
            tRect.left = 150 + m_DisplayOffsetX;
            tRect.top = 233;
            tRect.right = 250 + m_DisplayOffsetX;
            tRect.bottom = 259;
            pFont = mem.m_Font14Normal;
        }

        CSnHelp::m_hDisplayCDC->BitBlt( tRect.left, tRect.top, tRect.right - tRect.left,
            tRect.bottom - tRect.top, NULL, 0, 0, BLACKNESS);
        if (yDisplay)
            m_SnHelp.DrawTextOnDisplay(&tRect, pFont, m_csTextMode);
        else
            m_csTextMode = TEXT("");
    }
}

void CProcedureRegion::DrawTextMaxSpeed(SnBool yDisplay, SnBool ySmallFont)
{
    if (m_pParent->GetDrawRect() && m_yFirstPaintOccured)
    {
        RECT tRect = {250+m_DisplayOffsetX, 233, 310+m_DisplayOffsetX, 259};
	    CSharedMemory mem;

        CSnHelp::m_hDisplayCDC->BitBlt( tRect.left, tRect.top, tRect.right - tRect.left,
            tRect.bottom - tRect.top, NULL, 0, 0, BLACKNESS);
        if (yDisplay)
        {
            m_SnHelp.DrawTextOnDisplay(&tRect, ySmallFont ? mem.m_Font10Bold :mem.m_Font12Bold,
                m_csTextMaxSpeed);
        }
    }
}

void CProcedureRegion::InitSetSpeedDisplay(void)
{
    m_SetSpeedDisplay.iSetSpeedFiveDigitX = 3;
    m_SetSpeedDisplay.iSetSpeedFourDigitX = 23;
    m_SetSpeedDisplay.iSetSpeedThreeDigitX = 41;
    m_SetSpeedDisplay.iSetSpeedTwoDigitX = 59;
    m_SetSpeedDisplay.iSetSpeedOneDigitX = 77;

    m_SetSpeedDisplay.yRunning = FALSE;
    m_SetSpeedDisplay.yForceRedraw = FALSE;
}

void CProcedureRegion::DrawDigit(int iX, int iDigit, SnBool yRunning)
{
    CBitmap *pOldBitmap;
	EnterCriticalSection(&CSnHelp::m_DisplayCs);
    if (yRunning)
        pOldBitmap = CSnHelp::m_hMemoryCDC.SelectObject( &CProcedureScreen::m_hRunSetSpeedDigit[iDigit]);
    else
        pOldBitmap = CSnHelp::m_hMemoryCDC.SelectObject( &CProcedureScreen::m_hIdleSetSpeedDigit[iDigit]);
    CSnHelp::m_hDisplayCDC->BitBlt( 105 + m_DisplayOffsetX + iX, 136, 36, 83,
        &CSnHelp::m_hMemoryCDC, 0, 0, SRCCOPY);
    CSnHelp::m_hMemoryCDC.SelectObject( pOldBitmap);
	LeaveCriticalSection(&CSnHelp::m_DisplayCs);
}

void CProcedureRegion::UpdateSetSpeedDisplay(void)
{
    if (m_pParent->GetDrawRect() && m_yFirstPaintOccured)
    {
        SnBool yRunning = PortRunning();
        SnQByte qSetSpeed = m_wSetSpeed;
        SnBool yFill = FALSE;
        int iTenThousands;
        int iThousands;
        int iHundreds;
        int iTens;
        int iOnes;
        int iX;
        RECT tRect;

        if (yRunning != m_SetSpeedDisplay.yRunning)
        {
            m_SetSpeedDisplay.yRunning = yRunning;
            m_SetSpeedDisplay.yForceRedraw = TRUE;
        }

        for (iTenThousands = 0; qSetSpeed >= 10000; qSetSpeed -= 10000, iTenThousands++);
        for (iThousands = 0; qSetSpeed >= 1000; qSetSpeed -= 1000, iThousands++);
        for (iHundreds = 0; qSetSpeed >= 100; qSetSpeed -= 100, iHundreds++);
        for (iTens = 0; qSetSpeed >= 10; qSetSpeed -= 10, iTens++);
        iOnes = qSetSpeed;

        if (iTenThousands)
            iX = m_SetSpeedDisplay.iSetSpeedFiveDigitX;
        else if (iThousands)
            iX = m_SetSpeedDisplay.iSetSpeedFourDigitX;
        else if (iHundreds)
            iX = m_SetSpeedDisplay.iSetSpeedThreeDigitX;
        else if (iTens)
            iX = m_SetSpeedDisplay.iSetSpeedTwoDigitX;
        else
            iX = m_SetSpeedDisplay.iSetSpeedOneDigitX;

        // Force a redraw of digits if needed
        if (iX != m_SetSpeedDisplay.iLastX || m_SetSpeedDisplay.yForceRedraw) {
            m_SetSpeedDisplay.iLastTenThousands = -1;
            m_SetSpeedDisplay.iLastThousands = -1;
            m_SetSpeedDisplay.iLastHundreds = -1;
            m_SetSpeedDisplay.iLastTens = -1;
            m_SetSpeedDisplay.iLastOnes = -1;
        }

        // Redraw filler before and after the digits if needed
        if (iX > m_SetSpeedDisplay.iLastX || m_SetSpeedDisplay.yForceRedraw)
            yFill = TRUE;

        m_SetSpeedDisplay.iLastX = iX;

        // Fill box before the digits
        if (yFill) {
            tRect.left = 105 + m_DisplayOffsetX;
            tRect.top = 136;
            tRect.right = 105 + m_DisplayOffsetX + iX;
            tRect.bottom = 136 + 83;
            CSnHelp::m_hDisplayCDC->FillRect(&tRect,
                m_SetSpeedDisplay.yRunning ? &CProcedureScreen::m_hBrGreen : &CProcedureScreen::m_hBrBlack);
        }

        // Display digits
        if (m_SetSpeedDisplay.iLastX == m_SetSpeedDisplay.iSetSpeedFiveDigitX) {
            if (iTenThousands != m_SetSpeedDisplay.iLastTenThousands) {
                DrawDigit(iX, iTenThousands, m_SetSpeedDisplay.yRunning);
                m_SetSpeedDisplay.iLastTenThousands = iTenThousands;
            }
            iX += 36;
        }
        if (m_SetSpeedDisplay.iLastX <= m_SetSpeedDisplay.iSetSpeedFourDigitX) {
            if (iThousands != m_SetSpeedDisplay.iLastThousands) {
                DrawDigit(iX, iThousands, m_SetSpeedDisplay.yRunning);
                m_SetSpeedDisplay.iLastThousands = iThousands;
            }
            iX += 36;
        }
        if (m_SetSpeedDisplay.iLastX <= m_SetSpeedDisplay.iSetSpeedThreeDigitX) {
            if (iHundreds != m_SetSpeedDisplay.iLastHundreds) {
                DrawDigit(iX, iHundreds, m_SetSpeedDisplay.yRunning);
                m_SetSpeedDisplay.iLastHundreds = iHundreds;
            }
            iX += 36;
        }
        if (m_SetSpeedDisplay.iLastX <= m_SetSpeedDisplay.iSetSpeedTwoDigitX) {
            if (iTens != m_SetSpeedDisplay.iLastTens) {
                DrawDigit(iX, iTens, m_SetSpeedDisplay.yRunning);
                m_SetSpeedDisplay.iLastTens = iTens;
            }
            iX += 36;
        }
        if (iOnes != m_SetSpeedDisplay.iLastOnes) {
            DrawDigit(iX, iOnes, m_SetSpeedDisplay.yRunning);
            m_SetSpeedDisplay.iLastOnes = iOnes;
        }
        iX += 36;

        // Fill box after the digits
        if (yFill) {
            tRect.left = 105 + m_DisplayOffsetX + iX;
            tRect.top = 136;
            tRect.right = 105 + 190 + m_DisplayOffsetX;
            tRect.bottom = 136 + 83;
            CSnHelp::m_hDisplayCDC->FillRect( &tRect,
                m_SetSpeedDisplay.yRunning ? &CProcedureScreen::m_hBrGreen : &CProcedureScreen::m_hBrBlack);
        }

        m_SetSpeedDisplay.yForceRedraw = FALSE;
    }
}

