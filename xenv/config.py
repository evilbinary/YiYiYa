# coding:utf-8
# *******************************************************************
# * Copyright 2021-2080 evilbinary
# * 作者: evilbinary on 01/01/20
# * 邮箱: rootdebug@163.com
# ********************************************************************
# default libc 默认libc 选择
default_libc = 'libmusl'  # libmusl libc libnewlib

# board select platform,选择你要编译的平台
#riscv-virt dmulator i386-pc v3s raspi2 esp32 stm32f4xx raspi3 rk3128 rk3288 cubieboard2 dummy rk3288 orangepi-pc
platform = 'i386-pc'

# 是否要构建应用
default_apps = True

#构建的module

modules=['musl'] #'ewok'

# gcc path,这里是默认的gcc配置，使用的时候可以启用一下面配置实例注释
CC_PREFIX = ''
CC_PATH = ''
CC_LIB_PATH = ''

CC = 'gcc'
LD = 'ld'
AR = 'ar'
AS = 'as'
CXX = 'cxx'
STRIP = 'strip'
RANLIB = 'ranlib'
OBJCOPY = 'objcopy'
CFLAGS = ''
ARFLAGS = 'rc'
OBJCOPYFLAGS = ' -S -g'
LINKFLAGS = '-nostdlib -nostartfiles -e _start  -g '
LINKLD = 'link-'+platform+'.ld'+ '' # -Map=kernel.map 
SINGLE_KERNEL = False
ASFLAGS=''
LDFLAGS=''

# ******************************************************************** #

# oxs x86 duck 配置实例
# CC_PREFIX='i386-elf-'
# CC=CC_PREFIX+'gcc'
# CC_LIB_PATH ='/opt/local/lib/gcc/i386-elf/9.2.0/'
# AR=CC_PREFIX+'ar'
# LD=CC_PREFIX+'ld'
# AS=CC_PREFIX+'as'
# OBJCOPY=CC_PREFIX+'objcopy'
# RANLIB=CC_PREFIX+'ranlib'
# CFLAGS='-march=i486'

# LINKLD='-Map=kernel.map -T xlinker/link-dmulator.ld'

# default_apps=False

# osx arm 配置实例

# CC_PATH = ''
# CC_PREFIX = 'arm-none-eabi-'
# CC_LIB_PATH = '/opt/local/lib/gcc/arm-none-eabi/9.2.0/'
# CC = CC_PREFIX+'gcc'
# AR = CC_PREFIX+'ar'
# LD = CC_PREFIX+'ld'
# AS = CC_PREFIX+'as'
# OBJCOPY = CC_PREFIX+'objcopy'
# RANLIB = CC_PREFIX+'ranlib'
# CFLAGS = '-fstack-protector ' #-fstack-protector -mapcs-frame mov ip, sp 特权模式下 -fstack-protector   -fstack-protector-all  -DMALLOC_TRACE -fno-omit-frame-pointer
# LDFLAGS=' '
# LINKLD = 'link-'+platform+'.ld'+ '' # -Map=kernel.map 

# default_apps = False


# osx riscv 配置实例

# CC_PATH = ''
# CC_PREFIX = 'riscv64-unknown-elf-'
# CC_LIB_PATH = ''
# CC = CC_PREFIX+'gcc'
# AR = CC_PREFIX+'ar'
# LD = CC_PREFIX+'ld'
# AS = CC_PREFIX+'as'
# OBJCOPY = CC_PREFIX+'objcopy'
# RANLIB = CC_PREFIX+'ranlib'
# CFLAGS = ' -DSBI -fstack-protector -nostdlib -nostdinc  -fno-builtin -mcmodel=medany  -march=rv32imaczicsr -mabi=ilp32 -g -Wall' #-fstack-protector -mapcs-frame mov ip, sp 特权模式下 -fstack-protector   -fstack-protector-all  -DMALLOC_TRACE -fno-omit-frame-pointer
# LDFLAGS='  -nostdlib -march=rv32imaczicsr -mabi=ilp32  -mcmodel=medany '
# LINKFLAGS='   '

# LINKLD = 'link-'+platform+'-sbi.ld'+ '' # -Map=kernel.map 
# ASFLAGS= ' -march=rv32imaczicsr -mabi=ilp32 '
# SINGLE_KERNEL= True

# default_apps = False

# osx armv7 配置实例

# CC_PATH = ''
# CC_PREFIX = 'arm-none-eabi-'
# CC_LIB_PATH = ''
# CC = CC_PREFIX+'gcc'
# AR = CC_PREFIX+'ar'
# LD = CC_PREFIX+'ld'
# AS = CC_PREFIX+'as'
# OBJCOPY = CC_PREFIX+'objcopy'
# RANLIB = CC_PREFIX+'ranlib'
# CFLAGS = '-DSTM32F401xC -fstack-protector -nostdlib -nostdinc  -fno-builtin' #-fstack-protector -mapcs-frame -fstack-protector   -fstack-protector-all  -DMALLOC_TRACE -fno-omit-frame-pointer -DSTM32F401xC -DSTM32F429xx
# LDFLAGS=' -nostdlib '
# LINKLD = 'link-'+platform+'.ld'+ ' -Wl,-Map=kernel.map ' # -Map=kernel.map 
# SINGLE_KERNEL= True #单个文件 kernel.elf

# default_apps = False


# osx i668 配置实例

CC_PREFIX='i686-elf-'
CC='i686-elf-gcc'
CXX = 'i686-elf-gcc'
CC_LIB_PATH ='/usr/local/opt/i686-elf-gcc/lib/gcc/i686-elf/13.1.0/' #port install lib path /opt/local/lib/gcc/i686-elf/9.2.0/
AR='i686-elf-ar'
LD='i686-elf-ld'
AS='i686-elf-as'
OBJCOPY='i686-elf-objcopy'
RANLIB='i686-elf-ranlib'
CFLAGS='-march=i686 ' #-DMALLOC_TRACE -fsanitize=leak  -fsanitize=address -lasan -fno-omit-frame-pointer
LDFLAGS=' '
LINKLD = 'link.ld'+ ' -Xlinker -Map=kernel.map ' # 

default_libc = 'libmusl'
default_apps = False


# osx x86-64 for demulator 配置实例

# CC_PREFIX=''
# CC='gcc-mp-8'
# CXX = 'gcc-mp-8'
# CC_LIB_PATH ='/opt/local/lib/gcc/x86_64-elf/9.2.0/'
# AR='gcc-ar-mp-8'
# LD='i386-elf-ld'
# AS='i386-elf-as'
# OBJCOPY='i386-elf-objcopy'
# RANLIB='i386-elf-ranlib'
# CFLAGS=' ' #-DMALLOC_TRACE -fsanitize=leak  -fsanitize=address -lasan -fno-omit-frame-pointer
# LDFLAGS=' '
# LINKLD = '' # link.ld'+ ' -Xlinker -Map=kernel.map 

# default_libc = 'libmusl'
# default_apps = False

# clang
# CC_PREFIX=''
# CC='gcc -m32 '
# CXX = 'gcc -m32 '
# CC_LIB_PATH =''
# AR='ar'
# LD='ld'
# AS='as'
# OBJCOPY=''
# RANLIB='ranlib'
# CFLAGS=' -I/usr/include/ -D_XOPEN_SOURCE' #-DMALLOC_TRACE -fsanitize=leak  -fsanitize=address -lasan -fno-omit-frame-pointer
# LDFLAGS=''
# LINKLD = '' # link.ld'+ ' -Xlinker -Map=kernel.map
# ARFLAGS='-rv'
# LINKFLAGS = '-e start  -g '

# default_libc = 'libmusl'
# default_apps = False



# windows arm 配置实例

# CC_PATH=''
# CC_PREFIX='arm-none-eabi-'
# CC_LIB_PATH='D:\\Program Files\\gcc-arm-none-eabi-9\\lib\\gcc\\arm-none-eabi\\9.2.1\\'
# CC = CC_PREFIX+'gcc'
# AR = CC_PREFIX+'ar'
# LD = CC_PREFIX+'ld'
# AS = CC_PREFIX+'as'
# OBJCOPY = CC_PREFIX+'objcopy'
# RANLIB = CC_PREFIX+'ranlib'
# LDFLAGS=' '
# LINKLD = 'link.ld'+ '' # -Map=kernel.map 
# CFLAGS=''

# windows i386 配置实例
# CC_PATH='D:\\app\\gcc\\bin\\'
# CC_PREFIX='i686-elf-'
# CC = CC_PATH+CC_PREFIX+'gcc'
# CC_LIB_PATH ='D:\\app\\gcc\\lib\\gcc\\i686-elf\\7.1.0\\'
# AR =  CC_PATH+CC_PREFIX+'ar'
# LD=  CC_PATH+CC_PREFIX+'ld'
# AS= CC_PATH+CC_PREFIX+'as'
# OBJCOPY =  CC_PATH+CC_PREFIX+'objcopy'
# RANLIB = CC_PATH+CC_PREFIX+'ranlib'
# CFLAGS='-march=i486'
# LDFLAGS=' '
# LINKLD = 'link.ld'+ '' # -Map=kernel.map 

# windows arm 配置实例

# CC_PATH=''
# CC_PREFIX='i686-elf-'
# CC_LIB_PATH ='D:\\app\\gcc\\lib\\gcc\\i686-elf\\7.1.0\\'
# CC = CC_PREFIX+'gcc'
# AR = CC_PREFIX+'ar'
# LD = CC_PREFIX+'ld'
# AS = CC_PREFIX+'as'
# OBJCOPY = CC_PREFIX+'objcopy'
# RANLIB = CC_PREFIX+'ranlib'

# osx xteansa 配置实例

# CC_PATH='~/.espressif/tools/xtensa-esp32-elf/esp-2021r1-8.4.0/xtensa-esp32-elf/bin/'
# CC_PREFIX=CC_PATH+'xtensa-esp32-elf-'
# CC_LIB_PATH=''
# CC = CC_PREFIX+'gcc'
# AR = CC_PREFIX+'ar'
# LD = CC_PREFIX+'ld'
# AS = CC_PREFIX+'as'
# OBJCOPY = CC_PREFIX+'objcopy'
# RANLIB = CC_PREFIX+'ranlib'
# CFLAGS = '-fstack-protector -ffunction-sections '#-DMALLOC_TRACE  -mtext-section-literals 
# LINKFLAGS = ' -nostartfiles -e _start  -g '
# ASFLAGS ='-Iduck/arch/lx6 -I. -Iarch/lx6 '
# SINGLE_KERNEL= True

# default_apps = False