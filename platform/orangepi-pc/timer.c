/* This is a driver for the H3 Allwinner timer section
 *
 * Tom Trebisky  12-30-2016
 */

int timer_count;

struct v3s_timer {
  volatile unsigned int irq_ena;    /* 00 */
  volatile unsigned int irq_status; /* 04 */
  int __pad1[2];
  volatile unsigned int t0_ctrl; /* 10 */
  volatile unsigned int t0_ival; /* 14 */
  volatile unsigned int t0_cval; /* 18 */
  int __pad2;
  volatile unsigned int t1_ctrl; /* 20 */
  volatile unsigned int t1_ival; /* 24 */
  volatile unsigned int t1_cval; /* 28 */
};

#define TIMER_BASE ((struct v3s_timer *)0x01c20c00)

#define CTL_ENABLE 0x01
#define CTL_RELOAD 0x02 /* reload ival */
#define CTL_SRC_32K 0x00
#define CTL_SRC_24M 0x04

#define CTL_PRE_1 0x00
#define CTL_PRE_2 0x10
#define CTL_PRE_4 0x20
#define CTL_PRE_8 0x30
#define CTL_PRE_16 0x40
#define CTL_PRE_32 0x50
#define CTL_PRE_64 0x60
#define CTL_PRE_128 0x70

#define CTL_SINGLE 0x80
#define CTL_AUTO 0x00

#define IE_T0 0x01
#define IE_T1 0x02

#define IRQ_TIMER0 50

#define MEG 1000000

void timer_handler(int);

/* The timer is a down counter,
 *  intended to generate periodic interrupts
 * There are two of these.
 * There is also a watchdog and
 * an "AVS" timer (Audio/Video Synchronization)
 *  neither of which are supported here.
 *
 * The datasheet says they are 24 bit counters, but
 *  experiment clearly shows T0 and T1 are 32 bit,
 *  so the 24 bit claim perhaps/probably applies
 *  to the watchdog, who can say?
 *  Foggy documentation at best.
 * 24 bits can hold values up to 16,777,215
 */

/* All indications are that this is a 32 bit counter running
 *  -- at 30,384 Hz when we ask for 32K
 *  -- at 23897172 when we ask for 24M
 *  (note that this is off by .0045 percent, but in
 *   actual fact we are using the same crystal to run
 *   the CPU so this is meaningless.
 */

#define CLOCK_24M 24000000
#define CLOCK_24M_MS 24000
#define NUM_INTS 157

#define PMCR_ENABLE 0x01     /* enable all counters */
#define PMCR_EV_RESET 0x02   /* reset all event counters */
#define PMCR_CC_RESET 0x04   /* reset CCNT */
#define PMCR_CC_DIV 0x08     /* divide CCNT by 64 */
#define PMCR_EXPORT 0x10     /* export events */
#define PMCR_CC_DISABLE 0x20 /* disable CCNT in non-invasive regions */

/* There are 4 counters besides the CCNT (we ignore them at present) */
#define CENA_CCNT 0x80000000
#define CENA_CTR3 0x00000008
#define CENA_CTR2 0x00000004
#define CENA_CTR1 0x00000002
#define CENA_CTR0 0x00000001

void ccnt_enable(int div64) {
  int val;

  // val = get_pmcr ();
  asm volatile("mrc p15, 0, %0, c9, c12, 0" : "=r"(val));
  // printf ( " PMCR = %08x\n", val );
  val |= PMCR_ENABLE;
  if (div64) val |= PMCR_CC_DIV;
  // set_pmcr ( val );
  asm volatile("mcr p15, 0, %0, c9, c12, 0" : : "r"(val));

  asm volatile("mrc p15, 0, %0, c9, c12, 0" : "=r"(val));
  // printf ( " PMCR = %08x\n", val );

  // set_cena ( CENA_CCNT );
  val = CENA_CCNT;
  asm volatile("mcr p15, 0, %0, c9, c12, 1" : : "r"(val));

  asm volatile("mrc p15, 0, %0, c9, c12, 1" : "=r"(val));
  // printf ( " CENA = %08x\n", val );
}

void ccnt_reset(void) {
  int val;

  // set_pmcr ( get_pmcr() | PMCR_CC_RESET );
  asm volatile("mrc p15, 0, %0, c9, c12, 0" : "=r"(val));
  val |= PMCR_CC_RESET;
  asm volatile("mcr p15, 0, %0, c9, c12, 0" : : "r"(val));
}

void ms_delay(int ms) {
  int count;
  int limit = ms * MEG;

  ccnt_reset();

  for (;;) {
    asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(count));
    if (count > limit) break;
  }
}

void timer_init2(int hz) {
  struct v3s_timer *hp = TIMER_BASE;
  // hp->t0_ival = 0x000010;
  // hp->t0_ival = 0x800000000;
  hp->t0_ival = CLOCK_24M / hz;

  hp->t0_ctrl = 0; /* stop the timer */
  hp->irq_ena = IE_T0;

  hp->t0_ctrl = CTL_SRC_24M;
  hp->t0_ctrl |= CTL_RELOAD;
  while (hp->t0_ctrl & CTL_RELOAD)
    ;

  hp->t0_ctrl |= CTL_ENABLE;
  
  kprintf ("  Timer I val: %x\n", hp->t0_ival );
  kprintf ("  Timer C val: %x\n", hp->t0_cval );
  kprintf ("  Timer C val: %x\n", hp->t0_cval );
  
}

/* One shot, delay in milliseconds */
void timer_one(int delay) {
  struct v3s_timer *hp = TIMER_BASE;

  hp->t0_ival = CLOCK_24M_MS * delay;

  hp->t0_ctrl = 0; /* stop the timer */
  hp->irq_ena = IE_T0;

  hp->t0_ctrl = CTL_SRC_24M | CTL_SINGLE;
  hp->t0_ctrl |= CTL_RELOAD;
  while (hp->t0_ctrl & CTL_RELOAD)
    ;
  hp->t0_ctrl |= CTL_ENABLE;
}

void timer_ack(void) {
  struct v3s_timer *hp = TIMER_BASE;
  hp->irq_status = IE_T0;
}

/* Called at interrupt level */
void timer_handler(int junk) {
  timer_count++;
  timer_ack();
  // kprintf("timer_handler %d\n",timer_count);
}

void timer_watch(void) {
  struct v3s_timer *hp = TIMER_BASE;
  int val;
  int last;
  int del;

  val = hp->t0_cval;

  for (;;) {
    last = val;
    val = hp->t0_cval;
    /*
    del = last - val;
    printf ("  Timer: 0x%08x %d = %d\n", val, val, del );
    */
    kprintf("  Timer: 0x%x %d = %x\n", val, val, hp->irq_status);
    /*
    if ( hp->irq_status )
        timer_ack ();
    printf (" =Timer: 0x%08x %d = %08x\n", val, val, hp->irq_status );
    */
    // gig_delay ( 2 );
    ms_delay(2000);
  }
}

/* THE END */
