.section ".text.boot"
.global _start
.align 16
_start:


// Initialize window registers
// windowbase is a window index (0-15), not an address
movi a1, 0
movi a2, 0  // Set windowbase to 0 (valid index)
wsr.windowstart a1
wsr.windowbase a2
rsync

movi a0,0

movi a2, 0
wsr.lcount a2
rsync

// Set PS (Processor Status) register
// Clear UM bit (bit 4) to ensure kernel mode
// Set WOE bit (bit 0) for windowed exception support (needed even with call0)
movi a0, 0x00000011  // PS with WOE=1, UM=0 (kernel mode)
wsr.ps a0
rsync

movi a1, _estack
movi sp, _estack

// Set vector base to 0x40081000 where window overflow/underflow vectors are located
// Note: window_overflow_8 is at vecbase+0x80 = 0x40081000+0x80 = 0x40081080
movi a0, 0x40081000
wsr.vecbase a0
rsync

j init_boot

halt:
    j halt