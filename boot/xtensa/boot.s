.section ".text.boot"
.global _start
.align 16
_start:




movi a0, 0
wsr.lcount a0
rsync

movi sp, _estack

j init_boot

halt:
    j halt