ENTRY(_kstart)
K_VIR_ADDR = 0x42400000;
K_PHY_ADDR = 0x42400000;
PAGE_SIZE    = 0x100;
SECTIONS
{
        /* Code. */
        .text K_VIR_ADDR : AT(K_PHY_ADDR)
        {
		. = ALIGN(PAGE_SIZE);
                *(.text)
        }

        /* Read-only data. */
        .rodata BLOCK(4K) : ALIGN(4K)
        {
                *(.rodata)
        }

        /* Read-write data (initialized) */
        .data BLOCK(4K) : ALIGN(4K)
        {
                *(COMMON)
                *(.data)
        }

        /* Read-write data (uninitialized) and stack */
        __bss_start = .;
        .bss BLOCK(4K) : ALIGN(4K)
        {
                *(.bss)
        }
        __bss_end = .;
       _end = .;
}
