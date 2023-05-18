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
    SnBool bHandCtrlTimeout;
    SnQByte qHandCtrlErrTime;
    char pcSerialNumber[SERIAL_NUMBER_STORE];
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
    void   TickHandCtrlReadErr(void);
    void   ClearHandCtrlReadErr(void);

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