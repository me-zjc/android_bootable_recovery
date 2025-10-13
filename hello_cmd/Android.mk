LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := hello_cmd
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/system/bin
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := hello_cmd.cpp
include $(BUILD_EXECUTABLE)
