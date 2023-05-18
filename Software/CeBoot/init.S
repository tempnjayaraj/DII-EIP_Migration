#define TOP_OF_SRAM         0x314000    // Top of SRAM used for Stack
#define START_OF_SRAM       0x300000    // Start of SRAM

    .global ResetVector

    .text
    .align 4

/*
 * The entry point of a harware reset, jump to the ResetHandler
 */
ResetVector:
    b       ResetHandler
UndefVector:
    b       ClearModeRegister
SwiVector:
    b       SwiVector
PrefetchVector:
    b       PrefetchVector
AbortVector:
    b       AbortVector
ResevedVector:
    b       ResevedVector
IrqVector:
    b       IrqVector
FiqVector:
    b       FiqVector

Addr_SMC0_BASE:
    .word   0xFFFFE400
#ifdef BEEP
Addr_PIOB_BASE:
	.word	0xFFFFF400
#endif
Addr_PMC_BASE:
    .word   0xFFFFFC00
CkgrPllA:
	.word   0x2544bf5c
Addr_RSTC_RMR:
    .word   0xFFFFFD08
SmcPulse:
    .word   0x14140707
SmcCycle:
    .word   0x00140014
SmcCtrl:
    .word   0x10001000
Bss:
    .word   _bss

ClearModeRegister:
    ldr     r0, Addr_RSTC_RMR
    mov     r1, #0xA5000000
    str     r1, [r0]

ResetHandler:
#ifdef BEEP
	mov 	r1, #0x00040000
	ldr 	r0, Addr_PIOB_BASE
	str     r1, [r0, #0x30]                 /* PIOB_SODR */
	str     r1, [r0, #0x10]                 /* PIOB_OER */
#endif

/* Init the stack */
    mov     sp, #TOP_OF_SRAM

/* Enable Icache to speed up copy to SRAM */
    mrc     p15, 0, r0, c1, c0, 0
    orr     r0, r0, #0x1000
    mcr     p15, 0, r0, c1, c0, 0

/* Copy minimal boot to SRAM */
    mov     r0, #0                          /* NOR FLASH mapped to 0 on boot */
    mov     r1, #START_OF_SRAM 
    mov     r2, #StartMainBoot
CopyMinBootToSRAMLoop:
	ldmia	r0!, {r3-r10}
	stmia	r1!, {r3-r10}
	cmp     r0, r2
	ble     CopyMinBootToSRAMLoop

/* Jump to the SRAM copy of code */
    mov     r0, #CopiedMinBootToSRAM
    add     r0, r0, #START_OF_SRAM
    mov     pc, r0

CopiedMinBootToSRAM:
    ldr     r0, Addr_PMC_BASE

/* Enable the main oscillator */
    mov     r1, #(64 << 8)                  /* Delay 64 Slow Clocks */
    mov     r2, #0x1                        /* Enable Main Oscillator */
    orr     r1, r1, r2
    str     r1, [r0, #0x20]                 /* PMC_MOR */

WaitForMainOscillator:
    ldr     r1, [r0, #0x68]                 /* PMC_SR */
    mov     r2, #0x1                        /* Main Oscillator is Ready */
    ands    r1, r1, r2
    beq     WaitForMainOscillator
    
/* Configure PLLA */
    ldr     r1, CkgrPllA
    str     r1, [r0, #0x28]                 /* CKGR_PLLAR */
WaitForPLLA:
    ldr     r1, [r0, #0x68]                 /* PMC_SR */
    mov     r2, #(1<< 1)
    ands    r1, r1, r2
    beq     WaitForPLLA

/* Switch MCK on PLLA output */
    mov     r1, #0x00000100
    mov     r2, #0x00000002
    orr     r1, r1, r2
    str     r1, [r0, #0x30]                 /* PMC_MCKR */
WaitForMCKSwitch:
    ldr     r1, [r0, #0x68]                 /* PMC_SR */
    mov     r2, #(1<< 3)
    ands    r1, r1, r2
    beq     WaitForMCKSwitch
    
CopyRestOfBoot:
    ldr     r0, Addr_SMC0_BASE
    mov     r1, #0
    str     r1, [r0]                        /* SMC0_SETUP */
    ldr     r1, SmcPulse
    str     r1, [r0, #4]                    /* SMC0_PULSE */
    ldr     r1, SmcCycle
    str     r1, [r0, #8]                    /* SMC0_CYCLE */
    ldr     r1, SmcCtrl
    str     r1, [r0, #12]                   /* SMC0_MODE */

/* Copy the rest of boot to SRAM */
    mov     r0, #0x10000000                 /* NOR FLASH (non-mapped) */
    orr     r0, r0, #StartMainBoot 
    mov     r1, #START_OF_SRAM
    orr     r1, r1, #StartMainBoot 
    ldr     r2, Bss
CopyBootToSRAMLoop:
	ldmia	r0!, {r3-r10}
	stmia	r1!, {r3-r10}
	cmp     r1, r2
	ble     CopyBootToSRAMLoop

/* Invalidate Icache to make sure we execute the just copied code (StartMainBoot) */
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 0

StartMainBoot:
/* Zero out the bss section */
    ldr     r1, Bss
    ldr     r2, Ebss
    mov     r0, #0

BssZeroLoop:
    str     r0, [r1], #4
    cmp     r1, r2
    ble     BssZeroLoop

#ifdef BEEP
	mov 	r1, #0x00040000
	ldr 	r0, Addr_PIOB_BASE
	str     r1, [r0, #0x34]                 /* PIOB_CODR */
#endif

/* Branch on C code Main function (with interworking) */
    ldr     r4, Main
    mov     lr, pc
    bx      r4
ExitMain:
    b       ExitMain

Ebss:
    .word   _ebss
Main:
    .word   main
