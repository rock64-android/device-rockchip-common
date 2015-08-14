#!/system/bin/sh
echo "Entering WLAN NVM Partition"

DEFAULT_NVM_FILE="/system/etc/wifi/nvmDataDefault"
NVM_FILE="/nvm_fs_partition/wlan/nvmData"

if [ ! -d /nvm_fs_partition/wlan/ ]
then
	echo "making directory /nvm_fs_partition/wlan/"
	mkdir -p /nvm_fs_partition/wlan/
fi

chown wifi:wifi /nvm_fs_partition/wlan
chmod 0670 /nvm_fs_partition/wlan

if [ -f "${NVM_FILE}" ]
then
	echo "NVM file present in production partition"
else
	echo "NVM file Not present in production partition"
	cp "${DEFAULT_NVM_FILE}" "${NVM_FILE}"
fi

chown wifi:wifi "${NVM_FILE}"
chmod 0670 "${NVM_FILE}"

if [ ! -e /system/vendor/firmware/nvmData ]; then
	#mount -o rw,remount /system
	#ln -s /nvm_fs_partition/wlan/nvmData /system/vendor/firmware/nvmData
	#mount -o ro,remount /system
fi
