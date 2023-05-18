<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: MDU_Tester - Win32 (WCE ARMV4I) Release--------------------
</h3>
<h3>Command Lines</h3>
Creating command line "rc.exe /l 0x409 /fo"ARMV4IRel/MDU_Tester.res" /d UNDER_CE=500 /d _WIN32_WCE=500 /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "WCE_PLATFORM_Basic" /d "THUMB" /d "_THUMB_" /d "ARM" /d "_ARM_" /d "ARMV4I" /r "C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\MDU_Tester.rc"" 
Creating temporary file "C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP557.tmp" with contents
[
/nologo /W3 /Oxt /I "." /D "ARM" /D "_ARM_" /D "ARMV4I" /D UNDER_CE=500 /D _WIN32_WCE=500 /D "WCE_PLATFORM_Basic" /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /FR"ARMV4IRel/" /Fp"ARMV4IRel/MDU_Tester.pch" /Yu"stdafx.h" /Fo"ARMV4IRel/" /QRarch4T /QRinterwork-return /MC /c 
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\BitButton.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\Control.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\CProcedureRegion.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\CPump.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\CSerialPort.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\CustomOscScreen.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\Driver.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\FactoryScreen.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\Fifo.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\FootSwitch.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\FootSwitchScreen.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\HandpieceCountPopUpPortA.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\HandpieceCountPopUpPortB.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\IconStatusBar.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\IntellioLink.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\IntellioShaver.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\LanguagePopUp.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\LanguageScreen.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\Logger.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\MessageBox.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\MsgList.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\OscillationProfileScreen.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\Port.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\ProcedureScreen.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\RemoteSettings.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\SerialNumberPopUp.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\SettingsScreen.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\SharedMemory.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\Shaver.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\SlidingWindowFilter.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\SnHelp.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\SystemErrorDlg.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\SystemInfoScreen.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\TestMode.cpp"
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\Util.cpp"
]
Creating command line "clarm.exe @C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP557.tmp" 
Creating temporary file "C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP558.tmp" with contents
[
/nologo /W3 /Oxt /I "." /D "ARM" /D "_ARM_" /D "ARMV4I" /D UNDER_CE=500 /D _WIN32_WCE=500 /D "WCE_PLATFORM_Basic" /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /FR"ARMV4IRel/" /Fp"ARMV4IRel/MDU_Tester.pch" /Yc"stdafx.h" /Fo"ARMV4IRel/" /QRarch4T /QRinterwork-return /MC /c 
"C:\DII-EIP_VSS_LocalDirectory\Software\Applications\MDU_Tester\StdAfx.cpp"
]
Creating command line "clarm.exe @C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP558.tmp" 
Creating temporary file "C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP559.tmp" with contents
[
/nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"wWinMainCRTStartup" /incremental:no /pdb:"ARMV4IRel/MDU_Tester.pdb" /out:"ARMV4IRel/MDU_Tester.exe" /subsystem:windowsce,5.00 /MACHINE:THUMB 
".\ARMV4IRel\BitButton.obj"
".\ARMV4IRel\Control.obj"
".\ARMV4IRel\CProcedureRegion.obj"
".\ARMV4IRel\CPump.obj"
".\ARMV4IRel\CSerialPort.obj"
".\ARMV4IRel\CustomOscScreen.obj"
".\ARMV4IRel\Driver.obj"
".\ARMV4IRel\FactoryScreen.obj"
".\ARMV4IRel\Fifo.obj"
".\ARMV4IRel\FootSwitch.obj"
".\ARMV4IRel\FootSwitchScreen.obj"
".\ARMV4IRel\HandpieceCountPopUpPortA.obj"
".\ARMV4IRel\HandpieceCountPopUpPortB.obj"
".\ARMV4IRel\IconStatusBar.obj"
".\ARMV4IRel\IntellioLink.obj"
".\ARMV4IRel\IntellioShaver.obj"
".\ARMV4IRel\LanguagePopUp.obj"
".\ARMV4IRel\LanguageScreen.obj"
".\ARMV4IRel\Logger.obj"
".\ARMV4IRel\MessageBox.obj"
".\ARMV4IRel\MsgList.obj"
".\ARMV4IRel\OscillationProfileScreen.obj"
".\ARMV4IRel\Port.obj"
".\ARMV4IRel\ProcedureScreen.obj"
".\ARMV4IRel\RemoteSettings.obj"
".\ARMV4IRel\SerialNumberPopUp.obj"
".\ARMV4IRel\SettingsScreen.obj"
".\ARMV4IRel\SharedMemory.obj"
".\ARMV4IRel\Shaver.obj"
".\ARMV4IRel\SlidingWindowFilter.obj"
".\ARMV4IRel\SnHelp.obj"
".\ARMV4IRel\StdAfx.obj"
".\ARMV4IRel\SystemErrorDlg.obj"
".\ARMV4IRel\SystemInfoScreen.obj"
".\ARMV4IRel\TestMode.obj"
".\ARMV4IRel\Util.obj"
".\ARMV4IRel\MDU_Tester.res"
]
Creating command line "link.exe @C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP559.tmp"
<h3>Output Window</h3>
Compiling resources...
Compiling...
StdAfx.cpp
Compiling...
BitButton.cpp
Control.cpp
CProcedureRegion.cpp
CPump.cpp
CSerialPort.cpp
CustomOscScreen.cpp
Driver.cpp
FactoryScreen.cpp
Fifo.cpp
FootSwitch.cpp
FootSwitchScreen.cpp
HandpieceCountPopUpPortA.cpp
HandpieceCountPopUpPortB.cpp
IconStatusBar.cpp
IntellioLink.cpp
IntellioShaver.cpp
LanguagePopUp.cpp
LanguageScreen.cpp
Logger.cpp
MessageBox.cpp
Generating Code...
Compiling...
MsgList.cpp
OscillationProfileScreen.cpp
Port.cpp
ProcedureScreen.cpp
RemoteSettings.cpp
SerialNumberPopUp.cpp
SettingsScreen.cpp
SharedMemory.cpp
Shaver.cpp
SlidingWindowFilter.cpp
SnHelp.cpp
SystemErrorDlg.cpp
SystemInfoScreen.cpp
TestMode.cpp
Util.cpp
Generating Code...
Linking...
   Creating library ARMV4IRel/MDU_Tester.lib and object ARMV4IRel/MDU_Tester.exp
LINK : warning LNK4089: all references to 'WININET.dll' discarded by /OPT:REF
LINK : warning LNK4089: all references to 'commdlg.dll' discarded by /OPT:REF




<h3>Results</h3>
MDU_Tester.exe - 0 error(s), 2 warning(s)
</pre>
</body>
</html>
