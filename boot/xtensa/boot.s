.section ".text.boot"
.global _start

_start:

j init_boot

halt:
    j halt

.data
stack: .space 256
_stack_top: