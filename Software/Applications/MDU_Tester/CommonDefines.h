#ifndef _COMMONDEFINES_H
#define _COMMONDEFINES_H


#define THREAD_TERMINATION_WAIT		1000

#define LANGUAGE_ENGLISH			0
#define LANGUAGE_GERMAN				1
#define LANGUAGE_ITALIAN			2
#define LANGUAGE_SPANISH			3
#define LANGUAGE_FRENCH				4
#define LANGUAGE_DANISH				5
#define LANGUAGE_DUTCH				6
#define LANGUAGE_NORWEGIAN			7
#define LANGUAGE_PORTUGUESE			8
#define LANGUAGE_SWEDISH			9			

#define TMP_BUF_SIZE				1024

//
// Shaver stuff
//

#define DEFAULT_MODE			0
#define CUSTOM_MODE				1

#define UP						2
#define DOWN					1

#define FILE_NAME_RESETSN		_T("\\Hard Disk\\ResetSerialNumber.txt")
#define FILE_NAME_TEST			_T("\\Hard Disk\\TestMode.txt")
#define FILE_NAME_FACTORY		_T("\\Hard Disk\\FactoryMode.txt")
#define FILE_NAME_SERVICE		_T("\\Hard Disk\\ServiceMode.txt")

#define FILE_NAME_RELIANT_LONG_SERIAL_NUMBER    _T("\\Hard Disk\\ReliantLongSerialNumber.txt")       

#define FACTORY_MODE	1
#define TEST_MODE		2
#define SERVICE_MODE    3
#define RESET_SN_MODE   4

#define KEY_DOWN_TIME			1000	// milliseconds
#define KEY_DOWN_REPEAT_RATE	50		// milliseconds

#define OSC_MODE1				1	
#define OSC_MODE2				2

#define PROFILE_1				1
#define PROFILE_2				2

#define BLADE_TYPE_STRAIGHT		0
#define BLADE_TYPE_CURVED		1
#define BLADE_TYPE_BUR			2
#define BLADE_TYPE_FAST_BUR		3
#define	BLADE_TYPE_OTHER		4
#define BLADE_TYPE_UNKNOWN		25

#define BLADE_ID_STRAIGHT		0x10	// Straight i.e. Medium Speed Blade
#define BLADE_ID_CURVED			0x11	// Curved i.e. Low Speed Blade
#define BLADE_ID_BURR			0x12	// Burr i.e. High Speed Blade
#define BLADE_ID_FAST_BURR		0x13	// High Speed Burr i.e. Highest Speed Blade
#define BLADE_ID_STRAIGHT_HT	0x14	// Straight i.e. Medium Speed Blade - High Torque MDU
#define BLADE_ID_CURVED_HT		0x15	// Curved i.e. Low Speed Blade - High Torque MDU
#define BLADE_ID_BURR_HT		0x16	// Burr i.e. High Speed Blade - High Torque MDU
#define BLADE_ID_FAST_BURR_HT	0x17	// High Speed Burr i.e. Highest Speed Blade - High Torque MDU
#define BLADE_ID_OTHER			0xFF	// Other


// MDU defines
#define TYPE_INVALID				0
#define TYPE_ERROR					0xffff

// High Speed Sensing Mdu's
#define TYPE_MDU_STANDARD           1
#define TYPE_MDU_STANDARD_CTL       2

// Low Speed Sensing Mdu's
#define TYPE_MDU_UTLRA_IUR          5
#define TYPE_MDU_MINI               6
#define TYPE_MDU_TEST               7
#define TYPE_MDU_RELIANT            8
#define TYPE_MDU_RELIANT_CTL		9
#define TYPE_MDU_POWERMINI			10
#define TYPE_MDU_POWERMINI_CTL		11
#define TYPE_MDU_POWERMINI_BF		12
#define TYPE_MDU_RELIANT_BF			13

#define IS_TYPE_MDU(x)				((x) >= TYPE_MDU_STANDARD && (x) <= TYPE_MDU_RELIANT_BF)

#define TYPE_DP_DRILL               20
#define TYPE_DP_SAW                 21

#define IS_TYPE_POWER_INSTR(x)		((x) >= TYPE_DP_DRILL && (x) <= TYPE_DP_SAW)
#define IS_TYPE_DRILL(x)			((x) == TYPE_DP_DRILL)
#define IS_TYPE_SAW(x)				((x) == TYPE_DP_SAW)

#define TYPE_RF                     30

#define HALL_MDU_STANDARD           0x0003
#define HALL_MDU_STANDARD_CTL       0x001F
#define HALL_MDU_HIGH_TORQUE        0x0021
#define HALL_MDU_HIGH_TORQUE_CTL    0x003D
#define HALL_MDU_MASK               0x003F

#define HALL_FOOTPEDAL_ANALOG		0x0040

// Converts High Speed & Low Speed Hall patterns to Blade Family
#define HALL_TO_BLADE(x)            ((((x)&0x20)?((x)|2):(x))&3)

#define LOGIC_ID_MDU_BASIC      1
#define LOGIC_ID_MDU_MINI       2
#define LOGIC_ID_MDU_ULTRA_IUR	3
#define LOGIC_ID_DRILL          4
#define LOGIC_ID_SAW            5
#define LOGIC_ID_RS485          6
#define LOGIC_ID_RESERVED       7

#define RS485_TYPE_MDU_TEST				1
#define RS485_TYPE_PUMP_FOOTSWITCH		1
#define RS485_TYPE_MDU_POWERMINI_CTL	2
#define RS485_TYPE_MDU_RELIANT_CTL		3
#define RS485_TYPE_MDU_RELIANT 			4
#define RS485_TYPE_MDU_POWERMINI		5
#define RS485_TYPE_MDU_POWERMINI_BF		6
#define RS485_TYPE_MDU_RELIANT_BF		7

#define SERIAL_CMD_VERS         0x00
#define SERIAL_CMD_DEV_TYPE     0xb1

#define SERIAL_CMD_REQ_2        0xd2
#define SERIAL_CMD_REQ_3        0x63
#define SERIAL_CMD_REQ_4        0xe4
#define SERIAL_CMD_REQ_5        0x55
#define SERIAL_CMD_REQ_6        0x36
#define SERIAL_CMD_REQ_7        0x87
#define SERIAL_CMD_REQ_8        0x78
#define SERIAL_CMD_REQ_9        0xc9
#define SERIAL_CMD_REQ_10       0xaa
#define SERIAL_CMD_REQ_11       0x1b
#define SERIAL_CMD_REQ_12       0x9c
#define SERIAL_CMD_REQ_13       0x2d
#define SERIAL_CMD_REQ_14       0x4e
#define SERIAL_CMD_REQ_15       0xff

#define SERIAL_MDU_CTL_REV      0x0002
#define SERIAL_MDU_CTL_FWD      0x0004
#define SERIAL_MDU_CTL_OSC      0x0001
#define SERIAL_MDU_CTL_MASK     0x0007

// POWERMINI for the Small Joint project
// Small Joint Hand Mode bit mask
// for MDU.Out CmdData
#define SMJ_MOTOR_OFF				0x00
#define SMJ_MOTOR_REVERSE			0x01
#define SMJ_MOTOR_FORWARD			0x02
#define SMJ_MOTOR_OSCILLATE			0x04
#define	SMJ_MOTOR_WINDOW_LOCK		0x08

// Small Joint MDU.Enable CmdData
#define SMJ_BUTTON_DISTAL			0x10
#define SMJ_BUTTON_PROXIMAL			0x20

// Small Joint Blade Families
#define SMJ_BLADE_TYPE_MASK			0xF0
#define SMJ_BLADE_MEDIUM_SPEED		0x00	// Medium Speed Blade Family	
#define SMJ_BLADE_LOW_SPEED_R1		0x10	// Low Speed Blade Family Range 1
#define SMJ_BLADE_HIGH_SPEED_R1		0x20	// NEB High Speed Blade Family Range 1
											// NOTE:The following are future development
#define SMJ_BLADE_LOW_SPEED_R2		0x40	// Low Speed Blade Family Range 2
#define SMJ_BLADE_HIGH_SPEED_R2		0x80	// NEB High Speed Blade Family Range 2
#define SMJ_BLADE_SPECIALTY_R1		0x50	// Specialty Blade Range 1
#define SMJ_BLADE_SPECIALTY_R2		0x90	// Specialty Blade Range 2
#define SMJ_BLADE_SPECIALTY_R3		0x60	// Specialty Blade Range 3
#define SMJ_BLADE_SPECIALTY_R4		0xA0	// Specialty Blade Range 4


#define TYPE_DIGITAL_FOOTPEDAL		1
#define TYPE_ANALOG_FOOTPEDAL		2
#define TYPE_485_ANALOG_FOOTPEDAL   3
#define TYPE_WIRELESS_FOOTPEDAL		4
#define TYPE_UNKNOWN_FOOTPEDAL		0xFFFE

#define DIRECTION_FORWARD       0
#define DIRECTION_REVERSE       1


#define KEY_SET_SPEED_UP		1
#define KEY_SET_SPEED_DOWN		2
#define KEY_STOP_SET_SPEED		3


#endif //__COMMONDEFINES_H__
