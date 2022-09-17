##########################################################
# Copyright 2016-2080 evilbinary.
#作者:evilbinary on 12/24/16.
#邮箱:rootdebug@163.com
##########################################################

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
include $(LOCAL_PATH)/../Optimizations.mk

ifeq ($(TARGET_ARCH),x86)
PLATFORM := androidx86
else
PLATFORM := android
endif

3RD=$(LOCAL_PATH)/../3rdparty-arm/

LOCAL_ARM_MODE := arm


SCHEME_TARGET=arm32le

LOCAL_MODULE    := libchez
LOCAL_SRC_FILES :=   c/statics.c c/segment.c c/alloc.c c/symbol.c c/intern.c c/gcwrapper.c c/gc-ocd.c c/gc-oce.c\
                                           c/number.c c/schsig.c c/io.c c/new-io.c c/print.c c/fasl.c c/stats.c c/foreign.c c/prim.c c/prim5.c c/flushcache.c\
                                           c/schlib.c c/thread.c c/expeditor.c c/scheme.c \
                                           c/gc.c c/arm32le.c c/compress-io.c

#NDK_SUPPORT_DIR=${NDK_TOOLCHAIN_DIR}/include/support

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SCHEME_TARGET)/  \
                    $(LOCAL_PATH)/../libncurses     \
                    $(LOCAL_PATH)/../libncurses/include \
                    $(3RD)/lz4/lib/ $(3RD)/ \
#                     NDK_SUPPORT_INCLUDES="-I${NDK_SUPPORT_DIR}"


LOCAL_CFLAGS += -I$(SCHEME_TARGET)/ -I. -I./c/ -DSIGNAL_HANDLER
LOCAL_CFLAGS += -g -Wall -DANDROID    -DINLINES -DGC_MACROS -DARMV6  -Wpointer-arith -Wall -Wextra  -DLIBICONV_PLUG -fPIC -pie -fPIE #-DPTHREADS -DFEATURE_PTHREADS

LOCAL_STATIC_LIBRARIES := android_support libncurses liblz4 libuuid
#LOCAL_WHOLE_STATIC_LIBRARIES += libncurses
LOCAL_MULTILIB := 32

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)
#$(call import-module,Android/support)



