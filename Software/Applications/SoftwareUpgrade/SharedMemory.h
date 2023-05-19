// SharedMemory.h: interface for the CSharedMemory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHAREDMEMORY_H__5DC2D1F6_FE09_4F33_94A3_78C1A5535518__INCLUDED_)
#define AFX_SHAREDMEMORY_H__5DC2D1F6_FE09_4F33_94A3_78C1A5535518__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Driver.h"
#include "Util.h"

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


class CSharedMemory 
{
public:
	CSharedMemory();
	virtual ~CSharedMemory();

public:

	BOOL Init(CDriver* pDriver);
	void DeInit( void); 
	BOOL GetInitStatus( void);
	CString& GetString(int resourceID);
	void SetUpgradeFileName( char* pcFileName, int iLength);
	void GetUpgradeFileName( char* pcFileName, int iLength);
	void SetStringDetailMode( BOOL bDetail);
	BOOL GetStringDetailMode( void);
	void GetBuffer( char* dest, CString csSource);
		
private: // helper functions
	BOOL CreateFonts();
	int GetLanguageSelection();
	BOOL LoadLanguageDll();

public:
	BOOL		m_InitSucessful; 

	CFont*		m_Font12Bold;   // Fonts
	CFont*		m_Font14Normal;
	CFont*		m_Font15Normal;
	CFont*		m_Font16Normal;
	CFont*		m_Font16Bold;
	CFont*		m_Font20Normal;
	CFont*		m_Font20Bold;
	CFont*		m_Font25Normal;
	CFont*		m_Font30Normal;
	CFont*		m_Font30Bold;
	CFont*		m_Font40Bold;
	CFont*		m_Font40Normal;
	CFont*		m_Font55Bold;
	CFont*		m_Font75Bold;
	CFont*		m_Font100Bold;

		
private:

	CDriver*	m_hDriver;
};

#endif // !defined(AFX_SHAREDMEMORY_H__5DC2D1F6_FE09_4F33_94A3_78C1A5535518__INCLUDED_)
