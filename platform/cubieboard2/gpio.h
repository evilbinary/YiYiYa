#ifndef GPIO_H
#define GPIO_H

#define MMIO_BASE       0x01C20800

#define SUNXI_PIO_BASE		0x01C20800
#define SUNXI_UART0_BASE	0x01C28000

#define GPFSEL0         ((volatile unsigned int*)(MMIO_BASE+0x00200000))
#define GPFSEL1         ((volatile unsigned int*)(MMIO_BASE+0x00200004))
#define GPFSEL2         ((volatile unsigned int*)(MMIO_BASE+0x00200008))
#define GPFSEL3         ((volatile unsigned int*)(MMIO_BASE+0x0020000C))
#define GPFSEL4         ((volatile unsigned int*)(MMIO_BASE+0x00200010))
#define GPFSEL5         ((volatile unsigned int*)(MMIO_BASE+0x00200014))
#define GPSET0          ((volatile unsigned int*)(MMIO_BASE+0x0020001C))
#define GPSET1          ((volatile unsigned int*)(MMIO_BASE+0x00200020))
#define GPCLR0          ((volatile unsigned int*)(MMIO_BASE+0x00200028))
#define GPLEV0          ((volatile unsigned int*)(MMIO_BASE+0x00200034))
#define GPLEV1          ((volatile unsigned int*)(MMIO_BASE+0x00200038))
#define GPEDS0          ((volatile unsigned int*)(MMIO_BASE+0x00200040))
#define GPEDS1          ((volatile unsigned int*)(MMIO_BASE+0x00200044))
#define GPHEN0          ((volatile unsigned int*)(MMIO_BASE+0x00200064))
#define GPHEN1          ((volatile unsigned int*)(MMIO_BASE+0x00200068))
#define GPPUD           ((volatile unsigned int*)(MMIO_BASE+0x00200094))
#define GPPUDCLK0       ((volatile unsigned int*)(MMIO_BASE+0x00200098))
#define GPPUDCLK1       ((volatile unsigned int*)(MMIO_BASE+0x0020009C))

/* PL011 UART registers */
#define UART0_DR        ((volatile unsigned int*)(SUNXI_UART0_BASE + 0x14))
#define UART0_FR        ((volatile unsigned int*)(SUNXI_UART0_BASE + 0x0))
#define UART0_CR        ((volatile unsigned int*)(SUNXI_UART0_BASE + 0xc))

#define UART0_IBRD      ((volatile unsigned int*)(SUNXI_UART0_BASE + 0x4))
#define UART0_FBRD      ((volatile unsigned int*)(SUNXI_UART0_BASE + 0x4))

#define UART0_LCRH      ((volatile unsigned int*)(MMIO_BASE+0x0020102C))
#define UART0_IMSC      ((volatile unsigned int*)(MMIO_BASE+0x00201038))
#define UART0_ICR       ((volatile unsigned int*)(MMIO_BASE+0x00201044))

/** timer*/
#define TIMER_BASE 0x01C20C00

#define TMR_IRQ_EN_REG  (TIMER_BASE+ 0x0 ) //Timer IRQ Enable Register
#define TMR_IRQ_STA_REG  (TIMER_BASE +  0x4 ) //Timer Status Register

#define TMR0_CTRL_REG  (TIMER_BASE +  0x10 ) //Timer 0 Control Register
#define TMR0_INTV_VALUE_REG  (TIMER_BASE +  0x14 ) //Timer 0 Interval Value Register
#define TMR0_CUR_VALUE_REG  (TIMER_BASE +  0x18 ) //Timer 0 Current Value Register

#define TMR1_CTRL_REG  (TIMER_BASE +  0x20 ) //Timer 1 Control Register
#define TMR1_INTV_VALUE_REG  (TIMER_BASE +  0x24) //Timer 1 Interval Value Register
 
#define TMR1_CUR_VALUE_REG  (TIMER_BASE +  0x28 ) //Timer 1 Current Value Register
#define TMR2_CTRL_REG  (TIMER_BASE +  0x30 ) //Timer 2 Control Register
#define TMR2_INTV_VALUE_REG  (TIMER_BASE +  0x34 ) //Timer 2 Interval Value Register
#define TMR2_CUR_VALUE_REG  (TIMER_BASE +  0x38 ) //Timer 2 Current Value Register

#define AVS_CNT_CTL_REG  (TIMER_BASE +  0x80 ) //AVS Control Register
#define AVS_CNT0_REG  (TIMER_BASE +  0x84 ) //AVS Counter 0 Register 
#define AVS_CNT1_REG  (TIMER_BASE +  0x88 ) //AVS Counter 1 Register

#define AVS_CNT_DIV_REG  (TIMER_BASE +  0x8C) //AVS Divisor Register
#define WDOG_IRQ_EN_REG  (TIMER_BASE +  0xA0) //Watchdog IRQ Enable Register
#define WDOG_IRQ_STA_REG  (TIMER_BASE +  0xA4 ) //Watchdog Status Register
#define WDOG_CTRL_REG  (TIMER_BASE +  0xB0 ) //Watchdog Control Register
#define WDOG_CFG_REG  (TIMER_BASE +  0xB4 ) //Watchdog Configuration Register
#define WDOG_MODE_REG  (TIMER_BASE +  0xB8 ) //Watchdog Mode Register
#define CNT64_TEST_REG  (TIMER_BASE +  0xD0 ) //64-bit Counter Test Mode Register
#define CNT64_CTRL_REG  (TIMER_BASE +  0xD4 ) //64-bit Counter Control Register
#define CNT64_LOW_REG  (TIMER_BASE +  0xD8 ) //64-bit Counter Low Register
#define CNT64_HIGH_REG  (TIMER_BASE +  0xDC ) //64-bit Counter High Register


#define GIC_BASE				(0x01C80000)
#define GIC_DIST				(GIC_BASE+0x1000)
#define GIC_CPUIF				(GIC_BASE+0x2000)

#define CORE0_TIMER_IRQCNTL 0x40000040
#define CORE0_IRQ_SOURCE 0x40000060

#endif