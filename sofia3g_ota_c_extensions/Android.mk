# Copyright (C) 2013 Intel Mobile Communications GmbH
# Copyright (C) 2007 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
# librecovery_update_ifx is a set of edify extension functions for
# doing device specific OTA steps

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := sofia_extra_hooks.c
#LOCAL_STATIC_LIBRARIES += libmtdutils
LOCAL_C_INCLUDES += bootable/recovery
LOCAL_C_INCLUDES += system/core
LOCAL_MODULE := librecovery_sofia3gr_intel
include $(BUILD_STATIC_LIBRARY)

