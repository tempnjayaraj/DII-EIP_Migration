// CommandDuration.h: interface for the CCommandDuration class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMMANDDURATION_H__67D93794_F067_41F4_BC2E_D0332D26B17C__INCLUDED_)
#define AFX_COMMANDDURATION_H__67D93794_F067_41F4_BC2E_D0332D26B17C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCommandDuration  
{
public:
	CCommandDuration(SnQByte qSize);
	virtual ~CCommandDuration();

	SnQByte & operator[]( SnQByte qIndex );
	const CCommandDuration &operator=( const CCommandDuration & ); // assignment operator

	void Initialize(void);

	void NewEntry(SnQByte newDuration);
	SnQByte Size(void){return m_qSize;}
	SnQByte Samples(void){return m_qNumSamples;}
	double Median(void);
	double Maximum(void);

private:
	SnQByte	m_qSize;
	SnQByte* m_qElements;
	SnQByte	m_qNumSamples;

};

#endif // !defined(AFX_COMMANDDURATION_H__67D93794_F067_41F4_BC2E_D0332D26B17C__INCLUDED_)
