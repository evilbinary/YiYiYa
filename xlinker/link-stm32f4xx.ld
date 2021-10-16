MEMORY
{
  flash (rx): ORIGIN = 0x08000000, LENGTH = 1024K
  ram (rwx) : ORIGIN = 0x20000000, LENGTH = 32K
  ccm (rwx) : ORIGIN = 0x10000000, LENGTH = 64K
}

ENTRY(_start)

SECTIONS {
  .text : ALIGN(4) {
    *(.literal)
    *(.text .text.*)
  } > flash

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    *(*SCREEN_ASCII)
    . = ALIGN(4);
  } > flash


  .bss : ALIGN(4) {
    _bss_start = ABSOLUTE(.);
    *(.bss)
	  _bss_end = ABSOLUTE(.);
  } > ram


  .data : ALIGN(4) {
    *(.data)
    *(.data*)
  } > ram
}

