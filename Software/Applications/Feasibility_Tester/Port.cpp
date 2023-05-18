// Port.cpp: implementation of the CPort class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Control.h"
#include "Port.h"
#include "MotorData.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define MOTOR_SAFEMODE_OFF	0
#define MOTOR_SAFEMODE_1	1	// Curved blades only
#define MOTOR_SAFEMODE_2	2	// 10k burs to 8k burs, straight blades to curved
#define MOTOR_SAFEMODE_3	3	// Straight blades to curved
#define MOTOR_SAFEMODE_4	4

/***************************************************************
 Bldc Configuration
 ***************************************************************/

/* Bldc No Motor Safe Mode */
External g_tBldcNoMotor = tNoMotor;

/* Bldc PowerMax */
External g_tBldcPowerMax = tPowerMax;

/* Bldc Mini */
External g_tBldcMini = tMini;
 
/* Bldc DP MicroAire Drill */
External g_tBldcDpMicroAireDrill = tDrill;

/* Bldc DP MicroAire Saw */
External g_tBldcDpMicroAireSaw = tSaw;

/* Bldc EP-1 High Torque */
External g_tBldcHighTorque = tHiTorque;

//
// RS-485 Motors
//
 
/* Bldc PowerMax for now */
External g_tBldcSuperMax = tPowerMax;

/* Bldc Small Joint */
External g_tBldcSmallJoint = tPowerMini;

//////////////////////////////////////////////////////////////////////
// Static member definitions
//////////////////////////////////////////////////////////////////////

SnBool CPort::m_bControlsEnabled = TRUE;		// ***

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPort::CPort(void):
	m_cVelocityFast(10),
	m_cVelocity(20),
	m_cVelocitySlow(40),
	m_cMotorCurrent(20)
{
}

CPort::~CPort(void)
{
}

void CPort::Initialize(CControl * pControl, unsigned char ucPort)
{
	m_pControl = pControl;
	m_ucPort = ucPort;

	m_bSaveIfDirty = FALSE;
	m_bWindowLock = FALSE;
	m_bMode7 = FALSE;
	m_usMotorStallThreshold = DEVICE_STALL_SHUTOFF_LONG;
	m_usMotorStalledCount = 0;
	m_usMotorCurrentLimitCount = 0;
	m_usMotorTorqueLimitCount = 0;
	m_usMotorTacFaultCount = 0;
	m_bMotorTacTimerExpired = FALSE;
    m_us485DeviceVersionNum = 0;
	
	m_tPortStatus.sNewLogicIdCnt = 0;
	m_tPortStatus.usPrevLogic = 0;
	m_tPortStatus.usType = TYPE_INVALID; 
	m_tPortStatus.usPrevType = TYPE_MDU_TEST;
	m_tPortStatus.usHandMode = MOTOR_OFF;
	m_tPortStatus.usPrevHandMode = MOTOR_OFF;
	m_tPortStatus.usFootMode = MOTOR_OFF;
	m_tPortStatus.usPrevFootMode = MOTOR_OFF;
	m_tPortStatus.usDisplayMode  = MOTOR_OFF;
	m_tPortStatus.usPrevDisplayMode  = MOTOR_OFF;
	m_tPortStatus.usBladeId = BLADE_TYPE_UNKNOWN;
	m_tPortStatus.usBlade = BLADE_TYPE_UNKNOWN;
	m_tPortStatus.usPrevBlade = BLADE_TYPE_UNKNOWN;
	m_tPortStatus.usShaverBladeId = BLADE_ID_OTHER;
	m_tPortStatus.bRunning = FALSE;
	m_tPortStatus.wPrevDwell = 0;
	m_tPortStatus.usPrevRevolutions = 0;
	m_tPortStatus.wPrevPeriod = 0;
	m_tPortStatus.usPrevVelocity = 0;
	m_tPortStatus.bMotorReady = FALSE;
	m_tPortStatus.wStuckButton = 0;
	m_tPortStatus.bHandModeDualOn = FALSE;
	m_tPortStatus.usForwardDownCount = 0;
	m_tPortStatus.usReverseDownCount = 0;
	m_tPortStatus.usOscillateDownCount = 0;
	m_tPortStatus.usSafeMode = MOTOR_SAFEMODE_OFF;
    m_tPortStatus.wRS485Delay = RS485_HAND_SETTLE_DELAY_MS;
    m_tPortStatus.wHallDelay = HALL_SETTLE_DELAY_MS;
    m_tPortStatus.sTriggerDelay = TRIGGER_STABILITY_DELAY_MS;
	m_tPortStatus.wSerialLatch = 0;
	m_tPortStatus.wSerialPrevActive = 0;
	m_tPortStatus.usPrevFootHandMode = MOTOR_OFF;
	m_tPortStatus.bHandInCtl = FALSE;
    m_tPortStatus.bWaitForIdleCtl = FALSE;
    m_tPortStatus.yForceOscMode1 = FALSE;
	m_tPortStatus.usActualVelocity = 0;
	m_tPortStatus.usActualVelocityAbs = 0;
	m_tPortStatus.sMotorCurrent = 0;
	m_tPortStatus.sMotorCurrentAdc = 0;
	m_tPortStatus.sMotorCurrentAdcCount = 0;
	m_tPortStatus.sMotorTac = 0;
	m_tPortStatus.bHandButtonState = 0;
    m_tPortStatus.qShortCiruitTestCnt = 0;
    m_tPortStatus.qStableTypeInMs = 0;
    m_tPortStatus.qNonStableTypeInMs = 0;
    m_tPortStatus.bHandCtrlTimeout = FALSE;
    m_tPortStatus.qHandCtrlErrTime = 0;

	m_tPortEvents.m_hMotorStallEvent = NULL;
	m_tPortEvents.m_hMotorTacFaultEvent = NULL;
	m_tPortEvents.m_hMotorShortCircuitEvent = NULL;
	m_tPortEvents.m_hMotorShortCircuitTimeoutEvent = NULL;
	m_tPortEvents.m_hMotorCurrentLimitEvent = NULL;
	m_tPortEvents.m_hMotorCurrentLimitTimeoutEvent = NULL;
	m_tPortEvents.m_hMotorTorqueLimitEvent = NULL;
	m_tPortEvents.m_hMotorStallAndCurrentLimitEvent = NULL;
	m_tPortEvents.m_hUnknownBladeIdEvent = NULL;
	m_tPortEvents.m_hUnknownDeviceIdEvent = NULL;
	m_tPortEvents.m_hHallPatternFaultEvent = NULL;
	m_tPortEvents.m_hHandpieceStuckButtonEvent = NULL;
	m_tPortEvents.m_hFootswitchRequiredEvent = NULL;
	m_tPortEvents.m_hMotorKillTacTimerEvent = NULL;

	// Warning Status	
	m_tWarningStatus.bFootswitchRequired = FALSE;
	m_tWarningStatus.bHallPatternFault = FALSE;
	m_tWarningStatus.bHandpieceStuckButton = FALSE;
	m_tWarningStatus.bMotorCurrentLimit = FALSE;
	m_tWarningStatus.bMotorCurrentLimitTimeout = FALSE;
	m_tWarningStatus.bMotorShortCircuit = FALSE;
	m_tWarningStatus.bMotorShortCircuitTimeout = FALSE;
	m_tWarningStatus.bMotorStall = FALSE;
	m_tWarningStatus.bMotorStallAndCurrentLimit = FALSE;
	m_tWarningStatus.bMotorTacFault = FALSE;
	m_tWarningStatus.bUnknownBladeId= FALSE;
	m_tWarningStatus.bUnknownDeviceId = FALSE;
    m_tWarningStatus.bUnstableDeviceId = FALSE;

	m_tOffsetAddr.qVelocityCommand = MC_BLDCX_VELOCITY_SET(ucPort);
	m_tOffsetAddr.qVelocityActualAbs = MC_BLDCX_VEL_ACT_ABS(ucPort);
	m_tOffsetAddr.qMode = MC_BLDCX_MODE(ucPort);
	m_tOffsetAddr.qFault = MC_BLDCX_FAULT(ucPort);
	m_tOffsetAddr.qDwell = MC_BLDCX_DWELL(ucPort);
	m_tOffsetAddr.qPeriod = MC_BLDCX_PERIOD(ucPort);
	m_tOffsetAddr.qOverload = MC_BLDCX_OVERLOAD(ucPort);
	m_tOffsetAddr.qCurrent = (ucPort == PORTA)?MC_ANALOG_AVG0:MC_ANALOG_AVG8;
	m_tOffsetAddr.qCurrentSampleCount = (ucPort == PORTA)?MC_ANALOG_WCOUNT0:MC_ANALOG_WCOUNT8;
	m_tOffsetAddr.qLastTac = MC_BLDCX_LASTTAC(ucPort);

	m_tHallBusOffset.qDeviceExist = MC_HALLBUSX_DEVICE_EXIST(ucPort);
	m_tHallBusOffset.qDeviceLatch = MC_HALLBUSX_DEVICE_LATCH(ucPort);
	m_tHallBusOffset.qDeviceActive = MC_HALLBUSX_DEVICE_ACTIVE(ucPort);
	m_tHallBusOffset.qHallBusVq = MC_HALLBUSX_HALLBUS_VQ(ucPort);

	m_qSensorOffset = (ucPort == PORTA) ? MC_ANALOG_AVG4:MC_ANALOG_AVG7;

	m_cVelocity.Init();
	m_cVelocitySlow.Init();
	m_cVelocityFast.Init();
	m_cMotorCurrent.Init();
}

SnBool CPort::DisableMode7(void)
{
	// If the motor is put into MODE 7 it needs to be turned off after 1 second
	
	SnWord usMode = MOTOR_OFF;
	SnBool bStatus;
	
	// Write the mode to the motor control board		
	bStatus = m_pControl->WriteDsp( m_tOffsetAddr.qMode, 1, &usMode);
	if(!bStatus)
		return FALSE;
	
	m_bMode7 = FALSE;
	m_tPortStatus.usPrevFootHandMode = MOTOR_OFF;	

    return TRUE;
}

void CPort::GetRS485DeviceVersion(SnByte& major, SnByte& minor,SnByte& build)
{
	switch (m_tPortStatus.usType)
	{
    case TYPE_MDU_RELIANT:
	case TYPE_MDU_RELIANT_CTL:
	case TYPE_MDU_POWERMINI:
	case TYPE_MDU_POWERMINI_CTL:
		major = (m_us485DeviceVersionNum >> 8)  & 0x3;
		minor = (m_us485DeviceVersionNum >> 4)  & 0xF;
		build = m_us485DeviceVersionNum & 0xF;
		break;
	default:
		major = 0;
		minor = 0;
		build = 0;
	}
}

// Function:	GetPortParameters
// Purpose:		Gets the port parameters. Parameters dependent on port and device type detected
// Parameters:	None
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool CPort::GetPortParameters( void)
{
	
	SnWord usSavedForward, usSavedReverse, usSavedOscRpm; 
	SnWord usSavedForward2, usSavedReverse2;
	SnWord wSavedOscSeconds; 
    SnBool yUpdatePortSavedParams = FALSE;
	
	switch (m_tPortStatus.usType)
	{
	default:
		m_tPortStatus.usShaverBladeId = BLADE_ID_OTHER;
		break;
		
	case TYPE_DP_DRILL:
		m_tPortStatus.usPercentDefault = m_tPortDefaultParam.usDpDrillSpeed;
		m_tPortStatus.usPercentMin = 10;
		m_tPortStatus.usPercentMax = 100;
		m_tPortStatus.sPercentIncrement = 10;
		m_tPortStatus.usForwardMin = DP_DRILL_MIN_VELOCITY;
		m_tPortStatus.usForwardMax = DP_DRILL_MAX_VELOCITY;
		m_tPortStatus.usShaverBladeId = BLADE_ID_OTHER;
		
		if(m_tPortSavedParam.usDpDrillSpeed < m_tPortStatus.usPercentMin ||m_tPortSavedParam.usDpDrillSpeed > m_tPortStatus.usPercentMax)
		{
			m_tPortStatus.usPercent = m_tPortStatus.usPercentDefault;
			yUpdatePortSavedParams = TRUE;
		}
		else
			m_tPortStatus.usPercent = m_tPortSavedParam.usDpDrillSpeed;
		break;
		
	case TYPE_DP_SAW:
		m_tPortStatus.usPercentDefault = m_tPortDefaultParam.usDpSawSpeed;
		m_tPortStatus.usPercentMin = 10;
		m_tPortStatus.usPercentMax = 100;
		m_tPortStatus.sPercentIncrement = 10;
		m_tPortStatus.usForwardMin = DP_SAW_MIN_VELOCITY;
		m_tPortStatus.usForwardMax = DP_SAW_MAX_VELOCITY;
		m_tPortStatus.usShaverBladeId = BLADE_ID_OTHER;
		
		if(m_tPortSavedParam.usDpSawSpeed < m_tPortStatus.usPercentMin || m_tPortSavedParam.usDpSawSpeed > m_tPortStatus.usPercentMax)
		{
			m_tPortStatus.usPercent = m_tPortStatus.usPercentDefault;
			yUpdatePortSavedParams = TRUE;
		}
		else
			m_tPortStatus.usPercent = m_tPortSavedParam.usDpSawSpeed;
		break;
		
	case TYPE_MDU_STANDARD:
	case TYPE_MDU_STANDARD_CTL:
    case TYPE_MDU_RELIANT:
	case TYPE_MDU_RELIANT_CTL:
		// High speed Sensing Mdu's 
		// Ranges and defaults depend on blade type
		switch( m_tPortStatus.usBlade)
		{
		default:
		case BLADE_TYPE_UNKNOWN:
		case BLADE_TYPE_CURVED:
			m_tPortStatus.usShaverBladeId = BLADE_ID_CURVED;
			m_tPortStatus.usForwardMin = 100;
			m_tPortStatus.usForwardMax = 3000;
			m_tPortStatus.usForwardDefault = m_tPortDefaultParam.usHighSpeedCurvedForwardRpm;
			m_tPortStatus.usForward2Default = m_tPortDefaultParam.usHighSpeedCurvedForward2Rpm;
			m_tPortStatus.usForwardIncrement = 100;
			m_tPortStatus.usReverseMin = 100;
			m_tPortStatus.usReverseMax = 3000;
			m_tPortStatus.usReverseDefault = m_tPortDefaultParam.usHighSpeedCurvedReverseRpm;
			m_tPortStatus.usReverse2Default = m_tPortDefaultParam.usHighSpeedCurvedReverse2Rpm;
			m_tPortStatus.usReverseIncrement = 100;
			m_tPortStatus.usOscillateRpmMin = 500;
			m_tPortStatus.usOscillateRpmMax = 3000;
			m_tPortStatus.usOscillateRpmDefault = m_tPortDefaultParam.usHighSpeedCurvedOscillateRpm;
			m_tPortStatus.usOscillateRpmIncrement = 100;
			m_tPortStatus.wOscillateSecondsMin = 10;
			m_tPortStatus.wOscillateSecondsMax = 50;
			m_tPortStatus.wOscillateSecondsDefault = m_tPortDefaultParam.usHighSpeedCurvedOscillateSec;
			m_tPortStatus.wOscillateSecondsIncrement = 5;	
			
			if (m_tPortStatus.usRevolutions == 2)
				AdjustOscRangeForRevolutions(2);
			
			usSavedForward = m_tPortSavedParam.usHighSpeedCurvedForwardRpm;
			usSavedReverse = m_tPortSavedParam.usHighSpeedCurvedReverseRpm;
			usSavedForward2 = m_tPortSavedParam.usHighSpeedCurvedForward2Rpm;
			usSavedReverse2 = m_tPortSavedParam.usHighSpeedCurvedReverse2Rpm;
			usSavedOscRpm = m_tPortSavedParam.usHighSpeedCurvedOscillateRpm;
			wSavedOscSeconds = m_tPortSavedParam.usHighSpeedCurvedOscillateSec; 
			break;
			
		case BLADE_TYPE_STRAIGHT:
			m_tPortStatus.usShaverBladeId = BLADE_ID_STRAIGHT;
			m_tPortStatus.usForwardMin = 100;
			m_tPortStatus.usForwardMax = 5000;
			m_tPortStatus.usForwardDefault = m_tPortDefaultParam.usHighSpeedStraightForwardRpm;
			m_tPortStatus.usForward2Default = m_tPortDefaultParam.usHighSpeedStraightForward2Rpm;
			m_tPortStatus.usForwardIncrement = 100;
			m_tPortStatus.usReverseMin = 100;
			m_tPortStatus.usReverseMax = 5000;
			m_tPortStatus.usReverseDefault = m_tPortDefaultParam.usHighSpeedStraightReverseRpm;
			m_tPortStatus.usReverse2Default = m_tPortDefaultParam.usHighSpeedStraightReverse2Rpm;
			m_tPortStatus.usReverseIncrement = 100;
			m_tPortStatus.usOscillateRpmMin = 500;
			m_tPortStatus.usOscillateRpmMax = 3000;
			m_tPortStatus.usOscillateRpmDefault = m_tPortDefaultParam.usHighSpeedStraightOscillateRpm;
			m_tPortStatus.usOscillateRpmIncrement = 100;
			m_tPortStatus.wOscillateSecondsMin = 10;
			m_tPortStatus.wOscillateSecondsMax = 50;
			m_tPortStatus.wOscillateSecondsDefault = m_tPortDefaultParam.usHighSpeedStraightOscillateSec;
			m_tPortStatus.wOscillateSecondsIncrement = 5;
			
			if (m_tPortStatus.usRevolutions == 2)
				AdjustOscRangeForRevolutions(2);
			
			usSavedForward = m_tPortSavedParam.usHighSpeedStraightForwardRpm;
			usSavedReverse = m_tPortSavedParam.usHighSpeedStraightReverseRpm;
			usSavedForward2 = m_tPortSavedParam.usHighSpeedStraightForward2Rpm;
			usSavedReverse2 = m_tPortSavedParam.usHighSpeedStraightReverse2Rpm;
			usSavedOscRpm = m_tPortSavedParam.usHighSpeedStraightOscillateRpm;
			wSavedOscSeconds = m_tPortSavedParam.usHighSpeedStraightOscillateSec;
			break;
			
		case BLADE_TYPE_BUR:
			m_tPortStatus.usShaverBladeId = BLADE_ID_BURR;
			m_tPortStatus.usForwardMin = 500;
			m_tPortStatus.usForwardMax = 8000;
			m_tPortStatus.usForwardDefault = m_tPortDefaultParam.usHighSpeedBurForwardRpm;
			m_tPortStatus.usForward2Default = m_tPortDefaultParam.usHighSpeedBurForward2Rpm;
			m_tPortStatus.usForwardIncrement = 100;
			m_tPortStatus.usReverseMin = 500;
			m_tPortStatus.usReverseMax = 8000;
			m_tPortStatus.usReverseDefault = m_tPortDefaultParam.usHighSpeedBurReverseRpm;
			m_tPortStatus.usReverse2Default = m_tPortDefaultParam.usHighSpeedBurReverse2Rpm;
			m_tPortStatus.usReverseIncrement = 100;
			m_tPortStatus.usOscillateRpmMin = 500;
			m_tPortStatus.usOscillateRpmMax = 3000;
			m_tPortStatus.usOscillateRpmDefault = m_tPortDefaultParam.usHighSpeedBurOscillateRpm;
			m_tPortStatus.usOscillateRpmIncrement = 100;
			m_tPortStatus.wOscillateSecondsMin = 10;
			m_tPortStatus.wOscillateSecondsMax = 50;
			m_tPortStatus.wOscillateSecondsDefault = m_tPortDefaultParam.usHighSpeedBurOscillateSec;
			m_tPortStatus.wOscillateSecondsIncrement = 5;	
			
			usSavedForward = m_tPortSavedParam.usHighSpeedBurForwardRpm;
			usSavedReverse = m_tPortSavedParam.usHighSpeedBurReverseRpm;
			usSavedForward2 = m_tPortSavedParam.usHighSpeedBurForward2Rpm;
			usSavedReverse2 = m_tPortSavedParam.usHighSpeedBurReverse2Rpm;
			usSavedOscRpm = m_tPortSavedParam.usHighSpeedBurOscillateRpm;
			wSavedOscSeconds = m_tPortSavedParam.usHighSpeedBurOscillateSec;
			break;
			
		case BLADE_TYPE_FAST_BUR:
			m_tPortStatus.usShaverBladeId = BLADE_ID_FAST_BURR;
			m_tPortStatus.usForwardMin = 500;
			m_tPortStatus.usForwardMax = 10000;
			m_tPortStatus.usForwardDefault = m_tPortDefaultParam.usHighSpeedFastBurForwardRpm;
			m_tPortStatus.usForward2Default = m_tPortDefaultParam.usHighSpeedFastBurForward2Rpm;
			m_tPortStatus.usForwardIncrement = 100;
			m_tPortStatus.usReverseMin = 500;
			m_tPortStatus.usReverseMax = 10000;
			m_tPortStatus.usReverseDefault = m_tPortDefaultParam.usHighSpeedFastBurReverseRpm;
			m_tPortStatus.usReverse2Default = m_tPortDefaultParam.usHighSpeedFastBurReverse2Rpm;
			m_tPortStatus.usReverseIncrement = 100;
			m_tPortStatus.usOscillateRpmMin = 500;
			m_tPortStatus.usOscillateRpmMax = 3000;
			m_tPortStatus.usOscillateRpmDefault = m_tPortDefaultParam.usHighSpeedFastBurOscillateRpm;
			m_tPortStatus.usOscillateRpmIncrement = 100;
			m_tPortStatus.wOscillateSecondsMin = 10;
			m_tPortStatus.wOscillateSecondsMax = 50;
			m_tPortStatus.wOscillateSecondsDefault = m_tPortDefaultParam.usHighSpeedFastBurOscillateSec;
			m_tPortStatus.wOscillateSecondsIncrement = 5;	
			
			usSavedForward = m_tPortSavedParam.usHighSpeedFastBurForwardRpm;
			usSavedReverse = m_tPortSavedParam.usHighSpeedFastBurReverseRpm;
			usSavedForward2 = m_tPortSavedParam.usHighSpeedFastBurForward2Rpm;
			usSavedReverse2 = m_tPortSavedParam.usHighSpeedFastBurReverse2Rpm;
			usSavedOscRpm = m_tPortSavedParam.usHighSpeedFastBurOscillateRpm;
			wSavedOscSeconds = m_tPortSavedParam.usHighSpeedFastBurOscillateSec;
			break;
		}
		break;
		
	case TYPE_MDU_UTLRA_IUR:
		// Basic Shaver/Ultra Lite/IUR
		m_tPortStatus.usShaverBladeId = BLADE_ID_OTHER;
		m_tPortStatus.usForwardMin = 100;
		m_tPortStatus.usForwardMax = 5000;
		m_tPortStatus.usForwardDefault = m_tPortDefaultParam.usBasicForwardRpm;
		m_tPortStatus.usForward2Default = m_tPortDefaultParam.usBasicForward2Rpm;
		m_tPortStatus.usForwardIncrement = 100;
		m_tPortStatus.usReverseMin = 100;
		m_tPortStatus.usReverseMax = 5000;
		m_tPortStatus.usReverseDefault = m_tPortDefaultParam.usBasicReverseRpm;
		m_tPortStatus.usReverse2Default = m_tPortDefaultParam.usBasicReverse2Rpm;
		m_tPortStatus.usReverseIncrement = 100;
		m_tPortStatus.usOscillateRpmMin = 500;
		m_tPortStatus.usOscillateRpmMax = 3000;
		m_tPortStatus.usOscillateRpmDefault = m_tPortDefaultParam.usBasicOscillateRpm;
		m_tPortStatus.usOscillateRpmIncrement = 100;
		m_tPortStatus.wOscillateSecondsMin = 0;
		m_tPortStatus.wOscillateSecondsMax = 0;
		m_tPortStatus.wOscillateSecondsDefault = 0;
		m_tPortStatus.wOscillateSecondsIncrement = 0;	
		
		usSavedForward = m_tPortSavedParam.usBasicForwardRpm;
		usSavedReverse = m_tPortSavedParam.usBasicReverseRpm;
		usSavedForward2 = m_tPortSavedParam.usBasicForward2Rpm;
		usSavedReverse2 = m_tPortSavedParam.usBasicReverse2Rpm;
		usSavedOscRpm = m_tPortSavedParam.usBasicOscillateRpm;
		wSavedOscSeconds = m_tPortSavedParam.usBasicOscillateSec;
		break;
		
	case TYPE_MDU_MINI:
		// Mini 
		m_tPortStatus.usShaverBladeId = BLADE_ID_OTHER;
		m_tPortStatus.usForwardMin = 100;
		m_tPortStatus.usForwardMax = 3500;
		m_tPortStatus.usForwardDefault = m_tPortDefaultParam.usMiniForwardRpm;
		m_tPortStatus.usForward2Default = m_tPortDefaultParam.usMiniForward2Rpm;
		m_tPortStatus.usForwardIncrement = 100;
		m_tPortStatus.usReverseMin = 100;
		m_tPortStatus.usReverseMax = 3500;
		m_tPortStatus.usReverseDefault = m_tPortDefaultParam.usMiniReverseRpm;
		m_tPortStatus.usReverse2Default = m_tPortDefaultParam.usMiniReverse2Rpm;
		m_tPortStatus.usReverseIncrement = 100;
		m_tPortStatus.usOscillateRpmMin = 500;
		m_tPortStatus.usOscillateRpmMax = 3000;
		m_tPortStatus.usOscillateRpmDefault = m_tPortDefaultParam.usMiniOscillateRpm;
		m_tPortStatus.usOscillateRpmIncrement = 100;
		m_tPortStatus.wOscillateSecondsMin = 0;
		m_tPortStatus.wOscillateSecondsMax = 0;
		m_tPortStatus.wOscillateSecondsDefault = 0;
		m_tPortStatus.wOscillateSecondsIncrement = 0;	
		
		usSavedForward = m_tPortSavedParam.usMiniForwardRpm;
		usSavedReverse = m_tPortSavedParam.usMiniReverseRpm;
		usSavedForward2 = m_tPortSavedParam.usMiniForward2Rpm;
		usSavedReverse2 = m_tPortSavedParam.usMiniReverse2Rpm;
		usSavedOscRpm = m_tPortSavedParam.usMiniOscillateRpm;
		wSavedOscSeconds = m_tPortSavedParam.usMiniOscillateSec;
		break;
		
	case TYPE_MDU_POWERMINI:
	case TYPE_MDU_POWERMINI_CTL:
		// Ranges and defaults depend on blade type
		switch( m_tPortStatus.usBlade)
		{
		default:
		case BLADE_TYPE_UNKNOWN:
		case BLADE_TYPE_CURVED:
			m_tPortStatus.usShaverBladeId = BLADE_ID_CURVED;
			m_tPortStatus.usForwardMin = 100;
			m_tPortStatus.usForwardMax = 3000;
			m_tPortStatus.usForwardDefault = m_tPortDefaultParam.usSmallJointCurvedForwardRpm;
			m_tPortStatus.usForward2Default = m_tPortDefaultParam.usSmallJointCurvedForward2Rpm;
			m_tPortStatus.usForwardIncrement = 100;
			m_tPortStatus.usReverseMin = 100;
			m_tPortStatus.usReverseMax = 3000;
			m_tPortStatus.usReverseDefault = m_tPortDefaultParam.usSmallJointCurvedReverseRpm;
			m_tPortStatus.usReverse2Default = m_tPortDefaultParam.usSmallJointCurvedReverse2Rpm;
			m_tPortStatus.usReverseIncrement = 100;
			m_tPortStatus.usOscillateRpmMin = 500;
			m_tPortStatus.usOscillateRpmMax = 3000;
			m_tPortStatus.usOscillateRpmDefault = m_tPortDefaultParam.usSmallJointCurvedOscillateRpm;
			m_tPortStatus.usOscillateRpmIncrement = 100;
			m_tPortStatus.wOscillateSecondsMin = m_tPortDefaultParam.usSmallJointCurvedOscillateSec;
			m_tPortStatus.wOscillateSecondsMax = 50;
			m_tPortStatus.wOscillateSecondsDefault = m_tPortDefaultParam.usSmallJointCurvedOscillateSec;
			m_tPortStatus.wOscillateSecondsIncrement = 5;	
			
			usSavedForward = m_tPortSavedParam.usSmallJointCurvedForwardRpm;
			usSavedReverse = m_tPortSavedParam.usSmallJointCurvedReverseRpm;
			usSavedForward2 = m_tPortSavedParam.usSmallJointCurvedForward2Rpm;
			usSavedReverse2 = m_tPortSavedParam.usSmallJointCurvedReverse2Rpm;
			usSavedOscRpm = m_tPortSavedParam.usSmallJointCurvedOscillateRpm;
			wSavedOscSeconds = m_tPortSavedParam.usSmallJointCurvedOscillateSec; 
			break;
			
		case BLADE_TYPE_STRAIGHT:
			m_tPortStatus.usShaverBladeId = BLADE_ID_STRAIGHT;
			m_tPortStatus.usForwardMin = 100;
			m_tPortStatus.usForwardMax = 5000;
			m_tPortStatus.usForwardDefault = m_tPortDefaultParam.usSmallJointStraightForwardRpm;
			m_tPortStatus.usForward2Default = m_tPortDefaultParam.usSmallJointStraightForward2Rpm;
			m_tPortStatus.usForwardIncrement = 100;
			m_tPortStatus.usReverseMin = 100;
			m_tPortStatus.usReverseMax = 5000;
			m_tPortStatus.usReverseDefault = m_tPortDefaultParam.usSmallJointStraightReverseRpm;
			m_tPortStatus.usReverse2Default = m_tPortDefaultParam.usSmallJointStraightReverse2Rpm;
			m_tPortStatus.usReverseIncrement = 100;
			m_tPortStatus.usOscillateRpmMin = 500;
			m_tPortStatus.usOscillateRpmMax = 3000;
			m_tPortStatus.usOscillateRpmDefault = m_tPortDefaultParam.usSmallJointStraightOscillateRpm;
			m_tPortStatus.usOscillateRpmIncrement = 100;
			m_tPortStatus.wOscillateSecondsMin = m_tPortDefaultParam.usSmallJointStraightOscillateSec;
			m_tPortStatus.wOscillateSecondsMax = 50;
			m_tPortStatus.wOscillateSecondsDefault = m_tPortDefaultParam.usSmallJointStraightOscillateSec;
			m_tPortStatus.wOscillateSecondsIncrement = 5;	
			
			usSavedForward = m_tPortSavedParam.usSmallJointStraightForwardRpm;
			usSavedReverse = m_tPortSavedParam.usSmallJointStraightReverseRpm;
			usSavedForward2 = m_tPortSavedParam.usSmallJointStraightForward2Rpm;
			usSavedReverse2 = m_tPortSavedParam.usSmallJointStraightReverse2Rpm;
			usSavedOscRpm = m_tPortSavedParam.usSmallJointStraightOscillateRpm;
			wSavedOscSeconds = m_tPortSavedParam.usSmallJointStraightOscillateSec;
			break;
			
		case BLADE_TYPE_BUR:
			m_tPortStatus.usShaverBladeId = BLADE_ID_BURR;
			m_tPortStatus.usForwardMin = 500;
			m_tPortStatus.usForwardMax = 6000;
			m_tPortStatus.usForwardDefault = m_tPortDefaultParam.usSmallJointBurForwardRpm;
			m_tPortStatus.usForward2Default = m_tPortDefaultParam.usSmallJointBurForward2Rpm;
			m_tPortStatus.usForwardIncrement = 100;
			m_tPortStatus.usReverseMin = 500;
			m_tPortStatus.usReverseMax = 6000;
			m_tPortStatus.usReverseDefault = m_tPortDefaultParam.usSmallJointBurReverseRpm;
			m_tPortStatus.usReverse2Default = m_tPortDefaultParam.usSmallJointBurReverse2Rpm;
			m_tPortStatus.usReverseIncrement = 100;
			m_tPortStatus.usOscillateRpmMin = 500;
			m_tPortStatus.usOscillateRpmMax = 3000;
			m_tPortStatus.usOscillateRpmDefault = m_tPortDefaultParam.usSmallJointBurOscillateRpm;
			m_tPortStatus.usOscillateRpmIncrement = 100;
			m_tPortStatus.wOscillateSecondsMin = m_tPortDefaultParam.usSmallJointBurOscillateSec;
			m_tPortStatus.wOscillateSecondsMax = 50;
			m_tPortStatus.wOscillateSecondsDefault = m_tPortDefaultParam.usSmallJointBurOscillateSec;
			m_tPortStatus.wOscillateSecondsIncrement = 5;	
			
			usSavedForward = m_tPortSavedParam.usSmallJointBurForwardRpm;
			usSavedReverse = m_tPortSavedParam.usSmallJointBurReverseRpm;
			usSavedForward2 = m_tPortSavedParam.usSmallJointBurForward2Rpm;
			usSavedReverse2 = m_tPortSavedParam.usSmallJointBurReverse2Rpm;
			usSavedOscRpm = m_tPortSavedParam.usSmallJointBurOscillateRpm;
			wSavedOscSeconds = m_tPortSavedParam.usSmallJointBurOscillateSec;
			break;
		}
		break;
	}
	
	if( IS_TYPE_MDU(m_tPortStatus.usType))
	{
		// Use the saved values from persistent storage unless the values are out of range
		if( usSavedForward < m_tPortStatus.usForwardMin || usSavedForward > m_tPortStatus.usForwardMax)
		{
			m_tPortStatus.usForward = m_tPortStatus.usForwardDefault;
            yUpdatePortSavedParams = TRUE;
		}
		else
			m_tPortStatus.usForward = usSavedForward;	
		
		if( usSavedReverse < m_tPortStatus.usReverseMin || usSavedReverse > m_tPortStatus.usReverseMax)
		{
			m_tPortStatus.usReverse = m_tPortStatus.usReverseDefault;
            yUpdatePortSavedParams = TRUE;
		}
		else
			m_tPortStatus.usReverse = usSavedReverse;
		
		if( usSavedForward2 < m_tPortStatus.usForwardMin || usSavedForward2 > m_tPortStatus.usForwardMax)
		{
			m_tPortStatus.usForward2 = m_tPortStatus.usForward2Default;
            yUpdatePortSavedParams = TRUE;
		}
		else
			m_tPortStatus.usForward2 = usSavedForward2;
		
		if( usSavedReverse2 < m_tPortStatus.usReverseMin || usSavedReverse2 > m_tPortStatus.usReverseMax)
		{
			m_tPortStatus.usReverse2 = m_tPortStatus.usReverse2Default;
            yUpdatePortSavedParams = TRUE;
		}
		else
			m_tPortStatus.usReverse2 = usSavedReverse2;
		
		if( usSavedOscRpm < m_tPortStatus.usOscillateRpmMin || usSavedOscRpm > m_tPortStatus.usOscillateRpmMax)
		{
			m_tPortStatus.usOscillateRpm = m_tPortStatus.usOscillateRpmDefault;
            yUpdatePortSavedParams = TRUE;
		}
		else
			m_tPortStatus.usOscillateRpm = usSavedOscRpm; 
		
        if( m_tPortStatus.wOscillateSecondsMax == 0)
			m_tPortStatus.wOscillateSeconds = 0; 
		else if( wSavedOscSeconds < m_tPortStatus.wOscillateSecondsMin || wSavedOscSeconds > m_tPortStatus.wOscillateSecondsMax)
		{
			m_tPortStatus.wOscillateSeconds = m_tPortStatus.wOscillateSecondsDefault;
            yUpdatePortSavedParams = TRUE;
		}
		else
			m_tPortStatus.wOscillateSeconds = wSavedOscSeconds; 
	}
	
    if (yUpdatePortSavedParams)
        SetPortCustomSpeeds();
	
    return TRUE;
}


// Function:	WriteDwell
// Purpose:		Writes the Dwell Period to the Motor Control board 
//
// Parameters:	None
//
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool CPort::WriteDwell(void)
{
	
    SnBool bStatus;
	SnQByte m_qDwellOffsetAddr = m_tOffsetAddr.qDwell;
	SnWord usTemp;
	
	usTemp = (SnWord)m_tPortStatus.wDwell;
	
	// Write the correct dwell rate
	bStatus = m_pControl->WriteDsp(m_qDwellOffsetAddr, 1, &usTemp);
	if(!bStatus)
		return FALSE;
	
    return TRUE;
}

// Function:	WriteVelocity
// Purpose:		Writes the velocity to the Motor Control board 
//
// Parameters:	None
//
// Return:		TRUE indicates Success, FALSE indicates Failure
//

SnBool CPort::WriteVelocity(void)
{
    SnBool bStatus = FALSE;
	// Write the velocity
	bStatus = m_pControl->WriteDsp(m_tOffsetAddr.qVelocityCommand, 1, &m_tPortStatus.usVelocity);
	if(!bStatus)
		return FALSE;

   return TRUE;
}

// Function:	WritePeriod
// Purpose:		Writes the Period to the Motor Control board
//
// Parameters:	None
//
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool CPort::WritePeriod(void)
{
    SnBool bStatus = FALSE;
	SnWord buf[2];
    L_F_TYPE tPeriod;

	
	tPeriod.fType = (float)(m_tPortStatus.wPeriod) / 100.0f;
	buf[0] = (SnWord)(tPeriod.lType); // Low Word
	buf[1] = (SnWord)(tPeriod.lType >> 16); // High Word
 
	// Write the correct period
	bStatus = m_pControl->WriteDsp(m_tOffsetAddr.qPeriod, (sizeof(buf))/2, buf);
	if(!bStatus)
		return FALSE;

    return TRUE;
}

SnWord CPort::GetActualVelocity(void)
{
	SnBool bStatus; 
	SnWord usTemp = 0;
	
	bStatus = m_pControl->ReadDsp(m_tOffsetAddr.qVelocityActualAbs,1, &usTemp);
	if(bStatus)
	{
		m_cVelocity.AddValue(usTemp);
		m_cVelocitySlow.AddValue(usTemp);
		m_cVelocityFast.AddValue(usTemp);
		
		if (m_tPortStatus.usDisplayMode == MOTOR_OFF)
		{
			m_tPortStatus.usActualVelocityAbs = ((SnWord)usTemp + m_tPortStatus.usActualVelocityAbs)/2;
			m_tPortStatus.usActualVelocity = ((SnWord)usTemp + m_tPortStatus.usActualVelocity)/2;
		}
		else if (IS_TYPE_MDU(m_tPortStatus.usType))
		{
			if(m_tPortStatus.usDisplayMode == MOTOR_OSCILLATE_1 || m_tPortStatus.usDisplayMode == MOTOR_OSCILLATE_2)
			{
				m_tPortStatus.usActualVelocityAbs = (SnWord)m_cVelocity.Maximum();
				m_tPortStatus.usActualVelocity = (SnWord)m_cVelocityFast.Maximum();
			}
			else if (m_tPortStatus.usVelocity < 700)
			{
				m_tPortStatus.usActualVelocityAbs = (SnWord)m_cVelocitySlow.Average();
				m_tPortStatus.usActualVelocity = (SnWord)m_cVelocityFast.Average();
			}
			else
			{
				m_tPortStatus.usActualVelocityAbs = (SnWord)m_cVelocity.Average();
				m_tPortStatus.usActualVelocity = (SnWord)m_cVelocityFast.Average();
			}
		}
		else
		{
			m_tPortStatus.usActualVelocityAbs = (SnWord)m_cVelocity.Average();
			m_tPortStatus.usActualVelocity = (SnWord)m_cVelocityFast.Average();
		}
	}
	
	return usTemp;
}

SnSWord CPort::GetMotorCurrent(void)
{
	// The current is scales to 26.4 Amps to 4095 AD counts i.e. 6.446886 mA/AD count
	SnBool bStatus; 
	SnSWord sTemp = 0;
	
	bStatus = m_pControl->ReadDsp(m_tOffsetAddr.qCurrent,1, (SnWord *)&sTemp);
	if (bStatus)
	{
		double fTemp = 0.0;

		m_tPortStatus.sMotorCurrentAdc = sTemp;
//		Accomodate current sensing circuit not able to go to zero current
		fTemp = 0.6446886 * sTemp;
		sTemp = (SnSWord)(fTemp + 0.5);	// Round the result
		sTemp *= 10;					// to the nearest 10mA

		m_cMotorCurrent.AddValue(sTemp);
		sTemp = (SnSWord)m_cMotorCurrent.Filtered();

		if ((sTemp <= PORT_MIN_REPORTED_CURRENT+20) && (!m_tPortStatus.bRunning && !m_bWindowLock))
		{
			sTemp = 0; 
		}

		m_tPortStatus.sMotorCurrent = sTemp;
	}

	return (sTemp);
}

SnSWord CPort::GetMotorTac(void)
{
	SnBool bStatus; 
	SnSWord sTemp = 0;
	
	bStatus = m_pControl->ReadDsp(m_tOffsetAddr.qLastTac,1, (SnWord *)&sTemp);
	if(bStatus)
		m_tPortStatus.sMotorTac = sTemp;
	
	return sTemp;

}

// Function:	SetPortCustomSpeeds
// Purpose:		Takes the currently selected set speeds and copies them to a structure that will eventually
//				get saved to flash.. 
//
// Parameters:	None
//
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool CPort::SetPortCustomSpeeds( void )
{
	switch (m_tPortStatus.usType)
	{
	default:
		break;
		
	case TYPE_DP_DRILL:
		m_tPortSavedParam.usDpDrillSpeed = m_tPortStatus.usPercent;
		break;
		
	case TYPE_DP_SAW:
		m_tPortSavedParam.usDpSawSpeed = m_tPortStatus.usPercent;
		break;
		
	case TYPE_MDU_STANDARD:
	case TYPE_MDU_STANDARD_CTL:
	case TYPE_MDU_RELIANT:
	case TYPE_MDU_RELIANT_CTL:
		// High speed Sensing Mdu's 
		// Ranges and defaults depend on blade type
		switch( m_tPortStatus.usBlade)
		{
		default:
		case BLADE_TYPE_UNKNOWN:
		case BLADE_TYPE_CURVED:
			m_tPortSavedParam.usHighSpeedCurvedForwardRpm = m_tPortStatus.usForward;
			m_tPortSavedParam.usHighSpeedCurvedReverseRpm = m_tPortStatus.usReverse;
			m_tPortSavedParam.usHighSpeedCurvedForward2Rpm = m_tPortStatus.usForward2;
			m_tPortSavedParam.usHighSpeedCurvedReverse2Rpm = m_tPortStatus.usReverse2;
			m_tPortSavedParam.usHighSpeedCurvedOscillateRpm = m_tPortStatus.usOscillateRpm;
			m_tPortSavedParam.usHighSpeedCurvedOscillateSec = m_tPortStatus.wOscillateSeconds; 
			break;
			
		case BLADE_TYPE_STRAIGHT:
			m_tPortSavedParam.usHighSpeedStraightForwardRpm = m_tPortStatus.usForward;
			m_tPortSavedParam.usHighSpeedStraightReverseRpm = m_tPortStatus.usReverse;
			m_tPortSavedParam.usHighSpeedStraightForward2Rpm = m_tPortStatus.usForward2;
			m_tPortSavedParam.usHighSpeedStraightReverse2Rpm = m_tPortStatus.usReverse2;
			m_tPortSavedParam.usHighSpeedStraightOscillateRpm = m_tPortStatus.usOscillateRpm;
			m_tPortSavedParam.usHighSpeedStraightOscillateSec = m_tPortStatus.wOscillateSeconds;
			break;
			
		case BLADE_TYPE_BUR:
			m_tPortSavedParam.usHighSpeedBurForwardRpm = m_tPortStatus.usForward;
			m_tPortSavedParam.usHighSpeedBurReverseRpm = m_tPortStatus.usReverse;
			m_tPortSavedParam.usHighSpeedBurForward2Rpm = m_tPortStatus.usForward2;
			m_tPortSavedParam.usHighSpeedBurReverse2Rpm = m_tPortStatus.usReverse2;
			m_tPortSavedParam.usHighSpeedBurOscillateRpm = m_tPortStatus.usOscillateRpm;
			m_tPortSavedParam.usHighSpeedBurOscillateSec = m_tPortStatus.wOscillateSeconds;
			break;
			
		case BLADE_TYPE_FAST_BUR:
			m_tPortSavedParam.usHighSpeedFastBurForwardRpm = m_tPortStatus.usForward;
			m_tPortSavedParam.usHighSpeedFastBurReverseRpm = m_tPortStatus.usReverse;
			m_tPortSavedParam.usHighSpeedFastBurForward2Rpm = m_tPortStatus.usForward2;
			m_tPortSavedParam.usHighSpeedFastBurReverse2Rpm = m_tPortStatus.usReverse2;
			m_tPortSavedParam.usHighSpeedFastBurOscillateRpm = m_tPortStatus.usOscillateRpm;
			m_tPortSavedParam.usHighSpeedFastBurOscillateSec = m_tPortStatus.wOscillateSeconds;
			break;
		}
		break;	
		
	case TYPE_MDU_UTLRA_IUR:
		// Basic Shaver/Ultra Lite/IUR
		m_tPortSavedParam.usBasicForwardRpm = m_tPortStatus.usForward;
		m_tPortSavedParam.usBasicReverseRpm = m_tPortStatus.usReverse;
		m_tPortSavedParam.usBasicForward2Rpm = m_tPortStatus.usForward2;
		m_tPortSavedParam.usBasicReverse2Rpm = m_tPortStatus.usReverse2;
		m_tPortSavedParam.usBasicOscillateRpm = m_tPortStatus.usOscillateRpm;
		m_tPortSavedParam.usBasicOscillateSec = m_tPortStatus.wOscillateSeconds;
		break;
		
	case TYPE_MDU_MINI:
		m_tPortSavedParam.usMiniForwardRpm = m_tPortStatus.usForward;
		m_tPortSavedParam.usMiniReverseRpm = m_tPortStatus.usReverse;
		m_tPortSavedParam.usMiniForward2Rpm = m_tPortStatus.usForward2;
		m_tPortSavedParam.usMiniReverse2Rpm = m_tPortStatus.usReverse2;
		m_tPortSavedParam.usMiniOscillateRpm = m_tPortStatus.usOscillateRpm;
		m_tPortSavedParam.usMiniOscillateSec = m_tPortStatus.wOscillateSeconds;
		break;
		
	case TYPE_MDU_POWERMINI:
	case TYPE_MDU_POWERMINI_CTL:
		// Ranges and defaults depend on blade type
		switch( m_tPortStatus.usBlade)
		{
		default:
		case BLADE_TYPE_UNKNOWN:
		case BLADE_TYPE_CURVED:
			m_tPortSavedParam.usSmallJointCurvedForwardRpm = m_tPortStatus.usForward;
			m_tPortSavedParam.usSmallJointCurvedReverseRpm = m_tPortStatus.usReverse;
			m_tPortSavedParam.usSmallJointCurvedForward2Rpm = m_tPortStatus.usForward2;
			m_tPortSavedParam.usSmallJointCurvedReverse2Rpm = m_tPortStatus.usReverse2;
			m_tPortSavedParam.usSmallJointCurvedOscillateRpm = m_tPortStatus.usOscillateRpm;
			m_tPortSavedParam.usSmallJointCurvedOscillateSec = m_tPortStatus.wOscillateSeconds; 
			break;
			
		case BLADE_TYPE_STRAIGHT:
			m_tPortSavedParam.usSmallJointStraightForwardRpm = m_tPortStatus.usForward;
			m_tPortSavedParam.usSmallJointStraightReverseRpm = m_tPortStatus.usReverse;
			m_tPortSavedParam.usSmallJointStraightForward2Rpm = m_tPortStatus.usForward2;
			m_tPortSavedParam.usSmallJointStraightReverse2Rpm = m_tPortStatus.usReverse2;
			m_tPortSavedParam.usSmallJointStraightOscillateRpm = m_tPortStatus.usOscillateRpm;
			m_tPortSavedParam.usSmallJointStraightOscillateSec = m_tPortStatus.wOscillateSeconds;
			break;
			
		case BLADE_TYPE_BUR:
			m_tPortSavedParam.usSmallJointBurForwardRpm = m_tPortStatus.usForward;
			m_tPortSavedParam.usSmallJointBurReverseRpm = m_tPortStatus.usReverse;
			m_tPortSavedParam.usSmallJointBurForward2Rpm = m_tPortStatus.usForward2;
			m_tPortSavedParam.usSmallJointBurReverse2Rpm = m_tPortStatus.usReverse2;
			m_tPortSavedParam.usSmallJointBurOscillateRpm = m_tPortStatus.usOscillateRpm;
			m_tPortSavedParam.usSmallJointBurOscillateSec = m_tPortStatus.wOscillateSeconds;
			break;
		}
		break;
	}
	
	m_bSaveIfDirty = TRUE; // User parameters have been changed
	return TRUE;
}

void CPort::ClearAllMotorWarnings(void)
{
	if( m_tWarningStatus.bFootswitchRequired)
	{
		m_tWarningStatus.bFootswitchRequired = FALSE;
		SetEvent( m_tPortEvents.m_hFootswitchRequiredEvent);
	}
	if(m_tWarningStatus.bUnknownBladeId)
	{
		m_tWarningStatus.bUnknownBladeId = FALSE;
		SetEvent(m_tPortEvents.m_hUnknownBladeIdEvent);
	}
	if(m_tWarningStatus.bMotorShortCircuit)
	{
		m_tWarningStatus.bMotorShortCircuit = FALSE;
		SetEvent(m_tPortEvents.m_hMotorShortCircuitEvent);
	}
	if(m_tWarningStatus.bMotorShortCircuitTimeout)
	{
		m_tWarningStatus.bMotorShortCircuitTimeout = FALSE;
		SetEvent(m_tPortEvents.m_hMotorShortCircuitTimeoutEvent);
	}
	if(m_tWarningStatus.bMotorTacFault)
	{
		m_usMotorTacFaultCount = 0;
		m_tWarningStatus.bMotorTacFault = FALSE;
		SetEvent(m_tPortEvents.m_hMotorTacFaultEvent);
	}
	if(m_tWarningStatus.bMotorStall)
	{
		m_tWarningStatus.bMotorStall = FALSE;
		SetEvent(m_tPortEvents.m_hMotorStallEvent);
	}
	if(m_tWarningStatus.bMotorStallAndCurrentLimit)
	{
		m_tWarningStatus.bMotorStallAndCurrentLimit = FALSE;
		SetEvent(m_tPortEvents.m_hMotorStallAndCurrentLimitEvent);
	}
	if( m_tWarningStatus.bMotorCurrentLimit)
	{
		m_tWarningStatus.bMotorCurrentLimit = FALSE;
		SetEvent(m_tPortEvents.m_hMotorCurrentLimitEvent);
	}
	if(m_tWarningStatus.bMotorCurrentLimitTimeout)
	{
		m_tWarningStatus.bMotorCurrentLimitTimeout = FALSE;
		SetEvent(m_tPortEvents.m_hMotorCurrentLimitTimeoutEvent);
	}
	if(m_tWarningStatus.bMotorTorqueLimit)
	{
		m_tWarningStatus.bMotorTorqueLimit = FALSE;
		SetEvent(m_tPortEvents.m_hMotorTorqueLimitEvent);
	}
	if(m_tWarningStatus.bHandpieceStuckButton)
	{
		m_tWarningStatus.bHandpieceStuckButton = FALSE;
		SetEvent(m_tPortEvents.m_hHandpieceStuckButtonEvent);
	}
    m_tPortStatus.usSafeMode = MOTOR_SAFEMODE_OFF;
	if( m_tWarningStatus.bHallPatternFault)
	{
		m_tWarningStatus.bHallPatternFault = FALSE;
		SetEvent(m_tPortEvents.m_hHallPatternFaultEvent);
	}
}

// Function:	WriteMode
// Purpose:		Writes the mode  to the Motor Control board
//
// Parameters:	None
//
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool CPort::WriteMode(void)
{
    SnBool bStatus;
	SnWord usMode;
	SnBool bFootPedalAssigned ;
	SnBool bFootPedalOverride = m_pControl->FootswitchOverride();
	
	
	if( m_ucPort == PORTA || m_ucPort == PORTB)
	{
		bFootPedalAssigned = ( m_ucPort == m_pControl->FootswitchPortAssignment()) ;
	}
	else
	{
		return FALSE;
	}
	
	// GUI Window Lock feature is active
	if( m_bWindowLock)
	{
		usMode = MOTOR_WINDOW_LOCK;
	}
	else if(bFootPedalAssigned && bFootPedalOverride && 
		IS_TYPE_MDU(m_tPortStatus.usType))
	{
		// Hand Control Overide Enabled Foot Pedal in control of motor
		usMode = m_tPortStatus.usFootMode;
	}
	else if(bFootPedalAssigned && !bFootPedalOverride && 
		IS_TYPE_MDU(m_tPortStatus.usType))
	{
		// Foot Pedal or Mdu can drive the motor 
		if( m_tPortStatus.usHandMode == MOTOR_OFF && m_tPortStatus.usFootMode != MOTOR_OFF)
			usMode = m_tPortStatus.usFootMode;
		else if(m_tPortStatus.usHandMode != MOTOR_OFF && m_tPortStatus.usFootMode == MOTOR_OFF)
			usMode = m_tPortStatus.usHandMode;	
		else if(m_tPortStatus.usHandMode == MOTOR_OFF && m_tPortStatus.usFootMode == MOTOR_OFF)
			usMode = MOTOR_OFF;
		else 
			usMode = MOTOR_OFF;
	}	
	else
	{
		// Foot Pedal or Power Instrument can drive the motor 
		if( m_tPortStatus.usHandMode == MOTOR_OFF && m_tPortStatus.usFootMode != MOTOR_OFF)
			usMode = m_tPortStatus.usFootMode;
		else if(m_tPortStatus.usHandMode != MOTOR_OFF && m_tPortStatus.usFootMode == MOTOR_OFF)
			usMode = m_tPortStatus.usHandMode;	
		else if(m_tPortStatus.usHandMode == MOTOR_OFF && m_tPortStatus.usFootMode == MOTOR_OFF)
			usMode = MOTOR_OFF;
		else 
			usMode = MOTOR_OFF;
	}
	
	if(!m_bControlsEnabled)
		usMode = MOTOR_OFF; // turn the motor off
    else if (!m_tPortStatus.bMotorReady)
    {
        if (m_tWarningStatus.bMotorStallAndCurrentLimit && usMode != MOTOR_OFF &&
            (m_tPortStatus.usPrevFootHandMode == MOTOR_OFF))
        {
            m_usMotorCurrentLimitCount = 0;
            m_usMotorStalledCount = 0;
            m_tWarningStatus.bMotorStallAndCurrentLimit = FALSE;
			m_usMotorStallThreshold = DEVICE_STALL_SHUTOFF_LONG;
            SetEvent(m_tPortEvents.m_hMotorStallAndCurrentLimitEvent);
            m_tPortStatus.bMotorReady = TRUE;
        }
        else
            usMode = MOTOR_OFF;
    }
	
    if( !IS_TYPE_SAW(m_tPortStatus.usType) )
		m_tPortStatus.usDisplayMode = usMode;// The usDisplayMode is used by the GUI to determine direction
	// The mode for the Saw and Combo is always forward but the GUI thinks
	// it's in Oscillate mode or Forward mode. Hence, the reason for the DisplayMode variable.
	
	if( IS_TYPE_MDU(m_tPortStatus.usType) && m_tPortStatus.usPrevFootHandMode != MOTOR_WINDOW_LOCK &&
		usMode == MOTOR_OFF && m_tPortStatus.bMotorReady)
	{
		usMode = MOTOR_OFF_MODE7;
		m_bMode7 = TRUE;
	}
	else
	{
		m_bMode7 = FALSE;
	}
	
	if( IS_TYPE_MDU(m_tPortStatus.usType))
		m_tPortStatus.usPrevFootHandMode = usMode; // set the previous mode to the current mode
	
	// Write the mode to the motor control board		
	bStatus = m_pControl->WriteDsp( m_tOffsetAddr.qMode, 1, &usMode);
	if(!bStatus)
		return FALSE;
	
	
	if( usMode == MOTOR_OFF || usMode == MOTOR_OFF_MODE7) 
		m_tPortStatus.bRunning = FALSE;
    else {
		m_tPortStatus.bRunning = TRUE;
        m_pControl->NotifyNewUsage();
    }
	
    // Update the Pump with the new usMode if it is present
	if(m_pControl->m_RemotePumpConnectionType != CPump::PUMP_TYPE_UNKNOWN)
	{
		m_pControl->SetShaverSpeed(m_ucPort, usMode, m_tPortStatus.wPeriod, m_tPortStatus.usVelocity);
		m_pControl->SetShaverOpState(m_ucPort, usMode);
	}

	PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS, (WPARAM)MSG_UPDATE_RUNNING_STATUS, (LPARAM)m_ucPort);
	
    return TRUE;
}

// Function:	CheckBladeType
// Purpose:		Function checks the blade type for supported blades.
//				If the blade type is unsupported the function returns
//				a usable blade family
//
// Parameters:	usBladeType		Blade type
//
// Return:		Returns supported blade family
//
SnWord CPort::CheckBladeType(SnWord usBladeType)
{
	SnWord usSupportedBladeFamily;
	
	switch( usBladeType)
	{
	case BLADE_TYPE_CURVED:
	case BLADE_TYPE_STRAIGHT:
	case BLADE_TYPE_BUR:
	case BLADE_TYPE_FAST_BUR:
		usSupportedBladeFamily = usBladeType;
		
		if( m_tWarningStatus.bUnknownBladeId)
		{
			// Clear the unknown blade warning if it was set
			m_tWarningStatus.bUnknownBladeId = FALSE;
			SetEvent(m_tPortEvents.m_hUnknownBladeIdEvent);
		}
		
		break;
		
	default:
		// Blade type not supported/unknown 
		// Use curved blade family for this condtion
		usSupportedBladeFamily = BLADE_TYPE_CURVED; 
		
		if(!m_tWarningStatus.bUnknownBladeId)
		{
			// Initiate an unknown blade warning
			m_tWarningStatus.bUnknownBladeId = TRUE;
			SetEvent(m_tPortEvents.m_hUnknownBladeIdEvent);
		}
		break;
	}
	
	// Safe Modes only apply to HAL ID MDUs
	if (m_tWarningStatus.bUnknownBladeId == FALSE)
	{
		switch(m_tPortStatus.usSafeMode)
		{
		case MOTOR_SAFEMODE_1:
			usSupportedBladeFamily = BLADE_TYPE_CURVED; // Curved blades only 
			break;
		case MOTOR_SAFEMODE_2:
			if (usBladeType == BLADE_TYPE_STRAIGHT)
				usSupportedBladeFamily = BLADE_TYPE_CURVED; // replace staight blades with curved
			else if (usBladeType == BLADE_TYPE_FAST_BUR)
				usSupportedBladeFamily = BLADE_TYPE_BUR; // replace fast burs with slow burs
			break;
		case MOTOR_SAFEMODE_3:
			if (usBladeType == BLADE_TYPE_STRAIGHT)
				usSupportedBladeFamily = BLADE_TYPE_CURVED; // replace staight blades with curved
			break;
		}
	}

	return usSupportedBladeFamily;
}

// Function:	PowerMiniCheckBladeType
// Purpose:		Function checks the blade type for supported blades.
//				If the blade type is unsupported the function returns
//				a usable blade family
//
//				The Power Mini provides support for 9 blade type families.
//				At this time only 3 blade type families are supported
//				Straight, Curved, and Bur.
//
// Parameters:	mduOut		The mdu.Out parameter from the POWERMINI MDU
//							Accessory Specific Message 5
//
// Return:		Returns supported blade family
//
SnWord CPort::PowerMiniCheckBladeType(SnWord mduOut)
{
	SnWord usBladeType;

	mduOut &= SMJ_BLADE_TYPE_MASK;	// Retrieve the blade type

	switch( mduOut)
	{
	case SMJ_BLADE_MEDIUM_SPEED:				// Map Medium Speed Blade Family
		usBladeType = BLADE_TYPE_STRAIGHT;		// to Straight blade
		break;
	case SMJ_BLADE_LOW_SPEED_R1:				// Map Low Speed Blade Family Range 1
		usBladeType = BLADE_TYPE_CURVED;		// to Curved Blade type
		break;
	case SMJ_BLADE_HIGH_SPEED_R1:				// Map NEB High Speed Blade Family Range 1
		usBladeType = BLADE_TYPE_BUR;			// to Bur blade type
		break;
		
	default:
		usBladeType = mduOut >> 4;		// Force all other blade type to unknown
		break;
	}

	return (CheckBladeType(usBladeType));
}


SnBool CPort::ReadTrigger( SnWord wDeadBand, SnWord *pwVelocity)
{
    SnSWord sMaxLimit;          // run threshold
    SnSWord sMinLimit;          // run region limit
    SnSWord sTrigger;           // raw adc value from motor board
    SnBool yStatus;
	SnWord pwVelMinimum = m_tPortStatus.usForwardMin;
	
    // Read adc value from appropriate port
    yStatus = m_pControl->ReadDsp(m_qSensorOffset, 1, (SnWord *)&sTrigger);
    if (!yStatus)
		return FALSE;
	
    // Calculate ON region limits
    sMaxLimit = m_tPortStatus.sTriggerMax - wDeadBand;
    sMinLimit = m_tPortStatus.sTriggerMin + wDeadBand;
	
    // Filter out of bounds Trigger values
    if (sTrigger < 1500)
    {
		m_tPortStatus.sTriggerMax = 0;
		m_tPortStatus.sTriggerMin = DP_DRILL_SAW_START_MIN;
        m_tPortStatus.sTriggerDelay = TRIGGER_STABILITY_DELAY_MS;
        *pwVelocity = 0;
    }
    else
    {
		// Look for stable TriggerMax
        if (m_tPortStatus.sTriggerDelay > 0)
        {
            if (m_tPortStatus.sTriggerMax > 0 &&
              (sTrigger > (m_tPortStatus.sTriggerMax + 15) ||
               sTrigger < (m_tPortStatus.sTriggerMax - 15)))
                m_tPortStatus.sTriggerDelay = TRIGGER_STABILITY_DELAY_MS;

            *pwVelocity = 0;
            m_tPortStatus.sTriggerMax = sTrigger;
            m_tPortStatus.sTriggerMin = sTrigger - 2 * wDeadBand - 35;
            m_tPortStatus.sTriggerDelay -= m_pControl->GetDelayPeriodMs();
        }
        else
        {
            // Force first pass initialization through OFF when sMinLimit > sMaxLimit
            // then establish if legal condition for velocity calculation and turn ON
            if (sTrigger <= sMaxLimit && sMaxLimit > sMinLimit) {
		        
                // Save new TriggerMin
		        if (sTrigger < m_tPortStatus.sTriggerMin)
			        m_tPortStatus.sTriggerMin = sTrigger;
		        
                // Clamp Trigger at minimum
                if (sTrigger < sMinLimit)
                    sTrigger = sMinLimit;
		        
                // Scale MaxVelocity by the percent of trigger depression and speed setting
                *pwVelocity = (SnWord)(pwVelMinimum + (*pwVelocity - pwVelMinimum)
			        * (m_tPortStatus.usPercent * 1.0 * (sMaxLimit - sTrigger)) / ((sMaxLimit - sMinLimit) * 100.0));
            }
            // Turn OFF
            else
            {
                if (sTrigger > m_tPortStatus.sTriggerMax + 15)      // 15 adc counts hysteresis
                    m_tPortStatus.sTriggerDelay = TRIGGER_STABILITY_DELAY_MS;
                *pwVelocity = 0;
            }
        }
    }

    return TRUE;
}

void CPort::SetPortStatus(SN_PORT_STATUS* ptInputBuffer)
{

	m_tPortStatus.usForward = ptInputBuffer->usForward;	// Current Forward setting	
	m_tPortStatus.usForward2 = ptInputBuffer->usForward2;	// Second set speed
	m_tPortStatus.usReverse = ptInputBuffer->usReverse;	// Current Reverse setting
	m_tPortStatus.usReverse2 = ptInputBuffer->usReverse2;  // Second set speed
	m_tPortStatus.usOscillateRpm = ptInputBuffer->usOscillateRpm;// Current Oscillate setting
	m_tPortStatus.usOscMode = ptInputBuffer->usOscMode;	// Current Oscillate Mode:Mode1/Mode2
	m_tPortStatus.wOscillateSeconds = ptInputBuffer->wOscillateSeconds;		// Osc Mode 2 seconds
	m_tPortStatus.wDwell = ptInputBuffer->wDwell;			// Osc Mode 1 dwell period
	m_tPortStatus.usPercent = ptInputBuffer->usPercent;	// Current Hand Powered Tool setting
	if (m_tPortStatus.usRevolutions != ptInputBuffer->usRevolutions)
		AdjustOscRangeForRevolutions( ptInputBuffer->usRevolutions);
	m_tPortStatus.usRevolutions = ptInputBuffer->usRevolutions; // Osc Mode 2 # revolutions
}

// Function:	ReadPortStatus
// Purpose: 	Reads the mode status from the Motor Control board for Port A or B
//
// Parameters:	None
//
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool CPort::ReadPortStatus(void)
{
	
	SnWord usType = TYPE_INVALID;
	SnBool bHallbusHandControls = FALSE;
	SnBool bSerialHandControls = FALSE;
	External *ptBldc = &g_tBldcNoMotor;
	SnWord usLogic;
	SnWord usVelocity;
	SnBool bStatus;
	SnWord usPercent;
	SnWord usDirectionMask;
	SnWord usMode;
	SnWord wActive;
	SnBool bMduFound = FALSE;
	SnBool bHallOr485DelayActive = FALSE;
	SnWord usSafeMode = MOTOR_SAFEMODE_OFF;
	SnWord wDeadBand;
	SnWord usDeviceExist;
	SnWord usDirection;
	SnBool bFootPedalAssigned = FALSE;
	SnBool bFootPedalOverride = m_pControl->FootswitchOverride();
	SnWord wTotalDelay;
	
	usMode = MOTOR_OFF;
	usVelocity = 0;
	
	// Check for installed devices
	bStatus = m_pControl->ReadDsp(MC_DIGITAL_STATE_DATA_B, 1, &usLogic);
	if (!bStatus)
		return FALSE;
	
	GetActualVelocity();// monitor the actual velocity
	GetMotorCurrent();// monitor the motor current
	GetMotorTac();// monitor the motor current
	
	if (m_ucPort == PORTA)
	{
		usLogic = usLogic  & 7;
		usDirectionMask = 0x0004;
		
	}
	else if (m_ucPort == PORTB)
	{
		usLogic = (usLogic >> 3) & 7;
		usDirectionMask = 0x0008;
	}
	else
		return FALSE;
	
	bFootPedalAssigned = (m_ucPort == m_pControl->FootswitchPortAssignment()) ;
	
	usPercent = m_tPortStatus.usPercent;
	
	wTotalDelay = m_tPortStatus.sNewLogicIdCnt + m_pControl->GetDelayPeriodMs();
	
	if (usLogic != m_tPortStatus.usPrevLogic)
	{
		if (( usLogic && wTotalDelay < NEW_DEVICE_DELAY_MS) ||
			( !usLogic && wTotalDelay < (NEW_DEVICE_DELAY_MS >>1)))
		{
            if (m_tPortStatus.qNonStableTypeInMs < TYPE_INSTABILITY_MS)
                m_tPortStatus.qNonStableTypeInMs += m_pControl->GetDelayPeriodMs();
            m_tPortStatus.qStableTypeInMs = 0;
			m_tPortStatus.sNewLogicIdCnt = wTotalDelay;
			return TRUE;
		}
	}
	
	m_tPortStatus.sNewLogicIdCnt = 0;

    switch (usLogic)
	{
	case LOGIC_ID_DRILL:
	case LOGIC_ID_SAW:
		// Figure out and set the type of the Powered Instrument
		
		switch (usLogic)
		{
		case LOGIC_ID_DRILL:
			usType = TYPE_DP_DRILL;
			ptBldc = &g_tBldcDpMicroAireDrill;
			usVelocity = DP_DRILL_MAX_VELOCITY;
			wDeadBand = DP_DRILL_DEAD_BAND;
			break;
		case LOGIC_ID_SAW:
			usType = TYPE_DP_SAW;
			ptBldc = &g_tBldcDpMicroAireSaw;
			usVelocity = DP_SAW_MAX_VELOCITY;
			wDeadBand = DP_SAW_DEAD_BAND;
			break;
		}
		
		if (usType != m_tPortStatus.usPrevType)
		{
			m_tPortStatus.sTriggerMax = 0;
			m_tPortStatus.sTriggerMin = DP_DRILL_SAW_START_MIN;
            m_tPortStatus.sTriggerDelay = TRIGGER_STABILITY_DELAY_MS;
		}
		
		// Figure out the direction if a drill is plugged in, the saw is always forward
		if (IS_TYPE_DRILL( usType))
		{
			if (!IS_TYPE_DRILL( m_tPortStatus.usPrevType))
			{
				// Enable the Drill Direction bit for this port
				bStatus = m_pControl->ReadDsp(MC_DIGITAL_ENABLE_MASK_E, 1, &wActive);
				if (!bStatus)
					return FALSE;
				
				wActive |= usDirectionMask; 	// Drill direction
				
                // Turn on the Drill Direction, (but do not read it this time), so the 
				// motor controller has time to set the value
				bStatus = m_pControl->WriteDsp(MC_DIGITAL_ENABLE_MASK_E, 1, &wActive);
				if (!bStatus)
					return FALSE;
			}
			else
			{
				// Read the Drill Direction
				bStatus = m_pControl->ReadDsp(MC_DIGITAL_STATE_DATA_E, 1, &usDirection);
				if (!bStatus)
					return FALSE;
				
				if (usDirection & usDirectionMask)
					usMode = MOTOR_REVERSE;
				else
					usMode = MOTOR_FORWARD;
				
				if ((m_tPortStatus.usFootMode != MOTOR_OFF) &&
					(m_tPortStatus.bRunning || (m_tPortStatus.usHandMode == MOTOR_OFF)))
				{	
					
				}
				else
				{
					// Set the mode here, we can read the direction when the drill is not running
					m_tPortStatus.usDisplayMode = usMode;
				}
			}
		} 
		else if( IS_TYPE_SAW(usType))
		{
			usMode = MOTOR_FORWARD;			
			m_tPortStatus.usDisplayMode	= MOTOR_OSCILLATE_1;	// Saw is always oscillate
		}
		
		if (m_bControlsEnabled)
		{
			// Read the Sensor to check for trigger pressure
			bStatus = ReadTrigger(wDeadBand, &usVelocity);
			if (!bStatus)
				return FALSE;
		}
		else
		{
			usVelocity = 0;
		}
		
		if (usVelocity == 0)
		{
			wActive = 0;
			usMode = MOTOR_OFF;
		}
		else
		{
			wActive = HAND_TRIGGER;
		}
		
		if(CheckHandPieceStuckBtn( wActive, usType, FALSE))
		{
			// Button depressed when Handpiece plugged in. Can not run
			// motor until condition goes away.
		}
		else
		{
			if ((m_tPortStatus.usFootMode != MOTOR_OFF) &&
				(m_tPortStatus.bRunning || (m_tPortStatus.usHandMode == MOTOR_OFF)))
			{	
				
			}
			else
			{
				m_tPortStatus.usHandMode = usMode;
				m_tPortStatus.usVelocity = usVelocity;
			}
		}
		break;// end 	case LOGIC_ID_DRILL: case LOGIC_ID_SAW:
		
		
	case LOGIC_ID_MDU_BASIC:
		// MDU BASIC

        // Skip HallBus read while running if the MDU is already known or there was timeout
        usDeviceExist = 0;
        if (m_tPortStatus.bRunning || m_tPortStatus.bHandCtrlTimeout)
        {
            switch (m_tPortStatus.usType)
            {
            case TYPE_MDU_STANDARD:
                usDeviceExist = HALL_MDU_STANDARD;
                break;
            case TYPE_MDU_STANDARD_CTL:
                usDeviceExist = HALL_MDU_STANDARD_CTL;
                SnWord usTmpDeviceExist = 0;

                // Check for read errors on Hand Controls
                bStatus = m_pControl->ReadDsp(m_tHallBusOffset.qDeviceExist, 1, &usTmpDeviceExist);
		        if (!bStatus)
			        return FALSE;

                if ((usTmpDeviceExist & HALL_MDU_CTL_MASK) != (usDeviceExist & HALL_MDU_CTL_MASK))
				{
                    // Report read warning on Hand Controls
                    TickHandCtrlReadErr();
                }
				else
				{
                    // Clear read warning on Hand Controls
                    ClearHandCtrlReadErr();
                }
                break;
            }
        }
        if (!usDeviceExist)
        {
		    bStatus = m_pControl->ReadDsp(m_tHallBusOffset.qDeviceExist, 1, &usDeviceExist);
		    if (!bStatus)
			    return FALSE;
        }

		// Figure out what kind of MDU this is
		switch (usDeviceExist & HALL_MDU_MASK)
		{
		case HALL_MDU_STANDARD:
			ptBldc = &g_tBldcPowerMax;
			bHallbusHandControls = FALSE;
			bSerialHandControls = FALSE;
			usType = TYPE_MDU_STANDARD;
			bMduFound = TRUE;
			m_tPortStatus.wHallDelay = 0;
			m_tPortStatus.yForceOscMode1 = FALSE;
			break;
		case HALL_MDU_STANDARD_CTL:
			ptBldc = &g_tBldcPowerMax;
			bHallbusHandControls = TRUE;
			bSerialHandControls = FALSE;
			usType = TYPE_MDU_STANDARD_CTL;
			bMduFound = TRUE;
			m_tPortStatus.wHallDelay = 0;
			m_tPortStatus.yForceOscMode1 = FALSE;
			break;
		default:
			// Hall Type Unknown
            if (m_tPortStatus.wHallDelay > wTotalDelay)
            {
				usType = m_tPortStatus.usPrevType;
				bHallOr485DelayActive = TRUE;
                m_tPortStatus.wHallDelay -= wTotalDelay;
				break;
			}
			m_tPortStatus.wHallDelay = 0;
			bHallOr485DelayActive = FALSE;
			
			if( (usDeviceExist & HALL_MDU_CTL_MASK) == 0x00)
				bHallbusHandControls = FALSE; // all 3 buttons missing no hand control
			else
				bHallbusHandControls = TRUE;
			
			bSerialHandControls = FALSE; 
			
			if((usDeviceExist & 0x23) == 0x00)
			{
				// Default to Standard High Speed Mdu parameters
				ptBldc = &g_tBldcPowerMax;
				usType = (bHallbusHandControls)? TYPE_MDU_STANDARD_CTL :TYPE_MDU_STANDARD;
				bMduFound = TRUE;
				usSafeMode = MOTOR_SAFEMODE_1;					// Curved blades only
			}
			else if((usDeviceExist & 0x23) == 0x02)
			{
				// Default to Standard High Speed Mdu parameters
				ptBldc = &g_tBldcPowerMax;
				usType = (bHallbusHandControls)? TYPE_MDU_STANDARD_CTL :TYPE_MDU_STANDARD;
				bMduFound = TRUE;
				usSafeMode = MOTOR_SAFEMODE_2;					// 10k burs to 8k burs, straight blades to curved
			}
			else if((usDeviceExist & 0x23) == 0x20)
			{
				// Default to Low Speed Mdu parameters
                ptBldc = &g_tBldcPowerMax;
				usType = (bHallbusHandControls)? TYPE_MDU_STANDARD_CTL :TYPE_MDU_STANDARD;
				bMduFound = TRUE;
				usSafeMode = MOTOR_SAFEMODE_3;					// Straight blades to curved
			}
			else if((usDeviceExist & 0x22) == 0x00)
			{
				// Use the Standard High Speed Mdu parameters
				ptBldc = &g_tBldcPowerMax;
				usType = (bHallbusHandControls)? TYPE_MDU_STANDARD_CTL :TYPE_MDU_STANDARD;
				bMduFound = TRUE;
				usSafeMode = MOTOR_SAFEMODE_4;
			}
			else
			{	
				// Use the Standard High Speed Mdu parameters
				ptBldc = &g_tBldcPowerMax;
				usType = (bHallbusHandControls)? TYPE_MDU_STANDARD_CTL :TYPE_MDU_STANDARD;
				bMduFound = TRUE;
				usSafeMode = MOTOR_SAFEMODE_1;					// Curved blades only
			}
			break;
		}
		
		break;// END MDU BASIC
		
		
	case LOGIC_ID_MDU_MINI:
		// MDU MINI
		ptBldc = &g_tBldcMini;
		bHallbusHandControls = FALSE;
		bSerialHandControls = FALSE;
		usType = TYPE_MDU_MINI;
		bMduFound = TRUE;
		m_tPortStatus.yForceOscMode1 = TRUE;
		break;
		
	case LOGIC_ID_MDU_ULTRA_IUR:
		// MDU ULTRA/IUR
		ptBldc = &g_tBldcHighTorque;
		bHallbusHandControls = FALSE;
		bSerialHandControls = FALSE;
		usType = TYPE_MDU_UTLRA_IUR;
		bMduFound = TRUE;
		m_tPortStatus.yForceOscMode1 = TRUE;
		break;
		
	case LOGIC_ID_RS485:
		if (m_tPortStatus.wRS485Delay )
		{	
			if (m_tPortStatus.wRS485Delay > wTotalDelay)
			{
				m_tPortStatus.wRS485Delay -= wTotalDelay;
				bHallOr485DelayActive = TRUE;
			}
			else
			{
				m_tPortStatus.wRS485Delay = 0;
				bHallOr485DelayActive = FALSE;
			}
			
			SnWord pwRequests[2];
			SnBool bStatus = FALSE;
			SnWord wType;
			
			if (m_tPortStatus.wRS485Delay <= RS485_HAND_DELAY_OFFSET_MS)
			{
				// Get the device Type and ID - 57600 baud
				pwRequests[0] = HAND_PORT_CMD(m_ucPort, SERIAL_CMD_DEV_TYPE);
				pwRequests[1] = HAND_PORT_CMD(m_ucPort, SERIAL_CMD_VERS);
				
				bStatus = m_pControl->SendSerialRequests(2, pwRequests, 4);
				
				if (!bStatus && m_pControl->GetFactoryMode() && !bHallOr485DelayActive)
				{
					// Get the device Type and ID - 19200 baud (Detect TYPE_MDU_TEST)
					pwRequests[0] = SLOW_HAND_PORT_CMD(m_ucPort, SERIAL_CMD_DEV_TYPE);
					pwRequests[1] = SLOW_HAND_PORT_CMD(m_ucPort, SERIAL_CMD_VERS);
					
					bStatus = m_pControl->SendSerialRequests(2, pwRequests, 4);
				}
			}
			if (!bStatus)
			{
				pwRequests[0] = 0;
				pwRequests[1] = 0;
			}
			else
			{
				bHallOr485DelayActive = FALSE;
			}
			
			wType = pwRequests[0] & 0x3ff;
			m_us485DeviceVersionNum = pwRequests[1] & 0x3ff;
			
            SnBool yRequestSerialNumber = FALSE;

			// Set the device type
			switch (wType) 
			{
			case RS485_TYPE_MDU_TEST:
				// Test Fixture
				usType = TYPE_MDU_TEST;
				break;
			case RS485_TYPE_MDU_RELIANT:
				// Reliant MDU
                usType = TYPE_MDU_RELIANT;
                if (m_tPortStatus.usPrevType != TYPE_MDU_RELIANT) {
                    bStatus = m_pControl->SendSerialRequestMotorTable(m_ucPort);
                    if (bStatus) {
                        yRequestSerialNumber = TRUE;
                    } else {
                        usType = TYPE_INVALID;
                    }
                }
				break;
			case RS485_TYPE_MDU_RELIANT_CTL:
				// Reliant MDU with Hand Controls
                usType = TYPE_MDU_RELIANT_CTL;
                if (m_tPortStatus.usPrevType != TYPE_MDU_RELIANT_CTL) {
                    bStatus = m_pControl->SendSerialRequestMotorTable(m_ucPort);
                    if (bStatus) {
                        yRequestSerialNumber = TRUE;
                    } else {
                        usType = TYPE_INVALID;
                    }
                }
				break;
			case RS485_TYPE_MDU_POWERMINI:
				// Small Joint POWERMINI MDU without Hand Controls
				usType = TYPE_MDU_POWERMINI;
				break;
			case RS485_TYPE_MDU_POWERMINI_CTL:
				// Small Joint POWERMINI MDU with Hand Controls
				usType = TYPE_MDU_POWERMINI_CTL;
				break;
			default:
				break;
			}

            if (yRequestSerialNumber) {
				bStatus = m_pControl->SendSerialRequestSerialNumber(m_ucPort);
                if (bStatus) {
                    char pcSerialNumber[SERIAL_NUMBER_STORE];
                    SnQByte qOffset = (m_ucPort == PORTA) ? MC_SERIAL_NUMBER_A : MC_SERIAL_NUMBER_B;

                    bStatus = m_pControl->ReadDsp(qOffset, SERIAL_NUMBER_STORE/2, (SnWord *)pcSerialNumber);
                    if (bStatus) {
                        strcpy(m_tPortStatus.pcSerialNumber, pcSerialNumber);
                        m_pControl->UpdateHandpieceSerialNumber(m_ucPort, pcSerialNumber);
                    }
                }
            }
			
			// Unknown device ID
			if (usType == TYPE_INVALID && !m_tWarningStatus.bUnknownDeviceId && !bHallOr485DelayActive)
			{
				m_tWarningStatus.bUnknownDeviceId = TRUE;
				SetEvent(m_tPortEvents.m_hUnknownDeviceIdEvent);
			}
		} 
		else
		{
			switch (m_tPortStatus.usPrevType) 
			{
			case TYPE_MDU_TEST:
			case TYPE_MDU_RELIANT:
			case TYPE_MDU_RELIANT_CTL:
			case TYPE_MDU_POWERMINI:
			case TYPE_MDU_POWERMINI_CTL:
				usType = m_tPortStatus.usPrevType;
				break;
			default:
				usType = TYPE_INVALID;
				m_tPortStatus.wRS485Delay = (RS485_HAND_SETTLE_DELAY_MS) * 4;
				break;
			}
		}
		
		switch (usType) 
		{
		case TYPE_MDU_TEST:
			ptBldc = &g_tBldcNoMotor;
			bMduFound = TRUE;
			m_tPortStatus.yForceOscMode1 = TRUE;
			break;
		case TYPE_MDU_RELIANT:
		case TYPE_MDU_RELIANT_CTL:
			ptBldc = 0;							// Motor Table is sent from MDU
			bHallbusHandControls = FALSE;
			bSerialHandControls = (usType == TYPE_MDU_RELIANT_CTL);
			bMduFound = TRUE;
			m_tPortStatus.yForceOscMode1 = FALSE;
			break;
		case TYPE_MDU_POWERMINI:
		case TYPE_MDU_POWERMINI_CTL:
			ptBldc = &g_tBldcSmallJoint; 
			bHallbusHandControls = FALSE;
			bSerialHandControls = (usType == TYPE_MDU_POWERMINI_CTL);
			bMduFound = TRUE;
			m_tPortStatus.yForceOscMode1 = FALSE;
			break;
		default:
			ptBldc = &g_tBldcNoMotor;
			bHallbusHandControls = FALSE;
			bSerialHandControls = FALSE;
			bMduFound = FALSE;
			m_tPortStatus.yForceOscMode1 = FALSE;
			break;
		}
		break; // END RS485

    case LOGIC_ID_RESERVED:
		// Device installed but we don't know what it is
		if( !m_tWarningStatus.bUnknownDeviceId)
		{
			m_tWarningStatus.bUnknownDeviceId = TRUE;
			SetEvent(m_tPortEvents.m_hUnknownDeviceIdEvent);
		}
        break;
		
    default:
        // No devices found
        m_tPortStatus.usHandMode = MOTOR_OFF;
        m_tPortStatus.usFootMode = MOTOR_OFF;
        m_tPortStatus.usDisplayMode = MOTOR_OFF;
        m_tPortStatus.usVelocity = 0;
        m_tPortStatus.bRunning = FALSE;

        // Device has been pulled out put the motor in no motor mode
        ptBldc = &g_tBldcNoMotor;

        // Reset Delays
        m_tPortStatus.wHallDelay = (HALL_SETTLE_DELAY_MS);		// Initialize Hall Settle Delay interval
        m_tPortStatus.wRS485Delay = (RS485_HAND_SETTLE_DELAY_MS);	// Initialize RS485 Settle Delay interval
        break;
	}	// End switch (usLogic)

    // Port Type has changed!
	if (usType != m_tPortStatus.usPrevType || (m_tPortStatus.usPrevLogic && !usLogic))
	{
		switch (m_tPortStatus.usPrevType) 
		{
			// Reset from old device
		case TYPE_DP_DRILL:
			// Disable the Drill Direction bit for this port
			bStatus = m_pControl->ReadDsp(MC_DIGITAL_ENABLE_MASK_E, 1, &wActive);
			if (!bStatus)
				return FALSE;
			
			wActive &= ~usDirectionMask; 	// Drill direction
			
			m_pControl->WriteDsp(MC_DIGITAL_ENABLE_MASK_E, 1, &wActive);
			break;
		case TYPE_MDU_TEST:
		case TYPE_MDU_RELIANT:
		case TYPE_MDU_RELIANT_CTL:
		case TYPE_MDU_POWERMINI:
		case TYPE_MDU_POWERMINI_CTL:
			wActive = DISABLE_HAND_PORT_CMD(m_ucPort);
			m_pControl->SendSerialRequest(&wActive, 1);
			m_tPortStatus.wRS485Delay = (RS485_HAND_SETTLE_DELAY_MS);	// RS485 Settle Delay interval in ms
			break;
        case TYPE_MDU_STANDARD:
        case TYPE_MDU_STANDARD_CTL:
		default:
			m_tPortStatus.wHallDelay = (HALL_SETTLE_DELAY_MS);          // Adjust Hall Settle Delay interval in ms
			break;
		}
		
		// Clear Port warnings 
		ClearAllMotorWarnings();			
		
		// Clear unknown device id warning if set
		if( usLogic != LOGIC_ID_RESERVED && m_tWarningStatus.bUnknownDeviceId )
		{
			m_tWarningStatus.bUnknownDeviceId = FALSE;
   			SetEvent(m_tPortEvents.m_hUnknownDeviceIdEvent);
        }

		// If a Short Circuit Test was already in progress then
		// restart it since the device id has changed.
		bStatus = ConfigureMotor(ptBldc); // Configure Motor
		if(!bStatus)
			return FALSE;

        if (bHallbusHandControls)
        {
            // Reset the latched Hand Controls 
			SnWord wHallbusLatch = 0;
			m_pControl->WriteDsp(m_tHallBusOffset.qDeviceLatch, 1, &wHallbusLatch);
        }
		else if (bSerialHandControls)
		{
            // Reset the Serial latched Hand Controls 
			m_tPortStatus.wSerialLatch = 0;
        }

		if( m_pControl->m_usShaverPacketControl == m_ucPort && IS_TYPE_MDU(m_tPortStatus.usPrevType))
			m_pControl->InitShaverPacket();
		
		m_tPortStatus.usType = usType;
    }
    else if (m_tPortStatus.qShortCiruitTestCnt)
    {
        SnWord usTestMode;
		
		// Wait for short circuit testing to complete
		bStatus = m_pControl->ReadDsp(m_tOffsetAddr.qMode, 1, &usTestMode);
		if(!bStatus)
			return FALSE;
		
        if (usTestMode != MOTOR_OFF)
        {
            if( m_tPortStatus.qShortCiruitTestCnt) 
			{
				if( m_tPortStatus.qShortCiruitTestCnt <= wTotalDelay)
				{
					m_tPortStatus.qShortCiruitTestCnt = 0;
					// Have a problem with the short circuit protection
					// the test timed out
					m_tWarningStatus.bMotorShortCircuitTimeout = TRUE;
					SetEvent(m_tPortEvents.m_hMotorShortCircuitTimeoutEvent);
					return FALSE;
				}
				else
				{
					m_tPortStatus.qShortCiruitTestCnt -= wTotalDelay;
				}
			}
        }
        else
        {
            m_tPortStatus.qShortCiruitTestCnt = 0;
			
			// Check for short circuit fault
			SnWord usError;
			bStatus = m_pControl->ReadDsp(m_tOffsetAddr.qFault, 1, &usError);
			
			if( usError != 0)
			{
				// Found a problem
				if( usError & ERROR_SHORT_24)
				{
					// Short Circuit
					m_tWarningStatus.bMotorShortCircuit = TRUE;
					SetEvent(m_tPortEvents.m_hMotorShortCircuitEvent);
					return FALSE;
				}	
			}
			
			// Motor has been successfully configured
			WriteDwell();
			
            m_pControl->SetOscProfile(m_tPortStatus.usRevolutions, m_ucPort);
			m_tPortStatus.bMotorReady = TRUE;
        }
    }
	
	m_tPortStatus.usPrevLogic = usLogic;
	
	if( usLogic && !m_tPortStatus.bMotorReady && !m_tWarningStatus.bMotorStallAndCurrentLimit)
	{
		// The motor is not ready or the motor has been shut down because of a fault
		// Reset the Hall latched Hand Controls 
		SnWord wHallbusLatch = 0;
		m_pControl->WriteDsp(m_tHallBusOffset.qDeviceLatch, 1, &wHallbusLatch);
		
		// Reset the Serial latched Hand Controls 
		m_tPortStatus.wSerialLatch = 0;
	}
	
	if (usSafeMode != m_tPortStatus.usSafeMode)
	{
		m_tPortStatus.usSafeMode = usSafeMode;

		// Set/Clear Hallbus Motor Id fault based upon safemode state and Hand Control Timeout
		m_tWarningStatus.bHallPatternFault = (usSafeMode != MOTOR_SAFEMODE_OFF) ||
            m_tPortStatus.bHandCtrlTimeout;
		SetEvent(m_tPortEvents.m_hHallPatternFaultEvent);
	}

	if( bMduFound) 
	{
		
		// Check footpedal status
		if( !bHallbusHandControls && !bSerialHandControls && !bFootPedalAssigned)
		{
			if (!m_tWarningStatus.bFootswitchRequired)
			{
				// Mdu needs a footswitch send a warning
				m_tWarningStatus.bFootswitchRequired = TRUE;
				SetEvent( m_tPortEvents.m_hFootswitchRequiredEvent);
			}
		}
		else
		{
			// Clear error if it's set
			if( m_tWarningStatus.bFootswitchRequired)
			{
				m_tWarningStatus.bFootswitchRequired = FALSE;
				SetEvent( m_tPortEvents.m_hFootswitchRequiredEvent);
			}
		}
		
		// Hall bus device
		if (bHallbusHandControls)
		{
			// Mdu device has been found
			SnWord wHallbusLatch;
			
			// If there is no Hand Control Timeout:
			//   - Decode the Hallbus Hand Controls if enabled 
			//   - Decode if no FootPedal device is found or 
			//     a FootPedal is found but the Hand Control Overide is disabled.
			
			if ((!bFootPedalAssigned || (bFootPedalAssigned && !bFootPedalOverride) ||
				usType != m_tPortStatus.usPrevType) && !m_tPortStatus.bHandCtrlTimeout)
			{
				SnWord wActive = 0;

				// See if any buttons are depressed
				bStatus = m_pControl->ReadDsp(m_tHallBusOffset.qDeviceLatch, 1, &wHallbusLatch);
				if (!bStatus)
					return FALSE;
				// Use Device Active to determine if the button remains depressed
				SnWord wHallbusActive;
				bStatus = m_pControl->ReadDsp(m_tHallBusOffset.qDeviceActive, 1, &wHallbusActive);
				if (!bStatus)
					return FALSE;

				if( wHallbusActive	& HALL_MDU_CTL_FWD)
					wActive |= HAND_BUTTON_FWD;
				if( wHallbusActive	& HALL_MDU_CTL_REV)
					wActive |= HAND_BUTTON_REV;
				if( wHallbusActive	& HALL_MDU_CTL_OSC)
					wActive |= HAND_BUTTON_OSC;
				
				m_tPortStatus.bHandButtonState = wActive;
				
				if ((wHallbusLatch & HALL_MDU_CTL_REV) && !(m_tPortStatus.wStuckButton & HAND_BUTTON_REV) )
				{
					usMode = MOTOR_REVERSE;
					
					if(wHallbusActive  & HALL_MDU_CTL_REV)
						m_tPortStatus.usReverseDownCount++;
					else if(m_tPortStatus.bHandModeDualOn)
						m_tPortStatus.usReverseDownCount = 0;
					
					if( m_tPortStatus.usReverseDownCount == 30)
					{
						// Button has been depressed for 1 sec. 
						// Dual handpiece button control has just been activated
						m_tPortStatus.bHandModeDualOn = TRUE;
						
						PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS, (WPARAM)MSG_UPDATE_HANDPIECE_DUAL_RPM_ON,
							(LPARAM)m_ucPort);
					}
					if( m_tPortStatus.usReverseDownCount == 60)
					{
						// Button has been depressed for 2 sec. 
						// Dual handpiece button control has just been activated
						m_tPortStatus.bHandModeDualOn = FALSE;
						
						PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS, (WPARAM)MSG_UPDATE_HANDPIECE_DUAL_RPM_OFF,
							(LPARAM)m_ucPort);
						
						m_tPortStatus.usReverseDownCount = 0;
					}
					
				}
				else if ((wHallbusLatch & HALL_MDU_CTL_FWD) && !(m_tPortStatus.wStuckButton & HAND_BUTTON_FWD))
				{
					if(wHallbusActive  & HALL_MDU_CTL_FWD)
						m_tPortStatus.usForwardDownCount++;
					else
						m_tPortStatus.usForwardDownCount = 0;
					
					usMode = MOTOR_FORWARD;
					
					if( m_tPortStatus.usForwardDownCount == 30)
					{
						// Button has been depressed for 1 sec. 
						// Dual handpiece button control has just been activated
						m_tPortStatus.bHandModeDualOn = TRUE;
						
						PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS, (WPARAM)MSG_UPDATE_HANDPIECE_DUAL_RPM_ON,
							(LPARAM)m_ucPort);
					}
					if( m_tPortStatus.usForwardDownCount == 60)
					{
						// Button has been depressed for 2 sec. 
						// Dual handpiece button control has just been activated
						m_tPortStatus.bHandModeDualOn = FALSE;
						
						PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS, (WPARAM)MSG_UPDATE_HANDPIECE_DUAL_RPM_OFF,
							(LPARAM)m_ucPort);
						
						m_tPortStatus.usForwardDownCount = 0;
					}
					
				}
				else if ((wHallbusLatch & HALL_MDU_CTL_OSC) && !(m_tPortStatus.wStuckButton & HAND_BUTTON_OSC))
				{
					// Oscillate Mode
					if( !m_tPortStatus.yForceOscMode1 && m_tPortStatus.usOscMode == OSC_MODE2)
						usMode = MOTOR_OSCILLATE_2;
					else
						usMode = MOTOR_OSCILLATE_1;
					
					if(wHallbusActive  & HALL_MDU_CTL_OSC)
						m_tPortStatus.usOscillateDownCount++;
					else if(m_tPortStatus.bHandModeDualOn)
					{
						m_tPortStatus.usOscillateDownCount = 0;
						
						usMode = MOTOR_OFF;
						
						m_tPortStatus.bHandModeDualOn = FALSE;
						
						// Clear the Hall bus latch
						wHallbusLatch = 0;
						m_pControl->WriteDsp(m_tHallBusOffset.qDeviceLatch, 1, &wHallbusLatch);
					}
					
					if( m_tPortStatus.usOscillateDownCount >= 30)
					{
						// Button has been depressed for 1 sec. 
						// Dual handpiece control has just been activated
						usMode = MOTOR_WINDOW_LOCK;
						m_tPortStatus.bHandModeDualOn = TRUE;
					}
				}
				else
				{
					usMode = MOTOR_OFF;
					// No buttons down, clear all counts
					m_tPortStatus.usForwardDownCount = 0;
					m_tPortStatus.usReverseDownCount = 0;
					m_tPortStatus.usOscillateDownCount = 0;
					
					
					if( m_tPortStatus.bHandModeDualOn)
					{
						SnBool yUpdatePortSavedParams = FALSE;
						SnWord usRpm2;
						SnWord usRpm;
						
						if( m_tPortStatus.usPrevHandMode == MOTOR_REVERSE)
						{
							usRpm = m_tPortStatus.usReverse;
							usRpm2 = m_tPortStatus.usReverse2;
							// Swap 
							m_tPortStatus.usReverse = usRpm2;
							m_tPortStatus.usReverse2 = usRpm;
							
							yUpdatePortSavedParams = TRUE;
						}
						else if( m_tPortStatus.usPrevHandMode == MOTOR_FORWARD)
						{
							usRpm = m_tPortStatus.usForward;
							usRpm2 = m_tPortStatus.usForward2;
							// Swap 
							m_tPortStatus.usForward = usRpm2;
							m_tPortStatus.usForward2 = usRpm;
							
							yUpdatePortSavedParams = TRUE;
						}
						
						if (yUpdatePortSavedParams)
							SetPortCustomSpeeds();
						
						m_tPortStatus.bHandModeDualOn = FALSE;
						// Disable handpiece dual button control
						PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS, (WPARAM)MSG_UPDATE_HANDPIECE_DUAL_RPM_OFF,
							(LPARAM)m_ucPort);
					}
				}
			
				if(CheckHandPieceStuckBtn( wActive, usType, (bFootPedalAssigned && bFootPedalOverride) ))
				{
					// Button depressed when Handpiece plugged in. Can not run
					// motor until condition goes away.
				}

                // Only check Blade if there was no error in reading Hand Controls / Blade present
				else if (m_tPortStatus.qHandCtrlErrTime == 0)
				{

					// Get the Blade Family for Hallbus Devices				
					m_tPortStatus.usBladeId = HALL_TO_BLADE(wHallbusActive);					
					m_tPortStatus.usBlade = CheckBladeType(m_tPortStatus.usBladeId);// Blade type
					
					//
					// If the blade is changed or Controls are disabled,
					// reset the latched hall values and make sure the motor is off
					//
					if ((m_tPortStatus.usBlade != m_tPortStatus.usPrevBlade)|| (!m_bControlsEnabled))
					{
						// Reset the latched Hand Controls 
						SnWord wHallbusLatch = 0;
						m_pControl->WriteDsp(m_tHallBusOffset.qDeviceLatch, 1, &wHallbusLatch);
						usMode = MOTOR_OFF;
					}
										
					if((bFootPedalAssigned && m_tPortStatus.bRunning && m_tPortStatus.usFootMode != MOTOR_OFF) || m_bWindowLock)
					{
						// Motor already running return
						// Reset the latched Hand Controls 
						SnWord wHallbusLatch = 0;
						m_pControl->WriteDsp(m_tHallBusOffset.qDeviceLatch, 1, &wHallbusLatch);
						
						return TRUE;
					}
					else if (m_tPortStatus.usHandMode != usMode)
						m_tPortStatus.usHandMode = usMode;
				}
			}
			else
			{
				// If we can only use the footpedal get the Blade Family in case it changed
				SnWord wHallbusActive;
				SnWord wActive = 0;
				
				bStatus = m_pControl->ReadDsp(m_tHallBusOffset.qDeviceActive, 1, &wHallbusActive);
				if (!bStatus)
					return FALSE;
				
				if( wHallbusActive	& HALL_MDU_CTL_FWD)
					wActive |= HAND_BUTTON_FWD;
				if( wHallbusActive	& HALL_MDU_CTL_REV)
					wActive |= HAND_BUTTON_REV;
				if( wHallbusActive	& HALL_MDU_CTL_OSC)
					wActive |= HAND_BUTTON_OSC;
				
				m_tPortStatus.bHandButtonState = wActive;
				
				// Clear Stuck Button Alarm while assigned footpedal override is active
				CheckHandPieceStuckBtn( wActive, usType, (bFootPedalAssigned && bFootPedalOverride));
				
				m_tPortStatus.usBladeId = HALL_TO_BLADE(wHallbusActive);					
				m_tPortStatus.usBlade = CheckBladeType(m_tPortStatus.usBladeId);// Blade type			
				
				// Reset the latched Hand Controls while using the Footswitch
				wHallbusLatch = 0;
				m_pControl->WriteDsp(m_tHallBusOffset.qDeviceLatch, 1, &wHallbusLatch);
			}
		}										// 		end if (bHallbusHandControls)

		else if (( usType == TYPE_MDU_POWERMINI)|| ( usType == TYPE_MDU_POWERMINI_CTL))
		{
			// If there is no Hand Control Timeout:
			//   - Decode if no FootPedal device is found or 
			//     a FootPedal is found but the Hand Control Overide is disabled.
            SnWord pwRequests[3];
			SnBool bStatus = TRUE;
			
			if ((!bFootPedalAssigned || (bFootPedalAssigned && !bFootPedalOverride) ||
				usType != m_tPortStatus.usPrevType) && !m_tPortStatus.bHandCtrlTimeout)
			{
				SnWord wActive = 0;
				int buttonTemp = 0;
                
 				if (usType != m_tPortStatus.usPrevType)
					m_tPortStatus.wSerialPrevActive = 0;
				
                // Blade Type & Motor Operation
                pwRequests[0] = HAND_PORT_CMD(m_ucPort, SERIAL_CMD_REQ_5);
                
                // Get the Mode of operation for the Serial Device from MDU.out
                //  and get blade type from the Small Joint MDU.Out message
				
               if (usType == TYPE_MDU_POWERMINI_CTL)
                {
                    pwRequests[1] = HAND_PORT_CMD(m_ucPort, SERIAL_CMD_REQ_6);
                    pwRequests[2] = HAND_PORT_CMD(m_ucPort, SERIAL_CMD_REQ_3);
                    bStatus = m_pControl->SendSerialRequests(3, pwRequests, 4);
                    if (bStatus == FALSE)
                    {
                        pwRequests[0] = 0;
                        pwRequests[1] = 0;
                        pwRequests[2] = 0;
                        wActive = m_tPortStatus.wSerialPrevActive;
                        usMode = m_tPortStatus.usHandMode;

                        // Report read warning on Hand Controls
                        TickHandCtrlReadErr();
                    }
                    else
                    {
                        if( ~pwRequests[1] & SMJ_BUTTON_DISTAL)
                            wActive |= HAND_BUTTON_FWD;
                        if( ~pwRequests[1] & SMJ_BUTTON_PROXIMAL)
                            wActive |= HAND_BUTTON_OSC;

                        if (pwRequests[2] & (SMJ_MOTOR_FORWARD |SMJ_MOTOR_REVERSE))
                            buttonTemp |= HAND_BUTTON_FWD;
                        if (pwRequests[2] & (SMJ_MOTOR_OSCILLATE |SMJ_MOTOR_WINDOW_LOCK))
                            buttonTemp |= HAND_BUTTON_OSC;
 
                        // Clear read warning on Hand Controls
                        ClearHandCtrlReadErr();

                   }
                }
                else
                {
                    bStatus = m_pControl->SendSerialRequests(1, pwRequests, 4);
                    if ( bStatus == FALSE)
                    {
                        pwRequests[0] = 0;
                    }
                }
                
                if(bStatus)
                {
                    switch( pwRequests[0] & 0xf)
                    {
                    case SMJ_MOTOR_OFF:
                        usMode = MOTOR_OFF;
                        break;
						
                    case SMJ_MOTOR_FORWARD:
                        usMode = MOTOR_FORWARD;
                        break;
						
                    case SMJ_MOTOR_REVERSE:
                        usMode = MOTOR_REVERSE;
                        break;
						
                    case SMJ_MOTOR_OSCILLATE:						
                        if( !m_tPortStatus.yForceOscMode1 && m_tPortStatus.usOscMode == OSC_MODE2)
                            usMode = MOTOR_OSCILLATE_2;
                        else
                            usMode = MOTOR_OSCILLATE_1;
                        break;
                        
                    case SMJ_MOTOR_WINDOW_LOCK:
                        usMode = MOTOR_WINDOW_LOCK;
                        break;
                        
                    default:
                        usMode = MOTOR_OFF;
                        break;
                    }
					
					if (buttonTemp)
                    {
                        m_tPortStatus.bHandButtonState &= wActive;
                        m_tPortStatus.bHandButtonState |= buttonTemp;
                    }
                    else
                        m_tPortStatus.bHandButtonState = wActive;
				}		
				
				if (CheckHandPieceStuckBtn(wActive, usType, (bFootPedalAssigned && bFootPedalOverride)))
				{
					// Button depressed when Handpiece plugged in. Can not run
					// motor until condition goes away.
				}
				else
				{
					// Get the Blade Type for Serial Device
					if(bStatus)
					{
                        m_tPortStatus.usBladeId = pwRequests[0] & SMJ_BLADE_TYPE_MASK;
                        m_tPortStatus.usBlade = PowerMiniCheckBladeType(m_tPortStatus.usBladeId); // Blade type
                        m_tPortStatus.usBladeId >>= 4;
					}
					//
					// If the blade is changed or motor control is not enabled,
					// reset the latched serial values and make sure the motor is off
					//
					if( (m_tPortStatus.usBlade != m_tPortStatus.usPrevBlade) || 
						!m_bControlsEnabled )
					{
						if (usMode != MOTOR_OFF)
						{
							SnWord wResetCmd = HAND_PORT_CMD(m_ucPort, SERIAL_CMD_REQ_8);
							// Issue the reset command to clear the pressed button status
							if (m_pControl->SendSerialRequests(1, &wResetCmd, 4) == FALSE)
							{
								wResetCmd = 0;
							}
							usMode = MOTOR_OFF;
						}
						// Reset the Serial latched Hand Controls 
						m_tPortStatus.wSerialLatch = 0;
					}
					
					if ((bFootPedalAssigned && m_tPortStatus.bRunning && m_tPortStatus.usFootMode != MOTOR_OFF) || m_bWindowLock)
					{
						//	Motor already running reset buttons							
						if (usMode != MOTOR_OFF)
						{
							SnWord wResetCmd = HAND_PORT_CMD(m_ucPort, SERIAL_CMD_REQ_8);
							// Issue the reset command to clear the pressed button status
							if (m_pControl->SendSerialRequests(1, &wResetCmd, 4) == FALSE)
							{
								wResetCmd = 0;
							}
							usMode = MOTOR_OFF;
						}
						// Reset the Serial latched Hand Controls 
						m_tPortStatus.wSerialLatch = 0;
					}

					m_tPortStatus.usHandMode = usMode;
				}	
            } 
			else
			{
				// Get the Blade Type for Serial Device and make certain handpiece commands are disabled
                pwRequests[0] = HAND_PORT_CMD(m_ucPort, SERIAL_CMD_REQ_5);
                pwRequests[1] = HAND_PORT_CMD(m_ucPort, SERIAL_CMD_REQ_8);
                
                // Small Joint POWERMINI MDU read MDU.Out to get the blade type
                // and reset the mdu in case any of the buttons have been pressed
                if (m_pControl->SendSerialRequests(2, pwRequests, 4))
                {
                        m_tPortStatus.usBladeId = pwRequests[0] & SMJ_BLADE_TYPE_MASK;
                        m_tPortStatus.usBlade = PowerMiniCheckBladeType(m_tPortStatus.usBladeId); // Blade type
                        m_tPortStatus.usBladeId >>= 4;
                }
				m_tPortStatus.wSerialLatch = 0;
				m_tPortStatus.usHandMode = MOTOR_OFF;
			}
		}							// end if for type POWERMINI MDU with & without Hand Controls

		else if (( usType == TYPE_MDU_RELIANT)||( usType == TYPE_MDU_RELIANT_CTL))
		{
			// If there is no Hand Control Timeout:
			//   - Decode if no FootPedal device is found or 
			//     a FootPedal is found but the Hand Control Overide is disabled.
            SnWord pwRequests[3];
			SnBool bStatus = TRUE;
			
			if ((!bFootPedalAssigned || (bFootPedalAssigned && !bFootPedalOverride) ||
				usType != m_tPortStatus.usPrevType) && !m_tPortStatus.bHandCtrlTimeout)
			{
 				SnWord wActive = 0;
			
				if (usType != m_tPortStatus.usPrevType)
					m_tPortStatus.wSerialPrevActive = 0;
				
				// Request Blade Type. Uses two commands that return the same information.
                //     SERIAL_CMD_REQ_2 - Sent when the MDU is not running
                //     SERIAL_CMD_REQ_3 - Sent when the MUD is running
                pwRequests[0] = HAND_PORT_CMD(m_ucPort, m_tPortStatus.bRunning ? SERIAL_CMD_REQ_3 : SERIAL_CMD_REQ_2);
                               
                if ( usType == TYPE_MDU_RELIANT_CTL)
                {
                    //Get qMotorRunTime in seconds for Reliant MDU
                    SnWord newPwRequests[2];
                    newPwRequests[0] = HAND_PORT_CMD(m_ucPort, SERIAL_CMD_REQ_6);
                    newPwRequests[1] = HAND_PORT_CMD(m_ucPort, SERIAL_CMD_REQ_7);

                    bStatus = m_pControl->SendSerialRequests(2, newPwRequests, 4);
                    if (bStatus == FALSE)
                    {
                        newPwRequests[0] = 0;
                        newPwRequests[1] = 0;
                    }
                    else 
                    {
                        SnQByte qMotorRunTime = ((newPwRequests[0] & 0x3FF) << 10);
                        qMotorRunTime |= (newPwRequests[1] & 0x3FF);
                        m_tPortStatus.qMotorRunTime = qMotorRunTime; 
                    }

                    //Get Shaver PowerOnCount
                    newPwRequests[0] = HAND_PORT_CMD(m_ucPort, SERIAL_CMD_REQ_5);
                    newPwRequests[1] = 0;
                    
                    bStatus = m_pControl->SendSerialRequests(1, newPwRequests, 4);
                    if (bStatus == FALSE)
                    {
                        newPwRequests[0] = 0;
                        newPwRequests[1] = 0;
                    }
                    else 
                    {
                        m_tPortStatus.qShaverPowerOnCount = (newPwRequests[0] & 0x3FF); 
                    }

                    // Get the Switch State for Serial Device
                    pwRequests[1] = HAND_PORT_CMD(m_ucPort, SERIAL_CMD_REQ_4);
                    bStatus = m_pControl->SendSerialRequests(2, pwRequests, 4);
                    if ( bStatus == FALSE)
                    {
                        pwRequests[0] = 0;
                        pwRequests[1] = 0;
                        wActive = m_tPortStatus.wSerialPrevActive;

                        // Report read warning on Hand Controls
                        TickHandCtrlReadErr();
                    }
                    else
                    {
                        if( pwRequests[1]  & SERIAL_MDU_CTL_FWD)
                            wActive |= HAND_BUTTON_FWD;
                        if( pwRequests[1]  & SERIAL_MDU_CTL_REV)
                            wActive |= HAND_BUTTON_REV;
                        if( pwRequests[1]  & SERIAL_MDU_CTL_OSC)
                            wActive |= HAND_BUTTON_OSC;
 
                        // Clear read warning on Hand Controls
                        ClearHandCtrlReadErr();
					}
                }
                else
                {
                    bStatus = m_pControl->SendSerialRequests(1, pwRequests, 4);
                    if (bStatus == FALSE)
                    {
                        pwRequests[0] = 0;
                    }
                }
                
                m_tPortStatus.bHandButtonState = wActive;
                
                SnWord wTmp = (wActive ^ m_tPortStatus.wSerialPrevActive) & wActive;

                if (wTmp)
                {
                    if (m_tPortStatus.wSerialLatch)
                    {
                        m_tPortStatus.wSerialLatch = 0;
                    }
                    else 
                    {
                        m_tPortStatus.wSerialLatch = wTmp ^ (m_tPortStatus.wSerialLatch) & wTmp;
                    }
                }
                m_tPortStatus.wSerialPrevActive = wActive;
                
                if ((m_tPortStatus.wSerialLatch & HAND_BUTTON_REV) && !(m_tPortStatus.wStuckButton & HAND_BUTTON_REV))
                {
                    usMode = MOTOR_REVERSE;
                    
                    if(wActive	& HAND_BUTTON_REV)
                        m_tPortStatus.usReverseDownCount++;
                    else if(m_tPortStatus.bHandModeDualOn)
                        m_tPortStatus.usReverseDownCount = 0;
                    
                    if( m_tPortStatus.usReverseDownCount == 30)
                    {
                        // Button has been depressed for 1 sec. 
                        // Dual handpiece button control has just been activated
                        m_tPortStatus.bHandModeDualOn = TRUE;
                        
                        PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS, (WPARAM)MSG_UPDATE_HANDPIECE_DUAL_RPM_ON,
                            (LPARAM)m_ucPort);
                    }
                    if( m_tPortStatus.usReverseDownCount == 60)
                    {
                        // Button has been depressed for 2 sec. 
                        // Dual handpiece button control has just been activated
                        m_tPortStatus.bHandModeDualOn = FALSE;
                        
                        PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS, (WPARAM)MSG_UPDATE_HANDPIECE_DUAL_RPM_OFF,
                            (LPARAM)m_ucPort);
                        
                        m_tPortStatus.usReverseDownCount = 0;
                    }
                }
                else if ((m_tPortStatus.wSerialLatch & HAND_BUTTON_FWD) && !(m_tPortStatus.wStuckButton & HAND_BUTTON_FWD) )
                {
                    usMode = MOTOR_FORWARD;
                    
                    if(wActive	& HAND_BUTTON_FWD)
                        m_tPortStatus.usForwardDownCount++;
                    else
                        m_tPortStatus.usForwardDownCount = 0;
                    
                    if( m_tPortStatus.usForwardDownCount == 30)
                    {
                        // Button has been depressed for 1 sec. 
                        // Dual handpiece button control has just been activated
                        m_tPortStatus.bHandModeDualOn = TRUE;
                        
                        PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS, (WPARAM)MSG_UPDATE_HANDPIECE_DUAL_RPM_ON,
                            (LPARAM)m_ucPort);
                    }
                    if( m_tPortStatus.usForwardDownCount == 60)
                    {
                        // Button has been depressed for 2 sec. 
                        // Dual handpiece button control has just been activated
                        m_tPortStatus.bHandModeDualOn = FALSE;
                        
                        PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS, (WPARAM)MSG_UPDATE_HANDPIECE_DUAL_RPM_OFF,
                            (LPARAM)m_ucPort);
                        
                        m_tPortStatus.usForwardDownCount = 0;
                    }
                }
                else if ((m_tPortStatus.wSerialLatch & HAND_BUTTON_OSC) && !(m_tPortStatus.wStuckButton & HAND_BUTTON_OSC))
                {
                    // Oscillate Mode
                    if( !m_tPortStatus.yForceOscMode1 && m_tPortStatus.usOscMode == OSC_MODE2)
                        usMode = MOTOR_OSCILLATE_2;
                    else
                        usMode = MOTOR_OSCILLATE_1;
                    
                    if(wActive	& HAND_BUTTON_OSC)
                        m_tPortStatus.usOscillateDownCount++;
                    else if(m_tPortStatus.bHandModeDualOn)
                    {
                        m_tPortStatus.usOscillateDownCount = 0;
                        
                        usMode = MOTOR_OFF;
                        
                        m_tPortStatus.bHandModeDualOn = FALSE;
                        
                        // Clear the Serial latch
                        m_tPortStatus.wSerialLatch = 0;
                    }
                    
                    if( m_tPortStatus.usOscillateDownCount >= 30)
                    {
                        // Button has been depressed for 1 sec. 
                        // Dual handpiece control has just been activated
                        usMode = MOTOR_WINDOW_LOCK;
                        m_tPortStatus.bHandModeDualOn = TRUE;
                    }
                }
                else
                {
                    usMode = MOTOR_OFF;
                    // No buttons down, clear all counts
                    m_tPortStatus.usForwardDownCount = 0;
                    m_tPortStatus.usReverseDownCount = 0;
                    m_tPortStatus.usOscillateDownCount = 0;
                    
                    if( m_tPortStatus.bHandModeDualOn)
                    {
                        SnBool yUpdatePortSavedParams = FALSE;
                        SnWord usRpm2;
                        SnWord usRpm;
                        
                        if( m_tPortStatus.usPrevHandMode == MOTOR_REVERSE)
                        {
                            usRpm = m_tPortStatus.usReverse;
                            usRpm2 = m_tPortStatus.usReverse2;
                            // Swap 
                            m_tPortStatus.usReverse = usRpm2;
                            m_tPortStatus.usReverse2 = usRpm;
                            
                            yUpdatePortSavedParams = TRUE;
                        }
                        else if( m_tPortStatus.usPrevHandMode == MOTOR_FORWARD)
                        {
                            usRpm = m_tPortStatus.usForward;
                            usRpm2 = m_tPortStatus.usForward2;
                            // Swap 
                            m_tPortStatus.usForward = usRpm2;
                            m_tPortStatus.usForward2 = usRpm;
                            
                            yUpdatePortSavedParams = TRUE;
                        }
                        
                        if (yUpdatePortSavedParams)
                            SetPortCustomSpeeds();
                        
                        m_tPortStatus.bHandModeDualOn = FALSE;
                        // Disable handpiece dual button control
                        PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS, (WPARAM)MSG_UPDATE_HANDPIECE_DUAL_RPM_OFF,
                            (LPARAM)m_ucPort);
                    }
                }
				
				if (CheckHandPieceStuckBtn(wActive, usType, (bFootPedalAssigned && bFootPedalOverride)))
				{
					// Button depressed when Handpiece plugged in. Can not run
					// motor until condition goes away.
				}
				else
				{
					// Get the Blade Type for Serial Device
					if(bStatus)
					{
                        m_tPortStatus.usBladeId = pwRequests[0] & 0x3ff;
                        m_tPortStatus.usBlade = CheckBladeType(m_tPortStatus.usBladeId); // Blade type
					}
					//
					// If the blade is changed or motor control is not enabled,
					// reset the latched serial values and make sure the motor is off
					//
					if ((m_tPortStatus.usBlade != m_tPortStatus.usPrevBlade) || 
						(!m_bControlsEnabled && (usMode != MOTOR_OFF)))
					{
                        m_tPortStatus.wSerialLatch = 0;
						usMode = MOTOR_OFF;
					}
					
					if ((bFootPedalAssigned && m_tPortStatus.bRunning && m_tPortStatus.usFootMode != MOTOR_OFF) || m_bWindowLock)
					{
						//	Motor already running return
						
						// Reset the Serial latched Hand Controls 
						m_tPortStatus.wSerialLatch = 0;
						usMode = MOTOR_OFF;
					}

					m_tPortStatus.usHandMode = usMode;
				}	
            } 
			else
			{
				// Get the Blade Type for Serial Device
                pwRequests[0] = HAND_PORT_CMD(m_ucPort, SERIAL_CMD_REQ_2);
                if (m_pControl->SendSerialRequests(1, pwRequests, 4))
                {
                    m_tPortStatus.usBladeId = pwRequests[0] & 0x3ff;
                    m_tPortStatus.usBlade = CheckBladeType(m_tPortStatus.usBladeId); // Blade type
                }
				m_tPortStatus.wSerialLatch = 0;					
				m_tPortStatus.usHandMode   = MOTOR_OFF;
			}
		}							// end if for type FAST MDU with & without Hand Controls
		
		else if (usType == TYPE_MDU_STANDARD)
		{
			
			// Get the Blade Family for the non-hand controls hallbus case
			SnWord wHallbusActive;
			bStatus = m_pControl->ReadDsp(m_tHallBusOffset.qDeviceActive, 1, &wHallbusActive);
			if (!bStatus)
				return FALSE;
			
			m_tPortStatus.usBladeId = HALL_TO_BLADE(wHallbusActive);					
			m_tPortStatus.usBlade = CheckBladeType(m_tPortStatus.usBladeId);// Blade type
			
			m_tPortStatus.bHandButtonState = 0;
		}
		else if (usType == TYPE_MDU_UTLRA_IUR || usType == TYPE_MDU_MINI)
		{
			m_tPortStatus.usBlade = BLADE_TYPE_OTHER;
			m_tPortStatus.bHandButtonState = 0;
		}
		
		if(bFootPedalAssigned && bFootPedalOverride)
		{
		}
		else if((usMode == MOTOR_OFF) && (m_tPortStatus.usFootMode != MOTOR_OFF) &&
			( m_tPortStatus.bHandInCtl == FALSE))
		{
		}
		else
		{
			switch (usMode)
			{
			case MOTOR_REVERSE:
				if( m_tPortStatus.bHandModeDualOn)
					m_tPortStatus.usVelocity = m_tPortStatus.usReverse2;
				else
					m_tPortStatus.usVelocity = m_tPortStatus.usReverse;
				break;

			case MOTOR_FORWARD:
				if( m_tPortStatus.bHandModeDualOn)
					m_tPortStatus.usVelocity = m_tPortStatus.usForward2;
				else
					m_tPortStatus.usVelocity = m_tPortStatus.usForward;
				break;

			case MOTOR_OSCILLATE_1:
				m_tPortStatus.usVelocity = m_tPortStatus.usOscillateRpm;
				break;

			case MOTOR_OSCILLATE_2:
				m_tPortStatus.wPeriod = m_tPortStatus.wOscillateSeconds;
				break;
				
			case MOTOR_WINDOW_LOCK:
				m_tPortStatus.usVelocity = 0;
				break;
				
			case MOTOR_OFF:
				m_tPortStatus.usVelocity = 0;
				break;
			}
		}

        // If MDU is running and the Hand Control read Timeout is reached, then stop the MDU
        // and prevent further use of Hand Controls until the MDU is unplugged.
        if (m_tPortStatus.bRunning && m_tPortStatus.qHandCtrlErrTime >= HAND_CTRL_READ_TIMEOUT
            && !m_tPortStatus.bHandCtrlTimeout)
        {
            m_tPortStatus.bHandCtrlTimeout = TRUE;
            m_tPortStatus.usHandMode = MOTOR_OFF;
        }
	}
	else
	{
		if (m_tPortStatus.usBlade != BLADE_TYPE_UNKNOWN)
		{
			m_tPortStatus.usBlade = BLADE_TYPE_UNKNOWN;
			m_tPortStatus.usBladeId = BLADE_TYPE_UNKNOWN;
		}
		m_tPortStatus.bHandButtonState = 0;

		// Clear Hand Control Timeout
        m_tPortStatus.bHandCtrlTimeout = FALSE;
        m_tPortStatus.qHandCtrlErrTime = 0;
	}
	
	return TRUE;
}


// Function:	TickHandCtrlReadErr
// Purpose:		Keep track of how long there has been no valid Hand Control reads and
//              flags a Hand Sensor Fault warning once it passes HAND_CTRL_READ_WARNING time.
//              when  Status and sets appropriate events if over temp.
// Parameters:	None
// Return:		None
//
void CPort::TickHandCtrlReadErr(void)
{
    if (!m_tPortStatus.bHandCtrlTimeout && m_tPortStatus.qHandCtrlErrTime < HAND_CTRL_READ_TIMEOUT)
    {
        SnQByte qNewTime = m_tPortStatus.qHandCtrlErrTime + m_pControl->GetDelayPeriodMs();
        if (m_tPortStatus.qHandCtrlErrTime < HAND_CTRL_READ_WARNING &&
            qNewTime >= HAND_CTRL_READ_WARNING)
        {
		    m_tWarningStatus.bHallPatternFault = TRUE;
		    SetEvent(m_tPortEvents.m_hHallPatternFaultEvent);
        }
        m_tPortStatus.qHandCtrlErrTime = qNewTime;
    }
}

// Function:	ClearHandCtrlReadErr
// Purpose:		Clears the Hand Sensor Fault warning if the HAND_CTRL_READ_TIMEOUT has not
//              been reached.
// Parameters:	None
// Return:		None
//
void CPort::ClearHandCtrlReadErr(void)
{
    if (!m_tPortStatus.bHandCtrlTimeout) 
    {
        if (m_tPortStatus.qHandCtrlErrTime >= HAND_CTRL_READ_WARNING)
        {
		    m_tWarningStatus.bHallPatternFault = FALSE;
		    SetEvent(m_tPortEvents.m_hHallPatternFaultEvent);
        }
        m_tPortStatus.qHandCtrlErrTime = 0;
    }
}

// Function:	GetMotorStatus
// Purpose:		Monitors motor Status and sets appropriate events if over temp.
// Parameters:	None
// Return:		TRUE indicates Success, FALSE indicates Failure
// Note:		All counters presume a 100ms interval between calls of this function
//
SnBool CPort::GetMotorStatus(void)
{
	SnWord usFault;
	SnWord usInput;
	SnBool bStatus;
	SnWord usOverload;
	SnBool bTacFault = FALSE;
	
	HANDLE	hTacTimerThreadMotor;
	DWORD dwTimerID;
	
	if( m_ucPort == PORTA)
	{
		hTacTimerThreadMotor = m_pControl->m_hTacTimerThreadMotorA;
		dwTimerID = TAC_TIMER_MOTORA;
	}
	else if( m_ucPort == PORTB)
	{
		hTacTimerThreadMotor = m_pControl->m_hTacTimerThreadMotorB;
		dwTimerID = TAC_TIMER_MOTORB;
	}
	else
		return FALSE;

	// check the fault register to see if any motor faults are detected
	bStatus = m_pControl->ReadDsp(m_tOffsetAddr.qFault, 1, &usFault);
	if( !bStatus)
		return FALSE;

	// Clear the latching alarm flags Tac Fault Current Limit and Direction
	if (usFault & (ERROR_MOTOR_TAC|ERROR_MOTOR_TAC_NOISE|ERROR_MOTOR_ILIM|ERROR_MOTOR_DIRECTION))
	{
		usInput = usFault & ~(ERROR_MOTOR_TAC|ERROR_MOTOR_TAC_NOISE|ERROR_MOTOR_ILIM|ERROR_MOTOR_DIRECTION); 
		m_pControl->WriteDsp(m_tOffsetAddr.qFault, 1, &usInput);
	}
	
	if( m_tPortStatus.usHandMode != MOTOR_OFF || m_bMode7 || m_bWindowLock)
	{		
		// Check the Overload register to see if a stall is occurring
		bStatus = m_pControl->ReadDsp(m_tOffsetAddr.qOverload, 1, &usOverload);
		if( !bStatus)
			return FALSE;
		
		if(m_tPortStatus.usDisplayMode == MOTOR_OSCILLATE_1 || m_tPortStatus.usDisplayMode == MOTOR_OSCILLATE_2)
		{
			// Need to trigger the stall this way if the motor is in Oscillation mode
			if( usOverload > 100)
			{
				usFault = usFault | ERROR_MOTOR_STALL;
			}
		}
		
		if( usFault != 0 )
		{
			// Stalled Motor Fault
			if( usFault & ERROR_MOTOR_STALL)
			{
				(m_usMotorStalledCount)++; // motor stall detected, increment the count
				// If the motor is energized (i.e. pulling more than 300mA) then use
				// the short period, otherwise may be testing stand alone handpiece
				if (m_tPortStatus.sMotorCurrent <= -300 || m_tPortStatus.sMotorCurrent >= 300)
					m_usMotorStallThreshold = DEVICE_STALL_SHUTOFF_SHORT;
			}
			else if( m_usMotorStalledCount < m_usMotorStallThreshold)
			{
				m_usMotorStalledCount = 0; // clear error
				m_usMotorStallThreshold = DEVICE_STALL_SHUTOFF_LONG;
			}
			
			if( usFault & ERROR_MOTOR_ILIM)
				(m_usMotorCurrentLimitCount)++; // Current Limit detected, increment the count
			else if( m_usMotorCurrentLimitCount && m_usMotorCurrentLimitCount < 600)
				m_usMotorCurrentLimitCount = 0;
			
			if( usFault & ERROR_MOTOR_TLIM)
				(m_usMotorTorqueLimitCount)++; // Torque Limit detected, increment the count
			else
				m_usMotorTorqueLimitCount = 0;
			
			// Tac Fault
			if( usFault & ERROR_MOTOR_TAC)
			{
				// Start the timer
				if( !hTacTimerThreadMotor)
					m_pControl->StartTimer(dwTimerID); 
				
				(m_usMotorTacFaultCount)++; // tac fault detected increment the count
			}		

            // Motor is spinning in the Wrong Direction
			if( usFault & ERROR_MOTOR_DIRECTION)
			{
				// Clear the Fault
				m_tPortStatus.usPrevHandMode = m_tPortStatus.usHandMode = MOTOR_OFF;
				m_tPortStatus.usPrevFootMode = m_tPortStatus.usFootMode = MOTOR_OFF;
				m_tPortStatus.bMotorReady = FALSE;
				WriteMode(); // shut the motor down
			    m_tWarningStatus.bMotorTacFault = TRUE;
                SetEvent(m_tPortEvents.m_hMotorTacFaultEvent);
			}
		}
		else
		{
			if(m_usMotorStalledCount && m_usMotorStalledCount < m_usMotorStallThreshold) // do not clear if motor has been shut down
				m_usMotorStalledCount = 0;
			if(m_usMotorCurrentLimitCount && m_usMotorCurrentLimitCount < 600) // do not clear if motor has been shut down
				m_usMotorCurrentLimitCount = 0;
			m_usMotorTorqueLimitCount = 0;
		}
		
		if( m_usMotorStalledCount >= (m_usMotorStallThreshold/2))
		{
			if( usFault & ERROR_MOTOR_IHIGH)
			{
				// High Current also reached
				// Motor Stall + Current Limit
				m_tWarningStatus.bMotorStallAndCurrentLimit = TRUE;
				SetEvent(m_tPortEvents.m_hMotorStallAndCurrentLimitEvent);
			}
			else
			{
				// Motor Stall
				m_tWarningStatus.bMotorStall = TRUE;
				SetEvent(m_tPortEvents.m_hMotorStallEvent);
			}
			
			if( m_usMotorStalledCount >= m_usMotorStallThreshold)
			{
				if(m_tWarningStatus.bMotorStallAndCurrentLimit)
                {
                    m_tPortStatus.bWaitForIdleCtl = TRUE;
                    m_tPortStatus.wSerialLatch = 0;
                    SnWord wHallbusLatch = 0;
                    m_pControl->WriteDsp(m_tHallBusOffset.qDeviceLatch, 1, &wHallbusLatch);
                }
				m_tPortStatus.usPrevHandMode = m_tPortStatus.usHandMode = MOTOR_OFF;
				m_tPortStatus.usPrevFootMode = m_tPortStatus.usFootMode = MOTOR_OFF;
				m_tPortStatus.bMotorReady = FALSE;
				WriteMode(); // shut the motor down
			}
		}
		
		if( m_usMotorCurrentLimitCount >= 100 && m_usMotorCurrentLimitCount < 600)
		{
			
			// Motor Current Limit
			m_tWarningStatus.bMotorCurrentLimit = TRUE;
			SetEvent(m_tPortEvents.m_hMotorCurrentLimitEvent);
		}
		
		if( m_usMotorCurrentLimitCount >= 600)
		{
			// Motor Current Limit Timeout
			if(m_tWarningStatus.bMotorCurrentLimit)
			{
				// Clear the Current Limit flag
				m_tWarningStatus.bMotorCurrentLimit = FALSE;
				SetEvent(m_tPortEvents.m_hMotorCurrentLimitEvent);
			}
			
			// Set the Current Limit Timeout Event
			m_tWarningStatus.bMotorCurrentLimitTimeout = TRUE;
			SetEvent(m_tPortEvents.m_hMotorCurrentLimitTimeoutEvent);
			
			m_tPortStatus.usHandMode = MOTOR_OFF;
			m_tPortStatus.bMotorReady = FALSE;
			WriteMode(); // shut the motor down
		}
		if( m_usMotorTacFaultCount > 6)
		{
			// Tac Fault 
			m_tPortStatus.usHandMode = MOTOR_OFF;
			m_tPortStatus.bMotorReady = FALSE;
			WriteMode(); // shut the motor down
			m_tWarningStatus.bMotorTacFault = TRUE;
			SetEvent(m_tPortEvents.m_hMotorTacFaultEvent);
		}
		
		if( m_bMotorTacTimerExpired == TRUE)
		{
			// clear the tack fault count
			if (m_usMotorTacFaultCount != 0)
			{
				DEBUGMSG(TRUE, (TEXT("TAC Fault Count: %6d\n"),m_usMotorTacFaultCount));
				m_usMotorTacFaultCount = 0;
			}
			m_bMotorTacTimerExpired = FALSE;
		}

		// Motor running wait 3 seconds before attempting to readjust
		// motor current zero offset.
		m_tPortStatus.sMotorCurrentAdcCount = 30;

	}
	else
	{
		// Clear the following fault counts. If the motor has been shut down because of a
		// fault condition the count does not get cleared

		if(	m_tPortStatus.usType == TYPE_INVALID)
		{
			m_usMotorStalledCount = 0;
			m_usMotorCurrentLimitCount = 0;
			m_usMotorTorqueLimitCount = 0;
		}
		else
		{
			// don't clear the warnings if the motor has been shut down.
			if( m_usMotorStalledCount > 0 && m_usMotorStalledCount < m_usMotorStallThreshold)
				m_usMotorStalledCount = 0;
			
			if( m_usMotorCurrentLimitCount > 0 && m_usMotorCurrentLimitCount < 600 )
				m_usMotorCurrentLimitCount = 0;

			if ((m_usMotorStalledCount == 0) && (m_usMotorCurrentLimitCount == 0))
			{
				// Motor Current Zero Adjust
				//   Check that sufficient time has passed since the motor was running or
				//   a previous zero adjustment has occured
				// Set the delay timer before attempting to readjust the offset again.
				//
				// The current sensor circuit cannot read currents below 210mA so zero current
				// is scaled to report 210mA (33 ADC Counts).
				// If the zero current is outside of the range of +/-6 mA 
				// (2 ADC counts) then readjust the zero offset by the diffence in adc counts
				// Set the delay timer before attempting to readjust the offset again.
				//
				if (m_tPortStatus.sMotorCurrentAdcCount > 0)
					m_tPortStatus.sMotorCurrentAdcCount -= 1;
				else if ((m_tPortStatus.sMotorCurrentAdc <= (PORT_MIN_CURRENT_ADC - 1)) || 
						 ((PORT_MIN_CURRENT_ADC + 1) <= m_tPortStatus.sMotorCurrentAdc ))
				{
					CPort & otherPort = (m_ucPort == PORTA)? m_pControl->m_PortB:m_pControl->m_PortA;

					if (!(otherPort.m_tPortStatus.bRunning || otherPort.m_bMode7 || otherPort.m_bWindowLock))
					{
						SnSWord psCurrentOffset = 0;
						SnSWord psOffset = (m_ucPort == PORTA)? MC_ANALOG_OFFSET0: MC_ANALOG_OFFSET8;

						if (m_pControl->ReadDsp(psOffset,1, (SnWord *)(&psCurrentOffset)))
						{
							psCurrentOffset += (m_tPortStatus.sMotorCurrentAdc - PORT_MIN_CURRENT_ADC);
							if ((psCurrentOffset > 0) && (psCurrentOffset < 90))
							{
			 					m_pControl->WriteDsp(psOffset, 1, (SnWord *)(&psCurrentOffset));
							}

						}
					}
					// Wait 2 seconds before attempting to readjust the zero offset again.
					// Note: this value must be greater than 0.004 * sShaver.tAnalog.pwCount[0] and 
					// greater than 0.004 * sShaver.tAnalog.pwCount[8]
					m_tPortStatus.sMotorCurrentAdcCount = 20;
				}
			}
		}

	}
	
	
	// clear errors if warranted
	if(m_usMotorTacFaultCount == 0 && m_tWarningStatus.bMotorTacFault)
	{
		// Don't clear the tac fault. Doesn't get cleared until handpiece is pulled out	
	}
	
	if(m_usMotorStalledCount == 0 && m_tWarningStatus.bMotorStall)
	{
		// Clear		
		m_tWarningStatus.bMotorStall = FALSE;
		SetEvent(m_tPortEvents.m_hMotorStallEvent);
	}
	
	if(m_usMotorStalledCount == 0 && m_tWarningStatus.bMotorStallAndCurrentLimit)
	{
		m_tWarningStatus.bMotorStallAndCurrentLimit = FALSE;
		SetEvent(m_tPortEvents.m_hMotorStallAndCurrentLimitEvent);
	}
	
	if(m_usMotorCurrentLimitCount == 0 && m_tWarningStatus.bMotorCurrentLimit)
	{
		// Clear		
		m_tWarningStatus.bMotorCurrentLimit = FALSE;
		SetEvent(m_tPortEvents.m_hMotorCurrentLimitEvent);
	}
	
	if(m_usMotorTorqueLimitCount == 0 && m_tWarningStatus.bMotorTorqueLimit)
	{
		// Clear		
		m_tWarningStatus.bMotorTorqueLimit = FALSE;
		SetEvent(m_tPortEvents.m_hMotorTorqueLimitEvent);
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
void CPort::UpdatePortStatus(void)
{
    if (m_tPortStatus.qNonStableTypeInMs || m_tPortStatus.usType != m_tPortStatus.usPrevType)
    {
        if (m_tPortStatus.qNonStableTypeInMs < TYPE_INSTABILITY_MS)
            m_tPortStatus.qNonStableTypeInMs += m_pControl->GetDelayPeriodMs();
        else
        {
		    // Unstable Type, generate the unknown device ID warning
		    if (!m_tWarningStatus.bUnstableDeviceId)
		    {
			    m_tWarningStatus.bUnstableDeviceId = TRUE;
                SetEvent(m_tPortEvents.m_hUnknownDeviceIdEvent);
                m_tPortStatus.qStableTypeInMs = 0;
		    }
        }
    }

    if (m_tPortStatus.usType != m_tPortStatus.usPrevType) 
	{
        // Port Device status has changed

        // Set the defaults 
		GetPortParameters();
		
		m_tPortStatus.usPrevType = m_tPortStatus.usType;
		
		PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
			(WPARAM)MSG_UPDATE_PORT_STATUS, (LPARAM)m_ucPort);
		
		PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
			(WPARAM)MSG_UPDATE_FOOT_STATUS, (LPARAM)0);
        m_tPortStatus.qStableTypeInMs = 0;
		m_cVelocity.Init(0);
		m_cVelocitySlow.Init(0);
		m_cVelocityFast.Init(0);
	}
    else
    {
        if (m_tPortStatus.qStableTypeInMs < TYPE_STABILITY_MS)
            m_tPortStatus.qStableTypeInMs += m_pControl->GetDelayPeriodMs();
        else
        {
			// Clear unknown device id warning if set
			if (m_tWarningStatus.bUnstableDeviceId)
			{
				m_tWarningStatus.bUnstableDeviceId = FALSE;
			    SetEvent(m_tPortEvents.m_hUnknownDeviceIdEvent);
			}
            m_tPortStatus.qNonStableTypeInMs = 0;
        }
    }
	
	if( m_tPortStatus.usBlade != m_tPortStatus.usPrevBlade)
	{
		// Port Blade status has changed
		
		// Set the defaults needs to be done before calling SetShaverBladeId
		GetPortParameters();
		
		m_tPortStatus.usPrevBlade = m_tPortStatus.usBlade;
		
		PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
			(WPARAM)MSG_UPDATE_BLADE_STATUS, (LPARAM)m_ucPort);
		
		if( m_pControl->m_RemotePumpConnectionType != CPump::PUMP_TYPE_UNKNOWN)
		{
			m_pControl->SetShaverBladeId(m_ucPort, m_tPortStatus.usShaverBladeId);			
		}
	}
	
	if (m_tPortStatus.wPeriod != m_tPortStatus.wPrevPeriod && m_tPortStatus.bMotorReady)
	{
		// Write Port period
		if (WritePeriod())
		{
			m_tPortStatus.wPrevPeriod = m_tPortStatus.wPeriod;
			
			if( !m_tPortStatus.yForceOscMode1 && m_tPortStatus.usOscMode == OSC_MODE2 &&
				m_pControl->m_RemotePumpConnectionType != CPump::PUMP_TYPE_UNKNOWN)
			{
				SnWord usPortMode = (m_tPortStatus.usFootMode == 0) ? m_tPortStatus.usHandMode : m_tPortStatus.usFootMode;
				m_pControl->SetShaverSpeed(m_ucPort, usPortMode, m_tPortStatus.wPeriod, m_tPortStatus.usVelocity);
			}
		}
	}
	
	if (m_tPortStatus.wDwell != m_tPortStatus.wPrevDwell)
	{
		// Write Port Dwell period
		if (WriteDwell())
		{
			m_tPortStatus.wPrevDwell = m_tPortStatus.wDwell;
		}
		
	}
	
	if (m_tPortStatus.usRevolutions != m_tPortStatus.usPrevRevolutions)
	{
		// Write Port # revolutions
		m_pControl->SetOscProfile(m_tPortStatus.usRevolutions, m_ucPort);
		m_tPortStatus.usPrevRevolutions = m_tPortStatus.usRevolutions;
		
	}
	
	if (m_tPortStatus.usVelocity != m_tPortStatus.usPrevVelocity && m_tPortStatus.bMotorReady)
	{
		// Write Velocity Port 
		if (WriteVelocity())
		{
			m_tPortStatus.usPrevVelocity = m_tPortStatus.usVelocity;
			if(m_pControl->m_RemotePumpConnectionType != CPump::PUMP_TYPE_UNKNOWN)
				m_pControl->SetShaverSpeed(m_ucPort, m_tPortStatus.usHandMode, 
				m_tPortStatus.wPeriod, m_tPortStatus.usVelocity);
		}

		m_cVelocityFast.Init(m_tPortStatus.usVelocity);
	}
	
    if (m_tPortStatus.bWaitForIdleCtl && m_tPortStatus.usHandMode == MOTOR_OFF &&
        m_tPortStatus.usFootMode == MOTOR_OFF && !m_bWindowLock)
    {
        m_tPortStatus.bWaitForIdleCtl = FALSE;
    }
	
    if (!m_tPortStatus.bWaitForIdleCtl && m_tPortStatus.usHandMode != m_tPortStatus.usPrevHandMode && 
		m_tPortStatus.bMotorReady)
	{
		// set the Motor mode
		if (WriteMode())
			m_tPortStatus.usPrevHandMode = m_tPortStatus.usHandMode;
	}
	
	if (!m_tPortStatus.bWaitForIdleCtl && m_tPortStatus.usFootMode != m_tPortStatus.usPrevFootMode &&
		m_tPortStatus.bMotorReady)
	{
		// set the Motor mode
		if (WriteMode())
			m_tPortStatus.usPrevFootMode = m_tPortStatus.usFootMode;
    }
	
	if (!m_tPortStatus.bWaitForIdleCtl && m_tPortStatus.usDisplayMode != m_tPortStatus.usPrevDisplayMode)
	{
		// Port  mode status has changed
		m_tPortStatus.usPrevDisplayMode = m_tPortStatus.usDisplayMode;
		
		PostMessage(HWND_BROADCAST, WM_UPDATE_STATUS,
			(WPARAM)MSG_UPDATE_MODE_STATUS, (LPARAM)m_ucPort);
    }
	
}

// Function:	ConfigureMotor
// Purpose:		Configures selected motor for particular device
//
// Parameters:	tMotorConfig	Motor configuration parameters
//
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool CPort::ConfigureMotor(External* ptBldc)
{
	SnBool bStatus;
	SnQByte qBldcOffset;

	if (m_ucPort == PORTA)
	{
		qBldcOffset = (offsetof(Status_Control, tBldcA.tEx))/2;
	}
	else
	{
		qBldcOffset = (offsetof(Status_Control, tBldcB.tEx))/2;
	}
	
	m_tPortStatus.bMotorReady = FALSE;
	
	m_tPortStatus.usPrevHandMode = m_tPortStatus.usHandMode = MOTOR_OFF;
	m_tPortStatus.usPrevFootMode = m_tPortStatus.usFootMode = MOTOR_OFF;
	WriteMode(); // shut the motor down
	
	m_tPortStatus.usPrevVelocity = m_tPortStatus.usVelocity = 0; // Initialize velocity
	WriteVelocity(); // stop the motor
	
	m_tPortStatus.wPrevPeriod = m_tPortStatus.wPeriod = 0; // Initialize period
	WritePeriod();
	
	m_tPortStatus.usDisplayMode = MOTOR_OFF;
	m_tPortStatus.bRunning = FALSE;
	
	// Configure Motor
	if (ptBldc) {
		bStatus = m_pControl->WriteDsp(qBldcOffset,(sizeof(External)/2)-2, (SnWord*)ptBldc);
		if(!bStatus)
			return FALSE;
	}

    // Configure for no motors
    SnWord wTmp = 0x0000;
	SnWord wNewMotorStatus = (m_ucPort == PORTA) ? 0x0080: 0x8000;
 	bStatus = m_pControl->ReadDsp(MC_PORT_TYPE, 1, &wTmp);

	if ((!bStatus) || (wTmp & wNewMotorStatus))
		// Motor board has not completed previous request
		return FALSE;

	wTmp &= (m_ucPort == PORTA) ? 0xFF00: 0x00FF;
	wTmp |= (m_ucPort == PORTA) ? 0x0080: 0x8000;

   // Configure port for a brushless motor
	if (ptBldc != &g_tBldcNoMotor)
	{
		wTmp |= (m_ucPort == PORTA) ? 0x0002: 0x0200;
	}

	bStatus = m_pControl->WriteDsp(MC_PORT_TYPE, 1, &wTmp);
	if (!bStatus)
		return FALSE;
	
	int count = 0;
	do
	{
		Sleep(1);
		bStatus = m_pControl->ReadDsp(MC_PORT_TYPE, 1, &wTmp);
		count++;
		if (count > 5)
		{
			bStatus = FALSE;
		}
	}while (bStatus && (wTmp & wNewMotorStatus) );
	
	wTmp = MODE_DIAGNOSE;

	if (bStatus)
		bStatus = m_pControl->WriteDsp(m_tOffsetAddr.qMode, 1, &wTmp);

	if(bStatus)
	{
		// Wait up to 1 second before timing out
		m_tPortStatus.qShortCiruitTestCnt = SHORT_CIRCUIT_TIMEOUT_MS;
	}

	return bStatus;
}

SnBool CPort::CheckHandPieceStuckBtn( SnWord wActive, SnWord usType, SnBool bFootOverrideActive)
{
	if( ((m_tPortStatus.usPrevType == TYPE_INVALID) || (m_tPortStatus.usPrevType != usType)) && (wActive != 0) )
	{
		// Button down when handpiece plugged in
		m_tPortStatus.wStuckButton = wActive;
		
		if (!bFootOverrideActive)
		{
			m_tWarningStatus.bHandpieceStuckButton = TRUE;
			SetEvent(m_tPortEvents.m_hHandpieceStuckButtonEvent);
		}
		
		return TRUE;
	}
	else if( m_tPortStatus.wStuckButton)
	{
        // Clear any Buttons that became unstuck
        m_tPortStatus.wStuckButton &= wActive;
		
        if( m_tPortStatus.wStuckButton == 0)
		{
			// Clear the stuck button warning if it's set
			if(m_tWarningStatus.bHandpieceStuckButton )
			{
				m_tWarningStatus.bHandpieceStuckButton = FALSE;
				SetEvent(m_tPortEvents.m_hHandpieceStuckButtonEvent);
			}
        }
		else if(!bFootOverrideActive && !m_tWarningStatus.bHandpieceStuckButton)
		{
			m_tWarningStatus.bHandpieceStuckButton = TRUE;
			SetEvent(m_tPortEvents.m_hHandpieceStuckButtonEvent);
		}
	}
	
	return FALSE;
}

// Function:	SetSystemDefaults
// Purpose:		Sets the default values for system parameters
// Parameters:	None
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool CPort::SetSystemDefaults(void)
{

	// Set min/max/increment for system parameters
	m_tPortStatus.usRevolutionsMax = 2;
	m_tPortStatus.usRevolutionsMin = 1;
	m_tPortStatus.usRevolutionsDefault = 2;
	m_tPortStatus.usRevolutionsIncrement = 1;
	m_tPortStatus.wDwellMax = 100;
	m_tPortStatus.wDwellMin = 30;
	m_tPortStatus.wDwellDefault = 30;
	m_tPortStatus.wDwellIncrement = 10;


 	// Port Defaults
	m_tPortStatus.usOscMode = OSC_MODE1;	
	m_tPortStatus.usRevolutions = m_tPortStatus.usRevolutionsDefault;
	m_tPortStatus.wDwell = m_tPortStatus.wDwellDefault;
    AdjustOscRangeForRevolutions(m_tPortStatus.usRevolutions);

	return TRUE;
}


// Function:	SetPortDefaultSpeeds
// Purpose:		Sets the default values for set speeds
// Parameters:	None
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool CPort::SetPortDefaultSpeeds(void)
{
	
	// Powered Hand Tools
	m_tPortDefaultParam.usDpDrillSpeed = 50;
	m_tPortDefaultParam.usDpSawSpeed = 100;

	// High Speed Mdu's (includes Reliant 485 interface)
	m_tPortDefaultParam.usHighSpeedCurvedForwardRpm = 2000;
	m_tPortDefaultParam.usHighSpeedCurvedReverseRpm = 2000;
	m_tPortDefaultParam.usHighSpeedCurvedForward2Rpm = 2000;
	m_tPortDefaultParam.usHighSpeedCurvedReverse2Rpm = 2000;
	m_tPortDefaultParam.usHighSpeedCurvedOscillateRpm = 2000;
	m_tPortDefaultParam.usHighSpeedCurvedOscillateSec = 10;
	
	m_tPortDefaultParam.usHighSpeedStraightForwardRpm = 2000;
	m_tPortDefaultParam.usHighSpeedStraightReverseRpm = 2000;
	m_tPortDefaultParam.usHighSpeedStraightForward2Rpm = 2000;
	m_tPortDefaultParam.usHighSpeedStraightReverse2Rpm = 2000;
	m_tPortDefaultParam.usHighSpeedStraightOscillateRpm = 2000;
	m_tPortDefaultParam.usHighSpeedStraightOscillateSec = 10;

	m_tPortDefaultParam.usHighSpeedBurForwardRpm = 4000;
	m_tPortDefaultParam.usHighSpeedBurReverseRpm = 4000;
	m_tPortDefaultParam.usHighSpeedBurForward2Rpm = 4000;
	m_tPortDefaultParam.usHighSpeedBurReverse2Rpm = 4000;
	m_tPortDefaultParam.usHighSpeedBurOscillateRpm = 2000;
	m_tPortDefaultParam.usHighSpeedBurOscillateSec = 10;

	m_tPortDefaultParam.usHighSpeedFastBurForwardRpm = 5000;
	m_tPortDefaultParam.usHighSpeedFastBurReverseRpm = 5000;
	m_tPortDefaultParam.usHighSpeedFastBurForward2Rpm = 5000;
	m_tPortDefaultParam.usHighSpeedFastBurReverse2Rpm = 5000;
	m_tPortDefaultParam.usHighSpeedFastBurOscillateRpm = 2000;
	m_tPortDefaultParam.usHighSpeedFastBurOscillateSec = 10;

	// Basic/IUR/Ultralite  Mdu's
	m_tPortDefaultParam.usBasicForwardRpm = 3000;
	m_tPortDefaultParam.usBasicReverseRpm = 3000;
	m_tPortDefaultParam.usBasicForward2Rpm = 3000;
	m_tPortDefaultParam.usBasicReverse2Rpm = 3000;
	m_tPortDefaultParam.usBasicOscillateRpm = 1000;
	m_tPortDefaultParam.usBasicOscillateSec = 0;	

	// Mini Mdu's
	m_tPortDefaultParam.usMiniForwardRpm = 2000;
	m_tPortDefaultParam.usMiniReverseRpm = 2000;
	m_tPortDefaultParam.usMiniForward2Rpm = 2000;
	m_tPortDefaultParam.usMiniReverse2Rpm = 2000;
	m_tPortDefaultParam.usMiniOscillateRpm = 1000;
	m_tPortDefaultParam.usMiniOscillateSec = 0;		
		
	// Small Joint Mdu (485 interface)
	m_tPortDefaultParam.usSmallJointCurvedForwardRpm = 2000;
	m_tPortDefaultParam.usSmallJointCurvedReverseRpm = 2000;
	m_tPortDefaultParam.usSmallJointCurvedForward2Rpm = 2000;
	m_tPortDefaultParam.usSmallJointCurvedReverse2Rpm = 2000;
	m_tPortDefaultParam.usSmallJointCurvedOscillateRpm = 3000;
	m_tPortDefaultParam.usSmallJointCurvedOscillateSec = 15;
	
	m_tPortDefaultParam.usSmallJointStraightForwardRpm = 3500;
	m_tPortDefaultParam.usSmallJointStraightReverseRpm = 3500;
	m_tPortDefaultParam.usSmallJointStraightForward2Rpm = 3500;
	m_tPortDefaultParam.usSmallJointStraightReverse2Rpm = 3500;
	m_tPortDefaultParam.usSmallJointStraightOscillateRpm = 3000;
	m_tPortDefaultParam.usSmallJointStraightOscillateSec = 10;

	m_tPortDefaultParam.usSmallJointBurForwardRpm = 6000;
	m_tPortDefaultParam.usSmallJointBurReverseRpm = 6000;
	m_tPortDefaultParam.usSmallJointBurForward2Rpm = 6000;
	m_tPortDefaultParam.usSmallJointBurReverse2Rpm = 6000;
	m_tPortDefaultParam.usSmallJointBurOscillateRpm = 3000;
	m_tPortDefaultParam.usSmallJointBurOscillateSec = 10;

	return TRUE;
}

void CPort::AdjustOscRangeForRevolutions( SnWord wRevolutions)
{
    SnWord wBladeDefault = 0;
	
    switch (m_tPortStatus.usType) 
    {
	case TYPE_MDU_STANDARD:
	case TYPE_MDU_STANDARD_CTL:
	case TYPE_MDU_RELIANT:
	case TYPE_MDU_RELIANT_CTL:
		if (m_tPortStatus.usBlade == BLADE_TYPE_CURVED)
			wBladeDefault = m_tPortDefaultParam.usHighSpeedCurvedOscillateSec;
		else if (m_tPortStatus.usBlade == BLADE_TYPE_STRAIGHT)
			wBladeDefault = m_tPortDefaultParam.usHighSpeedStraightOscillateSec;
		break;
    }
	
    if (wBladeDefault)
    {
        if (wRevolutions == 1)
        {
            m_tPortStatus.wOscillateSecondsDefault = wBladeDefault;
            m_tPortStatus.wOscillateSecondsMin = wBladeDefault;
        }
        else
        {
            m_tPortStatus.wOscillateSecondsMin = 15;
            if (m_tPortStatus.wOscillateSecondsDefault < m_tPortStatus.wOscillateSecondsMin)
                m_tPortStatus.wOscillateSecondsDefault = m_tPortStatus.wOscillateSecondsMin;
            if (m_tPortStatus.wOscillateSeconds < m_tPortStatus.wOscillateSecondsMin)
                m_tPortStatus.wOscillateSeconds = m_tPortStatus.wOscillateSecondsMin;
        }
    }
}

SnBool CPort::ScaleVelocity( SnWord usMode, SnWord usPercentage)
{
	float fPercent = (float)usPercentage/100;
	
	switch(usMode)
	{
	case MOTOR_FORWARD:
		if( IS_TYPE_MDU(m_tPortStatus.usType))
			m_tPortStatus.usVelocity = (SnWord)(m_tPortStatus.usForward * fPercent);
		else if ( IS_TYPE_POWER_INSTR(m_tPortStatus.usType))
			m_tPortStatus.usVelocity = (SnWord)(((m_tPortStatus.usForwardMax * m_tPortStatus.usPercent) * fPercent) /100);
		break;

	case MOTOR_REVERSE:
		if( IS_TYPE_MDU(m_tPortStatus.usType))
			m_tPortStatus.usVelocity = (SnWord)(m_tPortStatus.usReverse * fPercent);
		else if ( IS_TYPE_POWER_INSTR(m_tPortStatus.usType))
			m_tPortStatus.usVelocity = (SnWord)(((m_tPortStatus.usForwardMax * m_tPortStatus.usPercent) * fPercent) /100);
		break;

	case MOTOR_OSCILLATE:
        if( !m_tPortStatus.yForceOscMode1 && m_tPortStatus.usOscMode == OSC_MODE2)
        {
            SnWord wPeriod = m_tPortStatus.wOscillateSeconds + (SnWord)((50 - m_tPortStatus.wOscillateSeconds) * (1.0f - fPercent));
			m_tPortStatus.wPeriod = wPeriod;
        }
		else
			m_tPortStatus.usVelocity = (SnWord)(m_tPortStatus.usOscillateRpm * fPercent);
		break;

	default:
		return FALSE;
		break;
	}
	return TRUE;
}

SnBool CPort::FootPedalStatusUpdate( SnBool bOverride, SnWord wPercent, SnWord wMode )
{
	if(m_bControlsEnabled)
	{
		SnBool bMdu = FALSE;
		SnBool bNeedsFootPedal = FALSE;
		SnBool bPowerInstr = FALSE;

		SnWord usOverride = m_pControl->FootswitchOverride();
		SnBool bFootGood = m_pControl->GoodFootswitch();

		switch( m_tPortStatus.usType)
		{
		case TYPE_MDU_UTLRA_IUR:
		case TYPE_MDU_STANDARD:
		case TYPE_MDU_MINI:
		case TYPE_MDU_RELIANT:
		case TYPE_MDU_POWERMINI:
			bMdu = TRUE;
			bNeedsFootPedal = TRUE;
			break;
			
		case TYPE_MDU_STANDARD_CTL:
		case TYPE_MDU_RELIANT_CTL:
		case TYPE_MDU_POWERMINI_CTL:
			bMdu = TRUE;
			bNeedsFootPedal = FALSE;
			break;
			
		case TYPE_DP_DRILL:
		case TYPE_DP_SAW:
			bPowerInstr = TRUE;
			break;
			
		default:
			bMdu = FALSE;	
			bNeedsFootPedal = FALSE;
			bPowerInstr = FALSE;
			break;
		}
			
		if(bMdu)
		{
			if( m_tPortStatus.bRunning && m_tPortStatus.usHandMode != MOTOR_OFF && usOverride)
			{
				// If the MDU is not in the off state and hand control override is on allow the footswitch 
				// take control
				m_tPortStatus.bHandInCtl = FALSE;
			}
			else if( (m_tPortStatus.bRunning && m_tPortStatus.usHandMode != MOTOR_OFF) || m_bWindowLock)
			{
				//Already activated by the Handpiece
				m_tPortStatus.bHandInCtl = TRUE;
				return TRUE;
			}
			else if( m_tPortStatus.bHandInCtl && wMode != MOTOR_OFF)
			{
				// Must release footpedal before you can gain control
				return TRUE;
			}	
			else
				m_tPortStatus.bHandInCtl = FALSE;
		}	
		else if(bPowerInstr)
		{
			if( m_tPortStatus.bRunning && m_tPortStatus.usHandMode != MOTOR_OFF)
			{
				//Already running return
				return TRUE;
			}
		}
	
		if( bMdu)
		{
			// If Hand Control Overide is ON make sure the Stuck Button Error condition on the MDU gets cleared
			if( usOverride) 
			{
				if(m_tWarningStatus.bHandpieceStuckButton)
				{
					m_tWarningStatus.bHandpieceStuckButton = FALSE;
					SetEvent(m_tPortEvents.m_hHandpieceStuckButtonEvent);
				}
			}
			
			if (bOverride)
			{
				if( wMode == MOTOR_REVERSE)
					ScaleVelocity(MOTOR_REVERSE, wPercent);
				else if( wMode == MOTOR_FORWARD)
					ScaleVelocity(MOTOR_FORWARD, wPercent);
				else if( wMode == MOTOR_OSCILLATE)
				{
					if (!m_tPortStatus.yForceOscMode1 && m_tPortStatus.usOscMode == OSC_MODE2)
						wMode = MOTOR_OSCILLATE_2;
					else
						wMode = MOTOR_OSCILLATE_1;
					ScaleVelocity(MOTOR_OSCILLATE, wPercent);
				}
			}
			else
			{
				if( wMode == MOTOR_REVERSE)
					m_tPortStatus.usVelocity = m_tPortStatus.usReverse;
				else if( wMode == MOTOR_FORWARD)
					m_tPortStatus.usVelocity = m_tPortStatus.usForward;
				else if( wMode == MOTOR_OSCILLATE)
				{
					if(!m_tPortStatus.yForceOscMode1 && m_tPortStatus.usOscMode == OSC_MODE2)
					{
						wMode = MOTOR_OSCILLATE_2;
						m_tPortStatus.wPeriod = m_tPortStatus.wOscillateSeconds;
					}
					else 
					{
						wMode = MOTOR_OSCILLATE_1;
						m_tPortStatus.usVelocity = m_tPortStatus.usOscillateRpm;
					}
				}
			}
			
			if (wMode == MOTOR_OFF)
			{
				// MOTOR_OFF
				if( usOverride) 
				{
					// If Hand Control Override is Enabled set the velocity to 0
					// otherwise leave the velocity setting alone.
					m_tPortStatus.usVelocity = 0;
					m_tPortStatus.usHandMode = MOTOR_OFF;
				}
				if (m_tPortStatus.usFootMode != wMode)
				    m_tPortStatus.usFootMode = wMode;
			}
			else
			{		
				if( bFootGood && m_tPortStatus.usHandMode == MOTOR_OFF)
					m_tPortStatus.usFootMode = wMode;
			}
			
		}
	
		if( bPowerInstr && (wMode == MOTOR_FORWARD || wMode == MOTOR_REVERSE ||
			wMode == MOTOR_OFF))
		{
			if ( wMode != MOTOR_OFF) 
			{
				if (bOverride)
				{
					// Variable Footswitch
					if( wMode == MOTOR_FORWARD)
						ScaleVelocity(MOTOR_FORWARD, wPercent);
					else
						ScaleVelocity(MOTOR_REVERSE, wPercent); // Reverse
				}
				else
					ScaleVelocity(wMode, 100);
				
				switch (m_tPortStatus.usType)
				{
				case TYPE_DP_DRILL:
					// Drill displays Forward or Reverse
					m_tPortStatus.usDisplayMode = wMode;
					break;
					
				case TYPE_DP_SAW:
					// Saw only displays Oscillate
					m_tPortStatus.usDisplayMode = MOTOR_OSCILLATE_1;
					break;	
				}
			    if( bFootGood && m_tPortStatus.usHandMode == MOTOR_OFF)
				    m_tPortStatus.usFootMode = wMode;
			}
            else
            {
				if (m_tPortStatus.usFootMode != wMode)
				    m_tPortStatus.usFootMode = wMode;
            }
		}
	}

	if (!m_tPortStatus.bWaitForIdleCtl && m_tPortStatus.usFootMode != m_tPortStatus.usPrevFootMode &&
	m_tPortStatus.bMotorReady)
	{
		// set the Motor mode
		if (WriteMode())
			m_tPortStatus.usPrevFootMode = m_tPortStatus.usFootMode;
	}

	return TRUE;
}


void CPort::SetWindowLock(SnWord wInputBuffer, SnWord usOverride)
{
	if( wInputBuffer == WINDOW_LOCK_ON && m_tPortStatus.bMotorReady)
	{
		if( !m_tPortStatus.bRunning && !m_tPortStatus.bWaitForIdleCtl)
		{
			// Handpiece not activated go ahead and start windowlock
			m_bWindowLock = TRUE;
			m_tPortStatus.usDisplayMode = MOTOR_WINDOW_LOCK;
			WriteMode();
		}
	}
	else if( wInputBuffer == WINDOW_LOCK_OFF)
	{
		if( m_bWindowLock)
		{
			m_bWindowLock = FALSE;
			if( usOverride)
				m_tPortStatus.usFootMode = MOTOR_OFF;
			else
				m_tPortStatus.usHandMode = MOTOR_OFF;
			
			m_tPortStatus.usDisplayMode = MOTOR_OFF;
			WriteMode();
		}
	}
}
