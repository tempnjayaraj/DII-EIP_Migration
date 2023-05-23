  
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
	
	GLOBAL Fcontinue_init_

	SUBROUTINE "Fcontinue_init_",Fcontinue_init_,Fcontinue_init_END-Fcontinue_init_

Fcontinue_init_:
; setup the OMr with the values required by C
	bfset	#NL_MODE,omr    ; ensure NL=1  (enables nsted DO loops)
	nop
	nop
	
	; ensure CM=0  (optional for C)
    ; ensure XP=0 to enable harvard architecture
    ; ensure R=0  (required for C)
    ; ensure SA=0 (required for C)
    
	bfclr	#(CM_MODE|XP_MODE|R_MODE|SA_MODE),omr		
	nop ;UH
	nop ;UH




; setup the m01 register for linear addressing

	move.w	#-1,x0
	moveu.w	x0,m01      ; set the m register to linear addressing
				    
	moveu.w	hws,la      ; clear the hardware stack
	moveu.w	hws,la
	nop
	nop




	 	    
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
                
    move.w #$0082,x:>>PLLCR         ; switch system clock to PLL postscalar                   
    jmp LRamDone

;----------------------------------------------------------------------------------------------------------------------

; RAM test
    ;------------------------------------------
    ; Write program space RAM with 0x5555
    moveu.w     #2048,LC    ; need two instructions between loading lc and doslc
    move.l      #$2f800,R2
    move.w      #$5555,Y0
    doslc       LPW5
    move.w      Y0,P:(R2)+
LPW5:

    ; Check that program space RAM is all 0x5555
    moveu.w     #2048,LC    ; need two instructions between loading lc and doslc
    move.l      #$2f800,R2
    nop
    doslc       LPR5
    move.w      P:(R2)+,X0
    cmp.w       #$5555,X0
    jne         LRamError
LPR5:

    ;------------------------------------------
    ; Write program space RAM with 0xAAAA
    moveu.w     #2048,LC    ; need two instructions between loading lc and doslc
    move.l      #$2f800,R2
    move.w      #$AAAA,Y0
    doslc       LPWA
    move.w      Y0,P:(R2)+
LPWA:

    ; Check that program space RAM is all 0xAAAA
    moveu.w     #2048,LC    ; need two instructions between loading lc and doslc
    move.l      #$2f800,R2
    nop
    doslc       LPRA
    move.w      P:(R2)+,X0
    cmp.w       #$AAAA,X0
    jne         LRamError
LPRA:

    ;------------------------------------------
    ; Write program space RAM with 0xFFFF
    moveu.w     #2048,LC    ; need two instructions between loading lc and doslc
    move.l      #$2f800,R2
    move.w      #$FFFF,Y0
    doslc       LPWF
    move.w      Y0,P:(R2)+
LPWF:

    ; Check that program space RAM is all 0xFFFF
    moveu.w     #2048,LC    ; need two instructions between loading lc and doslc
    move.l      #$2f800,R2
    nop
    doslc       LPRF
    move.w      P:(R2)+,X0
    cmp.w       #$FFFF,X0
    jne         LRamError
LPRF:    

;------------------------------------------
    ; Write program space RAM with 0x0000
    moveu.w     #2048,LC    ; need two instructions between loading lc and doslc
    move.l      #$2f800,R2
    move.w      #$0000,Y0
    doslc       LPW0
    move.w      Y0,P:(R2)+
LPW0:

    ; Check that program space RAM is all 0x0000
    moveu.w     #2048,LC    ; need two instructions between loading lc and doslc
    move.l      #$2f800,R2
    nop
    doslc       LPR0
    move.w      P:(R2)+,X0
    cmp.w       #$0000,X0
    jne         LRamError
LPR0:

    ;------------------------------------------
    ; Write data space RAM with 0x5555
    moveu.w     #8192,LC    ; need two instructions between loading lc and doslc
    move.l      #$0,R2
    move.w      #$5555,Y0
    doslc       LDW5
    move.w      Y0,X:(R2)+
LDW5:

    ; Check that data space RAM is all 0x5555
    moveu.w     #8192,LC    ; need two instructions between loading lc and doslc
    move.l      #$0000,R2
    nop
    doslc       LDR5
    move.w      X:(R2)+,X0
    cmp.w       #$5555,X0
    jne         LRamError
LDR5:

    ;------------------------------------------
    ; Write data space RAM with 0xAAAA
    moveu.w     #8192,LC    ; need two instructions between loading lc and doslc
    move.l      #$0,R2
    move.w      #$AAAA,Y0
    doslc       LDWA
    move.w      Y0,X:(R2)+
LDWA:

    ; Check that data space RAM is all 0xAAAA
    moveu.w     #8192,LC    ; need two instructions between loading lc and doslc
    move.l      #$0000,R2
    nop
    doslc       LDRA
    move.w      X:(R2)+,X0
    cmp.w       #$AAAA,X0
    jne         LRamError
LDRA:

    ;------------------------------------------
    ; Write data space RAM with 0xFFFF
    moveu.w     #8192,LC    ; need two instructions between loading lc and doslc
    move.l      #$0,R2
    move.w      #$FFFF,Y0
    doslc       LDWF
    move.w      Y0,X:(R2)+
LDWF:

    ; Check that data space RAM is all 0xFFFF
    moveu.w     #8192,LC    ; need two instructions between loading lc and doslc
    move.l      #$0000,R2
    nop
    doslc       LDRF
    move.w      X:(R2)+,X0
    cmp.w       #$FFFF,X0
    jne         LRamError
LDRF:

;------------------------------------------
    ; Write data space RAM with 0x0000
    moveu.w     #8192,LC    ; need two instructions between loading lc and doslc
    move.l      #$0,R2
    move.w      #$0000,Y0
    doslc       LDW0
    move.w      Y0,X:(R2)+
LDW0:

    ; Check that data space RAM is all 0x0000
    moveu.w     #8192,LC    ; need two instructions between loading lc and doslc
    move.l      #$0000,R2
    nop
    doslc       LDR0
    move.w      X:(R2)+,X0
    cmp.w       #$0000,X0
    jne         LRamError
LDR0:

    ;------------------------------------------
    ; Success
    bra LRamDone

LRamError:
    ;------------------------------------------
    ; Here on error
    enddo
    debughlt
    rts

LRamDone:
    ;------------------------------------------
    ; Here when RAM test passes


;----------------------------------------------------------------------------------------------------------------------


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

	jsr		F__init_sections        ; clear BSS (this routine defined in RT lib)	 	


; Copy data value to SRAM	

    move.l  #F_Ldata_size,r2        ; count
    move.l  #F_Ldata_ROM_addr,r3    ; src address  -- ROM data start
    move.l  #F_Ldata_RAM_addr,r1    ; dest address -- RAM data start    
    nop
    
    do      r2,>>end_romCopy        ; copy for y0 times
    move.w  p:(r3)+,x0              ; fetch p value at address r3
    nop
    nop
    nop
    move.w  x0,x:(r1)+              ; stash value at x address r1   
end_romCopy:

; call main()

	move.w	#0,y0                   ; pass parameters to main()
	move.w	#0,R2
	move.w	#0,R3

	jsr	 	Fmain                   ; call the users program
    
    debughlt                        ; or simple end of program; halt CPU	
	rts 
Fcontinue_init_END:

	endsec
	
	
	
	
	