#if !defined(_MOTORCONTROLCOMMANDS_H)
#define _MOTORCONTROLCOMMANDS_H

#define MAX_RETRIES				3

#define SAVE_NVRAM				1
#define SAVE_FLASH				2

// Battery status
#define     BATTERY_FAILED      0
#define     BATTERY_GOOD        1

// BEEPER
#define BEEPER_ON				0x1
#define BEEPER_OFF				0x0

// Motor Control 
#define		MOTOR_A				1
#define		MOTOR_B				2
#define		MOTOR_OFF			0
#define		MOTOR_REVERSE		1
#define		MOTOR_FORWARD		2
#define		MOTOR_OSCILLATE		3
#define		MOTOR_OSCILLATE_1	3
#define		MOTOR_OSCILLATE_2	4
#define		MOTOR_WINDOW_LOCK	5
#define		MOTOR_OFF_MODE7		7

#define		WINDOW_LOCK_OFF		0
#define		WINDOW_LOCK_ON		1

// Maximum allowable system temperatures
#define MAX_DSP_TEMP			100.0
#define MAX_ONBOARD_TEMP		50.0
#define MIN_ONBOARD_TEMP		5.0

//
// Common Commands
//
#define		INVALID_COMMAND		0

//
// Shaver Commands
//

// Read commands
#define		GET_MC_PORTA_STATUS			100	
#define		GET_MC_PORTB_STATUS			101	
#define		GET_MC_FOOT_TYPE			102
#define		GET_SYSTEM_REVISIONS		103
#define		GET_SYSTEM_LANGUAGE			104
#define		GET_MC_FOOT_STATUS			105
#define		GET_SHAVER_PACKET_CTL		106
#define		GET_SYSTEM_MODE				107
#define     GET_SYSTEM_BATTERY_STATUS   108

//Write commands
#define		SET_SYSTEM_BEEPER			1000
#define		SET_MC_CONTROLS 			1001
#define		SET_MC_PORTA_STATUS			1002
#define		SET_MC_PORTB_STATUS			1003
#define		SET_SYSTEM_LANGUAGE			1004
#define		SET_MC_FOOT_STATUS			1005
#define		SET_SYSTEM_PARAMETERS		1006
#define		SET_MC_WINDOW_LOCK_PORTA	1007
#define		SET_MC_WINDOW_LOCK_PORTB	1008
#define		SET_SHAVER_PACKET_CTL		1009
#define		SET_SYSTEM_MODE				1010
#define     SET_FACTORY_TEST_MODE       1011
#define     SET_MC_BEEPER               1012
#define     SET_PORTA_DEVICE_DATA       1013
#define     SET_PORTB_DEVICE_DATA       1014

// Message Defines
#define     MSG_UPDATE_TITLE					        1
#define     MSG_UPDATE_PORT_STATUS				        2
#define		MSG_UPDATE_MODE_STATUS				        3
#define		MSG_UPDATE_BLADE_STATUS				        4
#define		MSG_UPDATE_FOOT_STATUS				        5
#define		MSG_UPDATE_RUNNING_STATUS			        6
#define		MSG_PORT_MAPPING_STATUS				        7
#define		MSG_UPDATE_REMOTE_PUMP_STATUS		        8
#define		MSG_UPDATE_REMOTE_INTELLIO_SHAVER_STATUS    9
#define	    MSG_REMOTE_DISCONNECTED                     10
#define		MSG_REMOTE_CONNECTED                        11
#define		MSG_PUMP_RUNNING                            12
#define     MSG_PUMP_NOT_RUNNING                        13
#define		MSG_UPDATE_HANDPIECE_DUAL_RPM_ON            14
#define		MSG_UPDATE_HANDPIECE_DUAL_RPM_OFF           15
#define		MSG_UPDATE_LANGUAGE                         16
#define		MSG_UPDATE_SYSTEM_MODE                      17					
#define		MSG_UPDATE_ALL_SETTINGS                     18					


#define     SN_NO_TITLE                 0

#endif