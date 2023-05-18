// CommBase.h: interface for the CCommBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMMBASE_H__78F731C0_4158_45CE_B46C_45FEA21EFE6D__INCLUDED_)
#define AFX_COMMBASE_H__78F731C0_4158_45CE_B46C_45FEA21EFE6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCommBase  //base class for remote or local Communications
{
public:
	
	//constructor of child class must set port specific data

	virtual DWORD Write( char txByte ) = 0;	// Write Byte
	virtual DWORD WriteBytes( char *pTxBuf ) = 0; // Write Bytes
	
	virtual DWORD Read( char *pRxBuf, DWORD buf_size ) = 0; // Read byte
	virtual DWORD ReadBytes( char *pRxBuf, DWORD size ) = 0;// Read Bytes


};

#endif // !defined(AFX_COMMBASE_H__78F731C0_4158_45CE_B46C_45FEA21EFE6D__INCLUDED_)
