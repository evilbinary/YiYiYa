##########################################################
# Copyright 2016-2080 evilbinary.
#作者:evilbinary on 12/24/16.
#邮箱:rootdebug@163.com
##########################################################

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
#include $(LOCAL_PATH)/../Optimizations.mk

ifeq ($(TARGET_ARCH),x86)
PLATFORM := androidx86
else
PLATFORM := android
endif

LOCAL_ARM_MODE := arm

LOCAL_MODULE    := liblz4
LOCAL_SRC_FILES := $(LOCAL_PATH)/lib/lz4.c \
                $(LOCAL_PATH)/lib/lz4frame.c \
                $(LOCAL_PATH)/lib/lz4hc.c \
                $(LOCAL_PATH)/lib/xxhash.c \


LOCAL_C_INCLUDES := $(LOCAL_PATH)/lib

LOCAL_CFLAGS +=  -I./lib/ -I.  #-DSIGNAL_HANDLER
LOCAL_CFLAGS += -g -Wall -DANDROID  -fPIC -pie -fPIE #-DPTHREADS -DFEATURE_PTHREADS

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)



