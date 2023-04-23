.section ".text.boot"
.global _start

_start:

la sp,stack_top

j init_boot

halt:
    wfi
    j halt

.data
stack: .space 1024
stack_top: