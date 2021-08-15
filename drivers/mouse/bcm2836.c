/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "mouse.h"

#include "kernel/kernel.h"

mouse_device_t mouse_device;
mouse_event_t event;
u32 has_data=0;

static size_t read(device_t* dev, void* buf, size_t len) {

  return 0;
  // u32 ret = len;
  // if(has_data<0){
  //   return 0;
  // }
  // has_data--;
  // // mouse_event_t* e = cqueue_peek(mouse_device.events);
  // // if (e == NULL) {
  // //   return 0;
  // // }
  // mouse_event_t* data = buf;
  // *data = event;
  // // cqueue_put(mouse_device.events,e);
  // // kpool_put(e);
  // return ret;
}

INTERRUPT_SERVICE
void mouse_handler() {
  interrupt_entering_code(ISR_MOUSE, 0);
  interrupt_process(do_mouse);
  interrupt_exit();
}

u8 mouse_read() {
   
  return 0;
}

void mouse_write(u8 data) {
  
}

void mouse_wait(u8 type) {
  
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


  return 0;
}

void do_mouse(interrupt_context_t* context) {
  u32 read_count = 0;
  u8 state = 0; 
}

void mouse_exit(void) { kprintf("mouse exit\n"); }

module_t mouse_module = {
    .name = "mouse", .init = mouse_init, .exit = mouse_exit};
