MEMORY
{
  iram (RWX) :  org = 0x40080000, len = 0xfc00 /* 32K */
  dram (RW) :   org = 0x3FFF0000, len = 0x10000 /** 64K*/
}


ENTRY(_start)
SECTIONS
{

.text : ALIGN(4) {
    *(.literal .literal.*)
    *(.text .text.*)
  } > iram

  .bss : ALIGN(4) {
     _bss_start = ABSOLUTE(.);
    *(.bss .bss.*)
	_bss_end = ABSOLUTE(.);
  } > dram

  .data : ALIGN(4) {
    *(.data .data*)
    *(.rodata .rodata.*)
  } > dram

sp_top = .;
}