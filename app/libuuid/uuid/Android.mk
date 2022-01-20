LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

include $(CLEAR_VARS)
LOCAL_MODULE := libuuid

LOCAL_SRC_FILES := compare.c gen_uuid.c isnull.c \
		parse.c unpack.c clear.c copy.c \
		 pack.c unparse.c uuid_time.c

LOCAL_CFLAGS += -g -Wall -DANDROID   -DINLINES -DGC_MACROS -fPIC -Wno-unused-parameter -pie -fPIE

LOCAL_STATIC_LIBRARIES := libuuid
LOCAL_LDLIBS := -lc -lm -lstdc++
#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)
