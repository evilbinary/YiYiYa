ENTRY(_start)

__heap_size  = 0x10000;
__stack_size = 0x2000;

SECTIONS
{
    . = 0x02080000;   /* RK3568 U-Boot kernel_addr_r */

    .text : {
        __start = .;
        KEEP(*(.text.boot))
        . = ALIGN(2048);
        KEEP(*(.text.vectors))
        *(.text .text.* .gnu.linkonce.t*)
    }

    .rodata : {
        *(.rodata .rodata.* .gnu.linkonce.r*)
    }

    .data : {
        _sdata = .;
        *(.data .data.* .gnu.linkonce.d*)
        _edata = .;
    }
    _sidata = _sdata;

    .bss : {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }

    . = ALIGN(16);
    _estack = . + __stack_size;
    _stack_other = _estack;
    _sstack = _stack_other + __stack_size * 4;
    . = _sstack;
    __end = .;

    /DISCARD/ : { *(.comment) *(.note*) }
}
