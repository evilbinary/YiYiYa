 .thumb
 
.section ".text.boot"

.extern _kstart
.global _start

_start:
    movs  r1, #0
    bl kstart

halt:
    wfi
    b halt

.data
stack: .space 1024
stack_top:

.section  .isr_vector,"a",%progbits
  .word  stack_top
  .word  _start
  .word  _start
  .word  _start
  .word  _start
  .word  _start
  .word  _start
  .word  0
  .word  0
  .word  0
  .word  0
  .word  _start
  .word  _start
  .word  0
  .word  _start
  .word  _start