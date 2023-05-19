
; PLL defines

CLKGEN      EQU $00F2D0
TESTR       EQU $00F2D3 ; CLKGEN + 3  ; test register
PLLSR       EQU $00F2D2 ; CLKGEN + 2  ; pll status register
PLLDB       EQU $00F2D1 ; CLKGEN + 1  ; pll divide-by register
PLLCR       EQU $00F2D0 ; CLKGEN + 0  ; pll control register


; OMR mode bits

NL_MODE			EQU		$8000
CM_MODE			EQU		$0100
XP_MODE	 		EQU		$0080
R_MODE	 		EQU		$0020
SA_MODE	 		EQU		$0010


	section startup

	XREF	F_stack_addr
	XREF	F_pROM_to_xRAM
	XREF	F_Ldata_size
	XREF	F_Ldata_ROM_addr
	XREF    F_Ldata_RAM_addr
	org	p:

	
	GLOBAL Finit_MC56F835x_

	SUBROUTINE "Finit_MC56F835x_",Finit_MC56F835x_,Finit_MC56F835x_END-Finit_MC56F835x_

Finit_MC56F835x_:

; for internal memory target with pROM-to-xRAM copy 
; bootflash RESET and COP both restart init code
; startup code is generalized so no change for external targets
; this is just one example of bootflash use

; 0x20000 bootflash RESET
	nop     
	nop     
; 0x20002 bootflash COP reset 

; setup the OMr with the values required by C

	bfset	#NL_MODE,omr    ; ensure NL=1  (enables nested DO loops)
	nop
	nop
	bfclr	#(CM_MODE|XP_MODE|R_MODE|SA_MODE),omr		; ensure CM=0  (optional for C)
														; ensure XP=0 to enable harvard architecture
														; ensure R=0  (required for C)
														; ensure SA=0 (required for C)

	nop ;UH
	nop ;UH


; setup the m01 register for linear addressing

	move.w	#-1,x0
	moveu.w	x0,m01          ; set the m register to linear addressing
				    
	moveu.w	hws,la          ; clear the hardware stack
	moveu.w	hws,la
	nop
	nop


CALLMAIN:                           ; initialize compiler environment

                                    ; initialize the Stack
	move.l #>>F_Lstack_addr,r0
	bftsth #$0001,r0
	bcc noinc
	adda #1,r0
noinc:
	tfra	r0,sp				    ; set stack pointer too
	move.w	#0,r1
	nop
	move.w	r1,x:(sp)
	adda	#1,sp	
	
	
	jsr		F__init_sections        ; clear BSS
	 
 	    
; setup the PLL (phase locked loop)
								
    nop 
	move.l #>>PLLCR,r0
	move.w #$0082,x:(r0)

    nop 
	move.l #>>PLLDB,r0
	move.w #$0277,x:(r0)
    nop 

wait_for_lock: 
    bftsth #$0020,x:>>PLLSR         ; wait for lock 
    jcc wait_for_lock                     
    nop 
	
	
; optimized for apps with non-zero data size
	
; pROM-to-xRAM utility	

    move.l  #F_Ldata_size,r2        ; set data size
    move.l  #F_Ldata_ROM_addr,r3    ; src address -- pROM data start
    move.l  #F_Ldata_RAM_addr,r1    ; dest address -- xRAM data start
    do      r2,>>end_prom2xram      ; copy for r2 times
    move.w  p:(r3)+,x0              ; fetch value at address r3
    nop
    nop
    nop
    move.w  x0,x:(r1)+              ; stash value at address r1

end_prom2xram:

; call main()

	; pass parameters to main()
	;move.w	#0,y0					; Retain y0 passed in by boot app
	move.w	#0,R2
	move.w	#0,R3

	jsr	 	Fmain                   ; call the users program

    debughlt                        ; simple end of program; halt CPU	
	rts 

Finit_MC56F835x_END:

	endsec