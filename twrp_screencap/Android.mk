LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := twrp_screencap
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/system/bin
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -g -O0
LOCAL_STRIP := false

LOCAL_SRC_FILES := screencap.cpp
LOCAL_MODULE_STEM := screencap
include $(BUILD_EXECUTABLE)
