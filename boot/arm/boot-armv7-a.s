.section ".text.boot"
.global _start

.equ stack_size,0x200

_start:
    // save CPSR.
    mrs r0, cpsr

    mrc p15, #0, r4, c0, c0, #5
    and r4, r4, #3
    cmp r4, #0
    bne apu_entry //is other core

    //main core init 
    ldr sp, = _estack
    ldr fp, = _estack

    // set sp in abt mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x17
    msr cpsr_c,r1
    ldr sp, =_stack_abt

    // set sp in undf mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x1B
    msr cpsr_c,r1
    ldr sp, =_stack_undf

    // set sp in irq mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x12
    msr cpsr_c,r1
    ldr sp,= _stack_irq


   // set sp in sys mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x1F
    msr cpsr_c, r0
    ldr sp,= _stack_sys

    // set sp in svc mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x13
    msr cpsr_c, r0
    ldr sp,= _stack_svc

    //disable mmu
	mcr     p15, #0, r0, c8, c7, #0    @ invalidate tlb
	mrc     p15, #0, r0, c1, c0, #0
	bic     r0, r0, #1
	mcr     p15, #0, r0, c1, c0, #0    @ clear mmu bit
	dsb

    bl init_boot

apu_entry:
    //wfe
    ldr sp, = _estack
    ldr fp, = _estack

    ldr r2,=_stack_other
    mov r3,#256*4
    asr r3,r3,r4
    add r2,r2,r3

    // set sp in abt mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x17
    msr cpsr_c,r1
    mov sp, r2

    add r2,r2,$stack_size

    // set sp in undf mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x1B
    msr cpsr_c,r1
    mov sp, r2

    add r2,r2,$stack_size
    
    // set sp in irq mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x12
    msr cpsr_c,r1
    mov sp,r2

    add r2,r2,$stack_size
    
    // set sp in svc mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x13
    msr cpsr_c, r0
    mov sp, r2

    bl init_apu_boot
halt:
    //wfi
    b halt

.data