ENTRY(_start)
SECTIONS
{
. = 0x0000;
.text : { KEEP(*(.text.boot)) *(.text .text.* .gnu.linkonce.t*) }
.data : {*(.data)}
.bss : {*(.bss)}
sp_top = .;
}