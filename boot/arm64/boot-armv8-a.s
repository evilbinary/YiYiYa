.section ".text.boot"
.global _start
.global apu_entry

.equ stack_size, 0x4000

// x1 = logical core id. Clobbers x9.
// Pi 3 / A53: core id is Aff0 (MPIDR & 0xff), Aff1 is 0.
// Pi 5 / BCM2712 A76: Aff0 is 0 on every core, core id is Aff1
// (DT reg 0x000 / 0x100 / 0x200 / 0x300). Masking with #3 makes every
// Pi 5 core look like core 0, so they all share one stack and all run kmain.
.macro load_core_index
    mrs     x1, mpidr_el1
    and     x9, x1, #0xff
    cbnz    x9, 91f
    lsr     x1, x1, #8
    and     x1, x1, #0xff
    b       92f
91:
    and     x1, x1, #3
92:
.endm

// ARM64 (AArch64) boot code for Raspberry Pi 3
// Raspberry Pi 3 starts in EL2 or EL3, need to setup EL1

_start:
    // Check processor ID, only core 0 runs boot code
    load_core_index
    cbnz    x1, apu_entry        // if not core 0, jump to AP init

    // Core 0 init - Setup stack immediately at current EL
    ldr     x0, =_estack
    mov     sp, x0
    mov     x29, x0              // frame pointer

    // Get current exception level
    mrs     x0, CurrentEL
    lsr     x0, x0, #2           // CurrentEL[3:2] = EL

    // If at EL3, drop directly to EL1 (skip EL2 for QEMU raspi3 compatibility)
    cmp     x0, #3
    b.ne    el2_setup
    b       drop_to_el1_from_el3

el2_setup:
    // If at EL2, drop to EL1
    mrs     x0, CurrentEL
    lsr     x0, x0, #2
    cmp     x0, #2
    b.ne    el1_setup
    bl      drop_to_el1

el1_setup:
    // Setup stack for core 0
    ldr     x0, =_estack
    mov     sp, x0
    mov     x29, x0              // frame pointer

    // Enable FP/SIMD at EL1 (CPACR_EL1)
    mov     x0, #(3 << 20)       // FPEN = 0b11 (enable FP/SIMD at EL1)
    msr     cpacr_el1, x0
    isb

    // Zero BSS section
    ldr     x0, =__bss_start
    ldr     x1, =__bss_end
bss_loop:
    cmp     x0, x1
    b.ge    bss_done
    str     xzr, [x0], #8
    b       bss_loop
bss_done:

    // Invalidate caches
    bl      invalidate_caches

    // Jump to C init code
    bl      init_boot

halt_core0:
    wfi
    b       halt_core0

// Drop from EL3 to EL1 directly (for QEMU raspi3 compatibility)
drop_to_el1_from_el3:
    // Setup EL1 stack (per-core: core0 → _estack，AP → _stack_other+n*8KB)
    load_core_index
    ldr     x0, =_stack_other
    lsl     x2, x1, #13
    add     x0, x0, x2
    msr     sp_el1, x0

    // Initialize EL1 control registers
    msr     sctlr_el1, xzr        // Disable MMU, caches in EL1

    // Enable FP/SIMD at EL2/EL1 from EL3
    mov     x0, #0               // No traps for FP/SIMD
    msr     cptr_el3, x0

    // Setup SCR_EL3 for EL1 entry
    // RW=1 (bit 10): lower ELs are AArch64
    // ST=1 (bit 11): enable secure EL1
    mov     x0, #(1 << 10)        // RW bit, lower ELs are AArch64
    msr     scr_el3, x0
    isb                          // Ensure SCR_EL3 update is visible

    // Set SPSR_EL3 to enter EL1 in AArch64 mode with interrupts disabled
    // EL1h mode (0x5) | DAIF mask bits (0x3C0 = D|A|I|F at bits 9:6)
    mov     x0, #0x3C5           // EL1h mode with IRQ/FIQ/SError/Debug masked
    msr     spsr_el3, x0

    adr     x0, el1_from_el3_return
    msr     elr_el3, x0
    isb                          // Ensure ELR_EL3 update is visible
    eret
el1_from_el3_return:
    b       el1_setup

// Drop from EL3 to EL2
drop_to_el2:
    // Setup EL2 stack before dropping
    ldr     x0, =_estack
    msr     sp_el2, x0

    // Initialize EL2 control registers before dropping
    msr     sctlr_el2, xzr        // Disable MMU, caches in EL2
    msr     hcr_el2, xzr          // Disable EL2 traps

    // Setup SCR_EL3 for EL2 entry
    mov     x0, #(1 << 10)        // RW bit, EL2 is AArch64
    msr     scr_el3, x0
    isb                          // Ensure SCR_EL3 update is visible

    // Set SPSR_EL3 to enter EL2 in AArch64 mode with interrupts disabled
    // EL2h mode (0x9) | DAIF mask bits (0x3C0 = D|A|I|F at bits 9:6)
    mov     x0, #0x3C9           // EL2h mode with IRQ/FIQ/SError/Debug masked
    msr     spsr_el3, x0

    adr     x0, el2_return
    msr     elr_el3, x0
    isb                          // Ensure ELR_EL3 update is visible
    eret
el2_return:
    ret

// Drop from EL2 to EL1
drop_to_el1:
    // Disable EL2 traps
    msr     hcr_el2, xzr

    // EL1 用 AArch64。只置 RW。不要置 SWIO：它会把 dc isw 升级成
    // dc cisw，开缓存前的作废会把固件旧行写回内存，盖掉关 MMU 时写的页表。
    mov     x0, #(1 << 31)
    msr     hcr_el2, x0
    isb                          // Ensure HCR_EL2 update is visible

    // 固件可能把 SCTLR_EL1.M 留着。降到 EL1 后这条才生效，不关的话
    // 主核会带着固件页表一直跑到 mm_page_enable。RES1 位照 Circle：
    // M=C=I=0，后面 mm_page_enable 设好 MAIR/TCR/TTBR 再开。
    mov     x0, #0x0800
    movk    x0, #0x30d0, lsl #16
    msr     sctlr_el1, x0
    isb

    // 主核：EL2 的 M 和 C 要一起关。只关 C、留着 M，访问会按非缓存
    // 打到已经缓存的行上，这行以后不再侦听。先按当前翻译把内核映像
    // 写到 PoC，再两个位一起清。从核不走这里的清缓存。
    load_core_index
    cbnz    x1, 1f
    mrs     x0, sctlr_el2
    tbnz    x0, #0, 2f           // M
    tbz     x0, #2, 1f           // C 也是 0 就跳过。#5 不是合法逻辑立即数
2:
    ldr     x2, =__start
    ldr     x3, =__end
3:
    dc      civac, x2
    add     x2, x2, #64
    cmp     x2, x3
    b.lo    3b
    dsb     sy
    mrs     x0, sctlr_el2
    bic     x0, x0, #1           // M
    bic     x0, x0, #(1 << 2)    // C
    msr     sctlr_el2, x0
    isb
1:
    // Setup EL1 stack (per-core)
    ldr     x0, =_stack_other
    lsl     x2, x1, #13
    add     x0, x0, x2
    msr     sp_el1, x0

    // Set SPSR_EL2 to enter EL1 in AArch64 mode with interrupts disabled
    // EL1h mode (0x5) | DAIF mask bits (0x3C0 = D|A|I|F at bits 9:6)
    mov     x0, #0x3C5           // EL1h mode with IRQ/FIQ/SError/Debug masked
    msr     spsr_el2, x0

    adr     x0, el1_return
    msr     elr_el2, x0
    isb                          // Ensure ELR_EL2 update is visible
    eret
el1_return:
    ret

// Instruction cache only. A76 CCSIDR is the CCIDX format; a set/way
// invalidate here used the old field layout and did not hit the data cache.
invalidate_caches:
    ic      iallu
    dsb     sy
    isb
    ret

// AP (Other cores) entry point
apu_entry:
    // Calculate core ID for stack offset
    load_core_index

    // Setup stack for AP immediately at current EL
    ldr     x2, =_stack_other
    lsl     x3, x1, #13          // core_id * 8192 (8KB stack per core，与链接脚本 __stack_size 一致)
    add     x2, x2, x3
    mov     sp, x2
    mov     x29, x2

    // Get current EL
    mrs     x0, CurrentEL
    lsr     x0, x0, #2

    // If at EL3, drop directly to EL1
    cmp     x0, #3
    b.ne    ap_el2_check
    b       drop_to_el1_from_el3_ap

ap_el2_check:
    // If at EL2, drop to EL1
    mrs     x0, CurrentEL
    lsr     x0, x0, #2
    cmp     x0, #2
    b.ne    ap_el1_ready
    bl      drop_to_el1

ap_el1_ready:
    // Enable FP/SIMD at EL1
    mov     x0, #(3 << 20)       // FPEN = 0b11
    msr     cpacr_el1, x0
    isb
    // 【实验】从核在进任何 C 代码之前先开 MMU+cache。此前从核以 M=C=0
    // 跑完 apu_entry→init_apu_boot→…→page_init 整段 C，所有数据访问都是
    // 非缓存，会把主核已缓存的行从侦听域里拆掉，事后 SH=3/civac 都救不回
    // 来。这里只非缓存读 ap_ttbr 这一条 64B 行（cpu0 放行前写好
    // ttbr0/mair/tcr 并清缓存，之后不再重写），随即在汇编里开 MMU，之后
    // 进 C 的访问全部走缓存，与主核同一侦听域。ap_ttbr==0（raspi3 不发
    // 布，见 init.c）则跳过整块，保持原路径。
    ldr     x9, =ap_ttbr
    ldr     x0, [x9]             // ttbr0；一行三值一起进来，只碰这一条缓存行
    cbz     x0, 5f
    msr     ttbr0_el1, x0
    ldr     x0, [x9, #8]         // mair
    msr     mair_el1, x0
    ldr     x0, [x9, #16]        // tcr
    msr     tcr_el1, x0
    isb
    tlbi    vmalle1is
    dsb     sy
    ldr     x0, =0x30d01805      // 与 mm_page_enable 相同：M|C|I|SPAN|RES1
    msr     sctlr_el1, x0
    dsb     sy
    isb
5:
    // 【恢复】AP 必须进入 C 层：init_apu_boot 内部 WFE 等待 boot_info/
    // kernel_entry 就绪（cpu0 在 mm_init 前设置），随后 start_apu_kernel
    // → arch_init(cpu≠0) → ap_init → lcpu_wait_start 轮询 ap_release。
    // 之前 raspi5 移植时把这里改成了 WFI 停车循环，raspi3 的 AP 因此
    // 永远无法初始化（"ap N kernel init" 消失、系统崩溃）。
    bl      init_apu_boot
halt_ap_loop:
    wfi
    b       halt_ap_loop

halt_ap:
    wfi
    b       halt_ap

// Drop from EL3 to EL1 directly for AP cores
drop_to_el1_from_el3_ap:
    // Setup EL1 stack for AP（每核独立：_stack_other + core*8KB，
    // 之前误用 _estack 会让所有 AP 共用 core0 的栈）
    load_core_index
    ldr     x0, =_stack_other
    lsl     x2, x1, #13
    add     x0, x0, x2
    msr     sp_el1, x0

    // Initialize EL1 control registers
    msr     sctlr_el1, xzr        // Disable MMU, caches in EL1

    // Enable FP/SIMD at EL2/EL1 from EL3
    mov     x0, #0               // No traps for FP/SIMD
    msr     cptr_el3, x0

    // Setup SCR_EL3 for EL1 entry
    mov     x0, #(1 << 10)        // RW bit, lower ELs are AArch64
    msr     scr_el3, x0
    isb

    // Set SPSR_EL3 to enter EL1 in AArch64 mode with interrupts disabled
    // EL1h mode (0x5) | DAIF mask bits (0x3C0 = D|A|I|F at bits 9:6)
    mov     x0, #0x3C5           // EL1h mode with IRQ/FIQ/SError/Debug masked
    msr     spsr_el3, x0

    adr     x0, ap_el1_return
    msr     elr_el3, x0
    isb
    eret
ap_el1_return:
    b       ap_el1_ready

.data
    // 【实验】主核在 lcpu_send_start 放行前写好 {ttbr0, mair, tcr} 并清缓
    // 存；从核进门只非缓存读这一条 64B 行，随后在汇编里开 MMU。之后不再
    // 写。raspi3 不写它（保持 0），从核在 ap_el1_ready 跳过该块，行为不变。
    .align 6
    .global ap_ttbr
ap_ttbr:
    .quad 0, 0, 0
