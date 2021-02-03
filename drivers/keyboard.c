/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "kernel/kernel.h"

extern void acquire(u32* lock);
extern void release(u32* lock);

#define MAX_CHARCODE_BUFFER 32
static u8 scan_code_buffer[MAX_CHARCODE_BUFFER];
static u32 scan_code_index=0;

static size_t read(device_t* dev, void* buf, size_t len) {
  u32 ret=0;
  if (scan_code_index>0) {
    kstrncpy(buf, &scan_code_buffer[scan_code_index-1], 1);
    for(int i=0;i<scan_code_index;i++){
      scan_code_buffer[i]=scan_code_buffer[i+1];
    }
    scan_code_index--;
    ret=1;
  }
  return ret;
}

INTERRUPT_SERVICE
void keyboard_handler() {
  interrupt_entering_code(ISR_KEYBOARD, 0);
  interrupt_process(do_keyboard);
  interrupt_exit();
}

int keyboard_init(void) {
  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = "keyboard";
  dev->read = read;
  dev->id = DEVICE_KEYBOARD;
  dev->type=DEVICE_TYPE_CHAR;
  dev->data=scan_code_buffer;
  device_add(dev);
  scan_code_index=0;

  interrutp_regist(ISR_KEYBOARD, keyboard_handler);
  io_write8(io_read8(0x21) & 0xfd, 0x21);
  return 0;
}

void keyboard_exit(void) { kprintf("Goodbye World\n"); }

void do_keyboard(interrupt_context_t* context) {
  int com=0;
  int scan_code = io_read8(0x60);
  if(scan_code_index>MAX_CHARCODE_BUFFER){
    scan_code_index=0;
    kprintf("key buffer is full\n");
  }
  scan_code_buffer[scan_code_index++]=scan_code;
  // io_write8((com = io_read8(0x61)) | 0x80, 0x61);
  // io_write8(com & 0x7f, 0x61);
  io_write8(0x20, 0x20);
  io_write8(0xa0, 0x20);
}

module_t keyboard_module = {
    .name = "keyboard", .init = keyboard_init, .exit = keyboard_exit};
module_regit(keyboard_module);