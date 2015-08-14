#!/bin/bash

set -e
set -x

LOG=./.mkimagelog
rm -f $LOG
DTB=SF_3GR-svb.dtb

if [[ $1 =~ "dtb" ]]
then
DTB=$1
fi

echo ""
echo "Usage: ./mkotapackage.sh [ list | help ]"
echo "   or: ./mkotapackage.sh xxx.dtb"
echo ""
echo ---------------------- DTS list ------------------------
DTSLIST=`ls ./kernel/arch/x86/boot/dts/*.dts`
for dtsfile in $DTSLIST
do
	dtsname=`basename $dtsfile`
	dtbname=${dtsname%.*}.dtb
	
	if ! [[ $dtbname =~ "recovery" ]]
	then	
		echo -n "$dtbname"
	else
		continue
	fi
	if [ $dtbname == $DTB ]
	then
		echo -e "\t\t *** current ***"
	else
		echo ""
	fi
done
echo --------------------------------------------------------

DTB_RECOVERY=${DTB%.*}-recovery.dtb
if [ ! -f ./kernel/arch/x86/boot/dts/${DTB%.*}-recovery.dts ]
then
echo "$DTB_RECOVERY not found, now use default value."
DTB_RECOVERY=SF_3GR-svb-recovery.dtb
fi
echo "recovery_dtb : $DTB_RECOVERY"

echo -n "setup environment ..."
. build/envsetup.sh >/dev/null && setpaths
TARGET_PRODUCT=`get_build_var TARGET_PRODUCT`
BOARD_KERNEL_CMDLINE=`get_build_var BOARD_KERNEL_CMDLINE`
BOARD_KERNEL_BASE=`get_build_var BOARD_KERNEL_BASE`
BOARD_KERNEL_PAGESIZE=`get_build_var BOARD_KERNEL_PAGESIZE`
BUILD_NUMBER=`get_build_var BUILD_NUMBER`
USER=`get_build_var USER`
TARGET_RELEASETOOLS_EXTENSIONS=`get_build_var TARGET_RELEASETOOLS_EXTENSIONS`
BOARD_FLASH_BLOCK_SIZE=`get_build_var BOARD_FLASH_BLOCK_SIZE`
BOARD_BOOTIMAGE_PARTITION_SIZE=`get_build_var BOARD_BOOTIMAGE_PARTITION_SIZE`
BOARD_SYSTEMIMAGE_PARTITION_SIZE=`get_build_var BOARD_SYSTEMIMAGE_PARTITION_SIZE`
BOARD_RECOVERYIMAGE_PARTITION_SIZE=`get_build_var BOARD_RECOVERYIMAGE_PARTITION_SIZE`
BOARD_USERDATAIMAGE_PARTITION_SIZE=`get_build_var BOARD_USERDATAIMAGE_PARTITION_SIZE`
BOARD_CACHEIMAGE_PARTITION_SIZE=`get_build_var BOARD_CACHEIMAGE_PARTITION_SIZE`

DEFAULT_TARGET_RECOVERY_FSTYPE_MOUNT_OPTIONS=`get_build_var DEFAULT_TARGET_RECOVERY_FSTYPE_MOUNT_OPTIONS`
BOARD_MKBOOTIMG_ARGS=`get_build_var BOARD_MKBOOTIMG_ARGS`

if [[ $BUILD_NUMBER =~ "eng" ]]
then
  FILE_NAME_TAG=eng.$USER
else
  FILE_NAME_TAG=$BUILD_NUMBER
fi

######  VARS  ########
HOST_OUT_EXECUTABLES=$OUT/obj/EXECUTABLES
SELINUX_FC=$OUT/file_contexts
zip_root=$OUT/obj/PACKAGING/target_files_intermediates/$TARGET_PRODUCT

#### END OF VARS ####
echo "done"

#echo -n "make apkcerts-list ..."
#make apkcerts-list -j32 
#echo "done"

INTERNAL_OTA_PACKAGE_TARGET=$OUT/$TARGET_PRODUCT.zip
echo $INTERNAL_OTA_PACKAGE_TARGET

KEY_CERT_PAIR=build/target/product/security/testkey.x509.pem
rm -f $OUT/etc/security/otacerts.zip
mkdir -p $OUT/etc/security/
zip	-qj $OUT/etc/security/otacerts.zip $KEY_CERT_PAIR

BUILT_TARGET_FILES_PACKAGE=$OUT/obj/PACKAGING/target_files_intermediates/$TARGET_PRODUCT.zip

echo "Package target files ..."
rm -rf $BUILT_TARGET_FILES_PACKAGE $zip_root

mkdir -p $zip_root
mkdir -p $zip_root/RECOVERY
mkdir -p $zip_root/RECOVERY/RAMDISK
cp -rf $OUT/recovery/root/* $zip_root/RECOVERY/RAMDISK
cp -f  kernel/arch/x86/boot/vmlinux.bin $zip_root/RECOVERY/kernel
cp -f  $OUT/$DTB_RECOVERY $zip_root/RECOVERY/second
echo "$BOARD_KERNEL_CMDLINE" > $zip_root/RECOVERY/cmdline
echo "$BOARD_KERNEL_BASE" > $zip_root/RECOVERY/base
echo "$BOARD_KERNEL_PAGESIZE" > $zip_root/RECOVERY/pagesize

mkdir -p $zip_root/BOOT
mkdir -p $zip_root/BOOT/RAMDISK
cp -rf $OUT/root/* $zip_root/BOOT/RAMDISK
cp -f kernel/arch/x86/boot/vmlinux.bin	$zip_root/BOOT/kernel
cp -f $OUT/$DTB $zip_root/BOOT/second
echo "$BOARD_KERNEL_CMDLINE" > $zip_root/BOOT/cmdline
echo "$BOARD_KERNEL_BASE" > $zip_root/BOOT/base
echo "$BOARD_KERNEL_PAGESIZE" > $zip_root/BOOT/pagesize

#INSTALLED_RADIOIMAGE_TARGET=`get_build_var INSTALLED_RADIOIMAGE_TARGET`
#echo $INSTALLED_RADIOIMAGE_TARGET
mkdir -p $zip_root/SYSTEM
cp -rf $OUT/system/* $zip_root/SYSTEM

mkdir -p $zip_root/DATA
cp -rf $OUT/data/* $zip_root/DATA

mkdir -p $zip_root/VENDOR
test -d $OUT/vendor && cp -rf $OUT/vendor/* $zip_root/VENDOR

mkdir -p $zip_root/OTA/bin
cp -f $OUT/android-info.txt $zip_root/OTA/
cp -f $OUT/obj/EXECUTABLES/applypatch_intermediates/applypatch $zip_root/OTA/bin/
test -e $OUT/obj/EXECUTABLES/applypatch_static_intermediates/applypatch_static && cp -f $OUT/obj/EXECUTABLES/applypatch_static_intermediates/applypatch_static $zip_root/OTA/bin/
test -e $OUT/obj/EXECUTABLES/check_prereq_intermediates/check_prereq && cp -f $OUT/obj/EXECUTABLES/check_prereq_intermediates/check_prereq $zip_root/OTA/bin/
cp -f $OUT/obj/EXECUTABLES/sqlite3_intermediates/sqlite3 $zip_root/OTA/bin/
cp -f $OUT/obj/EXECUTABLES/updater_intermediates/updater $zip_root/OTA/bin/

mkdir -p $zip_root/META
mkdir -p $OUT/obj/PACKAGING/apkcerts_intermediates
cp -f $OUT/obj/PACKAGING/apkcerts_intermediates/$TARGET_PRODUCT-apkcerts-$FILE_NAME_TAG.txt $zip_root/META/apkcerts.txt
cp -f device/rockchip/sofia3gr/releasetools.py  $zip_root/META/
##$(hide) echo "$(PRODUCT_OTA_PUBLIC_KEYS)" > $(zip_root)/META/otakeys.txt
PRIVATE_RECOVERY_API_VERSION=3
PRIVATE_RECOVERY_FSTAB_VERSION=2
echo "recovery_api_version=$PRIVATE_RECOVERY_API_VERSION" > $zip_root/META/misc_info.txt
echo "fstab_version=$PRIVATE_RECOVERY_FSTAB_VERSION" >> $zip_root/META/misc_info.txt
echo "blocksize=$BOARD_FLASH_BLOCK_SIZE" >> $zip_root/META/misc_info.txt
echo "boot_size=$BOARD_BOOTIMAGE_PARTITION_SIZE" >> $zip_root/META/misc_info.txt
echo "recovery_size=$BOARD_RECOVERYIMAGE_PARTITION_SIZE" >> $zip_root/META/misc_info.txt
echo "recovery_mount_options=$DEFAULT_TARGET_RECOVERY_FSTYPE_MOUNT_OPTIONS" >> $zip_root/META/misc_info.txt
echo "tool_extensions=device/rockchip/sofia3gr/releasetools.py" >> $zip_root/META/misc_info.txt
DEFAULT_SYSTEM_DEV_CERTIFICATE=build/target/product/security/testkey
echo "default_system_dev_certificate=$DEFAULT_SYSTEM_DEV_CERTIFICATE" >> $zip_root/META/misc_info.txt
##$(hide) echo "extra_recovery_keys=$(PRODUCT_EXTRA_RECOVERY_KEYS)" >> $(zip_root)/META/misc_info.txt
echo 'mkbootimg_args=$BOARD_MKBOOTIMG_ARGS' >> $zip_root/META/misc_info.txt
echo "use_set_metadata=1" >> $zip_root/META/misc_info.txt
echo "multistage_support=1" >> $zip_root/META/misc_info.txt
echo "update_rename_support=1" >> $zip_root/META/misc_info.txt
##$(hide) echo "oem_fingerprint_properties=$(OEM_THUMBPRINT_PROPERTIES)" >> $(zip_root)/META/misc_info.txt

INTERNAL_USERIMAGES_EXT_VARIANT=ext4
BOARD_USERDATAIMAGE_FILE_SYSTEM_TYPE=ext4
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE=ext4
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE=ext4
echo "fs_type=$INTERNAL_USERIMAGES_EXT_VARIANT" >> $zip_root/META/misc_info.txt
echo "system_size=$BOARD_SYSTEMIMAGE_PARTITION_SIZE" >> $zip_root/META/misc_info.txt
echo "userdata_fs_type=$BOARD_USERDATAIMAGE_FILE_SYSTEM_TYPE" >> $zip_root/META/misc_info.txt
echo "userdata_size=$BOARD_USERDATAIMAGE_PARTITION_SIZE" >> $zip_root/META/misc_info.txt
echo "cache_fs_type=$BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE" >> $zip_root/META/misc_info.txt
echo "cache_size=$BOARD_CACHEIMAGE_PARTITION_SIZE" >> $zip_root/META/misc_info.txt
echo "vendor_fs_type=$BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE" >> $zip_root/META/misc_info.txt
echo "vendor_size=$BOARD_VENDORIMAGE_PARTITION_SIZE" >> $zip_root/META/misc_info.txt
echo "oem_size=$BOARD_OEMIMAGE_PARTITION_SIZE" >> $zip_root/META/misc_info.txt
#echo "extfs_sparse_flag=$(INTERNAL_USERIMAGES_SPARSE_EXT_FLAG)" >>
#echo "mkyaffs2_extra_flags=$(mkyaffs2_extra_flags)" >>
echo "selinux_fc=$OUT/file_contexts" >> $zip_root/META/misc_info.txt
#$(if $(PRODUCTS.$(INTERNAL_PRODUCT).PRODUCT_SUPPORTS_VERITY),$(hide) echo "verity=$(PRODUCTS.$(INTERNAL_PRODUCT).PRODUCT_SUPPORTS_VERITY)" >> $(1))
#$(if $(PRODUCTS.$(INTERNAL_PRODUCT).PRODUCT_SUPPORTS_VERITY),$(hide) echo "verity_key=$(PRODUCTS.$(INTERNAL_PRODUCT).PRODUCT_VERITY_SIGNING_KEY)" >> $(1))
#$(if $(PRODUCTS.$(INTERNAL_PRODUCT).PRODUCT_SUPPORTS_VERITY),$(hide) echo "verity_signer_cmd=$(VERITY_SIGNER)" >> $(1))
#$(if $(PRODUCTS.$(INTERNAL_PRODUCT).PRODUCT_SYSTEM_VERITY_PARTITION),$(hide) echo "system_verity_block_device=$(PRODUCTS.$(INTERNAL_PRODUCT).PRODUCT_SYSTEM_VERITY_PARTITION)" >> $(1))
#$(if $(PRODUCTS.$(INTERNAL_PRODUCT).PRODUCT_VENDOR_VERITY_PARTITION),$(hide) echo "vendor_verity_block_device=$(PRODUCTS.$(INTERNAL_PRODUCT).PRODUCT_VENDOR_VERITY_PARTITION)" >> $(1))
#$(if $(2),$(hide) $(foreach kv,$(2),echo "$(kv)" >> $(1);))

./build/tools/releasetools/make_recovery_patch $zip_root $zip_root
cd $zip_root && zip -qry ../$BUILT_TARGET_FILES_PACKAGE .
zipinfo -1 $BUILT_TARGET_FILES_PACKAGE | awk 'BEGIN { FS="SYSTEM/" } /^SYSTEM\// {print "system/" $$2}' | $HOST_OUT_EXECUTABLES/fs_config -C -S $SELINUX_FC > $zip_root/META/filesystem_config.txt
zipinfo -1 $BUILT_TARGET_FILES_PACKAGE | awk 'BEGIN { FS="VENDOR/" } /^VENDOR\// {print "vendor/" $$2}' | $HOST_OUT_EXECUTABLES/fs_config -C -S $SELINUX_FC > $zip_root/META/vendor_filesystem_config.txt
zipinfo -1 $BUILT_TARGET_FILES_PACKAGE | awk 'BEGIN { FS="BOOT/RAMDISK/" } /^BOOT\/RAMDISK\// {print $$2}' | $HOST_OUT_EXECUTABLES/fs_config -C -S $SELINUX_FC > $zip_root/META/boot_filesystem_config.txt
zipinfo -1 $BUILT_TARGET_FILES_PACKAGE | awk 'BEGIN { FS="RECOVERY/RAMDISK/" } /^RECOVERY\/RAMDISK\// {print $$2}' | $HOST_OUT_EXECUTABLES/fs_config -C -S $SELINUX_FC > $zip_root/META/recovery_filesystem_config.txt


cd $zip_root && zip -q ../$BUILT_TARGET_FILES_PACKAGE META/*filesystem_config.txt
./build/tools/releasetools/add_img_to_target_files -p $OUT $BUILT_TARGET_FILES_PACKAGE

