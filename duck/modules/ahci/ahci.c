/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "ahci.h"

// Find a free command list slot
int find_cmdslot(ahci_device_t* ahci_dev, int no) {
  hba_memory_t* hab_memory = ahci_dev->abar;
  hba_port_t* port = &hab_memory->ports[no];
  // If not set in SACT and CI, the slot is free
  u32 slots = (port->sact | port->ci);
  int cmdslots = (hab_memory->cap & 0x0f00) >> 8;  // Bit 8-12
  for (int i = 0; i < cmdslots; i++) {
    if ((slots & 1) == 0) return i;
    slots >>= 1;
  }
  kprintf("Cannot find free command list entry\n");
  return -1;
}

int ahci_dev_port_write(ahci_device_t* ahci_dev, int no, sector_t sector,
                        u32 count, u32 buf) {
  hba_memory_t* hab_memory = ahci_dev->abar;
  hba_port_t* port = &hab_memory->ports[no];

  port->is = (u32)-1;  // Clear pending interrupt bits
                       // int spin = 0;           // Spin lock timeout counter
  int slot = find_cmdslot(ahci_dev, no);
  if (slot == -1) return 0;
  uint64_t addr = 0;
  //   print("\n clb %x clbu %x", port->clb, port->clbu);
  addr = (((addr | port->clbu) << 32) | port->clb);

  hba_cmd_header_t* cmdheader = (hba_cmd_header_t*)addr;

  // hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)(port->clb);
  cmdheader += slot;
  cmdheader->cfl = sizeof(fis_reg_h2d_t) / sizeof(u32);  // Command FIS size
  cmdheader->w = 1;                                      // Read from device
  cmdheader->c = 1;                                      // Read from device
  cmdheader->p = 1;                                      // Read from device
  // 8K bytes (16 sectors) per PRDT
  cmdheader->prdtl = (u16)((count - 1) >> 4) + 1;  // PRDT entries count

  addr = (((cmdheader->ctbau) << 32) | cmdheader->ctba);
  hba_cmd_tbl_t* cmdtbl = (hba_cmd_tbl_t*)(addr);

  kmemset(cmdtbl, 0,
          sizeof(hba_cmd_tbl_t) +
              (cmdheader->prdtl - 1) * sizeof(hba_prdt_entry_t));
  int i = 0;
  //    print("[PRDTL][%d]", cmdheader->prdtl);
  // 8K bytes (16 sectors) per PRDT
  for (i = 0; i < cmdheader->prdtl - 1; i++) {
    cmdtbl->prdt_entry[i].dba = (u32)(buf & 0xFFFFFFFF);
    // cmdtbl->prdt_entry[i].dbau = (u32)((buf >> 32) & 0xFFFFFFFF);
    cmdtbl->prdt_entry[i].dbau = 0;
    cmdtbl->prdt_entry[i].dbc = 8 * 1024 - 1;  // 8K bytes
    cmdtbl->prdt_entry[i].i = 0;
    buf += 4 * 1024;  // 4K words
    count -= 16;      // 16 sectors
  }
  /**If the final Data FIS transfer in a command is for an odd number of 16-bit
words, the transmitter�s Transport layer is responsible for padding the final
u32 of a FIS with zeros. If the HBA receives one more word than is indicated
in the PRD table due to this padding requirement, the HBA shall not signal this
as an overflow condition. In addition, if the HBA inserts padding as required in
a FIS it is transmitting, an overflow error shall not be indicated. The PRD Byte
Count field shall be updated based on the number of words specified in the PRD
table, ignoring any additional padding.**/

  // Last entry

  cmdtbl->prdt_entry[i].dba = (u32)(buf & 0xFFFFFFFF);
  // cmdtbl->prdt_entry[i].dbau = (u32)((buf >> 32) & 0xFFFFFFFF);
  cmdtbl->prdt_entry[i].dbau = 0;
  cmdtbl->prdt_entry[i].dbc = (count << 9) - 1;  // 512 bytes per sector
  cmdtbl->prdt_entry[i].i = 0;

  // Setup command
  fis_reg_h2d_t* cmdfis = (fis_reg_h2d_t*)(&cmdtbl->cfis);

  cmdfis->fis_type = FIS_TYPE_REG_H2D;
  cmdfis->c = 1;  // Command
  cmdfis->command = ATA_CMD_WRITE_DMA_EX;

  cmdfis->lba0 = (u8)sector.startl;
  cmdfis->lba1 = (u8)(sector.startl >> 8);
  cmdfis->lba2 = (u8)(sector.startl >> 16);
  cmdfis->device = 1 << 6;  // LBA mode

  cmdfis->lba3 = (u8)(sector.startl >> 24);
  cmdfis->lba4 = (u8)sector.starth;
  cmdfis->lba5 = (u8)(sector.starth >> 8);

  cmdfis->countl = count & 0xff;
  cmdfis->counth = (count >> 8) & 0xff;

  //    print("[slot]{%d}", slot);
  port->ci = 1;  // Issue command
                 //    print("\n[Port ci ][%d]", port->ci);
                 //     print("\nafter issue : %d" , port->tfd);
                 //    print("\nafter issue : %d" , port->tfd);

  //    print("\nbefore while 1--> %d", slot);
  // Wait for completion
  while (1) {
    // In some longer duration reads, it may be helpful to spin on the DPS bit
    // in the PxIS port field as well (1 << 5)
    if ((port->ci & (1 << slot)) == 0) break;
    if (port->is & HBA_PxIS_TFES)  // Task file error
    {
      kprintf("write disk error startl:%x\n", sector.startl);
      return 0;
    }
  }
  //   print("\n after while 1");
  //   print("\nafter issue : %d" , port->tfd);
  // Check again
  if (port->is & HBA_PxIS_TFES) {
    kprintf("write disk error startl:%x\n", sector.startl);
    return 0;
  }

  //   print("\n[Port ci ][%d]", port->ci);
  // int k = 0;
  while (port->ci != 0) {
    //     print("[%d]", k++);
  }
  return 1;
}

int ahci_dev_port_read(ahci_device_t* ahci_dev, int no, sector_t sector,
                       u32 count, u32 buf) {
  //转成hba结构体描述，别问hba是啥，其实我也不知道（host bus adapter）是一个ahci的规范结构，用来内存和ata设备通信。
  hba_memory_t* hab_memory = ahci_dev->abar;
  //根据设备port号，获取对应的port，支持多port
  hba_port_t* port = &hab_memory->ports[no];
  port->is = (u32)-1;  // 清理中断 Clear pending interrupt bits
  int spin = 0;        // Spin lock timeout counter
  int slot = find_cmdslot(ahci_dev, no); //查找可以用的slot
  if (slot == -1){
    kprintf("not slot found\n");
    return 0;
  }
  uint64_t addr = 0;
  //   print("\n clb %x clbu %x", port->clb, port->clbu);
  //获取命令列表基地址（高位和低位）
  addr = (((port->clbu) << 32) | port->clb);

  //转命令指针
  hba_cmd_header_t* cmdheader = (hba_cmd_header_t*)addr;
  cmdheader += slot;//定位到空闲的命令头
  cmdheader->cfl = sizeof(fis_reg_h2d_t) / sizeof(u32);  // Command FIS size
  cmdheader->w = 0;                                      // Read from device
  cmdheader->prdtl = (uint16_t)((count - 1) >> 4) + 1;   // PRDT entries count

  //获取命令行
  addr = (((cmdheader->ctbau) << 32) | cmdheader->ctba);
  hba_cmd_tbl_t* cmdtbl = (hba_cmd_tbl_t*)(addr);
  kmemset(cmdtbl, 0,
          sizeof(hba_cmd_tbl_t) +
              (cmdheader->prdtl - 1) * sizeof(hba_prdt_entry_t));

  // 8K bytes (16 sectors) per PRDT
  int i;
  for (i = 0; i < cmdheader->prdtl  - 1; i++) {
    cmdtbl->prdt_entry[i].dba = (u32)(buf & 0xFFFFFFFF);
    // cmdtbl->prdt_entry[i].dbau = (u32)(((buf) >> 32) & 0xFFFFFFFF);
    cmdtbl->prdt_entry[i].dbau = 0;
    cmdtbl->prdt_entry[i].dbc = 8 * 1024 - 1;  // 8K bytes (this value should always be set to 1 less
                       // than the actual value)
    cmdtbl->prdt_entry[i].i = 1;
    buf += 4 * 1024;  // 4K words
    count -= 16;      // 16 sectors
  }
  // Last entry
  cmdtbl->prdt_entry[i].dba = (u32)(buf & 0xFFFFFFFF);
  cmdtbl->prdt_entry[i].dbau = 0;
  // cmdtbl->prdt_entry[i].dbau = (u32)(((buf) >> 32) & 0xFFFFFFFF);
  cmdtbl->prdt_entry[i].dbc = count << 9;  // 512 bytes per sector
  cmdtbl->prdt_entry[i].i = 1;

  // Setup command
  fis_reg_h2d_t* cmdfis = (fis_reg_h2d_t*)(&cmdtbl->cfis);

  cmdfis->fis_type = FIS_TYPE_REG_H2D;
  cmdfis->c = 1;  // Command
  cmdfis->command = ATA_CMD_READ_DMA_EX;

  cmdfis->lba0 = (uint8_t)(sector.startl & 0xFF);
  cmdfis->lba1 = (uint8_t)((sector.startl >> 8) & 0xFF);
  cmdfis->lba2 = (uint8_t)((sector.startl >> 16) & 0xFF);
  cmdfis->device = 1 << 6;  // LBA mode

  cmdfis->lba3 = (uint8_t)((sector.startl >> 24) & 0xFF);
  cmdfis->lba4 = (uint8_t)(sector.starth & 0xFF);
  cmdfis->lba5 = (uint8_t)((sector.starth >> 8) & 0xFF);

  cmdfis->countl = count & 0xFF;
  cmdfis->counth = (count >> 8) & 0xFF;

  // The below loop waits until the port is no longer busy before issuing a new
  // command
  while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) {
    spin++;
  }
  if (spin == 1000000) {
    kprintf("Port is hung\n");
    return 0;
  }

  port->ci = 1 << slot;  // Issue command

  // Wait for completion
  while (1) {
    // In some longer duration reads, it may be helpful to spin on the DPS bit
    // in the PxIS port field as well (1 << 5)
    if ((port->ci & (1 << slot)) == 0) break;
    if (port->is & HBA_PxIS_TFES)  // Task file error
    {
      kprintf("read disk error startl:%x\n", sector.startl);
      return 0;
    }
  }

  // Check again
  if (port->is & HBA_PxIS_TFES) {
    kprintf("read disk error startl:%x\n", sector.startl);
    return 0;
  }

  return 1;
}

// Start command engine
void start_cmd(hba_port_t* port) {
  // Wait until CR (bit15) is cleared
  while (port->cmd & HBA_PxCMD_CR)
    ;

  // Set FRE (bit4) and ST (bit0)
  port->cmd |= HBA_PxCMD_FRE;
  port->cmd |= HBA_PxCMD_ST;
}

// Stop command engine
void stop_cmd(hba_port_t* port) {
  // Clear ST (bit0)
  port->cmd &= ~HBA_PxCMD_ST;

  // Clear FRE (bit4)
  port->cmd &= ~HBA_PxCMD_FRE;

  // Wait until FR (bit14), CR (bit15) are cleared
  while (1) {
    if (port->cmd & HBA_PxCMD_FR) continue;
    if (port->cmd & HBA_PxCMD_CR) continue;
    break;
  }
}

void ahci_dev_port_init(ahci_device_t* ahci_dev, int no) {
  //又来一遍hba memory，来多了其实不好。
  hba_memory_t* hab_memory = ahci_dev->abar;
  hba_port_t* port = &hab_memory->ports[no];

  stop_cmd(port);  //停止运行 Stop command engine

  // Command list offset: 1K*portno
  // Command list entry size = 32
  // Command list entry maxim count = 32
  // Command list maxim size = 32*32 = 1K per port
  // void* ahci_base = kmalloc(40 * 1024 + 8 * 1024 * 32);
  //申请命令缓冲
  void* base_cmd = kmalloc_alignment(40 * 1024 + 8 * 1024 * 32, 1024);
  ahci_dev->base_cmd = base_cmd;
  port->clb = base_cmd + (no << 10);
  port->clbu = 0;
  kmemset((void*)(port->clb), 0, 1024);

  // FIS offset: 32K+256*portno
  // FIS entry size = 256 bytes per port
  port->fb = base_cmd + (32 << 10) + (no << 8);
  port->fbu = 0;
  kmemset((void*)(port->fb), 0, 256);

  // Command table offset: 40K + 8K*portno
  // Command table size = 256*32 = 8K per port
  hba_cmd_header_t* cmdheader = (hba_cmd_header_t*)(port->clb);
  for (int i = 0; i < 32; i++) {
    cmdheader[i].prdtl = 8;  // 8 prdt entries per command table
                             // 256 bytes per command table, 64+16+48+16*8
    // Command table offset: 40K + 8K*portno + cmdheader_index*256
    cmdheader[i].ctba = base_cmd + (40 << 10) + (no << 13) + (i << 8);
    cmdheader[i].ctbau = 0;
    kmemset((void*)cmdheader[i].ctba, 0, 256);
  }

  start_cmd(port);  // Start command engine
}

i32 ahci_get_port_type(hba_port_t* port) {
  u32 ssts = port->ssts;

  u8 ipm = (ssts >> 8) & 0x0F;
  u8 det = ssts & 0x0F;
  if (det != HBA_PORT_DET_PRESENT)  // Check drive status
    return AHCI_DEV_NULL;
  if (ipm != HBA_PORT_IPM_ACTIVE) return AHCI_DEV_NULL;

  switch (port->sig) {
    case SATA_SIG_ATAPI:
      return AHCI_DEV_SATAPI;
    case SATA_SIG_SEMB:
      return AHCI_DEV_SEMB;
    case SATA_SIG_PM:
      return AHCI_DEV_PM;
    default:
      return AHCI_DEV_SATA;
  }
}

void ahci_dev_prob(ahci_device_t* ahci_dev) {
  //你好，我是hba memory，就是前面的bar5 变过来的。
  hba_memory_t* hab_memory = ahci_dev->abar;
  u32 addr = hab_memory;
  //映射到内核中，地址一一映射，当然你可以自己映射到固定位置，这里怎么简单怎么来（偷懒）^_^!!
  map_page(addr, addr, PAGE_P | PAGE_USU | PAGE_RWW);
  u32 pi = hab_memory->pi;//看看port有几个一个bit代表一个，32bit长，需要循环32次看看。
  for (int i = 0; i < 32; i++, pi >>= 1) {
    if (!(pi & 1)) {//当前位置没有被设置成1，说明没有设备，继续。
      continue;
    }
    //让我康康，是什么设备，
    i32 type = ahci_get_port_type(&hab_memory->ports[i]);
    //原来是SATA设备
    if (type == AHCI_DEV_SATA) {
      //真的发现了ata设备，先初始化了再说
      ahci_dev_port_init(ahci_dev, i);
    } else if (type == AHCI_DEV_NULL) {
    } else {
      kprintf("type not support %d\n", type);
    }
  }
}

static size_t ahci_read(device_t* dev, void* buf, size_t len) {
  ahci_device_t* ahci_dev = dev->data;
  int no = dev->id - DEVICE_SATA;
  u32 startl = ahci_dev->offsetl / BYTE_PER_SECTOR;
  u32 starth = ahci_dev->offseth / BYTE_PER_SECTOR;
  u32 count = len / BYTE_PER_SECTOR;
  u32 rest = len % BYTE_PER_SECTOR;
  if (rest > 0) {
    kprintf("ahci read is more, may have error\n");
    count++;
  }
  u32 ret = 0;
  sector_t sector;
  sector.startl = startl;
  sector.starth = starth;
  ret = ahci_dev_port_read(ahci_dev, no, sector, count, buf);
  if (ret == 0) {
    return -1;
  }
  return count * BYTE_PER_SECTOR;
}

static size_t ahci_write(device_t* dev, void* buf, size_t len) {
  ahci_device_t* ahci_dev = dev->data;
  int no = dev->id - DEVICE_SATA;
  u32 startl = ahci_dev->offsetl / BYTE_PER_SECTOR;
  u32 starth = ahci_dev->offseth / BYTE_PER_SECTOR;
  u32 count = len / BYTE_PER_SECTOR;
  u32 rest = len % BYTE_PER_SECTOR;
  if (rest > 0) {
    kprintf("ahci write is more, may have error\n");
    count++;
  }
  u32 ret = 0;
  sector_t sector;
  sector.startl = startl;
  sector.starth = starth;
  ret = ahci_dev_port_write(ahci_dev, no, sector, count, buf);
  if (ret == 0) {
    return -1;
  }
  return count * BYTE_PER_SECTOR;
}

static size_t ahci_ioctl(device_t* dev, u32 cmd, ...) {
  u32 ret = 0;
  ahci_device_t* ahci_dev = dev->data;
  int no = dev->id - DEVICE_SATA;
  if (ahci_dev == NULL) {
    kprintf("not found vga\n");
    return ret;
  }
  va_list ap;
  va_start(ap, cmd);
  u32 offset = va_arg(ap, u32);
  if (cmd == IOC_READ_OFFSET) {
    ret = ahci_dev->offsetl | ahci_dev->offseth << 32;
  } else if (cmd == IOC_WRITE_OFFSET) {
    ahci_dev->offsetl = offset;
    // ahci_dev->offseth=(u32*)va_arg(ap,u32);
  }
  va_end(ap);

  return ret;
}

void ahci_dev_init(ahci_device_t* ahci_dev) { 
  //探测并，初始化ahci设备，可能有多个，有多个就初始化多个
  ahci_dev_prob(ahci_dev); 
}

int ahci_init(void) {
  //根据 pci 的ahci class 为0x0106，分类为01是大容量设备，0x6 是串行ata控制 Serial ATA Controller，所以为0x0106
  pci_device_t* pdev = pci_find_class(0x0106);
  if (pdev == NULL) {
    kprintf("can not find ahci device\n");
    return 0;
  }
  //获取bar5的地址，别问为什么，规范说bar5是pci为0x24的AHCI Base Address <BAR5>，获取后可以通过内存操作ata设备
  u32 bar5 = pci_dev_read32(pdev, PCI_BASE_ADDR5) & 0xFFFFFFF0;

  //分配一个设备玩一玩，骗你的呢，怎么会有人乱来，分配好后就添加到设备列表去。
  device_t* dev = kmalloc(sizeof(device_t));
  dev->name = "sata";
  dev->read = ahci_read;
  dev->write = ahci_write;
  dev->ioctl = ahci_ioctl;
  dev->id = DEVICE_SATA;
  dev->type = DEVICE_TYPE_BLOCK;
  device_add(dev); //添加到设备列表去

  //初始化一下ahci私有设备结构体，图啥呢，玩呢，别问为什么，往下面看就知道了。
  ahci_device_t* ahci_dev = kmalloc(sizeof(ahci_device_t));
  ahci_dev->abar = bar5;//把ahci 传进去，用于后面转hba memory操作ata设备。
  dev->data = ahci_dev;//设置私有设备地址
  //初始化一下ahci设备
  ahci_dev_init(ahci_dev);

  return 0;
}

void ahci_exit(void) { kprintf("ahci exit\n"); }

module_t ahci_module = {.name = "ahci", .init = ahci_init, .exit = ahci_exit};
