/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "i2c/i2c.h"
#include "kernel/kernel.h"
#include "mouse.h"

#define NS2009_READ_X_LOW_POWER_12BIT 0xc0
#define NS2009_READ_Y_LOW_POWER_12BIT 0xd0
#define NS2009_READ_Z1_LOW_POWER_12BIT 0xe0
#define NS2009_READ_Z2_LOW_POWER_12BIT 0xf0

#define NS2009_DEF_X_FUZZ 32
#define NS2009_DEF_Y_FUZZ 16

#define NS2009_PEN_UP_Z1_ERR 80

mouse_device_t mouse_device;
mouse_event_t event;
u32 has_data = 0;

u32 ns2009_read(device_t* dev, u32 cmd, int* val) {
  i2c_msg_t msg[2];
  u8 buf[4] = {0};
  if (dev == NULL) {
    kprintf("dev is null\n");
    return 0;
  }
  msg[0].addr = 0x48;        // addr
  msg[0].flags = I2C_WRITE;  // type
  msg[0].len = 1;            // len
  msg[0].buf = &cmd;         // cmd

  msg[1].addr = 0x48;       // addr
  msg[1].flags = I2C_READ;  // type
  msg[1].len = 2;           // len
  msg[1].buf = buf;         // buf

  // read data
  u32 ret = dev->read(dev, msg, 2);
  if (ret < 0) {
    kprintf("error %d\n", ret);
    return -1;
  }
  *val = (buf[0] << 4) | (buf[1] >> 4);
  return 0;
}

int ns2009_get_touch() {
  int z = 0;
  int x = 0;
  int y = 0;
  device_t* i2c_dev = mouse_device.data;
  u32 ret = ns2009_read(i2c_dev, NS2009_READ_Z1_LOW_POWER_12BIT, &z);
  if (z >= NS2009_PEN_UP_Z1_ERR) {  // touch down
    event.sate = 1;
    has_data++;
    ret = ns2009_read(i2c_dev, NS2009_READ_X_LOW_POWER_12BIT, &x);
    if (ret) {
      return ret;
    }
    ret = ns2009_read(i2c_dev, NS2009_READ_Y_LOW_POWER_12BIT, &y);
    if (ret) {
      return ret;
    }
    // kprintf("%d,%d\n", x, y);
    event.x = x / 10;
    event.y = y / 10;
    return 1;
  } else {
    event.sate = 0;
    // kprintf("up %d\n", z);
  }
  return 0;
}

// ns2009 touch
static size_t read(device_t* dev, void* buf, size_t len) {
  if (dev == NULL) return 0;
  u32 ret = ns2009_get_touch(dev->data);
  if (ret > 0) {
    mouse_event_t* data = buf;
    *data = event;
  }
  return ret;
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

int mouse_init(void) {
  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = "mouse";
  dev->read = read;
  dev->id = DEVICE_MOUSE;
  dev->type = DEVICE_TYPE_CHAR;
  dev->data = &mouse_device;

  device_add(dev);
  mouse_device.events = cqueue_create(EVENT_NUMBER, CQUEUE_DROP);

  device_t* i2c_dev = device_find(DEVICE_I2C);
  mouse_device.data = i2c_dev;
  return 0;
}

void do_mouse(interrupt_context_t* context) {
  u32 read_count = 0;
  u8 state = 0;
}

void mouse_exit(void) { kprintf("mouse exit\n"); }

module_t mouse_module = {
    .name = "mouse", .init = mouse_init, .exit = mouse_exit};
