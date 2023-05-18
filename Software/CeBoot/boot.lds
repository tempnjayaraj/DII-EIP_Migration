ENTRY(ResetVector)
SECTIONS
{
        . = 0xC0000000;
        .boot : { *(.boot) }
        .text : { *(.text) }
        .data : { *(.data) }
        .rodata : { *(.rodata*)
        . = ALIGN(4);}

        _text = ADDR(.text);
        _etext = ADDR(.rodata) + SIZEOF(.rodata);

        . = ALIGN(4);
        .bss : { *(.bss) }
        _bss = ADDR(.bss);
        _ebss = ADDR(.bss) + SIZEOF(.bss);

        . = ALIGN(4);
}
