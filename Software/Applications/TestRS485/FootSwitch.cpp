// FootSwitch.cpp: implementation of the Footswitch class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Control.h"


#define COMMAND_RESPONSE_TEST 0

FootSwitch::FootSwitch(void):
	m_cDuration(128)
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
    m_tFootPedalStatus.usType = TYPE_INVALID;
    m_tFootPedalStatus.usPrevType = TYPE_UNKNOWN_FOOTPEDAL;
	m_tFootPedalStatus.usFootAssignedPort = TYPE_INVALID;
	m_tFootPedalStatus.usPortControl = TYPE_INVALID;
	m_tFootPedalStatus.usPrevPortControl = TYPE_INVALID;
	m_tFootPedalStatus.bFootStuck = FALSE;
    m_tFootPedalStatus.bFootGood = FALSE;
	m_tFootPedalStatus.dCommandDuration = 0.0;
	m_tFootPedalStatus.qErrorCount = 0;
	m_tFootPedalStatus.qConnectDuration =0;
	m_tFootPedalStatus.bInSoftwareReset = FALSE;
	m_tFootPedalStatus.bInCalibration = FALSE;
	m_tFootPedalStatus.bInInvalidCommandTest = FALSE;

	ResetVariableFootPedals();

	m_tFootPedalStatus.wButtonStatus.w = 0;

	m_tFootswitchWarningStatus.bLowBattery = FALSE;
	m_tFootswitchWarningStatus.bStuckPedal = FALSE;	
	m_tFootswitchWarningStatus.bUnknownID = FALSE;

    m_bFootswitchEnabled = TRUE;
    m_sFootStatusDelay = RS485_FOOT_SETTLE_DELAY_MS;
	m_sFootConnectDebounce = FOOTSWITCH_CONNECT_DEBOUNCE_MS;
    m_bPollRs485 = TRUE;
 	m_bFastFootswitch = FALSE;
	m_wSpUp = 0;
	m_wSpDn = 0;
  	m_cDuration.Initialize();
	m_bDeviceConnected = FALSE;
	m_qConnectStartTime = 0;
	m_bSoftwareResetRequest = FALSE;
	m_StartStopCalibration = NULL;
  
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

SnBool FootSwitch::StartStopSoftwareReset(SnWord wStartStop)
{
	if(wStartStop ==0)
	{
		m_bSoftwareResetRequest = FALSE;
		m_tFootPedalStatus.bInSoftwareReset = FALSE;
		return TRUE;
	}
	else if (m_tFootPedalStatus.usType >= TYPE_DIGITAL_FOOTPEDAL && 
		m_tFootPedalStatus.usType <=TYPE_485_ANALOG_FOOTPEDAL) 
	{
		m_bSoftwareResetRequest = TRUE;
		return TRUE;
	}
	return FALSE;
}

SnBool FootSwitch::StartStopInvalidCommandTest(SnWord wStartStop)
{
	if(wStartStop ==0)
	{
		m_tFootPedalStatus.bInInvalidCommandTest = FALSE;
		return TRUE;
	}
	else if (m_tFootPedalStatus.usType ==TYPE_485_ANALOG_FOOTPEDAL) 
	{
		m_tFootPedalStatus.bInInvalidCommandTest = TRUE;
		return TRUE;
	}
	return FALSE;
}

SnBool FootSwitch::StartStopCalibration(SnWord wStartStop)
{
	if(m_tFootPedalStatus.usType == TYPE_485_ANALOG_FOOTPEDAL)
	{
		if(wStartStop == 1)
		{
			m_StartStopCalibration = START_CALIBRATION;
		}
		else 
		{
			m_StartStopCalibration = STOP_CALIBRATION;
		}
		return TRUE;
	}
	return FALSE;
}

void FootSwitch::CalcConnectTime(void)
{
	SnQByte qStopTime = GetTickCount();

	if(qStopTime >= m_qConnectStartTime)
		m_tFootPedalStatus.qConnectDuration = qStopTime - m_qConnectStartTime;
	else
		m_tFootPedalStatus.qConnectDuration = (MAXDWORD - m_qConnectStartTime) + qStopTime;
	m_tFootPedalStatus.bInCalibration = FALSE;
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
	SnBool bStartSoftwareReset = FALSE;
	
	// Check to see if we have a foot pedal plugged in, Spd Up status
	bStatus =  m_pControl->ReadDsp(MC_DIGITAL_STATE_DATA_D, 1, &wDigitalStatePortD);
	if( !bStatus)
		return FALSE;

	if(m_bSoftwareResetRequest)
	{
		wDigitalStatePortD &= ~0x0001;
		if (m_tFootPedalStatus.usType == TYPE_485_ANALOG_FOOTPEDAL)
		{
			SnWord pwResult= FOOT_CMD(SERIAL_CMD_REQ_13);
			m_pControl->SendSerialRequest(&pwResult, 4, m_cDuration);
		}
		m_tFootPedalStatus.bInSoftwareReset = TRUE;
		bStartSoftwareReset = TRUE;
		m_tFootPedalStatus.bInCalibration = FALSE;
		m_StartStopCalibration = 0;
		m_tFootPedalStatus.bInInvalidCommandTest = FALSE;
	}


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

		if (m_bDeviceConnected == FALSE)
		{
			m_bDeviceConnected = TRUE;
			m_qConnectStartTime = GetTickCount();
		}

		if (m_tFootPedalStatus.bInInvalidCommandTest)
		{
			InvalidCommandTest();
		}

		if(m_StartStopCalibration)
		{
			if (m_tFootPedalStatus.usType == TYPE_485_ANALOG_FOOTPEDAL)
			{
				SnWord pwResult = FOOT_CMD(SERIAL_CMD_REQ_14);
				
				switch(m_StartStopCalibration)
				{
				case START_CALIBRATION:
					pwResult = FOOT_CMD(SERIAL_CMD_REQ_14);
					if (m_pControl->SendSerialRequest(&pwResult, 6))
					{
						m_tFootPedalStatus.bInCalibration = TRUE;
						m_StartStopCalibration = NULL;
					}
					break;
				case STOP_CALIBRATION:
					pwResult = FOOT_CMD(SERIAL_CMD_REQ_14);
					if (m_pControl->SendSerialRequest(&pwResult, 6))
					{
						m_StartStopCalibration = 10;
					}
					break;
				default:
					if (m_StartStopCalibration <= 10)
						m_StartStopCalibration--;
					else
						m_StartStopCalibration = 10;
					if (m_StartStopCalibration <= 5)
					{
						pwResult = FOOT_CMD(SERIAL_CMD_DEV_TYPE);
						if (m_pControl->SendSerialRequest(&pwResult, 4) ||
							m_StartStopCalibration == 0)
						{
							m_tFootPedalStatus.bInCalibration = FALSE;
							m_StartStopCalibration = NULL;
						}
					}
					break;
				}
				return TRUE;
			}
			else
			{
				m_StartStopCalibration = NULL;
				m_tFootPedalStatus.bInCalibration = FALSE;
			}
		}
		
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
 				m_bFastFootswitch = TRUE;
				m_sFootConnectDebounce = 0;
				m_tFootPedalStatus.usType = TYPE_ANALOG_FOOTPEDAL;
				m_tFootPedalStatus.bInSoftwareReset = FALSE;
				CalcConnectTime();
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
				{
                    bWindowLock = TRUE;
					m_tFootPedalStatus.wButtonStatus.right = 1;
				}
				else
					m_tFootPedalStatus.wButtonStatus.right = 0;
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
 				m_bFastFootswitch = FALSE;
				
                if (m_pControl->SendSerialRequest(&wRequest, 4))
                {
					m_sFootStatusDelay = 0;
                    if ((wRequest & 0x3ff) == 0x1)
                    {
						if (m_tFootPedalStatus.usType != TYPE_485_ANALOG_FOOTPEDAL)
						{
							m_tFootPedalStatus.usType = TYPE_485_ANALOG_FOOTPEDAL;
	 						m_sFootConnectDebounce = FOOTSWITCH_CONNECT_DEBOUNCE_MS;
							m_bPollRs485 = FALSE;
			                ResetVariableFootPedals();
							m_tFootPedalStatus.bFootStuck = TRUE;
							checkBypassDebounce = TRUE;
                            m_usFootSwitchVersionNum = 0; // Read the version during debounce
							CalcConnectTime();
							m_tFootPedalStatus.bInSoftwareReset = FALSE;
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
 						CalcConnectTime();
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
						CalcConnectTime();
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

				m_tFootPedalStatus.wButtonStatus.left = (bSpeedDown)? 1:0;
				m_tFootPedalStatus.wButtonStatus.right = (bSpeedUp)? 1:0;

				if( m_bPollRs485)
				{
					if( (bForward || bReverse || bSpeedDown) )
					{
						m_bFastFootswitch = TRUE;
						m_bPollRs485 = FALSE;
						m_sFootConnectDebounce = 0;
						m_tFootPedalStatus.bInSoftwareReset = FALSE;
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

				m_tFootPedalStatus.pwPedalPercent[0] = (bReverse)?100:0;
				m_tFootPedalStatus.pwPedalPercent[1] = (bOsc)?100:0;
				m_tFootPedalStatus.pwPedalPercent[2] = (bForward)?100:0;

 				bWindowLock = DebounceSpeedUpDown(bSpeedUp, bSpeedDown, wPrevMode);
			}
            else if( m_tFootPedalStatus.usType == TYPE_485_ANALOG_FOOTPEDAL &&
					m_tFootPedalStatus.bInCalibration == FALSE )
            {
				SnBool byPassDebounce = FALSE;
				if (checkBypassDebounce)
				{
					SnWord wRequest = FOOT_CMD(SERIAL_CMD_VERS);
					// Get the Software version number
					if (m_pControl->SendSerialRequest(&wRequest, 4, m_cDuration))
					{
						byPassDebounce = TRUE;
						if (m_usFootSwitchVersionNum == 0)
                            m_usFootSwitchVersionNum = wRequest;
						}
					else
					{
						m_tFootPedalStatus.qErrorCount++;
						byPassDebounce = FALSE;
						m_tFootPedalStatus.bFootStuck = TRUE;
						m_sFootConnectDebounce = FOOTSWITCH_CONNECT_DEBOUNCE_MS;
					}
				}

				SnWord pwRequests[4] = {0};
				if( m_sFootConnectDebounce <= 0 || byPassDebounce )
				{
					int index;
					int ii =0;
					pwRequests[0] = FOOT_CMD(SERIAL_CMD_REQ_2);
					switch (wPrevMode)
					{
					case MOTOR_REVERSE:
						m_tFootPedalStatus.pwPedalPercent[1] = 0;

						// Get the Reverse Foot Pedal State for Serial Device, (0 - 100 in percent)
						if (m_tFootPedalStatus.usForward == FOOT_FORWARD_LEFT)
						{
							index = 2;
							pwRequests[1] = FOOT_CMD(SERIAL_CMD_REQ_5);
							m_tFootPedalStatus.pwPedalPercent[0] = 0;
						}
						else
						{
							index = 0;
							pwRequests[1] = FOOT_CMD(SERIAL_CMD_REQ_3);
							m_tFootPedalStatus.pwPedalPercent[2] = 0;
						}
 				
#if (COMMAND_RESPONSE_TEST ==0)
						if (m_pControl->SendSerialRequests(2, pwRequests, 4, m_cDuration) == FALSE)
						{
							m_tFootPedalStatus.qErrorCount++;
							pwRequests[0] = 0; 
							pwRequests[1] = 0; 
						}
#else
						for (ii = 0; ii <2; ii++)
						{
							if (m_pControl->SendSerialRequests(1, pwRequests+ii, 4, m_cDuration) == FALSE)
							{
								m_tFootPedalStatus.qErrorCount++;
								pwRequests[ii] = 0;
							}
						}
#endif
						wReversePercent = pwRequests[1] & 0x3ff;
						m_tFootPedalStatus.pwPedalPercent[index] = wReversePercent;
						break;

					case MOTOR_FORWARD:
						m_tFootPedalStatus.pwPedalPercent[1] = 0;

						// Get the Forward Foot Pedal State for Serial Device, (0 - 100 in percent)
						if (m_tFootPedalStatus.usForward == FOOT_FORWARD_LEFT)
						{
							index = 0;
							pwRequests[1] = FOOT_CMD(SERIAL_CMD_REQ_3);
							m_tFootPedalStatus.pwPedalPercent[2] = 0;
						}
						else
						{
							index = 2;
							pwRequests[1] = FOOT_CMD(SERIAL_CMD_REQ_5);
							m_tFootPedalStatus.pwPedalPercent[0] = 0;
						}
                    		
#if (COMMAND_RESPONSE_TEST ==0)
						if (m_pControl->SendSerialRequests(2, pwRequests, 4, m_cDuration) == FALSE)
						{
							m_tFootPedalStatus.qErrorCount++;
							pwRequests[0] = 0; 
							pwRequests[1] = 0; 
						}
#else
						for (ii = 0; ii <2; ii++)
						{
							if (m_pControl->SendSerialRequests(1, pwRequests+ii, 4, m_cDuration) == FALSE)
							{
								m_tFootPedalStatus.qErrorCount++;
								pwRequests[ii] = 0;
							}
						}
#endif
						wForwardPercent = pwRequests[1] & 0x3ff;
						m_tFootPedalStatus.pwPedalPercent[index] = wForwardPercent;
						break;

					case MOTOR_OSCILLATE:
						m_tFootPedalStatus.pwPedalPercent[0] = 0;
						m_tFootPedalStatus.pwPedalPercent[2] = 0;

						// Get the Oscillate Foot Pedal State for Serial Device, (0 - 100 in percent)
						pwRequests[1] = FOOT_CMD(SERIAL_CMD_REQ_4);
                    
#if (COMMAND_RESPONSE_TEST ==0)
						if (m_pControl->SendSerialRequests(2, pwRequests, 4, m_cDuration) == FALSE)
						{
							m_tFootPedalStatus.qErrorCount++;
							pwRequests[0] = 0; 
							pwRequests[1] = 0; 
						}
#else
						for (ii = 0; ii <2; ii++)
						{
							if (m_pControl->SendSerialRequests(1, pwRequests+ii, 4, m_cDuration) == FALSE)
							{
								m_tFootPedalStatus.qErrorCount++;
								pwRequests[ii] = 0;
							}
						}
#endif                    
						wOscPercent = pwRequests[1] & 0x3ff;
						m_tFootPedalStatus.pwPedalPercent[1] = wOscPercent;
						break;

					case MOTOR_WINDOW_LOCK:                
						m_tFootPedalStatus.pwPedalPercent[0] = 0;
						m_tFootPedalStatus.pwPedalPercent[1] = 0;
						m_tFootPedalStatus.pwPedalPercent[2] = 0;
						m_bFastFootswitch = TRUE;

						if (m_pControl->SendSerialRequest(pwRequests, 4, m_cDuration) == FALSE)
						{
							m_tFootPedalStatus.qErrorCount++;
							pwRequests[0] = 0; 
						}
						break;

					default:
						// Get the Forward, Reverse and Oscillate Foot Pedal States for Serial Device, (0 - 100 in percent)
						pwRequests[1] = FOOT_CMD(SERIAL_CMD_REQ_3);
						pwRequests[2] = FOOT_CMD(SERIAL_CMD_REQ_4);
						pwRequests[3] = FOOT_CMD(SERIAL_CMD_REQ_5);

#if (COMMAND_RESPONSE_TEST ==0)
						if (m_pControl->SendSerialRequests(4, pwRequests, 4, m_cDuration) == FALSE)
						{
							m_tFootPedalStatus.qErrorCount++;
							for (ii = 0; ii <4; ii++)
								pwRequests[ii] = 0;
						}
#else
						if (m_pControl->SendSerialRequests(1, pwRequests, 4, m_cDuration) == FALSE)
						{
							m_tFootPedalStatus.qErrorCount++;
							pwRequests[0] = 0;
						}
						if (m_pControl->SendSerialRequests(3, pwRequests+1, 4, m_cDuration) == FALSE)
						{
							for (ii = 1; ii <4; ii++)
							{
								m_tFootPedalStatus.qErrorCount++;
								pwRequests[ii] = 0;
							}
						}
#endif
						if (m_tFootPedalStatus.usForward == FOOT_FORWARD_LEFT)
						{
							wForwardPercent = pwRequests[1] & 0x3ff;
							wReversePercent = pwRequests[3] & 0x3ff;
							m_tFootPedalStatus.pwPedalPercent[0] =	wForwardPercent;
							m_tFootPedalStatus.pwPedalPercent[2] =	wReversePercent;
						}
						else
						{
							wForwardPercent = pwRequests[3] & 0x3ff;
							wReversePercent = pwRequests[1] & 0x3ff;
							m_tFootPedalStatus.pwPedalPercent[2] =	wForwardPercent;
							m_tFootPedalStatus.pwPedalPercent[0] =	wReversePercent;
						}
						wOscPercent = pwRequests[2] & 0x3ff;
						m_tFootPedalStatus.pwPedalPercent[1] =	wOscPercent;
						break;
					}
                
					// Get the Switch State for Serial Device, (0 - Released, 1 - Pressed)
					//	   Bit 0 - Left Switch State
					//	   Bit 1 - Middle Switch State
					//	   Bit 2 - Right Switch State

					m_tFootPedalStatus.wButtonStatus.w = pwRequests[0] & 0x3ff;

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
				m_tFootPedalStatus.dCommandDuration = m_cDuration.Median();
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

		if( m_tFootPedalStatus.usType == TYPE_485_ANALOG_FOOTPEDAL && !checkBypassDebounce)
		{
			if (wMode != MOTOR_OFF)
			{
				m_bFastFootswitch = TRUE;
			}
			else
			{
				m_bFastFootswitch = FALSE;
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
		int ii = 0;
        // Disable Rs485 & Enable Pullups
        SnWord wRequest = DISABLE_FOOT_CMD;
        m_pControl->SendSerialRequest(&wRequest, 1);
 
		if(m_tFootPedalStatus.usFootAssignedPort != TYPE_INVALID)
		{
			m_tFootPedalStatus.usFootAssignedPort = TYPE_INVALID;
			m_tFootPedalStatus.usPrevType = TYPE_UNKNOWN_FOOTPEDAL;
		}
  		m_tFootPedalStatus.usType = TYPE_INVALID;
		if (m_tFootPedalStatus.usPrevType != m_tFootPedalStatus.usType)
		{
 			m_tFootPedalStatus.bFootStuck = FALSE;
			m_tFootPedalStatus.bFootGood = FALSE;
			m_tFootPedalStatus.dCommandDuration = 0.0;
			m_tFootPedalStatus.qErrorCount = 0;
			m_tFootPedalStatus.qConnectDuration = 0;
			m_tFootPedalStatus.bInCalibration = FALSE;
			m_StartStopCalibration = NULL;
			m_tFootPedalStatus.bInInvalidCommandTest = FALSE;
			ResetVariableFootPedals();

			m_tFootPedalStatus.wButtonStatus.w = 0;

			m_pControl->SetFootMode(PORTA, MOTOR_OFF);
			m_pControl->SetFootMode(PORTB, MOTOR_OFF);
			m_bFastFootswitch = TRUE;
			m_sFootStatusDelay = RS485_FOOT_SETTLE_DELAY_MS;
			m_sFootConnectDebounce = FOOTSWITCH_CONNECT_DEBOUNCE_MS;
			m_bPollRs485 = TRUE;
			m_wSpUp = 0;
			m_wSpDn = 0;
			m_bDeviceConnected = FALSE;
			m_qConnectStartTime = 0;
  			m_cDuration.Initialize();
		}
       
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
		if(bStartSoftwareReset == TRUE)
		{
			m_bSoftwareResetRequest = FALSE;
			m_sFootStatusDelay = 0;
			m_sFootConnectDebounce = 0;
			m_bDeviceConnected = TRUE;
			m_qConnectStartTime = GetTickCount();
		}
		else
		{
			m_tFootPedalStatus.bInSoftwareReset = FALSE;
		}
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


SnByte bXorReceive(SnByte bBitIndex, SnByte bParity, SnByte bHelp)
{
    for(SnByte i=0;i<=bBitIndex;i++)     /*  Check Parity  */
    {
        bParity = bParity ^ (bHelp & 0x01);
        bHelp = bHelp >> 1;
    }
    return bParity;
}

#define E1_RECEIVE	0x1B
#define E2_RECEIVE	0x2D
#define E4_RECEIVE	0x4E

SnBool bCheckEcc(SnByte * bData)
{
    SnByte bEcc = 0;
	SnByte bBuffer = * bData;
	SnBool uiAcknowledge = TRUE;

    bEcc = (bXorReceive(7, bEcc, bBuffer)) << 1;      // Parity
    bEcc = (bXorReceive(6, bEcc, (bBuffer & E4_RECEIVE))) << 1;
    bEcc = (bXorReceive(6, bEcc, (bBuffer & E2_RECEIVE))) << 1;
    bEcc =  bXorReceive(6, bEcc, (bBuffer & E1_RECEIVE));
    
    // Perform error detection / correction
    // Parity   ECC     Action
    // 0        0       No errors
    // 0        1-7     Double error detected
    // 1        0       Parity bit wrong
    // 1        1-7     Single error, corrected
    switch(bEcc) {
    case 0x0:
        // No errors, nothing to do
        break;
    
    default:
        // cases 0x0 | (0x1 - 0x7)
        // Double error detect (no error correction), or RCV hardware error
        // Send back NAK
        uiAcknowledge = FALSE;
        break;
    
    case 0x8 | 0:
        // Parity bit is wrong, ignore
        break;
    
    case 0x8 | 1:
        // Fix E1, ignore
        break;
    case 0x8 | 2:
        // Fix E2, ignore
        break;
    case 0x8 | 3:
        // Fix D1
        bBuffer ^= 1;
        break;
    case 0x8 | 4:
        // Fix E4, ignore
        break;
    case 0x8 | 5:
        // Fix D2
        bBuffer ^= 2;
        break;
    case 0x8 | 6:
        // Fix D3
        bBuffer ^= 4;
        break;
    case 0x8 | 7:
        // Fix D4
        bBuffer ^= 8;
        break;
    }
    
	if (uiAcknowledge)
	{
		*bData = (bBuffer& 0x0F);
	}
	
	return uiAcknowledge;
}

typedef union  
{
    struct {
        unsigned B0:1;
        unsigned B1:1;
        unsigned B2:1;
        unsigned B3:1;
        unsigned B4:1;
        unsigned B5:1;
        unsigned B6:1;
        unsigned B7:1;
    };
    SnByte b;
} SnByteFlavors;

//-----------------------------------------------------------------------------------
// Rs485 functions

SnBool SenseRs485Cmd(SnByte* pbByte)
{
	SnBool yValidCmd = TRUE;
    SnByteFlavors tbRcvData;
    SnByte bECC;

	tbRcvData.b = *pbByte;
    
	// Check received byte for (7,4) ECC
    // Physical bit position:B7 B6 B5 B4 B3 B2 B1 B0
    // Physical mapping is:   P E4 E2 E1 D4 D3 D2 D1
    // Logical bit position is: 7  6  5  4  3  2  1
    // Logical address is:     d4 d3 d2 e4 d1 e2 e1
    // e1 = d1 ^ d2 ^ d4;
    // e2 = d1 ^ d3 ^ d4;
    // e4 = d2 ^ d3 ^ d4;
    bECC = 0;
    bECC |= ( tbRcvData.B0 ^ tbRcvData.B1 ^ tbRcvData.B3 ^ tbRcvData.B4);      //E1
    bECC |= ((tbRcvData.B0 ^ tbRcvData.B2 ^ tbRcvData.B3 ^ tbRcvData.B5) << 1);//E2
    bECC |= ((tbRcvData.B1 ^ tbRcvData.B2 ^ tbRcvData.B3 ^ tbRcvData.B6) << 2);//E4
    bECC |= ((tbRcvData.B0 ^ tbRcvData.B1 ^ tbRcvData.B2 ^ tbRcvData.B3 ^
              tbRcvData.B4 ^ tbRcvData.B5 ^ tbRcvData.B6 ^ tbRcvData.B7) << 3);//P

     // Perform error detection / correction
    // Parity   ECC     Action
    // 0        0       No errors
    // 0        1-7     Double error detected
    // 1        0       Parity bit wrong
    // 1        1-7     Single error, corrected
    switch(bECC) {
    case 0x0:
        // No errors, nothing to do
        break;

    default:
        // cases 0x0 | (0x1 - 0x7)
        // Double error detect (no error correction), or RCV hardware error
        // Send back NAK
        yValidCmd = FALSE;
        break;

    case 0x8 | 0:
        // Parity bit is wrong, ignore
        break;

    case 0x8 | 1:
        // Fix E1, ignore
        break;
    case 0x8 | 2:
        // Fix E2, ignore
        break;
    case 0x8 | 3:
        // Fix D1
        tbRcvData.B0 ^= 1;
        break;
    case 0x8 | 4:
        // Fix E4, ignore
        break;
    case 0x8 | 5:
        // Fix D2
        tbRcvData.B1 ^= 1;
        break;
    case 0x8 | 6:
        // Fix D3
        tbRcvData.B2 ^= 1;
        break;
    case 0x8 | 7:
        // Fix D4
        tbRcvData.B3 ^= 1;
        break;
    }

    if (yValidCmd) {
        *pbByte = (tbRcvData.b & 0x0F);
    }

    return yValidCmd;
}


void FootSwitch::InvalidCommandTest(void)
{
	SnWord pwResult;
	SnByte command;
	int results[16] = {0};
	SnBool failed = TRUE;
	int goodCommandCnt = 0;
	int difference = 0;
	SnBool goodCommand = FALSE;
	
	if (m_tFootPedalStatus.usType == TYPE_485_ANALOG_FOOTPEDAL)
	{
		SnWord ii =0;
		SnBool bDone = FALSE;
		
		do
		{
			command  = (SnByte)ii;
			
			if (goodCommand = SenseRs485Cmd(&command))
			{
				goodCommandCnt++;
				results[command]++;
			}
			
			pwResult = FOOT_CMD(ii);
			
			if (!goodCommand || (command  > 5 && command < 11) || command == 0xF)
			{
				if (m_pControl->SendSerialRequest(&pwResult, 8))
				{
					m_tFootPedalStatus.qErrorCount++;
				}
			}
			else if(command  <= 5 )
			{
				if (m_pControl->SendSerialRequest(&pwResult, 4))
				{
					switch (command)
					{
					case 0:
						break;
					case 1:
						if ((pwResult & 0x3FF) != 0x01)
							m_tFootPedalStatus.qErrorCount++;
						break;
					case 2:
						if ((pwResult & 0x3FF) != m_tFootPedalStatus.wButtonStatus.w)
							m_tFootPedalStatus.qErrorCount++;
						break;
					case 3:
						if ((pwResult & 0x3FF) != m_tFootPedalStatus.pwPedalPercent[0])
							m_tFootPedalStatus.qErrorCount++;
						break;
					case 4:
						if ((pwResult & 0x3FF) != m_tFootPedalStatus.pwPedalPercent[1])
							m_tFootPedalStatus.qErrorCount++;
						break;
					case 5:
						if ((pwResult & 0x3FF) != m_tFootPedalStatus.pwPedalPercent[2])
							m_tFootPedalStatus.qErrorCount++;
						break;
					}
				}
				else
				{
					m_tFootPedalStatus.qErrorCount++;
					ii = 0x100;
				}
			}
			ii++;
		}while (ii < 0x100);
	}
	m_tFootPedalStatus.bInInvalidCommandTest = FALSE;
}
