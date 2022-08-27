#ifndef V3S_SDHC_H
#define V3S_SDHC_H

#include "sdhci.h"

enum {
  /* Class 1 */
  MMC_GO_IDLE_STATE = 0,
  MMC_SEND_OP_COND = 1,
  MMC_ALL_SEND_CID = 2,
  MMC_SET_RELATIVE_ADDR = 3,
  MMC_SET_DSR = 4,
  MMC_SWITCH = 6,
  MMC_SELECT_CARD = 7,
  MMC_SEND_EXT_CSD = 8,
  MMC_SEND_CSD = 9,
  MMC_SEND_CID = 10,
  MMC_READ_DAT_UNTIL_STOP = 11,
  MMC_STOP_TRANSMISSION = 12,
  MMC_SEND_STATUS = 13,
  MMC_GO_INACTIVE_STATE = 15,
  MMC_SPI_READ_OCR = 58,
  MMC_SPI_CRC_ON_OFF = 59,

  /* Class 2 */
  MMC_SET_BLOCKLEN = 16,
  MMC_READ_SINGLE_BLOCK = 17,
  MMC_READ_MULTIPLE_BLOCK = 18,

  /* Class 3 */
  MMC_WRITE_DAT_UNTIL_STOP = 20,

  /* Class 4 */
  MMC_SET_BLOCK_COUNT = 23,
  MMC_WRITE_SINGLE_BLOCK = 24,
  MMC_WRITE_MULTIPLE_BLOCK = 25,
  MMC_PROGRAM_CID = 26,
  MMC_PROGRAM_CSD = 27,

  /* Class 5 */
  MMC_ERASE_GROUP_START = 35,
  MMC_ERASE_GROUP_END = 36,
  MMC_ERASE = 38,

  /* Class 6 */
  MMC_SET_WRITE_PROT = 28,
  MMC_CLR_WRITE_PROT = 29,
  MMC_SEND_WRITE_PROT = 30,

  /* Class 7 */
  MMC_LOCK_UNLOCK = 42,

  /* Class 8 */
  MMC_APP_CMD = 55,
  MMC_GEN_CMD = 56,

  /* Class 9 */
  MMC_FAST_IO = 39,
  MMC_GO_IRQ_STATE = 40,
};

enum {
  SD_CMD_SEND_RELATIVE_ADDR = 3,
  SD_CMD_SWITCH_FUNC = 6,
  SD_CMD_SEND_IF_COND = 8,
  SD_CMD_APP_SET_BUS_WIDTH = 6,
  SD_CMD_ERASE_WR_BLK_START = 32,
  SD_CMD_ERASE_WR_BLK_END = 33,
  SD_CMD_APP_SEND_OP_COND = 41,
  SD_CMD_APP_SEND_SCR = 51,
};

enum {
  MMC_RSP_PRESENT = (1 << 0),
  MMC_RSP_136 = (1 << 1),
  MMC_RSP_CRC = (1 << 2),
  MMC_RSP_BUSY = (1 << 3),
  MMC_RSP_OPCODE = (1 << 4),
};

enum {
  MMC_RSP_NONE = (0 << 24),
  MMC_RSP_R1 = (1 << 24) | (MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE),
  MMC_RSP_R1B = (1 << 24) |
                (MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE | MMC_RSP_BUSY),
  MMC_RSP_R2 = (2 << 24) | (MMC_RSP_PRESENT | MMC_RSP_136 | MMC_RSP_CRC),
  MMC_RSP_R3 = (3 << 24) | (MMC_RSP_PRESENT),
  MMC_RSP_R4 = (4 << 24) | (MMC_RSP_PRESENT),
  MMC_RSP_R5 = (5 << 24) | (MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE),
  MMC_RSP_R6 = (6 << 24) | (MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE),
  MMC_RSP_R7 = (7 << 24) | (MMC_RSP_PRESENT | MMC_RSP_CRC | MMC_RSP_OPCODE),
};

enum {
  MMC_STATUS_IDLE = 0,
  MMC_STATUS_READY = 1,
  MMC_STATUS_IDENT = 2,
  MMC_STATUS_STBY = 3,
  MMC_STATUS_TRAN = 4,
  MMC_STATUS_DATA = 5,
  MMC_STATUS_RCV = 6,
  MMC_STATUS_PRG = 7,
  MMC_STATUS_DIS = 8,
  MMC_STATUS_BTST = 9,
  MMC_STATUS_SLP = 10,
};

enum {
  OCR_BUSY = 0x80000000,
  OCR_HCS = 0x40000000,
  OCR_VOLTAGE_MASK = 0x00ffff80,
  OCR_ACCESS_MODE = 0x60000000,
};

enum {
  MMC_DATA_READ = (1 << 0),
  MMC_DATA_WRITE = (1 << 1),
};

enum {
  MMC_VDD_27_36 = (1 << 0),
  MMC_VDD_165_195 = (1 << 1),
};

enum {
  MMC_BUS_WIDTH_1 = (1 << 0),
  MMC_BUS_WIDTH_4 = (1 << 2),
  MMC_BUS_WIDTH_8 = (1 << 3),
};

enum {
  SD_VERSION_SD = 0x20000,
  SD_VERSION_3 = (SD_VERSION_SD | 0x300),
  SD_VERSION_2 = (SD_VERSION_SD | 0x200),
  SD_VERSION_1_0 = (SD_VERSION_SD | 0x100),
  SD_VERSION_1_10 = (SD_VERSION_SD | 0x10a),
  MMC_VERSION_MMC = 0x10000,
  MMC_VERSION_UNKNOWN = (MMC_VERSION_MMC),
  MMC_VERSION_1_2 = (MMC_VERSION_MMC | 0x102),
  MMC_VERSION_1_4 = (MMC_VERSION_MMC | 0x104),
  MMC_VERSION_2_2 = (MMC_VERSION_MMC | 0x202),
  MMC_VERSION_3 = (MMC_VERSION_MMC | 0x300),
  MMC_VERSION_4 = (MMC_VERSION_MMC | 0x400),
  MMC_VERSION_4_1 = (MMC_VERSION_MMC | 0x401),
  MMC_VERSION_4_2 = (MMC_VERSION_MMC | 0x402),
  MMC_VERSION_4_3 = (MMC_VERSION_MMC | 0x403),
  MMC_VERSION_4_41 = (MMC_VERSION_MMC | 0x429),
  MMC_VERSION_4_5 = (MMC_VERSION_MMC | 0x405),
  MMC_VERSION_5_0 = (MMC_VERSION_MMC | 0x500),
  MMC_VERSION_5_1 = (MMC_VERSION_MMC | 0x501),
};

typedef struct sdhci_cmd {
  u32 cmdidx;
  u32 cmdarg;
  u32 resptype;
  u32 response[4];
} sdhci_cmd_t;

typedef struct sdhci_data {
  u8 *buf;
  u32 flag;
  u32 blksz;
  u32 blkcnt;
} sdhci_data_t;

enum {
  SD_GCTL = 0x00,
  SD_CKCR = 0x04,
  SD_TMOR = 0x08,
  SD_BWDR = 0x0c,
  SD_BKSR = 0x10,
  SD_BYCR = 0x14,
  SD_CMDR = 0x18,
  SD_CAGR = 0x1c,
  SD_RESP0 = 0x20,
  SD_RESP1 = 0x24,
  SD_RESP2 = 0x28,
  SD_RESP3 = 0x2c,
  SD_IMKR = 0x30,
  SD_MISR = 0x34,
  SD_RISR = 0x38,
  SD_STAR = 0x3c,
  SD_FWLR = 0x40,
  SD_FUNS = 0x44,
  SD_A12A = 0x58,
  SD_NTSR = 0x5c,
  SD_SDBG = 0x60,
  SD_HWRST = 0x78,
  SD_DMAC = 0x80,
  SD_DLBA = 0x84,
  SD_IDST = 0x88,
  SD_IDIE = 0x8c,
  SD_THLDC = 0x100,
  SD_DSBD = 0x10c,
  SD_RES_CRC = 0x110,
  SD_DATA7_CRC = 0x114,
  SD_DATA6_CRC = 0x118,
  SD_DATA5_CRC = 0x11c,
  SD_DATA4_CRC = 0x120,
  SD_DATA3_CRC = 0x124,
  SD_DATA2_CRC = 0x128,
  SD_DATA1_CRC = 0x12c,
  SD_DATA0_CRC = 0x130,
  SD_CRC_STA = 0x134,
  SD_FIFO = 0x200,
};

/*
 * Global control register bits
 */
#define SDXC_SOFT_RESET (1 << 0)
#define SDXC_FIFO_RESET (1 << 1)
#define SDXC_DMA_RESET (1 << 2)
#define SDXC_INTERRUPT_ENABLE_BIT (1 << 4)
#define SDXC_DMA_ENABLE_BIT (1 << 5)
#define SDXC_DEBOUNCE_ENABLE_BIT (1 << 8)
#define SDXC_POSEDGE_LATCH_DATA (1 << 9)
#define SDXC_DDR_MODE (1 << 10)
#define SDXC_MEMORY_ACCESS_DONE (1 << 29)
#define SDXC_ACCESS_DONE_DIRECT (1 << 30)
#define SDXC_ACCESS_BY_AHB (1 << 31)
#define SDXC_ACCESS_BY_DMA (0 << 31)
#define SDXC_HARDWARE_RESET (SDXC_SOFT_RESET | SDXC_FIFO_RESET | SDXC_DMA_RESET)

/*
 * Clock control bits
 */
#define SDXC_CARD_CLOCK_ON (1 << 16)
#define SDXC_LOW_POWER_ON (1 << 17)

/*
 * Bus width
 */
#define SDXC_WIDTH1 (0)
#define SDXC_WIDTH4 (1)
#define SDXC_WIDTH8 (2)

/*
 * Smc command bits
 */
#define SDXC_RESP_EXPIRE (1 << 6)
#define SDXC_LONG_RESPONSE (1 << 7)
#define SDXC_CHECK_RESPONSE_CRC (1 << 8)
#define SDXC_DATA_EXPIRE (1 << 9)
#define SDXC_WRITE (1 << 10)
#define SDXC_SEQUENCE_MODE (1 << 11)
#define SDXC_SEND_AUTO_STOP (1 << 12)
#define SDXC_WAIT_PRE_OVER (1 << 13)
#define SDXC_STOP_ABORT_CMD (1 << 14)
#define SDXC_SEND_INIT_SEQUENCE (1 << 15)
#define SDXC_UPCLK_ONLY (1 << 21)
#define SDXC_READ_CEATA_DEV (1 << 22)
#define SDXC_CCS_EXPIRE (1 << 23)
#define SDXC_ENABLE_BIT_BOOT (1 << 24)
#define SDXC_ALT_BOOT_OPTIONS (1 << 25)
#define SDXC_BOOT_ACK_EXPIRE (1 << 26)
#define SDXC_BOOT_ABORT (1 << 27)
#define SDXC_VOLTAGE_SWITCH (1 << 28)
#define SDXC_USE_HOLD_REGISTER (1 << 29)
#define SDXC_START (1 << 31)

/*
 * Interrupt bits
 */
#define SDXC_RESP_ERROR (1 << 1)
#define SDXC_COMMAND_DONE (1 << 2)
#define SDXC_DATA_OVER (1 << 3)
#define SDXC_TX_DATA_REQUEST (1 << 4)
#define SDXC_RX_DATA_REQUEST (1 << 5)
#define SDXC_RESP_CRC_ERROR (1 << 6)
#define SDXC_DATA_CRC_ERROR (1 << 7)
#define SDXC_RESP_TIMEOUT (1 << 8)
#define SDXC_DATA_TIMEOUT (1 << 9)
#define SDXC_VOLTAGE_CHANGE_DONE (1 << 10)
#define SDXC_FIFO_RUN_ERROR (1 << 11)
#define SDXC_HARD_WARE_LOCKED (1 << 12)
#define SDXC_START_BIT_ERROR (1 << 13)
#define SDXC_AUTO_COMMAND_DONE (1 << 14)
#define SDXC_END_BIT_ERROR (1 << 15)
#define SDXC_SDIO_INTERRUPT (1 << 16)
#define SDXC_CARD_INSERT (1 << 30)
#define SDXC_CARD_REMOVE (1 << 31)
#define SDXC_INTERRUPT_ERROR_BIT                                 \
  (SDXC_RESP_ERROR | SDXC_RESP_CRC_ERROR | SDXC_DATA_CRC_ERROR | \
   SDXC_RESP_TIMEOUT | SDXC_DATA_TIMEOUT | SDXC_FIFO_RUN_ERROR | \
   SDXC_HARD_WARE_LOCKED | SDXC_START_BIT_ERROR | SDXC_END_BIT_ERROR)
#define SDXC_INTERRUPT_DONE_BIT                                  \
  (SDXC_AUTO_COMMAND_DONE | SDXC_DATA_OVER | SDXC_COMMAND_DONE | \
   SDXC_VOLTAGE_CHANGE_DONE)

/*
 * Status
 */
#define SDXC_RXWL_FLAG (1 << 0)
#define SDXC_TXWL_FLAG (1 << 1)
#define SDXC_FIFO_EMPTY (1 << 2)
#define SDXC_FIFO_FULL (1 << 3)
#define SDXC_CARD_PRESENT (1 << 8)
#define SDXC_CARD_DATA_BUSY (1 << 9)
#define SDXC_DATA_FSM_BUSY (1 << 10)
#define SDXC_DMA_REQUEST (1 << 31)
#define SDXC_FIFO_SIZE (16)

/*
 * Function select
 */
#define SDXC_CEATA_ON (0xceaa << 16)
#define SDXC_SEND_IRQ_RESPONSE (1 << 0)
#define SDXC_SDIO_READ_WAIT (1 << 1)
#define SDXC_ABORT_READ_DATA (1 << 2)
#define SDXC_SEND_CCSD (1 << 8)
#define SDXC_SEND_AUTO_STOPCCSD (1 << 9)
#define SDXC_CEATA_DEV_IRQ_ENABLE (1 << 10)

typedef struct sdhci_v3s_pdata {
  u32 high_capacity;
  u32 read_bl_len;
  u32 isspi;
  u32 write_bl_len;
  u64 capacity;
  u32 tran_speed;
  u32 dsr_imp;
  u32 width;
  u32 voltage;

  u32 clock;

  u32 version;
	u32 ocr;
	u32 rca;
	u32 cid[4];
	u32 csd[4];
	u8 extcsd[512];

  u32 virt;
  
  char *pclk;
  int reset;
  int clk;
  int clkcfg;
  int cmd;
  int cmdcfg;
  int dat0;
  int dat0cfg;
  int dat1;
  int dat1cfg;
  int dat2;
  int dat2cfg;
  int dat3;
  int dat3cfg;
  int dat4;
  int dat4cfg;
  int dat5;
  int dat5cfg;
  int dat6;
  int dat6cfg;
  int dat7;
  int dat7cfg;
  int cd;
  int cdcfg;
} sdhci_v3s_pdata_t;

#define FALSE 0
#define TRUE 1
#define min(a, b)		({typeof(a) _amin = (a); typeof(b) _bmin = (b); (void)(&_amin == &_bmin); _amin < _bmin ? _amin : _bmin;})



static int sdhci_v3s_transfer(sdhci_device_t *sdhci, sdhci_cmd_t *cmd,
                              sdhci_data_t *dat);

#endif