.section ".text.boot"
.global _start

_start:
ldr sp, = stack_top
ldr fp, = stack_top
 // save CPSR.
mrs r0, cpsr

// set sp in irq mode.
bic r1, r0, #0x1F
orr r1, r1, #0x12
msr cpsr_c,r1
mov sp, #0x2000
//ldr sp,= stack_irq


// set sp in svc mode.
msr cpsr_c, r0
mov sp, #0x3000
//ldr sp,= stack_svc

bl init_boot

halt:
    wfi
    b halt

.data
stack: .space 4096*4
stack_top:
