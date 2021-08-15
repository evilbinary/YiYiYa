MEMORY
{
  /* IRAM POOL1, used for APP CPU cache. Bootloader runs from here during the final stage of loading the app because APP CPU is still held in reset, the main app enables APP CPU cache */
  iram_loader (RWX) :           org = 0x40078000, len = 0x8000  /* 32KB, APP CPU cache */
  /* 63kB, IRAM. We skip the first 1k to prevent the entry point being
     placed into the same range as exception vectors in the app.
     This leads to idf_monitor decoding ROM bootloader "entry 0x40080xxx"
     message as one of the exception vectors, which looks scary to users.
  */
  iram (RWX) :                  org = 0x40080400, len = 0xfc00
  /* 64k at the end of DRAM, after ROM bootloader stack */
  dram (RW) :                  	org = 0x3FFF0000, len = 0x10000
}


ENTRY(_start)
SECTIONS
{

.text : ALIGN(4) {
    *(.literal)
    *(.text .text.*)
  } > iram

  .bss : ALIGN(4) {
     _bss_start = ABSOLUTE(.);
    *(.bss)
	_bss_end = ABSOLUTE(.);
  } > dram

  .data : ALIGN(4) {
    *(.data)
    *(.rodata .rodata.*)
  } > dram

sp_top = .;
}