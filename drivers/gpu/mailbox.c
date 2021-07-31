#include "mailbox.h"

u32 mailbox_read(int channel) {
  uint32_t data;
  do {
    while (BCM2835_MAILBOX->STATUS & BCM2835_MAILBOX_STATUS_RE)
      ;  // do nothing
    data = BCM2835_MAILBOX->READ;
  } while ((uint8_t)(data & 0xf) != channel);
  return (data & ~0xf);
}

void mailbox_write(int channel, u32 data) {
  while (BCM2835_MAILBOX->STATUS & BCM2835_MAILBOX_STATUS_WF)
    ;  // do nothing
  BCM2835_MAILBOX->WRITE = (data & ~0xf) | (uint32_t)(channel & 0xf);
}

void udelay(uint32_t d) {
  const uint32_t clo = BCM2835_ST->CLO;
  while ((BCM2835_ST->CLO - clo) < d) {
  }
}

void mailbox_flush(void) {
  while (!(BCM2835_MAILBOX->STATUS & BCM2835_MAILBOX_STATUS_RE)) {
    (void)BCM2835_MAILBOX->READ;
    // udelay(20);
  }
}

uint32_t mailbox_write_read(uint8_t channel, uint32_t data) {
#if defined(RPI2) || defined(RPI3)
  // clean_data_cache();
#endif

  dsb();

#if defined(RPI2) || defined(RPI3)
  // dmb();
#endif
  mailbox_flush();

  mailbox_write(channel, data);
  uint32_t address = mailbox_read(channel);
#if defined(RPI2) || defined(RPI3)
  // dmb();
  // invalidate_data_cache();
#endif

  // dmb();

  return address;
}
