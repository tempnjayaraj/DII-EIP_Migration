#include "stdafx.h"
#include "Shaver.h"
#include "SlidingWindowFilter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DYNAMIC_ARROWS	0	// If 1 then strobe the arrows

SlidingWindowFilter::SlidingWindowFilter(int size, long feedValue):
	arraySize(size), average(feedValue)
{
	arrayTable = new (long[arraySize]);
	coefA = new (double[arraySize]);

	arrayConstant = 0;
	for (index = 0; index < arraySize; index++)
	{
		coefA[index] = 0.54 - 0.46 * cos(2*PI()*index/(arraySize));		// Hamming
//		coefA[index] = 0.5 - 0.5 * cos(2*PI()*index/(arraySize-1));			//  Hann
//		coefA[index] = 1;
		arrayTable[index] = feedValue;
		arrayConstant += coefA[index];
	}
	average = feedValue;
	index = 0;
}

SlidingWindowFilter::Init(long feedValue)
{
	for (index = 0; index < arraySize; index++)
	arrayTable[index] = feedValue;
	index = 0;
	average = feedValue;
}

SlidingWindowFilter::~SlidingWindowFilter()
{
	delete[] coefA;
	coefA = NULL;
	delete[] arrayTable;
	arrayTable = NULL;
}

double SlidingWindowFilter::AddValue (long newValue)
{
	double averageTmp = 0;
	double filterTmp = 0;
	long maximumTmp = 0;
	arrayTable[index] = newValue;
	int ii = 0;
	int jj = index;

	while( jj < arraySize)
	{
		averageTmp += arrayTable[jj];
		filterTmp += arrayTable[jj] * coefA[ii];
		ii++;
		jj++;
	}
	for (jj = 0; ii < arraySize; ii++, jj++)
	{
		filterTmp += arrayTable[jj] * coefA[ii];
		averageTmp += arrayTable[jj];
	}
	for (ii = 0; ii < arraySize; ii++)
	{
		if (maximumTmp < arrayTable[ii])
			maximumTmp = arrayTable[ii];
	}

	filterTmp /= arrayConstant;
	averageTmp /= arraySize;

	index += 1;
	if (index == arraySize)
	{
		index = 0;
		average  = averageTmp + 0.5;
		filtered = filterTmp + 0.5;
		maximum  = maximumTmp;
	}

	return (average);
}