
#include "v3s.h"

#define UNSTUFF_BITS(resp, start, size)                     \
  ({                                                        \
    const int __size = size;                                \
    const u32 __mask = (__size < 32 ? 1 << __size : 0) - 1; \
    const int __off = 3 - ((start) / 32);                   \
    const int __shft = (start)&31;                          \
    u32 __res;                                              \
                                                            \
    __res = resp[__off] >> __shft;                          \
    if (__size + __shft > 32)                               \
      __res |= resp[__off - 1] << ((32 - __shft) % 32);     \
    __res &__mask;                                          \
  })

const unsigned tran_speed_unit[] = {
    [0] = 10000,
    [1] = 100000,
    [2] = 1000000,
    [3] = 10000000,
};

const unsigned char tran_speed_time[] = {
    0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80,
};

char *ssize(char *buf, u32 size) {
  const char *unit[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
  int count = 0;

  while ((size > 1024) && (count < 8)) {
    size /= 1024;
    count++;
  }
  sprintf(buf, "%d%s", size, unit[count]);
  return buf;
}

static char *sdcard_version_string(sdhci_v3s_pdata_t *pdat) {
  static char version[sizeof("xx.xxx")];
  unsigned int major, minor, micro;
  int n;

  major = (pdat->version >> 8) & 0xf;
  minor = (pdat->version >> 4) & 0xf;
  micro = pdat->version & 0xf;
  n = sprintf(version, "%u.%u", major, minor);
  if (micro) sprintf(version + n, "%u", micro);
  return version;
}

static unsigned int extract_mid(sdhci_v3s_pdata_t *pdat) {
  if ((pdat->version & MMC_VERSION_MMC) && (pdat->version <= MMC_VERSION_1_4))
    return UNSTUFF_BITS(pdat->cid, 104, 24);
  else
    return UNSTUFF_BITS(pdat->cid, 120, 8);
}

static unsigned int extract_oid(sdhci_v3s_pdata_t *pdat) {
  return (pdat->cid[0] >> 8) & 0xffff;
}

static unsigned int extract_prv(sdhci_v3s_pdata_t *pdat) {
  return (pdat->cid[2] >> 24);
}

static unsigned int extract_psn(sdhci_v3s_pdata_t *pdat) {
  if (pdat->version & SD_VERSION_SD) {
    return UNSTUFF_BITS(pdat->csd, 24, 32);
  } else {
    if (pdat->version > MMC_VERSION_1_4)
      return UNSTUFF_BITS(pdat->cid, 16, 32);
    else
      return UNSTUFF_BITS(pdat->cid, 16, 24);
  }
}

static unsigned int extract_month(sdhci_v3s_pdata_t *pdat) {
  if (pdat->version & SD_VERSION_SD)
    return UNSTUFF_BITS(pdat->cid, 8, 4);
  else
    return UNSTUFF_BITS(pdat->cid, 12, 4);
}

static unsigned int extract_year(sdhci_v3s_pdata_t *pdat) {
  unsigned int year;

  if (pdat->version & SD_VERSION_SD)
    year = UNSTUFF_BITS(pdat->cid, 12, 8) + 2000;
  else if (pdat->version < MMC_VERSION_4_41)
    return UNSTUFF_BITS(pdat->cid, 8, 4) + 1997;
  else {
    year = UNSTUFF_BITS(pdat->cid, 8, 4) + 1997;
    if (year < 2010) year += 16;
  }
  return year;
}

static int go_idle_state(sdhci_device_t *hci) {
  sdhci_v3s_pdata_t *pdat = (sdhci_v3s_pdata_t *)hci->data;

  sdhci_cmd_t cmd = {0};

  cmd.cmdidx = MMC_GO_IDLE_STATE;
  cmd.cmdarg = 0;
  cmd.resptype = pdat->isspi ? MMC_RSP_R1 : MMC_RSP_NONE;

  if (sdhci_v3s_transfer(hci, &cmd, NULL)) return TRUE;
  return sdhci_v3s_transfer(hci, &cmd, NULL);
}

static int v3s_transfer_command(sdhci_v3s_pdata_t *pdat, sdhci_cmd_t *cmd,
                                sdhci_data_t *dat) {
  u32 cmdval = SDXC_START;
  u32 status = 0;
  int timeout = 0;

  if (cmd->cmdidx == MMC_STOP_TRANSMISSION) {
    timeout = 10000;
    do {
      status = io_read32(pdat->virt + SD_STAR);
      if (!timeout--) {
        io_write32(pdat->virt + SD_GCTL, SDXC_HARDWARE_RESET);
        io_write32(pdat->virt + SD_RISR, 0xffffffff);
        kprintf("v3s_transfer_command failed %d\n", status);
        return FALSE;
      }
    } while (status & SDXC_CARD_DATA_BUSY);
    return TRUE;
  }

  if (cmd->cmdidx == MMC_GO_IDLE_STATE) cmdval |= SDXC_SEND_INIT_SEQUENCE;
  if (cmd->resptype & MMC_RSP_PRESENT) {
    cmdval |= SDXC_RESP_EXPIRE;
    if (cmd->resptype & MMC_RSP_136) cmdval |= SDXC_LONG_RESPONSE;
    if (cmd->resptype & MMC_RSP_CRC) cmdval |= SDXC_CHECK_RESPONSE_CRC;
  }

  if (dat) cmdval |= SDXC_DATA_EXPIRE | SDXC_WAIT_PRE_OVER;

  if (dat && (dat->flag & MMC_DATA_WRITE)) cmdval |= SDXC_WRITE;

  if (cmd->cmdidx == MMC_WRITE_MULTIPLE_BLOCK ||
      cmd->cmdidx == MMC_READ_MULTIPLE_BLOCK)
    cmdval |= SDXC_SEND_AUTO_STOP;

  io_write32(pdat->virt + SD_CAGR, cmd->cmdarg);

  if (dat)
    io_write32(pdat->virt + SD_GCTL,
               io_read32(pdat->virt + SD_GCTL) | 0x80000000);
  io_write32(pdat->virt + SD_CMDR, cmdval | cmd->cmdidx);

  timeout = 10000;
  do {
    status = io_read32(pdat->virt + SD_RISR);
    if (!timeout-- || (status & SDXC_INTERRUPT_ERROR_BIT)) {
      io_write32(pdat->virt + SD_GCTL, SDXC_HARDWARE_RESET);
      io_write32(pdat->virt + SD_RISR, 0xffffffff);
      kprintf("v3s_transfer_command 2 failed %d\n", status);
      return FALSE;
    }
  } while (!(status & SDXC_COMMAND_DONE));

  if (cmd->resptype & MMC_RSP_BUSY) {
    timeout = 10000;
    do {
      status = io_read32(pdat->virt + SD_STAR);
      if (!timeout--) {
        io_write32(pdat->virt + SD_GCTL, SDXC_HARDWARE_RESET);
        io_write32(pdat->virt + SD_RISR, 0xffffffff);
        kprintf("v3s_transfer_command 3 failed %d\n", status);
        return FALSE;
      }
    } while (status & (1 << 9));
  }

  if (cmd->resptype & MMC_RSP_136) {
    cmd->response[0] = io_read32(pdat->virt + SD_RESP3);
    cmd->response[1] = io_read32(pdat->virt + SD_RESP2);
    cmd->response[2] = io_read32(pdat->virt + SD_RESP1);
    cmd->response[3] = io_read32(pdat->virt + SD_RESP0);
  } else {
    cmd->response[0] = io_read32(pdat->virt + SD_RESP0);
  }
  io_write32(pdat->virt + SD_RISR, 0xffffffff);
  return TRUE;
}

static int read_bytes(sdhci_v3s_pdata_t *pdat, u32 *buf, u32 blkcount,
                      u32 blksize) {
  u64 count = blkcount * blksize;
  u32 *tmp = buf;
  u32 status, err, done;

  status = io_read32(pdat->virt + SD_STAR);
  err = io_read32(pdat->virt + SD_RISR) & SDXC_INTERRUPT_ERROR_BIT;
  while ((!err) && (count >= sizeof(u32))) {
    if (!(status & SDXC_FIFO_EMPTY)) {
      *(tmp) = io_read32(pdat->virt + SD_FIFO);
      tmp++;
      count -= sizeof(u32);
    }
    status = io_read32(pdat->virt + SD_STAR);
    err = io_read32(pdat->virt + SD_RISR) & SDXC_INTERRUPT_ERROR_BIT;
  }

  do {
    status = io_read32(pdat->virt + SD_RISR);
    err = status & SDXC_INTERRUPT_ERROR_BIT;
    if (blkcount > 1)
      done = status & SDXC_AUTO_COMMAND_DONE;
    else
      done = status & SDXC_DATA_OVER;
  } while (!done && !err);

  if (err & SDXC_INTERRUPT_ERROR_BIT) return FALSE;
  io_write32(pdat->virt + SD_RISR, 0xffffffff);

  if (count) return FALSE;
  return TRUE;
}

static int write_bytes(sdhci_v3s_pdata_t *pdat, u32 *buf, u32 blkcount,
                       u32 blksize) {
  u64 count = blkcount * blksize;
  u32 *tmp = buf;
  u32 status, err, done;

  status = io_read32(pdat->virt + SD_STAR);
  err = io_read32(pdat->virt + SD_RISR) & SDXC_INTERRUPT_ERROR_BIT;
  while (!err && count) {
    if (!(status & SDXC_FIFO_FULL)) {
      io_write32(pdat->virt + SD_FIFO, *tmp);
      tmp++;
      count -= sizeof(u32);
    }
    status = io_read32(pdat->virt + SD_STAR);
    err = io_read32(pdat->virt + SD_RISR) & SDXC_INTERRUPT_ERROR_BIT;
  }

  do {
    status = io_read32(pdat->virt + SD_RISR);
    err = status & SDXC_INTERRUPT_ERROR_BIT;
    if (blkcount > 1)
      done = status & SDXC_AUTO_COMMAND_DONE;
    else
      done = status & SDXC_DATA_OVER;
  } while (!done && !err);

  if (err & SDXC_INTERRUPT_ERROR_BIT) return FALSE;
  io_write32(pdat->virt + SD_GCTL,
             io_read32(pdat->virt + SD_GCTL) | SDXC_FIFO_RESET);
  io_write32(pdat->virt + SD_RISR, 0xffffffff);

  if (count) return FALSE;
  return TRUE;
}

static int v3s_transfer_data(sdhci_v3s_pdata_t *pdat, sdhci_cmd_t *cmd,
                             sdhci_data_t *dat) {
  u32 dlen = (u32)(dat->blkcnt * dat->blksz);
  int ret = FALSE;

  io_write32(pdat->virt + SD_BKSR, dat->blksz);
  io_write32(pdat->virt + SD_BYCR, dlen);
  if (dat->flag & MMC_DATA_READ) {
    if (!v3s_transfer_command(pdat, cmd, dat)) return FALSE;
    ret = read_bytes(pdat, (u32 *)dat->buf, dat->blkcnt, dat->blksz);
  } else if (dat->flag & MMC_DATA_WRITE) {
    if (!v3s_transfer_command(pdat, cmd, dat)) return FALSE;
    ret = write_bytes(pdat, (u32 *)dat->buf, dat->blkcnt, dat->blksz);
  }
  return ret;
}

static int sdhci_v3s_detect(sdhci_device_t *sdhci) {
  sdhci_v3s_pdata_t *pdat = (sdhci_v3s_pdata_t *)sdhci->data;

  // if ((pdat->cd >= 0) && gpio_get_value(pdat->cd)) return FALSE;
  return TRUE;
}

static int sdhci_v3s_reset(sdhci_device_t *sdhci) {
  sdhci_v3s_pdata_t *pdat = (sdhci_v3s_pdata_t *)sdhci->data;

  io_write32(pdat->virt + SD_GCTL, SDXC_HARDWARE_RESET);
  return TRUE;
}

static int sdhci_v3s_setvoltage(sdhci_device_t *sdhci, u32 voltage) {
  return TRUE;
}

static int sdhci_v3s_setwidth(sdhci_device_t *sdhci, u32 width) {
  sdhci_v3s_pdata_t *pdat = (sdhci_v3s_pdata_t *)sdhci->data;

  switch (width) {
    case MMC_BUS_WIDTH_1:
      io_write32(pdat->virt + SD_BWDR, SDXC_WIDTH1);
      break;
    case MMC_BUS_WIDTH_4:
      io_write32(pdat->virt + SD_BWDR, SDXC_WIDTH4);
      break;
    case MMC_BUS_WIDTH_8:
      io_write32(pdat->virt + SD_BWDR, SDXC_WIDTH8);
      break;
    default:
      return FALSE;
  }
  return TRUE;
}

static int sdhci_v3s_update_clk(sdhci_v3s_pdata_t *pdat) {
  u32 cmd = (1U << 31) | (1 << 21) | (1 << 13);
  int timeout = 10000;

  io_write32(pdat->virt + SD_CMDR, cmd);
  while ((io_read32(pdat->virt + SD_CMDR) & 0x80000000) && timeout--)
    ;
  if (!timeout) return FALSE;
  io_write32(pdat->virt + SD_RISR, io_read32(pdat->virt + SD_RISR));
  return TRUE;
}

static int sdhci_v3s_setclock(sdhci_device_t *sdhci, u32 clock) {
  sdhci_v3s_pdata_t *pdat = (sdhci_v3s_pdata_t *)sdhci->data;
  u32 ratio = (pdat->clock + 2 * clock - 1) / (2 * clock);

  if ((ratio & 0xff) != ratio) return FALSE;
  io_write32(pdat->virt + SD_CKCR,
             io_read32(pdat->virt + SD_CKCR) & ~(1 << 16));
  io_write32(pdat->virt + SD_CKCR, ratio);
  if (!sdhci_v3s_update_clk(pdat)) return FALSE;
  io_write32(pdat->virt + SD_CKCR, io_read32(pdat->virt + SD_CKCR) | (3 << 16));
  if (!sdhci_v3s_update_clk(pdat)) return FALSE;
  return TRUE;
}

static int sdhci_v3s_transfer(sdhci_device_t *sdhci, sdhci_cmd_t *cmd,
                              sdhci_data_t *dat) {
  sdhci_v3s_pdata_t *pdat = (sdhci_v3s_pdata_t *)sdhci->data;
  if (!dat) {
    return v3s_transfer_command(pdat, cmd, dat);
  }
  return v3s_transfer_data(pdat, cmd, dat);
}

static int sd_send_op_cond(sdhci_device_t *hci) {
  sdhci_v3s_pdata_t *pdat = (sdhci_v3s_pdata_t *)hci->data;
  sdhci_cmd_t cmd = {0};
  int retries = 100;

  do {
    cmd.cmdidx = MMC_APP_CMD;
    cmd.cmdarg = 0;
    cmd.resptype = MMC_RSP_R1;
    if (!sdhci_v3s_transfer(hci, &cmd, NULL)) continue;

    cmd.cmdidx = SD_CMD_APP_SEND_OP_COND;
    if (pdat->isspi) {
      cmd.cmdarg = 0;
      if (pdat->version == SD_VERSION_2) cmd.cmdarg |= OCR_HCS;
      cmd.resptype = MMC_RSP_R1;
      if (sdhci_v3s_transfer(hci, &cmd, NULL)) break;
    } else {
      if (pdat->voltage & MMC_VDD_27_36)
        cmd.cmdarg = 0x00ff8000;
      else if (pdat->voltage & MMC_VDD_165_195)
        cmd.cmdarg = 0x00000080;
      else
        cmd.cmdarg = 0;
      if (pdat->version == SD_VERSION_2) cmd.cmdarg |= OCR_HCS;
      cmd.resptype = MMC_RSP_R3;
      if (!sdhci_v3s_transfer(hci, &cmd, NULL) || (cmd.response[0] & OCR_BUSY))
        break;
    }
  } while (retries--);

  if (retries <= 0) return FALSE;

  if (pdat->version != SD_VERSION_2) pdat->version = SD_VERSION_1_0;
  if (pdat->isspi) {
    cmd.cmdidx = MMC_SPI_READ_OCR;
    cmd.cmdarg = 0;
    cmd.resptype = MMC_RSP_R3;
    if (!sdhci_v3s_transfer(hci, &cmd, NULL)) return FALSE;
  }
  pdat->ocr = cmd.response[0];
  pdat->high_capacity = ((pdat->ocr & OCR_HCS) == OCR_HCS);
  pdat->rca = 0;

  return TRUE;
}

static int mmc_send_op_cond(sdhci_device_t *hci) {
  sdhci_v3s_pdata_t *pdat = (sdhci_v3s_pdata_t *)hci->data;

  sdhci_cmd_t cmd = {0};
  int retries = 100;

  if (!go_idle_state(hci)) return FALSE;

  cmd.cmdidx = MMC_SEND_OP_COND;
  cmd.cmdarg = 0;
  cmd.resptype = MMC_RSP_R3;
  if (!sdhci_v3s_transfer(hci, &cmd, NULL)) return FALSE;

  do {
    cmd.cmdidx = MMC_SEND_OP_COND;
    cmd.cmdarg = pdat->isspi ? 0
                             : (pdat->ocr & OCR_VOLTAGE_MASK) |
                                   (pdat->ocr & OCR_ACCESS_MODE);
    cmd.cmdarg |= OCR_HCS;
    cmd.resptype = MMC_RSP_R3;
    if (!sdhci_v3s_transfer(hci, &cmd, NULL)) return FALSE;
  } while (!(cmd.response[0] & OCR_BUSY) && retries--);

  if (retries <= 0) return FALSE;

  if (pdat->isspi) {
    cmd.cmdidx = MMC_SPI_READ_OCR;
    cmd.cmdarg = 0;
    cmd.resptype = MMC_RSP_R3;
    if (!sdhci_v3s_transfer(hci, &cmd, NULL)) return FALSE;
  }
  pdat->version = MMC_VERSION_UNKNOWN;
  pdat->ocr = cmd.response[0];
  pdat->high_capacity = ((pdat->ocr & OCR_HCS) == OCR_HCS);
  pdat->rca = 0;
  return TRUE;
}

static int sd_send_if_cond(sdhci_device_t *hci) {
  sdhci_v3s_pdata_t *pdat = (sdhci_v3s_pdata_t *)hci->data;
  sdhci_cmd_t cmd = {0};

  cmd.cmdidx = SD_CMD_SEND_IF_COND;
  if (pdat->voltage & MMC_VDD_27_36)
    cmd.cmdarg = (0x1 << 8);
  else if (pdat->voltage & MMC_VDD_165_195)
    cmd.cmdarg = (0x2 << 8);
  else
    cmd.cmdarg = (0x0 << 8);
  cmd.cmdarg |= 0xaa;
  cmd.resptype = MMC_RSP_R7;
  if (!sdhci_v3s_transfer(hci, &cmd, NULL)) return FALSE;

  if ((cmd.response[0] & 0xff) != 0xaa) return FALSE;
  pdat->version = SD_VERSION_2;
  return TRUE;
}

static int mmc_status(sdhci_device_t *hci) {
  sdhci_v3s_pdata_t *pdat = (sdhci_v3s_pdata_t *)hci->data;
  sdhci_cmd_t cmd = {0};
  int retries = 100;

  cmd.cmdidx = MMC_SEND_STATUS;
  cmd.resptype = MMC_RSP_R1;
  cmd.cmdarg = pdat->rca << 16;
  do {
    if (!sdhci_v3s_transfer(hci, &cmd, NULL)) continue;
    if (cmd.response[0] & (1 << 8)) break;
    // udelay(1);
  } while (retries-- > 0);
  if (retries > 0) return ((cmd.response[0] >> 9) & 0xf);
  return -1;
}

static u32 mmc_read_blocks(sdhci_device_t *hci, u8 *buf, u32 start,
                           u32 blkcnt) {
  // kprintf("mmc_read_blocks buf:%x start:%x blkcnt:%d\n", buf, start, blkcnt);
  sdhci_v3s_pdata_t *pdat = (sdhci_v3s_pdata_t *)hci->data;
  sdhci_cmd_t cmd = {0};
  sdhci_data_t dat = {0};
  int status;

  if (blkcnt > 1)
    cmd.cmdidx = MMC_READ_MULTIPLE_BLOCK;
  else
    cmd.cmdidx = MMC_READ_SINGLE_BLOCK;
  if (pdat->high_capacity) {
    cmd.cmdarg = start;
  } else {
    cmd.cmdarg = start * pdat->read_bl_len;
  }
  cmd.resptype = MMC_RSP_R1;
  dat.buf = buf;
  dat.flag = MMC_DATA_READ;
  dat.blksz = pdat->read_bl_len;
  dat.blkcnt = blkcnt;
  if (!sdhci_v3s_transfer(hci, &cmd, &dat)) {
    kprintf("mmc transfer0 failed %d\n", status);
    return 0;
  }
  if (!pdat->isspi) {
    do {
      status = mmc_status(hci);
      if (status < 0) {
        kprintf("mmc status faied %d\n", status);
        return 0;
      }
    } while ((status != MMC_STATUS_TRAN) && (status != MMC_STATUS_DATA));
  }
  if (blkcnt > 1) {
    cmd.cmdidx = MMC_STOP_TRANSMISSION;
    cmd.cmdarg = 0;
    cmd.resptype = MMC_RSP_R1B;
    if (!sdhci_v3s_transfer(hci, &cmd, NULL)) {
      kprintf("mmc transfer failed %d\n", status);
      return 0;
    }
  }
  return blkcnt * dat.blksz;
}

static u32 mmc_write_blocks(sdhci_device_t *hci, u8 *buf, u32 start,
                            u32 blkcnt) {
  sdhci_v3s_pdata_t *pdat = (sdhci_v3s_pdata_t *)hci->data;
  sdhci_cmd_t cmd = {0};
  sdhci_data_t dat = {0};
  int status;

  if (blkcnt > 1)
    cmd.cmdidx = MMC_WRITE_MULTIPLE_BLOCK;
  else
    cmd.cmdidx = MMC_WRITE_SINGLE_BLOCK;
  if (pdat->high_capacity)
    cmd.cmdarg = start;
  else
    cmd.cmdarg = start * pdat->write_bl_len;
  cmd.resptype = MMC_RSP_R1;
  dat.buf = buf;
  dat.flag = MMC_DATA_WRITE;
  dat.blksz = pdat->write_bl_len;
  dat.blkcnt = blkcnt;
  if (!sdhci_v3s_transfer(hci, &cmd, &dat)) return 0;
  if (!pdat->isspi) {
    do {
      status = mmc_status(hci);
      if (status < 0) return 0;
    } while ((status != MMC_STATUS_TRAN) && (status != MMC_STATUS_RCV));
  }
  if (blkcnt > 1) {
    cmd.cmdidx = MMC_STOP_TRANSMISSION;
    cmd.cmdarg = 0;
    cmd.resptype = MMC_RSP_R1B;
    if (!sdhci_v3s_transfer(hci, &cmd, NULL)) return 0;
  }
  return blkcnt * dat.blksz;
}

static void print_hex(u8 *addr, u32 size) {
  for (int x = 0; x < size; x++) {
    kprintf("%x ", addr[x]);
    if (x != 0 && (x % 32) == 0) {
      kprintf("\n");
    }
  }
  kprintf("\n\r");
}

int sdhci_dev_port_read(sdhci_device_t *sdhci_dev, int no, sector_t sector,
                        u32 count, u32 buf) {
  // kprintf("sdhci_dev_port_read sector:%d count:%d buf:%x\n", sector.startl,
  //         count, buf);
  size_t buf_size = count * BYTE_PER_SECTOR;
  u32 ret = 0;

#ifdef CACHE_ENABLED
  if (count == 1) {
    int index = sector.startl & CACHE_MASK;
    void *cache_p = (void *)(sdhci_dev->cache_buffer + SECTOR_SIZE * index);
    if (sdhci_dev->cached_blocks[index] != sector.startl) {
      ret = mmc_read_blocks(sdhci_dev, cache_p, sector.startl, count);
      sdhci_dev->cached_blocks[index] = sector.startl;
    }
    kmemcpy(buf, cache_p, SECTOR_SIZE);
  } else {
#endif
    ret = mmc_read_blocks(sdhci_dev, buf, sector.startl, count);
    if (ret < buf_size) {
      kprintf("mm read failed ret:%d\n", ret);
      return -1;
    }
#ifdef CACHE_ENABLED
  }
#endif
  // kprintf("sd read offset:%x %x count:%d\n", sector.startl * BYTE_PER_SECTOR,
  //         sector.starth * BYTE_PER_SECTOR, count);
  // print_hex(buf, buf_size);
  // kprintf("ret %d\n", ret);
  return ret;
}

int sdhci_dev_port_write(sdhci_device_t *sdhci_dev, int no, sector_t sector,
                         u32 count, u32 buf) {
  u32 ret = 0;
  size_t buf_size = count * BYTE_PER_SECTOR;
  ret = mmc_write_blocks(sdhci_dev, buf, sector.startl, count);
  if (ret < buf_size) {
    return -1;
  }
#ifdef CACHE_ENABLED
  int i;
  u8 *p = buf;
  for (i = 0; i < count; i++) {
    int index = (sector.startl + i) & CACHE_MASK;
    void *cache_p = (void *)(sdhci_dev->cache_buffer + SECTOR_SIZE * index);

    kmemcpy(cache_p, &p[SECTOR_SIZE * i], SECTOR_SIZE);
  }
#endif
  return ret;
}

int sdhci_v3s_probe(sdhci_device_t *hci) {
  sdhci_v3s_pdata_t *pdat = (sdhci_v3s_pdata_t *)hci->data;
  sdhci_cmd_t cmd = {0};
  sdhci_data_t dat = {0};
  char scap[32];
  u64 csize, cmult;
  u32 unit, time;
  int width;
  int status;

  sdhci_v3s_reset(hci);
  sdhci_v3s_setclock(hci, 400 * 1000);
  sdhci_v3s_setwidth(hci, MMC_BUS_WIDTH_1);
  // sdhci_v3s_setclock(hci, pdat->clock);
  // sdhci_v3s_setwidth(hci, pdat->width);

  if (!go_idle_state(hci)) {
    kprintf("prob go idle false\n");
    return FALSE;
  }

  sd_send_if_cond(hci);
  if (!sd_send_op_cond(hci)) {
    if (!mmc_send_op_cond(hci)) {
      kprintf("prob 2 false\n");
      return FALSE;
    }
  }

  if (pdat->isspi) {
    cmd.cmdidx = MMC_SEND_CID;
    cmd.cmdarg = 0;
    cmd.resptype = MMC_RSP_R1;
    if (!sdhci_v3s_transfer(hci, &cmd, NULL)) {
      kprintf("prob 3 false\n");
      return FALSE;
    }
    pdat->cid[0] = cmd.response[0];
    pdat->cid[1] = cmd.response[1];
    pdat->cid[2] = cmd.response[2];
    pdat->cid[3] = cmd.response[3];

    cmd.cmdidx = MMC_SEND_CSD;
    cmd.cmdarg = 0;
    cmd.resptype = MMC_RSP_R1;
    if (!sdhci_v3s_transfer(hci, &cmd, NULL)) {
      kprintf("prob 4 false\n");
      return FALSE;
    }
    pdat->csd[0] = cmd.response[0];
    pdat->csd[1] = cmd.response[1];
    pdat->csd[2] = cmd.response[2];
    pdat->csd[3] = cmd.response[3];
  } else {
    cmd.cmdidx = MMC_ALL_SEND_CID;
    cmd.cmdarg = 0;
    cmd.resptype = MMC_RSP_R2;
    if (!sdhci_v3s_transfer(hci, &cmd, NULL)) {
      kprintf("prob 5 false\n");
      return FALSE;
    }
    pdat->cid[0] = cmd.response[0];
    pdat->cid[1] = cmd.response[1];
    pdat->cid[2] = cmd.response[2];
    pdat->cid[3] = cmd.response[3];

    cmd.cmdidx = SD_CMD_SEND_RELATIVE_ADDR;
    cmd.cmdarg = pdat->rca << 16;
    cmd.resptype = MMC_RSP_R6;
    if (!sdhci_v3s_transfer(hci, &cmd, NULL)) {
      kprintf("prob 6 false\n");
      return FALSE;
    }
    if (pdat->version & SD_VERSION_SD)
      pdat->rca = (cmd.response[0] >> 16) & 0xffff;

    cmd.cmdidx = MMC_SEND_CSD;
    cmd.cmdarg = pdat->rca << 16;
    cmd.resptype = MMC_RSP_R2;
    if (!sdhci_v3s_transfer(hci, &cmd, NULL)) {
      kprintf("prob 7 false\n");
      return FALSE;
    }
    pdat->csd[0] = cmd.response[0];
    pdat->csd[1] = cmd.response[1];
    pdat->csd[2] = cmd.response[2];
    pdat->csd[3] = cmd.response[3];

    cmd.cmdidx = MMC_SELECT_CARD;
    cmd.cmdarg = pdat->rca << 16;
    cmd.resptype = MMC_RSP_R1;
    if (!sdhci_v3s_transfer(hci, &cmd, NULL)) {
      kprintf("prob 8 false\n");
      return FALSE;
    }
    do {
      status = mmc_status(hci);
      if (status < 0) {
        kprintf("prob 9 false\n");
        return FALSE;
      }
    } while (status != MMC_STATUS_TRAN);
  }

  if (pdat->version == MMC_VERSION_UNKNOWN) {
    switch ((pdat->csd[0] >> 26) & 0xf) {
      case 0:
        pdat->version = MMC_VERSION_1_2;
        break;
      case 1:
        pdat->version = MMC_VERSION_1_4;
        break;
      case 2:
        pdat->version = MMC_VERSION_2_2;
        break;
      case 3:
        pdat->version = MMC_VERSION_3;
        break;
      case 4:
        pdat->version = MMC_VERSION_4;
        break;
      default:
        pdat->version = MMC_VERSION_1_2;
        break;
    };
  }

  unit = tran_speed_unit[(pdat->csd[0] & 0x7)];
  time = tran_speed_time[((pdat->csd[0] >> 3) & 0xf)];
  pdat->tran_speed = time * unit;
  pdat->dsr_imp = UNSTUFF_BITS(pdat->csd, 76, 1);

  pdat->read_bl_len = 1 << UNSTUFF_BITS(pdat->csd, 80, 4);
  if (pdat->version & SD_VERSION_SD)
    pdat->write_bl_len = pdat->read_bl_len;
  else
    pdat->write_bl_len = 1 << ((pdat->csd[3] >> 22) & 0xf);
  if (pdat->read_bl_len > 512) pdat->read_bl_len = 512;
  if (pdat->write_bl_len > 512) pdat->write_bl_len = 512;

  if ((pdat->version & MMC_VERSION_MMC) && (pdat->version >= MMC_VERSION_4)) {
    cmd.cmdidx = MMC_SEND_EXT_CSD;
    cmd.cmdarg = 0;
    cmd.resptype = MMC_RSP_R1;
    dat.buf = pdat->extcsd;
    dat.flag = MMC_DATA_READ;
    dat.blksz = 512;
    dat.blkcnt = 1;
    if (!sdhci_v3s_transfer(hci, &cmd, &dat)) {
      kprintf("prob 10 false\n");
      return FALSE;
    }
    if (!pdat->isspi) {
      do {
        status = mmc_status(hci);
        if (status < 0) {
          kprintf("prob 11 false\n");
          return FALSE;
        }
      } while (status != MMC_STATUS_TRAN);
    }
    switch (pdat->extcsd[192]) {
      case 1:
        pdat->version = MMC_VERSION_4_1;
        break;
      case 2:
        pdat->version = MMC_VERSION_4_2;
        break;
      case 3:
        pdat->version = MMC_VERSION_4_3;
        break;
      case 5:
        pdat->version = MMC_VERSION_4_41;
        break;
      case 6:
        pdat->version = MMC_VERSION_4_5;
        break;
      case 7:
        pdat->version = MMC_VERSION_5_0;
        break;
      case 8:
        pdat->version = MMC_VERSION_5_1;
        break;
      default:
        break;
    }
  }

  if (pdat->high_capacity) {
    if (pdat->version & SD_VERSION_SD) {
      csize = UNSTUFF_BITS(pdat->csd, 48, 22);
      pdat->capacity = (1 + csize) << 10;
    } else {
      pdat->capacity = pdat->extcsd[212] << 0 | pdat->extcsd[212 + 1] << 8 |
                       pdat->extcsd[212 + 2] << 16 |
                       pdat->extcsd[212 + 3] << 24;
    }
  } else {
    cmult = UNSTUFF_BITS(pdat->csd, 47, 3);
    csize = UNSTUFF_BITS(pdat->csd, 62, 12);
    pdat->capacity = (csize + 1) << (cmult + 2);
  }
  pdat->capacity *= 1 << UNSTUFF_BITS(pdat->csd, 80, 4);

  if (pdat->isspi) {
    sdhci_v3s_setclock(hci, min(pdat->tran_speed, pdat->clock));
    sdhci_v3s_setwidth(hci, MMC_BUS_WIDTH_1);
  } else {
    if (pdat->version & SD_VERSION_SD) {
      if ((pdat->width & MMC_BUS_WIDTH_8) || (pdat->width & MMC_BUS_WIDTH_4))
        width = 2;
      else
        width = 0;

      cmd.cmdidx = MMC_APP_CMD;
      cmd.cmdarg = pdat->rca << 16;
      cmd.resptype = MMC_RSP_R5;
      if (!sdhci_v3s_transfer(hci, &cmd, NULL)) {
        kprintf("prob 12 false\n");
        return FALSE;
      }

      cmd.cmdidx = SD_CMD_SWITCH_FUNC;
      cmd.cmdarg = width;
      cmd.resptype = MMC_RSP_R1;
      if (!sdhci_v3s_transfer(hci, &cmd, NULL)) {
        kprintf("prob 13 false\n");
        return FALSE;
      }

      sdhci_v3s_setclock(hci, min(pdat->tran_speed, pdat->clock));
      if ((pdat->width & MMC_BUS_WIDTH_8) || (pdat->width & MMC_BUS_WIDTH_4))
        sdhci_v3s_setwidth(hci, MMC_BUS_WIDTH_4);
      else
        sdhci_v3s_setwidth(hci, MMC_BUS_WIDTH_1);
    } else if (pdat->version & MMC_VERSION_MMC) {
      if (pdat->width & MMC_BUS_WIDTH_8)
        width = 2;
      else if (pdat->width & MMC_BUS_WIDTH_4)
        width = 1;
      else
        width = 0;

      cmd.cmdidx = MMC_APP_CMD;
      cmd.cmdarg = pdat->rca << 16;
      cmd.resptype = MMC_RSP_R5;
      if (!sdhci_v3s_transfer(hci, &cmd, NULL)) {
        kprintf("prob 14 false\n");
        return FALSE;
      }

      cmd.cmdidx = SD_CMD_SWITCH_FUNC;
      cmd.cmdarg = width;
      cmd.resptype = MMC_RSP_R1;
      if (!sdhci_v3s_transfer(hci, &cmd, NULL)) {
        kprintf("prob 15 false\n");
        return FALSE;
      }

      sdhci_v3s_setclock(hci, min(pdat->tran_speed, pdat->clock));
      if (pdat->width & MMC_BUS_WIDTH_8)
        sdhci_v3s_setwidth(hci, MMC_BUS_WIDTH_8);
      else if (pdat->width & MMC_BUS_WIDTH_4)
        sdhci_v3s_setwidth(hci, MMC_BUS_WIDTH_4);
      else if (pdat->width & MMC_BUS_WIDTH_1)
        sdhci_v3s_setwidth(hci, MMC_BUS_WIDTH_1);
    }
  }

  cmd.cmdidx = MMC_SET_BLOCKLEN;
  cmd.cmdarg = pdat->read_bl_len;
  cmd.resptype = MMC_RSP_R1;
  if (!sdhci_v3s_transfer(hci, &cmd, NULL)) return FALSE;

  kprintf("SD/MMC card   host controller:\n");
  kprintf("  Attached is a %s card\n",
          pdat->version & SD_VERSION_SD ? "SD" : "MMC");
  kprintf("  Version: %s\n", sdcard_version_string(pdat));
  kprintf("  Capacity: %s\n", ssize(scap, pdat->capacity));
  if (pdat->high_capacity) kprintf("  High capacity card");
  kprintf("  CID: %x-%x-%x-%x\n", pdat->cid[0], pdat->cid[1],
          pdat->cid[2], pdat->cid[3]);
  kprintf("  CSD: %x-%x-%x-%x\n", pdat->csd[0], pdat->csd[1],
          pdat->csd[2], pdat->csd[3]);
  kprintf("  Max transfer speed: %d HZ\n", pdat->tran_speed);
  kprintf("  Manufacturer ID: %x\n", extract_mid(pdat));
  kprintf("  OEM/Application ID: %x\n", extract_oid(pdat));
  kprintf("  Product name: '%c%c%c%c%c'\n", pdat->cid[0] & 0xff,
          (pdat->cid[1] >> 24), (pdat->cid[1] >> 16) & 0xff,
          (pdat->cid[1] >> 8) & 0xff, pdat->cid[1] & 0xff);
  kprintf("  Product revision: %d.%d\n", extract_prv(pdat) >> 4,
          extract_prv(pdat) & 0xf);
  kprintf("  Serial no: %d\n", extract_psn(pdat));
  kprintf("  Manufacturing date: %d.%d\n", extract_year(pdat),
          extract_month(pdat));
  return TRUE;
}

void sdhci_dev_init(sdhci_device_t *sdhci_dev) {
  sdhci_v3s_pdata_t *pdat = kmalloc(sizeof(sdhci_v3s_pdata_t));
  sdhci_dev->data = pdat;
  pdat->high_capacity = 0;
  pdat->read_bl_len = BYTE_PER_SECTOR;
  pdat->rca = 0;
  pdat->isspi = 0;
  pdat->write_bl_len = BYTE_PER_SECTOR;
  pdat->virt = 0x01c0f000;

  pdat->voltage = MMC_VDD_27_36;
  pdat->clock = 25 * 1000 * 1000*2;
  pdat->width = MMC_BUS_WIDTH_4;

  pdat->reset = 8;
  pdat->clk = 162;
  pdat->clkcfg = 2;
  pdat->cmd = 2;
  pdat->cmdcfg = 2;
  pdat->dat0 = 161;
  pdat->dat0cfg = 2;
  pdat->dat1 = 160;
  pdat->dat1cfg = 2;
  pdat->dat2 = 165;
  pdat->dat2cfg = 2;
  pdat->dat3 = 164;
  pdat->dat3cfg = 2;
  pdat->dat4 = -1;
  pdat->dat4cfg = -1;
  pdat->dat5 = -1;
  pdat->dat5cfg = -1;
  pdat->dat6 = -1;
  pdat->dat6cfg = -1;
  pdat->dat7 = -1;
  pdat->dat7cfg = -1;
  pdat->cd = 166;
  pdat->cdcfg = 0;

  // sd mmc0
  map_page(0x01c0f000, 0x01c0f000, 0);

  sdhci_v3s_probe(sdhci_dev);
  kprintf("sdh dev init end\n");
}
