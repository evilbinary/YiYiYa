.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb

.section ".text.boot"

.extern _kstart
.global _start



.section  .text.boot_reset_handler
.weak  boot_reset_handler
.type  boot_reset_handler, %function
_start:
boot_reset_handler:  
    ldr   sp, =_estack
    bl init_boot
    
boot_halt:
    b boot_halt

boot_nmi_handler:
    b boot_halt

boot_hardfault_handler:
    b boot_halt

boot_svc_handler:
    b boot_halt

boot_pendsv_handler:
    b boot_halt

boot_systick_handler:
    b boot_halt

.section  .isr_vector,"a",%progbits
  .word  _estack
  .word  boot_reset_handler
  .word  boot_nmi_handler
  .word  boot_hardfault_handler
  .word  boot_reset_handler
  .word  boot_reset_handler
  .word  boot_reset_handler
  .word  0
  .word  0
  .word  0
  .word  0
  .word  boot_svc_handler
  .word  boot_halt
  .word  0
  .word  boot_pendsv_handler
  .word  boot_systick_handler
  
.section .data 
_boot_info: .word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0
.word 0