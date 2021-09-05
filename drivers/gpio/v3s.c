#include "gpio.h"


// pull config
#define GPIO_PULL_DISABLE (0)
#define GPIO_PULL_UP (1)
#define GPIO_PULL_DOWN (2)
#define GPIO_PULL_RESERVED (3)

//cfg config
#define GPIO_INPUT        (0)
#define GPIO_OUTPUT       (1)
#define GPIO_DISABLE      (7)


#define GPIO_A 0
#define GPIO_B 1 //Port B(PB): 10 input/output port
#define GPIO_C 2 //4 input/output port
#define GPIO_D 3
#define GPIO_E 4 //25 input/output port
#define GPIO_F 5 //7 input/output port
#define GPIO_G 6 //6 input/output port
#define GPIO_H 7
#define GPIO_I 8
#define GPIO_L 9

typedef struct v3s_gpio {
  volatile unsigned long config[4];//config 0-3 n*0x24+0x00  n*0x24+0x04 n*0x24+0x08 n*0x24+0x0C
  volatile unsigned long data;  //data n*0x24+0x10
  volatile unsigned long drive[2]; //driv 0-1
  volatile unsigned long pull[2];  //pul 0-1
}v3s_gpio_t;

static v3s_gpio_t *gpio_base[] = {
    (v3s_gpio_t *)0x01C20800, /* GPIO_A */
    (v3s_gpio_t *)0x01C20824, /* GPIO_B */
    (v3s_gpio_t *)0x01C20848, /* GPIO_C */
    (v3s_gpio_t *)0x01C2086C, /* GPIO_D */
    (v3s_gpio_t *)0x01C20890, /* GPIO_E */
    (v3s_gpio_t *)0x01C208B4, /* GPIO_F */
    (v3s_gpio_t *)0x01C208D8, /* GPIO_G */
    (v3s_gpio_t *)0x01C208FC, /* GPIO_H */
    (v3s_gpio_t *)0x01C20920, /* GPIO_I */
    (v3s_gpio_t *)0x01F02c00, /* GPIO_L */
};

void gpio_config(u32 gpio, u32 pin, u32 val) {
  v3s_gpio_t *gp = gpio_base[gpio];
  int reg = pin / 8;
  int shift = (pin & 0x7) <<2;
  int tmp;
  tmp = gp->config[reg] & ~(0xf << shift);
  gp->config[reg] = tmp | (val << shift);
}

void gpio_pull(u32 gpio, u32 pin, u32 val) {
  v3s_gpio_t *gp = gpio_base[gpio];
  int reg = pin / 16;
  int shift = (pin & 0xf) <<1;
  int tmp;

  tmp = gp->pull[reg] & ~(0x3 << shift);
  gp->pull[reg] = tmp | (val << shift);
}

void gpio_output(u32 gpio, u32 pin, u32 val) {
  v3s_gpio_t *gp = gpio_base[gpio];
  if (val) {
    gp->data |= 1 << pin;
  } else {
    gp->data &= ~(1 << pin);
  }
}

u32 gpio_input(u32 gpio, u32 pin) {
  v3s_gpio_t *gp = gpio_base[gpio];
  return (gp->data >> pin) & 1;
}


void gpio_init_device(device_t* dev){

}