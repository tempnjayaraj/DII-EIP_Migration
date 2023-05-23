// SoftwareUpgrade.h : main header file for the SOFTWAREUPGRADE application
//

#if !defined(AFX_SOFTWAREUPGRADE_H__064B904C_660C_42C9_8955_E03EB6BC9E0C__INCLUDED_)
#define AFX_SOFTWAREUPGRADE_H__064B904C_660C_42C9_8955_E03EB6BC9E0C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "Resource.h"
#include "ColorFonts.h"
#include "Driver.h"
#include "Util.h"

/////////////////////////////////////////////////////////////////////////////
// CSoftwareUpgradeApp:
// See SoftwareUpgrade.cpp for the implementation of this class
//

class CSoftwareUpgradeApp : public CWinApp
{
public:
	CSoftwareUpgradeApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSoftwareUpgradeApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CSoftwareUpgradeApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:    // Local Functions
    SnBool GetValidFlashImage(SnQByte qAddr, SnByte **ppbData, SnQByte *pqSize);
    SnBool CopyFlashImage(SnQByte qSrcAddr, SnQByte qDstAddr);
    SnBool BackupFlashImageIfNeeded(void);
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOFTWAREUPGRADE_H__064B904C_660C_42C9_8955_E03EB6BC9E0C__INCLUDED_)
