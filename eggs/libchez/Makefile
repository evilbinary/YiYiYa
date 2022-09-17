##########################################################
# Copyright 2016-2080 evilbinary.
#作者:evilbinary on 12/24/16.
#邮箱:rootdebug@163.oom
##########################################################

LIBS =-lncurses 
CFLAGS =  -Ic   -DINLINES -DGC_MACROS   -Wpointer-arith -Wall -Wextra -g  -fPIC -DSIGNAL_HANDLER #-DLIBICONV_PLUG 
CXXFLAGS = -Wall -Wformat
ARFLAGS= crv
BITS =$(shell getconf LONG_BIT)

INCLUDES =-Ic -I. -I../3rdparty/include
CFLAGS+= $(INCLUDES) #-DSIGNAL_HANDLER

TARGET = libchez.a
OBJS = c/statics.o c/segment.o c/alloc.o c/symbol.o c/intern.o c/gcwrapper.o c/gc-ocd.o c/gc-oce.o\
                      c/number.o c/schsig.o c/io.o c/new-io.o c/print.o c/fasl.o c/stats.o c/foreign.o c/prim.o c/prim5.o c/flushcache.o\
                      c/schlib.o c/thread.o c/expeditor.o c/scheme.o \
                      c/gc.o c/i3le.o c/compress-io.o

#bit=$(shell uname -a | egrep 'amd64e|x86_64')

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += 
	CXXFLAGS += 
	CXXFLAGS += 
	CFLAGS +=

	ifeq ($(shell uname -m) ,armv7l)
                 CFLAGS +=-Iarm32le  -DINLINES -DGC_MACROS -DARMV6 -DMB_LEN_MAX=8 -D_LARGEFILE64_SOURCE -DHAVE_HIDDEN
                OBJS = c/statics.o c/segment.o c/alloc.o c/symbol.o c/intern.o c/gcwrapper.o c/gc-ocd.o c/gc-oce.o\
                      c/number.o c/schsig.o c/io.o c/new-io.o c/print.o c/fasl.o c/stats.o c/foreign.o c/prim.o c/prim5.o c/flushcache.o\
                      c/schlib.o c/thread.o c/expeditor.o c/scheme.o \
                      c/gc.o c/arm32le.o
        else
                ifeq ($(BITS), 32)
                        CFLAGS +=-Ii3le -DI386  -D_LARGEFILE64_SOURCE -DHAVE_HIDDEN
                else
                        CFLAGS +=-Ita6le -DX86_64  -D_LARGEFILE64_SOURCE -DHAVE_HIDDEN
                endif
        endif
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += 
	CXXFLAGS += 
	CXXFLAGS += 
	ifeq ($(BITS), 32)
		CFLAGS +=-Ii3osx -DI386
	else
		CFLAGS +=-Ita6osx -DX86_64 -DMB_LEN_MAX=4
	endif
#	CXXFLAGS += -D__APPLE__
endif

ifeq ($(findstring MINGW32_NT-, $(UNAME_S)), MINGW32_NT-)
	ECHO_MESSAGE="Windows32"
	LIBS+=
	CFLAGS+=-Iti3nt -DI386  -O2 -D__int64="long long"  -DPTHREADS  -DEWOULDBLOCK=11 -D_LARGEFILE64_SOURCE  -D__forceinline=inline  -DUSE_NO_MINGW_SETJMP_TWO_ARGS  #-D__SEH_NOOP
	CXXFLAGS+=$(CFLAGS)
	OBJS =   c/statics.o c/segment.o c/alloc.o c/symbol.o c/intern.o c/gcwrapper.o c/gc-ocd.o c/gc-oce.o\
       c/number.o c/schsig.o c/io.o c/new-io.o c/print.o c/fasl.o c/stats.o c/foreign.o c/prim.o c/prim5.o c/flushcache.o\
      c/schlib.o c/thread.o c/expeditor.o c/scheme.o \
      c/gc.o c/windows.o c/compress-io.o
endif

ifeq ($(findstring MINGW64_NT-, $(UNAME_S)), MINGW64_NT-)
	ECHO_MESSAGE="Windows64"
	LIBS+=
	CFLAGS+=-Ita6nt -DX86_64  -O2 -D__int64="long long"  -DPTHREADS  -DEWOULDBLOCK=11 -D_LARGEFILE64_SOURCE  -D__forceinline=inline  -DUSE_NO_MINGW_SETJMP_TWO_ARGS  #-D__SEH_NOOP
	CXXFLAGS+=$(CFLAGS)
	OBJS = c/gc-011.o c/gcwrapper.o c/statics.o c/segment.o c/alloc.o c/symbol.o c/intern.o c/gcwrapper.o c/gc-ocd.o c/gc-oce.o\
       c/number.o c/schsig.o c/io.o c/new-io.o c/print.o c/fasl.o c/stats.o c/foreign.o c/prim.o c/prim5.o c/flushcache.o\
      c/schlib.o c/thread.o c/expeditor.o c/scheme.o \
      c/gc.o c/windows.o c/compress-io.o
endif

ifeq ($(UNAME_S), MINGW64_NT-6.3)
   ECHO_MESSAGE = "Windows"
	LIBS += 
	CXXFLAGS += 
	CXXFLAGS += 
	CFLAGS +=
endif

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
all: $(TARGET)
	@echo Build complete for $(ECHO_MESSAGE)

$(TARGET): $(OBJS)
	@echo $(BITS)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJS)
clean:
	rm -rf $(TARGET)  *.o c/*.o

