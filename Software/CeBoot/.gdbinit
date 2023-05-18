target remote localhost:2331
monitor speed auto
load boot.elf
b main
display /i $pc
display /x $r3
display /x $r2
display /x $r1
display /x $r0
