#ifndef MOTORDATA_H
#define MOTORDATA_H

#include "ControllerTypes.h"

/***************************************************************
 Bldc motor structures
 NOTE: this file is shared with the application software
 ***************************************************************/

const External tNoMotor = {
  /* No Motor */                                        /* Bldc application initialization values */ 
  0x3F00, 0x3F00, 0x3F00, 0x3F00, 0x3F00, 0x3F00, 0x3F00, 0x3F00,     // FCommTable[]
  0x3F00, 0x3F00, 0x3F00, 0x3F00, 0x3F00, 0x3F00, 0x3F00, 0x3F00,     // RCommTable[]
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     // FDirTable[]
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     // RDirTable[]
  0, 0, 0, 0, 0, 0,                                     // JogTable[]
  0.0f, 0.0f, 1,                                        // Resistance, AccCvrt, TacPerRev
  0.0f, 0.0f, 0,                                        // Accel, Decel, VelMax
  1.0f, 5E-3f,                                          // StopTime, LoopRate
  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                   // Kf, Kp, Ki, Kd, Ka, Kl for velocity
  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                   // Kf, Kp, Ki, Kd, Ka, Kl for position
  0.0f, 0,                                              // f(RPM)A-B
  0.0f, 0, 0.0f, 0.0f,                                  // TlimA, TlimB, Ilim, Ihigh

                                                        /* Run-time application writeable variables */
  0, 0, 0, 1.0f,                                        // Mode, VelSet, Dwell, CycleTime
  0x3000, 0, 0                                          // ProfileCmd, Assert, Faults
};

const External tPowerMax = {
  /* Gearmotor: Phase=3 Poles=10 Ke=2.03E-3, Kt=2.47, J=56.2E-6 */
  0x3F00, 0x9F01, 0xBD04, 0x9F04, 0xB710, 0xB701, 0xBD10, 0x3F00,     // FCommTable[]
  0x3F00, 0xBD10, 0xB701, 0xB710, 0x9F04, 0xBD04, 0x9F01, 0x3F00,     // RCommTable[]
  0x0000, 0x0003, 0x0006, 0x0002, 0x0005, 0x0001, 0x0004, 0x0000,     // FDirTable[]
  0x0000, 0x0005, 0x0003, 0x0001, 0x0006, 0x0004, 0x0002, 0x0000,     // RDirTable[]
  4, 6, 2, 3, 1, 5,                                     // JogTable[]
  1.31f, 8.2E-6f, 30,                                   // Resistance (Rm+Rs), AccCvrt, TacPerRev
  60000.0f, -60000.0f, 10000,                           // Accel, Decel, VelMax
  0.25f, 5E-3f,                                         // StopTime, LoopRate
  0.0f, 0.05f, 0.05f, 0.05f, 1.0f, 0.0f,                // Kf, Kp, Ki, Kd, Ka, Kl for velocity
  0.0f, 25.0f, 2.5f, 0.5f, 0.0f, 0.0f,                  // Kf, Kp, Ki, Kd, Ka, Kl for position
  0.102f, 5,                                            // f(RPM)A-B
  6.25E-2f, 850, 20.0f, 7.5f,                           // TlimA, TlimB, Ilim, Ihigh

                                                        /* Run-time application writeable variables */
  0, 1000, 80, 0.10f,                                   // Mode, VelSet, Dwell, CycleTime
  0x3000, 0, 0                                          // ProfileCmd, Assert, Faults
};

const External tSuperMax = {
  /* Gearmotor: Phase=3 Poles=10 Ke=1.30E-3, Kt=1.76, J=56.2E-6 */ 
  0x3F00, 0x9F01, 0xBD04, 0x9F04, 0xB710, 0xB701, 0xBD10, 0x3F00,     // FCommTable[]
  0x3F00, 0xBD10, 0xB701, 0xB710, 0x9F04, 0xBD04, 0x9F01, 0x3F00,     // RCommTable[]
  0x0000, 0x0003, 0x0006, 0x0002, 0x0005, 0x0001, 0x0004, 0x0000,     // FDirTable[]
  0x0000, 0x0005, 0x0003, 0x0001, 0x0006, 0x0004, 0x0002, 0x0000,     // RDirTable[]
  4, 6, 2, 3, 1, 5,                                     // JogTable[]
  1.02f, 3.34E-6f, 30,                                  // Resistance (Rm+Rs), AccCvrt, TacPerRev
  60000.0f, -60000.0f, 10000,                           // Accel, Decel, VelMax
  0.25f, 5E-3f,                                         // StopTime, LoopRate
  0.0f, 0.005f, 0.015f, 0.0f, 0.0f, 0.0f,               // Kf, Kp, Ki, Kd, Ka, Kl for velocity
  0.0f, 4.0f, 0.01f, 1.0f, 0.0f, 0.0f,                  // Kf, Kp, Ki, Kd, Ka, Kl for position
  0.06f, 5,                                             // f(RPM)A-B
  0.0, 1200, 30.0f, 18.8f,                              // TlimA, TlimB, Ilim, Ihigh

                                                        /* Run-time application writeable variables */
  0, 1000, 80, 0.10f,                                   // Mode, VelSet, Dwell, CycleTime
  0x3000, 0, 0                                          // ProfileCmd, Assert, Faults
};

const External tHiTorque = {
  /* Motor: Ke=.78E-3, Kt=.957, J=30E-6, Grbx=5.0 */    /* Bldc application initialization values */ 
  0x3F00, 0x9F01, 0xBD04, 0x9F04, 0xB710, 0xB701, 0xBD10, 0x3F00,     // FCommTable[]
  0x3F00, 0xBD10, 0xB701, 0xB710, 0x9F04, 0xBD04, 0x9F01, 0x3F00,     // RCommTable[]
  0x0000, 0x0003, 0x0006, 0x0002, 0x0005, 0x0001, 0x0004, 0x0000,     // FDirTable[]
  0x0000, 0x0005, 0x0003, 0x0001, 0x0006, 0x0004, 0x0002, 0x0000,     // RDirTable[]
  4, 6, 2, 3, 1, 5,                                     // JogTable[]
  0.65f, 16.4E-6f, 60,                                  // Resistance, AccCvrt, TacPerRev
  60000.0f, -60000.0f, 5000,                            // Accel, Decel, VelMax
  0.50f, 5E-3f,                                         // StopTime, LoopRate
  0.0f, 0.1f, 0.1f, 0.1f, 1.0f, 0.0f,                   // Kf, Kp, Ki, Kd, Ka, Kl for velocity
  0.0f, 15.0f, 0.5f, 2.5f, 0.0f, 0.0f,                  // Kf, Kp, Ki, Kd, Ka, Kl for position
  0.195f, 125,                                          // f(RPM)A-B
  1.65E-1f, 480, 10.0f, 2.3f,                           // TlimA, TlimB, Ilim, Ihigh

                                                        /* Run-time application writeable variables */
  0, 1000, 80, 0.25f,                                   // Mode, VelSet, Dwell, CycleTime
  0x3000, 0, 0                                          // ProfileCmd, Assert, Faults
};

const External tMini = {
  /* Motor: Ke=.84E-3, Kt=1.14, J=5E-6, Grbx=5.0 */     /* Bldc application initialization values */ 
  0x3F00, 0x9F01, 0xBD04, 0x9F04, 0xB710, 0xB701, 0xBD10, 0x3F00,     // FCommTable[]
  0x3F00, 0xBD10, 0xB701, 0xB710, 0x9F04, 0xBD04, 0x9F01, 0x3F00,     // RCommTable[]
  0x0000, 0x0005, 0x0003, 0x0001, 0x0006, 0x0004, 0x0002, 0x0000,     // FDirTable[]
  0x0000, 0x0003, 0x0006, 0x0002, 0x0005, 0x0001, 0x0004, 0x0000,     // RDirTable[]
  5, 1, 3, 2, 6, 4,                                     // JogTable[]
  12.0f, 2.30E-6f, 60,                                  // Resistance, AccCvrt, TacPerRev
  60000.0f, -60000.0f, 3500,                            // Accel, Decel, VelMax
  0.25f, 5E-3f,                                         // StopTime, LoopRate
  0.0f, 0.1f, 0.1f, 0.1f, 1.0f, 0.0f,                   // Kf, Kp, Ki, Kd, Ka, Kl for velocity
  0.0f, 25.0f, 2.5f, 0.5f, 0.0f, 0.0f,                  // Kf, Kp, Ki, Kd, Ka, Kl for position
  0.21f, 120,                                           // f(RPM)A-B
  0.0f, 1200, 5.0f, 1.2f,                               // TlimA, TlimB, Ilim, Ihigh

                                                        /* Run-time application writeable variables */
  0, 1000, 80, 0.25f,                                   // Mode, VelSet, Dwell, CycleTime
  0x3000, 0, 0                                          // ProfileCmd, Assert, Faults
};

const External tPowerMini = {
  /* Motor: Ke=.450E-3, Kt=.61, J=6E-6, Grbx=4.0 */     /* Bldc application initialization values */ 
  0x3F00, 0x9F01, 0xBD04, 0x9F04, 0xB710, 0xB701, 0xBD10, 0x3F00,     // FCommTable[]
  0x3F00, 0xBD10, 0xB701, 0xB710, 0x9F04, 0xBD04, 0x9F01, 0x3F00,     // RCommTable[]
  0x0000, 0x0005, 0x0003, 0x0001, 0x0006, 0x0004, 0x0002, 0x0000,     // FDirTable[]
  0x0000, 0x0003, 0x0006, 0x0002, 0x0005, 0x0001, 0x0004, 0x0000,     // RDirTable[]
  5, 1, 3, 2, 6, 4,                                     // JogTable[]
  1.05f, 4.12E-6f, 48,                                  // Resistance, AccCvrt, TacPerRev
  50000.0f, -50000.0f, 6000,                            // Accel, Decel, VelMax
  0.25f, 5E-3f,                                         // StopTime, LoopRate
  0.0f, 0.04f, 0.04f, 0.0f, 0.0f, 0.0f,                 // Kf, Kp, Ki, Kd, Ka, Kl for velocity
  0.0f, 15.0f, 2.5f, 15.0f, 0.0f, 0.0f,                 // Kf, Kp, Ki, Kd, Ka, Kl for position
  0.090f, 27,                                           // f(RPM)A-B
  0.0f, 1200, 20.0f, 9.8f,                              // TlimA, TlimB, Ilim, Ihigh

                                                        /* Run-time application writeable variables */
  0, 1000, 80, 0.10f,                                   // Mode, VelSet, Dwell, CycleTime
  0x3000, 0, 0                                          // ProfileCmd, Assert, Faults
};

const External tDrill = {
  /* Motor: Grbx=18.16667  */                           /* Bldc application initialization values */ 
  0x3F00, 0x9F01, 0xBD04, 0x9F04, 0xB710, 0xB701, 0xBD10, 0x3F00,     // FCommTable[]
  0x3F00, 0xBD10, 0xB701, 0xB710, 0x9F04, 0xBD04, 0x9F01, 0x3F00,     // RCommTable[]
  0x0000, 0x0003, 0x0006, 0x0002, 0x0005, 0x0001, 0x0004, 0x0000,     // FDirTable[]
  0x0000, 0x0005, 0x0003, 0x0001, 0x0006, 0x0004, 0x0002, 0x0000,     // RDirTable[]
  4, 6, 2, 3, 1, 5,                                     // JogTable[]
  2.56f, 3.65E-5f, 218,                                 // Resistance, AccCvrt, TacPerRev
  16514.0f, -16514.0f, 2000,                            // Accel, Decel, VelMax
  0.5f, 5E-3f,                                          // StopTime, LoopRate
  0.0f, 0.363f, 0.363f, 0.363f, 1.0f, 0.0f,             // Kf, Kp, Ki, Kd, Ka, Kl for velocity
  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                   // Kf, Kp, Ki, Kd, Ka, Kl for position
  0.765f, 120,                                          // f(RPM)A-B
  0.0f, 1200, 20.0f, 12.8f,                             // TlimA, TlimB, Ilim, Ihigh

                                                        /* Run-time application writeable variables */
  0, 0, 80, 1.0f,                                       // Mode, VelSet, Dwell, CycleTime
  0x3000, 0, 0                                          // ProfileCmd, Assert, Faults
};

const External tSaw = {
  /* Motor: Grbx=1 */                                   /* Bldc application initialization values */ 
  0x3F00, 0x9F01, 0xBD04, 0x9F04, 0xB710, 0xB701, 0xBD10, 0x3F00,     // FCommTable[]
  0x3F00, 0xBD10, 0xB701, 0xB710, 0x9F04, 0xBD04, 0x9F01, 0x3F00,     // RCommTable[]
  0x0000, 0x0003, 0x0006, 0x0002, 0x0005, 0x0001, 0x0004, 0x0000,     // FDirTable[]
  0x0000, 0x0005, 0x0003, 0x0001, 0x0006, 0x0004, 0x0002, 0x0000,     // RDirTable[]
  4, 6, 2, 3, 1, 5,                                     // JogTable[]
  2.56f, 1.99E-6f, 12,                                  // Resistance, AccCvrt, TacPerRev
  300000.0f, -300000.0f, 32000,                         // Accel, Decel, VelMax
  0.5f, 5E-3f,                                          // StopTime, LoopRate
  0.0f, 0.02f, 0.02f, 0.02f, 1.0f, 0.0f,                // Kf, Kp, Ki, Kd, Ka, Kl for velocity
  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                   // Kf, Kp, Ki, Kd, Ka, Kl for position
  0.033f, 40,                                           // f(RPM)A-B
  0.0f, 1200, 20.0f, 12.8f,                             // TlimA, TlimB, Ilim, Ihigh

                                                        /* Run-time application writeable variables */
  0, 0, 80, 1.0f,                                       // Mode, VelSet, Dwell, CycleTime
  0x3000, 0, 0                                          // ProfileCmd, Assert, Faults
};

const External tFactoryTest = {
  /* Factory Test Mode */                               /* Bldc application initialization values */ 
  0x3F00, 0x3F00, 0x3F00, 0x3F00, 0x3F00, 0x3F00, 0x3F00, 0x3F00,     // FCommTable[]
  0x3F00, 0x3F00, 0x3F00, 0x3F00, 0x3F00, 0x3F00, 0x3F00, 0x3F00,     // RCommTable[]
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     // FDirTable[]
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,     // RDirTable[]
  0, 0, 0, 0, 0, 0,                                     // JogTable[]
  0.0f, 0.0f, 30,                                       // Resistance, AccCvrt, TacPerRev
  20000.0f, 0.0f, 10000,                                // Accel, Decel, VelMax
  1.0f, 5E-3f,                                          // StopTime, LoopRate
  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                   // Kf, Kp, Ki, Kd, Ka, Kl for velocity
  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,                   // Kf, Kp, Ki, Kd, Ka, Kl for position
  1.0f, 0,                                              // f(RPM)A-B
  0.0f, 1200, 2.0f, 1.0f,                               // TlimA, TlimB, Ilim, Ihigh

                                                        /* Run-time application writeable variables */
  0, 1200, 0, 1.0f,                                     // Mode, VelSet, Dwell, CycleTime
  0x3000, 0, 0                                          // ProfileCmd, Assert, Faults
};

#endif  // MOTORDATA_H
