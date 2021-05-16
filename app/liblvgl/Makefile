#
# Makefile
#
CC = /opt/local/bin/i386-elf-gcc
LD= /opt/local/bin/i386-elf-ld
CXX= /opt/local/bin/i386-elf-g++
AR=  /opt/local/bin/i386-elf-ar
STRIP= /opt/local/bin/i386-elf-strip
RANLIB= /opt/local/bin/i386-elf-ranlib
OBJCOPY= /opt/local/bin/i386-elf-objcopy
CFLAGS=-DX86 -march=i486 -I../libpng -I../../libs/include/c -I../../libs/include -I../libgui  -I. -I./include -Ilibs/include -g -m32 -c -std=c99 -std=gnu99 -w -nostdlib -nostdinc  #-fno-builtin 
LINKFLAGS =-nostdlib -nostartfiles  -g -static

LVGL_DIR ?= ${shell pwd}/..
LVGL_DIR_NAME ?= liblvgl

WARNINGS = -Werror -Wall -Wextra \
           -Wshadow -Wundef -Wmaybe-uninitialized -Wmissing-prototypes -Wpointer-arith -Wuninitialized \
           -Wunreachable-code -Wreturn-type -Wmultichar -Wformat-security -Wdouble-promotion -Wclobbered -Wdeprecated  \
           -Wempty-body -Wshift-negative-value -Wstack-usage=2048  \
           -Wtype-limits -Wsizeof-pointer-memaccess -Wmissing-prototypes -Wno-discarded-qualifiers

WARNINGS += -Wpedantic -pedantic-errors

#-Wno-unused-value -Wno-unused-parameter
OPTIMIZATION ?= #-O3 -g0

CFLAGS += -I$(LVGL_DIR)/ $(DEFINES)   -I$(LVGL_DIR) -I. $(OPTIMIZATION) $(WARNINGS)

LDFLAGS ?= $(LINKFLAGS) --entry main -T../../user.ld  -L../libpng/ -lpng -L../libgui -lgui -L../../libs/libc -lc -L../../libs/libalgorithm/ -lalgorithm  -lm

BIN ?= lvgl
LIB = liblvgl.a # 目标文件名 

#Collect the files to compile
MAINSRC = 

include lvgl.mk



CSRCS += 

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

$(LIB) : $(OBJS)
	echo  $(OBJS)
	$(AR) cr $@ $(OBJS)

## MAINOBJ -> OBJFILES

all: $(LIB) #default

%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"

default: $(AOBJS) $(COBJS) $(MAINOBJ)
	$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS)

clean:
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ)
