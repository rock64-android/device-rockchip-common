# Copyright (C) 2013, 2014 Intel Mobile Communications GmbH
# Copyright (C) 2011 The Android Open-Source Project
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
# Oct 20, 2013: Initial Creation for Sofia3G
# May 27 2014: IMC: add IMC test tools for engineering build

# Screen size is "normal", density is "mdpi"
#PRODUCT_AAPT_CONFIG := normal large mdpi tvdpi hdpi
#PRODUCT_AAPT_PREF_CONFIG := tvdpi


# Audio logging property for audio driver
ifeq ($(TARGET_BUILD_VARIANT),eng)
PRODUCT_PROPERTY_OVERRIDES += persist.audio.log=1
else
PRODUCT_PROPERTY_OVERRIDES += persist.audio.log=0
endif




ifeq ($(strip $(BOARD_HAS_GPS)),true)
PRODUCT_PACKAGES += \
    gps.$(TARGET_BOARD_HARDWARE) \
    libclientgps
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/init.gnss.normal.rc:root/init.gnss.normal.rc \
        $(LOCAL_PATH)/init.gnss.ptest.rc:root/init.gnss.ptest.rc
endif
# This is needed to enable silver art optimizer.
# This will build the plugins/libart-extension.so library,  which is dynamically loaded by
# AOSP and contains Intel optimizations to the compiler.
ifeq ($(strip $(BUILD_WITH_ART_EXTENSION)), true)
PRODUCT_PACKAGES += \
    libart-extension \
    libart-extension-cov
endif

#pcba
PRODUCT_PACKAGES += pcba_core
PRODUCT_PACKAGES += wpa_cli

# GPU Packages Stubbed.
PRODUCT_PACKAGES += \
    libGLES_mali

# Filesystem management tools
PRODUCT_PACKAGES += \
    e2fsck \
    setup_fs \
    charger \
    charger_res_images


# Widevine drm classic common
ifneq ($(BOARD_WIDEVINE_OEMCRYPTO_LEVEL),)
PRODUCT_PROPERTY_OVERRIDES += drm.service.enabled=true
PRODUCT_PACKAGES += com.google.widevine.software.drm.xml\
    com.google.widevine.software.drm\
    libdrmwvmplugin \
    libwvm \
    libdrmdecrypt
endif

# Widevine drm classic L1 only
ifeq ($(BOARD_WIDEVINE_OEMCRYPTO_LEVEL),1)
PRODUCT_PACKAGES += \
    libWVStreamControlAPI_L1 \
    libwvdrm_L1
endif

# Widevine drm classic L3 only
ifeq ($(BOARD_WIDEVINE_OEMCRYPTO_LEVEL),3)
PRODUCT_PACKAGES += \
    libWVStreamControlAPI_L3 \
    libwvdrm_L3
endif

# Widevine drm modular common
PRODUCT_PACKAGES += \
    libwvdrmengine

# Widevine drm modular L1 only
#ifeq ($(BOARD_WIDEVINE_OEMCRYPTO_LEVEL),1)
PRODUCT_PACKAGES += \
    liboemcrypto
#endif


ifeq ($(strip $(TARGET_USER_ZRAM)), true)
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/../sofia3gr/init.zram.rc:root/init.zram.rc\
	$(LOCAL_PATH)/../sofia3gr/fstab.zram:root/fstab.zram

endif

ifeq ($(strip $(TARGET_USER_KSM)), true)
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/init.ksm.rc:root/init.ksm.rc
endif


PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/init.recovery.sofiaboard.rc:root/init.recovery.sofiaboard.rc

PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/init.ptest.rc:root/init.ptest.rc

ifeq ($(BUILD_WITH_UMS),true)
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/../sofia3gr/fstab.sofiaboard_emmc-separate:root/fstab.sofiaboard_emmc \
        $(LOCAL_PATH)/../sofia3gr/fstab.sofiaboard_nand-separate:root/fstab.sofiaboard_nand
else
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/../sofia3gr//fstab.sofiaboard_emmc-merge:root/fstab.sofiaboard_emmc \
        $(LOCAL_PATH)/../sofia3gr//fstab.sofiaboard_nand-merge:root/fstab.sofiaboard_nand
endif

#gnss: copy init

#bluetooth: copy init
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/init.bluetooth.rc:root/init.bluetooth.rc

#call iwl wifi mk file
$(call inherit-product-if-exists, device/rockchip/common/wifi/iwl.mk)

#Copy FM Radio Rx firmware
PRODUCT_COPY_FILES += \
        hardware/intel/libfmrservice/firmware/fmr_rxmain.bin:system/vendor/firmware/fmr_rxmain.bin

#Copy FM Radio Rx configuration
PRODUCT_COPY_FILES += \
        hardware/intel/libfmrservice/firmware/fmr_rxnvm.bin:system/vendor/firmware/fmr_rxnvm.bin

# Key Character Map and Key Layout
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/vkp.kl:system/usr/keylayout/vkp.kl
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/vkp.kl:system/usr/keylayout/xgold-keypad.kl
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/vkp.kl:system/usr/keylayout/kpd-xgold.kl
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/vkp.kl:system/usr/keylayout/xgold-on-button.kl
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/vkp.kl:system/usr/keylayout/headset-xgold.kl
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/vkp.kcm:system/usr/keychars/vkp.kcm
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/vkp.kcm:system/usr/keychars/xgold-keypad.kcm
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/vkp.kcm:system/usr/keychars/kpd-xgold.kcm
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/vkp.kcm:system/usr/keychars/xgold-on-button.kcm
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/vkp.kcm:system/usr/keychars/headset-xgold.kcm
# Camera ISP tuing
ifeq ($(USE_CAMERA_HAL_3),true)
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/../common/tuning_cam/00ov8858.aiqb:system/etc/atomisp/00ov8858.aiqb $(LOCAL_PATH)/../common/tuning_cam/01ov2680.aiqb:system/etc/atomisp/01ov2680.aiqb
else
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_OV8858_cmk-cb0695-fv1.bin:system/etc/tuning_cam0.bin\
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_ov2680_cmk-cb0695-fv1.bin:system/etc/tuning_cam1.bin\
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_ov2680_cht835b.bin:system/etc/tuning_cam_ov2680_cht835b.bin\
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_ov2680_cmk-cb0695-fv1.bin:system/etc/tuning_cam_ov2680_cmk-cb0695-fv1.bin\
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_ov5648_cmk-cw2392-fv1.bin:system/etc/tuning_cam_ov5648_cmk-cw2392-fv1.bin\
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_ov5648_m5101.bin:system/etc/tuning_cam_ov5648_m5101.bin\
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_OV8858_cmk-cb0695-fv1.bin:system/etc/tuning_cam_OV8858_cmk-cb0695-fv1.bin\
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_OV8858_lg9569a2.bin:system/etc/tuning_cam_OV8858_lg9569a2.bin\
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_gc2355_hrx4769x5c211.bin:system/etc/tuning_cam_gc2355_hrx4769x5c211.bin\
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_gc2355_m206a-201.bin:system/etc/tuning_cam_gc2355_m206a-201.bin\
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_ov5648_cmk-cw1581-fv3.bin:system/etc/tuning_cam_ov5648_cmk-cw1581-fv3.bin\
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_ov5648_cmk-cw2611-fv3.bin:system/etc/tuning_cam_ov5648_cmk-cw2611-fv3.bin\
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_ov5648_m5001a.bin:system/etc/tuning_cam_ov5648_m5001a.bin\
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_OV8858_cmk-cb0832-fv3.bin:system/etc/tuning_cam_OV8858_cmk-cb0832-fv3.bin\
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_OV8858_lg40100a1.bin:system/etc/tuning_cam_OV8858_lg40100a1.bin \
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_sp2508_mb8-sp2008-sofia-3gr-v1.bin:system/etc/tuning_cam_sp2508_mb8-sp2008-sofia-3gr-v1.bin\
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_sp2508_m2002c-159.bin:system/etc/tuning_cam_sp2508_m2002c-159.bin\
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_gc2355_xhb-multi8-8qi.bin:system/etc/tuning_cam_gc2355_xhb-multi8-8qi.bin\
	$(LOCAL_PATH)/../common/tuning_cam/tuning_cam_gc2355_xc1269b-041.bin:system/etc/tuning_cam_gc2355_xc1269b-041.bin
endif

# Audio HAL policy
#PRODUCT_COPY_FILES += \
#	hardware/intel/libaudio_hal/audio_policy.conf:system/etc/audio_policy.conf



# media profiles and codecs files
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/media_profiles.xml:system/etc/media_profiles.xml


# camera profiles
ifeq ($(USE_CAMERA_HAL_3),true)
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/camera3_profiles.xml:system/etc/camera3_profiles.xml
else
PRODUCT_COPY_FILES += \
       $(LOCAL_PATH)/camera_profiles.xml:system/etc/camera_profiles.xml
endif
#ifneq ($(NON_IMC_BUILD), true)
# GNSS Configuration files
PRODUCT_COPY_FILES += \
	hardware/imc/gnss/lbs/config_files/LbsConfig.cfg:system/etc/LbsConfig.cfg
PRODUCT_COPY_FILES += \
	hardware/imc/gnss/lbs/config_files/LbsConfig_agnss.cfg:system/etc/LbsConfig_agnss.cfg
PRODUCT_COPY_FILES += \
	hardware/imc/gnss/lbs/config_files/LbsLogConfig.cfg:system/etc/LbsLogConfig.cfg
PRODUCT_COPY_FILES += \
	hardware/imc/gnss/lbs/config_files/LbsLogConfigPtest.cfg:system/etc/LbsLogConfigPtest.cfg
PRODUCT_COPY_FILES += \
	hardware/imc/gnss/lbs/config_files/LbsPltConfig.cfg:system/etc/LbsPltConfig.cfg
PRODUCT_COPY_FILES += \
    hardware/imc/gnss/lbs/lbsd.sh:system/bin/lbsd.sh
PRODUCT_COPY_FILES += \
    hardware/imc/gnss/lbs/lbsdPtest.sh:system/bin/lbsdPtest.sh

# thermal config files
ifeq ($(BOARD_THERMAL_PHONE),true)
  PRODUCT_COPY_FILES += \
         $(LOCAL_PATH)/../sofia3gr/thermal_sensor_config_phone.xml:system/etc/thermal_sensor_config.xml \
         $(LOCAL_PATH)/../sofia3gr/thermal_throttle_config.xml:system/etc/thermal_throttle_config.xml
else
  PRODUCT_COPY_FILES += \
         $(LOCAL_PATH)/../sofia3gr/thermal_sensor_config.xml:system/etc/thermal_sensor_config.xml \
         $(LOCAL_PATH)/../sofia3gr/thermal_throttle_config.xml:system/etc/thermal_throttle_config.xml
endif


#$(call inherit-product, frameworks/native/build/phone-hdpi-512-dalvik-heap.mk)

# default is nosdcard, S/W button enabled in resource
# PRODUCT_CHARACTERISTICS := nosdcard

#Pekall added for DSDS.
#Note the default build is DSDSifeq ($(strip $(DISABLE_MODEM)),false)
ifeq ($(strip $(DISABLE_MODEM)),false)
 ifeq ($(strip $(BUILD_DSDS)),true)
    ADDITIONAL_DEFAULT_PROPERTIES += \
    persist.tel.hot_swap.support=true \
    persist.ril-daemon.disable=dsds \
    persist.opt.network-camp=true \
    persist.radio.multisim.config=dsds
 else
    ADDITIONAL_DEFAULT_PROPERTIES += \
    persist.tel.hot_swap.support=true \
    persist.ril-daemon.disable=dsds \
    persist.opt.network-camp=true \
    persist.radio.multisim.config=false
 endif
 PRODUCT_PACKAGES += \
     Stk \
     librpc-ril \
     SimWidget

 # Cell Broadcast
 PRODUCT_PACKAGES += \
    CellBroadcastReceiver

PRODUCT_PROPERTY_OVERRIDES += \
        rild.libpath=/system/lib/librpc-ril.so
 # wifi only
else

ADDITIONAL_DEFAULT_PROPERTIES += \
   	persist.tel.hot_swap.support=false \
   	persist.ril-daemon.disable=1 \
   	persist.radio.multisim.config=false
PRODUCT_PROPERTY_OVERRIDES += \
        rild.libpath=/system/lib/libril-rk29-dataonly.so \
	rild.libargs=-d /dev/ttyACM0
endif
#Pekall added for FM.
PRODUCT_PACKAGES += \
    libfm_jni \
    libfmservice \
    pekallfmrserver \
    FMRadio



#Video codecs
PRODUCT_PACKAGES += \
    libvpu \
    libgralloc_priv_omx \
    libstagefrighthw \
    libRkOMX_Resourcemanager \
    librk_vpuapi \
    librk_hevcdec \
    librk_demux \
    librk_audio \
    libomxvpu_enc \
    libomxvpu_dec \
    libOMX_Core \
    libjpeghwenc \
    libjpeghwdec \
    libapedec \
    libhantro_omx_core \
    libOMX.hantro.G1.video.decoder \
    libOMX.hantro.H1.video.encoder
PRODUCT_PACKAGES += \
    libion

ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
PRODUCT_PACKAGES += \
    DeviceTest \
    StressTest \
    busybox
endif

    

########################################################
#	Houdini ARM for X86
ifeq ($(strip $(BUILD_ARM_FOR_X86)), true)
$(call inherit-product-if-exists, vendor/intel/houdini/houdini.mk)
endif

#copy ptest wifi/bt files
PRODUCT_COPY_FILES += \
         $(LOCAL_PATH)/../common/ptest/bin/phonetool_btapp:system/bin/phonetool_btapp \
	 $(LOCAL_PATH)/../common/ptest/bin/xvt_test:system/bin/xvt_test \
         $(LOCAL_PATH)/../common/ptest/lib/libbtuscplugin.so:system/lib/libbtuscplugin.so \
	 $(LOCAL_PATH)/../common/ptest/lib/libxvt_mos_actions.so:system/lib/libxvt_mos_actions.so \
	 $(LOCAL_PATH)/../common/ptest/lib/libxvt_rocs.so:system/lib/libxvt_rocs.so \
	 $(LOCAL_PATH)/../common/ptest/lib/libiwlwifiapi.so:system/lib/libiwlwifiapi.so \
	 $(LOCAL_PATH)/../common/ptest/lib/libnl-3.so:system/lib/libnl-3.so \
	 $(LOCAL_PATH)/../common/ptest/lib/libnl-genl-3.so:system/lib/libnl-genl-3.so \
	 $(LOCAL_PATH)/../common/ptest/etc/rocs/tinyxml2.h:system/etc/rocs/tinyxml2.h \
	$(LOCAL_PATH)/../common/ptest/etc/rocs/tinyxml2.cpp:system/etc/rocs/tinyxml2.cpp \
	$(LOCAL_PATH)/../common/ptest/etc/rocs/WLAN_SELF_CAL.ft:system/etc/rocs/WLAN_SELF_CAL.ft \
	$(LOCAL_PATH)/../common/ptest/etc/rocs/libxvt_rocs.cfg:system/etc/rocs/libxvt_rocs.cfg

# ----------------- BEGIN MIX-IN DEFINITIONS -----------------
# Mix-In definitions are auto-generated by mixin-update
##############################################################
# Source: device/intel/mixins/groups/debug-logs/true/product.mk
##############################################################
# Enable logs on file system for eng and userdebug builds
ifneq ($(TARGET_BUILD_VARIANT),user)
PRODUCT_COPY_FILES += device/rockchip/common/debug/init.logs.rc:root/init.logs.rc
PRODUCT_PACKAGES += \
    logcatext \
    start_log_srv.sh \
    logcat_ep.sh
endif
##############################################################
# Source: device/intel/mixins/groups/debug-crashlogd/true/product.mk
##############################################################
# Enable crashlogd for eng and userdebug builds
ifneq ($(TARGET_BUILD_VARIANT),user)
PRODUCT_COPY_FILES += device/rockchip/common/debug/init.crashlogd.rc:root/init.crashlogd.rc
PRODUCT_PACKAGES += crashlogd
endif
##############################################################
# Source: device/intel/mixins/groups/debug-coredump/true/product.mk
##############################################################
# Enable core dump on /data/logs/core for eng and userdebug builds
ifneq ($(TARGET_BUILD_VARIANT),user)
PRODUCT_COPY_FILES += device/rockchip/common/debug/init.coredump.rc:root/init.coredump.rc
endif












