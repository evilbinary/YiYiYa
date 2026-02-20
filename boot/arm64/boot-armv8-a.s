.section ".text.boot"
.global _start

.equ stack_size, 0x4000

// ARM64 (AArch64) boot code for Raspberry Pi 3
// Raspberry Pi 3 starts in EL2 or EL3, need to setup EL1

_start:
    // Check processor ID, only core 0 runs boot code
    mrs     x1, mpidr_el1
    and     x1, x1, #3
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
    // Setup EL1 stack
    ldr     x0, =_estack
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

    // Set EL1 to AArch64
    mov     x0, #(1 << 31)       // RW bit
    msr     hcr_el2, x0
    isb                          // Ensure HCR_EL2 update is visible

    // Setup EL1 stack
    ldr     x0, =_estack
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

// Invalidate caches
invalidate_caches:
    // Invalidate instruction cache
    ic      iallu
    dsb     sy
    isb
    
    // Invalidate data cache (by set/way)
    mov     x0, #0
    msr     csselr_el1, x0       // Select L1 data cache
    isb
    
    mrs     x0, ccsidr_el1       // Read cache size
    mov     x1, x0
    and     x2, x1, #7           // Line size
    add     x2, x2, #4           // Log2(line_size) + 4
    lsl     x2, x2, #1           // Words per line
    
    ubfx    x3, x1, #3, #10      // Number of sets - 1
    add     x3, x3, #1           // Number of sets
    
    ubfx    x1, x1, #13, #15     // Number of ways - 1
    add     x1, x1, #1           // Number of ways
    
    clz     x4, x1               // Way position
    
    mov     x5, #0               // Set counter
inv_dcache_loop_set:
    mov     x6, #0               // Way counter
inv_dcache_loop_way:
    lsl     x7, x6, x4
    orr     x7, x7, x5
    lsl     x7, x7, x2
    dc      isw, x7
    add     x6, x6, #1
    cmp     x6, x1
    b.lt    inv_dcache_loop_way
    add     x5, x5, #1
    cmp     x5, x3
    b.lt    inv_dcache_loop_set
    
    dsb     sy
    isb
    ret

// AP (Other cores) entry point
apu_entry:
    // Calculate core ID for stack offset
    mrs     x1, mpidr_el1
    and     x1, x1, #3

    // Setup stack for AP immediately at current EL
    ldr     x2, =_stack_other
    lsl     x3, x1, #12          // core_id * 4096 (4KB stack per core)
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
    // AP cores wait in WFI state until properly initialized
    // Do not call init_apu_boot as VBAR_EL1 is not set
halt_ap_loop:
    wfi
    b       halt_ap_loop

halt_ap:
    wfi
    b       halt_ap

// Drop from EL3 to EL1 directly for AP cores
drop_to_el1_from_el3_ap:
    // Setup EL1 stack for AP
    ldr     x0, =_estack
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
