 .thumb
 
.section ".text.boot"

.extern _kstart
.global _start

_start:
    .word stack_top
    .word init
    .word halt
    .word halt
    .word halt
    .word halt

init:
    mrs r0, cpsr
    ldr r0,= stack_top
    mov sp, r0
    mov fp, r0

bl kstart

halt:
    wfi
    b halt

.data
stack: .space 1024
stack_top: