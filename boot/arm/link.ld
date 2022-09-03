ENTRY(_start)

__heap_size  = 0x200;
__stack_size = 0x200;

SECTIONS
{
    . = 0x000;
    .text : { KEEP(*(.text.boot)) *(.text .text.* .gnu.linkonce.t*) }
    .data : {*(.data)}
    .bss : {*(.bss)}
    .stack :  ALIGN(4) {
        . = 0x6000;
        _estack = .;
        . = . + __stack_size;
        _stack_abt = .;

        . = . + __stack_size;
        _stack_undf = .;

        . = . + __stack_size;
        _stack_irq = .;

        . = . + __stack_size;
        _stack_svc = .;


        . = . + __stack_size*4*4 ;
        _stack_other = .;

        
        . = ALIGN(4);
        _sstack = .;
    }

}