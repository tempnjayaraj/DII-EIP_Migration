; metrowerks sample code

; PLL defines

CLKGEN      EQU $00F2D0
TESTR       EQU $00F2D3 ; CLKGEN + 3  ; test register
PLLSR       EQU $00F2D2 ; CLKGEN + 2  ; pll status register
PLLDB       EQU $00F2D1 ; CLKGEN + 1  ; pll divide-by register
PLLCR       EQU $00F2D0 ; CLKGEN + 0  ; pll control register

speedIndex  EQU $001D

; OMR mode bits

NL_MODE			EQU		$8000
CM_MODE			EQU		$0100
XP_MODE	 		EQU		$0080
R_MODE	 		EQU		$0020
SA_MODE	 		EQU		$0010

; init

	section startup
	
	org	p:

	XREF	F_stack_addr
	XREF	Fcontinue_init_
	
	GLOBAL Finit_MC56F83xx_

	SUBROUTINE "Finit_MC56F83xx_",Finit_MC56F83xx_,Finit_MC56F83xx_END-Finit_MC56F83xx_
	
; NOTE: This symbol MUST be the first symbol in this file, the linker command file assumes it when jumping here
Finit_MC56F83xx_:


; for internal memory target with pROM-to-xRAM copy 
; bootflash RESET and COP both restart init code
; startup code is generalized so no change for external targets
; this is just one example of bootflash use

; 0x20000 bootflash RESET     
; 0x20002 bootflash COP reset
	nop     
	nop
    jmp Fcontinue_init_
Finit_MC56F83xx_END:
	endsec
	
	
	
	
	