#include <stdio.h>
#include <stddef.h>
#include "SnTypes.h"
#include "ControllerTypes.h"

void PrintIndent(SnQByte qLevel)
{
	SnQByte qCnt;

	for (qCnt = 0; qCnt < qLevel; qCnt++) {
		printf("    ");
	}
}

void PrintOffset(SnQByte qOffset, SnQByte qLevel, const char *pcName)
{
	printf("0x%04x(%4d)", qOffset >> 1, qOffset >> 1);
	PrintIndent(qLevel);
	printf("%s\n", pcName);
}

void PrintHeader(SnQByte qLevel, const char *pcHeader)
{
	printf("            ");
	PrintIndent(qLevel++);
	printf("%s\n", pcHeader);
}

void PrintKgain(SnQByte qOffset, SnQByte qLevel, const char *pcHeader)
{
	PrintHeader(qLevel++, pcHeader);

	PrintOffset(qOffset + offsetof(Kgain, fKf), qLevel, "fKf");
	PrintOffset(qOffset + offsetof(Kgain, fKp), qLevel, "fKp");
	PrintOffset(qOffset + offsetof(Kgain, fKi), qLevel, "fKi");
	PrintOffset(qOffset + offsetof(Kgain, fKd), qLevel, "fKd");
	PrintOffset(qOffset + offsetof(Kgain, fKa), qLevel, "fKa");
	PrintOffset(qOffset + offsetof(Kgain, fKl), qLevel, "fKl");
}

void PrintExternal(SnQByte qOffset, SnQByte qLevel, const char *pcHeader)
{
	PrintHeader(qLevel++, pcHeader);

	PrintOffset(qOffset + offsetof(External, wFCommTable), qLevel, "wFCommTable");
	PrintOffset(qOffset + offsetof(External, wRCommTable), qLevel, "wRCommTable");
	PrintOffset(qOffset + offsetof(External, wFDirTable), qLevel, "wFDirTable");
	PrintOffset(qOffset + offsetof(External, wRDirTable), qLevel, "wRDirTable");
	PrintOffset(qOffset + offsetof(External, wJogTable), qLevel, "wJogTable");
	PrintOffset(qOffset + offsetof(External, fResistance), qLevel, "fResistance");
	PrintOffset(qOffset + offsetof(External, fAccCvrt), qLevel, "fAccCvrt");
	PrintOffset(qOffset + offsetof(External, wTacPerRevN), qLevel, "wTacPerRevN");
	PrintOffset(qOffset + offsetof(External, wTacPerRevD), qLevel, "wTacPerRevD");
	PrintOffset(qOffset + offsetof(External, fAccel), qLevel, "fAccel");
	PrintOffset(qOffset + offsetof(External, fDecel), qLevel, "fDecel");
	PrintOffset(qOffset + offsetof(External, sVelMax), qLevel, "sVelMax");
	PrintOffset(qOffset + offsetof(External, fStopTime), qLevel, "fStopTime");
	PrintOffset(qOffset + offsetof(External, fLoopRate), qLevel, "fLoopRate");
	PrintOffset(qOffset + offsetof(External, fPWMFreq), qLevel, "fPWMFreq");
	PrintKgain(qOffset + offsetof(External, tKvel), qLevel, "tKvel");
	PrintKgain(qOffset + offsetof(External, tKpos), qLevel, "tKpos");
	PrintOffset(qOffset + offsetof(External, fFrpmA), qLevel, "fFrpmA");
	PrintOffset(qOffset + offsetof(External, wFrpmB), qLevel, "wFrpmB");
	PrintOffset(qOffset + offsetof(External, fTlimA), qLevel, "fTlimA");
	PrintOffset(qOffset + offsetof(External, wTlimB), qLevel, "wTlimB");
	PrintOffset(qOffset + offsetof(External, fIlim), qLevel, "fIlim");
	PrintOffset(qOffset + offsetof(External, fIhigh), qLevel, "fIhigh");

	PrintOffset(qOffset + offsetof(External, wMode), qLevel, "wMode");
	PrintOffset(qOffset + offsetof(External, sVelSet), qLevel, "sVelSet");
	PrintOffset(qOffset + offsetof(External, wDwell), qLevel, "wDwell");
	PrintOffset(qOffset + offsetof(External, fCycleTime), qLevel, "fCycleTime");
	PrintOffset(qOffset + offsetof(External, wProfileCmd), qLevel, "wProfileCmd");
	PrintOffset(qOffset + offsetof(External, wAssert), qLevel, "wAssert");
	PrintOffset(qOffset + offsetof(External, wFault), qLevel, "wFault");
}

void PrintInternal(SnQByte qOffset, SnQByte qLevel, const char *pcHeader)
{
	PrintHeader(qLevel++, pcHeader);

	PrintOffset(qOffset + offsetof(Internal, sOverload), qLevel, "sOverload");
	PrintOffset(qOffset + offsetof(Internal, fCurrent), qLevel, "fCurrent");
	PrintOffset(qOffset + offsetof(Internal, fVelAct), qLevel, "fVelAct");
	PrintOffset(qOffset + offsetof(Internal, wVelAbs), qLevel, "wVelAbs");
	PrintOffset(qOffset + offsetof(Internal, wLastTac), qLevel, "wLastTac");
	PrintOffset(qOffset + offsetof(Internal, fTacPerRev), qLevel, "fTacPerRev");
	PrintOffset(qOffset + offsetof(Internal, fVelCmd), qLevel, "fVelCmd");
	PrintOffset(qOffset + offsetof(Internal, fVelTar), qLevel, "fVelTar");
	PrintOffset(qOffset + offsetof(Internal, wDirAct), qLevel, "wDirAct");
	PrintOffset(qOffset + offsetof(Internal, wDirCmd), qLevel, "wDirCmd");
	PrintOffset(qOffset + offsetof(Internal, lTacCnt), qLevel, "lTacCnt");
	PrintOffset(qOffset + offsetof(Internal, lTacCntLast), qLevel, "lTacCntLast");
	PrintOffset(qOffset + offsetof(Internal, lEndCnt), qLevel, "lEndCnt");
	PrintOffset(qOffset + offsetof(Internal, fAccAmp), qLevel, "fAccAmp");
	PrintOffset(qOffset + offsetof(Internal, wCtrl), qLevel, "wCtrl");
	PrintOffset(qOffset + offsetof(Internal, wCommIndex), qLevel, "wCommIndex");
	PrintOffset(qOffset + offsetof(Internal, wJogIndex), qLevel, "wJogIndex");
	PrintOffset(qOffset + offsetof(Internal, fProLast), qLevel, "fProLast");
	PrintOffset(qOffset + offsetof(Internal, fIntTerm), qLevel, "fIntTerm");
	PrintOffset(qOffset + offsetof(Internal, fDerTerm), qLevel, "fDerTerm");
	PrintOffset(qOffset + offsetof(Internal, wDwellCnt), qLevel, "wDwellCnt");
	PrintOffset(qOffset + offsetof(Internal, wDirFaultCtr), qLevel, "wDirFaultCtr");
	PrintOffset(qOffset + offsetof(Internal, sDirFaultCnt), qLevel, "sDirFaultCnt");
	PrintOffset(qOffset + offsetof(Internal, wMilliSec), qLevel, "wMilliSec");
 	PrintOffset(qOffset + offsetof(Internal, sXWinLock), qLevel, "sXWinLock");
	PrintOffset(qOffset + offsetof(Internal, wProIndex), qLevel, "wProIndex");
	PrintOffset(qOffset + offsetof(Internal, wSeqIndex), qLevel, "wSeqIndex");
	PrintOffset(qOffset + offsetof(Internal, wSeqCount), qLevel, "wSeqCount");
	PrintOffset(qOffset + offsetof(Internal, fdT), qLevel, "fdT");
	PrintOffset(qOffset + offsetof(Internal, fdS), qLevel, "fdS");
	PrintOffset(qOffset + offsetof(Internal, fT), qLevel, "fT");
	PrintOffset(qOffset + offsetof(Internal, fS), qLevel, "fS");
	PrintOffset(qOffset + offsetof(Internal, fV), qLevel, "fV");
	PrintOffset(qOffset + offsetof(Internal, fA), qLevel, "fA");
	PrintOffset(qOffset + offsetof(Internal, tStop), qLevel, "tStop");
}

void PrintAsync(SnQByte qOffset, SnQByte qLevel, const char *pcHeader)
{
	PrintHeader(qLevel++, pcHeader);

	PrintExternal(qOffset + offsetof(Async, tEx), qLevel, "tEx");
	PrintInternal(qOffset + offsetof(Async, tIn), qLevel, "tIn");
	PrintOffset(qOffset + offsetof(Async, tMotorProfile), qLevel, "tMotorProfile");
}

void PrintHallBus(SnQByte qOffset, SnQByte qLevel, const char *pcHeader)
{
	PrintHeader(qLevel++, pcHeader);

	PrintOffset(qOffset + offsetof(HallBus, wDeviceExist), qLevel, "wDeviceExist");
	PrintOffset(qOffset + offsetof(HallBus, wDeviceActive), qLevel, "wDeviceActive");
	PrintOffset(qOffset + offsetof(HallBus, wDeviceLatch), qLevel, "wDeviceLatch");
}

void PrintDigitalIn(SnQByte qOffset, SnQByte qLevel, const char *pcHeader)
{
	PrintHeader(qLevel++, pcHeader);

	PrintOffset(qOffset + offsetof(DigitalIn, wStateData), qLevel, "wStateData");
	PrintOffset(qOffset + offsetof(DigitalIn, wNewData), qLevel, "wNewData");
	PrintOffset(qOffset + offsetof(DigitalIn, wOldData), qLevel, "wOldData");
	PrintOffset(qOffset + offsetof(DigitalIn, wActive), qLevel, "wActive");
	PrintOffset(qOffset + offsetof(DigitalIn, wAssert), qLevel, "wAssert");
	PrintOffset(qOffset + offsetof(DigitalIn, wActiveLow), qLevel, "wActiveLow");
	PrintOffset(qOffset + offsetof(DigitalIn, wInputType), qLevel, "wInputType");
	PrintOffset(qOffset + offsetof(DigitalIn, wDebounce), qLevel, "wDebounce");
	PrintOffset(qOffset + offsetof(DigitalIn, wEvent), qLevel, "wEvent");
}

void PrintAnalogIn(SnQByte qOffset, SnQByte qLevel, const char *pcHeader)
{
	PrintHeader(qLevel++, pcHeader);

	PrintOffset(qOffset + offsetof(AnalogIn, sData), qLevel, "sData");
	PrintOffset(qOffset + offsetof(AnalogIn, sAverage), qLevel, "sAverage");
	PrintOffset(qOffset + offsetof(AnalogIn, sOffset), qLevel, "sOffset");
	PrintOffset(qOffset + offsetof(AnalogIn, wInvert), qLevel, "wInvert");
	PrintOffset(qOffset + offsetof(AnalogIn, wCount), qLevel, "wCount");
	PrintOffset(qOffset + offsetof(AnalogIn, fGain), qLevel, "fGain");
	PrintOffset(qOffset + offsetof(AnalogIn, wAssert), qLevel, "wAssert");
}

void PrintTemperature(SnQByte qOffset, SnQByte qLevel, const char *pcHeader)
{
	PrintHeader(qLevel++, pcHeader);

	PrintOffset(qOffset + offsetof(Temperature, fOnBoardTemp), qLevel, "fOnBoardTemp");
	PrintOffset(qOffset + offsetof(Temperature, fOnBoardHiLimit), qLevel, "fOnBoardHiLimit");
	PrintOffset(qOffset + offsetof(Temperature, fOnBoardLoLimit), qLevel, "fOnBoardLoLimit");
	PrintOffset(qOffset + offsetof(Temperature, fDspTemp), qLevel, "fDspTemp");
	PrintOffset(qOffset + offsetof(Temperature, fDspLimit), qLevel, "fDspLimit");
	PrintOffset(qOffset + offsetof(Temperature, wPriority), qLevel, "wPriority");
	PrintOffset(qOffset + offsetof(Temperature, wEvent), qLevel, "wEvent");
}

void PrintSysInterrupt(SnQByte qOffset, SnQByte qLevel, const char *pcHeader)
{
	PrintHeader(qLevel++, pcHeader);

	PrintOffset(qOffset + offsetof(SysInterrupt, wEnable), qLevel, "wEnable");
	PrintOffset(qOffset + offsetof(SysInterrupt, wFlags), qLevel, "wFlags");
	PrintOffset(qOffset + offsetof(SysInterrupt, wLocal), qLevel, "wLocal");
}

int main(int argc, char *argv[])
{
	printf("\n****  EIP Motor Controller ****\n\n\n");

	PrintHeader(0, "Status_Control");
 
	PrintOffset(offsetof(Status_Control, qVersion), 1, "qVersion");
	PrintOffset(offsetof(Status_Control, wPortType), 1, "wPortType");
	PrintAsync(offsetof(Status_Control, tBldcA), 1, "tBldcA");
	PrintAsync(offsetof(Status_Control, tBldcB), 1, "tBldcB");
	PrintHallBus(offsetof(Status_Control, tHallBusA), 1, "tHallBusA");
	PrintHallBus(offsetof(Status_Control, tHallBusB), 1, "tHallBusB");
	PrintDigitalIn(offsetof(Status_Control, tDigital), 1, "tDigital");
	PrintAnalogIn(offsetof(Status_Control, tAnalog), 1, "tAnalog");
	PrintTemperature(offsetof(Status_Control, tTemperature), 1, "tTemperature");
	PrintSysInterrupt(offsetof(Status_Control, tInterrupt), 1, "tInterrupt");
	PrintOffset(offsetof(Status_Control, wHeartCount), 1, "wHeartCount");
	PrintOffset(offsetof(Status_Control, wAuxillary), 1, "wAuxillary");
	PrintOffset(offsetof(Status_Control, wBuffer), 1, "wBuffer");
}
