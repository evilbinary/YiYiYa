#ifndef GPIO_H
#define GPIO_H

#if defined(RASPI_Z_W)
#define MMIO_BASE 0x20000000
#elif defined(RASPI2)
#define MMIO_BASE 0x3F000000
#elif defined(RASPI3)
#define MMIO_BASE 0x3F000000
#elif defined(RASPI4)
#define MMIO_BASE 0xFE000000
#endif

#define MMIO_LENGTH 0x01000000

#define GPFSEL0 ((volatile unsigned int*)(MMIO_BASE + 0x00200000))
#define GPFSEL1 ((volatile unsigned int*)(MMIO_BASE + 0x00200004))
#define GPFSEL2 ((volatile unsigned int*)(MMIO_BASE + 0x00200008))
#define GPFSEL3 ((volatile unsigned int*)(MMIO_BASE + 0x0020000C))
#define GPFSEL4 ((volatile unsigned int*)(MMIO_BASE + 0x00200010))
#define GPFSEL5 ((volatile unsigned int*)(MMIO_BASE + 0x00200014))
#define GPSET0 ((volatile unsigned int*)(MMIO_BASE + 0x0020001C))
#define GPSET1 ((volatile unsigned int*)(MMIO_BASE + 0x00200020))
#define GPCLR0 ((volatile unsigned int*)(MMIO_BASE + 0x00200028))
#define GPLEV0 ((volatile unsigned int*)(MMIO_BASE + 0x00200034))
#define GPLEV1 ((volatile unsigned int*)(MMIO_BASE + 0x00200038))
#define GPEDS0 ((volatile unsigned int*)(MMIO_BASE + 0x00200040))
#define GPEDS1 ((volatile unsigned int*)(MMIO_BASE + 0x00200044))
#define GPHEN0 ((volatile unsigned int*)(MMIO_BASE + 0x00200064))
#define GPHEN1 ((volatile unsigned int*)(MMIO_BASE + 0x00200068))
#define GPPUD ((volatile unsigned int*)(MMIO_BASE + 0x00200094))
#define GPPUDCLK0 ((volatile unsigned int*)(MMIO_BASE + 0x00200098))
#define GPPUDCLK1 ((volatile unsigned int*)(MMIO_BASE + 0x0020009C))

/* PL011 UART registers */
#define UART0_DR ((volatile unsigned int*)(MMIO_BASE + 0x00201000))
#define UART0_FR ((volatile unsigned int*)(MMIO_BASE + 0x00201018))
#define UART0_IBRD ((volatile unsigned int*)(MMIO_BASE + 0x00201024))
#define UART0_FBRD ((volatile unsigned int*)(MMIO_BASE + 0x00201028))
#define UART0_LCRH ((volatile unsigned int*)(MMIO_BASE + 0x0020102C))
#define UART0_CR ((volatile unsigned int*)(MMIO_BASE + 0x00201030))
#define UART0_IMSC ((volatile unsigned int*)(MMIO_BASE + 0x00201038))
#define UART0_ICR ((volatile unsigned int*)(MMIO_BASE + 0x00201044))

// Timers interrupt control registers
#define CORE0_TIMER_IRQCNTL 0x40000040
#define CORE1_TIMER_IRQCNTL 0x40000044
#define CORE2_TIMER_IRQCNTL 0x40000048
#define CORE3_TIMER_IRQCNTL 0x4000004C

// Where to route timer interrupt to, IRQ/FIQ
// Setting both the IRQ and FIQ bit gives an FIQ
#define TIMER0_IRQ 0x01
#define TIMER1_IRQ 0x02
#define TIMER2_IRQ 0x04
#define TIMER3_IRQ 0x08
#define TIMER0_FIQ 0x10
#define TIMER1_FIQ 0x20
#define TIMER2_FIQ 0x40
#define TIMER3_FIQ 0x80

// Mailbox interrupt control registers
#define CORE0_MBOX_IRQCNTL 0x40000050
#define CORE1_MBOX_IRQCNTL 0x40000054
#define CORE2_MBOX_IRQCNTL 0x40000058
#define CORE3_MBOX_IRQCNTL 0x4000005C

// /// Where to route mailbox interrupt IRQ/FIQ
#define MBOX0_IRQ 0x01
#define MBOX1_IRQ 0x02
#define MBOX2_IRQ 0x04
#define MBOX3_IRQ 0x08
#define MBOX0_FIQ 0x10
#define MBOX1_FIQ 0x20
#define MBOX2_FIQ 0x40
#define MBOX3_FIQ 0x80

// IRQ & FIQ source registers
#define CORE0_IRQ_SOURCE 0x40000060
#define CORE1_IRQ_SOURCE 0x40000064
#define CORE2_IRQ_SOURCE 0x40000068
#define CORE3_IRQ_SOURCE 0x4000006C
#define CORE0_FIQ_SOURCE 0x40000070
#define CORE1_FIQ_SOURCE 0x40000074
#define CORE2_FIQ_SOURCE 0x40000078
#define CORE3_FIQ_SOURCE 0x4000007C

#define INT_SRC_TIMER0 0x00000001
#define INT_SRC_TIMER1 0x00000002
#define INT_SRC_TIMER2 0x00000004
#define INT_SRC_TIMER3 0x00000008
#define INT_SRC_MBOX0 0x00000010
#define INT_SRC_MBOX1 0x00000020
#define INT_SRC_MBOX2 0x00000040
#define INT_SRC_MBOX3 0x00000080
#define INT_SRC_GPU 0x00000100
#define INT_SRC_PMU 0x00000200

// Mailbox write-set registers (Write only)
#define CORE0_MBOX0_SET 0x40000080
#define CORE0_MBOX1_SET 0x40000084
#define CORE0_MBOX2_SET 0x40000088
#define CORE0_MBOX3_SET 0x4000008C

#define CORE1_MBOX0_SET 0x40000090
#define CORE1_MBOX1_SET 0x40000094
#define CORE1_MBOX2_SET 0x40000098
#define CORE1_MBOX3_SET 0x4000009C
#define CORE2_MBOX0_SET 0x400000A0
#define CORE2_MBOX1_SET 0x400000A4
#define CORE2_MBOX2_SET 0x400000A8
#define CORE2_MBOX3_SET 0x400000AC
#define CORE3_MBOX0_SET 0x400000B0
#define CORE3_MBOX1_SET 0x400000B4
#define CORE3_MBOX2_SET 0x400000B8
#define CORE3_MBOX3_SET 0x400000BC

// Mailbox write-clear registers (Read & Write) 
#define CORE0_MBOX0_RDCLR 0x400000C0
#define CORE0_MBOX1_RDCLR 0x400000C4
#define CORE0_MBOX2_RDCLR 0x400000C8
#define CORE0_MBOX3_RDCLR 0x400000CC
#define CORE1_MBOX0_RDCLR 0x400000D0
#define CORE1_MBOX1_RDCLR 0x400000D4
#define CORE1_MBOX2_RDCLR 0x400000D8
#define CORE1_MBOX3_RDCLR 0x400000DC
#define CORE2_MBOX0_RDCLR 0x400000E0
#define CORE2_MBOX1_RDCLR 0x400000E4
#define CORE2_MBOX2_RDCLR 0x400000E8
#define CORE2_MBOX3_RDCLR 0x400000EC
#define CORE3_MBOX0_RDCLR 0x400000F0
#define CORE3_MBOX1_RDCLR 0x400000F4
#define CORE3_MBOX2_RDCLR 0x400000F8
#define CORE3_MBOX3_RDCLR 0x400000FC

#endif