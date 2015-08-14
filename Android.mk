LOCAL_PATH := $(call my-dir)
# Recover Secbin
include $(LOCAL_PATH)/otapackage_fwu.mk
include $(call all-makefiles-under,$(LOCAL_PATH))
