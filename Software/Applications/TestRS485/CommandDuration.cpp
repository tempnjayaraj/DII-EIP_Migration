// CommandDuration.cpp: implementation of the CCommandDuration class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SnTypes.h"
#include "CommandDuration.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCommandDuration::CCommandDuration(SnQByte qSize):
	m_qNumSamples(0)
{
	m_qSize = (qSize) ? qSize : 1;
	m_qElements = new SnQByte[m_qSize];
}

CCommandDuration::~CCommandDuration()
{
	delete [] m_qElements;
}

// Subscript operator for CCommandDuration.
SnQByte & CCommandDuration::operator[]( SnQByte qIndex )
{

  if( qIndex < m_qSize )
    return m_qElements[qIndex];
  else
  {
    return m_qElements[m_qSize-1];
  }
}

void CCommandDuration::NewEntry(SnQByte newDuration)
{
	SnQByte ii = newDuration/50;
	if (ii > m_qSize-1)
		ii = m_qSize-1;
	m_qElements[ii] += 1;
	m_qNumSamples += 1;

	int debugPrint = FALSE;

	if (m_qNumSamples > 0x70000000)
	{
		SnQByte numSamples =0;
		for (ii = 0; ii < m_qSize; ii++)
		{
			m_qElements[ii] >>= 1;
			numSamples += m_qElements[ii];
		}
		m_qNumSamples = numSamples;
	}
	if (debugPrint)
	{
		for (ii = 0; ii <m_qSize-1; ii++)
			if (m_qElements[ii])
			{
				CString str;
				str.Format(_T("%.2f"), (float)(ii*0.05));
				DEBUGMSG(TRUE, (TEXT("%s\t%d\n"),str,m_qElements[ii]));
			}
	}
}

// overloaded assignment operator;
// const return avoids: ( a1 = a2 ) = a3
const CCommandDuration &CCommandDuration::operator=( const CCommandDuration &newTable )
{
	if ( &newTable != this ) // avoid self-assignment
	{
		// for Arrays of different sizes, deallocate original
		// left-side array, then allocate new left-side array
		if ( m_qSize != newTable.m_qSize )
		{
			delete [] m_qElements; // release space
			m_qSize = newTable.m_qSize; // resize this object
			m_qElements = new SnQByte[ m_qSize ]; // create space for array copy
		} // end inner if
		
		for ( SnQByte ii = 0; ii < m_qSize; ii++ )
			m_qElements[ii] = newTable.m_qElements[ii]; // copy array into object
		m_qNumSamples = newTable.m_qNumSamples;			// copy number of samples
	} // end outer if
	
	return *this; // enables x = y = z, for example
} // end function operator=


void CCommandDuration::Initialize(void)
{
	  for(SnWord ii = 0; ii < m_qSize; ii++)
		  m_qElements[ii] = 0;

	  m_qNumSamples = 0;
}

double CCommandDuration::Median(void)
{
	SnQByte medianIndex =0;
	SnQByte medianSum = m_qElements[0];
	double dMedian = 0.0;

	for(SnQByte ii = 0; ii < m_qSize; ii++)
	{
		while(medianSum < (m_qNumSamples >> 1) && (medianIndex < ii))
		{
			medianIndex++;
			medianSum += m_qElements[medianIndex];
		}
	}
	dMedian = medianIndex * 0.05;
	return dMedian;
}


double CCommandDuration::Maximum(void)
{
	SnQByte maximumIndex =0;
	double dMaximum = 0.0;

	for(SnQByte ii = m_qSize; ii > 0; )
	{
		ii--;
		if(m_qElements[ii])
		{
			maximumIndex = ii;
			break;
		}
	}
	dMaximum = maximumIndex * 0.05;
	return dMaximum;
}


