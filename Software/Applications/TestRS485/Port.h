// Port.h: interface for the CLogger class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PORT_H__03B76E3C_A260_45AA_88C9_D4689CCA8F4B__INCLUDED_)
#define AFX_PORT_H__03B76E3C_A260_45AA_88C9_D4689CCA8F4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SnTypes.h"
#include "MotorControlCommands.h"
#include "ControllerTypes.h"
#include "CommonDefines.h"

#define TRIGGER_STABILITY_DELAY_MS		100	    // 0.1 Second delay
#define TYPE_STABILITY_MS               1500    // 1.5 Seconds       
#define TYPE_INSTABILITY_MS             6000    // 6 Seconds       

extern External g_tBldcNoMotor;

typedef struct 
{
	SnWord usBasicForwardRpm;
	SnWord usBasicReverseRpm;
	SnWord usBasicForward2Rpm;
	SnWord usBasicReverse2Rpm;
	SnWord usBasicOscillateRpm;
	SnWord usBasicOscillateSec;

	SnWord usMiniForwardRpm;
	SnWord usMiniReverseRpm;
	SnWord usMiniForward2Rpm;
	SnWord usMiniReverse2Rpm;
	SnWord usMiniOscillateRpm;
	SnWord usMiniOscillateSec;

	SnWord usHighSpeedCurvedForwardRpm;
	SnWord usHighSpeedCurvedReverseRpm;
	SnWord usHighSpeedCurvedForward2Rpm;
	SnWord usHighSpeedCurvedReverse2Rpm;
	SnWord usHighSpeedCurvedOscillateRpm;
	SnWord usHighSpeedCurvedOscillateSec;

	SnWord usHighSpeedStraightForwardRpm;
	SnWord usHighSpeedStraightReverseRpm;
	SnWord usHighSpeedStraightForward2Rpm;
	SnWord usHighSpeedStraightReverse2Rpm;
	SnWord usHighSpeedStraightOscillateRpm;
	SnWord usHighSpeedStraightOscillateSec;

	SnWord usHighSpeedBurForwardRpm;
	SnWord usHighSpeedBurReverseRpm;
	SnWord usHighSpeedBurForward2Rpm;
	SnWord usHighSpeedBurReverse2Rpm;
	SnWord usHighSpeedBurOscillateRpm;
	SnWord usHighSpeedBurOscillateSec;

	SnWord usHighSpeedFastBurForwardRpm;
	SnWord usHighSpeedFastBurReverseRpm;
	SnWord usHighSpeedFastBurForward2Rpm;
	SnWord usHighSpeedFastBurReverse2Rpm;
	SnWord usHighSpeedFastBurOscillateRpm;
	SnWord usHighSpeedFastBurOscillateSec;

	SnWord usLowSpeedCurvedForwardRpm;
	SnWord usLowSpeedCurvedReverseRpm;
	SnWord usLowSpeedCurvedForward2Rpm;
	SnWord usLowSpeedCurvedReverse2Rpm;
	SnWord usLowSpeedCurvedOscillateRpm;
	SnWord usLowSpeedCurvedOscillateSec;

	SnWord usLowSpeedStraightForwardRpm;
	SnWord usLowSpeedStraightReverseRpm;
	SnWord usLowSpeedStraightForward2Rpm;
	SnWord usLowSpeedStraightReverse2Rpm;
	SnWord usLowSpeedStraightOscillateRpm;
	SnWord usLowSpeedStraightOscillateSec;

	SnWord usLowSpeedBurForwardRpm;
	SnWord usLowSpeedBurReverseRpm;
	SnWord usLowSpeedBurForward2Rpm;
	SnWord usLowSpeedBurReverse2Rpm;
	SnWord usLowSpeedBurOscillateRpm;
	SnWord usLowSpeedBurOscillateSec;

	SnWord usLowSpeedFastBurForwardRpm;
	SnWord usLowSpeedFastBurReverseRpm;
	SnWord usLowSpeedFastBurForward2Rpm;
	SnWord usLowSpeedFastBurReverse2Rpm;
	SnWord usLowSpeedFastBurOscillateRpm;
	SnWord usLowSpeedFastBurOscillateSec;

	SnWord usSuperHighSpeedCurvedForwardRpm;
	SnWord usSuperHighSpeedCurvedReverseRpm;
	SnWord usSuperHighSpeedCurvedForward2Rpm;
	SnWord usSuperHighSpeedCurvedReverse2Rpm;
	SnWord usSuperHighSpeedCurvedOscillateRpm;
	SnWord usSuperHighSpeedCurvedOscillateSec;

	SnWord usSuperHighSpeedStraightForwardRpm;
	SnWord usSuperHighSpeedStraightReverseRpm;
	SnWord usSuperHighSpeedStraightForward2Rpm;
	SnWord usSuperHighSpeedStraightReverse2Rpm;
	SnWord usSuperHighSpeedStraightOscillateRpm;
	SnWord usSuperHighSpeedStraightOscillateSec;

	SnWord usSuperHighSpeedBurForwardRpm;
	SnWord usSuperHighSpeedBurReverseRpm;
	SnWord usSuperHighSpeedBurForward2Rpm;
	SnWord usSuperHighSpeedBurReverse2Rpm;
	SnWord usSuperHighSpeedBurOscillateRpm;
	SnWord usSuperHighSpeedBurOscillateSec;

	SnWord usSuperHighSpeedFastBurForwardRpm;
	SnWord usSuperHighSpeedFastBurReverseRpm;
	SnWord usSuperHighSpeedFastBurForward2Rpm;
	SnWord usSuperHighSpeedFastBurReverse2Rpm;
	SnWord usSuperHighSpeedFastBurOscillateRpm;
	SnWord usSuperHighSpeedFastBurOscillateSec;
	
	SnWord usDpDrillSpeed;
	SnWord usDpSawSpeed;

	SnWord usSmallJointCurvedForwardRpm;
	SnWord usSmallJointCurvedReverseRpm;
	SnWord usSmallJointCurvedForward2Rpm;
	SnWord usSmallJointCurvedReverse2Rpm;
	SnWord usSmallJointCurvedOscillateRpm;
	SnWord usSmallJointCurvedOscillateSec;

	SnWord usSmallJointStraightForwardRpm;
	SnWord usSmallJointStraightReverseRpm;
	SnWord usSmallJointStraightForward2Rpm;
	SnWord usSmallJointStraightReverse2Rpm;
	SnWord usSmallJointStraightOscillateRpm;
	SnWord usSmallJointStraightOscillateSec;

	SnWord usSmallJointBurForwardRpm;
	SnWord usSmallJointBurReverseRpm;
	SnWord usSmallJointBurForward2Rpm;
	SnWord usSmallJointBurReverse2Rpm;
	SnWord usSmallJointBurOscillateRpm;
	SnWord usSmallJointBurOscillateSec;

}DEVICE_DATA; 


typedef struct 
{
    SnByte ucVersion;

	SnByte ucBasicForwardRpm;
	SnByte ucBasicReverseRpm;
	SnByte ucBasicForward2Rpm;
	SnByte ucBasicReverse2Rpm;
	SnByte ucBasicOscillateRpm;
	SnByte ucBasicOscillateSec;

	SnByte ucMiniForwardRpm;
	SnByte ucMiniReverseRpm;
	SnByte ucMiniForward2Rpm;
	SnByte ucMiniReverse2Rpm;
	SnByte ucMiniOscillateRpm;
	SnByte ucMiniOscillateSec;

	SnByte ucHighSpeedCurvedForwardRpm;
	SnByte ucHighSpeedCurvedReverseRpm;
	SnByte ucHighSpeedCurvedForward2Rpm;
	SnByte ucHighSpeedCurvedReverse2Rpm;
	SnByte ucHighSpeedCurvedOscillateRpm;
	SnByte ucHighSpeedCurvedOscillateSec;

	SnByte ucHighSpeedStraightForwardRpm;
	SnByte ucHighSpeedStraightReverseRpm;
	SnByte ucHighSpeedStraightForward2Rpm;
	SnByte ucHighSpeedStraightReverse2Rpm;
	SnByte ucHighSpeedStraightOscillateRpm;
	SnByte ucHighSpeedStraightOscillateSec;

	SnByte ucHighSpeedBurForwardRpm;
	SnByte ucHighSpeedBurReverseRpm;
	SnByte ucHighSpeedBurForward2Rpm;
	SnByte ucHighSpeedBurReverse2Rpm;
	SnByte ucHighSpeedBurOscillateRpm;
	SnByte ucHighSpeedBurOscillateSec;

	SnByte ucHighSpeedFastBurForwardRpm;
	SnByte ucHighSpeedFastBurReverseRpm;
	SnByte ucHighSpeedFastBurForward2Rpm;
	SnByte ucHighSpeedFastBurReverse2Rpm;
	SnByte ucHighSpeedFastBurOscillateRpm;
	SnByte ucHighSpeedFastBurOscillateSec;

	SnByte ucLowSpeedCurvedForwardRpm;
	SnByte ucLowSpeedCurvedReverseRpm;
	SnByte ucLowSpeedCurvedForward2Rpm;
	SnByte ucLowSpeedCurvedReverse2Rpm;
	SnByte ucLowSpeedCurvedOscillateRpm;
	SnByte ucLowSpeedCurvedOscillateSec;

	SnByte ucLowSpeedStraightForwardRpm;
	SnByte ucLowSpeedStraightReverseRpm;
	SnByte ucLowSpeedStraightForward2Rpm;
	SnByte ucLowSpeedStraightReverse2Rpm;
	SnByte ucLowSpeedStraightOscillateRpm;
	SnByte ucLowSpeedStraightOscillateSec;

	SnByte ucLowSpeedBurForwardRpm;
	SnByte ucLowSpeedBurReverseRpm;
	SnByte ucLowSpeedBurForward2Rpm;
	SnByte ucLowSpeedBurReverse2Rpm;
	SnByte ucLowSpeedBurOscillateRpm;
	SnByte ucLowSpeedBurOscillateSec;

	SnByte ucLowSpeedFastBurForwardRpm;
	SnByte ucLowSpeedFastBurReverseRpm;
	SnByte ucLowSpeedFastBurForward2Rpm;
	SnByte ucLowSpeedFastBurReverse2Rpm;
	SnByte ucLowSpeedFastBurOscillateRpm;
	SnByte ucLowSpeedFastBurOscillateSec;

	SnByte ucSuperHighSpeedCurvedForwardRpm;
	SnByte ucSuperHighSpeedCurvedReverseRpm;
	SnByte ucSuperHighSpeedCurvedForward2Rpm;
	SnByte ucSuperHighSpeedCurvedReverse2Rpm;
	SnByte ucSuperHighSpeedCurvedOscillateRpm;
	SnByte ucSuperHighSpeedCurvedOscillateSec;

	SnByte ucSuperHighSpeedStraightForwardRpm;
	SnByte ucSuperHighSpeedStraightReverseRpm;
	SnByte ucSuperHighSpeedStraightForward2Rpm;
	SnByte ucSuperHighSpeedStraightReverse2Rpm;
	SnByte ucSuperHighSpeedStraightOscillateRpm;
	SnByte ucSuperHighSpeedStraightOscillateSec;

	SnByte ucSuperHighSpeedBurForwardRpm;
	SnByte ucSuperHighSpeedBurReverseRpm;
	SnByte ucSuperHighSpeedBurForward2Rpm;
	SnByte ucSuperHighSpeedBurReverse2Rpm;
	SnByte ucSuperHighSpeedBurOscillateRpm;
	SnByte ucSuperHighSpeedBurOscillateSec;

	SnByte ucSuperHighSpeedFastBurForwardRpm;
	SnByte ucSuperHighSpeedFastBurReverseRpm;
	SnByte ucSuperHighSpeedFastBurForward2Rpm;
	SnByte ucSuperHighSpeedFastBurReverse2Rpm;
	SnByte ucSuperHighSpeedFastBurOscillateRpm;
	SnByte ucSuperHighSpeedFastBurOscillateSec;

	SnByte ucDpDrillSpeed;
	SnByte ucDpSawSpeed;

	SnByte ucSmallJointCurvedForwardRpm;
	SnByte ucSmallJointCurvedReverseRpm;
	SnByte ucSmallJointCurvedForward2Rpm;
	SnByte ucSmallJointCurvedReverse2Rpm;
	SnByte ucSmallJointCurvedOscillateRpm;
	SnByte ucSmallJointCurvedOscillateSec;

	SnByte ucSmallJointStraightForwardRpm;
	SnByte ucSmallJointStraightReverseRpm;
	SnByte ucSmallJointStraightForward2Rpm;
	SnByte ucSmallJointStraightReverse2Rpm;
	SnByte ucSmallJointStraightOscillateRpm;
	SnByte ucSmallJointStraightOscillateSec;

	SnByte ucSmallJointBurForwardRpm;
	SnByte ucSmallJointBurReverseRpm;
	SnByte ucSmallJointBurForward2Rpm;
	SnByte ucSmallJointBurReverse2Rpm;
	SnByte ucSmallJointBurOscillateRpm;
	SnByte ucSmallJointBurOscillateSec;

}SAVE_DEVICE_DATA;

typedef struct 
{
	SnWord usPrevLogic;
	SnSWord sNewLogicIdCnt;	// Debounce counter before accepting the new type
	SnWord usType;			// Type of instrument:MDU/Hand Powered
	SnWord usPrevType;
	SnWord usDisplayMode;	// State of Display Forward/Reverse/Oscillate
	SnWord usPrevDisplayMode;	// State of Display Forward/Reverse/Oscillate
	SnWord usHandMode;			// State of Motor Forward/Reverse/Oscillate
	SnWord usPrevHandMode;
	SnWord usFootMode;		// State of Motor when Foot pedal is used
	SnWord usPrevFootMode;		// State of Motor when Foot pedal is used
	SnBool bHandModeDualOn;
	SnWord usSafeMode;
	SnWord usVelocity;		// Current Velocity
    SnWord usPrevVelocity;
	SnWord usActualVelocity;
	SnWord wPeriod;		// Current period for Oscillate Mode 2
	SnWord wPrevPeriod;
    SnWord usBlade;			// Blade Type
    SnWord usPrevBlade;
    SnWord usShaverBladeId;	// Blade speed to be used by the pump
	SnWord usForward;		// Current Forward setting
	SnWord usForward2;		// Seconcd set speed
    SnWord usForwardDefault;
	SnWord usForward2Default;
    SnWord usForwardMin;
    SnWord usForwardMax;
	SnWord usForwardIncrement;
	SnWord usForwardDownCount;
    SnWord usReverse;		// Current Reverse setting	
	SnWord usReverse2;		// Second set speed
    SnWord usReverseDefault;
	SnWord usReverse2Default;
    SnWord usReverseMin;
    SnWord usReverseMax;
	SnWord usReverseIncrement;
	SnWord usReverseDownCount;
    SnWord usOscillateRpm;		// Current Oscillate setting
    SnWord usOscillateRpmDefault;
    SnWord usOscillateRpmMin;
    SnWord usOscillateRpmMax;
    SnWord usOscillateRpmIncrement;
	SnWord usOscillateDownCount;
	SnWord usOscMode;		// Current Oscillate Mode:Default/Mode1/Mode2
    SnBool yForceOscMode1;
    SnWord wOscillateSeconds;
	SnWord wOscillateSecondsMax;		
	SnWord wOscillateSecondsMin;
	SnWord wOscillateSecondsDefault;
	SnWord wOscillateSecondsIncrement;
	SnWord usRevolutions;
	SnWord usPrevRevolutions;
	SnWord usRevolutionsMax;
	SnWord usRevolutionsMin;
	SnWord usRevolutionsDefault;
	SnWord usRevolutionsIncrement;
	SnWord wDwell;
	SnWord wPrevDwell;
	SnWord wDwellMin;
	SnWord wDwellMax;
	SnWord wDwellDefault;
	SnWord wDwellIncrement;
	SnWord usPercent;		// Current Hand Powered Tool setting	
    SnWord usPercentDefault;
    SnWord usPercentMin;
    SnWord usPercentMax;
    short  sPercentIncrement;
	SnBool bRunning;
	SnBool bMotorReady;
	SnWord wStuckButton;
    SnWord wRS485Delay;
    SnWord wHallDelay;
	SnWord wSerialLatch;
    SnWord wSerialPrevActive;
	SnSWord sTriggerMin;
    SnSWord sTriggerMax;
    SnSWord sTriggerDelay;
	SnWord	usPrevFootHandMode;
	SnBool	bHandInCtl;
    SnBool  bWaitForIdleCtl;
    SnQByte qShortCiruitTestCnt;
    SnQByte qStableTypeInMs;
    SnQByte qNonStableTypeInMs;
} SN_PORT_STATUS;

typedef struct
{
	SnBool bFootswitchRequired;
	SnBool bHallPatternFault;
	SnBool bHandpieceStuckButton;
	SnBool bMotorCurrentLimit;
	SnBool bMotorCurrentLimitTimeout;
	SnBool bMotorShortCircuit;
	SnBool bMotorShortCircuitTimeout;
	SnBool bMotorStall;
	SnBool bMotorStallAndCurrentLimit;
	SnBool bMotorTacFault;
	SnBool bMotorTorqueLimit;
	SnBool bUnknownBladeId;
	SnBool bUnknownDeviceId;
	SnBool bUnstableDeviceId;
} PORT_WARNING_STATUS;

typedef struct
{
	HANDLE					m_hMotorKillTacTimerEvent;				// Handle to event object
	HANDLE					m_hMotorStallEvent;
	HANDLE					m_hMotorTacFaultEvent;
	HANDLE					m_hMotorShortCircuitEvent;
	HANDLE					m_hMotorShortCircuitTimeoutEvent;
	HANDLE					m_hMotorStallAndCurrentLimitEvent;
	HANDLE					m_hUnknownBladeIdEvent;
	HANDLE					m_hUnknownDeviceIdEvent;
	HANDLE					m_hHallPatternFaultEvent;
	HANDLE					m_hMotorCurrentLimitEvent;
	HANDLE					m_hMotorCurrentLimitTimeoutEvent;
	HANDLE					m_hMotorTorqueLimitEvent;
	HANDLE					m_hHandpieceStuckButtonEvent;
	HANDLE					m_hFootswitchRequiredEvent;
} SN_PORT_EVENTS;

typedef struct
{
	SnQByte		qVelocityCommand;
	SnQByte		qVelocityActualAbs;
	SnQByte		qMode;
	SnQByte		qFault;
	SnQByte		qDwell;
	SnQByte		qPeriod;
	SnQByte		qOverload;
} SN_OFFSET_ADDRESS;

/* Hall bus for legacy shaver and footswitch data */
typedef struct {
  SnQByte  qDeviceExist;       // hall devices present in corresponding bit pattern
  SnQByte  qDeviceActive;      // activated hall devices
  SnQByte  qDeviceLatch;       // push-on / push-off state data
  SnQByte  qHallBusVq;         // bus quiescent voltage during reset
} SN_HALLBUS_OFFSETS;


class CControl;

class CPort
{

public:
	CPort(void);
	virtual ~CPort(void);

	inline SnWord GetType(void) {return m_tPortStatus.usType;}
    inline SnWord GetDeviceMode(void) {return m_tPortStatus.usDisplayMode;}
    inline SnWord GetVelocity(void) {return m_tPortStatus.usVelocity;}
    inline void SetMaxPercent(void) {m_tPortStatus.usPercent = 100;}
	inline static SnBool GetControlsEnabled(void) {return m_bControlsEnabled;};
	inline static void SetControlsEnabled(SnBool bControlsEnabled) {m_bControlsEnabled = bControlsEnabled;};
	inline SnBool GetWindowLock(void) {return m_bWindowLock;};
	inline SnBool GetMotorTacTimerExpired(void){ return m_bMotorTacTimerExpired;}
	inline void SetMotorTacTimerExpired(SnBool bMotorTacTimerExpired){ m_bMotorTacTimerExpired = bMotorTacTimerExpired;}
	inline SnBool GetMode7(void) {return m_bMode7;};
	inline SnBool GetSaveIfDirty(void) {return m_bSaveIfDirty;};
	inline void SetSaveIfDirty(SnBool bSaveIfDirty) {m_bSaveIfDirty = bSaveIfDirty;};

	void   Initialize( CControl * pControl, unsigned char ucPort );
	SnBool GetPortParameters( void);
	SnBool DisableMode7(void);
	SnBool WriteVelocity(void);
	SnBool WritePeriod(void);
	SnBool WriteMode(void);
	SnBool WriteDwell(void);
	SnWord GetActualVelocity(void);
	SnBool SetSystemDefaults(void);
	SnBool SetPortDefaultSpeeds(void);
	SnBool SetPortCustomSpeeds(void);
	void   GetRS485DeviceVersion(SnByte& major, SnByte& minor,SnByte& build);
	void   SetPortStatus(SN_PORT_STATUS* ptInputBuffer);
	SnBool ReadPortStatus(void);
	void   UpdatePortStatus(void);
	SnBool GetMotorStatus(void);
	void   ClearAllMotorWarnings(void);
	SnBool ConfigureMotor(External* ptBldc);
	SnBool CheckHandPieceStuckBtn( SnWord wActive, SnWord usType, SnBool bFootOverrideActive);
	void   AdjustOscRangeForRevolutions(SnWord wRevolutions);
	SnWord CheckBladeType(SnWord usBladeType);
	SnWord PowerMiniCheckBladeType(SnWord mduOut);
	SnBool ScaleVelocity( SnWord usMode, SnWord usPercentage);
	SnBool ReadTrigger( SnWord wDeadBand, SnWord *pwVelocity);
	SnBool FootPedalStatusUpdate(SnBool bOverride, SnWord wPercent, SnWord wMode);
	void   SetWindowLock(SnWord wInputBuffer, SnWord usOverride);


	DEVICE_DATA			m_tPortSavedParam;
	DEVICE_DATA			m_tPortDefaultParam;
	SN_PORT_STATUS		m_tPortStatus;
	PORT_WARNING_STATUS	m_tWarningStatus;

	SN_PORT_EVENTS		m_tPortEvents;

private:
	CControl*			m_pControl;
	unsigned char		m_ucPort;
	SnBool				m_bFootAssigned;		// TRUE = Footswitch Assigned to port

	SN_OFFSET_ADDRESS	m_tOffsetAddr;
	SN_HALLBUS_OFFSETS	m_tHallBusOffset;
	SnQByte				m_qSensorOffset;


	SnBool				m_bSaveIfDirty;			// TRUE = User parameters have changed ***
	static SnBool       m_bControlsEnabled;		// ***
	SnBool				m_bWindowLock;
	SnBool				m_bMode7;
	SnBool				m_bMotorTacTimerExpired;

	SnWord				m_usMotorStalledCount;
	SnWord				m_usMotorCurrentLimitCount;
	SnWord				m_usMotorTorqueLimitCount;
	SnWord				m_usMotorTacFaultCount;
	SnWord				m_us485DeviceVersionNum;
};


// *** Previously one entry for controlls

#endif // !defined(AFX_PORT_H__03B76E3C_A260_45AA_88C9_D4689CCA8F4B__INCLUDED_)