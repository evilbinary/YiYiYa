/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#ifndef PCI_H
#define PCI_H

#include "kernel/kernel.h"

#define MAX_PCI_NUMBER 40

//内存地址空间、IO地址空间和配置空间

#define PCI_CONFIG_ADDRESS  0xCF8 //PCI配置空间地址端口
#define PCI_CONFIG_DATA 0xCFC         //PCI配置空间数据端口

#define PCI_BASE_ADDR 0x80000000L


typedef struct pci_device{
    u32 vendor;
	u32 device;
    u32 class;
    u32 bus;
    u32 slot;
    u32 function;
}pci_device_t;


pci_device_t* pci_find_class(u32 class);

#endif