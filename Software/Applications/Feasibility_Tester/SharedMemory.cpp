// SharedMemory.cpp: implementation of the CSharedMemory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SharedMemory.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Shared variables
static CControl*		g_pControl = NULL;		// Pointer to a Control Layer Object

static CFont	gFont10Bold;
static CFont	gFont12Bold;
static CFont	gFont14Normal;
static CFont	gFont15Normal;
static CFont	gFont16Normal;
static CFont	gFont16Bold;
static CFont	gFont18Normal;
static CFont	gFont20Normal;
static CFont	gFont20Bold;
static CFont	gFont25Normal;
static CFont	gFont25Bold;
static CFont	gFont30Normal;
static CFont	gFont30Bold;
static CFont	gFont50Bold;

static SnBool gDirty = FALSE;
static SnBool gInitSuccesfull = FALSE;

static unsigned short g_usLanguage = LANGUAGE_ENGLISH;

HINSTANCE		g_hResLib = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSharedMemory::CSharedMemory()
{
	m_InitSucessfull = gInitSuccesfull;
	
	m_Font10Bold = &gFont10Bold;
	m_Font12Bold = &gFont12Bold;
	m_Font14Normal = &gFont14Normal;
	m_Font15Normal = &gFont15Normal;
	m_Font16Normal = &gFont16Normal;
	m_Font16Bold = &gFont16Bold;
	m_Font18Normal = &gFont18Normal;
	m_Font20Normal = &gFont20Normal;
	m_Font20Bold = &gFont20Bold;
	m_Font25Normal = &gFont25Normal;
	m_Font25Bold = &gFont25Bold;
	m_Font30Normal = &gFont30Normal;
	m_Font30Bold = &gFont30Bold;
	m_Font50Bold = &gFont50Bold;
}

CSharedMemory::~CSharedMemory()
{
	
}

unsigned short CSharedMemory::GetCurrentLanguage()
{
	return g_usLanguage;
}
SnBool CSharedMemory::GetInitStatus()
{
	return gInitSuccesfull;
}

SnBool CSharedMemory::Init( CWnd* pParent, CControl* pControl)
{
	
	SnBool bStatus;
	// Init must be called at least once so the structures can be initialized
	
	g_pControl = pControl; // Pointer to the control layer
	
	// Load the language dll
	bStatus = LoadLanguageDll();
	if( !bStatus)
	{
		DeInit(); // unwind the stack
		return FALSE;
	}
	
	// Create Fonts
	bStatus = CreateFonts();
	if( !bStatus)
	{
		DeInit(); // unwind the stack
		return FALSE;
	}
	
	gInitSuccesfull = TRUE;

	return TRUE;
}

void CSharedMemory::DeInit( void) 
{

	gInitSuccesfull = FALSE;

	// Delete all Font resources
	DeleteObject( gFont10Bold);
	DeleteObject( gFont12Bold);
	DeleteObject( gFont14Normal);
	DeleteObject( gFont15Normal);
	DeleteObject( gFont16Normal);
	DeleteObject( gFont16Bold);
	DeleteObject( gFont18Normal);
	DeleteObject( gFont20Normal);
	DeleteObject( gFont20Bold);
	DeleteObject( gFont25Normal);
	DeleteObject( gFont25Bold);
	DeleteObject( gFont30Normal);
	DeleteObject( gFont30Bold);
	DeleteObject( gFont50Bold);
	
	// free dll resources
	if( g_hResLib != NULL)
	{
		FreeLibrary( g_hResLib);
		g_hResLib = NULL;
	}
}

SnBool CSharedMemory::LoadLanguageDll()
{
	SnWord usLanguage;

	if( g_pControl) 
	{
	
		// Get the saved language selection
	    SnBool bStatus = g_pControl->GetCmdState(GET_SYSTEM_LANGUAGE, &usLanguage, sizeof(usLanguage));
		if(!bStatus)
			usLanguage = LANGUAGE_ENGLISH;
	}
	else
		usLanguage = LANGUAGE_ENGLISH;
	

    g_usLanguage = usLanguage; 
	
	// Load Language dll
	CString csDll;
	switch(g_usLanguage)
	{
		
		case LANGUAGE_ENGLISH:
			csDll = _T("EnglishRes");
			break;

		case LANGUAGE_GERMAN:
			csDll = _T("GermanRes");
			break;

		case LANGUAGE_ITALIAN:
			csDll = _T("ItalianRes");
			break;
	
		case LANGUAGE_SPANISH:
			csDll = _T("SpanishRes");
			break;


		case LANGUAGE_FRENCH:
			csDll = _T("FrenchRes");
			break;

		case LANGUAGE_DANISH:
			csDll = _T("DanishRes");
			break;

		case LANGUAGE_DUTCH:
			csDll = _T("DutchRes");
			break;

		case LANGUAGE_NORWEGIAN:
			csDll = _T("NorwegianRes");
			break;

		case LANGUAGE_PORTUGUESE:
			csDll = _T("PortugueseRes");
			break;

		case LANGUAGE_SWEDISH:
			csDll = _T("SwedishRes");
			break;

		default:
			csDll = _T("EnglishRes");		
			break;

	}
	
	if( g_hResLib != NULL)
	{
		FreeLibrary( g_hResLib);
		g_hResLib = NULL;
	}

	g_hResLib = LoadLibrary( csDll) ;

	if(g_hResLib == NULL)
	{
		if (g_usLanguage != LANGUAGE_ENGLISH)
		{
			// Try to load the English Dll as a last resort
			csDll = _T("EnglishRes");	
			g_hResLib = LoadLibrary (csDll); 
		
			if( g_hResLib == NULL)
				return FALSE;
		}

	}

	return TRUE;
}

SnBool CSharedMemory::LoadLanguageDll( SnWord usLanguage)
{

 	// Load Language dll
	CString csDll;
	switch(usLanguage)
	{
		
		case LANGUAGE_ENGLISH:
			csDll = _T("EnglishRes");
			break;

		case LANGUAGE_GERMAN:
			csDll = _T("GermanRes");
			break;

		case LANGUAGE_ITALIAN:
			csDll = _T("ItalianRes");
			break;
	
		case LANGUAGE_SPANISH:
			csDll = _T("SpanishRes");
			break;


		case LANGUAGE_FRENCH:
			csDll = _T("FrenchRes");
			break;

		case LANGUAGE_DANISH:
			csDll = _T("DanishRes");
			break;

		case LANGUAGE_DUTCH:
			csDll = _T("DutchRes");
			break;

		case LANGUAGE_NORWEGIAN:
			csDll = _T("NorwegianRes");
			break;

		case LANGUAGE_PORTUGUESE:
			csDll = _T("PortugueseRes");
			break;

		case LANGUAGE_SWEDISH:
			csDll = _T("SwedishRes");
			break;

		default:
			usLanguage = LANGUAGE_ENGLISH;
			csDll = _T("EnglishRes");		
			break;

	}
	
	if( g_hResLib != NULL)
	{
		FreeLibrary( g_hResLib);
		g_hResLib = NULL;
	}

	g_usLanguage = usLanguage; 

	g_hResLib = LoadLibrary( csDll) ;

	if(g_hResLib == NULL)
	{
		// Load Library failed	
		if (g_usLanguage != LANGUAGE_ENGLISH)
		{
			// Try to load the English Dll as a last resort
			csDll = _T("EnglishRes");	
			g_hResLib = LoadLibrary (csDll); 
		
			if( g_hResLib == NULL)
				return FALSE;
		}

	}

	return TRUE;
}

HINSTANCE CSharedMemory::GetResourceHandle()
{ 
	HINSTANCE hResLibHandle = g_hResLib;

	return hResLibHandle;
}

SnBool CSharedMemory::CreateFonts()
{
	SnBool bStatus;
	LOGFONT txtFont;

	memset(&txtFont, 0, sizeof(LOGFONT));  // Clear out the structure.
	
	// Setup the common members for all Fonts
	txtFont.lfWidth = 0;
	txtFont.lfEscapement = 0;
	txtFont.lfOrientation = 0;
	txtFont.lfItalic = FALSE;
	txtFont.lfUnderline = FALSE;
	txtFont.lfStrikeOut = 0;
	txtFont.lfCharSet = DEFAULT_CHARSET;
	txtFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	txtFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	txtFont.lfQuality = DEFAULT_QUALITY;
	txtFont.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
	wcscpy(txtFont.lfFaceName, _T("Arial"));
	
	// Create Font
	txtFont.lfHeight = 100;
	txtFont.lfWeight = FW_NORMAL;
	bStatus = gFont10Bold.CreatePointFontIndirect(&txtFont, NULL);
	
	// Create Font
	txtFont.lfHeight = 120;
	txtFont.lfWeight = FW_BOLD;
	bStatus = gFont12Bold.CreatePointFontIndirect(&txtFont, NULL);

	// Create Font
	txtFont.lfHeight = 140;
	txtFont.lfWeight = FW_NORMAL;
	bStatus = gFont14Normal.CreatePointFontIndirect(&txtFont, NULL);

	// Create Font
	txtFont.lfHeight = 150;
	txtFont.lfWeight = FW_NORMAL;
	bStatus = gFont15Normal.CreatePointFontIndirect(&txtFont, NULL);
	if( !bStatus)
		return FALSE;
	
	// Create Font
	txtFont.lfHeight = 160;
	txtFont.lfWeight = FW_NORMAL;
	bStatus = gFont16Normal.CreatePointFontIndirect(&txtFont, NULL);
	if( !bStatus)
		return FALSE;
	
	// Create Font
	txtFont.lfHeight = 160;
	txtFont.lfWeight = FW_BOLD;
	bStatus = gFont16Bold.CreatePointFontIndirect(&txtFont, NULL);
	if( !bStatus)
		return FALSE;
	
	// Create Font
	txtFont.lfHeight = 180;
	txtFont.lfWeight = FW_NORMAL;
	bStatus = gFont18Normal.CreatePointFontIndirect(&txtFont, NULL);
	if( !bStatus)
		return FALSE;
	
	// Create Font
	txtFont.lfHeight = 200;
	txtFont.lfWeight = FW_NORMAL;
	bStatus = gFont20Normal.CreatePointFontIndirect(&txtFont, NULL);
	if( !bStatus)
		return FALSE;

	// Create Font
	txtFont.lfHeight = 200;
	txtFont.lfWeight = FW_BOLD;
	bStatus = gFont20Bold.CreatePointFontIndirect(&txtFont, NULL);
	if( !bStatus)
		return FALSE;

	// Create Font
	txtFont.lfHeight = 250;
	txtFont.lfWeight = FW_NORMAL;
	bStatus = gFont25Normal.CreatePointFontIndirect(&txtFont, NULL);
	if( !bStatus)
		return FALSE;

	// Create Font
	txtFont.lfHeight = 250;
	txtFont.lfWeight = FW_BOLD;
	bStatus = gFont25Bold.CreatePointFontIndirect(&txtFont, NULL);
	if( !bStatus)
		return FALSE;
	
	// Create Font
	txtFont.lfHeight = 300;
	txtFont.lfWeight = FW_NORMAL;
	bStatus = gFont30Normal.CreatePointFontIndirect(&txtFont, NULL);
	if( !bStatus)
		return FALSE;

	// Create Font
	txtFont.lfHeight = 300;
	txtFont.lfWeight = FW_BOLD;
	bStatus = gFont30Bold.CreatePointFontIndirect(&txtFont, NULL);
	if( !bStatus)
		return FALSE;

	// Create Font
	txtFont.lfHeight = 500;
	txtFont.lfWeight = FW_BOLD;
	bStatus = gFont50Bold.CreatePointFontIndirect(&txtFont, NULL);
	if( !bStatus)
		return FALSE;

    return TRUE;

}

