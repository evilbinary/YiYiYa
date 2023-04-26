.section ".text.boot"
.global _start

_start:

la sp,_stack_svc
la t0, _stack_irq
csrw  sscratch, t0
mv  gp, zero
li   t0, 0x40000
csrs sstatus,t0

j init_boot

halt:
    wfi
    j halt

.data
stack: .space 1024
stack_top: