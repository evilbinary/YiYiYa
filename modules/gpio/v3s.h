#ifndef _V3S_GPIO_H__
#define _V3S_GPIO_H__

// pull config
#define GPIO_PULL_DISABLE (0)   // 00: Pull-up/down disable
#define GPIO_PULL_UP (1)        // 01: Pull-up
#define GPIO_PULL_DOWN (2)      // 10: Pull-down
#define GPIO_PULL_RESERVED (3)  // 11: Reserved

// cfg config
#define GPIO_INPUT (0)    // 000: Input
#define GPIO_OUTPUT (1)   // 001: Output
#define GPIO_DISABLE (7)  // 111: IO Disable

#define GPIO_A 0
#define GPIO_B 1  // Port B(PB): 10 input/output port
#define GPIO_C 2  // 4 input/output port
#define GPIO_D 3
#define GPIO_E 4  // 25 input/output port
#define GPIO_F 5  // 7 input/output port
#define GPIO_G 6  // 6 input/output port
#define GPIO_H 7
#define GPIO_I 8
#define GPIO_L 9

#endif