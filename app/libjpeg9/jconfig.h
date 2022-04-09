#ifndef __JCONFIG_H__
#define __JCONFIG_H__


/*
 * jconfig.doc
 *
 * Copyright (C) 1991-1994, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file documents the configuration options that are required to
 * customize the JPEG software for a particular system.
 *
 */

#include "types.h"
#include "stdio.h"

#ifdef ARM
#define HAVE_PROTOTYPES 1
// typedef unsigned char boolean;
// #define HAVE_BOOLEAN
// #ifdef JPEG_INTERNALS
// #define INLINE __inline__
// #endif
#endif

/*
 * These symbols indicate the properties of your machine or compiler.
 * #define the symbol if yes, #undef it if no.
 */

/* Does your compiler support the declaration "unsigned char" ?
 * How about "unsigned short" ?
 */
#define HAVE_UNSIGNED_CHAR


/* Define this if an ordinary "char" type is unsigned.
 * If you're not sure, leaving it undefined will work at some cost in speed.
 * If you defined HAVE_UNSIGNED_CHAR then the speed difference is minimal.
 */
#ifdef CHAR_IS_UNSIGNED
  #undef CHAR_IS_UNSIGNED
#endif
#define HAVE_STDDEF_H   /* Define this if your system has an ANSI-conforming <stddef.h> file. */
#define HAVE_STDLIB_H   /* Define this if your system has an ANSI-conforming <stdlib.h> file. */
#ifdef NEED_BSD_STRINGS
  #undef NEED_BSD_STRINGS /* Define this if your system does not have an ANSI/SysV <string.h>, but does have a BSD-style <strings.h>. */
#endif

/* Define this if your system does not provide typedef size_t in any of the
 * ANSI-standard places (stddef.h, stdlib.h, or stdio.h), but places it in
 * <sys/types.h> instead.
 */
#ifdef NEED_SYS_TYPES_H
  #undef NEED_SYS_TYPES_H
#endif

/* For 80x86 machines, you need to define NEED_FAR_POINTERS,
 * unless you are using a large-data memory model or 80386 flat-memory mode.
 * On less brain-damaged CPUs this symbol must not be defined.
 * (Defining this symbol causes large data structures to be referenced through
 * "far" pointers and to be allocated with a special version of malloc.)
 */
#ifdef NEED_SYS_TYPES_H
  #undef NEED_FAR_POINTERS
#endif


/* Although a real ANSI C compiler can deal perfectly well with pointers to
 * unspecified structures (see "incomplete types" in the spec), a few pre-ANSI
 * and pseudo-ANSI compilers get confused.  To keep one of these bozos happy,
 * define INCOMPLETE_TYPES_BROKEN.  This is not recommended unless you
 * actually get "missing structure definition" warnings or errors while
 * compiling the JPEG code.
 */
#ifdef INCOMPLETE_TYPES_BROKEN
  #undef INCOMPLETE_TYPES_BROKEN
#endif


/*
 * The following options affect code selection within the JPEG library,
 * but they don't need to be visible to applications using the library.
 * To minimize application namespace pollution, the symbols won't be
 * defined unless JPEG_INTERNALS has been defined.
 */

#ifdef JPEG_INTERNALS

/* Define this if your compiler implements ">>" on signed values as a logical
 * (unsigned) shift; leave it undefined if ">>" is a signed (arithmetic) shift,
 * which is the normal and rational definition.
 */
#ifdef RIGHT_SHIFT_IS_UNSIGNED
  #undef RIGHT_SHIFT_IS_UNSIGNED
#endif


#endif /* JPEG_INTERNALS */

#ifndef GUI_USE_PARA
  #define GUI_USE_PARA(x)
#endif


#endif /* __JCONFIG_H__ */
