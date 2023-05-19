// Control.cpp: implementation of the CControl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Control.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static CDriver *g_hDriver = NULL;
#if PROFILE_TEST
/***************************************************************
Brushless motor move profiles
***************************************************************/
MotorProfile g_tOscillateMode1 = {
	10,0,
		0.00f, 0.0f,
		0.05f, 0.125f,
		0.10f, 0.250f,
		0.40f, 0.250f,
		0.45f, 0.375f,
		0.50f, 0.500f,
		0.55f, 0.375f,
		0.60f, 0.250f,
		0.90f, 0.250f,
		0.95f, 0.125f,
		1.00f, 0.0f
}; // Profile # 1

MotorProfile g_tOscillateMode2 = {
	4,0,
		0.000f, 0.0f,
		0.375f, 1.0f,
		0.750f, 2.0f,
		0.875f, 1.5f,
		1.000f, 1.0f
		
}; // Profile # 2
#else
   /***************************************************************
   Brushless motor move profiles
***************************************************************/
MotorProfile g_tOscillateMode1 = {
	4,0,
		0.00, 0.0,
		0.25, 0.5625,
		0.50, 1.125,
		0.75, 0.5625,
		1.00, 0.0
		
}; // Profile # 1

MotorProfile g_tOscillateMode2 = {
	4,0,
		0.00, 0.0,
		0.25, 1.0625,
		0.50, 2.125,
		0.75, 1.0625,
		1.00, 0.0
		
}; // Profile # 2
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CControl::CControl()
{
}

CControl::~CControl()
{
	DeInit(FALSE);
}

// Function:	SetMessageHandler
// Purpose: 	Sets the Window Handle 
// Parameters:	hWnd = Callers Window Handle (GUI)
// Return:		TRUE = Success, FALSE = Failure
//
SnBool CControl::SetMessageHandler(HWND hWnd)
{
	m_hGuiWnd = hWnd;
	m_Footswitch.SetMessageHandler(hWnd);
	SetEvent(m_hDisplayInitializedEvent);
	return TRUE;
}

// Function:	Init
// Purpose: 	Initializes the Control Layer
// Parameters:	None
// Return:		TRUE = Success, FALSE = Failure
//
SnBool CControl::Init(SnWord* pusTypeFailure, SnWord* pusDetailFailure)
{
	if( pusTypeFailure == NULL || pusDetailFailure == NULL)
	{
		return FALSE; // Init failed
	}
	
	// Initialize error conditions
	*pusTypeFailure = CLEAR_ERROR_CONDITION;
	*pusDetailFailure = CLEAR_ERROR_CONDITION;
	
	SnBool bStatus;
	
	// Initialize member variables
	m_hDriver = NULL; 
#if LOGGING
	m_hLogger = NULL; 
	m_bSaveToFlash = FALSE;
#endif
	InitializeCriticalSection (&m_csWriteFlashAccess);
	m_hGuiWnd = NULL;
	m_hIconWnd = NULL;
	m_hTacTimerThreadMotorA = NULL;
	m_hTacTimerThreadMotorB = NULL;
	m_hPersistentStorageThread = NULL;
	m_hStatusThread = NULL;
	m_hStatusThread2 = NULL;
	m_hEventHandlerThread = NULL;
	m_hRemoteStatusThread = NULL;
	m_bKillThreads = FALSE;
	m_bStorageThreadKilled = FALSE;
	m_bRemoteStatusThreadKilled = FALSE;
	m_bStatusThreadKilled = FALSE;
	m_bStatusThread2Killed = FALSE;
	m_hKillThreadEvent = NULL;
	m_hHandlerThreadKilledEvent = NULL;
	m_hRemoteStatusThreadKilledEvent = NULL;
	m_hStorageThreadKilledEvent = NULL;
	m_hStatusThreadKilledEvent = NULL;
	m_hStatusThread2KilledEvent = NULL;
	m_hCommFailureEvent = NULL;
	m_hTemperatureFailureEvent = NULL;
	m_hSystemResourceFailureEvent = NULL;
	m_hWatchDogTimerEvent = NULL;
	m_hDisplayInitializedEvent = NULL;
	m_hMutex = NULL;
	
	m_Footswitch.Initialize(this);
	
	// Port A Status
	m_PortA.Initialize(this, PORTA);
	
	// Port B Status
	m_PortB.Initialize(this, PORTB);
	
	CPort::SetControlsEnabled(TRUE);
	
	m_wBatteryStatus = BATTERY_GOOD;
	m_bShaverPacketDirty = FALSE;
	m_bRecallFlashFailed = FALSE;
	m_bRecallNvRamFailed = FALSE;	
	
	m_RemotePumpConnectionType = CPump::PUMP_TYPE_UNKNOWN;
	m_RemotePumpRunning = FALSE;
	
	m_bHardwareReady = FALSE;
	m_pPumpConnection = NULL;
	
	m_dwTemperature = CLEAR_ERROR_CONDITION; 
	m_bPulseBeeper = FALSE;
	m_bBeepThreeTimes = FALSE;
	m_ulDeviceID = 1;
	m_bCommunicationFailure = FALSE;
	m_bTimerExpired = FALSE;
	m_bTimer2Expired = FALSE;
	m_bFactoryMode = FALSE;
	
	SetDelayPeriodMs(SLEEP_PERIOD_20);
	
	// Initialize outgoing shaver information (Pump/Shaver interface)
	m_pucPumpData[0] = 0;
	m_pucPumpData[1] = 0;
	m_pucPumpData[2] = 0;
	m_pucPumpData[3] = 0;
	m_pucPumpData[4] = 0; 
	m_tOutGoingShaverPacket.ucBladeId = BLADE_ID_OTHER;
	m_tOutGoingShaverPacket.ucCmd = 0;
	m_tOutGoingShaverPacket.ucOpState = 0;
	m_tOutGoingShaverPacket.ucSpeedHigh = 0;
	m_tOutGoingShaverPacket.ucSpeedLow = 0;
	
	// Remote connection status
	m_usPumpStatus = MSG_REMOTE_DISCONNECTED;
	
	m_bFlashSaveError = FALSE;
	
	if( m_hDriver == NULL)
	{
		m_hDriver = (CDriver*)new(CDriver);
		
		// Open the Driver
		if( m_hDriver)
		{
			// Initialize the driver
			bStatus = m_hDriver->InitDriver();
			if(!bStatus)
			{
				*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
				goto InitFailed; // Init failed
			}
			
			g_hDriver = m_hDriver;
		}
		else
		{
			
			*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
			goto InitFailed;	// resource allocation failed
		}
	}
	
	
#if LOGGING
	if( m_hLogger == NULL)
	{
		m_hLogger = (CLogger*)new(CLogger);
		
		// Open the Logger for collecting run-time data
		if( m_hLogger)
		{
			// Initialize the Logger, use a 6 MB buffer
			bStatus = m_hLogger->InitLogger(6 * 0x100000);
			if(!bStatus)
			{
				*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
				goto InitFailed; // Init failed
			}
		}
		else
		{
			*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
			goto InitFailed;	// resource allocation failed
		}
	}
#endif
	
	
	// Create Mutex
	m_hMutex = CreateMutex(NULL, FALSE, NULL);
	if( m_hMutex == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	//
	// Create Events
	//
	// Create event to terminate running threads thread 
	m_hKillThreadEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if( m_hKillThreadEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event to indicate thread termination
	m_hStatusThreadKilledEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if( m_hStatusThreadKilledEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event to indicate thread termination
	m_hStatusThread2KilledEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if( m_hStatusThread2KilledEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event to indicate thread termination
	m_hStorageThreadKilledEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if( m_hStorageThreadKilledEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event to indicate thread termination
	m_hHandlerThreadKilledEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if( m_hHandlerThreadKilledEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event to indicate thread termination
	m_hRemoteStatusThreadKilledEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if( m_hRemoteStatusThreadKilledEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event to terminate the timer thread
	m_PortA.m_tPortEvents.m_hMotorKillTacTimerEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if( m_PortA.m_tPortEvents.m_hMotorKillTacTimerEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event to terminate the timer thread
	m_PortB.m_tPortEvents.m_hMotorKillTacTimerEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if( m_PortB.m_tPortEvents.m_hMotorKillTacTimerEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event to flag an over temperature condition
	m_hTemperatureFailureEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if( m_hTemperatureFailureEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event to flag a system communication failure
	m_hCommFailureEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if( m_hCommFailureEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event to flag system resourse failures
	m_hSystemResourceFailureEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
	if( m_hSystemResourceFailureEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortA.m_tPortEvents.m_hMotorStallEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortA.m_tPortEvents.m_hMotorStallEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortB.m_tPortEvents.m_hMotorStallEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortB.m_tPortEvents.m_hMotorStallEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortA.m_tPortEvents.m_hMotorTacFaultEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortA.m_tPortEvents.m_hMotorTacFaultEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortB.m_tPortEvents.m_hMotorTacFaultEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortB.m_tPortEvents.m_hMotorTacFaultEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortA.m_tPortEvents.m_hMotorShortCircuitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortA.m_tPortEvents.m_hMotorShortCircuitEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortB.m_tPortEvents.m_hMotorShortCircuitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortB.m_tPortEvents.m_hMotorShortCircuitEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortA.m_tPortEvents.m_hMotorShortCircuitTimeoutEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortA.m_tPortEvents.m_hMotorShortCircuitTimeoutEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortB.m_tPortEvents.m_hMotorShortCircuitTimeoutEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortB.m_tPortEvents.m_hMotorShortCircuitTimeoutEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	m_hWatchDogTimerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_hWatchDogTimerEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	m_hDisplayInitializedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_hDisplayInitializedEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortA.m_tPortEvents.m_hUnknownBladeIdEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortA.m_tPortEvents.m_hUnknownBladeIdEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortB.m_tPortEvents.m_hUnknownBladeIdEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortB.m_tPortEvents.m_hUnknownBladeIdEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortA.m_tPortEvents.m_hUnknownDeviceIdEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortA.m_tPortEvents.m_hUnknownDeviceIdEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortB.m_tPortEvents.m_hUnknownDeviceIdEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortB.m_tPortEvents.m_hUnknownDeviceIdEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortA.m_tPortEvents.m_hHallPatternFaultEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortA.m_tPortEvents.m_hHallPatternFaultEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortB.m_tPortEvents.m_hHallPatternFaultEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortB.m_tPortEvents.m_hHallPatternFaultEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortA.m_tPortEvents.m_hMotorCurrentLimitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortA.m_tPortEvents.m_hMotorCurrentLimitEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortA.m_tPortEvents.m_hMotorCurrentLimitTimeoutEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortA.m_tPortEvents.m_hMotorCurrentLimitTimeoutEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortA.m_tPortEvents.m_hMotorTorqueLimitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortA.m_tPortEvents.m_hMotorTorqueLimitEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortB.m_tPortEvents.m_hMotorCurrentLimitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortB.m_tPortEvents.m_hMotorCurrentLimitEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortB.m_tPortEvents.m_hMotorCurrentLimitTimeoutEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortB.m_tPortEvents.m_hMotorCurrentLimitTimeoutEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortB.m_tPortEvents.m_hMotorTorqueLimitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortB.m_tPortEvents.m_hMotorTorqueLimitEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	
	// Create event
	m_Footswitch.m_tFootswitchEvents.m_hLowBatteryEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_Footswitch.m_tFootswitchEvents.m_hLowBatteryEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_Footswitch.m_tFootswitchEvents.m_hStuckPedalEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_Footswitch.m_tFootswitchEvents.m_hStuckPedalEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_Footswitch.m_tFootswitchEvents.m_hUnknownIdEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_Footswitch.m_tFootswitchEvents.m_hUnknownIdEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortA.m_tPortEvents.m_hHandpieceStuckButtonEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortA.m_tPortEvents.m_hHandpieceStuckButtonEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortB.m_tPortEvents.m_hHandpieceStuckButtonEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortB.m_tPortEvents.m_hHandpieceStuckButtonEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortA.m_tPortEvents.m_hMotorStallAndCurrentLimitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortA.m_tPortEvents.m_hMotorStallAndCurrentLimitEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortB.m_tPortEvents.m_hMotorStallAndCurrentLimitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortB.m_tPortEvents.m_hMotorStallAndCurrentLimitEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortA.m_tPortEvents.m_hFootswitchRequiredEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortA.m_tPortEvents.m_hFootswitchRequiredEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	// Create event
	m_PortB.m_tPortEvents.m_hFootswitchRequiredEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if( m_PortB.m_tPortEvents.m_hFootswitchRequiredEvent == NULL)
	{ 
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	
	//
	// Create Threads
	//
	
	//Create and spawn thread that retrieves status
	m_hStatusThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
		0,
		StatusThread,
		this,
		0,
		&m_hStatusThreadID);
	
	if (m_hStatusThread == NULL)
	{
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	DEBUGMSG(TRUE, (TEXT("Control StatusThread: 0x%08X\n"),m_hStatusThreadID));
	
	// Bump up the priority 
	//	CeSetThreadPriority (m_hStatusThread, 110);
	SetThreadPriority(m_hStatusThread,THREAD_PRIORITY_TIME_CRITICAL);
	
	//Create and spawn thread that retrieves status
	m_hStatusThread2 = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
		0,
		StatusThread2,
		this,
		0,
		&m_hStatusThread2ID);
	
	if (m_hStatusThread2 == NULL)
	{
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	DEBUGMSG(TRUE, (TEXT("Control StatusThread2: 0x%08X\n"),m_hStatusThread2ID));
	
	//Create and spawn thread that periodically saves parameters to persis. storage
	m_hPersistentStorageThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
		0,
		StorageThread,
		this,
		0,
		&m_hPersistentStorageThreadID);
	
	if (m_hPersistentStorageThread == NULL)
	{
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	DEBUGMSG(TRUE, (TEXT("Control StorageThread: 0x%08X\n"),m_hPersistentStorageThreadID));
	
	// Set the thread priority
	//	SetThreadPriority(m_hPersistentStorageThread, THREAD_PRIORITY_BELOW_NORMAL);
	
	//Create and spawn thread that Handles Events
	m_hEventHandlerThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
		0,
		EventHandlerThread,
		this,
		0,
		&m_hEventHandlerThreadID);
	
	if (m_hEventHandlerThread == NULL)
	{
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	DEBUGMSG(TRUE, (TEXT("Control EventHandlerThread: 0x%08X\n"),m_hEventHandlerThreadID));
	
	//Create and spawn thread that monitors remote connections
	m_hRemoteStatusThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
		0,
		RemoteStatusThread,
		this,
		0,
		&m_hRemoteStatusThreadID);
	
	if (m_hRemoteStatusThread == NULL)
	{
		*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
		goto InitFailed;
	}
	DEBUGMSG(TRUE, (TEXT("Control RemoteStatusThread: 0x%08X\n"),m_hRemoteStatusThreadID));
	
	// Open the Pump/Shaver interface
	if( m_pPumpConnection == NULL)
	{
		m_pPumpConnection = (CPump*)new(CPump);
		if(m_pPumpConnection)
		{
			bStatus = m_pPumpConnection->Open(m_pucPumpData);
			if(!bStatus)
			{
				*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
				goto InitFailed; // Init failed
			}
		}
		else
		{
			*pusTypeFailure = SYSTEM_RESOURCE_ERROR;
			goto InitFailed;	// resource allocation failed
		}
		
	}
	
	// Initialize the Motor Control Board
	bStatus = InitHardwareShaver(pusTypeFailure, pusDetailFailure);
	if(!bStatus)
	{
		if (*pusTypeFailure == COMMUNICATION_ERROR)
			m_bCommunicationFailure = TRUE;
		goto InitFailed; // Hardware initialization failed
	}
	
	// Set System Default Parameters
	SetSystemDefaults();
	
	// Set Port Default speeds
	m_PortA.SetPortDefaultSpeeds();
	m_PortB.SetPortDefaultSpeeds();
	
	// Recall Persitent storage
	bStatus = RecallNvRamData();
	if( !bStatus)
		m_bRecallNvRamFailed = TRUE;
	
	if (m_usCustDefaultMode == CUSTOM_MODE)
	{
		bStatus = RecallFlashData();
		if( !bStatus)
			m_bRecallFlashFailed = TRUE;
	}
	else
	{
		// Copy default values to Port A and Port B structures
		memcpy(&m_PortA.m_tPortSavedParam, &m_PortA.m_tPortDefaultParam, sizeof(DEVICE_DATA));
		memcpy(&m_PortB.m_tPortSavedParam, &m_PortB.m_tPortDefaultParam, sizeof(DEVICE_DATA));
	}
	
	// Set up the Dwell periods. Oscillate Mode 1
	if(!m_PortA.WriteDwell())
	{
		m_bCommunicationFailure = TRUE;
		goto InitFailed; // Hardware initialization failed
	}
	if(!m_PortB.WriteDwell())
	{
		m_bCommunicationFailure = TRUE;
		goto InitFailed; // Hardware initialization failed
	}
	
	return TRUE;
	
InitFailed:
	DeInitHardware(TRUE);
	return FALSE;
}

// Function:	DeInit
// Purpose: 	Removes all used resources
// Parameters:	None
// Return:		None
//
void CControl::DeInit(SnBool yBuzzer)
{
	// Stop the timer thread if it's running
	if( m_hTacTimerThreadMotorA)
		StopTimer(TAC_TIMER_MOTORA);
	if( m_hTacTimerThreadMotorB)
		StopTimer(TAC_TIMER_MOTORB);
	
	// Bring down the hardware if needed
	DeInitHardware(yBuzzer);
	
	// Terminate any running threads
	KillThreads();
	
	DeleteCriticalSection(&m_csWriteFlashAccess);
	
	// Close the Driver.
	if( m_hDriver)
	{
		m_hDriver->DeInitDriver();
		delete m_hDriver;
		g_hDriver = m_hDriver = NULL;
	}
	
	// Close thread handles
	if( m_hStatusThread)
	{
		CloseHandle( m_hStatusThread);
		m_hStatusThread = NULL;
	}
	
	if( m_hStatusThread2)
	{
		CloseHandle( m_hStatusThread2);
		m_hStatusThread2 = NULL;
	}
	
	if( m_hPersistentStorageThread)
	{
		CloseHandle(m_hPersistentStorageThread);
		m_hPersistentStorageThread = NULL;
	}
	
	if( m_hEventHandlerThread)
	{
		CloseHandle( m_hEventHandlerThread);
		m_hEventHandlerThread = NULL;
	}
	
	if( m_hRemoteStatusThread)
	{
		CloseHandle(m_hRemoteStatusThread);
		m_hRemoteStatusThread = NULL;
	}
	
	if( m_hTacTimerThreadMotorA)
	{
		CloseHandle(m_hTacTimerThreadMotorA);
		m_hTacTimerThreadMotorA = NULL;
	}
	
	if(m_hTacTimerThreadMotorB)
	{
		CloseHandle(m_hTacTimerThreadMotorB);
		m_hTacTimerThreadMotorB = NULL;
	}
	
	// Close Event Handles
	if( m_hStatusThreadKilledEvent)
	{
		CloseHandle( m_hStatusThreadKilledEvent);
		m_hStatusThreadKilledEvent = NULL;
	}
	
	if( m_hStatusThread2KilledEvent)
	{
		CloseHandle( m_hStatusThread2KilledEvent);
		m_hStatusThread2KilledEvent = NULL;
	}
	
	if( m_hStorageThreadKilledEvent)
	{
		CloseHandle( m_hStorageThreadKilledEvent);
		m_hStorageThreadKilledEvent = NULL;
	}
	
	if( m_hHandlerThreadKilledEvent)
	{
		CloseHandle( m_hHandlerThreadKilledEvent);
		m_hHandlerThreadKilledEvent = NULL;
	}
	
	if( m_hRemoteStatusThreadKilledEvent)
	{
		CloseHandle( m_hRemoteStatusThreadKilledEvent);
		m_hRemoteStatusThreadKilledEvent = NULL;
	}
	
	if( m_PortA.m_tPortEvents.m_hMotorKillTacTimerEvent)
	{
		CloseHandle( m_PortA.m_tPortEvents.m_hMotorKillTacTimerEvent);
		m_PortA.m_tPortEvents.m_hMotorKillTacTimerEvent = NULL;
	}
	
	if( m_PortB.m_tPortEvents.m_hMotorKillTacTimerEvent)
	{
		CloseHandle( m_PortB.m_tPortEvents.m_hMotorKillTacTimerEvent);
		m_PortB.m_tPortEvents.m_hMotorKillTacTimerEvent = NULL;
	}
	
	if( m_hCommFailureEvent)
	{
		CloseHandle( m_hCommFailureEvent);
		m_hCommFailureEvent = NULL;
	}
	
	if( m_hTemperatureFailureEvent)
	{
		CloseHandle( m_hTemperatureFailureEvent);
		m_hTemperatureFailureEvent = NULL;
	}
	
	if( m_hSystemResourceFailureEvent)
	{
		CloseHandle(m_hSystemResourceFailureEvent);
		m_hSystemResourceFailureEvent = NULL;
	}
	
	if( m_hKillThreadEvent)
	{
		CloseHandle( m_hKillThreadEvent);
		m_hKillThreadEvent = NULL;
	}
	
	if( m_PortA.m_tPortEvents.m_hMotorStallEvent)
	{
		CloseHandle( m_PortA.m_tPortEvents.m_hMotorStallEvent);
		m_PortA.m_tPortEvents.m_hMotorStallEvent = NULL;
	}
	
	if( m_PortB.m_tPortEvents.m_hMotorStallEvent)
	{
		CloseHandle( m_PortB.m_tPortEvents.m_hMotorStallEvent);
		m_PortB.m_tPortEvents.m_hMotorStallEvent = NULL;
	}
	
	if( m_PortA.m_tPortEvents.m_hMotorTacFaultEvent)
	{
		CloseHandle( m_PortA.m_tPortEvents.m_hMotorTacFaultEvent);
		m_PortA.m_tPortEvents.m_hMotorTacFaultEvent = NULL;
	}
	
	if( m_PortB.m_tPortEvents.m_hMotorTacFaultEvent)
	{
		CloseHandle( m_PortB.m_tPortEvents.m_hMotorTacFaultEvent);
		m_PortB.m_tPortEvents.m_hMotorTacFaultEvent = NULL;
	}
	
	if( m_PortA.m_tPortEvents.m_hMotorShortCircuitEvent)
	{
		CloseHandle( m_PortA.m_tPortEvents.m_hMotorShortCircuitEvent);
		m_PortA.m_tPortEvents.m_hMotorShortCircuitEvent = NULL;
	}
	
	if( m_PortB.m_tPortEvents.m_hMotorShortCircuitEvent)
	{
		CloseHandle( m_PortB.m_tPortEvents.m_hMotorShortCircuitEvent);
		m_PortB.m_tPortEvents.m_hMotorShortCircuitEvent = NULL;
	}
	
	if( m_PortA.m_tPortEvents.m_hMotorShortCircuitTimeoutEvent)
	{
		CloseHandle( m_PortA.m_tPortEvents.m_hMotorShortCircuitTimeoutEvent);
		m_PortA.m_tPortEvents.m_hMotorShortCircuitTimeoutEvent = NULL;
	}
	
	if( m_PortB.m_tPortEvents.m_hMotorShortCircuitTimeoutEvent)
	{
		CloseHandle( m_PortB.m_tPortEvents.m_hMotorShortCircuitTimeoutEvent);
		m_PortB.m_tPortEvents.m_hMotorShortCircuitTimeoutEvent = NULL;
	}
	
	if( m_PortA.m_tPortEvents.m_hMotorCurrentLimitEvent)
	{
		CloseHandle( m_PortA.m_tPortEvents.m_hMotorCurrentLimitEvent);
		m_PortA.m_tPortEvents.m_hMotorCurrentLimitEvent = NULL;
	}
	
	if( m_PortA.m_tPortEvents.m_hMotorCurrentLimitTimeoutEvent)
	{
		CloseHandle( m_PortA.m_tPortEvents.m_hMotorCurrentLimitTimeoutEvent);
		m_PortA.m_tPortEvents.m_hMotorCurrentLimitTimeoutEvent = NULL;
	}
	
	if( m_PortA.m_tPortEvents.m_hMotorTorqueLimitEvent)
	{
		CloseHandle( m_PortA.m_tPortEvents.m_hMotorTorqueLimitEvent);
		m_PortA.m_tPortEvents.m_hMotorTorqueLimitEvent = NULL;
	}
	
	if( m_PortB.m_tPortEvents.m_hMotorCurrentLimitEvent)
	{
		CloseHandle( m_PortB.m_tPortEvents.m_hMotorCurrentLimitEvent);
		m_PortB.m_tPortEvents.m_hMotorCurrentLimitEvent = NULL;
	}
	
	if( m_PortB.m_tPortEvents.m_hMotorCurrentLimitTimeoutEvent)
	{
		CloseHandle( m_PortB.m_tPortEvents.m_hMotorCurrentLimitTimeoutEvent);
		m_PortB.m_tPortEvents.m_hMotorCurrentLimitTimeoutEvent = NULL;
	}
	
	if( m_PortB.m_tPortEvents.m_hMotorTorqueLimitEvent)
	{
		CloseHandle( m_PortB.m_tPortEvents.m_hMotorTorqueLimitEvent);
		m_PortB.m_tPortEvents.m_hMotorTorqueLimitEvent = NULL;
	}
	
	if( m_hWatchDogTimerEvent)
	{
		CloseHandle( m_hWatchDogTimerEvent);
		m_hWatchDogTimerEvent = NULL;
	}
	
	if( m_hDisplayInitializedEvent)
	{
		CloseHandle( m_hDisplayInitializedEvent);
		m_hDisplayInitializedEvent = NULL;
	}
	
	if( m_PortA.m_tPortEvents.m_hUnknownBladeIdEvent)
	{
		CloseHandle( m_PortA.m_tPortEvents.m_hUnknownBladeIdEvent);
		m_PortA.m_tPortEvents.m_hUnknownBladeIdEvent = NULL;
	}
	
	if( m_PortB.m_tPortEvents.m_hUnknownBladeIdEvent)
	{
		CloseHandle( m_PortB.m_tPortEvents.m_hUnknownBladeIdEvent);
		m_PortB.m_tPortEvents.m_hUnknownBladeIdEvent = NULL;
	}
	
	if( m_PortA.m_tPortEvents.m_hUnknownDeviceIdEvent)
	{
		CloseHandle( m_PortA.m_tPortEvents.m_hUnknownDeviceIdEvent);
		m_PortA.m_tPortEvents.m_hUnknownDeviceIdEvent = NULL;
	}
	
	if( m_PortB.m_tPortEvents.m_hUnknownDeviceIdEvent)
	{
		CloseHandle( m_PortB.m_tPortEvents.m_hUnknownDeviceIdEvent);
		m_PortB.m_tPortEvents.m_hUnknownDeviceIdEvent = NULL;
	}
	
	if( m_PortA.m_tPortEvents.m_hHallPatternFaultEvent)
	{
		CloseHandle( m_PortA.m_tPortEvents.m_hHallPatternFaultEvent);
		m_PortA.m_tPortEvents.m_hHallPatternFaultEvent = NULL;
	}
	
	if( m_PortB.m_tPortEvents.m_hHallPatternFaultEvent)
	{
		CloseHandle( m_PortB.m_tPortEvents.m_hHallPatternFaultEvent);
		m_PortB.m_tPortEvents.m_hHallPatternFaultEvent = NULL;
	}
	
	if( m_Footswitch.m_tFootswitchEvents.m_hLowBatteryEvent)
	{
		CloseHandle( m_Footswitch.m_tFootswitchEvents.m_hLowBatteryEvent);
		m_Footswitch.m_tFootswitchEvents.m_hLowBatteryEvent = NULL;
	}
	
	if( m_Footswitch.m_tFootswitchEvents.m_hStuckPedalEvent)
	{
		CloseHandle( m_Footswitch.m_tFootswitchEvents.m_hStuckPedalEvent);
		m_Footswitch.m_tFootswitchEvents.m_hStuckPedalEvent = NULL;
	}
	
	if( m_Footswitch.m_tFootswitchEvents.m_hUnknownIdEvent)
	{
		CloseHandle( m_Footswitch.m_tFootswitchEvents.m_hUnknownIdEvent);
		m_Footswitch.m_tFootswitchEvents.m_hUnknownIdEvent = NULL;
	}
	
	if( m_PortA.m_tPortEvents.m_hHandpieceStuckButtonEvent)
	{
		CloseHandle( m_PortA.m_tPortEvents.m_hHandpieceStuckButtonEvent);
		m_PortA.m_tPortEvents.m_hHandpieceStuckButtonEvent = NULL;
	}
	
	if( m_PortB.m_tPortEvents.m_hHandpieceStuckButtonEvent)
	{
		CloseHandle( m_PortB.m_tPortEvents.m_hHandpieceStuckButtonEvent);
		m_PortB.m_tPortEvents.m_hHandpieceStuckButtonEvent = NULL;
	}
	
	if( m_PortA.m_tPortEvents.m_hMotorStallAndCurrentLimitEvent)
	{
		CloseHandle( m_PortA.m_tPortEvents.m_hMotorStallAndCurrentLimitEvent);
		m_PortA.m_tPortEvents.m_hMotorStallAndCurrentLimitEvent = NULL;
	}
	
	if( m_PortB.m_tPortEvents.m_hMotorStallAndCurrentLimitEvent)
	{
		CloseHandle( m_PortB.m_tPortEvents.m_hMotorStallAndCurrentLimitEvent);
		m_PortB.m_tPortEvents.m_hMotorStallAndCurrentLimitEvent = NULL;
	}
	
	if( m_PortA.m_tPortEvents.m_hFootswitchRequiredEvent)
	{
		CloseHandle( m_PortA.m_tPortEvents.m_hFootswitchRequiredEvent);
		m_PortA.m_tPortEvents.m_hFootswitchRequiredEvent = NULL;
	}
	
	if( m_PortB.m_tPortEvents.m_hFootswitchRequiredEvent)
	{
		CloseHandle( m_PortB.m_tPortEvents.m_hFootswitchRequiredEvent);
		m_PortB.m_tPortEvents.m_hFootswitchRequiredEvent = NULL;
	}
	
	if( m_hMutex)
	{
		CloseHandle( m_hMutex);
		m_hMutex = NULL;
	}
	
#if LOGGING
	// Shut down the Logger
	if( m_hLogger)
	{
		m_hLogger->DeInitLogger();
		delete m_hLogger;
		m_hLogger = NULL;
	}
#endif
	
	// Close the shaver/pump interface
	if(m_pPumpConnection)
	{
		m_pPumpConnection->Close();
		delete m_pPumpConnection;
		m_pPumpConnection = NULL;
	}
}

// Function:	GetCmdState
// Purpose: 	Return requested information
//				
// Parameters:	qMsg =				command request
//				pwReturnBuffer =	pointer to return buffer of type SnWord 
//				qSize = 			Size of return buffer
//
// Return:		TRUE = Success, FALSE = Failure
//
SnBool CControl::GetCmdState(SnQByte qMsg, SnWord* pwReturnBuffer, SnQByte qSize)
{
	SnBool bStatus;
	
	bStatus = TRUE;
	
	if( qSize < sizeof(SnWord))
		return FALSE;
	
	if( pwReturnBuffer == NULL)
		return FALSE;
	
	switch(qMsg)
	{
	case GET_MC_FOOT_TYPE:
		*pwReturnBuffer = m_Footswitch.m_tFootPedalStatus.usType;
		break;
		
	case GET_SYSTEM_LANGUAGE:
		*pwReturnBuffer = m_usLanguage;
		break;
		
	case GET_SHAVER_PACKET_CTL:
		*pwReturnBuffer = m_usShaverPacketControl;
		break;
		
		
	case GET_SYSTEM_MODE:
		*pwReturnBuffer = m_usCustDefaultMode;
		break;
		
	case GET_SYSTEM_BATTERY_STATUS:
		*pwReturnBuffer = m_wBatteryStatus;
		break;
		
	default:
		bStatus = FALSE;
		break;
	}
	
	return bStatus;
}


// Function:	GetCmdState
// Purpose: 	Returns requested information
//				
// Parameters:	qMsg =				command request
//				ptReturnBuffer =	pointer to return buffer of type SN_PORT_STATUS
//				qSize = 			Size of return buffer
//
// Return:		TRUE = Success, FALSE = Failure
//
SnBool CControl::GetCmdState(SnQByte qMsg, SN_PORT_STATUS* ptReturnBuffer, SnQByte qSize)
{
	SnBool bStatus;
	
	bStatus = TRUE;
	
	if( qSize < sizeof(SN_PORT_STATUS))
		return FALSE;
	
	if( ptReturnBuffer == NULL)
		return FALSE;
	
	switch(qMsg)
	{
	case GET_MC_PORTA_STATUS:
		*ptReturnBuffer = m_PortA.m_tPortStatus;
		break;
		
	case GET_MC_PORTB_STATUS:
		*ptReturnBuffer = m_PortB.m_tPortStatus;
		break;
		
	default:
		bStatus = FALSE;
		break;
	}
	
	return bStatus;
}

// Function:	GetCmdState
// Purpose: 	Returns requested information
//				
// Parameters:	qMsg =				command request
/// 			ptReturnBuffer =	pointer to return buffer of type SN_FOOT_STATUS
//				qSize = 			Size of return buffer
//
// Return:		TRUE = Success, FALSE = Failure
//
SnBool CControl::GetCmdState(SnQByte qMsg, SN_FOOT_STATUS* ptReturnBuffer, SnQByte qSize)
{
	
	SnBool bStatus;
	
	bStatus = TRUE;
	
	if( qSize < sizeof(SN_FOOT_STATUS))
		return FALSE;
	
	if( ptReturnBuffer == NULL)
		return FALSE;
	
	switch(qMsg)
	{
	case GET_MC_FOOT_STATUS:
		*ptReturnBuffer = m_Footswitch.m_tFootPedalStatus;
		break;
		
	default:
		bStatus = FALSE;
		break;
	}
	
	return bStatus;
}

// Function:	GetCmdState
// Purpose: 	Returns requested information
//				
// Parameters:	qMsg =				command request
/// 			ptReturnBuffer =	pointer to return buffer of type SN_SYS_REVISION
//				qSize = 			Size of return buffer
//
// Return:		TRUE = Success, FALSE = Failure
//
SnBool CControl::GetCmdState(SnQByte qMsg, SN_SYS_REVISION* ptReturnBuffer, SnQByte qSize)
{
	
	SnBool bStatus;
	
	bStatus = TRUE;
	
	if( qSize < sizeof(SN_SYS_REVISION))
		return FALSE;
	
	if( ptReturnBuffer == NULL)
		return FALSE;
	
	switch(qMsg)
	{	
	case GET_SYSTEM_REVISIONS:
		bStatus = GetRevisionNumbers(ptReturnBuffer);
		break;
		
	default:
		bStatus = FALSE;
		break;
	}
	
	return bStatus;
}

// Function:	SetCmdState
// Purpose: 	Performs request
// Parameters:	qMsg =		Command request
//				pwInputBuffer = Pointer to buffer of type SnWord specifying command data 
//				qSize = 		Size of input buffer
//
// Return:		TRUE = Success, FALSE = Failure
//
SnBool CControl::SetCmdState(SnQByte qMsg, SnWord* pwInputBuffer, SnQByte qSize)
{
	
	SnBool bStatus;
	
	bStatus = TRUE;
	
	if( qSize > sizeof(SnWord))
		return FALSE;
	
	if( pwInputBuffer == NULL)
		return FALSE;
	
	switch(qMsg)
	{
	case SET_SYSTEM_BEEPER:
		if( m_bHardwareReady)
		{
			if( *pwInputBuffer == BEEPER_ON)
			{
				m_bPulseBeeper = TRUE;
			}
			else if( *pwInputBuffer == BEEPER_OFF)
			{
				m_bPulseBeeper = FALSE;
				// Turn the beeper off
				if(m_hDriver)
					bStatus = m_hDriver->SetSystemBuzzer(FALSE);
			}
			else
				bStatus = FALSE;
		}
		else
			bStatus = FALSE;
		break;
		
	case SET_MC_CONTROLS:
		CPort::SetControlsEnabled((SnBool)*pwInputBuffer); 
		break;
		
	case SET_SYSTEM_PARAMETERS:
		if( *pwInputBuffer == SAVE_NVRAM)
			bStatus = SaveNvRamData();
		else if( *pwInputBuffer == SAVE_FLASH)
		{
			m_PortA.SetPortCustomSpeeds();
			m_PortB.SetPortCustomSpeeds();
		}
		break;
		
	case SET_MC_WINDOW_LOCK_PORTA:
		m_PortA.SetWindowLock(*pwInputBuffer, m_Footswitch.Override());
		break;
		
	case SET_MC_WINDOW_LOCK_PORTB:
		m_PortB.SetWindowLock(*pwInputBuffer, m_Footswitch.Override());
		break;
		
	case SET_SYSTEM_LANGUAGE:
		m_usLanguage = *pwInputBuffer; 
		break;
		
	case SET_SHAVER_PACKET_CTL:
		m_usShaverPacketControl = *pwInputBuffer; 
		break;
		
	case SET_SYSTEM_MODE:
		if (m_usCustDefaultMode == DEFAULT_MODE && *pwInputBuffer == CUSTOM_MODE)
			RecallFlashData();
		
		m_usCustDefaultMode = *pwInputBuffer;
		m_PortA.GetPortParameters(); // Update port information 
		m_PortB.GetPortParameters(); // Update port information
		break;
		
	case SET_FACTORY_TEST_MODE:
		if( *pwInputBuffer == 1) {
			m_bFactoryMode = TRUE;
		} else {
			m_bFactoryMode = FALSE;
		}
		break;
		
	case SET_MC_BEEPER:
		if (*pwInputBuffer == BEEPER_ON) {
			SnWord wOutput = 1;
			bStatus = WriteDsp(MC_AUXILLARY, 1, &wOutput);
			if (!bStatus) {
				SetEvent(m_hCommFailureEvent); // Failure condition
				bStatus = FALSE;
			}
		} else if (*pwInputBuffer == BEEPER_OFF) {
			SnWord wOutput = 0;
			bStatus = WriteDsp(MC_AUXILLARY, 1, &wOutput);
			if (!bStatus) {
				SetEvent(m_hCommFailureEvent); // Failure condition
				bStatus = FALSE;
			}
		} else {
			bStatus = FALSE;
		}
		break;
	case SET_FOOTSWITCH_RESET:
		bStatus = m_Footswitch.StartStopSoftwareReset(*pwInputBuffer);
		break;
		
	case SET_FOOTSWITCH_CALIBRATE:
		bStatus = m_Footswitch.StartStopCalibration(*pwInputBuffer);
		break;
		
	case SET_FOOTSWITCH_INVALID_TEST:
		bStatus = m_Footswitch.StartStopInvalidCommandTest(*pwInputBuffer);
		break;
	default:
		bStatus = FALSE;
		break;
	}
	
	return bStatus;
}


// Function:	SetCmdState
// Purpose: 	Examine command and performs request
// Parameters:	qMsg =			Command request
//				ptInputBuffer = Pointer to buffer of type SN_PORT_STATUS specifying command data 
//				qSize = 		Size of input buffer
//
// Return:		TRUE = Success, FALSE = Failure
//
SnBool CControl::SetCmdState(SnQByte qMsg, SN_PORT_STATUS* ptInputBuffer, SnQByte qSize)
{
	SnBool bStatus;
	
	bStatus = TRUE;
	
	if( qSize > sizeof(SN_PORT_STATUS))
		return FALSE;
	
	if( ptInputBuffer == NULL)
		return FALSE;
	
	switch(qMsg)
	{
	case SET_MC_PORTA_STATUS:
		m_PortA.SetPortStatus(ptInputBuffer);
		break;
		
	case SET_MC_PORTB_STATUS: 
		m_PortB.SetPortStatus(ptInputBuffer);
		break;
		
	default:
		bStatus = FALSE;
		break;
	}
	
	return bStatus;
}


// Function:	SetCmdState
// Purpose: 	Performs request
// Parameters:	qMsg =			Command request
//				ptInputBuffer = Pointer to buffer of type SN_FOOT_STATUS specifying command data 
//				qSize = 		Size of input buffer
//
// Return:		TRUE = Success, FALSE = Failure
//
SnBool CControl::SetCmdState(SnQByte qMsg, SN_FOOT_STATUS* ptInputBuffer, SnQByte qSize)
{
	
	SnBool bStatus;
	
	bStatus = TRUE;
	
	if( qSize > sizeof(SN_FOOT_STATUS))
		return FALSE;
	
	if( ptInputBuffer == NULL)
		return FALSE;
	
	switch(qMsg)
	{
	case SET_MC_FOOT_STATUS:
		m_Footswitch.m_tFootPedalStatus.usForward = ptInputBuffer->usForward;
		m_Footswitch.m_tFootPedalStatus.usMode = ptInputBuffer->usMode;
		m_Footswitch.m_tFootPedalStatus.usOverride = ptInputBuffer->usOverride;
		m_Footswitch.m_tFootPedalStatus.usPortControl = ptInputBuffer->usPortControl;
		break;
		
	default:
		bStatus = FALSE;
		break;
	}
	
	return bStatus;
}


// Function:	GetRevisionNumbers
// Purpose: 	Gets the system and motor board revision numbers
// Parameters:	ptRevision = Specifies the address of a SN_SYS_REVISION structure that receives the revision numbers
//
// Return:		TRUE = Success, FALSE = Failure
//
SnBool CControl::GetRevisionNumbers( SN_SYS_REVISION* ptRevision)
{
	SnWinCeHdr tWinCeHdr;
	SnBool bStatus;
	SnWord pwRevision[2];
	
	// Get the revision numbers from the Motor Control Board
	bStatus = ReadDsp( MC_REVISION, 2, pwRevision);
	if (!bStatus)
		return FALSE;
	
	ptRevision->ucMotorBoardMajor = (pwRevision[1] & 0xff);
	ptRevision->ucMotorBoardMinor = pwRevision[0] >> 8;
	ptRevision->ucMotorBoardBuild = (pwRevision[0] & 0xff);
	
	// Get the system Revision Numbers
	if (m_hDriver == NULL) {
		return FALSE;
	}
	
	// First try the lower image at offset
	bStatus =  m_hDriver->ReadFlashData(LOWER_FLASH_ADDR, (SnByte*)&tWinCeHdr,
		sizeof(SnWinCeHdr));
	if (!bStatus)
		return FALSE;
	
	// If the Header data is corrupt, try the other offset
	if (CrcMem((SnByte *)&tWinCeHdr, sizeof(SnWinCeHdr)) != 0) {
		bStatus =  m_hDriver->ReadFlashData(UPPER_FLASH_ADDR, (SnByte*)&tWinCeHdr,
			sizeof(SnWinCeHdr));
		if(!bStatus)
			return FALSE;
		
		// If both images are corrupt, then return an error
		if (CrcMem((SnByte *)&tWinCeHdr, sizeof(SnWinCeHdr)) != 0) {
			return FALSE;
		}
	}
	
	ptRevision->ucSystemMajor = tWinCeHdr.bMajorVers;
	ptRevision->ucSystemMinor = tWinCeHdr.bMinorVers;
	ptRevision->ucSystemBuild = tWinCeHdr.bBuildVers;
	
	m_Footswitch.GetRS485DeviceVersion(ptRevision->ucFootswitchMajor, ptRevision->ucFootswitchMinor,ptRevision->ucFootswitchBuild);
	m_PortA.GetRS485DeviceVersion(ptRevision->ucPortAMajor, ptRevision->ucPortAMinor,ptRevision->ucPortABuild);
	m_PortB.GetRS485DeviceVersion(ptRevision->ucPortBMajor, ptRevision->ucPortBMinor,ptRevision->ucPortBBuild);
	return TRUE;
}

// Function:	WriteDsp
// Purpose: 	Writes to a device through the Dsp interface
// Parameters:	qDevID =		Device ID
//				qOffset =		Register offset
//				qNumWords = 	Number of words to be written
//				pwData =		Pointer to buffer that contains data to write to device
//
// Return:		TRUE = Success, FALSE = Failure
//
SnBool CControl::WriteDsp(SnQByte qOffset, SnQByte qNumWords, SnWord *pwData)
{
	SnBool bStatus;
	
	if(!m_hDriver)
		return FALSE;
	
	if( qNumWords < 1 || pwData == NULL)
		return FALSE;
	
	if(qNumWords == 1)
	{
		// Write one word
		bStatus = m_hDriver->WriteWordToDevice(qOffset, *pwData);
		if(!bStatus)
		{
			SetEvent(m_hCommFailureEvent);
		}
	}
	else
	{
		// Write qNumWords word
		bStatus = m_hDriver->WriteWordsToDevice(qOffset, qNumWords, pwData);	
		if(!bStatus)
		{
			SetEvent(m_hCommFailureEvent);
		}
	}
	
	return bStatus;
	
}

// Function:	ReadDsp
// Purpose: 	Reads a device through the Dsp interface
// Parameters:	qDevID =		Device ID
//				qOffset =		Register offset
//				qNumWords = 	Number of words to be read
//				pwData =		Pointer to buffer that will contain data read from device
//
// Return:		TRUE = Success, FALSE = Failure
//
SnBool CControl::ReadDsp(SnQByte qOffset, SnQByte qNumWords, SnWord *pwData)
{
	SnBool bStatus;
	
	if(!m_hDriver)
		return FALSE;
	
	if( qNumWords < 1 || pwData == NULL)
		return FALSE;
	
	if(qNumWords == 1)
	{
		// Read one word(16 bits)
		bStatus = m_hDriver->ReadWordFromDevice(qOffset, pwData);
		if(!bStatus)
		{
			SetEvent(m_hCommFailureEvent); // Failure
			
		}
	}
	else
	{
		// Read qNumWords
		bStatus = m_hDriver->ReadWordsFromDevice(qOffset,qNumWords,pwData);
		if(!bStatus)
		{
			SetEvent(m_hCommFailureEvent); //Failure
		}	
	}
	
	return bStatus;
	
}

// Function:	InitHardwareShaver
// Purpose: 	Initializes the Hardware 
// Parameters:	pwTypeFailure = Pointer to SnWord that receives the Failure Type
//				pwDetailFailure = Pointer to SnWord that receives the Failure Details
//
// Return:		TRUE = Success, FALSE = Failure
//
SnBool CControl::InitHardwareShaver(SnWord* pwTypeFailure, SnWord* pwDetailFailure)
{
	SnBool	bStatus;
	
	// Initialize the hardware
	if( m_hDriver)
	{
		//
		// Configure Hardware for Shaver application
		//
		Status_Control sShaver;
		SnQByte qNumWords;
		
		
		// Bldc A Config motor for safe startup
		sShaver.tBldcA.tEx = g_tBldcNoMotor;	
		qNumWords = (sizeof(sShaver.tBldcA.tEx)/2)-2;
		
		// Set BldcA config
		bStatus = WriteDsp((offsetof(Status_Control, tBldcA.tEx))/2, qNumWords, (SnWord*)&sShaver.tBldcA.tEx);
		if( !bStatus)
		{
			*pwTypeFailure = COMMUNICATION_ERROR;
			return FALSE;
		}
		
		// Bldc B config motor for safe startup
		sShaver.tBldcB.tEx = g_tBldcNoMotor;
		qNumWords = (sizeof(sShaver.tBldcB.tEx)/2)-2;
		
		// Set BldcB config
		bStatus = WriteDsp((offsetof(Status_Control, tBldcB.tEx))/2, qNumWords, (SnWord*)&sShaver.tBldcB.tEx);
		if( !bStatus)
		{
			*pwTypeFailure = COMMUNICATION_ERROR;
			return FALSE;
		}
		
		// Port Types
		sShaver.wPortType = 0x8080; // Port Type No Motor
		qNumWords = sizeof(sShaver.wPortType)/2;
		
		// Set Port Types
		bStatus = WriteDsp( (offsetof(Status_Control, wPortType))/2, qNumWords, (SnWord*)&sShaver.wPortType);
		if( !bStatus)
		{
			*pwTypeFailure = COMMUNICATION_ERROR;
			return FALSE;
		}
		// HallBus A
		sShaver.tHallBusA.wDeviceExist = 0;
		sShaver.tHallBusA.wDeviceActive = 0;
		sShaver.tHallBusA.wDeviceLatch = 0;
		
		qNumWords = sizeof(sShaver.tHallBusA)/2;
		
		// Set HallBus A config
		bStatus = WriteDsp((offsetof(Status_Control, tHallBusA))/2, qNumWords, (SnWord*)&sShaver.tHallBusA);
		if( !bStatus)
		{
			*pwTypeFailure = COMMUNICATION_ERROR;
			return FALSE;
		}
		
		// HallBus B
		sShaver.tHallBusB.wDeviceExist = 0;
		sShaver.tHallBusB.wDeviceActive = 0;
		sShaver.tHallBusB.wDeviceLatch = 0;
		
		qNumWords = sizeof(sShaver.tHallBusB)/2;
		
		// Set HallBus B config
		bStatus = WriteDsp((offsetof(Status_Control, tHallBusB))/2, qNumWords, (SnWord*)&sShaver.tHallBusB);
		if( !bStatus)
		{
			*pwTypeFailure = COMMUNICATION_ERROR;
			return FALSE;
		}
		
		// Digital Inputs
		sShaver.tDigital.wStateData[0] = 0;
		sShaver.tDigital.wStateData[1] = 0;
		sShaver.tDigital.wStateData[2] = 0;
		sShaver.tDigital.wStateData[3] = 0;
		sShaver.tDigital.wStateData[4] = 0;
		sShaver.tDigital.wStateData[5] = 0;
		
		sShaver.tDigital.wNewData[0] = 0;
		sShaver.tDigital.wNewData[1] = 0;
		sShaver.tDigital.wNewData[2] = 0;
		sShaver.tDigital.wNewData[3] = 0;
		sShaver.tDigital.wNewData[4] = 0;
		sShaver.tDigital.wNewData[5] = 0;
		
		sShaver.tDigital.wOldData[0] = 0;
		sShaver.tDigital.wOldData[1] = 0;
		sShaver.tDigital.wOldData[2] = 0;
		sShaver.tDigital.wOldData[3] = 0;
		sShaver.tDigital.wOldData[4] = 0;
		sShaver.tDigital.wOldData[5] = 0;
		
		sShaver.tDigital.wActive[0] = 0;
		sShaver.tDigital.wActive[1] = 0x003F;	   // Port A & B - Enable A_Logic0-2 & B_Logic0-2
		sShaver.tDigital.wActive[2] = 0;
		sShaver.tDigital.wActive[3] = 0x0007;	   // Enable Footswitch (Footswitch0-2)
		sShaver.tDigital.wActive[4] = 0x0000;	   // Drill direction bits turned off (Bits 2, 3)
		sShaver.tDigital.wActive[5] = 0;
		
		sShaver.tDigital.wAssert[0] = 0;
		sShaver.tDigital.wAssert[1] = 0;
		sShaver.tDigital.wAssert[2] = 0;
		sShaver.tDigital.wAssert[3] = 0;
		sShaver.tDigital.wAssert[4] = 0;
		sShaver.tDigital.wAssert[5] = 0;
		
		sShaver.tDigital.wActiveLow[0] = 0;
		sShaver.tDigital.wActiveLow[1] = 0x003F;
		sShaver.tDigital.wActiveLow[2] = 0;
		sShaver.tDigital.wActiveLow[3] = 0x0007;
		sShaver.tDigital.wActiveLow[4] = 0x000C;
		sShaver.tDigital.wActiveLow[5] = 0;
		
		sShaver.tDigital.wInputType[0] = 0;
		sShaver.tDigital.wInputType[1] = 0;
		sShaver.tDigital.wInputType[2] = 0;
		sShaver.tDigital.wInputType[3] = 0;
		sShaver.tDigital.wInputType[4] = 0;
		sShaver.tDigital.wInputType[5] = 0;
		
		sShaver.tDigital.wDebounce[0] = 0;
		sShaver.tDigital.wDebounce[1] = 900;
		sShaver.tDigital.wDebounce[2] = 0;
		sShaver.tDigital.wDebounce[3] = 900;
		sShaver.tDigital.wDebounce[4] = 900;
		sShaver.tDigital.wDebounce[5] = 0;
		
		sShaver.tDigital.wEvent = 0;
		
		qNumWords = sizeof(sShaver.tDigital)/2;
		
		// Set Digital In config
		bStatus = WriteDsp((offsetof(Status_Control, tDigital))/2, qNumWords, (SnWord*)&sShaver.tDigital);
		if( !bStatus)
		{
			*pwTypeFailure = COMMUNICATION_ERROR;
			return FALSE;
		}
		
		// Analog Inputs
		sShaver.tAnalog.sData[0] = 0;
		sShaver.tAnalog.sData[1] = 0;
		sShaver.tAnalog.sData[2] = 0;
		sShaver.tAnalog.sData[3] = 0;
		sShaver.tAnalog.sData[4] = 0;
		sShaver.tAnalog.sData[5] = 0;
		sShaver.tAnalog.sData[6] = 0;
		sShaver.tAnalog.sData[7] = 0;
		sShaver.tAnalog.sData[8] = 0;
		sShaver.tAnalog.sData[9] = 0;
		sShaver.tAnalog.sData[10] = 0;
		sShaver.tAnalog.sData[11] = 0;
		sShaver.tAnalog.sData[12] = 0;
		sShaver.tAnalog.sData[13] = 0;
		sShaver.tAnalog.sData[14] = 0;
		sShaver.tAnalog.sData[15] = 0;
		
		sShaver.tAnalog.sAverage[0] = 0;
		sShaver.tAnalog.sAverage[1] = 2000;
		sShaver.tAnalog.sAverage[2] = 2000;
		sShaver.tAnalog.sAverage[3] = 0;
		sShaver.tAnalog.sAverage[4] = 0;
		sShaver.tAnalog.sAverage[5] = 0;
		sShaver.tAnalog.sAverage[6] = 0;
		sShaver.tAnalog.sAverage[7] = 0;
		sShaver.tAnalog.sAverage[8] = 0;
		sShaver.tAnalog.sAverage[9] = 0;
		sShaver.tAnalog.sAverage[10] = 0;
		sShaver.tAnalog.sAverage[11] = 0;
		sShaver.tAnalog.sAverage[12] = 0;
		sShaver.tAnalog.sAverage[13] = 0;
		sShaver.tAnalog.sAverage[14] = 0;
		sShaver.tAnalog.sAverage[15] = 0;
		
		
		sShaver.tAnalog.sOffset[0] = 45;
		sShaver.tAnalog.sOffset[1] = 0;
		sShaver.tAnalog.sOffset[2] = 0;
		sShaver.tAnalog.sOffset[3] = 0;
		sShaver.tAnalog.sOffset[4] = 0;
		sShaver.tAnalog.sOffset[5] = 0;
		sShaver.tAnalog.sOffset[6] = 0;
		sShaver.tAnalog.sOffset[7] = 0;
		sShaver.tAnalog.sOffset[8] = 45;
		sShaver.tAnalog.sOffset[9] = 0;
		sShaver.tAnalog.sOffset[10] = 0;
		sShaver.tAnalog.sOffset[11] = 0;
		sShaver.tAnalog.sOffset[12] = 0;
		sShaver.tAnalog.sOffset[13] = 0;
		sShaver.tAnalog.sOffset[14] = 0;
		sShaver.tAnalog.sOffset[15] = 0;
		
		sShaver.tAnalog.wInvert[0] = 0;
		sShaver.tAnalog.wInvert[1] = 0;
		sShaver.tAnalog.wInvert[2] = 0;
		sShaver.tAnalog.wInvert[3] = 0;
		sShaver.tAnalog.wInvert[4] = 0;
		sShaver.tAnalog.wInvert[5] = 0;
		sShaver.tAnalog.wInvert[6] = 0;
		sShaver.tAnalog.wInvert[7] = 0;
		sShaver.tAnalog.wInvert[8] = 0;
		sShaver.tAnalog.wInvert[9] = 0;
		sShaver.tAnalog.wInvert[10] = 0;
		sShaver.tAnalog.wInvert[11] = 0;
		sShaver.tAnalog.wInvert[12] = 0;
		sShaver.tAnalog.wInvert[13] = 0;
		sShaver.tAnalog.wInvert[14] = 0;
		sShaver.tAnalog.wInvert[15] = 0;
		
		
		sShaver.tAnalog.wCount[0] = 10;
		sShaver.tAnalog.wCount[1] = 500;
		sShaver.tAnalog.wCount[2] = 500;
		sShaver.tAnalog.wCount[3] = 1;
		sShaver.tAnalog.wCount[4] = 50;
		sShaver.tAnalog.wCount[5] = 50;
		sShaver.tAnalog.wCount[6] = 50;
		sShaver.tAnalog.wCount[7] = 50;
		sShaver.tAnalog.wCount[8] = 10;
		sShaver.tAnalog.wCount[9] = 1;
		sShaver.tAnalog.wCount[10] = 50;
		sShaver.tAnalog.wCount[11] = 50;
		sShaver.tAnalog.wCount[12] = 50;
		sShaver.tAnalog.wCount[13] = 0;
		sShaver.tAnalog.wCount[14] = 0;
		sShaver.tAnalog.wCount[15] = 0;
		
		sShaver.tAnalog.fGain[0] = 1;
		sShaver.tAnalog.fGain[1] = 1;
		sShaver.tAnalog.fGain[2] = 1;
		sShaver.tAnalog.fGain[3] = 1;
		sShaver.tAnalog.fGain[4] = 1;
		sShaver.tAnalog.fGain[5] = 1;
		sShaver.tAnalog.fGain[6] = 1;
		sShaver.tAnalog.fGain[7] = 1;
		sShaver.tAnalog.fGain[8] = 1;
		sShaver.tAnalog.fGain[9] = 1;
		sShaver.tAnalog.fGain[10] = 1;
		sShaver.tAnalog.fGain[11] = 1;
		sShaver.tAnalog.fGain[12] = 1;
		sShaver.tAnalog.fGain[13] = 0;
		sShaver.tAnalog.fGain[14] = 0;
		sShaver.tAnalog.fGain[15] = 0;
		
		sShaver.tAnalog.wAssert[0] = 0;
		sShaver.tAnalog.wAssert[1] = 0;
		sShaver.tAnalog.wAssert[2] = 0;
		sShaver.tAnalog.wAssert[3] = 0;
		sShaver.tAnalog.wAssert[4] = 400;
		sShaver.tAnalog.wAssert[5] = 0;
		sShaver.tAnalog.wAssert[6] = 0;
		sShaver.tAnalog.wAssert[7] = 400;
		sShaver.tAnalog.wAssert[8] = 0;
		sShaver.tAnalog.wAssert[9] = 0;
		sShaver.tAnalog.wAssert[10] = 0;
		sShaver.tAnalog.wAssert[11] = 0;
		sShaver.tAnalog.wAssert[12] = 0;
		sShaver.tAnalog.wAssert[13] = 0;
		sShaver.tAnalog.wAssert[14] = 0;
		sShaver.tAnalog.wAssert[15] = 0;
		
		qNumWords = sizeof(sShaver.tAnalog)/2;
		
		// Set Analog Inputs  config
		bStatus = WriteDsp((offsetof(Status_Control, tAnalog))/2, qNumWords, (SnWord*)&sShaver.tAnalog);
		if( !bStatus)
		{
			*pwTypeFailure = COMMUNICATION_ERROR;
			return FALSE;
		}
		
		sShaver.tTemperature.fOnBoardTemp = 0;
		sShaver.tTemperature.fOnBoardHiLimit = 50.0;
		sShaver.tTemperature.fOnBoardLoLimit = 5.0;
		sShaver.tTemperature.fDspTemp = 0;
		sShaver.tTemperature.fDspLimit = 100.0;
		sShaver.tTemperature.wPriority = 0;
		sShaver.tTemperature.wEvent = 0;
		
		qNumWords = sizeof(sShaver.tTemperature)/2;
		
		// Set Temperature config
		bStatus = WriteDsp((offsetof(Status_Control, tTemperature))/2, qNumWords, (SnWord*)&sShaver.tTemperature);
		if( !bStatus)
		{
			*pwTypeFailure = COMMUNICATION_ERROR;
			return FALSE;
		}
		
		sShaver.tInterrupt.wEnable = 0;
		sShaver.tInterrupt.wFlags = 0;
		
		// Set Interrupt config
		qNumWords = sizeof(sShaver.tInterrupt)/2;
		
		bStatus = WriteDsp((offsetof(Status_Control, tInterrupt))/2, qNumWords, (SnWord*)&sShaver.tInterrupt);
		if( !bStatus)
		{
			*pwTypeFailure = COMMUNICATION_ERROR;
			return FALSE;
		}
		
		sShaver.tSerial.wNumCmds = 0;
		sShaver.tSerial.wCmdResult[0] = 0;
		sShaver.tSerial.wCmdResult[1] = 0;
		sShaver.tSerial.wCmdResult[2] = 0;
		sShaver.tSerial.wCmdResult[3] = 0;
		sShaver.tSerial.wCmdResult[4] = 0;
		sShaver.tSerial.wCmdResult[5] = 0;
		sShaver.tSerial.wCmdResult[6] = 0;
		sShaver.tSerial.wCmdResult[7] = 0;
		sShaver.tSerial.wCmdResult[8] = 0;
		sShaver.tSerial.wCmdResult[9] = 0;
		sShaver.tSerial.wCmdResult[10] = 0;
		sShaver.tSerial.wCmdResult[11] = 0;
		sShaver.tSerial.wCmdResult[12] = 0;
		sShaver.tSerial.wCmdResult[13] = 0;
		sShaver.tSerial.wCmdResult[14] = 0;
		sShaver.tSerial.wCmdResult[15] = 0;
		sShaver.tSerial.wRcvErrCnt0 = 0;
		sShaver.tSerial.wRcvErrCnt1 = 0;
		
		// Set Serial 0 config
		qNumWords = sizeof(sShaver.tSerial)/2;
		
		bStatus = WriteDsp((offsetof(Status_Control, tSerial))/2, qNumWords, (SnWord*)&sShaver.tSerial);
		if( !bStatus)
		{
			*pwTypeFailure = COMMUNICATION_ERROR;
			return FALSE;
		}
		//
		// Done Hardware Configuration
		//
		
	}	
	return TRUE;
}

// Function:	SetSystemDefaults
// Purpose: 	Sets the default values for system parameters
// Parameters:	None
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool CControl::SetSystemDefaults()
{
	// Set the defaults
	m_usCustDefaultMode = CUSTOM_MODE; // Default/Custom
	m_usShaverPacketControl = PORTA;
	m_usPrevShaverPacketControl = PORTA;
	
	m_PortA.SetSystemDefaults();
	m_PortB.SetSystemDefaults();
	
	m_Footswitch.SetSystemDefaults();
	
	// Setup the NVRAM default struct
	m_tNvRamDefaults.ucFootMode = m_Footswitch.m_tFootPedalStatus.usMode;
	m_tNvRamDefaults.ucFootForward = m_Footswitch.m_tFootPedalStatus.usForward;
	m_tNvRamDefaults.ucFootHandCtl = m_Footswitch.m_tFootPedalStatus.usOverride;
	m_tNvRamDefaults.ucPortCtl = m_Footswitch.m_tFootPedalStatus.usPortControl;
	m_tNvRamDefaults.ucOscModePortA = m_PortA.m_tPortStatus.usOscMode ; 
	m_tNvRamDefaults.ucOscPortARev = m_PortA.m_tPortStatus.usRevolutions ;		
	m_tNvRamDefaults.ucOscPortASec	= (SnByte)m_PortA.m_tPortStatus.wDwell;
	m_tNvRamDefaults.ucOscModePortB = m_PortB.m_tPortStatus.usOscMode;	
	m_tNvRamDefaults.ucOscPortBRev = m_PortB.m_tPortStatus.usRevolutions;		
	m_tNvRamDefaults.ucOscPortBSec = (SnByte)m_PortB.m_tPortStatus.wDwell;
	m_tNvRamDefaults.ucShaverPktCtl = m_usShaverPacketControl;
	m_tNvRamDefaults.ucCustDefaultMode = m_usCustDefaultMode;  // Default/Custom
	m_tNvRamDefaults.ucLanguage = m_usLanguage;
	
	return TRUE;
	
}

// Function:	DeInitHardware
// Purpose: 	Brings down the hardware
//
// Return:		None
//
void CControl::DeInitHardware(SnBool yBuzzer)
{
	// Stop the status thread 
	m_bHardwareReady = FALSE;
	
	// Shaver DeInit
	if( m_hDriver)
	{
		if(m_bCommunicationFailure == TRUE)
		{
			// Force shut down of the motors via Reset
			m_hDriver->ResetDsp();
		}
		else
		{
			// Shut down the motors
			m_hDriver->WriteWordToDevice(MC_BLDCA_MODE, 0);
			m_hDriver->WriteWordToDevice(MC_BLDCB_MODE, 0);
		}
		
		// Wait up to 2 seconds to turn off the system Heartbeat
		SnQByte qTimeout = 0;
		while (qTimeout < 2000 && !m_hDriver->WriteWordToDevice(MC_HEARTBEAT, 0))
		{
			Sleep(100);
			qTimeout += 100;
		}
		
		ResetDisplayBase();
		
		m_bPulseBeeper = FALSE;
		m_hDriver->SetSystemBuzzer(yBuzzer);
	}
}

// Function:	PowerOnSelfTest
// Purpose: 	Performs Power On Self Test
//
// Return:		TRUE = PASS, FALSE = FAIL
//
SnBool CControl::PowerOnSelfTest(void)
{
	SnBool bStatus;
	
	if(!m_hDriver)
		return FALSE;
	
	// Sound the buzzer, the hardware is up and running
	bStatus = m_hDriver->SetSystemBuzzer(TRUE);
	if(!bStatus)
	{
		SetEvent(m_hCommFailureEvent); // Failure condition
		return FALSE;
	}
	
	Sleep(400); 
	
	// Turn the buzzer off
	bStatus = m_hDriver->SetSystemBuzzer(FALSE);
	if(!bStatus)
	{
		SetEvent(m_hCommFailureEvent); // Failure condition
		return FALSE;
	}
	
	// Set the system Heartbeat
#ifdef DEBUG
	SnWord wOutput = 0;
#else
	SnWord wOutput = 2000;
#endif
	bStatus = WriteDsp(MC_HEARTBEAT, 1, &wOutput);
	if(!bStatus)
		return FALSE;
	
	// Set the hardware ready flag
	m_bHardwareReady = TRUE;
	
	
	return TRUE;
}


// Function:	GetWatchDogStatus
// Purpose: 	Monitors Watch Dog Timer status
// Parameters:	None
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool CControl::GetWatchDogStatus()
{
	SnWord usHeartBeat;
	
	// Read the Port Type status
	SnBool bStatus = ReadDsp(MC_HEARTBEAT, 1, &usHeartBeat);
	if( !bStatus)
		return FALSE;
	
	if( usHeartBeat == 15000)
		SetEvent( m_hWatchDogTimerEvent); // Watch Dog Timer fired, set the event
	
	return TRUE;
}

SnBool CControl::GetTemperatures(float *pfOnBoardTemp, float *pfDspTemp)
{
	SnBool bStatus;
	L_F_TYPE ulDspTemp;
	L_F_TYPE ulOnBoardTemp;
	unsigned long ulTemp;
	SnWord pwBuf[2];
	
	// Monitor Dsp and On-Board temps. If temp. error condition clears
	// clear the Dsp temperature event register.
	bStatus = ReadDsp(MC_TEMPERATURE_ONBOARD,2, pwBuf);
	if(!bStatus)
		return FALSE;
	
	ulTemp = pwBuf[1] << 16;	// shift to the left 16 times
	ulTemp = ulTemp | pwBuf[0]; // Or lower word with mask	
	
	ulOnBoardTemp.lType = ulTemp;
	*pfOnBoardTemp = ulOnBoardTemp.fType;
	
	bStatus = ReadDsp(MC_TEMPERATURE_DSP, 2, pwBuf);
	if(!bStatus)
		return FALSE;
	
	ulTemp = pwBuf[1] << 16;	// shift to the left 16 times
	ulTemp = ulTemp | pwBuf[0]; // Or lower word with mask	
	
	ulDspTemp.lType = ulTemp;
	*pfDspTemp = ulDspTemp.fType;
	
	return TRUE;
}

// Function:	GetTemperatureStatus
// Purpose: 	Monitors system temperature and sets appropriate events if over temp.
// Parameters:	None
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool CControl::GetTemperatureStatus()
{
	SnBool bStatus;
	SnWord usOutput;
	float fDspTemp;
	float fOnBoardTemp;
	
	bStatus = ReadDsp(MC_TEMPERATURE_FAULT,1, &usOutput);
	if( !bStatus)
		return FALSE;
	
	if( usOutput > 0)
	{
		if (usOutput & (ERROR_HIGH_TEMP | ERROR_LOW_TEMP | ERROR_DSP_TEMP))
		{
			// m_usTemperature = MAX_TEMPERATURE;	// Fatal Temperature failure
			m_dwTemperature = TEMPERATURE_WARNING ; // Over Temp. Warning
			SetEvent(m_hTemperatureFailureEvent);  // System Over/Under Temp.
			
			bStatus = GetTemperatures(&fOnBoardTemp, &fDspTemp);
			if(!bStatus)
				return FALSE;
			
			if( (fOnBoardTemp < MAX_ONBOARD_TEMP && fOnBoardTemp > MIN_ONBOARD_TEMP) &&
				fDspTemp < MAX_DSP_TEMP)
			{
				SnWord wInput = 0;
				// Clear the error condition
				WriteDsp(MC_TEMPERATURE_FAULT, 1, &wInput);
				
				// Let the GUI know the error went away
				m_dwTemperature = CLEAR_ERROR_CONDITION ; 
				SetEvent(m_hTemperatureFailureEvent); // error went away  
			}		
		}
	}
	return TRUE;
}

SnBool CControl::ErrorCorrectSerialResponse(SnWord *pwResponse)
{
	SnWordFlavors twResponse;
	SnByte bECC;
	int iBit;
	
	twResponse.w = *pwResponse;
	
	// Compute ECC
	
	// Physical bits:	   B15 B14 B13 B12 B11 B10	B9 B8 B7 B6 B5 B4 B3 B2 B1 B0
	// Physical mapping is:  X	E4	E3	E2	E1 D11 D10 D9 D8 D7 D6 D5 D4 D3 D2 D1
	// Logical address is:	  15  14 13 12 11 10  9  8	7  6  5  4	3  2  1
	//						   1   1  1  1	1  1  1  1	0  0  0  0	0  0  0
	//						   1   1  1  1	0  0  0  0	1  1  1  1	0  0  0
	//						   1   1  0  0	1  1  0  0	1  1  0  0	1  1  0
	//						   1   0  1  0	1  0  1  0	1  0  1  0	1  0  1
	// Logical mapping is:	 D11 D10 D9 D8 D7 D6 D5 e8 D4 D3 D2 e4 D1 e2 e1
	// e1 = d1 ^ d2 ^ d4 ^ d5 ^ d7 ^ d9 ^ d11;
	// e2 = d1 ^ d3 ^ d4 ^ d6 ^ d7 ^ d10 ^ d11; 
	// e4 = d2 ^ d3 ^ d4 ^ d8 ^ d9 ^ d10 ^ d11;
	// e8 = d5 ^ d6 ^ d7 ^ d8 ^ d9 ^ d10 ^ d11;
	bECC =
		(twResponse.B0 ^ twResponse.B1 ^ twResponse.B3 ^ twResponse.B4
		^ twResponse.B6 ^ twResponse.B8 ^ twResponse.B10 ^ twResponse.B11) 	  // E1
		|
		((twResponse.B0 ^ twResponse.B2 ^ twResponse.B3 ^ twResponse.B5
		^ twResponse.B6 ^ twResponse.B9 ^ twResponse.B10 ^ twResponse.B12) << 1) // E2
		|
		((twResponse.B1 ^ twResponse.B2 ^ twResponse.B3 ^ twResponse.B7
		^ twResponse.B8 ^ twResponse.B9 ^ twResponse.B10 ^ twResponse.B13) << 2) // E4
		|
		((twResponse.B4 ^ twResponse.B5 ^ twResponse.B6 ^ twResponse.B7
		^ twResponse.B8 ^ twResponse.B9 ^ twResponse.B10 ^ twResponse.B14) << 3) // E8
		;
	
	for (iBit = 0; iBit < 15; iBit++) {
		twResponse.yParity ^= (twResponse.w >> iBit) & 1;
	}
	
	// Perform error detection / correction
	// Parity	ECC 	Action
	// 0		0		No errors
	// 0		1-15	Double error detected
	// 1		0		Parity bit wrong
	// 1		1-15	Single error, corrected
	
	if (!twResponse.yParity && bECC) {
		// Double error detected, fail
		twResponse.yAckNak = FALSE;
	} else if (twResponse.yParity && bECC) {
		// Perform error correction
		switch(bECC) {
		default:	//FallThrough - values other than 0-7 cannot happen
		case 0:
			// No error
			break;
		case 1:
			// Fix E1, ignore
			break;
		case 2:
			// Fix E2, ignore
			break;
		case 3:
			// Fix D1
			twResponse.B0 ^= 1;
			break;
		case 4:
			// Fix E4, ignore
			break;
		case 5:
			// Fix D2
			twResponse.B1 ^= 1;
			break;
		case 6:
			// Fix D3
			twResponse.B2 ^= 1;
			break;
		case 7:
			// Fix D4
			twResponse.B3 ^= 1;
			break;
		case 8:
			// Fix E8, ignore
			break;
		case 9:
			// Fix D5
			twResponse.B4 ^= 1;
			break;
		case 10:
			// Fix D6
			twResponse.B5 ^= 1;
			break;
		case 11:
			// Fix D7
			twResponse.B6 ^= 1;
			break;
		case 12:
			// Fix D8
			twResponse.B7 ^= 1;
			break;
		case 13:
			// Fix D9
			twResponse.B8 ^= 1;
			break;
		case 14:
			// Fix D10
			twResponse.B9 ^= 1;
			break;
		case 15:
			// Fix D11
			twResponse.B10 ^= 1;
			break;
		}
	}
	
	*pwResponse = twResponse.w;
	
	return twResponse.yAckNak;
}

SnBool CControl::SendSerialRequests(SnQByte qNumRequests, SnWord *pwResults, SnWord wTimeout, CCommandDuration & duration)
{
	SnBool result = FALSE;
	
	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER ticksPerMilliSecond;
	LARGE_INTEGER ticksStart;	// A point in time
	LARGE_INTEGER ticksStop;   // A point in time
	LARGE_INTEGER newDuration;
	
	// get the high resolution counter's accuracy
	QueryPerformanceFrequency(&ticksPerSecond);
	ticksPerMilliSecond.QuadPart = ticksPerSecond.QuadPart / 1000;
	
	// what time is it?
	QueryPerformanceCounter(&ticksStart);
	result = SendSerialRequests(qNumRequests, pwResults, wTimeout);
	QueryPerformanceCounter(&ticksStop);
	newDuration.QuadPart = ticksStop.QuadPart - ticksStart.QuadPart;
	newDuration.QuadPart = (newDuration.QuadPart * 1000) / ticksPerMilliSecond.QuadPart;
	if (newDuration.QuadPart > 250)
		newDuration.QuadPart -= 250;	// Remove DII overhead
	else
		newDuration.QuadPart =0;
	newDuration.QuadPart /= qNumRequests;
	if (newDuration.QuadPart > MAXDWORD)
		newDuration.QuadPart = MAXDWORD;
	duration.NewEntry((unsigned long) newDuration.QuadPart);
	
	return result;
}

SnBool CControl::SendSerialRequests(SnQByte qNumRequests, SnWord *pwResults, SnWord wTimeout)
{
	SnQByte qCmdListBytes = qNumRequests * sizeof(SnWord);
	SnWord pwCmdBackup[20]; 	// Array size must be greater than 15 (max requests per spec)
	SnQByte qRetrys = 0;
	SnBool yStatus;
#if RS485_TRACK_ERRORS
	SnWord wPort = (*pwResults & 0x8000) ? 1 : 0;
#endif
	
	memcpy(pwCmdBackup, pwResults, qCmdListBytes);
	do {
		SnQByte qCnt;
		
		yStatus = m_hDriver->SerialCmdsToDevice(MC_SERIAL_CMD_RESULTS, qNumRequests, pwResults, wTimeout);
		
		if (yStatus) {
			for (qCnt = 0; qCnt < qNumRequests; qCnt++) {
				if (ErrorCorrectSerialResponse(&pwResults[qCnt]) == FALSE)
					yStatus = FALSE;
			}
		}
		if (!yStatus) {
			memcpy(pwResults, pwCmdBackup, qCmdListBytes);
		}
		
	} while (!yStatus && ++qRetrys < 1);
	
#if RS485_TRACK_ERRORS
	if (wPort) {
		m_qSerial1Requests += qRetrys + 1;
		m_qSerial1Retrys += qRetrys;
		if (qRetrys >= 3)
			m_qSerial1Errs++;
	} else {
		m_qSerial0Requests += qRetrys + 1;
		m_qSerial0Retrys += qRetrys;
		if (qRetrys >= 3)
			m_qSerial0Errs++;
	}
#endif
	
	return yStatus;
}

SnBool CControl::SendSerialRequest(SnWord *pwResults, SnWord wTimeout, CCommandDuration &  duration)
{
	SnBool result = FALSE;
	
	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER ticksPerMilliSecond;
	LARGE_INTEGER ticksStart;	// A point in time
	LARGE_INTEGER ticksStop;   // A point in time
	LARGE_INTEGER newDuration;
	
	// get the high resolution counter's accuracy
	QueryPerformanceFrequency(&ticksPerSecond);
	ticksPerMilliSecond.QuadPart = ticksPerSecond.QuadPart / 1000;
	
	// what time is it?
	QueryPerformanceCounter(&ticksStart);
	result = SendSerialRequest(pwResults, wTimeout);
	
	QueryPerformanceCounter(&ticksStop);
	newDuration.QuadPart = ticksStop.QuadPart - ticksStart.QuadPart;
	newDuration.QuadPart = (newDuration.QuadPart * 1000) / ticksPerMilliSecond.QuadPart;
	if (newDuration.QuadPart > 250)
		newDuration.QuadPart -= 250;	// Remove DII overhead
	else
		newDuration.QuadPart =0;
	if (newDuration.QuadPart > MAXDWORD)
		newDuration.QuadPart = MAXDWORD;
	duration.NewEntry((unsigned long) newDuration.QuadPart);
	
	return result;
}

SnBool CControl::SendSerialRequest(SnWord *pwResults, SnWord wTimeout)
{
	SnBool yStatus;
	
#if RS485_TRACK_ERRORS
	SnQByte qRetrys = 0;
	SnWord wPort = (*pwResults & 0x8000) ? 1 : 0;
#endif
	
	yStatus = m_hDriver->SerialCmdsToDevice(MC_SERIAL_CMD_RESULTS, 1, pwResults, wTimeout);
	
	if (yStatus && ErrorCorrectSerialResponse(pwResults) == FALSE)
		yStatus = FALSE;
	
#if RS485_TRACK_ERRORS
	if (wPort) {
		m_qSerial1Requests += qRetrys + 1;
		m_qSerial1Retrys += qRetrys;
		if (qRetrys >= 3)
			m_qSerial1Errs++;
	} else {
		m_qSerial0Requests += qRetrys + 1;
		m_qSerial0Retrys += qRetrys;
		if (qRetrys >= 3)
			m_qSerial0Errs++;
	}
#endif
	
	return yStatus;
}

SnBool CControl::FootPedalStatusUpdate( SnWord usPort, SnBool bOverride, SnWord wPercent, SnWord wMode )
{
	if(usPort == PORTA)
	{
		m_PortA.FootPedalStatusUpdate( bOverride, wPercent, wMode);
	}
	else if(usPort == PORTB)
	{
		m_PortB.FootPedalStatusUpdate(bOverride, wPercent, wMode);
	}
	else
		return TRUE;
	
	return FALSE;
}

SnBool CControl::GetFootMode( SnWord usPort, SnWord & usFootMode)
{
	if( usPort == PORTA)
		usFootMode = m_PortA.m_tPortStatus.usFootMode;
	else if( usPort == PORTB)
		usFootMode = m_PortB.m_tPortStatus.usFootMode;
	else
		return FALSE;
	return TRUE;
}

SnBool CControl::SetFootMode( SnWord usPort, SnWord usFootMode)
{
	if( usPort == PORTA)
		m_PortA.m_tPortStatus.usFootMode = usFootMode;
	else if( usPort == PORTB)
		m_PortB.m_tPortStatus.usFootMode = usFootMode;
	else
		return FALSE;
	return TRUE;
}

SnBool CControl::IsPoweredInstrument( SnWord usPort)
{
	if( usPort == PORTA)
		return IS_TYPE_POWER_INSTR(m_PortA.m_tPortStatus.usType);
	else if( usPort == PORTB)
		return IS_TYPE_POWER_INSTR(m_PortB.m_tPortStatus.usType);
	else
		return FALSE;
}


void CControl::NotifyPump(SnByte bPumpCmd)
{
	if (bPumpCmd != m_tOutGoingShaverPacket.ucCmd)
	{
		m_tOutGoingShaverPacket.ucCmd = bPumpCmd;
		m_bShaverPacketDirty = TRUE;
	}
}


// Function:	UpdatePortStatus
// Purpose: 	Informs Gui and remote connections of a change in port status
//
// Parameters:	None
//
// Return:		TRUE indicates Success, FALSE indicates Failure
//
void CControl::UpdatePortStatus()
{
	
	if( m_usShaverPacketControl != m_usPrevShaverPacketControl)
	{
		
		// Shaver packet routing has changed
		if( m_RemotePumpConnectionType != CPump::PUMP_TYPE_UNKNOWN)
			PostMessage(HWND_BROADCAST, WM_UPDATE_REMOTE_STATUS, (WPARAM) MSG_UPDATE_REMOTE_PUMP_STATUS,
			(LPARAM) MSG_PORT_MAPPING_STATUS);
		
		m_usPrevShaverPacketControl = m_usShaverPacketControl;
		
		// Update Shaver Packet
		InitShaverPacket();
		
	}
	
}


// Function:	SetOscProfile
// Purpose: 	Writes the Oscillate mode to the Motor Control board 
//
// Parameters:	None
//
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool CControl::SetOscProfile( DWORD dwProfile , DWORD dwPort)
{
	
	SnWord usBusy;
	SnQByte qNumWords;
	SnQByte qAddressProfileCmd;
	SnQByte qAddressProfile;
	SnQByte qAddressFault;
	SnWord* pwData;
	unsigned int i;
	L_F_TYPE x, y;
	SnWord HighWord;
	SnWord LowWord;
	MotorProfile tProfile;
	
	if( dwPort == PORTA)
	{
		// Offset Addresses for Port A
		qAddressProfileCmd = MC_BLDCA_PROFILE_CMD;
		qAddressProfile = MC_BUFFER;
		qAddressFault = MC_BLDCA_FAULT;
	}
	else if( dwPort == PORTB)
	{
		// Offset Addresses for Port B
		qAddressProfileCmd = MC_BLDCB_PROFILE_CMD;
		qAddressProfile = MC_BUFFER;
		qAddressFault = MC_BLDCB_FAULT;
	}
	else
		return FALSE;
	
	if( dwProfile == PROFILE_1)
		tProfile = g_tOscillateMode1;
	else if( dwProfile == PROFILE_2)
		tProfile = g_tOscillateMode2;
	else 
		return FALSE;
	
	// Make sure the busy flag is clear
	ReadDsp(qAddressProfileCmd, 1, &usBusy);
	if( (usBusy & PCON_RX_DATA_READY) != 0x0000)
	{
		// Wait and try again
		Sleep(5);
		
		ReadDsp(qAddressProfileCmd, 1, &usBusy);
		if( (usBusy & PCON_RX_DATA_READY) != 0x0000)
			return FALSE; // still busy
	}
	
	SnWord buf[sizeof(MotorProfile)/(sizeof(SnWord))];
	
	pwData = buf;
	
	memset(buf, 0, sizeof(buf));
	
	qNumWords = 0;
	pwData[qNumWords++] = tProfile.Count;
	pwData[qNumWords++] = tProfile.Fill;
	   
	for( i=0; i <= tProfile.Count; i++)
	{	
		x.fType = tProfile.P[i].X;
		LowWord = (SnWord)(x.lType);
		HighWord = (SnWord)(x.lType >> 16); 
		
		pwData[qNumWords++] = LowWord;
		pwData[qNumWords++] = HighWord;
		
		y.fType = tProfile.P[i].Y;
		LowWord = (SnWord)(y.lType);	
		HighWord = (SnWord)(y.lType >> 16); 
		
		pwData[qNumWords++] = LowWord;
		pwData[qNumWords++] = HighWord;
	}
	
	pwData = buf; // set the pointer back to the begining
	
	// Set Oscillate mode 
	if( qNumWords <= PROFILE_BUFFER_SIZE)
	{
		WriteDsp( qAddressProfile, qNumWords, pwData);
		
		// Set the busy flag to indicate a mode change
		ReadDsp(qAddressProfileCmd, 1, &usBusy); // Read/Modify/Write
		usBusy = usBusy | PCON_RX_DATA_READY;
		WriteDsp( qAddressProfileCmd, 1, &usBusy);	
	}
	else
	{
		SnQByte qTempWords;
		SnQByte qCurrentWords, count;
		
		qTempWords = PROFILE_BUFFER_SIZE;
		count = 0;
		qCurrentWords = PROFILE_BUFFER_SIZE;
		
		while( qTempWords != 0)
		{
			WriteDsp( qAddressProfile, qTempWords, pwData);
			
			// Set the busy flag to indicate a mode change
			ReadDsp( qAddressProfileCmd, 1, &usBusy); // Read/Modify/Write
			usBusy = usBusy | PCON_RX_DATA_READY;
			WriteDsp( qAddressProfileCmd, 1, &usBusy);
			
			pwData = pwData + qTempWords;
			
			count = count + qTempWords;
			qCurrentWords = qNumWords - count;
			
			if(qCurrentWords < PROFILE_BUFFER_SIZE)
			{
				qTempWords = qCurrentWords;
			}
			else 
			{
				qTempWords = PROFILE_BUFFER_SIZE;
			}
			
			if( count <= 0)
				qTempWords = 0;
			
			// Make sure the busy flag is clear
			ReadDsp( qAddressProfileCmd, 1, &usBusy);
			if( (usBusy & PCON_RX_DATA_READY) != 0x0000)
			{		
				// Wait and try again
				Sleep(5);
				
				ReadDsp( qAddressProfileCmd, 1, &usBusy);
				if( (usBusy & PCON_RX_DATA_READY) != 0x0000)
					return FALSE; // still busy
			}
		}
	}  
	return TRUE;
}

SnBool CControl::InitShaverPacket(void)
{
	
	// Block
	WaitForSingleObject(m_hMutex, INFINITE);
	
	SN_PORT_STATUS* tPortStatus;
	
	if( m_usShaverPacketControl == PORTA)
		tPortStatus = &m_PortA.m_tPortStatus;
	else
		tPortStatus = &m_PortB.m_tPortStatus;
	
	
	// Initialize the Shaver Packet
	if( tPortStatus)
	{
		
		
		if( !IS_TYPE_MDU(tPortStatus->usType) )
		{
			m_tOutGoingShaverPacket.ucBladeId = BLADE_ID_OTHER; // other blade family;
			m_tOutGoingShaverPacket.ucSpeedHigh = 0x00;
			m_tOutGoingShaverPacket.ucSpeedLow =  0x00;
			m_tOutGoingShaverPacket.ucOpState = 0x00;		
			
			m_bShaverPacketDirty = TRUE;
		}
		else
		{
			
			SetShaverBladeId(m_usShaverPacketControl, tPortStatus->usShaverBladeId);
			SetShaverSpeed(m_usShaverPacketControl, tPortStatus->usHandMode, tPortStatus->wPeriod, tPortStatus->usVelocity);
			
			//
			// Init Operational state
			//
			SetShaverOpState(m_usShaverPacketControl,tPortStatus->usHandMode);
		}
	}
	
	ReleaseMutex( m_hMutex);// unblock
	
	return TRUE;
	
}

// Function:	GetRemotePumpStatus
// Purpose: 	Checks for remote pump connection
//
// Parameters:	None.
//
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool CControl::GetRemotePumpStatus()
{
	CPump::PumpType ePumpType;
	SnBool bRunning = FALSE;
	
	if( m_pPumpConnection)
	{
		// Check for remote pump connection
		bRunning = m_pPumpConnection->GetPumpStatus( &ePumpType);
		
		if(ePumpType != m_RemotePumpConnectionType)
		{
			
			if(ePumpType == CPump::PUMP_TYPE_UNKNOWN) 
			{
				// Pump connection not detected
				PostMessage(HWND_BROADCAST, WM_UPDATE_REMOTE_STATUS, 
					(WPARAM) MSG_UPDATE_REMOTE_PUMP_STATUS,
					(LPARAM) MSG_REMOTE_DISCONNECTED);
			}
			else if( ePumpType == CPump::PUMP_TYPE_FMS || ePumpType == CPump::PUMP_TYPE_DYONICS25)
			{
				// Pump connection detected
				PostMessage(HWND_BROADCAST, WM_UPDATE_REMOTE_STATUS, 
					(WPARAM) MSG_UPDATE_REMOTE_PUMP_STATUS,
					(LPARAM) MSG_REMOTE_CONNECTED);
				
				InitShaverPacket();		
			}
			
		}
		
		if( ePumpType == CPump::PUMP_TYPE_DYONICS25)
		{
			// New pump check impeller status
			if( bRunning != m_RemotePumpRunning)
			{
				if(bRunning)
				{
					// Remote pump is running
					PostMessage(HWND_BROADCAST, WM_UPDATE_REMOTE_STATUS, 
						(WPARAM) MSG_UPDATE_REMOTE_PUMP_STATUS,
						(LPARAM) MSG_PUMP_RUNNING);
					
				}
				else
				{
					// Remote pump is not running
					PostMessage(HWND_BROADCAST, WM_UPDATE_REMOTE_STATUS, 
						(WPARAM) MSG_UPDATE_REMOTE_PUMP_STATUS,
						(LPARAM) MSG_PUMP_NOT_RUNNING);
				}
				
			}
		}
		
		m_RemotePumpRunning = bRunning; // Save status
		m_RemotePumpConnectionType = ePumpType; // Save status
	}
	
	
	return TRUE;
	
}
// Function:	XmtShaverPacket
// Purpose:		Transmits the current out going shaver packet
//
// Parameters:	None.
//
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool CControl::XmtShaverPacket()
{
	SnBool bStatus = FALSE;
	
	
	// If a Drill or Saw is connected set packet info. to zeros. Not used by pump
	if((m_usShaverPacketControl == PORTA && !IS_TYPE_MDU(m_PortA.m_tPortStatus.usType)) ||
		(m_usShaverPacketControl == PORTB && !IS_TYPE_MDU(m_PortB.m_tPortStatus.usType)))
	{
		// set packet info. to zero otherwise the pump will use data for suction control
		m_tOutGoingShaverPacket.ucBladeId = BLADE_ID_OTHER;
		m_tOutGoingShaverPacket.ucOpState = 0;
		m_tOutGoingShaverPacket.ucSpeedHigh = 0;
		m_tOutGoingShaverPacket.ucSpeedLow = 0;
		
	}
	
	
	// Assemble packet
	m_pucPumpData[0] = m_tOutGoingShaverPacket.ucSpeedLow;
	m_pucPumpData[1] = m_tOutGoingShaverPacket.ucSpeedHigh;
	m_pucPumpData[2] = m_tOutGoingShaverPacket.ucOpState;
	m_pucPumpData[3] = m_tOutGoingShaverPacket.ucBladeId;
	m_pucPumpData[4] = m_tOutGoingShaverPacket.ucCmd;
	
	//Transmit data
	if( m_pPumpConnection)
		bStatus = m_pPumpConnection->XmtPacket( m_pucPumpData);
	
	
	return bStatus;
	
}

// Function:	KillThreads
// Purpose:		Terminates any running threads
//
// Parameters:	None
//
// Return:		TRUE indicates Success, FALSE indicates Failure
//
void CControl::KillThreads()
{
	DWORD waitStatus;
	DWORD exitCode;
	SnBool bStatus;
	
    if (!m_bKillThreads)
    {
		// Terminate any running threads
		m_bKillThreads = TRUE;          // Kills polling threads
		SetEvent(m_hKillThreadEvent);   // Kills Event Handler Thread
		
		if( m_bStatusThreadKilled == FALSE )
		{
			// wait for the thread to terminate, time out after a certain period of time
			if( m_hStatusThreadKilledEvent)
				waitStatus = WaitForSingleObject( m_hStatusThreadKilledEvent,CTL_THREAD_TERMINATION_WAIT );
			
			if( waitStatus == WAIT_TIMEOUT) 
			{
				bStatus = GetExitCodeThread( m_hStatusThread, &exitCode);
				bStatus = TerminateThread( m_hStatusThread, exitCode);
			}
			
			m_bStatusThreadKilled = TRUE;
		}
		
		if( m_bStatusThread2Killed == FALSE )
		{
			// wait for the thread to terminate, time out after a certain period of time
			if( m_hStatusThread2KilledEvent)
				waitStatus = WaitForSingleObject( m_hStatusThread2KilledEvent,CTL_THREAD_TERMINATION_WAIT );
			
			if( waitStatus == WAIT_TIMEOUT) 
			{
				bStatus = GetExitCodeThread( m_hStatusThread2, &exitCode);
				bStatus = TerminateThread( m_hStatusThread2, exitCode);
			}
			
			m_bStatusThread2Killed = TRUE;
		}
		
		if( m_bStorageThreadKilled == FALSE )
		{
			// wait for the thread to terminate, time out after a certain period of time
			if( m_hStorageThreadKilledEvent)
				waitStatus = WaitForSingleObject( m_hStorageThreadKilledEvent,CTL_THREAD_TERMINATION_WAIT );
			
			if( waitStatus == WAIT_TIMEOUT) 
			{
				bStatus = GetExitCodeThread( m_hPersistentStorageThread, &exitCode);
				bStatus = TerminateThread( m_hPersistentStorageThread, exitCode);
			}
			
			m_bStorageThreadKilled = TRUE;
		}
		
		if( m_bEventHandlerThreadKilled == FALSE )
		{
			// wait for the thread to terminate, time out after a certain period of time
			if( m_hHandlerThreadKilledEvent)
				waitStatus = WaitForSingleObject( m_hHandlerThreadKilledEvent,CTL_THREAD_TERMINATION_WAIT );
			
			if( waitStatus == WAIT_TIMEOUT) 
			{
				bStatus = GetExitCodeThread( m_hEventHandlerThread, &exitCode);
				bStatus = TerminateThread( m_hEventHandlerThread, exitCode);
			}
			
			m_bEventHandlerThreadKilled = TRUE;
		}
		
		if( m_bRemoteStatusThreadKilled == FALSE )
		{
			// wait for the thread to terminate, time out after a certain period of time
			if( m_hRemoteStatusThreadKilledEvent)
				waitStatus = WaitForSingleObject( m_hRemoteStatusThreadKilledEvent,CTL_THREAD_TERMINATION_WAIT );
			
			if( waitStatus == WAIT_TIMEOUT) 
			{
				bStatus = GetExitCodeThread( m_hRemoteStatusThread, &exitCode);
				bStatus = TerminateThread( m_hRemoteStatusThread, exitCode);
			}
			
			m_bRemoteStatusThreadKilled = TRUE;
		}
    }
}

// Function:	StartTimer
// Purpose:		Starts Timers 1 or 2
//
// Parameters:	None
//
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool CControl::StartTimer(DWORD dwTimerID)
{
	
	if( dwTimerID == TAC_TIMER_MOTORA)
	{
		if( m_hTacTimerThreadMotorA) // The timer thread is already running
			return FALSE;
		
		// Start the timer thread	
		m_hTacTimerThreadMotorA = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
			0,
			TacTimerThreadMotorA,
			this,
			0,
			&m_hTacTimerThreadMotorAID);
		
		
		if(!m_hTacTimerThreadMotorA)
		{
			SetEvent( m_hSystemResourceFailureEvent); // system cannot allocate resources
			return FALSE; // Timer could not be started
		}
		DEBUGMSG(TRUE, (TEXT("Control m_hTacTimerThreadMotorA: 0x%08X\n"),m_hTacTimerThreadMotorAID));
	}
	else if( dwTimerID == TAC_TIMER_MOTORB)
	{
		if( m_hTacTimerThreadMotorB) // The timer thread is already running
			return FALSE;
		
		// Start the timer thread	
		m_hTacTimerThreadMotorB = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
			0,
			TacTimerThreadMotorB,
			this,
			0,
			&m_hTacTimerThreadMotorBID);
		
		
		if(!m_hTacTimerThreadMotorB)
		{
			SetEvent( m_hSystemResourceFailureEvent); // system cannot allocate resources
			return FALSE; // Timer could not be started
		}
		DEBUGMSG(TRUE, (TEXT("Control TacTimerThreadMotorB: 0x%08X\n"),m_hTacTimerThreadMotorBID));
	}
	else
		return FALSE;
	
	return TRUE;
}

SnBool CControl::PulseBeeper(SnBool bBeeperState)
{
	SnBool bStatus;
	
	bStatus = TRUE;
	
	if(m_hDriver)
	{
		bStatus = m_hDriver->SetSystemBuzzer(bBeeperState == TRUE);
	}	
	else
		bStatus = FALSE;
	
	return bStatus;
	
}

// Function:	StopTimer
// Purpose:		Stops timers 1 or 2
//
// Parameters:	None
//
// Return:		TRUE indicates Success, FALSE indicates Failure
//
SnBool CControl::StopTimer( DWORD dwTimerID)
{
	
	if( dwTimerID == TAC_TIMER_MOTORA)
	{
		// If the timer thread is running stop it
		if( m_hTacTimerThreadMotorA)
		{
			// Set the KillTimer event
			if( m_PortA.m_tPortEvents.m_hMotorKillTacTimerEvent)
				SetEvent( m_PortA.m_tPortEvents.m_hMotorKillTacTimerEvent);
		}
	}
	else if( dwTimerID == TAC_TIMER_MOTORB)
	{
		// If the timer thread is running stop it
		if( m_hTacTimerThreadMotorB)
		{
			// Set the KillTimer event
			if( m_PortB.m_tPortEvents.m_hMotorKillTacTimerEvent)
				SetEvent(m_PortB.m_tPortEvents. m_hMotorKillTacTimerEvent);
		}
	}
	else 
		return FALSE;
	
	return TRUE;
}

// Function:	TacTimerThreadMotorA
// Purpose:		Thread used for timed events
// Parameters;	None
//
// Return:		None
DWORD WINAPI TacTimerThreadMotorA(LPVOID pParam)
{
	CControl *pClass = (CControl*)pParam;
	DWORD	 dwStatus;
	
	dwStatus = WaitForSingleObject( pClass->m_PortA.m_tPortEvents.m_hMotorKillTacTimerEvent,1000 );
	if( dwStatus == WAIT_TIMEOUT)
	{
		// Timer expired set the flag
		pClass->m_PortA.SetMotorTacTimerExpired(TRUE);
	}
	
	if( pClass->m_hTacTimerThreadMotorA)
	{
		CloseHandle(pClass->m_hTacTimerThreadMotorA);
		pClass->m_hTacTimerThreadMotorA = NULL;
	}
	
	return 0;
}

// Function:	TacTimerThreadMotorB
// Purpose:		Thread used for timed events
// Parameters;	None
//
// Return:		None
DWORD WINAPI TacTimerThreadMotorB(LPVOID pParam)
{
	CControl *pClass = (CControl*)pParam;
	DWORD	 dwStatus;
	
	dwStatus = WaitForSingleObject( pClass->m_PortB.m_tPortEvents.m_hMotorKillTacTimerEvent,1000 );
	if( dwStatus == WAIT_TIMEOUT)
	{
		// Timer expired set the flag
		pClass->m_PortB.SetMotorTacTimerExpired(TRUE);
	}
	
	if( pClass->m_hTacTimerThreadMotorB)
	{
		CloseHandle(pClass->m_hTacTimerThreadMotorB);
		pClass->m_hTacTimerThreadMotorB = NULL;
	}
	
	return 0;
}

void AdjustedSleep(SnQByte qTimerStart, SnQByte qSleepPeriod, SnQByte qMaxAdjustment) 
{
	SnQByte	qTimerDelta = 0;
	SnQByte	qTimerEnd =  GetTickCount();		// Get the "current time"
	if (qTimerEnd >= qTimerStart)
	{
		// Calculate the delta time
		qTimerDelta = qTimerEnd - qTimerStart;
	}
	else
	{
		// Timer has wrapped so calculate the time as the amount of time before the
		// wrap + the time after the counter wrapped.
		qTimerDelta = (~((SnQByte)0) - qTimerStart) + qTimerEnd + 1;
	}
	// Adjust the sleep period down by the amount of time taken to execute the code.
	qSleepPeriod -= ( qTimerDelta < qMaxAdjustment)? qTimerDelta: qMaxAdjustment;
	Sleep (qSleepPeriod);
}

// Function:	StatusThread
// Purpose:		Thread used for acquiring status from the Motor Control Board
// Parameters;	None
//
// Return:		None
DWORD WINAPI StatusThread(LPVOID pParam)
{
	CControl *pClass = (CControl*)pParam;
	SnWord wMode7CountPortA = 0;
	SnWord wMode7CountPortB = 0;
	
	pClass->m_bStatusThreadKilled = FALSE; // Reset the ThreadKilled flag
	
	while( !pClass->m_bKillThreads)
	{	
		// Make sure the Hardware Ready flag is true before we begin data collection.
		// This indicates the Hardware has been Initialized and configured properly
		if( pClass->m_bHardwareReady)
		{
			SnQByte	qTimerStart = GetTickCount();
			SnWord	wSleepPeriod;
			SnWord	wMaxAdjustment;
			SnWord  wDelayPeriodAdjustment = 0;
			SnWord	wLoopCount = 0;
			
			
			while(wLoopCount < SLEEP_PERIOD_100 && !pClass->m_bKillThreads)
			{
				// if Handpiece controls are enabled then
				//   perform data collection at timed intervals of
				//   10ms if a RS485 footswitch is NOT connected or 
				//                              is connected and running
				//   20ms if a RS485 footswitch is connected and idle
				
				// otherwise execute the code once every 100ms
				if (pClass->m_Footswitch.GetFastFootswitch())
				{
					wSleepPeriod   = SLEEP_PERIOD_9;
					wMaxAdjustment = SLEEP_PERIOD_9;
					wLoopCount    += SLEEP_PERIOD_9 + 1;
					// Adjust delay timer periods to accomodate any skipped updates
					wDelayPeriodAdjustment += SLEEP_PERIOD_9 + 1;
				}
				else
				{
					wSleepPeriod   = SLEEP_PERIOD_16;
					wMaxAdjustment = SLEEP_PERIOD_16;
					wLoopCount    += SLEEP_PERIOD_16 + 1;
					// Adjust delay timer periods to accomodate any skipped updates
					wDelayPeriodAdjustment += SLEEP_PERIOD_16 + 1;
				}
				
				if( CPort::GetControlsEnabled() || wLoopCount >= SLEEP_PERIOD_100 )
				{
					pClass->SetDelayPeriodMs(wDelayPeriodAdjustment);
					wDelayPeriodAdjustment = 0;
					
					pClass->m_Footswitch.ReadFootPedalStatus();
					pClass->m_Footswitch.UpdatePortStatus();
					pClass->UpdatePortStatus();
					pClass->m_PortA.ReadPortStatus();
					pClass->m_PortA.UpdatePortStatus();
					pClass->m_PortB.ReadPortStatus();
					pClass->m_PortB.UpdatePortStatus();
				}
				
				// The following code assumes an ~100ms interval between calls
				if(wLoopCount >= SLEEP_PERIOD_100)
				{
					// Monitor the motor status
					pClass->m_PortA.GetMotorStatus();
					
					// The following disables Mode 7 after a period of ~2 Seconds
					if( pClass->m_PortA.GetMode7())
					{
						wMode7CountPortA++;
						if( wMode7CountPortA > 20)
						{
							// Disable Mode 7
							if( pClass->m_PortA.DisableMode7())
								wMode7CountPortA = 0;
						}
					}
					else
						wMode7CountPortA = 0;
					
					// Monitor system temp
					pClass->GetTemperatureStatus();
				}
				if(wLoopCount >= SLEEP_PERIOD_100)
				{
					// Monitor the motor status
					pClass->m_PortB.GetMotorStatus();
					
					// The following disables Mode 7 after a period of ~2 Seconds
					if( pClass->m_PortB.GetMode7())
					{
						wMode7CountPortB++;
						if( wMode7CountPortB > 20)
						{
							if( pClass->m_PortB.DisableMode7())
								wMode7CountPortB = 0;
						}
					}
					else
						wMode7CountPortB = 0;
					
					// Monitor Watch Dog Timer status
					pClass->GetWatchDogStatus();					
				}
				//
				// End of 100ms based code
				AdjustedSleep(qTimerStart, wSleepPeriod, wMaxAdjustment);
				qTimerStart = GetTickCount();
			}
		}
		else
		{
			Sleep(SLEEP_PERIOD_100); 
		}
	}
	
	// Set the Thread Killed Event
	SetEvent( pClass->m_hStatusThreadKilledEvent);
	
	return 0;
}

// Function:	StatusThread2
// Purpose:		Thread used for acquiring status from the Motor Control Board
// Parameters;	None
//
// Return:		None
DWORD WINAPI StatusThread2(LPVOID pParam)
{
	CControl *pClass = (CControl*)pParam;
	SnWord wCount = 0;
	SnWord wBeepCount = 0;
	
	pClass->m_bStatusThread2Killed = FALSE; // Reset the ThreadKilled flag
	
	while( !pClass->m_bKillThreads)
	{
		
		if( pClass->m_bPulseBeeper || pClass->m_bBeepThreeTimes)
		{
			if( wCount == 0)
				pClass->PulseBeeper(TRUE); // Turn the Beeper on
			
			if( wCount == 5)
				pClass->PulseBeeper(FALSE); // Turn the Beeper off
			
			wCount++;
			
			if( wCount > 9)
				wCount = 0; // reset the counter
			
			if( pClass->m_bBeepThreeTimes)
			{
				wBeepCount ++;
				
				if(wBeepCount == 30)
				{
					pClass->m_bBeepThreeTimes = FALSE;
					wBeepCount = 0;
					wCount = 0;
				}
			}
		}
		else
			wCount = 0;
		
		Sleep(100); 
	}
	
	// Set the Thread Killed Event
	SetEvent( pClass->m_hStatusThread2KilledEvent);
	
	return 0;
}

// Function:	StorageThread
// Purpose:		Thread used for acquiring status from the Motor Control Board
// Parameters;	None
//
// Return:		None
DWORD WINAPI StorageThread(LPVOID pParam)
{
	CControl *pClass = (CControl*)pParam;
	SnWord wCount = 0;
	SnWord wBeepCount = 0;
	SnByte ucCount = 0;
	SnBool bStatus = FALSE;
	
#if RS485_TRACK_ERRORS
	SnQByte qSerial0Errs = 0;
	SnQByte qSerial1Errs = 0;
	SnQByte qSerial0Retrys = 0;
	SnQByte qSerial1Retrys = 0;
#endif
	
	pClass->m_bStorageThreadKilled = FALSE; // Reset the ThreadKilled flag
	
	//
	// Give the system a chance to sense all Handpieces before attempting to save
	// data. Wait a total of 5 seconds (4 + the normal 1 second interval).
	//
	for (wCount = 0; wCount < 40; wCount++)
	{
		if (pClass->m_bKillThreads)
			break;
		Sleep(100);
	}
	
	while( !pClass->m_bKillThreads)
	{
		
		// Make sure the Hardware Ready flag is true before we begin data collection.
		// This indicates the Hardware has been Initialized and configured properly
		if( pClass->m_bHardwareReady)
		{
#if RS485_TRACK_ERRORS
			if ((qSerial0Errs != pClass->m_qSerial0Errs) || (qSerial1Errs != pClass->m_qSerial1Errs) ||
				(qSerial0Retrys != pClass->m_qSerial0Retrys) || (qSerial1Retrys != pClass->m_qSerial1Retrys))
			{
				TCHAR pwMsg[1024];
				
				swprintf(pwMsg, TEXT("Cmd0: %d   Cmd1: %d\nRetry0: %d   Retry1: %d\nErr0: %d   Err1: %d\n"),
					pClass->m_qSerial0Requests, pClass->m_qSerial1Requests,
					pClass->m_qSerial0Retrys, pClass->m_qSerial1Retrys,
					pClass->m_qSerial0Errs, pClass->m_qSerial1Errs);
				
				qSerial0Retrys = pClass->m_qSerial0Retrys;
				qSerial1Retrys = pClass->m_qSerial1Retrys;
				qSerial0Errs = pClass->m_qSerial0Errs;
				qSerial1Errs = pClass->m_qSerial1Errs;
				
				MessageBox(NULL, pwMsg, TEXT("Serial Requests"), MB_OK|MB_TOPMOST);
			}
#endif
			if( ucCount >= 10)
			{
				// Periodically check to see if user parameters need to be saved
				if( (pClass->m_PortA.GetSaveIfDirty() || pClass->m_PortB.GetSaveIfDirty()) && 
					pClass->m_usCustDefaultMode == CUSTOM_MODE )
				{
					if(TryEnterCriticalSection(&pClass->m_csWriteFlashAccess))
					{
						bStatus = pClass->SaveFlashData();
						if ( !bStatus && !pClass->m_bFlashSaveError )
						{
							// Only post the failure once
							PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)FLASH_SAVE_ERROR, (LPARAM)0);
							
							pClass->m_bFlashSaveError = TRUE;
						}
						else if ( bStatus && pClass->m_bFlashSaveError )
						{
							pClass->m_bFlashSaveError = FALSE;
						}
						LeaveCriticalSection(&pClass->m_csWriteFlashAccess);
					}
				}
				
				ucCount = 0;
			}
			
			ucCount++;
		}
		
		// perform data collection at timed intervals
		Sleep(100); 
	}
	
	// Set the Thread Killed Event
	SetEvent( pClass->m_hStorageThreadKilledEvent);
	
	return 0;
}

// Function:	RemoteStatusThread
// Purpose:		Thread used for acquiring status from the Motor Control Board
// Parameters;	None
//
// Return:		None
DWORD WINAPI RemoteStatusThread(LPVOID pParam)
{
	CControl *pClass = (CControl*)pParam;
	
	pClass->m_bRemoteStatusThreadKilled = FALSE; // Reset the ThreadKilled flag
	
	while( !pClass->m_bKillThreads && !pClass->m_bFactoryMode)
	{
		
		// Make sure the Hardware Ready flag is true before we begin data collection.
		// This indicates the Hardware has been Initialized and configured properly
		if( pClass->m_bHardwareReady)
		{
			// Get remote status
			pClass->GetRemotePumpStatus();
			
			if( pClass->m_bShaverPacketDirty)
			{
				// Clear the flag first before sending the message
				// If the message cannot be put on the queue restore the flag for
				// next time
				pClass->m_bShaverPacketDirty = FALSE;
				if (!pClass->XmtShaverPacket())
					pClass->m_bShaverPacketDirty = TRUE;
				// Just sent a message to the pump add an extra delay so that 
				// the communications to the pump does not develop a backlog of messages.
				Sleep(50); 
				
			}
		}
		
		// perform data collection at time intervals
		Sleep(50); 
	}
	
	if(pClass->m_bFactoryMode && pClass->m_pPumpConnection)
	{
		pClass->m_pPumpConnection->Close();
		delete pClass->m_pPumpConnection;
		pClass->m_pPumpConnection = NULL;
	}
	
	// Set the Thread Killed Event
	SetEvent( pClass->m_hRemoteStatusThreadKilledEvent);
	
	return 0;
}

// Function:	EventHandlerThread
// Purpose:		Thread used for Servicing Events
// Parameters;	None
//
// Return:		None
DWORD WINAPI EventHandlerThread(LPVOID pParam)
{
	CControl *pClass = (CControl*)pParam;
	SnBool bDone = FALSE;
	DWORD dwObj, dwStatus;
	
	pClass->m_bEventHandlerThreadKilled = FALSE; // Reset the ThreadKilled flag
	
	const DWORD  dwEventCount = 34;
	HANDLE hEvents[dwEventCount] =	{ pClass->m_hCommFailureEvent,
		pClass->m_hTemperatureFailureEvent,
		pClass->m_hSystemResourceFailureEvent,
		pClass->m_hWatchDogTimerEvent,
		pClass->m_hKillThreadEvent,
		pClass->m_PortA.m_tPortEvents.m_hMotorStallEvent,
		pClass->m_PortB.m_tPortEvents.m_hMotorStallEvent,
		pClass->m_PortA.m_tPortEvents.m_hMotorTacFaultEvent,
		pClass->m_PortB.m_tPortEvents.m_hMotorTacFaultEvent,
		pClass->m_PortA.m_tPortEvents.m_hMotorShortCircuitEvent,
		pClass->m_PortB.m_tPortEvents.m_hMotorShortCircuitEvent,
		pClass->m_PortA.m_tPortEvents.m_hMotorShortCircuitTimeoutEvent,
		pClass->m_PortB.m_tPortEvents.m_hMotorShortCircuitTimeoutEvent,
		pClass->m_PortA.m_tPortEvents.m_hUnknownBladeIdEvent,
		pClass->m_PortB.m_tPortEvents.m_hUnknownBladeIdEvent,
		pClass->m_PortA.m_tPortEvents.m_hUnknownDeviceIdEvent,
		pClass->m_PortB.m_tPortEvents.m_hUnknownDeviceIdEvent,
		pClass->m_PortA.m_tPortEvents.m_hHallPatternFaultEvent,
		pClass->m_PortB.m_tPortEvents.m_hHallPatternFaultEvent,
		pClass->m_Footswitch.m_tFootswitchEvents.m_hLowBatteryEvent,
		pClass->m_Footswitch.m_tFootswitchEvents.m_hStuckPedalEvent,
		pClass->m_PortA.m_tPortEvents.m_hMotorCurrentLimitEvent,
		pClass->m_PortA.m_tPortEvents.m_hMotorTorqueLimitEvent,
		pClass->m_PortB.m_tPortEvents.m_hMotorCurrentLimitEvent,
		pClass->m_PortB.m_tPortEvents.m_hMotorTorqueLimitEvent,
		pClass->m_Footswitch.m_tFootswitchEvents.m_hUnknownIdEvent,
		pClass->m_PortA.m_tPortEvents.m_hHandpieceStuckButtonEvent,
		pClass->m_PortB.m_tPortEvents.m_hHandpieceStuckButtonEvent,
		pClass->m_PortA.m_tPortEvents.m_hMotorCurrentLimitTimeoutEvent,
		pClass->m_PortB.m_tPortEvents.m_hMotorCurrentLimitTimeoutEvent,
		pClass->m_PortA.m_tPortEvents.m_hMotorStallAndCurrentLimitEvent,
		pClass->m_PortB.m_tPortEvents.m_hMotorStallAndCurrentLimitEvent,
		pClass->m_PortA.m_tPortEvents.m_hFootswitchRequiredEvent,
		pClass->m_PortB.m_tPortEvents.m_hFootswitchRequiredEvent
	};
	
	// Wait up to 3 seconds for the display to initialize before sending
	// Warning updates to the display.
	WaitForSingleObject(pClass->m_hDisplayInitializedEvent,3000);
	
	while( !bDone)
	{
		
		dwObj = WaitForMultipleObjects(dwEventCount, &hEvents[0], FALSE, INFINITE);
		dwStatus = dwObj - WAIT_OBJECT_0;
		
		switch( dwStatus)
		{
		case 0:
			pClass->m_bCommunicationFailure = TRUE;
			
			// Shut down the hardware; Fatal error
			pClass->DeInitHardware(TRUE);
			
			// Broadcast Communication error message to anyone who's listening
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)COMMUNICATION_ERROR, (LPARAM)0);
			break;
			
		case 1:
			
			if( pClass->m_dwTemperature == TEMPERATURE_WARNING || pClass->m_dwTemperature == CLEAR_ERROR_CONDITION)
			{
				
				// Tell the registered GUI that an Over Temperature condition has occurred
				PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)TEMPERATURE_ERROR,
					(LPARAM)pClass->m_dwTemperature);
			}
			else if( pClass->m_dwTemperature == MAX_TEMPERATURE) 
			{
				
				// Shut down the hardware; Fatal error
				pClass->DeInitHardware(TRUE);
				
				// Broadcast Fatal Temperature error message to anyone who's listening
				PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)TEMPERATURE_ERROR,
					(LPARAM)pClass->m_dwTemperature);
			}
			break;
			
		case 2:
			
			// Shut down the hardware; Fatal error
			pClass->DeInitHardware(TRUE);
			
			// Broadcast Fatal System Resource error message to anyone who's listening
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)SYSTEM_RESOURCE_ERROR, (LPARAM)0);
			break;
			
		case 3:
			
			// Shut down the hardware; Fatal error
			pClass->DeInitHardware(TRUE);
			
			// Broadcast Fatal System error Watch Dog Timer fired
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)WATCHDOG_TIMER_ERROR, (LPARAM)0);
			break;
			
		case 4:
			// Exit Thread Event
			bDone = TRUE; // get out 
			break;
			
		case 5:
			// Motor A Stall
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)MOTORA_STALL, 
				(LPARAM)pClass->m_PortA.m_tWarningStatus.bMotorStall);		
			break;
			
		case 6: 
			// Motor B Stall
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)MOTORB_STALL, 
				(LPARAM)pClass->m_PortB.m_tWarningStatus.bMotorStall);		
			break;
		case 7:
			// Motor A Tac Fault
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)MOTORA_TAC_FAULT, 
				(LPARAM)pClass->m_PortA.m_tWarningStatus.bMotorTacFault);	
			break;
			
		case 8:
			// Motor B Tac Fault
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)MOTORB_TAC_FAULT, 
				(LPARAM)pClass->m_PortB.m_tWarningStatus.bMotorTacFault);	
			break;
			
		case 9:
			// Motor A Short Circuit
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)MOTORA_SHORT_CIRCUIT,
				(LPARAM)pClass->m_PortA.m_tWarningStatus.bMotorShortCircuit);
			break;
			
		case 10:
			// Motor B Short Circuit
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)MOTORB_SHORT_CIRCUIT,
				(LPARAM)pClass->m_PortB.m_tWarningStatus.bMotorShortCircuit);
			break;
			
		case 11:
			// Motor A Short Circuit Timeout
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)MOTORA_SHORT_CIRCUIT_TIMEOUT,
				(LPARAM)pClass->m_PortA.m_tWarningStatus.bMotorShortCircuitTimeout);
			break;
			
		case 12:
			// Motor B Short Circuit Timeout
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)MOTORB_SHORT_CIRCUIT_TIMEOUT,
				(LPARAM)pClass->m_PortB.m_tWarningStatus.bMotorShortCircuitTimeout);
			break;
			
			
		case 13:
			// Unknown Blade ID Port A
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)UNKNOWN_BLADE_ID_PORTA,
				(LPARAM)pClass->m_PortA.m_tWarningStatus.bUnknownBladeId);
			break;
			
		case 14:
			// Unknown Blade ID Port B
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)UNKNOWN_BLADE_ID_PORTB,
				(LPARAM)pClass->m_PortB.m_tWarningStatus.bUnknownBladeId);
			break;
			
		case 15:
			// Unknown Device ID Port A
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)UNKNOWN_DEVICE_ID_PORTA,
				(LPARAM)(pClass->m_PortA.m_tWarningStatus.bUnknownDeviceId ||
				pClass->m_PortA.m_tWarningStatus.bUnstableDeviceId));
			break;
			
		case 16:
			// Unknown Device ID Port B
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)UNKNOWN_DEVICE_ID_PORTB,
				(LPARAM)(pClass->m_PortB.m_tWarningStatus.bUnknownDeviceId ||
				pClass->m_PortB.m_tWarningStatus.bUnstableDeviceId));
			break;
			
		case 17:
			// Hall Pattern Fault Port A
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)HALL_PATTERN_FAULT_PORTA,
				(LPARAM)pClass->m_PortA.m_tWarningStatus.bHallPatternFault);
			break;
			
		case 18:
			// Hall Pattern Fault Port B
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)HALL_PATTERN_FAULT_PORTB,
				(LPARAM)pClass->m_PortB.m_tWarningStatus.bHallPatternFault);
			break;
			
		case 19:
			// Footswitch Battery Low
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)FOOTSWITCH_LOW_BATTERY,
				(LPARAM)pClass->m_Footswitch.m_tFootswitchWarningStatus.bLowBattery);
			break;
			
		case 20:
			// Footswitch Stuck Pedal
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)FOOTSWITCH_STUCK_PEDAL,
				(LPARAM)pClass->m_Footswitch.m_tFootswitchWarningStatus.bStuckPedal);
			break;
			
		case 21:
			// Current Limit Port A
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)MOTORA_CURRENT_LIMIT,
				(LPARAM)pClass->m_PortA.m_tWarningStatus.bMotorCurrentLimit);
			break;
			
		case 22:
			// Torque Limit Port A
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)MOTORA_TORQUE_LIMIT,
				(LPARAM)pClass->m_PortA.m_tWarningStatus.bMotorTorqueLimit);
			break;
			
		case 23:
			// Current Limit Port B
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)MOTORB_CURRENT_LIMIT,
				(LPARAM)pClass->m_PortB.m_tWarningStatus.bMotorCurrentLimit);
			break;
			
		case 24:
			// Torque Limit Port B
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)MOTORB_TORQUE_LIMIT,
				(LPARAM)pClass->m_PortB.m_tWarningStatus.bMotorTorqueLimit);
			break;
			
		case 25:
			// Unknown Footswitch ID
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)UNKNOWN_FOOTSWITCH_ID,
				(LPARAM)pClass->m_Footswitch.m_tFootswitchWarningStatus.bUnknownID);
			break;
			
		case 26:
			// Handpiece stuck button Motor A
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)HANDPIECE_STUCK_BUTTON_PORTA,
				(LPARAM)pClass->m_PortA.m_tWarningStatus.bHandpieceStuckButton);
			break;
			
		case 27:
			// Handpiece stuck button Motor B
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)HANDPIECE_STUCK_BUTTON_PORTB,
				(LPARAM)pClass->m_PortB.m_tWarningStatus.bHandpieceStuckButton);
			break;
			
			
		case 28:
			// Current Limit Timeout PortA
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)MOTORA_CURRENT_LIMIT_TIMEOUT,
				(LPARAM)pClass->m_PortA.m_tWarningStatus.bMotorCurrentLimitTimeout);
			break;
			
		case 29:
			// Current Limit Timeout PortB
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)MOTORB_CURRENT_LIMIT_TIMEOUT,
				(LPARAM)pClass->m_PortB.m_tWarningStatus.bMotorCurrentLimitTimeout);
			break;
			
		case 30:
			// Motor Stall + Current Limit Port A
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)MOTORA_STALL_AND_CURRENT_LIMIT,
				(LPARAM)pClass->m_PortA.m_tWarningStatus.bMotorStallAndCurrentLimit);
			break;
			
		case 31:
			// Motor Stall + Current Limit Port B
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)MOTORB_STALL_AND_CURRENT_LIMIT,
				(LPARAM)pClass->m_PortB.m_tWarningStatus.bMotorStallAndCurrentLimit);
			break;
			
		case 32:
			// Footswitch is required Port A
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)FOOTSWITCH_REQUIRED_PORTA,
				(LPARAM)pClass->m_PortA.m_tWarningStatus.bFootswitchRequired);
			break;
			
		case 33:
			// Footswitch is required PortB
			PostMessage(HWND_BROADCAST, WM_ERROR_CONDITION, (WPARAM)FOOTSWITCH_REQUIRED_PORTB,
				(LPARAM)pClass->m_PortB.m_tWarningStatus.bFootswitchRequired);
			break;
			
		default:
			break;
		}// end switch	
		
	} // end while
	
	// Set the Thread Killed Event
	SetEvent( pClass->m_hHandlerThreadKilledEvent);
	
	return 0;
}

SnByte CControl::CrcMemChunk(SnByte* pbSrc,SnQByte qLen,SnByte bCrc)
{
    do 
	{
        bCrc = pucCrcTable[bCrc ^ *pbSrc++];
    } while (--qLen > 0);
	
	return bCrc;
}

SnByte CControl::CrcMem(SnByte *pbSrc, SnQByte qLen)
{
    return CrcMemChunk(pbSrc,qLen,0);
}

SnBool CControl::RecallNvRamData()
{
	
	NVRAM_DATA tSavedNvRamInfo;
	SnQByte qBytesRead, qBytesWritten;
    int iRetry;
	SnBool bStatus;
	SnByte ucCrc;
	SnBool bRestoreFailed = FALSE;
	
	
	if( !m_hDriver)
		return FALSE;
	
	// Check the battery
    if(!m_hDriver->CheckNvRamBattery())
	{
		m_wBatteryStatus = BATTERY_FAILED;
		SetSystemDefaults();
        SaveNvRamData();
		return FALSE;
    }
	
	// Read structure from non-volatile memory
	for(iRetry = MAX_RETRIES; iRetry > 0; iRetry--) 
	{
		// Get the data from the NVRAM
		bStatus = m_hDriver->ReadNvRam(&tSavedNvRamInfo,0,sizeof(tSavedNvRamInfo),&qBytesRead);
		// Get the CRC
		ucCrc = CrcMem((unsigned char*)&tSavedNvRamInfo,sizeof(tSavedNvRamInfo));
		
		if( bStatus && (qBytesRead == sizeof(tSavedNvRamInfo)) && ucCrc == 0)
			break; // retrieved data, get out
		
    }
	
	if(iRetry == 0)
    {
		// Might be the first time recalling data, the check some might not be 0
		// try to save the default values 
		
		// Calculate the CRC
		m_tNvRamDefaults.ucCrc = 	CrcMem((unsigned char*)&m_tNvRamDefaults,(sizeof(m_tNvRamDefaults)) - 1);
		
		bStatus = m_hDriver->WriteNvRam((unsigned char*)&m_tNvRamDefaults,0,sizeof(m_tNvRamDefaults),&qBytesWritten);
		
		if( bStatus && qBytesWritten == sizeof(m_tNvRamDefaults))
		{
			// Recall Data the write was sucessfull
			// Read structure from NVRAM memory
			for(iRetry = MAX_RETRIES; iRetry > 0; iRetry--) 
			{
				bStatus = m_hDriver->ReadNvRam(&tSavedNvRamInfo,0,sizeof(tSavedNvRamInfo),&qBytesRead);
				// Get the CRC
				ucCrc = CrcMem((unsigned char*)&tSavedNvRamInfo,sizeof(tSavedNvRamInfo));
				
				if( bStatus && (qBytesRead == sizeof(tSavedNvRamInfo)) && ucCrc == 0)
					break; // Everything is good, get out
				
			}
			
			if(iRetry == 0)
				bRestoreFailed = TRUE; // Failed again
		}
		else
			bRestoreFailed = TRUE;
	}
	
	
	if( !bRestoreFailed)
	{		
		// Check data for correctness
		if(tSavedNvRamInfo.ucFootMode != FOOT_MODE_ON_OFF && tSavedNvRamInfo.ucFootMode != FOOT_MODE_VARIABLE)
			bRestoreFailed = TRUE;
		
		if( tSavedNvRamInfo.ucFootForward != FOOT_FORWARD_RIGHT && tSavedNvRamInfo.ucFootForward != FOOT_FORWARD_LEFT)
			bRestoreFailed = TRUE;
		
		if( tSavedNvRamInfo.ucFootHandCtl != FOOT_HAND_OVERRIDE_ON && tSavedNvRamInfo.ucFootHandCtl != FOOT_HAND_OVERRIDE_OFF)
			bRestoreFailed = TRUE;
		
		if( tSavedNvRamInfo.ucPortCtl != PORTA && tSavedNvRamInfo.ucPortCtl != PORTB)
			bRestoreFailed = TRUE;
		
		if( tSavedNvRamInfo.ucOscModePortA != OSC_MODE1 && tSavedNvRamInfo.ucOscModePortA != OSC_MODE2)
			bRestoreFailed = TRUE;
		
		if( tSavedNvRamInfo.ucOscPortARev < m_PortA.m_tPortStatus.usRevolutionsMin || tSavedNvRamInfo.ucOscPortARev > m_PortA.m_tPortStatus.usRevolutionsMax)
			bRestoreFailed = TRUE;
		
		if( tSavedNvRamInfo.ucOscPortASec < m_PortA.m_tPortStatus.wDwellMin || tSavedNvRamInfo.ucOscPortASec > m_PortA.m_tPortStatus.wDwellMax)
			bRestoreFailed = TRUE;
		
		if( tSavedNvRamInfo.ucOscModePortB != OSC_MODE1 && tSavedNvRamInfo.ucOscModePortB != OSC_MODE2)
			bRestoreFailed = TRUE;
		
		if( tSavedNvRamInfo.ucOscPortBRev < m_PortB.m_tPortStatus.usRevolutionsMin || tSavedNvRamInfo.ucOscPortBRev > m_PortB.m_tPortStatus.usRevolutionsMax)
			bRestoreFailed = TRUE;
		
		if( tSavedNvRamInfo.ucOscPortBSec < m_PortB.m_tPortStatus.wDwellMin || tSavedNvRamInfo.ucOscPortBSec > m_PortB.m_tPortStatus.wDwellMax)
			bRestoreFailed = TRUE;
		
		if( tSavedNvRamInfo.ucCustDefaultMode != DEFAULT_MODE && tSavedNvRamInfo.ucCustDefaultMode != CUSTOM_MODE)
			bRestoreFailed = TRUE;
		
		if( tSavedNvRamInfo.ucShaverPktCtl != PORTA && tSavedNvRamInfo.ucShaverPktCtl != PORTB)
			bRestoreFailed = TRUE;
	}
	
	if (bRestoreFailed) 
	{
        // Use the defaults
		SetSystemDefaults();
        SaveNvRamData();
    } 
	else
	{
		
		// Foot Pedal Status
		m_Footswitch.m_tFootPedalStatus.usMode = tSavedNvRamInfo.ucFootMode;
		m_Footswitch.m_tFootPedalStatus.usForward = tSavedNvRamInfo.ucFootForward;
		m_Footswitch.m_tFootPedalStatus.usOverride = tSavedNvRamInfo.ucFootHandCtl;
		m_Footswitch.m_tFootPedalStatus.usPortControl = tSavedNvRamInfo.ucPortCtl;
		
		// Port A Status
		m_PortA.m_tPortStatus.usOscMode = tSavedNvRamInfo.ucOscModePortA;	
		m_PortA.m_tPortStatus.usRevolutions = 	tSavedNvRamInfo.ucOscPortARev;		
		m_PortA.m_tPortStatus.wDwell = tSavedNvRamInfo.ucOscPortASec;
		
		// Port B Status
		m_PortB.m_tPortStatus.usOscMode = tSavedNvRamInfo.ucOscModePortB;	
		m_PortB.m_tPortStatus.usRevolutions = 	tSavedNvRamInfo.ucOscPortBRev;		
		m_PortB.m_tPortStatus.wDwell = tSavedNvRamInfo.ucOscPortBSec;
		m_usCustDefaultMode = 	tSavedNvRamInfo.ucCustDefaultMode;  // Default/Custom
		
        m_usLanguage = tSavedNvRamInfo.ucLanguage;
		m_usShaverPacketControl = tSavedNvRamInfo.ucShaverPktCtl;
		
	}
	
	if( bRestoreFailed)
		return FALSE;
	else
		return TRUE;
}

SnBool CControl::SaveNvRamData()
{
	NVRAM_DATA tNvRamInfo;
	SnQByte qBytesWritten;
    int iRetry;
	
	// Foot Pedal Status
	tNvRamInfo.ucFootMode = m_Footswitch.m_tFootPedalStatus.usMode;
	tNvRamInfo.ucFootForward = m_Footswitch.m_tFootPedalStatus.usForward;
	tNvRamInfo.ucFootHandCtl = m_Footswitch.m_tFootPedalStatus.usOverride;
	tNvRamInfo.ucPortCtl = m_Footswitch.m_tFootPedalStatus.usPortControl;
	
	// Port A Status
	tNvRamInfo.ucOscModePortA = m_PortA.m_tPortStatus.usOscMode;	
	tNvRamInfo.ucOscPortARev = m_PortA.m_tPortStatus.usRevolutions;		
	tNvRamInfo.ucOscPortASec = (SnByte)m_PortA.m_tPortStatus.wDwell;
	
	// Port B Status
	tNvRamInfo.ucOscModePortB = m_PortB.m_tPortStatus.usOscMode;	
	tNvRamInfo.ucOscPortBRev = m_PortB.m_tPortStatus.usRevolutions;		
	tNvRamInfo.ucOscPortBSec = (SnByte)m_PortB.m_tPortStatus.wDwell;
	
	// System status
	tNvRamInfo.ucCustDefaultMode = m_usCustDefaultMode;  // Default/Custom
	tNvRamInfo.ucLanguage = m_usLanguage;
	tNvRamInfo.ucShaverPktCtl = m_usShaverPacketControl;
	
	
	// Calculate the check sum
	tNvRamInfo.ucCrc = CrcMem((unsigned char*)&tNvRamInfo,(sizeof(tNvRamInfo) - 1));
	
	
    // Write structure to NVRAM memory, retrying a few times
    for(iRetry = MAX_RETRIES; iRetry > 0; iRetry--) 
	{
        if((m_hDriver->WriteNvRam((unsigned char*)&tNvRamInfo,0,sizeof(tNvRamInfo),&qBytesWritten) != FALSE)
            && (qBytesWritten == sizeof(tNvRamInfo))) 
		{
            break; // Everything ok, get out
        }
    }
	
	
	if( iRetry == 0)
		return FALSE; // Save failed
	
	return TRUE;
	
}

void CControl::DeviceToFlash(DEVICE_DATA ptSrc, SAVE_DEVICE_DATA *ptDst)
{
    ptDst->ucVersion = 0;
	
	ptDst->ucBasicForwardRpm = ptSrc.usBasicForwardRpm / 100;
	ptDst->ucBasicReverseRpm = ptSrc.usBasicReverseRpm / 100;
	ptDst->ucBasicForward2Rpm = ptSrc.usBasicForward2Rpm / 100;
	ptDst->ucBasicReverse2Rpm = ptSrc.usBasicReverse2Rpm / 100;
	ptDst->ucBasicOscillateRpm = ptSrc.usBasicOscillateRpm / 100;
	ptDst->ucBasicOscillateSec = (SnByte)ptSrc.usBasicOscillateSec;
	
	ptDst->ucMiniForwardRpm = ptSrc.usMiniForwardRpm / 100;
	ptDst->ucMiniReverseRpm = ptSrc.usMiniReverseRpm / 100;
	ptDst->ucMiniForward2Rpm = ptSrc.usMiniForward2Rpm / 100;
	ptDst->ucMiniReverse2Rpm = ptSrc.usMiniReverse2Rpm / 100;
	ptDst->ucMiniOscillateRpm = ptSrc.usMiniOscillateRpm / 100;
	ptDst->ucMiniOscillateSec = (SnByte)ptSrc.usMiniOscillateSec;
	
	ptDst->ucHighSpeedCurvedForwardRpm = ptSrc.usHighSpeedCurvedForwardRpm / 100;
	ptDst->ucHighSpeedCurvedReverseRpm = ptSrc.usHighSpeedCurvedReverseRpm / 100;
	ptDst->ucHighSpeedCurvedForward2Rpm = ptSrc.usHighSpeedCurvedForward2Rpm / 100;
	ptDst->ucHighSpeedCurvedReverse2Rpm = ptSrc.usHighSpeedCurvedReverse2Rpm / 100;
	ptDst->ucHighSpeedCurvedOscillateRpm = ptSrc.usHighSpeedCurvedOscillateRpm / 100;
	ptDst->ucHighSpeedCurvedOscillateSec = (SnByte)ptSrc.usHighSpeedCurvedOscillateSec;
	
	ptDst->ucHighSpeedStraightForwardRpm = ptSrc.usHighSpeedStraightForwardRpm / 100;
	ptDst->ucHighSpeedStraightReverseRpm = ptSrc.usHighSpeedStraightReverseRpm / 100;
	ptDst->ucHighSpeedStraightForward2Rpm = ptSrc.usHighSpeedStraightForward2Rpm / 100;
	ptDst->ucHighSpeedStraightReverse2Rpm = ptSrc.usHighSpeedStraightReverse2Rpm / 100;
	ptDst->ucHighSpeedStraightOscillateRpm = ptSrc.usHighSpeedStraightOscillateRpm / 100;
	ptDst->ucHighSpeedStraightOscillateSec = (SnByte)ptSrc.usHighSpeedStraightOscillateSec;
	
	ptDst->ucHighSpeedBurForwardRpm = ptSrc.usHighSpeedBurForwardRpm / 100;
	ptDst->ucHighSpeedBurReverseRpm = ptSrc.usHighSpeedBurReverseRpm / 100;
	ptDst->ucHighSpeedBurForward2Rpm = ptSrc.usHighSpeedBurForward2Rpm / 100;
	ptDst->ucHighSpeedBurReverse2Rpm = ptSrc.usHighSpeedBurReverse2Rpm / 100;
	ptDst->ucHighSpeedBurOscillateRpm = ptSrc.usHighSpeedBurOscillateRpm / 100;
	ptDst->ucHighSpeedBurOscillateSec = (SnByte)ptSrc.usHighSpeedBurOscillateSec;
	
	ptDst->ucHighSpeedFastBurForwardRpm = ptSrc.usHighSpeedFastBurForwardRpm / 100;
	ptDst->ucHighSpeedFastBurReverseRpm = ptSrc.usHighSpeedFastBurReverseRpm / 100;
	ptDst->ucHighSpeedFastBurForward2Rpm = ptSrc.usHighSpeedFastBurForward2Rpm / 100;
	ptDst->ucHighSpeedFastBurReverse2Rpm = ptSrc.usHighSpeedFastBurReverse2Rpm / 100;
	ptDst->ucHighSpeedFastBurOscillateRpm = ptSrc.usHighSpeedFastBurOscillateRpm / 100;
	ptDst->ucHighSpeedFastBurOscillateSec = (SnByte)ptSrc.usHighSpeedFastBurOscillateSec;
	
    ptDst->ucLowSpeedCurvedForwardRpm = ptSrc.usLowSpeedCurvedForwardRpm / 100;
	ptDst->ucLowSpeedCurvedReverseRpm = ptSrc.usLowSpeedCurvedReverseRpm / 100;
	ptDst->ucLowSpeedCurvedForward2Rpm = ptSrc.usLowSpeedCurvedForward2Rpm / 100;
	ptDst->ucLowSpeedCurvedReverse2Rpm = ptSrc.usLowSpeedCurvedReverse2Rpm / 100;
	ptDst->ucLowSpeedCurvedOscillateRpm = ptSrc.usLowSpeedCurvedOscillateRpm / 100;
	ptDst->ucLowSpeedCurvedOscillateSec = (SnByte)ptSrc.usLowSpeedCurvedOscillateSec;
	
	ptDst->ucLowSpeedStraightForwardRpm = ptSrc.usLowSpeedStraightForwardRpm / 100;
	ptDst->ucLowSpeedStraightReverseRpm = ptSrc.usLowSpeedStraightReverseRpm / 100;
	ptDst->ucLowSpeedStraightForward2Rpm = ptSrc.usLowSpeedStraightForward2Rpm / 100;
	ptDst->ucLowSpeedStraightReverse2Rpm = ptSrc.usLowSpeedStraightReverse2Rpm / 100;
	ptDst->ucLowSpeedStraightOscillateRpm = ptSrc.usLowSpeedStraightOscillateRpm / 100;
	ptDst->ucLowSpeedStraightOscillateSec = (SnByte)ptSrc.usLowSpeedStraightOscillateSec;
	
	ptDst->ucLowSpeedBurForwardRpm = ptSrc.usLowSpeedBurForwardRpm / 100;
	ptDst->ucLowSpeedBurReverseRpm = ptSrc.usLowSpeedBurReverseRpm / 100;
	ptDst->ucLowSpeedBurForward2Rpm = ptSrc.usLowSpeedBurForward2Rpm / 100;
	ptDst->ucLowSpeedBurReverse2Rpm = ptSrc.usLowSpeedBurReverse2Rpm / 100;
	ptDst->ucLowSpeedBurOscillateRpm = ptSrc.usLowSpeedBurOscillateRpm / 100;
	ptDst->ucLowSpeedBurOscillateSec = (SnByte)ptSrc.usLowSpeedBurOscillateSec;
	
	ptDst->ucLowSpeedFastBurForwardRpm = ptSrc.usLowSpeedFastBurForwardRpm / 100;
	ptDst->ucLowSpeedFastBurReverseRpm = ptSrc.usLowSpeedFastBurReverseRpm / 100;
	ptDst->ucLowSpeedFastBurForward2Rpm = ptSrc.usLowSpeedFastBurForward2Rpm / 100;
	ptDst->ucLowSpeedFastBurReverse2Rpm = ptSrc.usLowSpeedFastBurReverse2Rpm / 100;
	ptDst->ucLowSpeedFastBurOscillateRpm = ptSrc.usLowSpeedFastBurOscillateRpm / 100;
	ptDst->ucLowSpeedFastBurOscillateSec = (SnByte)ptSrc.usLowSpeedFastBurOscillateSec;
	
	ptDst->ucSuperHighSpeedCurvedForwardRpm = ptSrc.usSuperHighSpeedCurvedForwardRpm / 100;
	ptDst->ucSuperHighSpeedCurvedReverseRpm = ptSrc.usSuperHighSpeedCurvedReverseRpm / 100;
	ptDst->ucSuperHighSpeedCurvedForward2Rpm = ptSrc.usSuperHighSpeedCurvedForward2Rpm / 100;
	ptDst->ucSuperHighSpeedCurvedReverse2Rpm = ptSrc.usSuperHighSpeedCurvedReverse2Rpm / 100;
	ptDst->ucSuperHighSpeedCurvedOscillateRpm = ptSrc.usSuperHighSpeedCurvedOscillateRpm / 100;
	ptDst->ucSuperHighSpeedCurvedOscillateSec = (SnByte)ptSrc.usSuperHighSpeedCurvedOscillateSec;
	
	ptDst->ucSuperHighSpeedStraightForwardRpm = ptSrc.usSuperHighSpeedStraightForwardRpm / 100;
	ptDst->ucSuperHighSpeedStraightReverseRpm = ptSrc.usSuperHighSpeedStraightReverseRpm / 100;
	ptDst->ucSuperHighSpeedStraightForward2Rpm = ptSrc.usSuperHighSpeedStraightForward2Rpm / 100;
	ptDst->ucSuperHighSpeedStraightReverse2Rpm = ptSrc.usSuperHighSpeedStraightReverse2Rpm / 100;
	ptDst->ucSuperHighSpeedStraightOscillateRpm = ptSrc.usSuperHighSpeedStraightOscillateRpm / 100;
	ptDst->ucSuperHighSpeedStraightOscillateSec = (SnByte)ptSrc.usSuperHighSpeedStraightOscillateSec;
	
	ptDst->ucSuperHighSpeedBurForwardRpm = ptSrc.usSuperHighSpeedBurForwardRpm / 100;
	ptDst->ucSuperHighSpeedBurReverseRpm = ptSrc.usSuperHighSpeedBurReverseRpm / 100;
	ptDst->ucSuperHighSpeedBurForward2Rpm = ptSrc.usSuperHighSpeedBurForward2Rpm / 100;
	ptDst->ucSuperHighSpeedBurReverse2Rpm = ptSrc.usSuperHighSpeedBurReverse2Rpm / 100;
	ptDst->ucSuperHighSpeedBurOscillateRpm = ptSrc.usSuperHighSpeedBurOscillateRpm / 100;
	ptDst->ucSuperHighSpeedBurOscillateSec = (SnByte)ptSrc.usSuperHighSpeedBurOscillateSec;
	
	ptDst->ucSuperHighSpeedFastBurForwardRpm = ptSrc.usSuperHighSpeedFastBurForwardRpm / 100;
	ptDst->ucSuperHighSpeedFastBurReverseRpm = ptSrc.usSuperHighSpeedFastBurReverseRpm / 100;
	ptDst->ucSuperHighSpeedFastBurForward2Rpm = ptSrc.usSuperHighSpeedFastBurForward2Rpm / 100;
	ptDst->ucSuperHighSpeedFastBurReverse2Rpm = ptSrc.usSuperHighSpeedFastBurReverse2Rpm / 100;
	ptDst->ucSuperHighSpeedFastBurOscillateRpm = ptSrc.usSuperHighSpeedFastBurOscillateRpm / 100;
	ptDst->ucSuperHighSpeedFastBurOscillateSec = (SnByte)ptSrc.usSuperHighSpeedFastBurOscillateSec;
	
	ptDst->ucSmallJointCurvedForwardRpm = ptSrc.usSmallJointCurvedForwardRpm / 100;
	ptDst->ucSmallJointCurvedReverseRpm = ptSrc.usSmallJointCurvedReverseRpm / 100;
	ptDst->ucSmallJointCurvedForward2Rpm = ptSrc.usSmallJointCurvedForward2Rpm / 100;
	ptDst->ucSmallJointCurvedReverse2Rpm = ptSrc.usSmallJointCurvedReverse2Rpm / 100;
	ptDst->ucSmallJointCurvedOscillateRpm = ptSrc.usSmallJointCurvedOscillateRpm / 100;
	ptDst->ucSmallJointCurvedOscillateSec = (SnByte)ptSrc.usSmallJointCurvedOscillateSec;
	
	ptDst->ucSmallJointStraightForwardRpm = ptSrc.usSmallJointStraightForwardRpm / 100;
	ptDst->ucSmallJointStraightReverseRpm = ptSrc.usSmallJointStraightReverseRpm / 100;
	ptDst->ucSmallJointStraightForward2Rpm = ptSrc.usSmallJointStraightForward2Rpm / 100;
	ptDst->ucSmallJointStraightReverse2Rpm = ptSrc.usSmallJointStraightReverse2Rpm / 100;
	ptDst->ucSmallJointStraightOscillateRpm = ptSrc.usSmallJointStraightOscillateRpm / 100;
	ptDst->ucSmallJointStraightOscillateSec = (SnByte)ptSrc.usSmallJointStraightOscillateSec;
	
	ptDst->ucSmallJointBurForwardRpm = ptSrc.usSmallJointBurForwardRpm / 100;
	ptDst->ucSmallJointBurReverseRpm = ptSrc.usSmallJointBurReverseRpm / 100;
	ptDst->ucSmallJointBurForward2Rpm = ptSrc.usSmallJointBurForward2Rpm / 100;
	ptDst->ucSmallJointBurReverse2Rpm = ptSrc.usSmallJointBurReverse2Rpm / 100;
	ptDst->ucSmallJointBurOscillateRpm = ptSrc.usSmallJointBurOscillateRpm / 100;
	ptDst->ucSmallJointBurOscillateSec = (SnByte)ptSrc.usSmallJointBurOscillateSec;
	
	ptDst->ucDpDrillSpeed = (SnByte)ptSrc.usDpDrillSpeed;
	ptDst->ucDpSawSpeed = (SnByte)ptSrc.usDpSawSpeed;
}

void CControl::FlashToDevice(SAVE_DEVICE_DATA *ptSrc, DEVICE_DATA *ptDst)
{
	ptDst->usBasicForwardRpm = (SnWord)ptSrc->ucBasicForwardRpm * 100;
	ptDst->usBasicReverseRpm = (SnWord)ptSrc->ucBasicReverseRpm * 100;
	ptDst->usBasicForward2Rpm = (SnWord)ptSrc->ucBasicForward2Rpm * 100;
	ptDst->usBasicReverse2Rpm = (SnWord)ptSrc->ucBasicReverse2Rpm * 100;
	ptDst->usBasicOscillateRpm = (SnWord)ptSrc->ucBasicOscillateRpm * 100;
	ptDst->usBasicOscillateSec = (SnWord)ptSrc->ucBasicOscillateSec;
	
	ptDst->usMiniForwardRpm = (SnWord)ptSrc->ucMiniForwardRpm * 100;
	ptDst->usMiniReverseRpm = (SnWord)ptSrc->ucMiniReverseRpm * 100;
	ptDst->usMiniForward2Rpm = (SnWord)ptSrc->ucMiniForward2Rpm * 100;
	ptDst->usMiniReverse2Rpm = (SnWord)ptSrc->ucMiniReverse2Rpm * 100;
	ptDst->usMiniOscillateRpm = (SnWord)ptSrc->ucMiniOscillateRpm * 100;
	ptDst->usMiniOscillateSec = (SnWord)ptSrc->ucMiniOscillateSec;
	
	ptDst->usHighSpeedCurvedForwardRpm = (SnWord)ptSrc->ucHighSpeedCurvedForwardRpm * 100;
	ptDst->usHighSpeedCurvedReverseRpm = (SnWord)ptSrc->ucHighSpeedCurvedReverseRpm * 100;
	ptDst->usHighSpeedCurvedForward2Rpm = (SnWord)ptSrc->ucHighSpeedCurvedForward2Rpm * 100;
	ptDst->usHighSpeedCurvedReverse2Rpm = (SnWord)ptSrc->ucHighSpeedCurvedReverse2Rpm * 100;
	ptDst->usHighSpeedCurvedOscillateRpm = (SnWord)ptSrc->ucHighSpeedCurvedOscillateRpm * 100;
	ptDst->usHighSpeedCurvedOscillateSec = (SnWord)ptSrc->ucHighSpeedCurvedOscillateSec;
	
	ptDst->usHighSpeedStraightForwardRpm = (SnWord)ptSrc->ucHighSpeedStraightForwardRpm * 100;
	ptDst->usHighSpeedStraightReverseRpm = (SnWord)ptSrc->ucHighSpeedStraightReverseRpm * 100;
	ptDst->usHighSpeedStraightForward2Rpm = (SnWord)ptSrc->ucHighSpeedStraightForward2Rpm * 100;
	ptDst->usHighSpeedStraightReverse2Rpm = (SnWord)ptSrc->ucHighSpeedStraightReverse2Rpm * 100;
	ptDst->usHighSpeedStraightOscillateRpm = (SnWord)ptSrc->ucHighSpeedStraightOscillateRpm * 100;
	ptDst->usHighSpeedStraightOscillateSec = (SnWord)ptSrc->ucHighSpeedStraightOscillateSec;
	
	ptDst->usHighSpeedBurForwardRpm = (SnWord)ptSrc->ucHighSpeedBurForwardRpm * 100;
	ptDst->usHighSpeedBurReverseRpm = (SnWord)ptSrc->ucHighSpeedBurReverseRpm * 100;
	ptDst->usHighSpeedBurForward2Rpm = (SnWord)ptSrc->ucHighSpeedBurForward2Rpm * 100;
	ptDst->usHighSpeedBurReverse2Rpm = (SnWord)ptSrc->ucHighSpeedBurReverse2Rpm * 100;
	ptDst->usHighSpeedBurOscillateRpm = (SnWord)ptSrc->ucHighSpeedBurOscillateRpm * 100;
	ptDst->usHighSpeedBurOscillateSec = (SnWord)ptSrc->ucHighSpeedBurOscillateSec;
	
	ptDst->usHighSpeedFastBurForwardRpm = (SnWord)ptSrc->ucHighSpeedFastBurForwardRpm * 100;
	ptDst->usHighSpeedFastBurReverseRpm = (SnWord)ptSrc->ucHighSpeedFastBurReverseRpm * 100;
	ptDst->usHighSpeedFastBurForward2Rpm = (SnWord)ptSrc->ucHighSpeedFastBurForward2Rpm * 100;
	ptDst->usHighSpeedFastBurReverse2Rpm = (SnWord)ptSrc->ucHighSpeedFastBurReverse2Rpm * 100;
	ptDst->usHighSpeedFastBurOscillateRpm = (SnWord)ptSrc->ucHighSpeedFastBurOscillateRpm * 100;
	ptDst->usHighSpeedFastBurOscillateSec = (SnWord)ptSrc->ucHighSpeedFastBurOscillateSec;
	
    ptDst->usLowSpeedCurvedForwardRpm = (SnWord)ptSrc->ucLowSpeedCurvedForwardRpm * 100;
	ptDst->usLowSpeedCurvedReverseRpm = (SnWord)ptSrc->ucLowSpeedCurvedReverseRpm * 100;
	ptDst->usLowSpeedCurvedForward2Rpm = (SnWord)ptSrc->ucLowSpeedCurvedForward2Rpm * 100;
	ptDst->usLowSpeedCurvedReverse2Rpm = (SnWord)ptSrc->ucLowSpeedCurvedReverse2Rpm * 100;
	ptDst->usLowSpeedCurvedOscillateRpm = (SnWord)ptSrc->ucLowSpeedCurvedOscillateRpm * 100;
	ptDst->usLowSpeedCurvedOscillateSec = (SnWord)ptSrc->ucLowSpeedCurvedOscillateSec;
	
	ptDst->usLowSpeedStraightForwardRpm = (SnWord)ptSrc->ucLowSpeedStraightForwardRpm * 100;
	ptDst->usLowSpeedStraightReverseRpm = (SnWord)ptSrc->ucLowSpeedStraightReverseRpm * 100;
	ptDst->usLowSpeedStraightForward2Rpm = (SnWord)ptSrc->ucLowSpeedStraightForward2Rpm * 100;
	ptDst->usLowSpeedStraightReverse2Rpm = (SnWord)ptSrc->ucLowSpeedStraightReverse2Rpm * 100;
	ptDst->usLowSpeedStraightOscillateRpm = (SnWord)ptSrc->ucLowSpeedStraightOscillateRpm * 100;
	ptDst->usLowSpeedStraightOscillateSec = (SnWord)ptSrc->ucLowSpeedStraightOscillateSec;
	
	ptDst->usLowSpeedBurForwardRpm = (SnWord)ptSrc->ucLowSpeedBurForwardRpm * 100;
	ptDst->usLowSpeedBurReverseRpm = (SnWord)ptSrc->ucLowSpeedBurReverseRpm * 100;
	ptDst->usLowSpeedBurForward2Rpm = (SnWord)ptSrc->ucLowSpeedBurForward2Rpm * 100;
	ptDst->usLowSpeedBurReverse2Rpm = (SnWord)ptSrc->ucLowSpeedBurReverse2Rpm * 100;
	ptDst->usLowSpeedBurOscillateRpm = (SnWord)ptSrc->ucLowSpeedBurOscillateRpm * 100;
	ptDst->usLowSpeedBurOscillateSec = (SnWord)ptSrc->ucLowSpeedBurOscillateSec;
	
	ptDst->usLowSpeedFastBurForwardRpm = (SnWord)ptSrc->ucLowSpeedFastBurForwardRpm * 100;
	ptDst->usLowSpeedFastBurReverseRpm = (SnWord)ptSrc->ucLowSpeedFastBurReverseRpm * 100;
	ptDst->usLowSpeedFastBurForward2Rpm = (SnWord)ptSrc->ucLowSpeedFastBurForward2Rpm * 100;
	ptDst->usLowSpeedFastBurReverse2Rpm = (SnWord)ptSrc->ucLowSpeedFastBurReverse2Rpm * 100;
	ptDst->usLowSpeedFastBurOscillateRpm = (SnWord)ptSrc->ucLowSpeedFastBurOscillateRpm * 100;
	ptDst->usLowSpeedFastBurOscillateSec = (SnWord)ptSrc->ucLowSpeedFastBurOscillateSec;
	
	ptDst->usSuperHighSpeedCurvedForwardRpm = (SnWord)ptSrc->ucSuperHighSpeedCurvedForwardRpm * 100;
	ptDst->usSuperHighSpeedCurvedReverseRpm = (SnWord)ptSrc->ucSuperHighSpeedCurvedReverseRpm * 100;
	ptDst->usSuperHighSpeedCurvedForward2Rpm = (SnWord)ptSrc->ucSuperHighSpeedCurvedForward2Rpm * 100;
	ptDst->usSuperHighSpeedCurvedReverse2Rpm = (SnWord)ptSrc->ucSuperHighSpeedCurvedReverse2Rpm * 100;
	ptDst->usSuperHighSpeedCurvedOscillateRpm = (SnWord)ptSrc->ucSuperHighSpeedCurvedOscillateRpm * 100;
	ptDst->usSuperHighSpeedCurvedOscillateSec = (SnWord)ptSrc->ucSuperHighSpeedCurvedOscillateSec;
	
	ptDst->usSuperHighSpeedStraightForwardRpm = (SnWord)ptSrc->ucSuperHighSpeedStraightForwardRpm * 100;
	ptDst->usSuperHighSpeedStraightReverseRpm = (SnWord)ptSrc->ucSuperHighSpeedStraightReverseRpm * 100;
	ptDst->usSuperHighSpeedStraightForward2Rpm = (SnWord)ptSrc->ucSuperHighSpeedStraightForward2Rpm * 100;
	ptDst->usSuperHighSpeedStraightReverse2Rpm = (SnWord)ptSrc->ucSuperHighSpeedStraightReverse2Rpm * 100;
	ptDst->usSuperHighSpeedStraightOscillateRpm = (SnWord)ptSrc->ucSuperHighSpeedStraightOscillateRpm * 100;
	ptDst->usSuperHighSpeedStraightOscillateSec = (SnWord)ptSrc->ucSuperHighSpeedStraightOscillateSec;
	
	ptDst->usSuperHighSpeedBurForwardRpm = (SnWord)ptSrc->ucSuperHighSpeedBurForwardRpm * 100;
	ptDst->usSuperHighSpeedBurReverseRpm = (SnWord)ptSrc->ucSuperHighSpeedBurReverseRpm * 100;
	ptDst->usSuperHighSpeedBurForward2Rpm = (SnWord)ptSrc->ucSuperHighSpeedBurForward2Rpm * 100;
	ptDst->usSuperHighSpeedBurReverse2Rpm = (SnWord)ptSrc->ucSuperHighSpeedBurReverse2Rpm * 100;
	ptDst->usSuperHighSpeedBurOscillateRpm = (SnWord)ptSrc->ucSuperHighSpeedBurOscillateRpm * 100;
	ptDst->usSuperHighSpeedBurOscillateSec = (SnWord)ptSrc->ucSuperHighSpeedBurOscillateSec;
	
	ptDst->usSuperHighSpeedFastBurForwardRpm = (SnWord)ptSrc->ucSuperHighSpeedFastBurForwardRpm * 100;
	ptDst->usSuperHighSpeedFastBurReverseRpm = (SnWord)ptSrc->ucSuperHighSpeedFastBurReverseRpm * 100;
	ptDst->usSuperHighSpeedFastBurForward2Rpm = (SnWord)ptSrc->ucSuperHighSpeedFastBurForward2Rpm * 100;
	ptDst->usSuperHighSpeedFastBurReverse2Rpm = (SnWord)ptSrc->ucSuperHighSpeedFastBurReverse2Rpm * 100;
	ptDst->usSuperHighSpeedFastBurOscillateRpm = (SnWord)ptSrc->ucSuperHighSpeedFastBurOscillateRpm * 100;
	ptDst->usSuperHighSpeedFastBurOscillateSec = (SnWord)ptSrc->ucSuperHighSpeedFastBurOscillateSec;
	
	ptDst->usSmallJointCurvedForwardRpm = (SnWord)ptSrc->ucSmallJointCurvedForwardRpm * 100;
	ptDst->usSmallJointCurvedReverseRpm = (SnWord)ptSrc->ucSmallJointCurvedReverseRpm * 100;
	ptDst->usSmallJointCurvedForward2Rpm = (SnWord)ptSrc->ucSmallJointCurvedForward2Rpm * 100;
	ptDst->usSmallJointCurvedReverse2Rpm = (SnWord)ptSrc->ucSmallJointCurvedReverse2Rpm * 100;
	ptDst->usSmallJointCurvedOscillateRpm = (SnWord)ptSrc->ucSmallJointCurvedOscillateRpm * 100;
	ptDst->usSmallJointCurvedOscillateSec = (SnWord)ptSrc->ucSmallJointCurvedOscillateSec;
	
	ptDst->usSmallJointStraightForwardRpm = (SnWord)ptSrc->ucSmallJointStraightForwardRpm * 100;
	ptDst->usSmallJointStraightReverseRpm = (SnWord)ptSrc->ucSmallJointStraightReverseRpm * 100;
	ptDst->usSmallJointStraightForward2Rpm = (SnWord)ptSrc->ucSmallJointStraightForward2Rpm * 100;
	ptDst->usSmallJointStraightReverse2Rpm = (SnWord)ptSrc->ucSmallJointStraightReverse2Rpm * 100;
	ptDst->usSmallJointStraightOscillateRpm = (SnWord)ptSrc->ucSmallJointStraightOscillateRpm * 100;
	ptDst->usSmallJointStraightOscillateSec = (SnWord)ptSrc->ucSmallJointStraightOscillateSec;
	
	ptDst->usSmallJointBurForwardRpm = (SnWord)ptSrc->ucSmallJointBurForwardRpm * 100;
	ptDst->usSmallJointBurReverseRpm = (SnWord)ptSrc->ucSmallJointBurReverseRpm * 100;
	ptDst->usSmallJointBurForward2Rpm = (SnWord)ptSrc->ucSmallJointBurForward2Rpm * 100;
	ptDst->usSmallJointBurReverse2Rpm = (SnWord)ptSrc->ucSmallJointBurReverse2Rpm * 100;
	ptDst->usSmallJointBurOscillateRpm = (SnWord)ptSrc->ucSmallJointBurOscillateRpm * 100;
	ptDst->usSmallJointBurOscillateSec = (SnWord)ptSrc->ucSmallJointBurOscillateSec;
	
	ptDst->usDpDrillSpeed = (SnWord)ptSrc->ucDpDrillSpeed;
	ptDst->usDpSawSpeed = (SnWord)ptSrc->ucDpSawSpeed;
}

SnBool CControl::SaveFlashData()
{
	SnBool bStatus = FALSE;
	SnByte pbBuf[FLASH_SAVE_SIZE];
	
	if (m_hDriver)
	{
		// Copy PortA and PortB parameters into buffer
		m_PortA.SetSaveIfDirty(FALSE);
        DeviceToFlash(m_PortA.m_tPortSavedParam, (SAVE_DEVICE_DATA*)pbBuf);
		
		m_PortB.SetSaveIfDirty(FALSE);
		DeviceToFlash(m_PortB.m_tPortSavedParam, ((SAVE_DEVICE_DATA*)pbBuf) + 1);
		
        bStatus = m_hDriver->WriteFlashStore(pbBuf, FLASH_SAVE_SIZE);
	}
	
	return bStatus;
}

void CControl::EraseAndRestoreDefaults(void)
{
    SetSystemDefaults();
    SaveNvRamData();
	EnterCriticalSection(&m_csWriteFlashAccess);
    m_hDriver->EraseFlashStore();
	
	// Copy default values to Port A and Port B structures
	memcpy(&m_PortA.m_tPortSavedParam, &m_PortA.m_tPortDefaultParam, sizeof(DEVICE_DATA));
	memcpy(&m_PortB.m_tPortSavedParam, &m_PortB.m_tPortDefaultParam, sizeof(DEVICE_DATA));
	
    SaveFlashData();
	LeaveCriticalSection(&m_csWriteFlashAccess);
}

SnBool CControl::RecallFlashData()
{
	SnByte pbBuf[FLASH_SAVE_SIZE];
    SnQByte qLastStoreSize;
	SnBool bStatus = FALSE;
	
    if (m_hDriver)
	{
		EnterCriticalSection(&m_csWriteFlashAccess);
        // Read the Flash device
        bStatus = m_hDriver->ReadFlashStore(FLASH_SAVE_BASE, FLASH_SAVE_TOP,
			FLASH_SAVE_SIZE, pbBuf, &qLastStoreSize);
		
		if (!bStatus || qLastStoreSize != FLASH_SAVE_SIZE)
		{
			// Copy default values to Port A and Port B structures
			memcpy(&m_PortA.m_tPortSavedParam, &m_PortA.m_tPortDefaultParam, sizeof(DEVICE_DATA));
			memcpy(&m_PortB.m_tPortSavedParam, &m_PortB.m_tPortDefaultParam, sizeof(DEVICE_DATA));
			
            // Different Store Size
            if (bStatus && qLastStoreSize > 0)
			{
                m_hDriver->EraseFlashStore();
            }
			
            SaveFlashData();
		}
		
		// Copy buffer to Port A and Port B structures
        FlashToDevice((SAVE_DEVICE_DATA*)pbBuf, &m_PortA.m_tPortSavedParam);
        FlashToDevice(((SAVE_DEVICE_DATA*)pbBuf) + 1, &m_PortB.m_tPortSavedParam);
		LeaveCriticalSection(&m_csWriteFlashAccess);
	}
	
	return bStatus;
}

void CControl::SetShaverBladeId(SnWord usPort, SnWord usPortBladeId)
{
	if( m_usShaverPacketControl == usPort )
	{
		//
		// Set blade type for the Pump
		//		
		m_tOutGoingShaverPacket.ucBladeId = (SnByte)usPortBladeId;
		m_bShaverPacketDirty = TRUE;
	}
}

void CControl::SetShaverOpState(SnWord usPort, SnWord usHandMode)
{
	if( m_usShaverPacketControl == usPort )
	{
		//
		// Init Operational state
		//
		SnWord usMode;
		if(usHandMode == MOTOR_FORWARD)
			usMode = 0x01;
		else if(usHandMode == MOTOR_REVERSE)
			usMode = 0x02;
		else if(usHandMode == MOTOR_OSCILLATE_1 || usHandMode == MOTOR_OSCILLATE_2)
			usMode = 0x03;
		else
			usMode = 0x00; // off
		
		m_tOutGoingShaverPacket.ucOpState = (unsigned char)usMode;
		
		m_bShaverPacketDirty = TRUE;
	}
}


void CControl::SetShaverSpeed(SnWord usPort, SnWord usHandMode, SnWord wPeriod, SnWord usVelocity)
{
	
	if( m_usShaverPacketControl ==  usPort)
	{
		//
		// Init Speed status (velocity or period)
		//
		if( usHandMode == MOTOR_OSCILLATE_2)
		{
			// period
			SnWord usTemp;
			usTemp = wPeriod * 10; // convert seconds to milli seconds
			m_tOutGoingShaverPacket.ucSpeedHigh = (unsigned char)(usTemp >> 8);
			m_tOutGoingShaverPacket.ucSpeedHigh = m_tOutGoingShaverPacket.ucSpeedHigh | 0x80; // set msb to 1
			m_tOutGoingShaverPacket.ucSpeedLow =  (unsigned char)usTemp;
			
		}
		else
		{
			// velocity
			m_tOutGoingShaverPacket.ucSpeedHigh = (unsigned char)(usVelocity >> 8);
			m_tOutGoingShaverPacket.ucSpeedLow =  (unsigned char) usVelocity;
		}
		m_bShaverPacketDirty = TRUE;
	}						
}
