#ifndef _COLORSFONTS_H
#define _COLORSFONTS_H

// Primary color constants.
const COLORREF WHITE     = RGB(255, 255, 255);
							// Decimal values
const COLORREF SN_WHITE				= RGB(255, 255, 255);
const COLORREF SN_RED				= RGB(255,0,0);
const COLORREF SN_DKGRAY			= RGB(89, 95,  98);
const COLORREF SN_BLACK				= RGB( 0, 0,   0);
const COLORREF SN_BKGND_COLOR		= RGB( 0, 0,   0);
const COLORREF SN_YELLOW			= RGB( 243,209,0);
const COLORREF SN_BLUE				= RGB( 117,197,240);
const COLORREF SN_PURPLE			= RGB(186,143,186);
const COLORREF SN_GOLD				= RGB(248,180, 0);
const COLORREF SN_GREEN				= RGB( 129,184,51);

#define SN_LINE_COLOR				SN_DKGRAY
#define SN_BKGROUND_COLOR			SN_BLACK


// Line Widths

#define SN_LINE_WIDTH_10	10
#define SN_LINE_WIDTH_6		6
#define SN_LINE_WIDTH_4		4
#define SN_LINE_WIDTH_5		5

#define SN_CLEAR_TEXT		_T("")
#define SN_APP_VERSION		_T("1.1.53")	// Application Software version

#define SN_SHAVER_REF		_T("72200873")	// Model
// Need to put 2 &
#define SN_COPYRIGHT		_T("2009 Smith && Nephew, Inc.") // Copyright

#define SN_NUM_ONE			_T("1")
#define SN_NUM_TWO			_T("2")
#define SN_NUM_THREE		_T("3")
#define SN_NUM_FOUR			_T("4")


#endif
