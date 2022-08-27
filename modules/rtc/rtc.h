/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef RTC_H
#define RTC_H

#include "kernel/kernel.h"

#define RTC_ADDR 0x70
#define RTC_DATA 0x71

typedef struct rtc_time{
    u8 second;
    u8 minute;
    u8 hour;
    u8 day;
    u8 month;
    u8 year;
}rtc_time_t;

#endif