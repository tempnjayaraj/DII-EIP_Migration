#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "MC56F8357.h"
#include "ControllerTypes.h"
#include "MotorXfrData.h"
#include "SnFlash.h"
#include "SnBoot.h"

/*
  Revision history
  Version	  Date		    Description
  4.1.0.0	  KK080104	  Strip out unnecessary DII code
  4.1.0.1	  KK080909	  Release for preliminary testing
  4.1.1.0   KK080917    Changed communication from SPI to parallel
  4.1.1.1   KK080929    Tuned gains
  4.1.1.2   KK081007    Tuned gains and WindowLock stop
  4.1.1.3   KW090128    Added wrong direction fault, noise filter for tacs, REDIAGNOSE,
                        jump from MODE_LOCK directly to MODE_JOG mode, variable timeouts for
                        serial cmds, faster scan and update of analog values.
  4.1.1.4   KK090219    Re-integration with Ken Krause code base.
  4.1.2.0   KK090316    Reorganized Async motor data structure, share MotorData.h
  4.1.2.1   KW090423    Changed PortF bits 0-15 to be open-drain output.
  4.1.2.2   KW090611    Check for valid 8 bit Serial Page command, (0x9c), not just 0xc.
                        Fixed a race condition for the RS485 timeout counter.
  4.1.2.3   KW090708    Changed HallPeriod to 0.5ms tick and debounce of 4, removed FIXME's,
                        Added more error checking to Msg layer.
  4.1.2.4   KW090811    Fixed race condition in Msg layer.
  4.1.2.5	  DT090824	  Fixed Analog Footpedal Window Lock affecting latched state of MDU buttons.
  4.1.2.6	  DT090831	  Updated motor tuning values for PowerMax,PowerMini, Drill and Saw to match DII
  4.1.2.7	  DT090831	  Profile data is fixed size
  4.1.2.8   KK090910    Added IHIGH motor flag, tuned current and torque limits
  4.1.2.9   DT090911    Corrected PowerMax Elite TlimA to reduce maximum motor torque to <35oz-in
  4.1.2.10  DT090917    Motor Current Limit flag is now a latching flag
  4.1.2.11  DT101216    Calculate Absolute Velocity as an average over 60 ms to smooth
  4.1.2.12  KW180420    Changed current flow to PWM Top FETs instead of Bottom FETs (reduces effect of cable short)
                        Added support for the MDU to provide the motor parameter table for the DII
                        Added support for non-integer gear ratio motors (including Window Lock)
                        Added support for PWM frequency adjustable via the motor parameter table
                        Changed JOG to use the position fKp instead of the constant 25 for a multiplier
  4.1.2.13  KW190612    Skip over version as this was used for the Intellio Link Test Tool
  4.1.2.14  KW190612    Added support for the MDU to choose either 0.25f or 0.5f for fStopTime
  4.1.3.00  KW190617    Changed Serial Number Request byte from 0xFF to 0xAA.
  4.2.0.00  KW190809    Bumped Major version number in preperation for release
  4.2.0.01  KW190905    Added special code that can be compiled in for verifcation.
                        The code is not present in the release.
  4.2.99.00 DT201019    Change software version for manufacturing build.
  4.3.00.00 DT201030	Change software version to 3.00 for DII BF project
*/

/*******************************************************
 Declare motor controller board version information
 *******************************************************/

#define MCB_APPTYPE 4         // must match MOTOR_HDR in build upgrade tool
#define MCB_MAJOR   3         // hardware revision level
#define MCB_MINOR   00
#define MCB_BUILD   00

/* Create app version number from components */
#define MAKE_VERSION \
    (SnQByte)MCB_APPTYPE << 24 | \
    (SnQByte)MCB_MAJOR << 16 | \
    (SnQByte)MCB_MINOR << 8 | \
    (SnQByte)MCB_BUILD

/*******************************************************
 Constant definitions
 *******************************************************/

#define FALSE 0
#define TRUE 1
#define REV_TO_RAD 6.28318530718        // revolutions to radians: 2 * Pi
#define AMP_CONVERT 6.44688644689E-3    // motor current conversion 26.4 / 4095
#define ILIM_CONVERT 2482.42424242      // motor current limit conversion 65536 / 26.4
#define VCC 24.0                        // motor supply voltage
#define PWM_TOP_BOT_OFF 0xAA00          // short circuit test FET configuration
#define FETS_OFF 0x3F00                 // turn off all FETs
#define FWD 0                           // forward commutation offset
#define REV 8                           // reverse commutation offset
#define CLR_SERIAL_TIMEOUT  0xFFFF      // clear the serial timeout counter

/*******************************************************
 Verification Test definitions
 *******************************************************/
/*
 These defines are used to enable special code that is
 used in the verification. Only one define should
 be enabled at a time and none of these flags should be enabled
 for production code.
*/
#define UT_NONE           0   // Disable Unit Tests
#define UT_WATCHDOG       1   // Force a Watchdog Reset
#define UT_HALL_TIMER     2   // 500us Interval HallBus Timer
#define UT_CTRL_A_TIMER   3   // 5ms Port A Motor Control Timer
#define UT_CTRL_B_TIMER   4   // 5ms Port B Motor Control Timer
#define UT_DIAGNOSE       5   // Used for timing short circuit test
#define UT_DEFAULT        6   // Used for all the rest of the tests

// Set to one of the above UT defines
#define UT_TEST   UT_NONE

/*******************************************************
 Macro definitions
 *******************************************************/

/* Port pin macros */
#define DbgD11ON  ptGPIOD->wDR |=  0x0800
#define DbgD11OFF ptGPIOD->wDR &= ~0x0800
#define DbgD11TOG ptGPIOD->wDR ^=  0x0800

/* System board interrupt assert macro */
#define SysInt  ptGPIOA->wDR ^=  0x0200

/* Port pin macros */
#define BeepON  ptGPIOD->wDR |=  0x1000
#define BeepOFF ptGPIOD->wDR &= ~0x1000
#define BeepTOG ptGPIOD->wDR ^=  0x1000

/* Reload COP watchdog timer */
#define Reload_Watchdog {   \
  ptCOP->wCOPCTR = 0x5555;  \
  ptCOP->wCOPCTR = 0xAAAA;  \
  }

/* Disable commutation timer interrupts */
#define Disable_Commutation_Interrupts  \
  if (ptBldc == &g_tVarArray.tBldcA) {  \
    ptITCN->wIPR[7] &= ~0xC000;         \
    ptITCN->wIPR[8] &= ~0x000F;         \
  }                                     \
  else {                                \
    ptITCN->wIPR[7] &= ~0x0FC0;         \
  }

/* Enable commutation timer interrupts */
#define Enable_Commutation_Interrupts                                           \
  if (!(ptBldc->tEx.wFault & ERROR_SHORT_24) && ptBldc->tEx.wMode != MODE_JOG) {\
    if (ptBldc == &g_tVarArray.tBldcA) {                                        \
      ptITCN->wIPR[8] |= 0x000F;                                                \
      ptITCN->wIPR[7] |= 0xC000;                                                \
    }                                                                           \
    else {                                                                      \
      ptITCN->wIPR[7] |= 0x0FC0;                                                \
    }                                                                           \
  }
/* Reset PWM control variables */
#define ResetPID ptBldc->tIn.lTacCnt = ptBldc->tIn.lTacCntLast = ptBldc->tIn.fProLast = ptBldc->tIn.fDerTerm = ptBldc->tIn.fIntTerm = 0
#define ResetPWM { ptPwm->wVAL[0] = 0; ptPwm->wCTL |= 2; }

enum RetCode {OK, OVERFLOW, DIVIDE_BY_ZERO, NOT_A_TRAPEZOID, UNDEFINED, BUSY};

/*******************************************************
 Interrupt and event flag definitions
 *******************************************************/

/* System level interrupt event definitions */
enum Interrupts {
  INTERRUPT_HALL_BUS_A        = 0x0001,
  INTERRUPT_HALL_BUS_B        = 0x0002,
  INTERRUPT_ANALOG            = 0x0004,
  INTERRUPT_DIGITAL           = 0x0008,

/* System level interrupt fault definitions */
  INTERRUPT_MOTOR_A           = 0x0400,
  INTERRUPT_MOTOR_B           = 0x0800,
  INTERRUPT_ADC_EOC           = 0x1000,
  INTERRUPT_DIGITAL_DEBOUNCE  = 0x2000,
  INTERRUPT_TEMPERATURE       = 0x4000,
  INTERRUPT_BOOT_ANOMALY      = 0x8000
};  

/*******************************************************
 Externs
 *******************************************************/

extern Status_Control g_tVarArray;
extern SnFlashFuncTable g_tFlashFuncTable;
extern SnByte g_pbCrcTable[];
extern SnWord g_wHeartbeat;
extern volatile SnWord g_wLoadSerialTimeout;
extern DIIMotorTblXfr g_tXfrMotorTblA, g_tXfrMotorTblB;
extern SnBool g_yNewXfrMotorTblA, g_yNewXfrMotorTblB;

/*******************************************************
 Function prototypes
 *******************************************************/

/* Interrupt routines */
void COPReset(void);
void StackOverflow(void);
void IllegalInstruction(void);
void MisalignedAccess(void);
void UnexpectedInterrupt(void);

void HallBus_Timer_Interrupt(void);       // read both hall buses
void PortA_Timer_Interrupt(void);         // PID controll algorithms
void PortB_Timer_Interrupt(void);
void TacA0_Interrupt(void);               // motor A hall devices
void TacA1_Interrupt(void);
void TacA2_Interrupt(void);
void TacB0_Interrupt(void);               // motor B hall devices
void TacB1_Interrupt(void);
void TacB2_Interrupt(void);
void Msg_Interrupt(void);

/* Configuration */
void init_MC56F835x_(void);               // startup routine
void DSP_Init(void);                      // initialize DSP registers
void ConfigureMotorPorts(void);           // initialize motor ports A & B per control variable wPortType

/* Process ancillary items */
void UpdateAnalog(void);
void LoadXfrMotorTable(DIIMotorTblXfr *ptXfr, External *ptEx);
void DeferredFlashCommand(void);          // perform deferred Flash command
void OnChange(void);                      // update as necessary when control parameters change

/* Asynchronous motor control routine */
void ControlBldc(Async*, PWM*);
void Commutate(Channel*, PWM*, DigitalPort*, SnWord, Async*);   // commutation routine
void GetActualVelocity(Async*);                                 // get actual velocity from TacCount
float CalculateTargetVelocity(Async*);                          // target velocity profiler
float CalculateTargetPosition(Async*, Profile*, float, float); // calculate move profile position target
Profile* NewProfile(SnWord, SnWord*);                           // create new profile as defined by word array
SnWord DeleteProfile(Profile*);                                 // remove profile from memory
float StopOnPosition(Async*);                                   // stop blade on WindowLock position
SnWord GetPositionProfile(Async*);                              // get brushless motor position profile from main application
SnWord TestForShort(Async*, PWM*);                              // check for phase to ground shorts

/* Analog */
void ReadADC(ADC*, SnSWord*);             // read all 8 ADCs and store in buffer. Result is 12 bit value
void GetAnalogInput(void);                // average and prescale analog inputs

/* Read and debounce digital inputs */
void GetDigitalInput(void);

/* CAN routines */
void FlexCanXmtMsg(SnSWord, SnBool, SnQByte, SnSWord, const SnByte*);
void FlexCanPrepRcvBuf(SnSWord, SnBool);
void FlexCanInit(void);
void DeferredFlexCanActions(void);
void FlexCanReadMsgData(SnSWord, SnSWord, SnByte*);
void FlexCanWriteMsgData(SnSWord, SnSWord, const SnByte*);

/* Serial port routine */
void StartSerialCmdList(volatile SnWord*);
void Sci0_Transmit_Idle_Interrupt(void);
void Sci1_Transmit_Idle_Interrupt(void);
void Sci0_Receive_Full_Interrupt(void);
void Sci1_Receive_Full_Interrupt(void);
void Sci0_Receive_Error_Interrupt(void);
void Sci1_Receive_Error_Interrupt(void);
void SerialInit(void);
void SerialCmdsComplete(SnBool yError);

#endif  // CONTROLLER_H