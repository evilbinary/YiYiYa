/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "mouse.h"

#include "kernel/kernel.h"
#include "pic/pic.h"

mouse_device_t mouse_device;
mouse_event_t event;
u32 has_data=0;

static size_t read(device_t* dev, void* buf, size_t len) {
  u32 ret = len;
  if(has_data<0){
    return 0;
  }
  has_data--;
  // mouse_event_t* e = cqueue_peek(mouse_device.events);
  // if (e == NULL) {
  //   return 0;
  // }
  mouse_event_t* data = buf;
  *data = event;
  // cqueue_put(mouse_device.events,e);
  // kpool_put(e);
  return ret;
}

INTERRUPT_SERVICE
void mouse_handler() {
  interrupt_entering_code(ISR_MOUSE, 0);
  interrupt_process(do_mouse);
  interrupt_exit();
}

void mouse_wait(u8 type) {
  u32 time_out = 1000000;
  for (; time_out > 0; time_out--) {
    if ((io_read8(MOUSE_STATUS) & (1 + type)) == (1 - type)) {
      break;
    }
  }
}

u8 mouse_read() {
  mouse_wait(0);
  return io_read8(MOUSE_DATA);
}

void mouse_write(u8 data) {
  mouse_wait(1);
  io_write8(MOUSE_COMMAND, 0xD4);
  mouse_wait(1);
  io_write8(MOUSE_DATA, data);
}


int mouse_init(void) {
  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = "mouse";
  dev->read = read;
  dev->id = DEVICE_MOUSE;
  dev->type = DEVICE_TYPE_CHAR;
  dev->data = &mouse_device;

  device_add(dev);
  interrutp_regist(ISR_MOUSE, mouse_handler);

  mouse_device.events = cqueue_create(EVENT_NUMBER, CQUEUE_DROP);

  // for(int i=0;i<EVENT_NUMBER;i++){
  //   void* e=kpool_poll();
  //   cqueue_put(mouse_device.events,e);
  // }

  mouse_wait(1);
  // Enable second PS/2 port (only if 2 PS/2 ports supported)
  io_write8(MOUSE_COMMAND, 0xa8);
  mouse_wait(1);

  // Read "byte 0" from internal RAM
  io_write8(MOUSE_COMMAND, 0x20);
  mouse_wait(0);

  u8 status = (io_read8(MOUSE_DATA) | 2);
  mouse_wait(1);

  // Write next byte to "byte 0" of internal RAM (Controller Configuration Byte
  io_write8(MOUSE_STATUS, 0x60);
  mouse_wait(1);
  io_write8(MOUSE_DATA, status);

  // mouse_wait(1);

  // Set Defaults
  mouse_write(0xf6);
  mouse_read();

  // Enable Data Reporting
  mouse_write(0xf4);
  mouse_read();

  pic_enable(ISR_MOUSE);

  return 0;
}

void do_mouse(interrupt_context_t* context) {
  u32 read_count = 0;
  u8 state = 0;
  for (; io_read8(MOUSE_STATUS) & 1;) {
    u8 data = io_read8(MOUSE_DATA);
    if (read_count == 0) {
      event.sate = data;
      state = data;
    } else if (read_count == 1) {
      event.x = event.x + data - ((state << 4) & 0x100);
    } else if (read_count == 2) {
      event.y = event.y + data - ((state << 3) & 0x100);

      if (event.x < 0) {
        event.x = 0;
      }
      if (event.y < 0) {
        event.y = 0;
      }
      // mouse_event_t* e = kpool_poll();
      // if(e==NULL){
      //  mouse_event_t* e=cqueue_poll(mouse_device.events);
      // }
      // *e = event;
      // cqueue_put(mouse_device.events, e);
    }
    read_count = (read_count + 1) % 3;
  }
  has_data++;

  pic_eof(ISR_MOUSE);
}

void mouse_exit(void) { kprintf("mouse exit\n"); }

module_t mouse_module = {
    .name = "mouse", .init = mouse_init, .exit = mouse_exit};
