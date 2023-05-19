// FootSwitch.cpp: interface for the Footswitch class.
//
//////////////////////////////////////////////////////////////////////
#ifndef FOOTSWITCH_H
#define FOOTSWITHC_H

#include "SnTypes.h"
#include "CommandDuration.h"


#define START_CALIBRATION	0xA5
#define STOP_CALIBRATION	0x5A

typedef union  
{
    struct {
        unsigned left:1;
        unsigned middle:1;
        unsigned right:1;
    };
    SnWord w;
} ButtonStatus;

typedef struct
{
    SnWord usType;					// Type of footswitch
    SnWord usPrevType;
	SnWord usFootAssignedPort;		// NULL, PORTA, PORTB
	SnWord usMode;					// Variable or On/Off
	SnWord usForward;				// Forward Pedal Assignment Left/Right
	SnWord usOverride;				// Hand Control Override: On/Off
	SnWord usPortControl;			// Foot switch mapping control PortA/PortB/None
	SnWord usPrevPortControl;		// Previous Foot switch mapping control port
    SnSWord psPedalMin[3];
    SnSWord psPedalMax[3];
    SnWord pwPedalPercent[3];
    ButtonStatus wButtonStatus;
	SnBool bFootStuck;
	SnBool bFootGood;
	double dCommandDuration;
	SnQByte qConnectDuration;
	SnQByte qErrorCount;
	SnBool bInSoftwareReset;
	SnBool bInCalibration;
	SnBool bInInvalidCommandTest;
} SN_FOOT_STATUS;

typedef struct
{
	SnBool bLowBattery;
	SnBool bStuckPedal;
	SnBool bUnknownID;
} FOOTSWITCH_WARNING_STATUS;

typedef struct
{
	HANDLE					m_hUnknownIdEvent;
 	HANDLE					m_hLowBatteryEvent;
	HANDLE					m_hStuckPedalEvent;
} FOOTSWITCH_EVENTS;

class CControl;

class FootSwitch
{
public:
	FootSwitch(void);
	virtual ~FootSwitch(void);
	void Initialize(CControl* pControl);
	SnBool SetSystemDefaults();
	SnBool SetFootPedalPrescaleValues( SnWord usType);
    void ResetVariableFootPedals(void);
    SnBool ReadVariableFootPedals(void);
    SnBool DebounceSpeedUpDown(SnBool ySpeedUp, SnBool ySpeedDown, SnWord wPrevMode);
	SnBool ReadFootPedalStatus(void);
	void UpdatePortStatus();
	SnBool StartStopSoftwareReset(SnWord wStartStop);
	SnBool StartStopCalibration(SnWord wStartStop);
	SnBool StartStopInvalidCommandTest(SnWord wStartStop);
	void InvalidCommandTest(void);
	void CalcConnectTime(void);


	void   GetRS485DeviceVersion(SnByte& major, SnByte& minor,SnByte& build);
	inline SnWord GetFootPedalType(void) {return m_tFootPedalStatus.usType;}
    inline SnBool GetFastFootswitch(void) {return m_bFastFootswitch;}
    inline void EnableFootswitch(void) {m_bFootswitchEnabled = TRUE;}
    inline void DisableFootswitch(void) {m_bFootswitchEnabled = FALSE;}
	inline void SetMessageHandler(HWND hWnd){ m_hGuiWnd = hWnd;}
	inline SnWord FootAssignedPort(void){return m_tFootPedalStatus.usFootAssignedPort;}
	inline SnWord Override(void){return m_tFootPedalStatus.usOverride;}
	inline SnBool GoodFootswitch(void){return m_tFootPedalStatus.bFootGood;}

	FOOTSWITCH_WARNING_STATUS	m_tFootswitchWarningStatus;
	FOOTSWITCH_EVENTS			m_tFootswitchEvents;
	SN_FOOT_STATUS				m_tFootPedalStatus;

private:
	CControl*		m_pControl;		// Pointer to the Control Layer Object
	SnBool          m_bFootswitchEnabled;
	SnBool          m_bFastFootswitch;
	SnWord			m_usFootSwitchVersionNum;
    SnSWord			m_sFootStatusDelay;
    SnSWord			m_sFootConnectDebounce;
	SnWord			m_wSpUp;
    SnWord			m_wSpDn;
    SnBool			m_bPollRs485;

	CCommandDuration m_cDuration;
	SnBool			m_bSoftwareResetRequest;
	SnWord			m_StartStopCalibration;
	SnBool			m_bDeviceConnected;
	SnQByte			m_qConnectStartTime;

	HWND			m_hGuiWnd;				// Handle to Update Window (GUI)
};

#endif