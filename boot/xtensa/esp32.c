#include "init.h"

typedef int esp_err_t;

/* Definitions for error constants. */
#define ESP_OK 0    /*!< esp_err_t value indicating success (no error) */
#define ESP_FAIL -1 /*!< Generic esp_err_t code indicating failure */

#define ESP_ERR_NO_MEM 0x101        /*!< Out of memory */
#define ESP_ERR_INVALID_ARG 0x102   /*!< Invalid argument */
#define ESP_ERR_INVALID_STATE 0x103 /*!< Invalid state */
#define ESP_ERR_INVALID_SIZE 0x104  /*!< Invalid size */
#define ESP_ERR_NOT_FOUND 0x105     /*!< Requested resource not found */
#define ESP_ERR_NOT_SUPPORTED 0x106 /*!< Operation or feature not supported */
#define ESP_ERR_TIMEOUT 0x107       /*!< Operation timed out */
#define ESP_ERR_INVALID_RESPONSE 0x108 /*!< Received response was invalid */
#define ESP_ERR_INVALID_CRC 0x109      /*!< CRC or checksum was invalid */
#define ESP_ERR_INVALID_VERSION 0x10A  /*!< Version was invalid */
#define ESP_ERR_INVALID_MAC 0x10B      /*!< MAC address was invalid */
#define ESP_ERR_NOT_FINISHED                      \
  0x10C /*!< There are items remained to retrieve \
         */

#define ESP_ERR_WIFI_BASE 0x3000  /*!< Starting number of WiFi error codes */
#define ESP_ERR_MESH_BASE 0x4000  /*!< Starting number of MESH error codes */
#define ESP_ERR_FLASH_BASE 0x6000 /*!< Starting number of flash error codes */
#define ESP_ERR_HW_CRYPTO_BASE \
  0xc000 /*!< Starting number of HW cryptography module error codes */

#define RSR(reg, at) asm volatile("rsr %0, %1" : "=r"(at) : "i"(reg))
#define WSR(reg, at) asm volatile("wsr %0, %1" : : "r"(at), "i"(reg))
#define XSR(reg, at) asm volatile("xsr %0, %1" : "+r"(at) : "i"(reg))

#define RER(reg, at) asm volatile("rer %0, %1" : "=r"(at) : "r"(reg))

#define WITLB(at, as) \
  asm volatile("witlb  %0, %1; \n isync \n " : : "r"(at), "r"(as))
#define WDTLB(at, as) \
  asm volatile("wdtlb  %0, %1; \n dsync \n " : : "r"(at), "r"(as))

typedef enum {
  MPU_REGION_ILLEGAL,
#if SOC_MPU_REGION_RO_SUPPORTED
  MPU_REGION_RO,  // read-only
#endif
#if SOC_MPU_REGION_WO_SUPPORTED
  MPU_REGION_WO,  // write-only
#endif
  MPU_REGION_RW,  // read-write
  MPU_REGION_X,   // executable
  MPU_REGION_RWX  // read-write-executable
} mpu_access_t;

/*-------------------------- MPU CAPS ----------------------------------------*/
// TODO: correct the caller and remove unsupported lines
#define SOC_MPU_CONFIGURABLE_REGIONS_SUPPORTED 0
#define SOC_MPU_MIN_REGION_SIZE 0x20000000U
#define SOC_MPU_REGIONS_MAX_NUM 8
#define SOC_MPU_REGION_RO_SUPPORTED 0
#define SOC_MPU_REGION_WO_SUPPORTED 0

static inline void mpu_ll_set_region_rw(uint32_t addr) {
  WDTLB(0x0, addr);  // cached, no allocate
}

static inline void mpu_ll_set_region_rwx(uint32_t addr) {
  WDTLB(0x2, addr);  // bypass cache
}

static inline void mpu_ll_set_region_x(uint32_t addr) {
  WITLB(0x3, addr);  // cached
}

static inline void mpu_ll_set_region_illegal(uint32_t addr) {
  WITLB(0xF, addr);
  WDTLB(0xF, addr);
}

static inline uint32_t mpu_ll_id_to_addr(unsigned id) {
  // vpn - id
  // 0x00000000 = 0
  // 0x20000000 = 1
  // 0x40000000 = 2
  // 0x60000000 = 3
  // 0x80000000 = 4
  // 0xa0000000 = 5
  // 0xc0000000 = 6
  // 0xe0000000 = 7
  return id * SOC_MPU_MIN_REGION_SIZE;
}

void mpu_hal_set_region_access(int id, mpu_access_t access) {
  uint32_t addr = mpu_ll_id_to_addr(id);

  switch (access) {
#if SOC_MPU_REGION_RO_SUPPORTED
    case MPU_REGION_RO:
      mpu_ll_set_region_ro(addr);
      break;
#endif
#if SOC_MPU_REGION_WO_SUPPORTED
    case MPU_REGION_WO:
      mpu_ll_set_region_wo(addr);
      break;
#endif
    case MPU_REGION_RW:
      mpu_ll_set_region_rw(addr);
      break;
    case MPU_REGION_X:
      mpu_ll_set_region_x(addr);
      break;
    case MPU_REGION_RWX:
      mpu_ll_set_region_rwx(addr);
      break;
    case MPU_REGION_ILLEGAL:
      mpu_ll_set_region_illegal(addr);
      break;
    default:
      break;
  }
}

/* Note: currently this is configured the same on all Xtensa targets
 *
 * Both chips have the address space divided into 8 regions, 512MB each.
 */
void esp_cpu_configure_region_protection(void) {
  const int illegal_regions[6] = {
      0, 4, 5, 6,
      7};  // 0x00000000, 0x80000000, 0xa0000000, 0xc0000000, 0xe0000000
  for (size_t i = 0; i < sizeof(illegal_regions) / sizeof(illegal_regions[0]);
       ++i) {
    mpu_hal_set_region_access(illegal_regions[i], MPU_REGION_ILLEGAL);
  }

  mpu_hal_set_region_access(1, MPU_REGION_RW);   // 0x20000000
  mpu_hal_set_region_access(2, MPU_REGION_RWX);  // 0x40000000
}

void bootloader_init_mem(void) {
  // protect memory region
  esp_cpu_configure_region_protection();
}

esp_err_t bootloader_init(void) {
  esp_err_t ret = ESP_OK;

  bootloader_init_mem();

  // clear bss section
//   bootloader_clear_bss_section();
// init eFuse virtual mode (read eFuses to RAM)
#ifdef CONFIG_EFUSE_VIRTUAL
  ESP_LOGW(
      TAG,
      "eFuse virtual mode is enabled. If Secure boot or Flash encryption is "
      "enabled then it does not provide any security. FOR TESTING ONLY!");
#ifndef CONFIG_EFUSE_VIRTUAL_KEEP_IN_FLASH
  esp_efuse_init_virtual_mode_in_ram();
#endif
#endif
  // bootst up vddsdio
//   bootloader_common_vddsdio_configure();
// reset MMU
//   bootloader_reset_mmu();
// check rated CPU clock
//   if ((ret = bootloader_check_rated_cpu_clock()) != ESP_OK) {
//     goto err;
//   }
//   // config clock
//   bootloader_clock_configure();
//   // initialize uart console, from now on, we can use esp_log
//   bootloader_console_init();
//   /* print 2nd bootloader banner */
//   bootloader_print_banner();
//   // update flash ID
//   bootloader_flash_update_id();

//   // read bootloader header
//   if ((ret = bootloader_read_bootloader_header()) != ESP_OK) {
//     goto err;
//   }
//   // read chip revision and check if it's compatible to bootloader
//   if ((ret = bootloader_check_bootloader_validity()) != ESP_OK) {
//     goto err;
//   }
//   // initialize spi flash
//   if ((ret = bootloader_init_spi_flash()) != ESP_OK) {
//     goto err;
//   }
//   // check whether a WDT reset happend
//   bootloader_check_wdt_reset();
//   // config WDT
//   bootloader_config_wdt();
//   // enable RNG early entropy source
//   bootloader_enable_random();
err:
  return ret;
}