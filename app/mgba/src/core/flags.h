#ifndef FLAGS_H
#define FLAGS_H

#ifndef MINIMAL_CORE
// #define MINIMAL_CORE @MINIMAL_CORE@
#endif

// BUILD flags

// #ifndef BUILD_GL
// #define BUILD_GL
// #endif

// #ifndef BUILD_GLES2
// #define BUILD_GLES2
// #endif

// #ifndef BUILD_GLES3
// #define BUILD_GLES3
// #endif

// Miscellaneous flags

#ifndef COLOR_16_BIT
#define COLOR_16_BIT
#endif

#ifndef COLOR_5_6_5
#define COLOR_5_6_5
#endif

#ifndef DISABLE_THREADING
#define DISABLE_THREADING
#endif

// #ifndef FIXED_ROM_BUFFER
// #define FIXED_ROM_BUFFER
// #endif

// M_CORE flags

#ifndef M_CORE_GBA
#define M_CORE_GBA
#endif

#ifndef M_CORE_GB
#define M_CORE_GB
#endif

// ENABLE flags

#ifndef ENABLE_SCRIPTING
#define ENABLE_SCRIPTING
#endif

// USE flags

#ifndef USE_DEBUGGERS
#define USE_DEBUGGERS
#endif

// #ifndef USE_EDITLINE
// #define USE_EDITLINE
// #endif

// #ifndef USE_ELF
// #define USE_ELF
// #endif

// #ifndef USE_EPOXY
// #define USE_EPOXY
// #endif

// #ifndef USE_FFMPEG
// #define USE_FFMPEG
// #endif

// #ifndef USE_GDB_STUB
// #define USE_GDB_STUB
// #endif

// #ifndef USE_LIBAV
// #define USE_LIBAV
// #endif

#ifndef USE_LIBAVRESAMPLE
#define USE_LIBAVRESAMPLE
#endif

#ifndef USE_LIBSWRESAMPLE
#define USE_LIBSWRESAMPLE
#endif

// #ifndef USE_LIBZIP
// #define USE_LIBZIP
// #endif

// #ifndef USE_LZMA
// #define USE_LZMA
// #endif

#ifndef USE_MINIZIP
#define USE_MINIZIP
#endif

#ifndef USE_PNG
#define USE_PNG
#endif

// #ifndef USE_PTHREADS
// #define USE_PTHREADS
// #endif

// #ifndef USE_SQLITE3
// #define USE_SQLITE3
// #endif

#ifndef USE_ZLIB
#define USE_ZLIB
#endif


#ifndef HAVE_STRDUP
#define HAVE_STRDUP
#endif


#ifndef HAVE_STRNDUP
#define HAVE_STRNDUP
#endif


#ifndef HAVE_CRC32
#define HAVE_CRC32
#endif


#ifndef HAVE_LOCALTIME_R
#define HAVE_LOCALTIME_R
#endif



#endif
