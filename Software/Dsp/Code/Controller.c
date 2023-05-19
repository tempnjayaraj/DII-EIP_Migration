#include "Controller.h"
#include "MotorData.h"

/***************************************************************
 Global variable definitions
 ***************************************************************/
//
// Global data structure used by boot code and build upgrade tool
// Defined to be at a fixed address in the linker command file
// (and also not dead stripped).
//
// Application developer should only have to change the arguments to MAKE_VERSION
// Everything else should be left untouched (and in fact the application is not
// responsible for filling them in, that is done by the build upgrade tool).
//
//
// Define this to allow the boot code to accept the debug verison of the Application
//
#if UT_TEST != UT_NONE
#define DEBUG_APP      1
#endif

const SnBootInfo g_tBootInfo = {
#if defined(DEBUG_APP) && DEBUG_APP
	0xFEEDC0DE,                       //SnQByte qVersion;
	0x6174,                           //SnWord wEntryPoint;
	0x42,                             //SnByte bProgramFlashCrc;
	0x61,                             //SnByte bAlign;
	0x74,                             //SnByte bNumProgramFlashPages;
	0x42,                             //SnByte bDataFlashCrc;
#else
	MAKE_VERSION,                     //SnQByte qVersion;
	0,                                //SnWord wEntryPoint;
	0,                                //SnByte bProgramFlashCrc;
	0,                                //SnByte bAlign;
	0,                                //SnByte bNumProgramFlashPages;
	0,                                //SnByte bDataFlashCrc;
#endif
                                    //SnByte pbConfigBytes[18];
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

/* Data space copy of flash func table */
SnFlashFuncTable g_tFlashFuncTable;

/***************************************************************
 Status and control data array default initialization
 ***************************************************************/

Status_Control g_tVarArray = {

  /* Version information */
  MAKE_VERSION,                                         // make 32 bit version information

  /* Port Configuration */
  0x8080,                                               // Port BA - brushless motor port types

  /* Brushless motors */
  {0}, {0},                                             // tBldcA, tBldcB

  /* HallBuses */
  0, 0, 0, 0x1C, 0, 0,                                  // A: DeviceExist, DeviceActive, DeviceLatch, DeviceAssert, HallBusVq, DeltaCnt
  0, 0, 0, 0x1C, 0, 0,                                  // B: DeviceExist, DeviceActive, DeviceLatch, DeviceAssert, HallBusVq, DeltaCnt

  /* Digital port data structure */
  0, 0, 0, 0, 0, 0,                                     // processed inputs
  0, 0, 0, 0, 0, 0,                                     // debounced raw data
  0, 0, 0, 0, 0, 0,                                     // last raw data
  0, 0x003F, 0, 0x0007, 0x000C, 0,                      // active inputs (with drill direction bits)
  0, 0, 0, 0, 0, 0,                                     // assert event
  0, 0x003F, 0, 0x0007, 0, 0,                           // active low masks
  0, 0, 0, 0, 0, 0,                                     // push-on / push-off mask
  0, 5000, 0, 5000, 5000, 0,                            // debounce count (5000 ~ 28 msec)
  0,                                                    // events

  /* Analog data structure */
  0, 0, 0, 0, 0, 0, 0, 0,                               // ANA data
  0, 0, 0, 0, 0, 0, 0, 0,                               // ANB

  0, 2000, 2000, 0, 0, 0, 0, 0,                         // ANA average scaled result
  0, 0, 0, 0, 0, 0, 0, 0,                               // ANB

  45, 0, 0, 0, 0, 0, 0, 0,                              // ANA offset from zero
  45, 0, 0, 0, 0, 0, 0, 0,                              // ANB

  0, 0, 0, 0, 0, 0, 0, 0,                               // ANA TRUE = invert data
  0, 0, 0, 0, 0, 0, 0, 0,                               // ANB

  10, 500, 500, 1, 50, 50, 50, 50,                      // ANA number of values to average @ 1 msec each
  10, 1, 50, 50, 50, 0, 0, 0,                           // ANB

  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,       // ANA gain scale factor
  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,       // ANB

  0, 0, 0, 0, 400, 0, 0, 400,                           // ANA TRUE = assert event
  0, 0, 0, 0, 0, 0, 0, 0,                               // ANB

  /* Serial data */
  {0},

  /* Temperature data */
  0, 50.0, 5.0, 0, 100.0,                               // OnBoardTemp, OnBoardHiLimit, OnBoardLoLimit, DspTemp. DspLimit
  0, 0,                                                 // Priority, Event

  /* System board interrupt and control data */
  0x7, 0, 0,                                            // Enable, Flags, Local

  /* Communications heartbeat */
  0,                                                    // wHeartbeat milliseconds until shutdown (0 = disabled)

  /* Auxillary */
  0,                                                    // wAuxillary used for blinks and beeps

  /* PROFILE_BUFFER_SIZE word receive buffer */
  {
    4, 0,
    0, 0, 0, 0,

    0, 0x3EC0, 0, 0x3F80,   // 37.5% 1.0 2/1 oscillate profile
    0, 0x3F40, 0, 0x4000,   // 75%   2.0 rev
    0, 0x3F60, 0, 0x3FC0,   // 87.5% 1.5 rev
    0, 0x3F80, 0, 0x3F80    // 100%  1.0 rev
  },
 
  { 0 }
};


/***************************************************************
 Main foreground process handles input configuration and control
 ***************************************************************/
void main(SnBootError eBootError)
{
  if(eBootError) g_tVarArray.tInterrupt.wFlags |= INTERRUPT_BOOT_ANOMALY;

  CopyFlashFuncTable(&g_tFlashFuncTable);
  g_tFlashFuncTable.rCopyCrcTable(g_pbCrcTable);
  
#if UT_TEST != UT_NONE
  g_tVarArray.wPortType = 0x8282;         // turn on brushless ports
  g_tVarArray.tBldcA.tEx = tPowerMax;     // PortA motor initialization
  g_tVarArray.tBldcB.tEx = tPowerMax;     // PortB motor initialization
#else
  g_tVarArray.tBldcA.tEx = tNoMotor;      // PortA no motor initialization
  g_tVarArray.tBldcB.tEx = tNoMotor;      // PortB no motor initialization
#endif

  DSP_Init();                             // initialize MC56F8357

  /* Main loop executes in approximately 1 msec */
  for (;;) {

    /* Refresh 2 second COP watchdog timer */
#if UT_TEST == UT_WATCHDOG
    if (ptGPIOD->wDR & 1) {
      DbgD11OFF;
      Reload_Watchdog;
    } else {
      DbgD11ON;
    }
#else
    Reload_Watchdog;
#endif
    
    /* Process polled inputs */
    GetDigitalInput();
    UpdateAnalog();

    /* Update associated items when control parameters change */
    DeferredFlashCommand();
    DeferredFlexCanActions();
    OnChange();
  }
}