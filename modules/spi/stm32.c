/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "platform/stm32f4xx/gpio.h"
#include "spi.h"

// #define SPI_DMA 1

static SPI_HandleTypeDef hspi1;

static u32 stm32_spi_read(spi_t* spi, u32* data, u32 count) {
  if (data == NULL || count <= 0) {
    return 0;
  }
#ifdef SPI_DMA
  HAL_StatusTypeDef errorcode =
      HAL_SPI_Receive_DMA(&hspi1, data, count);
#else
  HAL_StatusTypeDef errorcode =
      HAL_SPI_Receive(&hspi1, data, count, HAL_MAX_DELAY);
#endif
  if (errorcode != HAL_OK) {
    return -1;
  }
  return count;
}

static u32 stm32_spi_write(spi_t* spi, u32* data, u32 count) {
  if (data == NULL || count <= 0) {
    return 0;
  }
#ifdef SPI_DMA
  HAL_StatusTypeDef errorcode =
      HAL_SPI_Transmit_DMA(&hspi1, data, count);
#else
  HAL_StatusTypeDef errorcode =
      HAL_SPI_Transmit(&hspi1, data, count, HAL_MAX_DELAY);
#endif
  if (errorcode != HAL_OK) {
    return -1;
  }
  return count;
}

static void stm32_spi_init() {
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  gpio_config(GPIOB, GPIO_PIN_11, GPIO_MODE_OUTPUT_PP);
  gpio_config(GPIOB, GPIO_PIN_6 | GPIO_PIN_7, GPIO_MODE_OUTPUT_PP);

  /* Peripheral clock enable */
  __HAL_RCC_SPI1_CLK_ENABLE();

  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK) {
    kprintf("spi init error\n");
  } else {
    kprintf("spi init success\n");
  }
}

int spi_init_device(device_t* dev) {
  spi_t* spi = kmalloc(sizeof(spi_t));
  dev->data = spi;

  spi->inited = 0;
  spi->read = stm32_spi_read;
  spi->write = stm32_spi_write;
  stm32_spi_init();
  spi->inited = 1;
  return 0;
}