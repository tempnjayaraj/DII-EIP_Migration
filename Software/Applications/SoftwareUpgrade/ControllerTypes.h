#ifndef CONTROLLERTYPES_H
#define CONTROLLERTYPES_H

#include "SnTypes.h"

/***********************************************************
 Declare common enumerations and controller board structures
 NOTE: this file is shared with the application software
 ***********************************************************/

enum Mode {MODE_OFF, MODE_REVERSE, MODE_FORWARD, MODE_OSCILLATE, MODE_POSITION,
           MODE_JOG, MODE_LOCK, MODE_STOP, MODE_DIAGNOSE, MODE_CONT};

/* Lower level fault definitions */
enum Errors {
  ERROR_MOTOR_TAC             = 0x0001,   // brushless motors
  ERROR_MOTOR_STALL           = 0x0002,
  ERROR_MOTOR_IHIGH           = 0x0004,
  ERROR_MOTOR_OVERFLOW        = 0x0008,
  ERROR_MOTOR_ILIM            = 0x0010,
  ERROR_MOTOR_TLIM            = 0x0020,
  ERROR_SHORT_24              = 0x0040,
  ERROR_MOTOR_TAC_NOISE       = 0x0080,
  ERROR_MOTOR_SET_SPEED       = 0x0100,
  ERROR_MOTOR_INT_SAT         = 0x0200,
  ERROR_MOTOR_DIRECTION       = 0x0400,

  ERROR_HIGH_TEMP             = 0x0800,   // temperature sensors
  ERROR_LOW_TEMP              = 0x1000,
  ERROR_DSP_TEMP              = 0x2000,

  ERROR_DIGITAL_DEBOUNCE      = 0x4000    // digital input debounce overflow
};

/* Position profile control bits */
enum ProfileControl {
  PCON_RESET         = 0x0010,
  PCON_HALT          = 0x0020,
  PCON_START         = 0x0040,
  PCON_STEP          = 0x0080,
  PCON_PROFILE       = 0x0100,
  PCON_DIR_REVERSE   = 0x0200,
  PCON_RX_DATA_READY = 0x1000,
  PCON_PROFILE_READY = 0x2000,
  PCON_REDRAW        = 0x4000,
  PCON_ZERO          = 0x8000
};

/* Time and position coordinates for motor move */
typedef struct {
	float X;                    // % time coordinate
	float Y;                    // revolution distance coordinate
} Point;

/* Time and position profile for motor move */
typedef struct {
	SnWord Count;
	SnWord Fill;
	Point P[2];                 // number for stop profile
} Profile;

/* Time and position profile for motor move */
typedef struct {
	SnWord Count;
	SnWord Fill;
	Point P[16];                // number for normal profile
} MotorProfile;

/* Control loop gain coefficients */
typedef struct {
  float     fKf;              // feed forward coefficient
  float     fKp;              // proportional coefficient for ei
  float     fKi;              // integral coefficient for  ei
  float     fKd;              // derivative coefficient for ei - ei-1
  float     fKa;              // acceleration coefficient
  float     fKl;              // load compensation coefficient
} Kgain;

/* Brushless motors */
typedef struct {
  // initialization write once table values
  SnWord    wFCommTable[8];   // CW commutation pattern
  SnWord    wRCommTable[8];   // CCW commutation pattern
  SnWord    wFDirTable[8];    // test values for clockwise direction
  SnWord    wRDirTable[8];    // test values for counter clockwise direction
  SnWord    wJogTable[6];     // CW synchronous commutation pattern table
  float     fResistance;      // motor winding resistance in ohms
  float     fAccCvrt;         // converts armature acceleration to amps (pi/30*j*grbx/kt)
  SnWord    wTacPerRevN;      // motor electrical state changes per revolution numerator (poles*phases*grbx)
  SnWord    wTacPerRevD;      // motor electrical state changes per revolution denominator (grbx)
  float     fAccel;           // output acceleration in rpm/sec
  float     fDecel;           // output deceleration in rpm/sec
  SnSWord   sVelMax;          // maximum allowable velocity command
  float     fStopTime;        // time to decelerate to stop
  float     fLoopRate;        // PID timer interrupt period (microseconds)
  float     fPWMFreq;         // PWM frequency (kHz)
  Kgain     tKvel;            // velocity loop coefficients
  Kgain     tKpos;            // position loop coefficients
  float     fFrpmA;           // f(rpm) first order coefficient (pwmcm/nls)
  SnWord    wFrpmB;           // f(rpm) offset (PWM)
  float     fTlimA;           // linear torque limit coefficient
  SnWord    wTlimB;           // torque limit offset (PWM)
  float     fIlim;            // current limit (amp)
  float     fIhigh;           // high current value (75% EIP measured stall current)
  // external run-time application write and/or read user variables
  SnWord    wMode;            // off / reverse / forward / oscillate
  SnSWord   sVelSet;          // signed welocity target in rpm
  SnWord    wDwell;           // oscillate mode dwell period in milliseconds
  float     fCycleTime;       // position cycle period in seconds
  SnWord    wProfileCmd;      // control bits for position profiling
  SnWord    wAssert;          // system interrupt mask
  SnWord    wFault;           // error conditions
} External;

typedef struct {
  // internal DSP port specific utility readable variables
  SnSWord   sOverload;        // overload level
  float     fCurrent;         // motor current in amps
  float     fVelAct;          // actual velocity
  SnWord    wVelAbs;          // integer absolute value of actual velocity
  SnWord    wLastTac;         // direction tracking data
  float     fTacPerRev;       // motor electrical state changes per revolution (poles*phases*grbx)
  float     fVelCmd;          // velocity command from main controller
  float     fVelTar;          // profiled velocity target based on acceleration
  SnWord    wDirAct;          // actual direction
  SnWord    wDirCmd;          // commanded direction MODE_FORWARD MODE_REVERSE
  SnSQByte  lTacCnt;          // sum of commutations that occurred (position)
  SnSQByte  lTacCntLast;      // saved lTacCnt by velocity routine
  SnSQByte  lEndCnt;          // end position of profile
  float     fAccAmp;          // current due to acceleration
  SnWord    wCtrl;            // brushless motor control bits
  SnWord    wCommIndex;       // index to commutation array
  SnWord    wJogIndex;        // index to synchronous commutation array
  float     fProLast;         // last proportional error for derivative
  float     fIntTerm;         // integral velocity term
  float     fDerTerm;         // derivative velocity term
  SnWord    wDwellCnt;        // oscillate counter
  SnWord    wDirFaultCtr;     // direction test counter
  SnSWord   sDirFaultCnt;     // direction test count
  SnWord    wMilliSec;        // millisecond timer
  SnSWord   sWinLock;         // output shaft position
  SnWord    wProIndex;        // index for profile points
  SnWord    wSeqIndex;        // index for dt steps through sequences
  SnWord    wSeqCount;        // number of dt steps in sequence
  float     fdT;              // sequence time in seconds
  float     fdS;              // dequence distance in tac counts
  float     fT;               // time from beginning of sequence
  float     fS;               // position relative to beginning of sequence
  float     fV;               // velocity at time fT
  float     fA;               // sequence acceleration
  Profile   tStop;            // 2 point stop profile
} Internal;

typedef struct {
  External  tEx;              // motor variables with external application access
  Internal  tIn;              // motor variables with internal DSP use only
  MotorProfile tMotorProfile;
} Async;

/* Hall bus for legacy shaver and footswitch data */
typedef struct {
  SnWord  wDeviceExist;       // hall devices present in corresponding bit pattern
  SnWord  wDeviceActive;      // activated hall devices
  SnWord  wDeviceLatch;       // push-on / push-off state data
  SnWord  wDeviceAssert;      // devices that assert system interrupts
  SnWord  wHallBusVq;         // bus quiescent voltage during reset
  SnWord  wDeltaCnt;          // incremented for every hall change
} HallBus;

// Hall device button status bits  - 1 indicates button depressed
#define HALL_MDU_CTL_REV            0x0004
#define HALL_MDU_CTL_FWD            0x0008
#define HALL_MDU_CTL_OSC            0x0010
#define HALL_MDU_CTL_MASK           0x001C

#define HALL_FOOT_CTL_WINDOW_LOCK	0x0040

/* Serial Ports for new shaver and footswitch data */
typedef struct {
  SnWord  wNumCmds;
  SnWord  wCmdResult[16];     // List of response words to the cmd requests
  SnWord  wRcvErrCnt0;        // Number of Sci 0 Receive Errors since last clear
  SnWord  wRcvErrCnt1;        // Number of Sci 1 Receive Errors since last clear
} Serial;

/* DSP digital inputs */
typedef struct {
  SnWord  wStateData[6];      // processed input state data
  SnWord  wNewData[6];        // debouncing momentary status
  SnWord  wOldData[6];        // last new data
  SnWord  wActive[6];         // 1 = active digital input
  SnWord  wAssert[6];         // 1 = assert event
  SnWord  wActiveLow[6];      // 1 = active low input (inverts input)
  SnWord  wInputType[6];      // 1 = push on / push off
  SnWord  wDebounce[6];       // number of consecutive identical reads
  SnWord  wEvent;             // events and errors
} DigitalIn;

/* DSP analog inputs */
typedef struct {
  SnSWord sData[16];          // raw adc data
  SnSWord sAverage[16];       // prescaled average input data
  SnSWord sOffset[16];        // offset from zero
  SnWord  wInvert[16];        // TRUE = negate value - offset
  SnWord  wCount[16];         // number of values to average
  float   fGain[16];          // gain for 4095 max value
  SnWord  wAssert[16];        // 1 = assert interrupt
} AnalogIn;

/* Temperature */
typedef struct {
  float   fOnBoardTemp;       // LM20 sensor
  float   fOnBoardHiLimit;    // fault high temp trigger limit
  float   fOnBoardLoLimit;    // fault low temp trigger limit
  float   fDspTemp;           // internal uncalibrated DSP sensor
  float   fDspLimit;          // fault trigger limit
  SnWord  wPriority;          // 1 = raise event
  SnWord  wEvent;             // events and errors
} Temperature;

/* Interrupt control flags */
typedef struct {
  SnWord  wEnable;            // 1 = interrupt enabled
  SnWord  wFlags;             // fault and state data interrupt flag
  SnWord  wLocal;             // enable local control
} SysInterrupt;

#define PROFILE_BUFFER_SIZE	52

/*
 * Handpiece Serial Number
 */
 
// 10 character alphanumeric null terminated string (11 bytes total)
#define SERIAL_NUMBER_XFR   11

// Rounded to 12 bytes to conform to the control structure 2 byte min
#define SERIAL_NUMBER_STORE 12 

/* Status and control structure */
typedef struct {
  SnQByte       qVersion;     // 32 bit version information
  SnWord        wPortType;
  Async         tBldcA;
  Async         tBldcB;
  HallBus       tHallBusA;
  HallBus       tHallBusB;
  DigitalIn     tDigital;
  AnalogIn      tAnalog;
  Serial        tSerial;
  Temperature   tTemperature;
  SysInterrupt  tInterrupt;
  SnWord        wHeartCount;
  SnWord        wAuxillary;
  SnWord        wBuffer[PROFILE_BUFFER_SIZE];
  SnByte        pbSerialNumberA[SERIAL_NUMBER_STORE];
  SnByte        pbSerialNumberB[SERIAL_NUMBER_STORE];
} Status_Control;

#endif  // CONTROLLERTYPES_H
