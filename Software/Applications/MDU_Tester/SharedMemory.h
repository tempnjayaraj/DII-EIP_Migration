// SharedMemory.h: interface for the CSharedMemory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHAREDMEMORY_H__5DC2D1F6_FE09_4F33_94A3_78C1A5535518__INCLUDED_)
#define AFX_SHAREDMEMORY_H__5DC2D1F6_FE09_4F33_94A3_78C1A5535518__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Control.h"
//#include "MduToolSettingsScreen.h"

class CSharedMemory 
{
public:
	CSharedMemory();
	virtual ~CSharedMemory();

public:

	SnBool Init(  CWnd* pParent, CControl* pControl);
	
	void DeInit( void);
	SnBool GetInitStatus( void);
	HINSTANCE GetResourceHandle( );
	SnBool LoadLanguageDll();
	SnBool LoadLanguageDll( SnWord usLanguage);
	unsigned short GetCurrentLanguage();
	
private: // helper functions
	SnBool CreateFonts();
	
public:
	SnBool		m_InitSucessfull; 
	CFont*		m_Font10Bold;   // Fonts
	CFont*		m_Font12Bold;   
	CFont*		m_Font14Normal;
	CFont*		m_Font15Normal;
	CFont*		m_Font16Normal;
	CFont*		m_Font16Bold;
	CFont*		m_Font18Normal;
	CFont*		m_Font20Normal;
	CFont*		m_Font20Bold;
	CFont*		m_Font25Normal;
	CFont*		m_Font25Bold;
	CFont*		m_Font30Normal;
	CFont*		m_Font30Bold;
	CFont*		m_Font40Bold;
	CFont*		m_Font40Normal;
	CFont*		m_Font45Bold;
	CFont*		m_Font50Bold;
	CFont*		m_Font75Bold;
	CFont*		m_Font90Bold;
	CFont*		m_Font100Bold;

};

#endif // !defined(AFX_SHAREDMEMORY_H__5DC2D1F6_FE09_4F33_94A3_78C1A5535518__INCLUDED_)
