// FootSwitch.cpp: implementation of the Footswitch class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Control.h"

FootSwitch::FootSwitch(void)
{
	m_pControl = NULL;
}


FootSwitch::~FootSwitch(void)
{
}

void FootSwitch::Initialize(CControl* pControl)
{
	m_pControl = pControl;
	m_usFootSwitchVersionNum = 0;
	m_hGuiWnd = NULL;

	// Foot Pedal Status
	SetSystemDefaults();
    m_tFootPedalStatus.usType = TYPE_INVALID;
    m_tFootPedalStatus.usPrevType = TYPE_UNKNOWN_FOOTPEDAL;
	m_tFootPedalStatus.usFootAssignedPort = TYPE_INVALID;
	m_tFootPedalStatus.usPrevPortControl = TYPE_INVALID;
	m_tFootPedalStatus.bFootStuck = FALSE;
    m_tFootPedalStatus.bFootGood = FALSE;

	m_tFootswitchWarningStatus.bLowBattery = FALSE;
	m_tFootswitchWarningStatus.bStuckPedal = FALSE;	
	m_tFootswitchWarningStatus.bUnknownID = FALSE;

    m_bFootswitchEnabled = TRUE;
    m_bFastFootswitch = TRUE;
    m_sFootStatusDelay = RS485_FOOT_SETTLE_DELAY_MS;
	m_sFootConnectDebounce = FOOTSWITCH_CONNECT_DEBOUNCE_MS;
    m_bPollRs485 = TRUE;
	m_wSpUp = 0;
	m_wSpDn = 0;

	m_tFootswitchEvents.m_hLowBatteryEvent = NULL;
	m_tFootswitchEvents.m_hStuckPedalEvent = NULL;
	m_tFootswitchEvents.m_hUnknownIdEvent = NULL;
}

SnBool FootSwitch::SetSystemDefaults()
{
 	// Foot Pedal Defaults
	m_tFootPedalStatus.usMode = FOOT_MODE_VARIABLE;
	m_tFootPedalStatus.usForward = FOOT_FORWARD_RIGHT;
	m_tFootPedalStatus.usOverride = FOOT_HAND_OVERRIDE_ON;
	m_tFootPedalStatus.usPortControl = PORTA;
	return(TRUE);
}

void FootSwitch::ResetVariableFootPedals(void)
{
    int iCnt;

    for (iCnt = 0; iCnt < 3; iCnt++) {
        m_tFootPedalStatus.psPedalMax[iCnt] = 0;
        m_tFootPedalStatus.psPedalMin[iCnt] = VARIABLE_FOOTSWITCH_START_MIN;
        m_tFootPedalStatus.pwPedalPercent[iCnt] = 0;
    }
}

void FootSwitch::GetRS485DeviceVersion(SnByte& major, SnByte& minor,SnByte& build)
{
	if( m_tFootPedalStatus.usType == TYPE_485_ANALOG_FOOTPEDAL)
	{
		major = (m_usFootSwitchVersionNum >> 8) & 0x3;
		minor = (m_usFootSwitchVersionNum >> 4) & 0xF;
		build = m_usFootSwitchVersionNum & 0xF;
	}
	else
	{
		major = 0x00;
		minor = 0x00;
		build = 0x00;
	}

}


SnBool FootSwitch::ReadVariableFootPedals(void)
{
	SnWord usFootMode;
    SnSWord psPedalADC[3];
    SnBool yStatus;
    int iCnt;
	
	if (!m_pControl->GetFootMode( m_tFootPedalStatus.usPortControl, usFootMode))
		return FALSE;

	yStatus = m_pControl->ReadDsp(MC_ANALOG_AVG10, 3, (SnWord *)(psPedalADC));
	if (!yStatus)
		return FALSE;

    for (iCnt = 0; iCnt < 3; iCnt++) {
        short sPedalStart = m_tFootPedalStatus.psPedalMax[iCnt] - VARIABLE_FOOTSWITCH_DEAD_BAND;

	    if (psPedalADC[iCnt] < sPedalStart) {
            if (usFootMode == MOTOR_OFF) {
                m_tFootPedalStatus.psPedalMin[iCnt] = VARIABLE_FOOTSWITCH_START_MIN;
                m_tFootPedalStatus.pwPedalPercent[iCnt] = 100;
            } else {
                float fPercent;

                // Save new Pedal MIN
		        if (psPedalADC[iCnt] < m_tFootPedalStatus.psPedalMin[iCnt])
			        m_tFootPedalStatus.psPedalMin[iCnt] = psPedalADC[iCnt];

                // Set Pedal Percent Depression
                fPercent = (float)(sPedalStart - psPedalADC[iCnt]) / (float)(sPedalStart - m_tFootPedalStatus.psPedalMin[iCnt]);
                m_tFootPedalStatus.pwPedalPercent[iCnt] = (SnWord)((fPercent * 100.0f) + 0.5f);

                // On/Off Footswitch is either zero or 100 percent
                if (m_tFootPedalStatus.usMode == FOOT_MODE_ON_OFF)
                    m_tFootPedalStatus.pwPedalPercent[iCnt] = 100;
            }
        } else {
		    // Save new Pedal MAX
            if (psPedalADC[iCnt] > m_tFootPedalStatus.psPedalMax[iCnt] || usFootMode != MOTOR_OFF)
			    m_tFootPedalStatus.psPedalMax[iCnt] = psPedalADC[iCnt];

            // Stop
            m_tFootPedalStatus.pwPedalPercent[iCnt] = 0;
	    }
    }

    return TRUE;
}

SnBool FootSwitch::DebounceSpeedUpDown(SnBool ySpeedUp, SnBool ySpeedDown, SnWord wPrevMode)
{
	SnBool yWindowLock = FALSE;

	if ( ySpeedUp && ySpeedDown )
	{
		if((m_wSpUp >= SPEED_UP_DOWN_DELAY_MS ||
			m_wSpDn >= SPEED_UP_DOWN_DELAY_MS) && m_hGuiWnd)
			PostMessage(m_hGuiWnd, WM_REMOTE_CMD, (WPARAM)KEY_STOP_SET_SPEED,(LPARAM)0);
		m_wSpUp = 1;
        m_wSpDn = 1;
		yWindowLock = TRUE;
	}
	else if (wPrevMode == MOTOR_WINDOW_LOCK && (ySpeedUp || ySpeedDown))
	{
 		yWindowLock = TRUE;
		if (ySpeedUp)
		{
            if (m_wSpUp < SPEED_UP_DOWN_DELAY_LONG_MS)
                m_wSpUp += m_pControl->GetDelayPeriodMs();
			else
 				yWindowLock = FALSE;
			m_wSpDn = 0;
		}
        else if (ySpeedDown)
        {
            if (m_wSpDn < SPEED_UP_DOWN_DELAY_LONG_MS)
                m_wSpDn += m_pControl->GetDelayPeriodMs();
			else
 				yWindowLock = FALSE;
			m_wSpUp = 0;
		}
	}

	if (!yWindowLock)
	{
        if (m_wSpUp >= 1 && m_wSpDn >= 1)
        {
            m_wSpUp = 0;
            m_wSpDn = 0;
        }

        if (ySpeedUp)
        {
            if (m_wSpUp < SPEED_UP_DOWN_DELAY_MS)
                m_wSpUp += m_pControl->GetDelayPeriodMs();
            else if (m_tFootPedalStatus.bFootGood && m_hGuiWnd)
				PostMessage(m_hGuiWnd, WM_REMOTE_CMD, (WPARAM)KEY_SET_SPEED_UP,(LPARAM)0);
            m_wSpDn = 0;
        }
        else if (ySpeedDown)
        {
            if (m_wSpDn < SPEED_UP_DOWN_DELAY_MS)
                m_wSpDn += m_pControl->GetDelayPeriodMs();
            else if (m_tFootPedalStatus.bFootGood && m_hGuiWnd)
				PostMessage(m_hGuiWnd, WM_REMOTE_CMD, (WPARAM)KEY_SET_SPEED_DOWN,(LPARAM)0);
            m_wSpUp = 0;
        }
        else
        {
            if((m_wSpUp >= SPEED_UP_DOWN_DELAY_MS ||
                m_wSpDn >= SPEED_UP_DOWN_DELAY_MS) && m_hGuiWnd)
			    PostMessage(m_hGuiWnd, WM_REMOTE_CMD, (WPARAM)KEY_STOP_SET_SPEED,(LPARAM)0);
            m_wSpUp = 0;
            m_wSpDn = 0;
        }
    }

	return yWindowLock;
}

// Function:	ReadFootPedalStatus
// Purpose:		Checks for foot pedal status ( IN/OUT). If the foot pedal is
//				detected the direction status is acquired.
//
// Parameters:	None
//
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool FootSwitch::ReadFootPedalStatus()
{
	if (!m_bFootswitchEnabled)
		return(FALSE);

	SnWord wDigitalStatePortD = 0;
	SnBool bStatus = FALSE;
	SnByte bPumpCmd= 0;
	
	// Check to see if we have a foot pedal plugged in, Spd Up status
	bStatus =  m_pControl->ReadDsp(MC_DIGITAL_STATE_DATA_D, 1, &wDigitalStatePortD);
	if( !bStatus)
		return FALSE;
	
    // If Wired Footswitch is plugged into Port A (0x0001)
	if ((wDigitalStatePortD & 0x0001))
    {
		SnWord wStatus = 0;
		SnBool bForward = FALSE;
		SnBool bReverse = FALSE;
		SnBool bOsc	= FALSE;
		SnBool bWindowLock = FALSE;
		SnBool bOverride = FALSE;
		SnWord wForwardPercent = 0;
		SnWord wReversePercent = 0;
		SnWord wOscPercent = 0;
		SnBool checkBypassDebounce = FALSE;
		
		// Figure out what mode to use
		SnWord wPrevMode = MOTOR_OFF;
		SnBool bPowerInstr = FALSE;
		

		// More than one pedal was pressed at the same time use previous mode
		m_pControl->GetFootMode( m_tFootPedalStatus.usPortControl, wPrevMode);
		if (wPrevMode == MOTOR_OSCILLATE_1 || wPrevMode == MOTOR_OSCILLATE_2)
		{
			wPrevMode = MOTOR_OSCILLATE;
		}
		bPowerInstr = m_pControl->IsPoweredInstrument(m_tFootPedalStatus.usPortControl);
		
        bStatus = m_pControl->ReadDsp(MC_HALLBUSB_DEVICE_EXIST, 1, &wStatus);
        if (!bStatus)
            return FALSE;

		if (m_sFootConnectDebounce > 0)
		{
			m_sFootConnectDebounce -= m_pControl->GetDelayPeriodMs();
			if (m_tFootPedalStatus.usType != TYPE_485_ANALOG_FOOTPEDAL)
			{
				if (m_sFootConnectDebounce <= 0)
				{
					m_sFootConnectDebounce = 0;
					m_tFootPedalStatus.bFootStuck = TRUE;
				}
			}
			else
				checkBypassDebounce = TRUE;
		}

        if ((wStatus & HALL_FOOTPEDAL_ANALOG) || m_tFootPedalStatus.usType == TYPE_ANALOG_FOOTPEDAL)
        {
			checkBypassDebounce = FALSE;
			if(m_tFootPedalStatus.usType != TYPE_ANALOG_FOOTPEDAL)
			{
				ResetVariableFootPedals();
				m_bPollRs485 = FALSE;
				m_sFootConnectDebounce = 0;
				m_tFootPedalStatus.usType = TYPE_ANALOG_FOOTPEDAL;
			}

			if (CPort::GetControlsEnabled())
            {
                if (ReadVariableFootPedals() == FALSE)
                    return FALSE;
                
                // Read Forward Status
                wForwardPercent = m_tFootPedalStatus.pwPedalPercent[0];
                if (wForwardPercent > 0 ) {
                    bForward = TRUE;
                    if (m_tFootPedalStatus.usMode == FOOT_MODE_VARIABLE) {
                        bOverride = TRUE;
                    }
                }
                
                // Read Reverse Status
                wReversePercent = m_tFootPedalStatus.pwPedalPercent[1];
                if (wReversePercent > 0 ) {
                    bReverse = TRUE;
                    if (m_tFootPedalStatus.usMode == FOOT_MODE_VARIABLE) {
                        bOverride = TRUE;
                    }
                }
                
                // Read Oscillate status
                wOscPercent = m_tFootPedalStatus.pwPedalPercent[2];
                if (wOscPercent > 0 ) {
                    bOsc = TRUE;
                    if (m_tFootPedalStatus.usMode == FOOT_MODE_VARIABLE) {
                        bOverride = TRUE;
                    }
                }
                
                // Read Window Lock status
                bStatus = m_pControl->ReadDsp(MC_HALLBUSB_DEVICE_ACTIVE, 1, &wStatus);
                if (!bStatus) 
                    return FALSE;
                
                if ((wStatus & HALL_FOOT_CTL_WINDOW_LOCK))
                    bWindowLock = TRUE;
            }
        } 
        else
        {
            if (m_sFootStatusDelay > 0)
                m_sFootStatusDelay -= m_pControl->GetDelayPeriodMs();

            if (m_sFootStatusDelay <= 0 && m_bPollRs485)
            {
			    // Check for DII FootSwitch
                SnWord wRequest = FOOT_CMD(SERIAL_CMD_DEV_TYPE);
				
                if (m_pControl->SendSerialRequest(&wRequest, 4))
                {
                    m_sFootStatusDelay = 0;
                    if ((wRequest & 0x3ff) == 0x1)
                    {
						if (m_tFootPedalStatus.usType != TYPE_485_ANALOG_FOOTPEDAL)
						{
							m_tFootPedalStatus.usType = TYPE_485_ANALOG_FOOTPEDAL;
							m_bFastFootswitch = FALSE;
							m_bPollRs485 = FALSE;
	 						m_sFootConnectDebounce = FOOTSWITCH_CONNECT_DEBOUNCE_MS;
			                ResetVariableFootPedals();
							m_tFootPedalStatus.bFootStuck = TRUE;
							checkBypassDebounce = TRUE;
                            m_usFootSwitchVersionNum = 0; // Read the version during debounce
						}

                     }
                    else
                    {
                        m_tFootPedalStatus.usType = TYPE_INVALID;
						m_bFastFootswitch = TRUE;
                        // Footswitch detected but we don't know what it is
                        if (!m_tFootswitchWarningStatus.bUnknownID)
                        {
                            m_tFootswitchWarningStatus.bUnknownID = TRUE;
                            SetEvent(m_tFootswitchEvents.m_hUnknownIdEvent);
                        }
                        return FALSE;
                    }
                }
                else
                {
					if (m_tFootPedalStatus.usType != TYPE_DIGITAL_FOOTPEDAL)
					{
						m_tFootPedalStatus.usType = TYPE_DIGITAL_FOOTPEDAL;
						m_bFastFootswitch = TRUE;
						if (m_sFootConnectDebounce < FOOTSWITCH_CONNECT_DEBOUNCE_MS)
							m_sFootConnectDebounce = FOOTSWITCH_CONNECT_DEBOUNCE_MS;
						ResetVariableFootPedals();
						checkBypassDebounce = FALSE;
					}

                    wRequest = DISABLE_FOOT_CMD;
                    m_pControl->SendSerialRequest(&wRequest, 1);
                }
            }
            
            if( m_tFootPedalStatus.usType == TYPE_DIGITAL_FOOTPEDAL)
            {	
				SnWord psPedalADC[3];
				SnBool bSpeedUp = FALSE;
				SnBool bSpeedDown = FALSE;
				SnSWord swAverage = 0;

				// Read Forward Status
				bStatus = m_pControl->ReadDsp(MC_ANALOG_AVG10, 3, psPedalADC);
				if( !bStatus)
					return FALSE;
				
				swAverage = (SnSWord)psPedalADC[0];
				if( swAverage <= DIGITAL_ON_OFF_THRESHOLD_VALUE)
					bForward = TRUE;
            
				// Read Reverse Status
				swAverage = (SnSWord)psPedalADC[1];
				if( swAverage <= DIGITAL_ON_OFF_THRESHOLD_VALUE)
					bReverse = TRUE;
            
				// Read Window Lock/SpeedDown status
				swAverage = (SnSWord)psPedalADC[2];
				if( swAverage <= DIGITAL_LOCK_THRESHOLD_VALUE)
					bSpeedDown = TRUE;

				bSpeedUp = (wDigitalStatePortD & 0x0004) ? TRUE:FALSE;
				
				if( m_bPollRs485)
				{
					if( (bForward || bReverse || bSpeedDown) )
					{
						m_bFastFootswitch = TRUE;
						m_bPollRs485 = FALSE;
						m_sFootConnectDebounce = 0;
					}
					else 
					{
						// Check if this Speed Up button press the presence of a 100K ohm
						// resistor on one of the analog signals of a RS485 footswitch.
						// If the resistor is there i.e. RS485 footswitch - psPedalMax[0] ~0xC34
						// if the resistor is not there i.e. digital footswitch - psPedalMax[0] ~0xC98.
						// This test is not sufficient for final determination of the 
						// footswitch type digital vs RS485.
						ReadVariableFootPedals();
						if (m_tFootPedalStatus.psPedalMax[0] < 0x0c70)
							m_sFootConnectDebounce = FOOTSWITCH_CONNECT_DEBOUNCE_MS;
						if (m_sFootConnectDebounce > 0)
							bSpeedUp = FALSE;
					}
				}
				
				if( bForward && bReverse)
				{
					bOsc = TRUE;
					bForward = FALSE;
					bReverse = FALSE;
				}
							
 				bWindowLock = DebounceSpeedUpDown(bSpeedUp, bSpeedDown, wPrevMode);
			}
            else if( m_tFootPedalStatus.usType == TYPE_485_ANALOG_FOOTPEDAL)
            {
				SnBool byPassDebounce = FALSE;
				if (checkBypassDebounce)
				{
					SnWord wRequest = FOOT_CMD(SERIAL_CMD_VERS);
					// Get the Software version number
					if (m_pControl->SendSerialRequest(&wRequest, 4))
					{
						byPassDebounce = TRUE;
						if (m_usFootSwitchVersionNum == 0)
                            m_usFootSwitchVersionNum = wRequest;
					}
					else
					{
						byPassDebounce = FALSE;
						m_tFootPedalStatus.bFootStuck = TRUE;
						m_sFootConnectDebounce = FOOTSWITCH_CONNECT_DEBOUNCE_MS;
					}
				}

				SnWord pwRequests[4] = {0};
				if( m_sFootConnectDebounce <= 0 || byPassDebounce )
				{
					pwRequests[0] = FOOT_CMD(SERIAL_CMD_REQ_2);
					switch (wPrevMode)
					{
					case MOTOR_REVERSE:
						// Get the Reverse Foot Pedal State for Serial Device, (0 - 100 in percent)
						if (m_tFootPedalStatus.usForward == FOOT_FORWARD_LEFT)
							pwRequests[1] = FOOT_CMD(SERIAL_CMD_REQ_5);
						else
							pwRequests[1] = FOOT_CMD(SERIAL_CMD_REQ_3);
                    
						if (m_pControl->SendSerialRequests(2, pwRequests, 6) == FALSE)
						{
							pwRequests[0] = 0; 
							pwRequests[1] = 0; 
						}

						wReversePercent = pwRequests[1] & 0x3ff;
						break;

					case MOTOR_FORWARD:
						 // Get the Forward Foot Pedal State for Serial Device, (0 - 100 in percent)
						if (m_tFootPedalStatus.usForward == FOOT_FORWARD_LEFT)
							pwRequests[1] = FOOT_CMD(SERIAL_CMD_REQ_3);
						else
							pwRequests[1] = FOOT_CMD(SERIAL_CMD_REQ_5);
                    
						if (m_pControl->SendSerialRequests(2, pwRequests, 6) == FALSE)
						{
							pwRequests[0] = 0; 
							pwRequests[1] = 0; 
						}

						wForwardPercent = pwRequests[1] & 0x3ff;
						break;

					case MOTOR_OSCILLATE:
						// Get the Oscillate Foot Pedal State for Serial Device, (0 - 100 in percent)
						pwRequests[1] = FOOT_CMD(SERIAL_CMD_REQ_4);
                    
						if (m_pControl->SendSerialRequests(2, pwRequests, 6) == FALSE)
						{
							pwRequests[0] = 0; 
							pwRequests[1] = 0; 
						}
                    
						wOscPercent = pwRequests[1] & 0x3ff;
						break;

					case MOTOR_WINDOW_LOCK:                
						if (m_pControl->SendSerialRequest(pwRequests, 6) == FALSE)
						{
							pwRequests[0] = 0; 
						}
						break;

					default:
						// Get the Forward, Reverse and Oscillate Foot Pedal States for Serial Device, (0 - 100 in percent)
						pwRequests[1] = FOOT_CMD(SERIAL_CMD_REQ_3);
						pwRequests[2] = FOOT_CMD(SERIAL_CMD_REQ_4);
						pwRequests[3] = FOOT_CMD(SERIAL_CMD_REQ_5);
                    
						if (m_pControl->SendSerialRequests(4, pwRequests, 6) == FALSE)
						{
							for (int ii = 0; ii <4; ii++)
								pwRequests[ii] = 0;
						}
                    
						if (m_tFootPedalStatus.usForward == FOOT_FORWARD_LEFT)
						{
							wForwardPercent = pwRequests[1] & 0x3ff;
							wReversePercent = pwRequests[3] & 0x3ff;
						}
						else
						{
							wForwardPercent = pwRequests[3] & 0x3ff;
							wReversePercent = pwRequests[1] & 0x3ff;
						}
						wOscPercent = pwRequests[2] & 0x3ff;
						break;
					}
                
					// Get the Switch State for Serial Device, (0 - Released, 1 - Pressed)
					//	   Bit 0 - Left Switch State
					//	   Bit 1 - Middle Switch State
					//	   Bit 2 - Right Switch State
					if( pwRequests[0] & 1)
						bPumpCmd = REMOTE_TOGGLE_LAVAGE;
					if( pwRequests[0] & 4)
						bWindowLock = TRUE;

					// Verify that the data from the footswitch is within 0 - 100%
					if (wForwardPercent > 100)
						wForwardPercent = 0;
					if (wReversePercent > 100)
						wReversePercent = 0;
					if (wOscPercent > 100)
						wOscPercent = 0;
				}
                
                if( wPrevMode == MOTOR_REVERSE || wPrevMode == MOTOR_OFF)
                {
                    if (wReversePercent >= PERCENT_ON_VALUE && m_tFootPedalStatus.usMode == FOOT_MODE_ON_OFF)
                        bReverse = TRUE;
                    else if( wReversePercent >= PERCENT_START_THRESHOLD &&
                        m_tFootPedalStatus.usMode == FOOT_MODE_VARIABLE)
                    {
                        bReverse = TRUE;
                        bOverride = TRUE;
                    }
                    else
                    {
                        wPrevMode = MOTOR_OFF;
                    }
                }
                
                if( wPrevMode == MOTOR_FORWARD || wPrevMode == MOTOR_OFF)
                {
                    if( wForwardPercent >= PERCENT_ON_VALUE && m_tFootPedalStatus.usMode == FOOT_MODE_ON_OFF)
                        bForward = TRUE;
                    else if( wForwardPercent >= PERCENT_START_THRESHOLD &&
						m_tFootPedalStatus.usMode == FOOT_MODE_VARIABLE)
                    {
                        bForward = TRUE;
                        bOverride = TRUE;
                    }
                    else
                    {
                        wPrevMode = MOTOR_OFF;
                    }
                }
                
                if( wPrevMode == MOTOR_OSCILLATE || wPrevMode == MOTOR_OFF)
                {
                    if( wOscPercent >= PERCENT_ON_VALUE && m_tFootPedalStatus.usMode == FOOT_MODE_ON_OFF)
                        bOsc = TRUE;
                    else if( wOscPercent >= PERCENT_START_THRESHOLD &&
                        m_tFootPedalStatus.usMode == FOOT_MODE_VARIABLE)
                    {
                        bOsc = TRUE;
                        bOverride = TRUE;
                    }
                    else
                    {
                        wPrevMode = MOTOR_OFF;
                    }
                }
            }
        }

		SnWord wPercent = 0; 
		SnWord wMode = MOTOR_OFF;
		
		if( bOsc && !bPowerInstr)
		{
			if( wPrevMode == MOTOR_OSCILLATE || (!bWindowLock))
			{
				wMode = MOTOR_OSCILLATE;
				wPercent = wOscPercent;
			}
		}
		if( bForward)
		{
			if( wPrevMode == MOTOR_FORWARD || (!bWindowLock && !bReverse && !bOsc))
			{
				wMode = MOTOR_FORWARD;
				wPercent = wForwardPercent;
			}
		}
		if( bReverse)
		{
			if( wPrevMode == MOTOR_REVERSE || (!bWindowLock && !bForward && !bOsc))
			{
				wMode = MOTOR_REVERSE;
				wPercent = wReversePercent;
			}
		}
		if( bWindowLock && !bPowerInstr)
		{
			if( wPrevMode == MOTOR_WINDOW_LOCK || (!bForward && !bReverse && !bOsc))
			{
				wMode = MOTOR_WINDOW_LOCK;
				wPercent = 0;
			}
		}
		
		// Assign foot pedal to port
		if( (m_tFootPedalStatus.usPortControl == PORTA) || (m_tFootPedalStatus.usPortControl == PORTB))
		{
			m_tFootPedalStatus.usFootAssignedPort = m_tFootPedalStatus.usPortControl;
		}
		else
		{
			m_tFootPedalStatus.usFootAssignedPort = TYPE_INVALID;
		}
		
		// Check for stuck pedals
		if( m_tFootPedalStatus.usType != m_tFootPedalStatus.usPrevType)
		{
			if (bForward || bReverse || bOsc || m_wSpUp ||
				m_wSpDn || bWindowLock || bPumpCmd)
			{
				// Pedal or switch down when footswitch is plugged in
				m_tFootPedalStatus.bFootStuck = TRUE;
				m_tFootPedalStatus.bFootGood = FALSE;
				
				if (!m_tFootswitchWarningStatus.bStuckPedal)
				{
					m_tFootswitchWarningStatus.bStuckPedal = TRUE;
					SetEvent(m_tFootswitchEvents.m_hStuckPedalEvent);
				}
				return FALSE;
			}
		}
		else if( m_tFootPedalStatus.bFootStuck)
		{
			if( !bForward && !bReverse && !bOsc && bPumpCmd == 0 &&
				!m_wSpUp && !m_wSpDn && !bWindowLock)
			{
				m_tFootPedalStatus.bFootStuck = FALSE;
				// Clear the stuck pedal warning if it's set
				if(m_tFootswitchWarningStatus.bStuckPedal)
				{
					m_tFootswitchWarningStatus.bStuckPedal = FALSE;
					SetEvent(m_tFootswitchEvents.m_hStuckPedalEvent);
					m_tFootPedalStatus.bFootGood = TRUE;
				}
			}
			else
			{
				if(!m_tFootswitchWarningStatus.bStuckPedal)
				{
					m_tFootswitchWarningStatus.bStuckPedal = TRUE;
					SetEvent(m_tFootswitchEvents.m_hStuckPedalEvent);
					m_tFootPedalStatus.bFootGood = FALSE;
				}
				return FALSE;
			}
		}
		else
			m_tFootPedalStatus.bFootGood = TRUE;
		
		if (CPort::GetControlsEnabled())
		{
			m_pControl->NotifyPump(bPumpCmd);
			m_pControl->FootPedalStatusUpdate( m_tFootPedalStatus.usFootAssignedPort,
				bOverride, wPercent, wMode);
		}
	}
	else
	{
        // Disable Rs485 & Enable Pullups
        SnWord wRequest = DISABLE_FOOT_CMD;
        m_pControl->SendSerialRequest(&wRequest, 1);
        if(m_tFootPedalStatus.usFootAssignedPort != TYPE_INVALID)
		{
	        m_tFootPedalStatus.usFootAssignedPort = TYPE_INVALID;
			m_tFootPedalStatus.usPrevType = TYPE_UNKNOWN_FOOTPEDAL;
		}
		m_tFootPedalStatus.usType = TYPE_INVALID;
		m_tFootPedalStatus.bFootGood = FALSE;
 		m_tFootPedalStatus.bFootStuck = FALSE;

        m_pControl->SetFootMode(PORTA, MOTOR_OFF);
        m_pControl->SetFootMode(PORTB, MOTOR_OFF);
		m_sFootStatusDelay = RS485_FOOT_SETTLE_DELAY_MS;
		m_sFootConnectDebounce = FOOTSWITCH_CONNECT_DEBOUNCE_MS;
        m_bPollRs485 = TRUE;
		m_wSpUp = 0;
		m_wSpDn = 0;
		m_bFastFootswitch = TRUE;


        // If the Unknown footswitch ID is set, clear it
		if(m_tFootswitchWarningStatus.bUnknownID)
		{
			m_tFootswitchWarningStatus.bUnknownID = FALSE;
			SetEvent(m_tFootswitchEvents.m_hUnknownIdEvent);
		}
		// If the foot pedal stuck warning is set, clear it
		if(m_tFootswitchWarningStatus.bStuckPedal)
		{
			m_tFootswitchWarningStatus.bStuckPedal = FALSE;
			SetEvent(m_tFootswitchEvents.m_hStuckPedalEvent);
		}
		
		m_pControl->NotifyPump(bPumpCmd);
	}

	return TRUE;
}


// Function:	UpdatePortStatus
// Purpose:		Informs Gui and remote connections of a change in port status
//
// Parameters:	None
//
// Return:		TRUE indicates Success, FALSE indicates Failure
//
void FootSwitch::UpdatePortStatus()
{

	// Foot Pedal status has changed
	// The footswitch type has changed or it has been disconnected
	if (m_tFootPedalStatus.usType != m_tFootPedalStatus.usPrevType)
	{
		m_tFootPedalStatus.usPrevType = m_tFootPedalStatus.usType;
 	
		PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
                   (WPARAM)MSG_UPDATE_FOOT_STATUS, (LPARAM)0);
    }

	// Foot Pedal Port Control status has changed
	// It has been assigned to control a different MDU/Tool Port
	if (m_tFootPedalStatus.usPortControl != m_tFootPedalStatus.usPrevPortControl)
	{
		// Resend footswitch warnings
		if(m_tFootswitchWarningStatus.bUnknownID)
		{
		// If the Unknown footswitch ID is set, resend it
			SetEvent(m_tFootswitchEvents.m_hUnknownIdEvent);
		}
		
		if(m_tFootswitchWarningStatus.bStuckPedal)
		{
			// If the foot pedal stuck warning is set, resend it
			SetEvent(m_tFootswitchEvents.m_hStuckPedalEvent);
		}
		
		m_tFootPedalStatus.usPrevPortControl = m_tFootPedalStatus.usPortControl;
 	
		PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
                   (WPARAM)MSG_UPDATE_FOOT_STATUS, (LPARAM)0);
    }

}
