/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "dma/dma.h"
#include "kernel/kernel.h"

#define DSP_ADDR_PORT 0x224
#define DSP_DATA_PORT 0x225
#define DSP_RESET 0x226
#define DSP_READ 0x22A
#define DSP_WRITE 0x22C
#define DSP_STATUS 0x22E
#define DSP_INT_ACK 0x22F

#define MIXER_IRQ 0x25
#define MIXER_IRQ_DATA 0x2

#define DSP_SET_TIME 0x40
#define DSP_SET_RATE 0x41
#define DSP_ON 0xD1
#define DSP_OFF 0xD3
#define DSP_OFF_8 0xD0
#define DSP_ON_8 0xD4
#define DSP_OFF_16 0xD5
#define DSP_ON_16 0xD6
#define DSP_VERSION 0xE1

#define DSP_PROG_16 0xB0
#define DSP_PROG_8 0xC0
#define DSP_AUTO_INIT 0x06
#define DSP_PLAY 0x00
#define DSP_RECORD 0x08
#define DSP_MONO 0x00
#define DSP_STEREO 0x20
#define DSP_UNSIGNED 0x00
#define DSP_SIGNED 0x10

#define SAMPLE_RATE 44100

#define DSP_VOLUME 0x22
#define DSP_IRQ 0x80

#define DMA_CHANNEL_16 5
#define DMA_FLIP_FLOP 0xD8
#define DMA_BASE_ADDR 0xC4
#define DMA_COUNT 0xC6

#define SAMPLE_RATE 22050
#define BUFFER_MS 40
#define BUFFER_SIZE ((size_t)(SAMPLE_RATE * (BUFFER_MS / 1000.0)))

static i16 buffer[BUFFER_SIZE];
static bool buffer_flip = false;

void delay(u32 d) {
  for (int i = 0; i < d; i++)
    ;
}

void dsp_write(u8 value) {
  while (io_read8(DSP_WRITE) & 0x80)
    ;
  io_write8(DSP_WRITE, value);
}

u8 dsp_read() {
  while (!(io_read8(DSP_STATUS) & 0x80))
    ;
  return io_read8(DSP_READ);
}

void set_sample_rate(u16 hz) {
  dsp_write(DSP_SET_RATE);
  dsp_write((u8)((hz >> 8) & 0xFF));
  dsp_write((u8)(hz & 0xFF));
}

void dsp_reset() {
  io_write8(DSP_RESET, 1);
  delay(1000);
  io_write8(DSP_RESET, 0);

  if (dsp_read() != 0xAA) {
    kprintf("init sb16 failed\n");
  }

  io_write8(DSP_WRITE, DSP_VERSION);
  u8 major = io_read8(DSP_READ);
  u8 minor = io_read8(DSP_READ);

  kprintf("reset sb16 major %d minor %d\n", major, minor);
}

static void transfer(void* buf, u32 len) {
  u8 mode = 0x48;

  // disable DMA channel
  io_write8(DSP_ON_8, 4 + (DMA_CHANNEL_16 % 4));

  // clear byte-poiner flip-flop
  io_write8(DMA_FLIP_FLOP, 1);

  // write DMA mode for transfer
  io_write8(DSP_ON_16, (DMA_CHANNEL_16 % 4) | mode | (1 << 4));

  // write buffer offset (div 2 for 16-bit)
  u16 offset = (((uintptr_t)buf) / 2) % 65536;
  io_write8(DMA_BASE_ADDR, (u8)((offset >> 0) & 0xFF));
  io_write8(DMA_BASE_ADDR, (u8)((offset >> 8) & 0xFF));

  // write transfer length
  io_write8(DMA_COUNT, (u8)(((len - 1) >> 0) & 0xFF));
  io_write8(DMA_COUNT, (u8)(((len - 1) >> 8) & 0xFF));

  // write buffer
  io_write8(0x8B, ((uintptr_t)buf) >> 16);

  // enable DMA channel
  io_write8(0xD4, DMA_CHANNEL_16 % 4);
}

void do_sb16(interrupt_context_t* context) {
  kprintf("sb16 handler\n");

   buffer_flip = !buffer_flip;
    // fill(
    //     &buffer[buffer_flip ? 0 : (BUFFER_SIZE / 2)],
    //     (BUFFER_SIZE / 2)
    // );


 u16 sample_count = (BUFFER_SIZE / 2) - 1;
  dsp_write(DSP_PLAY | DSP_PROG_16 | DSP_AUTO_INIT);
  dsp_write(DSP_SIGNED | DSP_MONO);
  dsp_write((u8)((sample_count >> 0) & 0xFF));
  dsp_write((u8)((sample_count >> 8) & 0xFF));


  io_read8(DSP_STATUS);
  io_read8(DSP_INT_ACK);
}

INTERRUPT_SERVICE
void sb16_handler(interrupt_context_t* context) {
  interrupt_entering_code(MIXER_IRQ, 0);
  interrupt_process(do_sb16);
  interrupt_exit();
}

int sb16_init(void) {
  kprintf("sb16 init\n");
  // interrutp_regist(MIXER_IRQ, sb16_handler);

  // // reset
  // dsp_reset();

  // // set irq
  // io_write8(DSP_ADDR_PORT, DSP_IRQ);
  // io_write8(DSP_DATA_PORT, MIXER_IRQ_DATA);

  // // dma_trans(0,DMA_MODE_WRITE_TRANSFER | DMA_MODE_TRANSFER_BLOCK,buffer, ,
  // // size);

  // transfer(buffer, BUFFER_SIZE);
  // set_sample_rate(SAMPLE_RATE);

  // u16 sample_count = (BUFFER_SIZE / 2) - 1;
  // dsp_write(DSP_PLAY | DSP_PROG_16 | DSP_AUTO_INIT);
  // dsp_write(DSP_SIGNED | DSP_MONO);
  // dsp_write((u8)((sample_count >> 0) & 0xFF));
  // dsp_write((u8)((sample_count >> 8) & 0xFF));

  // dsp_write(DSP_ON);
  // dsp_write(DSP_ON_16);
  // pic_enable(MIXER_IRQ);

  return 0;
}

void sb16_exit(void) { kprintf("sb16 exit\n"); }

module_t sb16_module = {.name = "sb16", .init = sb16_init, .exit = sb16_exit};
