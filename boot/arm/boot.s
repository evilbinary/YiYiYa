.section ".text.boot"
.global _start

_start:
 // save CPSR.
mrs r0, cpsr

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
//ldr sp,= stack_svc

bl init_boot

halt:
    wfi
    b halt

.data
stack: .space 1024
stack_top:
