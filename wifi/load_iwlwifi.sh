wlan_nvm=/system/vendor/firmware/nvmData

if [ $1 == "--ptest-boot" ]; then
IS_PTEST_BOOT="true"
fi

if [ $IS_PTEST_BOOT == "true" ]; then
	insmod system/lib/modules/compat.ko
	insmod system/lib/modules/cfg80211.ko
	insmod system/lib/modules/mac80211.ko
	
	j=0
        while [ $j -lt 5 ];
        do
            if [ -e $wlan_nvm ]; then
                insmod system/lib/modules/iwlwifi.ko nvm_file=nvmData xvt_default_mode=1
                break
            fi
            j=$((j+1))
            sleep 1
        done
        if [ ! -e $wlan_nvm ]; then
            insmod system/lib/modules/iwlwifi.ko nvm_file=nvmDataDefault xvt_default_mode=1
        fi
	
	insmod system/lib/modules/iwlxvt.ko
else
	insmod system/lib/modules/compat.ko
	insmod system/lib/modules/cfg80211.ko
	insmod system/lib/modules/mac80211.ko
	j=0
	while [ $j -lt 5 ];
	do
	    if [ -e $wlan_nvm ]; then
	        insmod system/lib/modules/iwlwifi.ko nvm_file=nvmData
	        break
	    fi
	    j=$((j+1))
	    sleep 1
	done 
	if [ ! -e $wlan_nvm ]; then
	    insmod system/lib/modules/iwlwifi.ko nvm_file=nvmDataDefault
	fi    
	insmod system/lib/modules/iwlmvm.ko
	# set driver wifi driver property so hal will know that it is loaded
	setprop wlan.driver.status "ok"
fi
