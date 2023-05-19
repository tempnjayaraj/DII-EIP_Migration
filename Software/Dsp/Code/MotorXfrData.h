#ifndef MOTORXFRDATA_H
#define MOTORXFRDATA_H

#include "SnTypes.h"

/*
 * bConfig defines
 */
#define MT_CONF_INV_FLOW    0x01    // Inverted Current Flow (CCW motion facing shaft)
                                    //  0=A_C, B_A, B_C, C_B, A_B, C_A
                                    //  1=C_A, A_B, C_B, B_C, B_A, A_C

#define MT_CONF_INV_HALL    0x02    // Inverted Hall Sequence (CCW motion facing shaft) 
                                    //  0=5, 1, 3, 2, 6, 4
                                    //  1=4, 6, 2, 3, 1, 5)

#define MT_CONF_K_ACCEL     0x04    // Defines fKa value
                                    // 0=fKa set to 0.0f
                                    // 1=fKa set to 1.0f

#define MT_CONF_STOP_TIME   0x08    // Defines fStopTime value
                                    // 0=fStopTime set to 0.25f
                                    // 1=fStopTime set to 0.5f

/*
 * Represent a float by 12 bits of base (0-4095) and 4 bits of negative exponent (0-15).
 *
 * Example: 0.000456 or 4.56E-4 is represented by WFLOAT(456, 6)
 */
#define WFLOAT(base, nexp)  (SnWord)(nexp << 12 | base)

typedef struct {
  SnWord    wTacPerRevN;    // Numerator for TacsPerRev
  SnByte    bTacPerRevD;    // Denominator for TacsPerRev 
  SnByte    bConfig;        // -- (See bConfig defines above) --
  SnWord    wAccDecel;      // Output acceleration/deceleration in rpm/sec
  SnWord    wResistance;    // Total resistance (motor + cable) in ohms
  SnWord    wAccCvrt;       // Converts armature acceleration to amps (pi/30*j*grbx/kt)
  SnWord    wVelMax;        // Maximum allowable velocity command
  SnByte    bLoopPeriod;    // PID timer interrupt interval in milliseconds
  SnByte    bPWMFreq;       // PWM frequency in kHz
  SnWord    wVelKp;         // Velocity Proportional coefficient for ei
  SnWord    wVelKi;         // Velocity Integral coefficient for  ei
  SnWord    wVelKd;         // Velocity Derivative coefficient for ei - ei-1
  SnWord    wPosKp;         // Position Proportional coefficient for ei
  SnWord    wPosKi;         // Position Integral coefficient for  ei
  SnWord    wPosKd;         // Position Derivative coefficient for ei - ei-1
  SnWord    wTlimA;         // Linear torque limit coefficient
  SnWord    wTlimB;         // Torque limit offset (PWM)
  SnWord    wIlim;          // Current limit (amp)
  SnWord    wIhigh;         // High current value (75% EIP measured stall current)
} DIIMotorTblXfr;

#endif  // MOTORXFRDATA_H
