@echo off
echo Programming Dsp FLASH...
xcopy "..\..\dsp\boot\output\sdm_pROM-xRAM.elf.S" "..\DII_EIP_Flash\Motor Controller Boot.elf.S" 
xcopy "..\..\dsp\output\sdm_pROM-xRAM.elf.S" "..\DII_EIP_Flash\Motor Controller Application.elf.S"

