ENTRY(_start)

__heap_size  = 0x10000;    /* 64KB heap */
__stack_size = 0x2000;     /* 8KB per stack */

SECTIONS
{
    . = 0x100000;   /* QEMU raspi3b kernel load address (1MB) */

    .text : {
        KEEP(*(.text.boot))
        *(.text .text.* .gnu.linkonce.t*)
    }

    .rodata : {
        *(.rodata .rodata.* .gnu.linkonce.r*)
    }

    .data : {
        *(.data .data.* .gnu.linkonce.d*)
    }

    .bss : {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }

    /* Stack at end of code/data, grows downward */
    . = ALIGN(16);
    _estack = . + __stack_size;        /* Core 0 stack (8KB) */
    _stack_other = _estack;            /* AP stacks follow */
    _sstack = _stack_other + __stack_size * 4;  /* End of all stacks */

    /DISCARD/ : { *(.comment) *(.note*) }
}
