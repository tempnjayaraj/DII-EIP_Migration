// CalcPLL.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

typedef unsigned long SnQByte;

int _tmain(int argc, _TCHAR* argv[])
{
    SnQByte qDiv, qTargDiv;
	SnQByte qMul, qTargMul;
	double dTarget = 1.0;
	double dVal;

	for (qMul = 0; qMul <= 0x800; qMul++) {
		for (qDiv = 1; qDiv < 0x100; qDiv++) {
			dVal = (16.3676 * (double)(qMul + 1)) / (double)qDiv;
			if (dVal < 133.33333333 && dVal > dTarget) {
				dTarget = dVal;
				qTargDiv = qDiv;
				qTargMul = qMul;
			}
		}
	}

	MessageBox(NULL,TEXT("Created CE.CMD file"),TEXT("SUCCESS"),MB_OK|MB_ICONINFORMATION|MB_TOPMOST);
	return 0;
}

