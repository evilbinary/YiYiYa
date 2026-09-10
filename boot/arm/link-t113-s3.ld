ENTRY(_start)

__heap_size  = 0x200;
__stack_size = 0x200;

SECTIONS
{
    . = 0x8000;
    .text : { KEEP(*(.text.boot)) *(.text .text.* .gnu.linkonce.t*) }
    .data : {*(.data)}
    .bss : {*(.bss)}
    .stack :  ALIGN(4) {
        /* APU（其它核）每核 4 种模式的栈区：必须整块位于内核 .text 起始
         * 地址 0x8000 之下。原布局把它排在主 CPU 栈区之后
         * （0x6A00 + __stack_size*4*4 = 0x8A00），会覆盖 0x8000 开始的内核
         * 代码段 —— APU 任何一次异常压栈都会改写内核指令。
         * 这里放到主 CPU 栈区下方：0x4000..0x6000。
         * （_stack_other 必须是区域基址：apu_entry 里按 base + cpu*4*size 取用） */
        . = 0x4000;
        _stack_other = .;
        . = . + __stack_size*4*4;

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

        . = . + __stack_size;
        _stack_sys = .;

        . = ALIGN(4);
        _sstack = .;
    }

}