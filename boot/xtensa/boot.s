.section ".text.boot"
.global _start
.align 16
_start:


movi a1, 0
movi a2,_stack_other
wsr.windowstart a1
wsr.windowbase a2
rsync

movi a0,0

movi a2, 0
wsr.lcount a2
rsync

movi a1, _estack
movi sp, _estack

j init_boot

halt:
    j halt