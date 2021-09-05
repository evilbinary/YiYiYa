#ifndef __GPIO_H__
#define __GPIO_H__

#include "kernel/kernel.h"

typedef struct gpio{
  char* name;
  int base;
  int ngpio;
  void* data;
}gpio_t;

void gpio_config(u32 gpio, u32 pin, u32 val);
void gpio_pull(u32 gpio, u32 pin, u32 val);
void gpio_output(u32 gpio, u32 pin, u32 val);
u32 gpio_input(u32 gpio, u32 pin);

#endif