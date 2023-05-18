  
  


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

	XREF	F_stack_addr

	
	org	p:

	
	GLOBAL Finit_MC56F83xx_

	SUBROUTINE "Finit_MC56F83xx_",Finit_MC56F83xx_,Finit_MC56F83xx_END-Finit_MC56F83xx_

Finit_MC56F83xx_:


; for internal memory target with pROM-to-xRAM copy 
; bootflash RESET and COP both restart init code
; startup code is generalized so no change for external targets
; this is just one example of bootflash use

; 0x20000 bootflash RESET
	nop     
	nop     
; 0x20002 bootflash COP reset 


; setup the OMr with the values required by C

	bfset	#NL_MODE,omr    ; ensure NL=1  (enables nsted DO loops)
	nop
	nop
	
	; ensure CM=0  (optional for C)
    ; ensure XP=0 to enable harvard architecture
    ; ensure R=0  (required for C)
    ; ensure SA=0 (required for C)
    
	bfclr	#(CM_MODE|XP_MODE|R_MODE|SA_MODE),omr		




; setup the m01 register for linear addressing

	move.w	#-1,x0
	moveu.w	x0,m01      ; set the m register to linear addressing
				    
	moveu.w	hws,la      ; clear the hardware stack
	moveu.w	hws,la
	nop
	nop





CALLMAIN:                           ; initialize compiler environment

                                    ; initialize the stack
	move.l #>>F_Lstack_addr,r0
	bftsth #$0001,r0
	bcc noinc
	adda #1,r0
noinc:
	tfra	r0,sp				    ; set stack pointer too
	move.w	#0,r1
	nop
	move.w	r1,x:(sp)               ; put zero at top of stack
	adda	#1,sp	                ; increment stack
	move.w	r1,x:(sp)               ; put another zero
	
	

	 	    
; setup the PLL (phase locked loop)
								
    nop 
    bfclr #$0006,x:>>PLLCR          ; select prescaler 
    bfset #$0001,x:>>PLLCR 

    bfset #$0010,x:>>PLLCR          ; power down PLL 

    nop 
    bfset #speedIndex,x:>>PLLDB     ; set speed index
    nop 

    bfset #$0080,x:>>PLLCR          ; set lock detector on 
    bfclr #$0010,x:>>PLLCR          ; power on PLL 

    nop 

wait_for_lock: 
    bftsth #$0020,x:>>PLLSR         ; wait for lock 
    jcc wait_for_lock                     
    nop 
                
    move.w #$0082,x:>>PLLCR         ; switch system clock to PLL postscalar                   



; utilities

	jsr		F__init_sections        ; clear BSS (this routine defined in RT lib)	 
	jsr     F__romCopy              ; do any rom-to-ram copy
	                                ; LCF sets rom-to-ram flag 
	


; call main()

	move.w	#0,y0                   ; pass parameters to main()
	move.w	#0,R2
	move.w	#0,R3

	jsr	 	Fmain                   ; call the users program



                                    ; end of program
                                    
                                    ; hostIO cleanup with exit_halt
    jsr     Fexit_halt              ; stationery examples use standard i/o 
                                    ; such as printf implemented with hostIO 
                                    ; so we use exit_halt here 
                                    ; see runtime file exit_dsp.asm
                                    ; comment out exit_halt if not using hostIO
                                                                     
    
    debughlt                        ; or simple end of program; halt CPU	
	rts 
Finit_MC56F83xx_END:

	endsec
	
	
	
	
	