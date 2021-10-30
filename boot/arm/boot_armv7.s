.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb

.section ".text.boot"

.extern _kstart
.global _start
_start:

/* start address for the initialization values of the .data section. 
defined in linker script */
.word  _sidata
/* start address for the .data section. defined in linker script */  
.word  _sdata
/* end address for the .data section. defined in linker script */
.word  _edata
/* start address for the .bss section. defined in linker script */
.word  _sbss
/* end address for the .bss section. defined in linker script */
.word  _ebss
/* stack used for SystemInit_ExtMemCtl; always internal RAM used */


.section  .text.boot_reset_handler
.weak  boot_reset_handler
.type  boot_reset_handler, %function

boot_reset_handler:  
    ldr   sp, =_estack      /* set stack pointer */


/* Zero fill the bss segment. */
  ldr r2, =_sbss
  ldr r4, =_ebss
  movs r3, #0
  b LoopFillZerobss

FillZerobss:
  str  r3, [r2]
  adds r2, r2, #4

LoopFillZerobss:
  cmp r2, r4
  bcc FillZerobss

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