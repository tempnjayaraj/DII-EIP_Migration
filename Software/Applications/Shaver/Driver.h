//
// Driver.h: interface for the CDriver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRIVER_H__8D6B37CD_653F_4FE1_B7B9_EAAAD9C48863__INCLUDED_)
#define AFX_DRIVER_H__8D6B37CD_653F_4FE1_B7B9_EAAAD9C48863__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDriver  
{
public:
	CDriver();
	virtual ~CDriver();

public:
	SnBool InitDriver(void);
	void DeInitDriver(void);

    SnBool ReadFlashData(SnQByte qAddr, SnByte *pbData, SnQByte qBytes);
    SnBool EraseFlashPages(SnQByte qAddr, SnQByte qPages);
    SnBool WriteFlashData(SnQByte qAddr, SnByte *pbData, SnQByte qBytes);
    SnBool VerifyFlashData(SnQByte qAddr, SnByte *pbData, SnQByte qBytes);
 
    SnBool FlashStoreIsErased(SnAddr aFlashStoreAddr, SnQByte qMaxStoreSize);
    SnBool ReadFlashStore(SnAddr aFlashStoreBase, SnAddr aFlashStoreTop, SnQByte qMaxStoreBufSize,
                               SnByte *pbLastStore, SnQByte *pqLastStoreSize);
    SnBool WriteFlashStore(SnByte *pbStoreData, SnQByte qStoreSize);
    SnBool EraseFlashStore(void);
    SnBool CrcBufData(SnByte *pbData, SnQByte qBytes, SnByte *pbCrc);

    SnBool ReadNvRam(void *pxData, SnQByte qOffset, SnQByte qLen, SnQByte *pqBytesRead);
    SnBool WriteNvRam(void *pxData, SnQByte qOffset, SnQByte qLen, SnQByte *pqBytesWritten);

	SnBool ReadWordFromDevice(SnQByte qOffset, SnWord *pwData);
	SnBool WriteWordToDevice(SnQByte qOffset, SnWord wData);
	SnBool ReadWordsFromDevice(SnQByte qOffset, SnQByte qNumWords, SnWord *pwData);
	SnBool WriteWordsToDevice(SnQByte qOffset, SnQByte qNumWords, SnWord *pwData);
    SnBool SerialCmdsToDevice(SnQByte qOffset, SnQByte qNumCmds, SnWord *pwCmds, SnWord wTimeout);
    SnBool SerialPageToDevice(SnQByte qPageCmdSize, SnByte *pbPageCmd);
	SnBool FlashBlockToDevice(SnByte bPage, SnWord *pwData);

    SnBool ResetDisplayBase(void);
    SnBool SetSystemBuzzer(SnBool yEnable);
    SnBool ResetDsp(void);
    SnBool CheckNvRamBattery(void);
    SnBool CanTest(void);
    SnBool CheckBootStatus(SnQByte *pqBootStatus);

    void GetDriverError(void);
 
    // Private member variables
private:
	HANDLE m_hDriverDev;
    HANDLE m_hFlashAccess;
    HANDLE m_hNewCmd;
};

#endif // !defined(AFX_DRIVER_H__8D6B37CD_653F_4FE1_B7B9_EAAAD9C48863__INCLUDED_)
