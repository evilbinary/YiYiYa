.section ".text.boot"
.global _start

/* APU（apu_entry）按 base + cpu*(4*__stack_size) 逐块取用四种模式的栈，
 * 因此尺寸一律取【正在链接的那个镜像】的链接脚本符号 __stack_size：
 *   - 内核镜像（QEMU / 真机运行时）用 app/xlinker/link-*.ld（STACK_SIZE；
 *     armv7 平台已统一为 0x1000，脚本里另有 __stack_size = STACK_SIZE 别名）
 *   - boot/arm/link-*.ld 是另一份引导镜像，其 __stack_size 为 0x200
 *     （该镜像的栈区在 0x1000..0x8000 的低窗口里，放不下更大的块）
 * 用符号而不是写死常量，可避免汇编与脚本各写一个数字而互相不一致
 * （历史 bug：asm 0x1000 / ld 0x200 ⇒ APU 栈指针落到保留区之外）。 */

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
    msr cpsr_c, r1
    ldr sp,= _stack_sys

    // set sp in svc mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x13
    msr cpsr_c, r1
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
    /* 每个模式的栈尺寸/步长一律取链接脚本的 __stack_size，不要写死常量：
     * 写死曾导致与 ld 不一致（asm 0x1000 / ld 0x200），cpu1/2/3 的四个模式
     * 栈指针落到保留区之外（最多偏出 0xC000）。现在由符号保证每个镜像自洽。
     * per-CPU block = 4 modes * __stack_size。 */
    ldr r5,=__stack_size
    mul r3, r4, r5
    lsl r3, r3, #2
    add r2, r2, r3

    /* 每种模式 sp = 本模式栈块的“顶”（栈向下生长），再把 r2 推进一块。
     * 与主 CPU 的 _stack_abt/_stack_irq/_stack_svc（同样指向块顶）保持一致。 */
    // set sp in abt mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x17
    msr cpsr_c,r1
    add r2,r2,r5
    mov sp, r2

    // set sp in undf mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x1B
    msr cpsr_c,r1
    add r2,r2,r5
    mov sp, r2

    // set sp in irq mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x12
    msr cpsr_c,r1
    add r2,r2,r5
    mov sp,r2

    // set sp in svc mode.
    bic r1, r0, #0x1F
    orr r1, r1, #0x13
    msr cpsr_c, r1
    add r2,r2,r5
    mov sp, r2

    bl init_apu_boot
halt:
    //wfi
    b halt

.data