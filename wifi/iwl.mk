# Add iwl relvent copy files and defs

PRODUCT_PACKAGES += \
    libiwlwifiapi \
    libiconv \
    xvt_test \
    wlan-ipcservice \
    wpa_supplicant \
    hostapd

#iwlwifi: copy init and loading scripts to root
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/init.wifi.normal.rc:root/init.wifi.normal.rc

#iwlwifi: copy init.wifi.ptest.rc to root directory.
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/init.wifi.ptest.rc:root/init.wifi.ptest.rc

PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/load_iwlwifi.sh:system/bin/load_iwlwifi.sh

#copy wlan_nvm_init.sh to system/etc/
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/wlan_nvm_init.sh:system/etc/wifi/wlan_nvm_init.sh

#copy iwlwifi wpa p2p config files
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/p2p_supplicant.conf:system/etc/wifi/p2p_supplicant.conf \
        $(LOCAL_PATH)/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf \
        $(LOCAL_PATH)/rftest:system/bin/rftest \
        frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
        frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml

#call iwl firmware mk file
$(call inherit-product-if-exists, hardware/imc/wlan/iwl-firmware/iwl-fw-a620-sofia.mk)

