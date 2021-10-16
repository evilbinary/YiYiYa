MEMORY
{
  flash (rx): ORIGIN = 0x08000000, LENGTH = 4K
  ram (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
  ccm (rwx) : ORIGIN = 0x10000000, LENGTH = 64K
}


ENTRY(_start)
SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >flash
  
.text : ALIGN(4) {
    *(.literal .literal.*)
    *(.text .text.*)
  } > flash

  .bss : ALIGN(4) {
     _bss_start = ABSOLUTE(.);
    *(.bss .bss.*)
	_bss_end = ABSOLUTE(.);
  } > ram

  .data : ALIGN(4) {
    *(.data .data*)
    *(.rodata .rodata.*)
  } > ram

sp_top = .;
}