
FLSTOOL             := vendor/intel/tools/FlsTool
BINARY_MERGE_TOOL   := vendor/intel/tools/binary_merge
FWU_PACK_GENERATE_TOOL = vendor/intel/tools/fwpgen

#this is location where fls file is located
FLASHFILES_DIR      := rockdev/Image-sofia3gr

#this is the folder to hold anything about fwu_image generation
FWU_IMG_DIR        := $(PRODUCT_OUT)/fwu_image
FWU_EXTRA_TEMP		:= $(FWU_IMG_DIR)/temp


## Firmware update
FWU_IMAGE_BIN		:= $(FWU_IMG_DIR)/fwu_image.bin


INSTALLED_RADIOIMAGE_TARGET += $(FWU_IMAGE_BIN)

SECP_EXT := *.fls_ID0_*_SecureBlock.bin
DATA_EXT := *.fls_ID0_*_LoadMap*

define GEN_FIRMWARE_UPDATE_PACK_RULES
$(FWU_EXTRA_TEMP)/$(1):  $(FLASHFILES_DIR)/$(1).fls
	$(FLSTOOL) -o $(FWU_EXTRA_TEMP)/$(1) -x $(FLASHFILES_DIR)/$(1).fls
endef


#SYSTEM_FLS_LIST ?=
SYSTEM_FLS_LIST   += $(FLASHFILES_DIR)/boot.fls
SYSTEM_FLS_LIST   += $(FLASHFILES_DIR)/recovery.fls
SYSTEM_FLS_LIST   += $(FLASHFILES_DIR)/ucode_patch.fls
SYSTEM_FLS_LIST   += $(FLASHFILES_DIR)/secvm.fls
SYSTEM_FLS_LIST   += $(FLASHFILES_DIR)/mobilevisor.fls
SYSTEM_FLS_LIST   += $(FLASHFILES_DIR)/mvconfig_smp.fls
SYSTEM_FLS_LIST   += $(FLASHFILES_DIR)/splash_img.fls

#ANDROID_FLS_LIST ?=
ANDROID_FLS_LIST  += $(FLASHFILES_DIR)/system.fls
ANDROID_FLS_LIST  += $(FLASHFILES_DIR)/userdata.fls
ANDROID_FLS_LIST  += $(FLASHFILES_DIR)/cache.fls
ANDROID_FLS_LIST   += $(FLASHFILES_DIR)/modem.fls

PSI_FLS   = $(FLASHFILES_DIR)/psi_flash.fls
SLB_FLS   = $(FLASHFILES_DIR)/slb.fls

FWU_PACKAGE_LIST    = $(basename $(notdir $(PSI_FLS)))
FWU_PACKAGE_LIST 	+= $(basename $(notdir $(SLB_FLS)))
FWU_PACKAGE_LIST	+= $(basename $(notdir $(SYSTEM_FLS_LIST)))

FWU_PACKAGE_SECPACK_ONLY_LIST += $(basename $(notdir $(ANDROID_FLS_LIST)))

FWU_DEP_LIST := $(addprefix $(FWU_EXTRA_TEMP)/,$(FWU_PACKAGE_LIST))
$(foreach t,$(FWU_PACKAGE_LIST),$(eval $(call GEN_FIRMWARE_UPDATE_PACK_RULES,$(t))))

FWU_DEP_SECPACK_ONLY_LIST := $(addprefix $(FWU_EXTRA_TEMP)/,$(FWU_PACKAGE_SECPACK_ONLY_LIST))
$(foreach t,$(FWU_PACKAGE_SECPACK_ONLY_LIST),$(eval $(call GEN_FIRMWARE_UPDATE_PACK_RULES,$(t))))


.PHONY: fwu_image
fwu_image: $(FWU_DEP_LIST) $(FWU_DEP_SECPACK_ONLY_LIST) 
	@echo "---------- Generate fwu_image --------------------"
	$(foreach a, $(FWU_DEP_LIST), $(shell $(FWU_PACK_GENERATE_TOOL) --input $(FWU_IMAGE_BIN) --output $(FWU_IMAGE_BIN)_temp --secpack $(a)/$(SECP_EXT) --data $(a)/$(DATA_EXT) ; cp $(FWU_IMAGE_BIN)_temp $(FWU_IMAGE_BIN)))
	$(foreach a, $(FWU_DEP_SECPACK_ONLY_LIST), $(shell $(FWU_PACK_GENERATE_TOOL) --input $(FWU_IMAGE_BIN) --output $(FWU_IMAGE_BIN)_temp --secpack $(a)/$(SECP_EXT) ; cp $(FWU_IMAGE_BIN)_temp $(FWU_IMAGE_BIN)))
	@echo "---------- Generate fwu_image Done ---------------"
#$(foreach a, $(FWU_DEP_LIST), $(shell $(FWU_PACK_GENERATE_TOOL) --input $(FWU_IMAGE_BIN) --output $(FWU_IMAGE_BIN)_temp --secpack $(a)/$(SECP_EXT) --data $(a)/$(DATA_EXT) ; cp $(FWU_IMAGE_BIN)_temp $(FWU_IMAGE_BIN)))
#$(foreach a, $(FWU_DEP_SECPACK_ONLY_LIST), $(shell $(FWU_PACK_GENERATE_TOOL) --input $(FWU_IMAGE_BIN) --output $(FWU_IMAGE_BIN)_temp --secpack $(a)/$(SECP_EXT) ; cp $(FWU_IMAGE_BIN)_temp $(FWU_IMAGE_BIN)))



