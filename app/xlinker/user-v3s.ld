ENTRY(_start)
K_VIR_ADDR = 0x71000000;
K_PHY_ADDR = 0x71000000;
PAGE_SIZE    = 0x1000;
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
                *(.data)
        }

        /* Read-write data (uninitialized) and stack */
        .bss BLOCK(4K) : ALIGN(4K)
        {
                *(COMMON)
                *(.bss)
        }
        .eh_frame BLOCK(4K) : ALIGN(4K)
        {
                *(.eh_frame)
        }
       _end = .;
}
