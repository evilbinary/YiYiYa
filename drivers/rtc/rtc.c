/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "rtc.h"

#include "kernel/kernel.h"

rtc_time_t rtc_time;

u8 rtc_get_reg(u32 reg) {
  io_write8(RTC_ADDR, reg);
  return io_read8(RTC_DATA);
}

void rtc_set_reg(u32 reg, u8 val) {
  io_write8(RTC_ADDR, reg);
  io_write8(RTC_DATA, val);
}

u32 rtc_is_updating() {
  io_write8(RTC_ADDR, 0x0A);
  u32 status = io_read8(RTC_DATA);
  return (status & 0x80);
}

void rtc_convert_bcd(rtc_time_t* current, u32 b) {
  if (!(b & 0x04)) {
    current->second = (current->second & 0x0F) + ((current->second / 16) * 10);
    current->minute = (current->minute & 0x0F) + ((current->minute / 16) * 10);
    current->hour =
        ((current->hour & 0x0F) + (((current->hour & 0x70) / 16) * 10)) |
        (current->hour & 0x80);
    current->day = (current->day & 0x0F) + ((current->day / 16) * 10);
    current->month = (current->month & 0x0F) + ((current->month / 16) * 10);
    current->year = (current->year & 0x0F) + ((current->year / 16) * 10);
  }
}
void rtc_get_time() {
  for (; rtc_is_updating();)
    ;
  rtc_time.second = rtc_get_reg(0);
  rtc_time.minute = rtc_get_reg(0x02);
  rtc_time.hour = rtc_get_reg(0x04);
  rtc_time.day = rtc_get_reg(0x07);
  rtc_time.month = rtc_get_reg(0x08);
  rtc_time.year = rtc_get_reg(0x09);
  u32 b = rtc_get_reg(0x0b);

  rtc_convert_bcd(&rtc_time, b);
}

void rtc_write_time(rtc_time_t* current) {
  for (; rtc_is_updating();)
    ;
  rtc_set_reg(0, current->second);
  rtc_set_reg(0x00, current->second);
  rtc_set_reg(0x02, current->minute);
  rtc_set_reg(0x04, current->hour);
  rtc_set_reg(0x07, current->day);
  rtc_set_reg(0x08, current->month);
  rtc_set_reg(0x09, current->year);
}

static size_t read(device_t* dev, void* buf, size_t len) {
  u32 ret = 0;
  rtc_get_time();

  rtc_time_t* rtc = buf;
  *rtc = rtc_time;

  return ret;
}

static size_t write(device_t* dev, void* buf, size_t len) {
  u32 ret = len;
  rtc_time_t* rtc = buf;
  rtc_write_time(rtc);
  return ret;
}

int rtc_init(void) {
  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = "rtc";
  dev->read = read;
  dev->write = write;
  dev->id = DEVICE_RTC;
  dev->type = DEVICE_TYPE_CHAR;
  dev->data = &rtc_time;

  device_add(dev);
  return 0;
}

void rtc_exit(void) { kprintf("rtc exit\n"); }

module_t rtc_module = {.name = "rtc", .init = rtc_init, .exit = rtc_exit};
