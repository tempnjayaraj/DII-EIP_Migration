#ifndef SLIDING_WINDOW_FILTER_H
#define SLIDING_WINDOW_FILTER_H
#include "SnTypes.h"

class SlidingWindowFilter
{
public:
	SlidingWindowFilter(int size = 7, long feedValue = 0.0);
	~SlidingWindowFilter();
	Init(long feedValue = 0);
	double AddValue(long newValue);
	long Maximum(void){ return maximum;}
	double Average(void){ return average;}
	double Filtered(void){ return filtered;}
	double PI(void){return 3.1415926535;}

private:
	double *coefA;
	long *arrayTable;
	double arrayConstant;
	long maximum;
	double average;
	double filtered;
	long  arraySize;
	short index;
};

#endif