.section ".text.boot"
.global _start

_start:

j init_boot

halt:
    j halt

.section .iram0
.data
stack: .space 256
_stack_top: