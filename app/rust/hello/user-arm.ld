ENTRY(_start)
K_VIR_ADDR = 0x4000000;
K_PHY_ADDR = 0x4000000;
PAGE_SIZE    = 0x1000;
SECTIONS
{
        /* Code. */
        .text K_VIR_ADDR : AT(K_PHY_ADDR)
        {
		. = ALIGN(PAGE_SIZE);
                *(.text*)
        }

        /* Read-only data. */
        .rodata  : ALIGN(4K)
        {
                *(.rodata*)
        }

        /* Read-write data (initialized) */
        .data  : ALIGN(4K)
        {
                *(.data)
        }

        /* Read-write data (uninitialized) and stack */
        .bss  : ALIGN(4K)
        {
                *(COMMON)
                *(.bss)
        }
        .eh_frame  : ALIGN(4K)
        {
                *(.eh_frame)
        }
        .debug  : ALIGN(4K)
        {
                *(.debug*)
        }
       _end = .;
}
