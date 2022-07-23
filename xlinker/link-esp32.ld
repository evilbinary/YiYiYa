MEMORY {
  /*iram : org = 0x40082000, len = 0xFC00*/
  /* dram : org = 0x3ffc0000, len = 0x20000 */
  dram   (rw)   : ORIGIN = 0x3FFC0000, LENGTH = 0x20000  /* 0x3FFAE000-0x40000000 */
  iram   (rwx)  : ORIGIN = 0x40084000, LENGTH = 0x20000  /* 0x40080000-0x400A0000  */

  drom   (r)    : ORIGIN = 0x3F400000, LENGTH = 0x20000 /* 0x3F400000-0x3F800000 */
  irom   (rx)   : ORIGIN = 0x400D0000, LENGTH = 0x20000 /* 0x400D0000-0x40400000  */
}

ENTRY(_kstart)

SECTIONS {
  .text : ALIGN(4) {
    *(.literal)
    *(.text .text.*)
  } > irom

  .bss : ALIGN(4) {
     _bss_start = ABSOLUTE(.);
    *(.bss)
	_bss_end = ABSOLUTE(.);
  } > dram

  .data : ALIGN(4) {
    *(.data)
  } > dram


  .rodata : ALIGN(4) {
    *(.rodata .rodata.*)
  } > drom
}

