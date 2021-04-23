.section ".text.boot"
.global _start

_start:
ldr sp, = stack_top
    
 // save CPSR.
mrs r0, cpsr

// set sp in irq mode.
bic r1, r0, #0x1F
orr r1, r1, #0x12
msr cpsr_c,r1
mov sp, #0x4000

// set sp in svc mode.
msr cpsr_c, r0
mov sp, #0x8000

bl init_boot

halt:
    wfi
    b halt

.data
stack: .space 4 * 4096
stack_top:
