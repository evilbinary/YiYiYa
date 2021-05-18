/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef PCI_H
#define PCI_H

#include "kernel/kernel.h"

#define MAX_PCI_NUMBER 240

//内存地址空间、IO地址空间和配置空间

#define PCI_CONFIG_ADDRESS  0xCF8 //PCI配置空间地址端口
#define PCI_CONFIG_DATA 0xCFC         //PCI配置空间数据端口

#define PCI_BASE_ADDR 0x80000000L

#define PCI_DEVICE_ID	0x00
#define PCI_STATUS	0x04
#define PCI_CLASS_CODE	0x09


#define PCI_BASE_ADDR0	0x10
#define PCI_BASE_ADDR1	0x14
#define PCI_BASE_ADDR2	0x18
#define PCI_BASE_ADDR3	0x1c
#define PCI_BASE_ADDR4	0x20
#define PCI_BASE_ADDR5	0x24

typedef struct pci_device{
    u32 vendor;
	u32 device;
    u32 class;
    u32 bus;
    u32 slot;
    u32 function;
}pci_device_t;

pci_device_t* pci_find_class(u32 class);
pci_device_t* pci_find_vendor_device(u32 vendor,u32 device);

u32 pci_read32(u16 bus, u16 slot, u16 func, u16 offset);
u32 pci_dev_read32(pci_device_t* pdev,u16 offset);
u16 pci_read16(u16 bus, u16 slot, u16 func, u16 offset);

#endif