



# ----------------------------------------------------

# Metrowerks, a company of Motorola
# sample code

# linker command file for DSP56835x EVM
# using 
#    flash pROM
#    flash xROM 
#    internal xRAM 
#    Small Data Model (SDM)

# ----------------------------------------------------



# see end of file for additional notes & revision history
# see Motorola docs for specific EVM memory map



# memory use for this LCF: 
# interrupt vectors --> flash pROM starting at zero
#      program code --> flash pROM
#         constants --> flash xROM 
#      dynamic data --> flash pROM (copied to xRAM with init) 



# CodeWarrior preference settings:
#
#   M56800E Processor:
#     Small Program Model: OFF
#        Large Data Model: OFF
#
#   M56800E Assembler:
#        Default Data Memory Model: 16-bit
#     Default Program Memory Model: 19-bit
#
#
#   M56800E Target pref panel:
#     config file: 56835x_flash.cfg
#     (sets OMR for internal memory)


# EVM board jumpers:
#     JG4 on
#     (if not set by config file)




# 56835x memory map for:
#    Small Data Model (SDM)
#    x memory: EX=0 (OMR)
#    p memory: mode 0
 
 
# x memory above 7FFF not available with SDM
# see below for memory notes


MEMORY 
{
    .p_interrupts_ROM     (RX)  : ORIGIN = 0x0000,   LENGTH = 0x00A4   # reserved for interrupts
    .p_flash_ROM          (RX)  : ORIGIN = 0x00A4,   LENGTH = 0x1FF5C  # 128K words
 
     # p_flash_ROM_data mirrors internal xRAM, mapping to origin and length
     # the "X" flag in "RX" tells the debugger to download to p-memory.
     # the download to p-memory is directed to the address determined by AT
     # in the section definition below.  
     
    .p_flash_ROM_data     (RX)  : ORIGIN = 0x0001,   LENGTH = 0x1FFF   # internal xRAM mirror

    .p_boot_flash_ROM     (RX)  : ORIGIN = 0x20000,  LENGTH = 0x2000   #  8K words   
    .p_reserved           (RX)  : ORIGIN = 0x22000,  LENGTH = 0xF800   # 54K words
    .p_internal_RAM       (RWX) : ORIGIN = 0x2F800,  LENGTH = 0xD800   #  2K words  
    .p_external_RAM       (RWX) : ORIGIN = 0x30000,  LENGTH = 0x0000   # max available   
    .x_internal_RAM       (RW)  : ORIGIN = 0x0001,   LENGTH = 0x1FFF   # 8K x 16
    
    # The only way to force Fg_tBootInfo to be at the fixed address 0x2FF2 in data flash is to
    # break up the memory definition into two as shown here, and then adding a section to fill
    # up x_flash_ROM2 with Fg_tBootInfo
    .x_flash_ROM          (RW)  : ORIGIN = 0x2000,   LENGTH = 0x0FF2   # 4K x 16 - 0xE
    .x_flash_ROM2         (RW)  : ORIGIN = 0x2FF2,   LENGTH = 0xE	   # 0xE words
    .x_onchip_peripherals (R)   : ORIGIN = 0xF000,   LENGTH = 0x1000   # not accessible w/SDM
    .x_external_RAM_2     (RW)  : ORIGIN = 0x010000, LENGTH = 0xFEFF00 # n/a w/SDM
    .x_EOnCE              (R)   : ORIGIN = 0xFFFF00, LENGTH = 0x0000   # n/a w/SDM
}






# we ensure the interrupt vector section is not deadstripped here
# the label "interrupt_vectors" comes from file 56835x_vector.asm

KEEP_SECTION{ interrupt_vectors.text}

# Fg_tBootInfo should not be stripped because the boot code and the build upgrade tool use it
# and depend on it being at a fixed location
FORCE_ACTIVE{Fg_tBootInfo}





SECTIONS 
{
    .interrupt_code :
    {
        * (interrupt_vectors.text)
       
    } > .p_interrupts_ROM
    
	
	# The startup code is part of .executing_code i.e. in program flash rather than boot flash
	                     


	.executing_code :
	{
		# .text sections
		    
		* (startup.text)
		* (.text)		
		* (interrupt_routines.text)
		* (rtlib.text)
		* (fp_engine.text)
		* (user.text)	
		
		
		# save address where for the data start in pROM
		 
        . = ALIGN(2);
 	    __pROM_data_start = .;  
 	    
	} > .p_flash_ROM

	# This section fills in x_flash_ROM2 completely with Fg_tBootInfo. Since the linker discovers symbols as they
	# are declared, make sure that this section is declared before the x_flash_ROM section, otherwise the
	# glob '* (const.data) will swallow up Fg_tBootInfo, and this section will have no effect (silently)
	.data_in_xROM2 :
	{
        OBJECT(Fg_tBootInfo,Controller.c)
	} > .x_flash_ROM2
     
    .data_in_xROM : 
	{                      
	                   	
        # constant data is placed in x flash ROM
        
        # This must be the first object in data flash (4 words)

        * (.const.data.char)   # used if "Emit Separate Char Data Section" enabled
        * (.const.data)
	} > .x_flash_ROM
	    	 

    



# AT sets the download address
# the download stashes the data just after the program code
# as determined by our saved pROM data start variable

	.data_in_p_flash_ROM : AT(__pROM_data_start) 
	{                             
	    # the data sections flashed to pROM
	    # save data start so we can copy data later to xRAM
	    
 	    __xRAM_data_start = .; 
 	    
 	    
        * (.data.char)  # used if "Emit Separate Char Data Section"    
        * (.data)	    
	    * (fp_state.data)
		* (rtlib.data)
 	    
 	    
 	    # save data end and calculate data block size
 	    
 	    . = ALIGN(1);       # ensure _data_size is word-aligned
 	                        # since rom-to-ram copy is by word
 	                        # and we could have odd-number bytes
 	                        # in .data section since 56800E 
 	                        # has byte addressing

		__xRAM_data_end = .;
		__data_size = __xRAM_data_end - __xRAM_data_start;

	} > .p_flash_ROM_data	 # this section is designated as p-memory 
	                         # with X flag in the memory map
	                         # the start address and length map to 
	                         # actual internal xRAM
	
	
		
		
		
	.data : 
	{                             

        # save space for the pROM data copy 
        
        . = __data_size +1 ;

		        
        # .bss sections
        
        * (rtlib.bss.lo)
        * (rtlib.bss)
        . = ALIGN(1);
        _START_BSS = .;
        * (.bss.char)         # used if "Emit Separate Char Data Section" enabled
        * (.bss)
        _END_BSS   = .;



        # setup the heap address 
        
        . = ALIGN(4);
        _HEAP_ADDR = .;
        _HEAP_SIZE = 0x100;
        _HEAP_END = _HEAP_ADDR + _HEAP_SIZE; 
        . = _HEAP_END;



        # setup the stack address 
        
        _min_stack_size = 0x200;
        _stack_addr = _HEAP_END;
        _stack_end  = _stack_addr + _min_stack_size;
        . = _stack_end;
        
        
        
        # used by MSL 
            
        F_heap_addr   = _HEAP_ADDR;
        F_heap_end    = _HEAP_END;
        
     
        
	# stationery init code uses these globals:

        F_Lstack_addr   = _HEAP_END;
        
        # rom-to-ram utility
		F_Ldata_size     = __data_size;
		F_Ldata_RAM_addr = __xRAM_data_start;
		F_Ldata_ROM_addr = __pROM_data_start;
		
        F_xROM_to_xRAM   = 0x0000;
        F_pROM_to_xRAM   = 0x0001; 	# true
               
        

	# runtime code __init_sections uses these globals:

        F_start_bss   = _START_BSS;
        F_end_bss     = _END_BSS;
		 
 	
	} > .x_internal_RAM	 	                    	
}



# -------------------------------------------------------
# additional notes:


# about the reserved sections:

# p_interrupts_RAM -- reserved in internal pRAM
# memory space reserved for interrupt vectors
# interrupt vectors must start at address zero




# about the memory map:

# SDM xRAM limit is 0x7FFF





# about LCF conventions:

# program memory (p memory)
# (RWX) read/write/execute for pRAM
# (RX) read/execute for flashed pROM

# data memory (X memory)
# (RW) read/write for xRAM
# (R)  read for data flashed xROM or reserved x memory

# LENGTH = next start address - previous
# LENGTH = 0x0000 means use all remaing memory




# about ROM-to-RAM copying at init time:

# In embedded programming, it is common for a portion of 
# a program resident in ROM to be copied into RAM at runtime.
# For starters, program variables cannot be accessed until 
# they are copied to RAM. 

# To indicate data or code that is meant to be copied 
# from ROM to RAM,the data or code is given two addresses.

# One address is the resident location in ROM (defined by 
# the linker command file). The other is the intended
# location in RAM (defined in C code where we will 
# do the actual copying).

# To create a section with the resident location in ROM 
# and an intended location in RAM, you define the two addresses 
# in the linker command file.

# Use the MEMORY segment to specify the intended RAM location,
# and the AT(address)parameter to specify the resident ROM address.



# revision history 
# 011226 R1.0  c.m. first version
# 020220 R1.1  a.h. updates
# 020308 R1.1  a.h. 56838E
# 021101 R2.0  a.h. R2.0 prep
# 021204 R2.01 a.h. bss align by 1
# 030222 R2.1  a.h. 568345/346
# 030630 R6.0  a.h. ldm pROM-xRAM
# 030814 R6.0  a.h. align(1) for data

# -- end -- 
