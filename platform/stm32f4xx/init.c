#include "libs/include/types.h"

typedef volatile u32 IoRegister;
typedef struct stm32_usart {
  IoRegister SR;
  IoRegister DR;
  IoRegister BRR;
  IoRegister CR1;
  IoRegister CR2;
  IoRegister CR3;
  IoRegister GTPR;
} stm32_usart_t;

typedef struct stm32_gpio {
  u32 MODER; /*!< GPIO port mode register,               Address offset: 0x00 */
  u32 OTYPER;  /*!< GPIO port output type register,        Address offset: 0x04
                */
  u32 OSPEEDR; /*!< GPIO port output speed register,       Address offset: 0x08
                */
  u32 PUPDR; /*!< GPIO port pull-up/pull-down register,  Address offset: 0x0C */
  u32 IDR; /*!< GPIO port input data register,         Address offset: 0x10   */
  u32 ODR; /*!< GPIO port output data register,        Address offset: 0x14   */
  u16 BSRRL; /*!< GPIO port bit set/reset low register,  Address offset:
                     0x18      */
  u16 BSRRH; /*!< GPIO port bit set/reset high register, Address offset:
                     0x1A      */
  u32 LCKR;  /*!< GPIO port configuration lock register, Address offset: 0x1C */
  u32 AFR[2]; /*!< GPIO alternate function registers,     Address offset:
                 0x20-0x24 */
} stm32_gpio_t;

#define uart1 \
  (stm32_usart_t *)(0x40011000)  // APB2 PA8  clk PA9 tx PA10 rx ,max 84 MHz
#define uart2 (stm32_usart_t *)(0x40004400)  // APB1 max 42 MHz
#define uart6 (stm32_usart_t *)(0x40011400)  // APB2 max 84 MHz

uart_init() {
  stm32_usart_t *usart = uart1;
  long long mode;
  int desiredBaudrate = 115200;
  int peripheralFreq = 45000000;

  int mantissa, fractional;
  float divider;
  int shiftFrac = 8;
  int over8 = 1;

  usart->CR1 = (u32)(mode & 0xffff);
  usart->CR2 = (u32)((mode >> 16) & 0xffff);
  usart->CR3 = (u32)(mode >> 32);

  // if (usartOverSample16(usart)) {
  //   shiftFrac = 16;
  //   over8 = 0;
  // }

  divider = ((float)peripheralFreq / (8 * ((2 - over8) * desiredBaudrate)));
  mantissa = (int)divider;
  fractional = (int)((float)(divider - mantissa) * shiftFrac);

  usart->BRR = (mantissa << 4) | (fractional);
}

void timer_init(int hz) { kprintf("timer init\n"); }
void timer_end() {}

void uart_send(unsigned int c) {
  stm32_usart_t *usart = uart1;
  usart->DR = c & 0x01FF;
}

void platform_init() {
  uart_init();
  uart_send('A');
}