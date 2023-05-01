.section ".text.boot"
.global _start

_start:

#csrr t0, mhartid
#bnez t0, halt

la sp,_stack_svc
la t0, _stack_irq
csrw  sscratch, t0
mv  gp, zero

li   t0, 0x40100
csrs sstatus,t0

csrw satp, zero

#csrr t0,mstatus
#csrw mstatus,t0
#la  t0,	init_boot
#csrw mepc, t0
#mret

j init_boot

halt:
    wfi
    j halt

.data
stack: .space 1024
stack_top: