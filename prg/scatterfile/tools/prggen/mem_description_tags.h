// BEGIN CCCT SECTION
/* -------------------------------------------------------------------------
 Copyright (C) 2013-2014 Intel Mobile Communications GmbH
 
      Sec Class: Intel Confidential (IC)
 ----------------------------------------------------------------------
 Revision Information:
    $File name:  /mhw_drv_src/memory_management/mem/scatterfile/tools/prggen/mem_description_tags.h $
 ----------------------------------------------------------------------
 by CCCT (0.12e)
 ---------------------------------------------------------------------- */
// END CCCT SECTION

// tags description file

#if defined(MMAP_INCLUDE_MEMORYMAP_INFO)
#define FEAT_PSI_FLASH_EXCLUDED
// declare tags first
#if defined(FEAT_NAND_PRGGEN) || defined(FEAT_FTLE_PRGGEN)
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(PSI_IMAGE, "PSI_FLASH:1#0")
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(SLB_IMAGE, "SLB:1#10")
#else
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(PSI_IMAGE, "CODE_IMAGE")
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(SLB_IMAGE, "CODE_IMAGE")
#endif

  MMAP_UPDATE_MEMORYMAP_INFO_TAG(BCB, "MISC:1#4A")
  
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(PSI_RAM, "PSI_RAM")
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(EBL, "EBL")
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(CODE_IMAGE, "CODE_IMAGE")
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(DSP_IMAGE, "DSP_IMAGE")
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(UA_IMAGE, "UA_IMAGE")
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(PAGING_IMAGE, "PAGING_IMAGE")
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(ROOT_DISC, "ROOT_DISC")
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(USER_DISC, "USER_DISC")
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(CUSTOMIZATION_DISC, "CUSTOMIZATION_DISC")
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(CUSTOMIZATION_DATA, "CUSTOMIZATION_DATA")
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(NVM_STATIC_CALIB, "NVM_CALI:1#16")  
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(NVM_DYNAMIC, "NVM_DYN:1#18")  
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(NVM_STATIC_FIX, "NVM_FIX:1#17")  
  MMAP_UPDATE_MEMORYMAP_INFO_TAG(CERTIFICATE_DATA, "CERT_DAT:1#1B")  



// then hex2fls rules
  MMAP_UPDATE_MEMORYMAP_INFO_TAG_H2F(
        PSI_RAM,
        FMT_HEX2FLS_TOOL FMT_PRGFILE FMT_VERSION FMT_SIGN_SCRIPT FMT_OUTPUT FMT_TAG FMT_INPUT_P,
        V_HEX2FLS_TOOL, V_PRGFILE("modem_sw"), V_VERSION_BOOT("psi_ram.version.txt"), V_SIGN_SCRIPT("psi_ram.xor_script.txt"), V_OUTPUT_BOOTID(PSI_RAM), V_TAG_AUTO(PSI_RAM), V_INPUT_BOOT("psi_ram.hex"))

  MMAP_UPDATE_MEMORYMAP_INFO_TAG_H2F(
        EBL,
        FMT_HEX2FLS_TOOL FMT_PRGFILE FMT_VERSION FMT_OUTPUT FMT_INJECT_PSIR FMT_TAG FMT_INPUT_P,
        V_HEX2FLS_TOOL, V_PRGFILE("modem_sw"), V_VERSION_BOOT("ebl.version.txt"), V_OUTPUT_BOOTID(EBL), V_INJECT_PSIR, V_TAG_AUTO(EBL), V_INPUT_BOOT("ebl.hex"))

#if defined(FEAT_PSI_FLASH_EXCLUDED)
  MMAP_UPDATE_MEMORYMAP_INFO_TAG_H2F(PSI_IMAGE,
                                     FMT_STANDARD FMT_VERSION FMT_SIGN_SCRIPT FMT_OUTPUT FMT_INPUT_P,
                                     V_STANDARD(PSI_IMAGE), V_VERSION_BOOT("psi_flash.version.txt"), V_SIGN_SCRIPT("psi_flash.xor_script.txt"), V_OUTPUT_ID(PSI_IMAGE), V_INPUT_BOOT("psi_flash.hex"))
#endif

#if !defined(FEAT_SLB_IN_PCM)
  MMAP_UPDATE_MEMORYMAP_INFO_TAG_H2F(SLB_IMAGE,
                                     FMT_STANDARD FMT_VERSION FMT_OUTPUT FMT_INPUT_P,
                                     V_STANDARD(SLB_IMAGE), V_VERSION_BOOT("slb.version.txt"), V_OUTPUT_ID(SLB_IMAGE), V_INPUT_BOOT("slb.hex"))
#endif

#if defined(VMM_MOBILEVISOR)
  MMAP_UPDATE_MEMORYMAP_INFO_TAG_H2F(MODEM_IMAGE,
#else
  MMAP_UPDATE_MEMORYMAP_INFO_TAG_H2F(CODE_IMAGE,
#endif  
                                     FMT_STANDARD FMT_STDIO
#if !defined(FEAT_PSI_FLASH_EXCLUDED)
                                     FMT_SIGN_SCRIPT
                                     FMT_INPUT_P
  #if defined(FEAT_SLB_IN_PCM)
                                     FMT_INPUT_P
  #endif  /* FEAT_SLB_IN_PCM */
#endif  /* FEAT_PSI_FLASH_EXCLUDED */
#if defined(VMM_MOBILEVISOR)
                                     ,V_STANDARD(MODEM_IMAGE), V_STDIO(MODEM_IMAGE)
#else
                                     ,V_STANDARD(CODE_IMAGE), V_STDIO(CODE_IMAGE)
#endif                                    
#if !defined(FEAT_PSI_FLASH_EXCLUDED)
                                     ,V_SIGN_SCRIPT("psi_flash.xor_script.txt")
                                     ,V_INPUT_BOOT("psi_flash.hex")
  #if defined(FEAT_SLB_IN_PCM)
                                     ,V_INPUT_BOOT("slb.hex")
  #endif
#endif  /* FEAT_PSI_FLASH_EXCLUDED */
                                    )

  MMAP_UPDATE_MEMORYMAP_INFO_TAG_H2F(UA_IMAGE,
                                     FMT_STANDARD FMT_STDIO,
                                     V_STANDARD(UA_IMAGE), V_STDIO(UA_IMAGE))
  MMAP_UPDATE_MEMORYMAP_INFO_TAG_H2F(DSP_IMAGE,
                                     FMT_STANDARD FMT_STDIO,
                                     V_STANDARD(DSP_IMAGE), V_STDIO(DSP_IMAGE))

#if defined(UCODE_PATCH_INPUT_PATH)
  MMAP_UPDATE_MEMORYMAP_INFO_TAG_H2F(UCODE_PATCH,
                                     FMT_NO_INJECT FMT_OUTPUT FMT_INPUT,
                                     V_NO_INJECT(UCODE_PATCH), V_OUTPUT_ID(UCODE_PATCH), UCODE_PATCH_INPUT_PATH)
#endif
                                     
  MMAP_UPDATE_MEMORYMAP_INFO_TAG_H2F(PAGING_IMAGE,
                                     FMT_STANDARD FMT_STDIO,
                                     V_STANDARD(PAGING_IMAGE), V_STDIO(PAGING_IMAGE))

#if defined(ROOT_DISK_BIN_INPUT_PATH)
  MMAP_UPDATE_MEMORYMAP_INFO_TAG_H2F(ROOT_DISC,
                                     FMT_STANDARD            FMT_OUTPUT              FMT_INPUT,
                                     V_STANDARD(ROOT_DISC),  ROOT_DISK_IMAGE_OUTPUT  ROOT_DISK_BIN_INPUT_PATH)
#endif  /* ROOT_DISK_BIN_INPUT_PATH */
#if defined(USER_DISK_BIN_INPUT_PATH)
  MMAP_UPDATE_MEMORYMAP_INFO_TAG_H2F(USER_DISC,
                                     FMT_STANDARD            FMT_OUTPUT              FMT_INPUT,
                                     V_STANDARD(USER_DISC),  USER_DISK_IMAGE_OUTPUT  USER_DISK_BIN_INPUT_PATH)
#endif  /* USER_DISK_BIN_INPUT_PATH */
#if defined(CUST_DISK_BIN_INPUT_PATH)
  MMAP_UPDATE_MEMORYMAP_INFO_TAG_H2F(CUSTOMIZATION_DISC,
                                     FMT_STANDARD                     FMT_OUTPUT              FMT_INPUT,
                                     V_STANDARD(CUSTOMIZATION_DISC),  CUST_DISK_IMAGE_OUTPUT  CUST_DISK_BIN_INPUT_PATH)
#endif  /* CUST_DISK_BIN_INPUT_PATH */
#if defined(CUST_PACK_BIN_INPUT_PATH)
  MMAP_UPDATE_MEMORYMAP_INFO_TAG_H2F(CUSTOMIZATION_DATA,
                                     FMT_STANDARD                     FMT_OUTPUT              FMT_INPUT,
                                     V_STANDARD(CUSTOMIZATION_DATA),  CUST_PACK_IMAGE_OUTPUT  CUST_PACK_BIN_INPUT_PATH)
#endif  /* CUST_PACK_BIN_INPUT_PATH */

#endif  /* MMAP_INCLUDE_MEMORYMAP_INFO */
