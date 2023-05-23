// TouchCalibrate.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

extern "C" BOOL WINAPI TouchCalibrate(void);

int _tmain(int argc, TCHAR *argv[], TCHAR *envp[])
{
    TouchCalibrate();
    return 0;
}
