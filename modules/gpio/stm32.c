/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "gpio.h"
#include "platform/stm32f4xx/gpio.h"

void gpio_config(u32 gpio, u32 pin, u32 val) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = pin;
  GPIO_InitStruct.Mode = val;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(gpio, &GPIO_InitStruct);
}

void gpio_pull(u32 gpio, u32 pin, u32 val) {
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin  = pin;   
  GPIO_InitStruct.Mode = val;
  HAL_GPIO_Init(gpio, &GPIO_InitStruct);
}

void gpio_output(u32 gpio, u32 pin, u32 val) {
   HAL_GPIO_WritePin(gpio,pin,val);
}

u32 gpio_input(u32 gpio, u32 pin) {
  return HAL_GPIO_ReadPin(gpio,pin);
}

void gpio_init_device(device_t *dev) {}