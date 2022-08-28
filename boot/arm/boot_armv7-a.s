.section ".text.boot"
.global _start

_start:
 // save CPSR.
mrs r0, cpsr

mrc p15, #0, r4, c0, c0, #5
and r4, r4, #3
cmp r4, #0
bne apu_entry //is other core

//main core init 
ldr sp, = stack_top
ldr fp, = stack_top

// set sp in abt mode.
bic r1, r0, #0x1F
orr r1, r1, #0x17
msr cpsr_c,r1
mov sp, #0x1000

// set sp in undf mode.
bic r1, r0, #0x1F
orr r1, r1, #0x1B
msr cpsr_c,r1
mov sp, #0x1000

// set sp in irq mode.
bic r1, r0, #0x1F
orr r1, r1, #0x12
msr cpsr_c,r1
mov sp, #0x2000
//ldr sp,= stack_irq


// set sp in svc mode.
bic r1, r0, #0x1F
orr r1, r1, #0x13
msr cpsr_c, r0
mov sp, #0x3000
//ldr sp,= stack_svc_top

bl init_boot

apu_entry:
    //wfe
    ldr sp, = stack_top
    ldr fp, = stack_top

    // set sp in abt mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x17
    msr cpsr_c,r1
    mov sp, #0x1000

    // set sp in undf mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x1B
    msr cpsr_c,r1
    mov sp, #0x1000


    mov r2,#0x2000
    asr r2,r2,r4
    // set sp in irq mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x12
    msr cpsr_c,r1
    mov sp,r2
    //ldr sp,= stack_irq

    mov r2,#0x3000
    asr r2,r2,r4
    // set sp in svc mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x13
    msr cpsr_c, r0
    mov sp, r2
    //ldr sp,= stack_svc_top

    bl init_apu_boot
halt:
    wfi
    b halt

.data
stack: .space 1024
stack_top: