
/*
 * Copyright (c) 2006-2012 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef BYTEORDERING_H
#define BYTEORDERING_H

#include "kernel/kernel.h"

#define LITTLE_ENDIAN

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \addtogroup byteordering
 *
 * @{
 */
/**
 * \file
 * Byte-order handling header (license: GPLv2 or LGPLv2.1)
 *
 * \author Roland Riegel
 */

#define SWAP16(val) ((((uint16_t) (val)) << 8) | \
                     (((uint16_t) (val)) >> 8)   \
                    )
#define SWAP32(val) (((((uint32_t) (val)) & 0x000000ff) << 24) | \
                     ((((uint32_t) (val)) & 0x0000ff00) <<  8) | \
                     ((((uint32_t) (val)) & 0x00ff0000) >>  8) | \
                     ((((uint32_t) (val)) & 0xff000000) >> 24)   \
                    )

#ifdef LITTLE_ENDIAN 
#define SWAP_NEEDED 0
#elif BIG_ENDIAN
#define SWAP_NEEDED 1
#else
#error "Endianess undefined! Please define LITTLE_ENDIAN=1 or BIG_ENDIAN=1."
#endif

/**
 * \def HTOL16(val)
 *
 * Converts a 16-bit integer from host byte order to little-endian byte order.
 *
 * Use this macro for compile time constants only. For variable values
 * use the function htol16() instead. This saves code size.
 *
 * \param[in] val A 16-bit integer in host byte order.
 * \returns The given 16-bit integer converted to little-endian byte order.
 */
/**
 * \def HTOL32(val)
 *
 * Converts a 32-bit integer from host byte order to little-endian byte order.
 *
 * Use this macro for compile time constants only. For variable values
 * use the function htol32() instead. This saves code size.
 *
 * \param[in] val A 32-bit integer in host byte order.
 * \returns The given 32-bit integer converted to little-endian byte order.
 */
/**
 * \def LTOH16(val)
 *
 * Converts a 16-bit integer from little-endian byte order to host byte order.
 *
 * Use this macro for compile time constants only. For variable values
 * use the function ltoh16() instead. This saves code size.
 *
 * \param[in] val A 16-bit integer in little-endian byte order.
 * \returns The given 16-bit integer converted to host byte order.
 */
/**
 * \def LTOH32(val)
 *
 * Converts a 32-bit integer from little-endian byte order to host byte order.
 *
 * Use this macro for compile time constants only. For variable values
 * use the function ltoh32() instead. This saves code size.
 *
 * \param[in] val A 32-bit integer in little-endian byte order.
 * \returns The given 32-bit integer converted to host byte order.
 */

#if SWAP_NEEDED
#define HTOL16(val) SWAP16(val)
#define HTOL32(val) SWAP32(val)
#define LTOH16(val) SWAP16(val)
#define LTOH32(val) SWAP32(val)
#else
#define HTOL16(val) (val)
#define HTOL32(val) (val)
#define LTOH16(val) (val)
#define LTOH32(val) (val)
#endif

#if DOXYGEN

/**
 * Converts a 16-bit integer from host byte order to little-endian byte order.
 *
 * Use this function on variable values instead of the
 * macro HTOL16(). This saves code size.
 *
 * \param[in] h A 16-bit integer in host byte order.
 * \returns The given 16-bit integer converted to little-endian byte order.
 */
uint16_t htol16(uint16_t h);

/**
 * Converts a 32-bit integer from host byte order to little-endian byte order.
 *
 * Use this function on variable values instead of the
 * macro HTOL32(). This saves code size.
 *
 * \param[in] h A 32-bit integer in host byte order.
 * \returns The given 32-bit integer converted to little-endian byte order.
 */
uint32_t htol32(uint32_t h);

/**
 * Converts a 16-bit integer from little-endian byte order to host byte order.
 *
 * Use this function on variable values instead of the
 * macro LTOH16(). This saves code size.
 *
 * \param[in] l A 16-bit integer in little-endian byte order.
 * \returns The given 16-bit integer converted to host byte order.
 */
uint16_t ltoh16(uint16_t l);

/**
 * Converts a 32-bit integer from little-endian byte order to host byte order.
 *
 * Use this function on variable values instead of the
 * macro LTOH32(). This saves code size.
 *
 * \param[in] l A 32-bit integer in little-endian byte order.
 * \returns The given 32-bit integer converted to host byte order.
 */
uint32_t ltoh32(uint32_t l);

#elif SWAP_NEEDED

#define htol16(h) swap16(h)
#define htol32(h) swap32(h)
#define ltoh16(l) swap16(l)
#define ltoh32(l) swap32(l)

#else

#define htol16(h) (h)
#define htol32(h) (h)
#define ltoh16(l) (l)
#define ltoh32(l) (l)

#endif

uint16_t read16(const uint8_t* p);
uint32_t read32(const uint8_t* p);
void write16(uint8_t* p, uint16_t i);
void write32(uint8_t* p, uint32_t i);

/**
 * @}
 */

#if SWAP_NEEDED
uint16_t swap16(uint16_t i);
uint32_t swap32(uint32_t i);
#endif

#ifdef __cplusplus
}
#endif

#endif

