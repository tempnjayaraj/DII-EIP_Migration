#ifndef _COLORSFONTS_H
#define _COLORSFONTS_H

// Primary color constants.

#define BRIGHT 255
#define MEDIUM 192
#define DARK    96

const COLORREF RED       = RGB(BRIGHT,      0,      0);
const COLORREF DKRED     = RGB(  DARK,      0,      0);
const COLORREF GREEN	 = RGB(     0, BRIGHT,      0);
const COLORREF DKGREEN   = RGB(     0,   DARK,      0);
const COLORREF BLUE      = RGB(     0,      0, BRIGHT);
const COLORREF DKBLUE    = RGB(     0,      0,   DARK);
const COLORREF CYAN      = RGB(     0, BRIGHT, BRIGHT);
const COLORREF DKCYAN    = RGB(     0,   DARK,   DARK);
const COLORREF MAGENTA   = RGB(BRIGHT,      0, BRIGHT);
const COLORREF DKMAGENTA = RGB(  DARK,      0,   DARK);
const COLORREF YELLOW    = RGB(BRIGHT, BRIGHT,      0);
const COLORREF BROWN     = RGB(  DARK,   DARK,      0);
const COLORREF BLACK     = RGB(     0,     0,       0);
const COLORREF WHITE     = RGB(BRIGHT, BRIGHT, BRIGHT);
const COLORREF LTGRAY    = RGB(MEDIUM, MEDIUM, MEDIUM);
const COLORREF DKGRAY    = RGB(  DARK  , DARK,   DARK);


// Decimal values
const COLORREF SN_WHITE				= RGB(255, 255, 255);
const COLORREF SN_RED				= RGB(255,0,0);
const COLORREF SN_DKGRAY				= RGB(89, 95,  98);
const COLORREF SN_BLACK				= RGB( 0, 0,   0);
const COLORREF SN_BKGND_COLOR		= RGB( 0, 0,   0);
const COLORREF SN_YELLOW				= RGB( 243,209,0);
const COLORREF SN_BLUE				= RGB(  94,137,191);
const COLORREF SN_GREEN				= RGB( 129,184,51);
const COLORREF SN_STATIC_BOX_GREEN	= RGB( 184,215,106);
const COLORREF SN_STATIC_BOX_ORANGE	= RGB( 223,143,57);
const COLORREF SN_STATIC_BOX_BLUE	= RGB( 139,172,213);

#define SN_LINE_COLOR				SN_DKGRAY
#define SN_BKGROUND_BLKBTN_COLOR	SN_DKGRAY
#define SN_BKGROUND_COLOR			SN_BLACK

// Font constants.

const int MIN_FONT    =   2;  // Minimum font point size.
const int MAX_FONT    =  70;  // Maximum font point size.
const int DEF_PT_SIZE =   8;  // Default text font point size.


// Line Widths

#define SN_LINE_WIDTH_6		6
#define SN_LINE_WIDTH_4		4

#define SN_CLEAR_TEXT		_T("")
#define SN_APP_VERSION		_T("1.0.0")
#define SN_DASH				_T("- -")	


#endif
