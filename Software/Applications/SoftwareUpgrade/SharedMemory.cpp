// SharedMemory.cpp: implementation of the CSharedMemory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SharedMemory.h"
#include "SnTypes.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Global variables
HINSTANCE		g_hResLib = NULL;

static char		g_cpUpgradeFileName[MAX_PATH];
static BOOL		g_bDetail = FALSE;

static CFont	gFont12Bold;
static CFont	gFont14Normal;
static CFont	gFont15Normal;
static CFont	gFont16Normal;
static CFont	gFont16Bold;
static CFont	gFont20Normal;
static CFont	gFont20Bold;
static CFont	gFont25Normal;
static CFont	gFont30Normal;
static CFont	gFont30Bold;
static CFont	gFont40Normal;
static CFont	gFont40Bold;
static CFont	gFont55Bold;
static CFont	gFont75Bold;
static CFont	gFont100Bold;

static BOOL gInitSuccesfull = FALSE;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSharedMemory::CSharedMemory()
{
	m_InitSucessful = gInitSuccesfull;

	m_Font12Bold =	 &gFont12Bold;
	m_Font14Normal = &gFont14Normal;
	m_Font15Normal = &gFont15Normal;
	m_Font16Normal = &gFont16Normal;
	m_Font16Bold =	 &gFont16Bold;
	m_Font20Normal = &gFont20Normal;
	m_Font20Bold =	 &gFont20Bold;
	m_Font25Normal = &gFont25Normal;
	m_Font30Normal = &gFont30Normal;
	m_Font40Normal = &gFont40Normal;
	m_Font30Bold =	 &gFont30Bold;
	m_Font40Bold =   &gFont40Bold;
	m_Font55Bold =	 &gFont55Bold;
	m_Font75Bold =	 &gFont75Bold;
	m_Font100Bold =	 &gFont100Bold;
}

CSharedMemory::~CSharedMemory()
{
}
void CSharedMemory::SetUpgradeFileName( char* pcFileName, int iLength)
{
	int i;

	if( pcFileName)
	{
		for( i=0; i< iLength; i++)
		{
			g_cpUpgradeFileName[i] = *pcFileName;
			pcFileName++;
		}
	}
	
}
void CSharedMemory::GetUpgradeFileName( char* pcFileName, int iLength)
{
	int i;

	if( pcFileName)
	{
		for( i=0; i < iLength; i++)
		{
			*pcFileName = g_cpUpgradeFileName[i];
			pcFileName++;
		}

	}
}
void CSharedMemory::SetStringDetailMode( BOOL bDetail)
{
	g_bDetail = bDetail;
}
BOOL CSharedMemory::GetStringDetailMode( void)
{
	return g_bDetail;

}
BOOL CSharedMemory::GetInitStatus()
{
	return gInitSuccesfull;
}

BOOL CSharedMemory::Init(CDriver* pDriver)
{
	BOOL bStatus;

	if( pDriver == NULL)
		return FALSE;

	m_hDriver = pDriver;

	//
	// Init must be called at least once so the structures can be initialized
	//
	
	// Load the language resources
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

	// The Init was successfull
	gInitSuccesfull = TRUE;
	
	return TRUE;
}

void CSharedMemory::DeInit( void) 
{

	// Delete all our resources
	
	// free dll resources
	if( g_hResLib != NULL)
		FreeLibrary( g_hResLib);	
	
	// Delete all Font resources
	DeleteObject( gFont12Bold);
	DeleteObject( gFont14Normal);
	DeleteObject( gFont15Normal);
	DeleteObject( gFont16Normal);
	DeleteObject( gFont16Bold);
	DeleteObject( gFont20Normal);
	DeleteObject( gFont20Bold);
	DeleteObject( gFont25Normal);
	DeleteObject( gFont30Normal);
	DeleteObject( gFont30Bold);
	DeleteObject( gFont40Normal);
	DeleteObject( gFont40Bold);
	DeleteObject( gFont55Bold);
	DeleteObject( gFont75Bold);
	DeleteObject( gFont100Bold);

	gInitSuccesfull = FALSE;
}

BOOL CSharedMemory::CreateFonts()
{
	BOOL bStatus;
	LOGFONT txtFont;

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
	txtFont.lfHeight = 400;
	txtFont.lfWeight = FW_NORMAL;
	bStatus = gFont40Normal.CreatePointFontIndirect(&txtFont, NULL);
	if( !bStatus)
		return FALSE;

	// Create Font
	txtFont.lfHeight = 400;
	txtFont.lfWeight = FW_BOLD;
	bStatus = gFont40Bold.CreatePointFontIndirect(&txtFont, NULL);
	if( !bStatus)
		return FALSE;
	
	// Create Font
	txtFont.lfHeight = 550;
	txtFont.lfWeight = FW_BOLD;
	bStatus = gFont55Bold.CreatePointFontIndirect(&txtFont, NULL);
	if( !bStatus)
		return FALSE;
	
	// Create Font
	txtFont.lfHeight = 750;
	txtFont.lfWeight = FW_BOLD;
	bStatus = gFont75Bold.CreatePointFontIndirect(&txtFont, NULL);
	if( !bStatus)
		return FALSE;

	// Create Font
	txtFont.lfHeight = 1000;
	txtFont.lfWeight = FW_BOLD;
	bStatus = gFont100Bold.CreatePointFontIndirect(&txtFont, NULL);
	if( !bStatus)
		return FALSE;

	return TRUE;
}

int CSharedMemory::GetLanguageSelection()
{
	SAVED_SHAVER_INFO tShaverNvRam;
    SnQByte qBytesRead;
	CUtil util;
	int iLanguage;
	int iRetry;

	if( m_hDriver == NULL)
		return LANGUAGE_ENGLISH; // can't access driver set language to default
 
	// Get the language selection from NVRAM memory
	for(iRetry = 3; iRetry > 0; iRetry--) 
	{
		if((m_hDriver->ReadNvRam(&tShaverNvRam,0,sizeof(tShaverNvRam),&qBytesRead) != FALSE)
			&& (qBytesRead == sizeof(tShaverNvRam))
			&& (util.CrcMem((unsigned char*)&tShaverNvRam,sizeof(tShaverNvRam)) == 0))
		{
			break;
		}
	}
	
	if(iRetry == 0)
	{
		iLanguage = LANGUAGE_ENGLISH; // Failed set language to default
	}
	else
		iLanguage = tShaverNvRam.language;
	
	return iLanguage;
}

BOOL CSharedMemory::LoadLanguageDll()
{
	int iLanguage;
	CString csDll;

	if( g_hResLib != NULL)
		FreeLibrary( g_hResLib);

	if( g_bDetail)
	{
		// We're in Detail mode which enables the display of status information
		// This mode is only supported in English
		csDll = _T("EnglishRes");
	}
	else
	{
		iLanguage = GetLanguageSelection();

		// Load Language dll
		
		switch(iLanguage)
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
	}
	
	g_hResLib = LoadLibrary( csDll) ;

	return TRUE;
}

CString& CSharedMemory::GetString(int resourceID)
{


	CSharedMemory mem;
	static CString text;
	TCHAR  buf[1024];

	if( g_hResLib != NULL)
	{
		//	Dll string table access
		if (LoadString( g_hResLib, resourceID, buf,  sizeof(buf)))
			text = buf;
		else
			text.LoadString(resourceID);
	}
	else
	{
		// Returns text extracted from the local String Table resource
		text.LoadString(resourceID);
	}

	
	return text;
}
// Function:	Get Buffer
// Purpose:		convert a CString type to a char buffer
// Return:		void
//
void CSharedMemory::GetBuffer( char* dest, CString csSource)
{
	CString csTmp;
	int length = 0;
	int i;
	LPTSTR  pBuffer;

	csTmp = csSource;
	length = csTmp.GetLength();
	length = length + 1; // add 1 for the null character

	pBuffer = csTmp.GetBuffer( length);
	for(i=0;i<length;i++)
	{
		dest[i] = (char)*pBuffer;
		pBuffer++;
	}
}