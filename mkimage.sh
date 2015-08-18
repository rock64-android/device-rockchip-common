#!/bin/bash

set -e
#set -x

LOG=./.mkimagelog
rm -f $LOG

if [[ $1 =~ dtb ]]
then
DTB=$1
elif [[ $2 =~ dtb ]]
then
DTB=$2
else
DTB=SF_3GR-svb.dtb
fi

echo ""
echo "Usage: ./mkimage.sh [ list | help ]"
echo "   or: ./mkimage.sh [devicename.dtb] [ota]"
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

if [[ $1 == "list" || $1 == "help" ]]
then
exit
fi

DTB_RECOVERY=${DTB%.*}-recovery.dtb
if [ ! -f ./kernel/arch/x86/boot/dts/${DTB%.*}-recovery.dts ]
then
echo "$DTB_RECOVERY not found, now use default value."
DTB_RECOVERY=SF_3GR-es1-svb-rk-recovery.dtb
fi
echo "recovery_dtb : $DTB_RECOVERY"

echo  "setup environment ..."
. build/envsetup.sh >/dev/null && setpaths
if [ "$TARGET_PRODUCT" == "" ]
then
    echo -e "\033[32mcurrent TARGET_PRODUCT is null,please run lunch first! \033[m"
    exit
fi

echo -e "\033[32mcurrent TARGET_PRODUCT is:$TARGET_PRODUCT ,mkimage.sh will pack: $1 \033[m"
SYSTEMIMAGE_PARTITION_SIZE=`get_build_var BOARD_SYSTEMIMAGE_PARTITION_SIZE`
TARGET_PRODUCT=`get_build_var TARGET_PRODUCT`
BOARD_KERNEL_CMDLINE=`get_build_var BOARD_KERNEL_CMDLINE`
FEAT_POW_EMIF_MAX_DDR2_FREQ=`get_build_var FEAT_POW_EMIF_MAX_DDR2_FREQ`
FEAT_IS_PHONE=`get_build_var FEAT_IS_PHONE`
BAND_FEID=`get_build_var BAND_FEID`
FEAT_NO_MODEM=`get_build_var DISABLE_MODEM`
TARGET_BUILD_VARIANT=`get_build_var TARGET_BUILD_VARIANT`
echo -n "FEAT_POW_EMIF_MAX_DDR2_FREQ=$FEAT_POW_EMIF_MAX_DDR2_FREQ"
#echo cmdline=$BOARD_KERNEL_CMDLINE
echo "TARGET_BUILD_VARIANT:$TARGET_BUILD_VARIANT"
echo "done"

FLASHTOOLS=vendor/intel/tools/FlsTool
BINARYMERGE=vendor/intel/tools/binary_merge
SPLASH_IMG_GENERATE_TOOL=vendor/intel/tools/splash_img_generate
VBT_GENERATE_TOOLS=vendor/intel/tools/vbtgen

IMAGE_PATH=rockdev/Image-$TARGET_PRODUCT
rm -rf $IMAGE_PATH
mkdir -p $IMAGE_PATH
mkdir -p $IMAGE_PATH/calibration
mkdir -p $OUT/flashloader
rm -rf $OUT/secbin/temp
mkdir -p $OUT/secbin/temp

echo -n "create 2ndbootloader ..."
rm -f kernel/arch/x86/boot/dts/$DTB
rm -f kernel/arch/x86/boot/dts/$DTB_RECOVERY
cd kernel && make  $DTB > $LOG && make $DTB_RECOVERY > $LOG && cd - > $LOG
cp -fp kernel/arch/x86/boot/dts/$DTB $OUT/android.dtb
cp -fp kernel/arch/x86/boot/dts/$DTB_RECOVERY $OUT/recovery.dtb
echo "done"

echo -n "make modules to out folder ..."
mkdir -p $OUT/system/lib/modules
cp kernel/drivers/compat_iwlwifi/compat/compat.ko $OUT/system/lib/modules/
cp kernel/drivers/compat_iwlwifi/drivers/net/wireless/iwlwifi/xvt/iwlxvt.ko $OUT/system/lib/modules
cp kernel/drivers/compat_iwlwifi/net/mac80211/mac80211.ko $OUT/system/lib/modules
cp kernel/drivers/compat_iwlwifi/drivers/net/wireless/iwlwifi/iwlwifi.ko $OUT/system/lib/modules
#cp kernel/drivers/media/radio/intel_lnp/fmdrv_intel.ko $OUT/system/lib/modules
cp kernel/drivers/compat_iwlwifi/net/wireless/cfg80211.ko $OUT/system/lib/modules
cp kernel/drivers/compat_iwlwifi/drivers/net/wireless/iwlwifi/mvm/iwlmvm.ko $OUT/system/lib/modules
#cp kernel/drivers/idi/peripherals/btif_drv.ko $OUT/system/lib/modules
echo "done"

echo "ln nvmData"
rm -f $OUT/system/vendor/firmware/nvmData
ln -s /nvm_fs_partition/wlan/nvmData $OUT/system/vendor/firmware/nvmData
echo "done"
###################################################################
# pcba for ptest
PCBA_PATH=external/rk-pcba-test
echo "packaging pcba to system"

if [[ $TARGET_BUILD_VARIANT == "user" ]]; then
echo "It's USER mode......"
cp -f vendor/rockchip/common/bin/x86/busybox $OUT/system/bin/busybox
fi

cp -rf $PCBA_PATH/gui/basicTheme/*  $OUT/system/etc/
cp -rf $PCBA_PATH/res/test_config_sofia.cfg $OUT/system/etc/
cp -rf $PCBA_PATH/res/*.pcm $OUT/system/etc/
cp -rf $PCBA_PATH/res/*.wav $OUT/system/etc/
cp -rf $PCBA_PATH/res/audio_pcb $OUT/system/bin/
cp -rf $PCBA_PATH/res/alc $OUT/system/bin/
cp -rf $PCBA_PATH/res/*.sh $OUT/system/bin/
mv -f $OUT/system/bin/emmctester_sofia.sh $OUT/system/bin/emmctester.sh
mv -f $OUT/system/bin/mmctester_sofia.sh $OUT/system/bin/mmctester.sh
mv -f $OUT/system/bin/udisktester_sofia.sh $OUT/system/bin/udisktester.sh
mv -f $OUT/system/bin/wifi_sofia.sh $OUT/system/bin/wifi.sh
mv -f $OUT/system/etc/test_config_sofia.cfg $OUT/system/etc/test_config.cfg
mv -f $OUT/system/bin/gnsstester_sofia.sh $OUT/system/bin/gnsstester.sh
mv -f $OUT/system/bin/fmtester_sofia.sh $OUT/system/bin/fmtester.sh

echo "packaging pcba to system done"

###################################################################
#   out -> img
###################################################################
INSTALLED_BOOTIMAGE_TARGET=$OUT/boot.img
INSTALLED_SYSTEMIMAGE=$OUT/system.img

source device/rockchip/common/prg/prg.sh

PRG=$OUT/scatter_obj/modem_sw.prg

PSI_RAM=$OUT/flashloader/psi_ram.fls
EBL=$OUT/flashloader/ebl.fls


if [[ $FEAT_NO_MODEM == "true" ]]; then
BOOTCORE_PATH=vendor/intel/images/nomodem
else
BOOTCORE_PATH=vendor/intel/images
fi
echo "$BOOTCORE_PATH"
BAND_PATH=device/rockchip/common/rk_band

if [[ $FEAT_IS_PHONE == "true" ]]; then
echo "packaging phone fls......"
SLB_HEX_PATH=$BOOTCORE_PATH/phone_slb/
else
echo "packaging tablet fls......"
SLB_HEX_PATH=$BOOTCORE_PATH/hex
fi

if [ $FEAT_POW_EMIF_MAX_DDR2_FREQ -eq 312 ]; then
echo "packaging with DDR freq=$FEAT_POW_EMIF_MAX_DDR2_FREQ MHz"
BOOTCORE_PATH_PSI=vendor/intel/images/psi/psi_ddr312m
elif [ $FEAT_POW_EMIF_MAX_DDR2_FREQ -eq 400 ]; then
echo "packaging with DDR freq=$FEAT_POW_EMIF_MAX_DDR2_FREQ MHz"
BOOTCORE_PATH_PSI=vendor/intel/images/psi/psi_ddr400m
elif [ $FEAT_POW_EMIF_MAX_DDR2_FREQ -eq 455 ]; then
echo "packaging with DDR freq=$FEAT_POW_EMIF_MAX_DDR2_FREQ MHz"
BOOTCORE_PATH_PSI=vendor/intel/images/psi/psi_ddr455m
else
echo "packaging with DDR freq=$FEAT_POW_EMIF_MAX_DDR2_FREQ MHz"
BOOTCORE_PATH_PSI=vendor/intel/images/psi/psi_ddr533m
fi


if [ ! -f kernel/arch/x86/boot/vmlinux.bin ]
then
    echo "Please make kernel first !"
    exit
fi

#----- EBL & PSI_RAM -----#
echo -n "create ebl.fls ..."
$FLASHTOOLS --prg $PRG --output $OUT/flashloader/ebl.fls --meta $BOOTCORE_PATH/hex/ebl.version.txt --tag EBL $BOOTCORE_PATH/hex/ebl.hex --replace --to-fls2 > $LOG
echo "done"

echo -n "create psi_ram.fls ..."
$FLASHTOOLS --prg $PRG --output $OUT/flashloader/psi_ram.fls --script $BOOTCORE_PATH_PSI/scripts/psi_ram.xor_script.txt --meta $BOOTCORE_PATH_PSI/hex/psi_ram.version.txt --tag PSI_RAM $BOOTCORE_PATH_PSI/hex/psi_ram.hex --replace --to-fls2 > $LOG
echo "done"

echo -n "packing boot.img ..."
mkbootfs $OUT/root | minigzip > $OUT/ramdisk.img
cp -fp kernel/arch/x86/boot/vmlinux.bin $OUT/kernel
dd if=$OUT/kernel of=$OUT/kernel bs=8M count=1 conv=sync,nocreat,notrunc > $LOG 2>&1
dd if=$OUT/android.dtb of=$OUT/android.dtb  bs=512K count=1 conv=sync,nocreat,notrunc > $LOG 2>&1
mkbootimg --second $OUT/android.dtb --kernel $OUT/kernel --ramdisk $OUT/ramdisk.img --cmdline "$BOARD_KERNEL_CMDLINE" --base 0x0 --pagesize 4096 --kernel_offset 0x2600000 --ramdisk_offset 0x400000 --second_offset 0x2400010 --output $INSTALLED_BOOTIMAGE_TARGET  >> $LOG
echo "done"

echo -n "packing recovery.img ..."
mkbootfs $OUT/recovery/root | minigzip > $OUT/ramdisk-recovery.img
mkbootimg --second $OUT/recovery.dtb --kernel $OUT/kernel  --ramdisk $OUT/ramdisk-recovery.img --cmdline "$BOARD_KERNEL_CMDLINE" --base 0x0 --pagesize 4096 --kernel_offset 0x2600000 --ramdisk_offset 0x400000 --second_offset 0x2400010 --output $OUT/recovery.img > $LOG
echo "done"

echo -n "package modem ..."

$FLASHTOOLS --prg $PRG --output $IMAGE_PATH/modem.fls --tag MODEM_IMG --psi $PSI_RAM --ebl $EBL $BOOTCORE_PATH/hex/SF_3GR.ihex --replace --to-fls2 > $LOG 2>&1
$FLASHTOOLS -o $OUT/secbin/temp/modem -x $IMAGE_PATH/modem.fls > $LOG 2>&1
cp $OUT/secbin/temp/modem/modem.fls_ID0_CUST_LoadMap0.bin $OUT/system/vendor/firmware/modem.fls_ID0_CUST_LoadMap0.bin
cp $OUT/secbin/temp/modem/modem.fls_ID0_CUST_SecureBlock.bin $OUT/system/vendor/firmware/modem.fls_ID0_CUST_SecureBlock.bin
rm -f $IMAGE_PATH/modem.fls
echo "done"

echo -n "packing system.img ..."
make_ext4fs -s -S $OUT/root/file_contexts -l $SYSTEMIMAGE_PARTITION_SIZE -a system $INSTALLED_SYSTEMIMAGE $OUT/system > $LOG
echo "done"

###################################################################
#   img -> fls
###################################################################

echo -n "create psi_flash.fls ..."
$FLASHTOOLS --prg $PRG --output $IMAGE_PATH/psi_flash.fls --script $BOOTCORE_PATH_PSI/scripts/psi_flash.xor_script.txt --meta $BOOTCORE_PATH_PSI/hex/psi_flash.version.txt --tag PSI_FLASH $BOOTCORE_PATH_PSI/hex/psi_flash.hex --psi $PSI_RAM --ebl $EBL --replace --to-fls2 > $LOG
echo "done"

echo -n "create slb.fls ..."
$FLASHTOOLS --prg $PRG --output $IMAGE_PATH/slb.fls --meta $SLB_HEX_PATH/slb.version.txt --tag SLB $SLB_HEX_PATH/slb.hex --psi $PSI_RAM --ebl $EBL --replace --to-fls2 > $LOG
echo "done"

echo -n "create boot.fls ..."
$FLASHTOOLS --prg $PRG --output $IMAGE_PATH/boot.fls --tag BOOT_IMG --psi $PSI_RAM --ebl $EBL $OUT/boot.img --replace --to-fls2 > $LOG
echo "done"

echo -n "create system.fls ..."
$FLASHTOOLS --prg $PRG --output $IMAGE_PATH/system.fls --tag SYSTEM --psi $PSI_RAM --ebl $EBL $OUT/system.img --replace --to-fls2 > $LOG
echo "done"

echo -n "create recovery.fls ..."
#$FLASHTOOLS --prg=$PRG --output $IMAGE_PATH/recovery.fls --tag RECOVERY --psi $PSI_RAM --ebl $EBL $BOOTCORE_PATH/hex/mobilevisor.hex:2 $BOOTCORE_PATH/hex/mvconfig_smp.bin:1 $OUT/recovery.img:3 --replace --to-fls2 > $LOG
$FLASHTOOLS --prg=$PRG --output $IMAGE_PATH/recovery.fls --tag RECOVERY --psi $PSI_RAM --ebl $EBL $OUT/recovery.img --replace --to-fls2 > $LOG
echo "done"

echo -n "create userdata.fls ..."
$FLASHTOOLS --prg $PRG --output $IMAGE_PATH/userdata.fls --tag USERDATA --psi $PSI_RAM --ebl $EBL $OUT/userdata.img --replace --to-fls2 > $LOG
echo "done"

echo -n "create cache.fls ..."
$FLASHTOOLS --prg $PRG --output $IMAGE_PATH/cache.fls --tag CACHE --psi $PSI_RAM --ebl $EBL $OUT/cache.img --replace --to-fls2 > $LOG
echo "done"

echo -n "create ucode_patch.fls ..."
$FLASHTOOLS --prg $PRG --output $IMAGE_PATH/ucode_patch.fls --tag UC_PATCH $BOOTCORE_PATH/hex/ucode_patch.bin --psi $PSI_RAM --ebl $EBL --replace --to-fls2 > $LOG
echo "done"

echo -n "create nvm_fix_$BAND_FEID.fls ..."
$FLASHTOOLS --prg $PRG --output $IMAGE_PATH/nvm_fix_$BAND_FEID.fls --tag NVM_FIX $BAND_PATH/$BAND_FEID/nvm_fix_$BAND_FEID.bin --psi $PSI_RAM --ebl $EBL --replace --to-fls2 > $LOG
echo "done"

echo -n "create nvm_cal_$BAND_FEID.fls ..."
$FLASHTOOLS --prg $PRG --output $IMAGE_PATH/calibration/nvm_cal_$BAND_FEID.fls --tag NVM_CALI $BAND_PATH/$BAND_FEID/nvm_cal_$BAND_FEID.bin --psi $PSI_RAM --ebl $EBL --replace --to-fls2 > $LOG
echo "done"

echo "done"o -n "create splash_img.fls ..."


SPLASH_IMG_OUTPUT_DIR=$IMAGE_PATH
SPLASH_IMG_BIN_PATH=$BOOTCORE_PATH/hex
SPLASH_FILE_BIN_PATH=device/rockchip/$TARGET_PRODUCT/boot_logo
SPLASH_IMG_FILE_1=$SPLASH_FILE_BIN_PATH/splash_screen.jpg
SPLASH_IMG_FILE_2=$SPLASH_FILE_BIN_PATH/fastboot.jpg
SPLASH_DISPLAY_DTS=$SPLASH_FILE_BIN_PATH/splash_display_config.dts
SPLASH_IMG_DISPLAY_CONFIG=../vbt.bin
SPLASH_IMG_HEADER=$OUT/splash_hdr.bin
SPLASH_IMG_BIN_0=$OUT/splash_config.bin
SPLASH_IMG_BIN_1=$OUT/splash_screen.bin
SPLASH_IMG_BIN_2=$OUT/fastboot.bin
SPLASH_IMG_FLS=$SPLASH_IMG_OUTPUT_DIR/splash_img.fls
BINARY_MERGE_TOOLS=vendor/intel/tools/binary_merge

DISPLAY_BIN=$OUT/display.bin
DISPLAY_LOGO_SIZE=$OUT/logo_size_info.txt

if [ -f $DISPLAY_LOGO_SIZE ]
then
    rm $DISPLAY_LOGO_SIZE
fi

#chargeing logo picture
SPLASH_CHARGE_FILE_PATH=device/rockchip/$TARGET_PRODUCT/charge_logo/



#$VBT_GENERATE_TOOLS -i $SPLASH_DISPLAY_DTS -o $SPLASH_IMG_DISPLAY_CONFIG -splash $SPLASH_IMG_HEADER

echo "!SPLASH!" > $DISPLAY_LOGO_SIZE
convert $SPLASH_IMG_FILE_1 -depth 8 rgba:$SPLASH_IMG_BIN_1
identify $SPLASH_IMG_FILE_1 | cut -d' ' -f3 >> $DISPLAY_LOGO_SIZE
dd if=$SPLASH_IMG_BIN_1 of=$SPLASH_IMG_BIN_1 bs=512 count=1 conv=sync,nocreat,notrunc 

convert $SPLASH_IMG_FILE_2 -depth 8 rgba:$SPLASH_IMG_BIN_2
identify $SPLASH_IMG_FILE_2 | cut -d' ' -f3 >> $DISPLAY_LOGO_SIZE
dd if=$SPLASH_IMG_BIN_2 of=$SPLASH_IMG_BIN_2 bs=512 count=1 conv=sync,nocreat,notrunc

convert $SPLASH_CHARGE_FILE_PATH/battery_0.bmp -depth 8 rgba:$OUT/battery_0.bin
identify $SPLASH_CHARGE_FILE_PATH/battery_0.bmp | cut -d' ' -f3 >> $DISPLAY_LOGO_SIZE
dd if=$OUT/battery_0.bin of=$OUT/battery_0.bin bs=512 count=1 conv=sync,nocreat,notrunc

convert $SPLASH_CHARGE_FILE_PATH/battery_1.bmp -depth 8 rgba:$OUT/battery_1.bin
identify $SPLASH_CHARGE_FILE_PATH/battery_1.bmp | cut -d' ' -f3 >> $DISPLAY_LOGO_SIZE
dd if=$OUT/battery_1.bin of=$OUT/battery_1.bin bs=512 count=1 conv=sync,nocreat,notrunc

convert $SPLASH_CHARGE_FILE_PATH/battery_2.bmp -depth 8 rgba:$OUT/battery_2.bin
identify $SPLASH_CHARGE_FILE_PATH/battery_2.bmp | cut -d' ' -f3 >> $DISPLAY_LOGO_SIZE
dd if=$OUT/battery_2.bin of=$OUT/battery_2.bin bs=512 count=1 conv=sync,nocreat,notrunc

convert $SPLASH_CHARGE_FILE_PATH/battery_3.bmp -depth 8 rgba:$OUT/battery_3.bin
identify $SPLASH_CHARGE_FILE_PATH/battery_3.bmp | cut -d' ' -f3 >> $DISPLAY_LOGO_SIZE
dd if=$OUT/battery_3.bin of=$OUT/battery_3.bin bs=512 count=1 conv=sync,nocreat,notrunc

convert $SPLASH_CHARGE_FILE_PATH/battery_4.bmp -depth 8 rgba:$OUT/battery_4.bin
identify $SPLASH_CHARGE_FILE_PATH/battery_4.bmp | cut -d' ' -f3 >> $DISPLAY_LOGO_SIZE
dd if=$OUT/battery_4.bin of=$OUT/battery_4.bin bs=512 count=1 conv=sync,nocreat,notrunc

convert $SPLASH_CHARGE_FILE_PATH/battery_5.bmp -depth 8 rgba:$OUT/battery_5.bin
identify $SPLASH_CHARGE_FILE_PATH/battery_5.bmp | cut -d' ' -f3 >> $DISPLAY_LOGO_SIZE
dd if=$OUT/battery_5.bin of=$OUT/battery_5.bin bs=512 count=1 conv=sync,nocreat,notrunc

convert $SPLASH_CHARGE_FILE_PATH/battery_fail.bmp -depth 8 rgba:$OUT/battery_fail.bin
identify $SPLASH_CHARGE_FILE_PATH/battery_fail.bmp | cut -d' ' -f3 >> $DISPLAY_LOGO_SIZE
dd if=$OUT/battery_fail.bin of=$OUT/battery_fail.bin bs=512 count=1 conv=sync,nocreat,notrunc

cat $DISPLAY_LOGO_SIZE > $SPLASH_IMG_BIN_0
dd if=$SPLASH_IMG_BIN_0 of=$SPLASH_IMG_BIN_0 bs=512 count=1 conv=sync,nocreat,notrunc

$BINARY_MERGE_TOOLS -o $DISPLAY_BIN -b 512 -p 0 $SPLASH_IMG_BIN_0 $SPLASH_IMG_BIN_1 $SPLASH_IMG_BIN_2 $OUT/battery_0.bin $OUT/battery_1.bin $OUT/battery_2.bin $OUT/battery_3.bin $OUT/battery_4.bin $OUT/battery_5.bin $OUT/battery_fail.bin

$FLASHTOOLS --prg $PRG --output $IMAGE_PATH/splash_img.fls --tag SPLASH_SCRN --psi $PSI_RAM --ebl $EBL $DISPLAY_BIN --replace --to-fls2

echo "done"



echo -n "create secvm.fls ..."
$FLASHTOOLS --prg $PRG --output $IMAGE_PATH/secvm.fls --tag SECURE_VM --psi $PSI_RAM --ebl $EBL $BOOTCORE_PATH/hex/secvm.hex --replace --to-fls2 > $LOG
echo "done"

echo -n "create mobilevisor.fls ..."
$FLASHTOOLS --prg $PRG --output $IMAGE_PATH/mobilevisor.fls --tag MOBILEVISOR --psi $PSI_RAM --ebl $EBL $BOOTCORE_PATH/hex/mobilevisor.hex --replace --to-fls2 > $LOG
echo "done"

echo -n "create mvconfig_smp.fls ..."
$FLASHTOOLS --prg $PRG --output $IMAGE_PATH/mvconfig_smp.fls --tag MV_CONFIG --psi $PSI_RAM --ebl $EBL $BOOTCORE_PATH/hex/mvconfig_smp.bin --replace --to-fls2 > $LOG
echo "done"

echo -n "create misc.fls ..."
$FLASHTOOLS --prg $PRG --output $IMAGE_PATH/misc.fls --tag MISC --psi $PSI_RAM --ebl $EBL rkst/Image/misc.img --replace --to-fls2 > $LOG
echo "done"

echo -e "\033[32mAll Done. You can find them in '$IMAGE_PATH' \033[m"


#################################################
# OTA

if [[ $1 != 'ota' && $2 != 'ota' ]]
then
exit 0
fi

echo -n "package modem for fwu_image generation... "
$FLASHTOOLS --prg $PRG --output $IMAGE_PATH/modem.fls --tag MODEM_IMG --psi $PSI_RAM --ebl $EBL $BOOTCORE_PATH/hex/SF_3GR.ihex --replace --to-fls2 > $LOG 2>&1

rm -rf $OUT/fwu_image/
mkdir -p $OUT/fwu_image/temp

echo -n "generating fwu_image for OTA.... "
make -j24 fwu_image

rm -f $IMAGE_PATH/modem.fls
echo -n "generating final OTA package.... "
make -j24 otapackage
