// Logger.h: interface for the CLogger class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGGER_H__03B76E3C_A260_45AA_88C9_D4689CCA8F4B__INCLUDED_)
#define AFX_LOGGER_H__03B76E3C_A260_45AA_88C9_D4689CCA8F4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLogger  
{
public:
	CLogger();
	virtual ~CLogger();

public:
	SnBool InitLogger(SnQByte qDataMaxSize);
	void DeInitLogger(void);
    SnBool AddString(const char *pcString);
    SnBool SaveData(const TCHAR *pwFileName);
    inline SnBool LoggerIsInitialized(void) { return m_yInitialized; }

private:
    SnBool      m_yInitialized;
    char        *m_pcDataBuf;
    char        *m_pcDataBufPtr;
    SnQByte     m_qDataMaxSize;
    SnQByte     m_qDataCurSize;
};

#endif // !defined(AFX_LOGGER_H__03B76E3C_A260_45AA_88C9_D4689CCA8F4B__INCLUDED_)
