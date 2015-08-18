// BEGIN CCCT SECTION
/* -------------------------------------------------------------------------
 Copyright (C) 2013-2014 Intel Mobile Communications GmbH
 
      Sec Class: Intel Confidential (IC)
 ----------------------------------------------------------------------
 Revision Information:
    $File name:  /mhw_drv_src/memory_management/mem/scatterfile/XMM6321_REV_1.0/mem_static_cfg.h $
 ----------------------------------------------------------------------
 by CCCT (0.12l)
 ---------------------------------------------------------------------- */
// END CCCT SECTION

#if !defined(MEM_STATIC_CFG_H)
#define MEM_STATIC_CFG_H

#if defined(MEMORY_MAP_HEADER_GEN_ENABLED)
#include "memory_map.h"
#else
#include "memory_map_def.h"
#endif

#define PLATFORM_PROJECTNAME                "SoFIA 3G"
#define MMAP_DESCRIPTION_CATEGORY           android

/* ToDo: use these to support for platforms where load and execution memories are different */
#if defined(EXPAND_LOAD_START_MACROS)
#include <load_addrs.lds>
#endif

#define MMAP_MODEM_SW_FLASH_CS              CS_NA
#define MMAP_MODEM_SW_FLASH_SIZE            (8192 MB)
#define MMAP_FLASH_BLOCK_SIZE               0x200

// IRAM Settings
#define MMAP_SW_IRAM_SIZE                   (0x20000)
#define MMAP_SW_IRAM_BASE                   (0xFFF00000)
#define MMAP_SW_ERAM_BASE                   (0x00000100) // For Bootloader placement purpose setting it to 0x0(actual base) + 0x100(random offset)

#define IRAM_DATA_SIZE                      (0x8800) //34 KB
#define IRAM_DATA_START                     (MMAP_MODEM_SW_IRAM_BASE)
#define IRAM_CODE_START                     (MMAP_MODEM_SW_IRAM_BASE + IRAM_DATA_SIZE)
#define IRAM_BOOTINFO_SECTION_BASE          (IRAM_DATA_START + IRAM_DATA_SIZE - 0x900)
#define IRAM_NOCLEARDATA_SECTION_BASE       (IRAM_DATA_START + IRAM_DATA_SIZE - 0x800)

#if !defined(MMAP_BCB_BASE)
#define MMAP_BCB_BASE                        0x00000000
#endif
#if !defined(MMAP_BCB_SIZE)
#define MMAP_BCB_SIZE                        0x800
#endif
#if defined(LOLLIPOP_PARTITION_LAYOUT)
#define BCB_PREVIOUS_BLOCK_PARTITION_ID PARTITION_ID_FW_UPDATE
#else
#define BCB_PREVIOUS_BLOCK_PARTITION_ID PARTITION_ID_COMBO_RECOVERY_MIRROR
#endif
// ERAM Settings
#define MMAP_MODEM_SW_EXTRAM_CS             CS0

#if defined(VMM_MOBILEVISOR)
  #define MMAP_MODEM_SW_EXTRAM_SIZE           (0x2C00000) //44MB
  #define EXTRAM_DATA_SIZE                    (0x1980000) //25.5MB
#else
  #define MMAP_MODEM_SW_EXTRAM_SIZE           (0x3600000) //54MB
  #define EXTRAM_DATA_SIZE                    (0x2380000) //36 MB
#endif

#define EXTRAM_MODEM_CODE_SIZE              (0x01280000) //18.5MB

/* Should be at the end of the Physical RAM always */
#define NO_OF_VCPU                          (1)
#define EXTRAM_VCPU_SHARED_MEM_SIZE         (0x8000*NO_OF_VCPU) //32KB


/* RO Regions */
#define MODEM_EXTRAM_BASE                   (MMAP_MODEM_SW_EXTRAM_BASE + MMAP_CODE_IMAGE_BASE)

#define MODEM_SW_RO_BASE                    (MODEM_EXTRAM_BASE)
#define MMAP_MODEM_SW_FI_BASE               (MMAP_MODEM_SW_STATIC_CFG_BASE + MMAP_MODEM_SW_STATIC_CFG_SIZE)

#define MMAP_FIRMWARE_IMAGE_BASE_ADDRESS    (MMAP_NVM_MIRROR_SECTION_BASE - MMAP_FIRMWARE_IMAGE_SIZE)
#if defined(FEAT_OPM_MEM_SECTION_FIRMWARE)
#define MMAP_FIRMWARE_IMAGE_SIZE            (0x80000) //512KB
#else
#define MMAP_FIRMWARE_IMAGE_SIZE            (0x0)
#endif

// Used only to allocate space in RAM. This is same as the space in flash if NVM_PARTITION_SEGGREGATION is disabled.
#define MMAP_NVM_MIRROR_SECTION_BASE        (MODEM_EXTRAM_BASE + EXTRAM_MODEM_CODE_SIZE - MMAP_NVM_MIRROR_SECTION_SIZE)
#define MMAP_NVM_MIRROR_SECTION_SIZE        (0x00180000) //1.5MB

#if defined(NVM_PARTITION_SEGGREGATION)
#define MMAP_NVM_STATIC_CALIB_BASE          (0x16)
#define MMAP_NVM_STATIC_CALIB_SIZE          (0x00100000) //1MB
#define MMAP_NVM_STATIC_FIX_BASE            (0x17)
#define MMAP_NVM_STATIC_FIX_SIZE            (0x00100000) //1MB
#define MMAP_NVM_DYNAMIC_BASE               (0x18)
#define MMAP_NVM_DYNAMIC_SIZE               (0x00100000) //1MB
#endif

#if defined(FEAT_OPM_MEM_SECTION_CERT) 
#if !defined(VMM_MOBILEVISOR)
#define MMAP_CERT_SECTION_BASE              (MMAP_FIRMWARE_IMAGE_BASE_ADDRESS - MMAP_CERT_SIZE) //Size is 0x40000, set in mem_static_internal.h
#else
#define MMAP_CERT_SECTION_BASE              (0x0000001B) //Size is 0x40000, set in mem_static_internal.h
#endif
#endif

#define MMAP_MODEM_SW_STATIC_CFG_SIZE       (0xA000) //40 KB
#define LOAD_MODEM_SW_RO_REGION             (MODEM_SW_RO_BASE)

/* RW Regions */
#define MODEM_SW_RW_BASE                    (MODEM_EXTRAM_BASE + EXTRAM_MODEM_CODE_SIZE)
#define MMAP_MODEM_SW_RW_LIMIT              (EXTRAM_DATA_SIZE)
#define MMAP_MODEM_SW_RW_SIZE               (MMAP_MODEM_SW_RW_LIMIT - MMAP_MODEM_SW_EXTRAM_PAGETABLE_SIZE)
#define EXTRAM_VCPU_SHARED_MEM_BASE         (MODEM_SW_RW_BASE + MMAP_MODEM_SW_RW_LIMIT - EXTRAM_VCPU_SHARED_MEM_SIZE) //32KB

/* ToDo: use bit masks for section attributes, no init attribute (bit 0), bss attribute (bit 1), early init (bit 2)  are used as of now */
#define XATTR_INIT_BITMASK                  (0x1)
#define XATTR_BSS_BITMASK                   (0x2)
#define XATTR_EARLY_INIT_BITMASK            (0x4)
#define XATTR_NOINIT                        (0)
#define XATTR_INIT                          (1)
#define DEFAULT_XATTR                       (0x1) /* used when explicit attributes are not used */
#define XATTR_BSS                           (2)
#define XATTR_NOBSS                         (0)
#define XATTR_EARLY_INIT                    (1)
#define XATTR_NOEARLY_INIT                  (0)

#if defined (FEAT_OPM_MEM_GPT_SUPPORT)
#if defined (FEAT_OPM_MEM_BLK_NUM_REL_EMMC_END)
 #define BOOT_PARTITION_SIZE_IN_BLOCKS          (0x0000FF)
 #define MEM_PRG_USABLE_FLASH_SIZE              (0x80000000)   /* MAXIMUM BLOCK NUMBER (-0x0) in signed magnitude number systems */ 
#else /* #if defined (FEAT_OPM_MEM_BLK_NUM_REL_EMMC_END) */
  #if defined(FEAT_OPM_MEM_BOOT_PART_SIZE_8K_BLKS) /* BOOT size partition is 0x2000 (blocks) for Loren-with Hynix memory */
    #define BOOT_PARTITION_SIZE_IN_BLOCKS          (0x2000)
  #else                                            /* BOOT size partition is 0x1000 (blocks) for A1, A2, Loren-with Samsung memory */
    #define BOOT_PARTITION_SIZE_IN_BLOCKS          (0x1000)
  #endif /* #if defined(FEAT_OPM_MEM_BOOT_PART_SIZE_8K_BLKS) */
  #if defined(_65536MBIT_FLASH) 
    #define MEM_PRG_USABLE_FLASH_SIZE             (0xE90000) /* in Blocks */
  #elif defined(_32768MBIT_FLASH)
    #define MEM_PRG_USABLE_FLASH_SIZE             (0x748000)
  #elif defined(_4096MBIT_FLASH)
    #define MEM_PRG_USABLE_FLASH_SIZE             (0xE9000)  
  #endif /*#if defined(_65536MBIT_FLASH) */
#endif /* #if defined (FEAT_OPM_MEM_BLK_NUM_REL_EMMC_END) */
#endif /* #if defined (FEAT_OPM_MEM_GPT_SUPPORT) */

#if defined(UCODE_PATCH_V2)
#define UCODE_PATCH_INPUT_PATH                    "../../../android/device/intel/sofia3g_xges2_0_ages2_svb/ucode_patch/ucode_patch.bin"
#elif defined(UCODE_PATCH_V1)
#define UCODE_PATCH_INPUT_PATH                    "../../../android/device/intel/sofia3g_xges1_1_ages2_svb/ucode_patch/ucode_patch.bin"
#else
/* No ucode patch needed */
#endif

#if defined(VMM_MOBILEVISOR)
// userdata size dummy map to overcome build errors. Prggen has to be adopted to remove this define
#define MMAP_USERDATA_SIZE                        (0x40000000)
#define ANDROID_OUTPUT_DIR                        "out/target/product/sofia3g_xges1_1_ages2_svb/fls"
#define ANDROID_OUTPUT_FLS                        FALSE
#define MVCONFIG_BIN_INPUT_PATH                   "out/target/product/sofia3g_xges1_1_ages2_svb/vmm_build/mvconfig.bin"
#define MOBILEVISOR_BIN_INPUT_PATH                "out/target/product/sofia3g_xges1_1_ages2_svb/vmm_build/mobilevisor/mobilevisor.bin"
#define SECURE_VM_BIN_INPUT_PATH                  "out/target/product/sofia3g_xges1_1_ages2_svb/secvm/secvm.bin"
#define BOOT_IMG_BIN_INPUT_PATH                   "out/target/product/sofia3g_xges1_1_ages2_svb/secvm/secvm.bin"

/* USERDATA is flexible partition, LINUX_NVM_FS,PT,PGPT,SGPT,BCB are in negative */
#if !defined(MEM_DYNAMIC_PARTITION_ID)
#define MEM_DYNAMIC_PARTITION_ID   0x45 //USERDATA  /* flexibile partition starts from here, anything below will be in negative partition */
#endif
#endif

#if !defined(BOOT_TYPE)
  #if defined(FEAT_FTLE_PRGGEN)
    #define BOOT_TYPE                            "EMMC_BOOT"
  #elif defined(FEAT_NAND_PRGGEN)                
    #define BOOT_TYPE                            "NAND_BOOT"
  #else
    #define BOOT_TYPE                            "NOR_BOOT"
  #endif
#endif  /* BOOT_TYPE */
#endif  /* MEM_STATIC_CFG_H */

/* -------------------------------------------------------------------------------------------------------- */
/* Below entries will result in the below: 
 *         1. Memory Map entry in PRG
 *         2. Partition in PRG
 *         3. Entry in createfls file for FLS generation only if the rule is present in mem_description_tags.h
 */
#if defined(MMAP_INCLUDE_MEMORYMAP) && defined(MMAP_INCLUDE_PARTITION)

/* Modem partition details addition */
#if !defined(VMM_MOBILEVISOR)
  MMAP_ADD_MEMORYMAP_ENTRY(CODE_IMAGE, MODEM_SW_RO_BASE, MMAP_CODE_IMAGE_SIZE)
#if defined(FEAT_OPM_MEM_SECTION_FIRMWARE)
  MMAP_ADD_MEMORYMAP_ENTRY(DSP_IMAGE, MMAP_FIRMWARE_IMAGE_BASE_ADDRESS, MMAP_FIRMWARE_IMAGE_SIZE)  
#endif
#if defined(FEAT_OPM_MEM_SECTION_NVM_MIRROR) && !defined(NVM_PARTITION_DISABLE) 
  MMAP_ADD_MEMORYMAP_ENTRY(NVM_STATIC_CALIB, MMAP_NVM_MIRROR_SECTION_BASE, MMAP_NVM_MIRROR_SECTION_SIZE)  
#endif /* #if defined(FEAT_OPM_MEM_SECTION_NVM_MIRROR) */
#if defined(FEAT_OPM_MEM_SECTION_CERT)
   MMAP_ADD_MEMORYMAP_ENTRY(CERTIFICATE_DATA, MMAP_CERT_SECTION_BASE, MMAP_CERT_SIZE)
#endif
#endif /* VMM_MOBILEVISOR */  

#if defined(VMM_MOBILEVISOR)
  #if defined(FEAT_OPM_MEM_SECTION_NVM_MIRROR)  && !defined(NVM_PARTITION_DISABLE) 
  #if defined(NVM_PARTITION_SEGGREGATION)
  MMAP_ADD_MEMORYMAP_ENTRY(NVM_STATIC_CALIB, MMAP_NVM_STATIC_CALIB_BASE, MMAP_NVM_STATIC_CALIB_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(NVM_STATIC_FIX, MMAP_NVM_STATIC_FIX_BASE, MMAP_NVM_STATIC_FIX_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(NVM_DYNAMIC, MMAP_NVM_DYNAMIC_BASE, MMAP_NVM_DYNAMIC_SIZE)
  #else
  MMAP_ADD_MEMORYMAP_ENTRY(NVM_STATIC_CALIB, MMAP_NVM_MIRROR_SECTION_BASE, MMAP_NVM_MIRROR_SECTION_SIZE)
  #endif
  #endif /* #if defined(FEAT_OPM_MEM_SECTION_NVM_MIRROR) */
  #if defined(MMAP_SECURITY_IMAGE_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(SECURITY_IMAGE , MMAP_SECURITY_IMAGE_BASE, MMAP_SECURITY_IMAGE_SIZE)
  #endif
  MMAP_ADD_MEMORYMAP_ENTRY(LINUX_NVM_FS, MMAP_LINUX_NVM_FS_BASE, MMAP_LINUX_NVM_FS_SIZE) 
  MMAP_ADD_MEMORYMAP_ENTRY(MOBILEVISOR_CONFIG, MMAP_MOBILEVISOR_CONFIG_BASE, MMAP_MOBILEVISOR_CONFIG_SIZE)
  /* NVM should be in Partition 3 of GPT in Android Build. So please dont change this order */
  MMAP_ADD_MEMORYMAP_ENTRY(CODE_IMAGE, MMAP_MOBILEVISOR_BASE, MMAP_MOBILEVISOR_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(SECURE_VM, MMAP_SECURE_VM_BASE, MMAP_SECURE_VM_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(BOOT_IMG, MMAP_BOOT_IMG_BASE, MMAP_BOOT_IMG_SIZE)
#if defined(LOLLIPOP_PARTITION_LAYOUT)
  MMAP_ADD_MEMORYMAP_ENTRY(RECOVERY, MMAP_RECOVERY_BASE, MMAP_RECOVERY_SIZE)
#endif  
  MMAP_ADD_MEMORYMAP_ENTRY(STARTUP_IMAGE, MMAP_SPLASH_SCREEN_BASE, MMAP_SPLASH_SCREEN_SIZE)
#if defined(LOLLIPOP_PARTITION_LAYOUT)
  MMAP_ADD_MEMORYMAP_ENTRY(FW_UPDATE, MMAP_FW_UPDATE_BASE, MMAP_FW_UPDATE_SIZE)
#else
  MMAP_ADD_MEMORYMAP_ENTRY(COMBO_RECOVERY, MMAP_COMBO_RECOVERY_BASE, MMAP_COMBO_RECOVERY_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(COMBO_RECOVERY_MIRROR, MMAP_COMBO_RECOVERY_MIRROR_BASE, MMAP_COMBO_RECOVERY_MIRROR_SIZE)
#endif
  #if defined(MMAP_USER_DISC_EXTEND4_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(USER_DISC_EXTEND4, MMAP_USER_DISC_EXTEND4_BASE, MMAP_USER_DISC_EXTEND4_SIZE) 
  #endif
  #if defined(MMAP_USER_DISC_EXTEND2_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(USER_DISC_EXTEND2, MMAP_USER_DISC_EXTEND2_BASE, MMAP_USER_DISC_EXTEND2_SIZE) 
  #endif
  #if defined(MMAP_USER_DISC_EXTEND3_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(USER_DISC_EXTEND3, MMAP_USER_DISC_EXTEND3_BASE, MMAP_USER_DISC_EXTEND3_SIZE) 
  #endif
  MMAP_ADD_MEMORYMAP_ENTRY(CACHE, MMAP_CACHE_BASE, MMAP_CACHE_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(SYSTEM, MMAP_SYSTEM_BASE, MMAP_SYSTEM_SIZE)
  // USERDATA  should be the last entry to ensure that all the free blocks gets allocated for this. So please add any new partitions above only.
  MMAP_ADD_MEMORYMAP_ENTRY(USERDATA, MMAP_USERDATA_BASE, MMAP_USERDATA_SIZE) 
#endif /* VMM Mobilevisor */
#endif /* defined(MMAP_INCLUDE_MEMORYMAP) && defined(MMAP_INCLUDE_PARTITION) */

/* -------------------------------------------------------------------------------------------------------- */
/* Below entries will result in the below: 
 *         1. Memory Map entry in PRG
 */
#if defined(MMAP_INCLUDE_MEMORYMAP) && !defined(MMAP_INCLUDE_PARTITION)
#if defined(VMM_MOBILEVISOR)
/* Memory Map entry needed for modem FLS - Here signed modem.fls generated will be packed in system image and no explicit parition needed for modem.fls.
Building the modem.fls needs this dummy memory map */
#if !defined(MMAP_CODE_IMAGE_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(MODEM_IMAGE, MODEM_SW_RO_BASE, 0x1C00000)
#endif
#if defined(FEAT_OPM_MEM_SINGLE_FILE_RECOVERY)
  MMAP_ADD_MEMORYMAP_ENTRY(CUSTOMIZATION_DISC_EXTEND1, MMAP_MOBILEVISOR_CONFIG_BASE, MMAP_MOBILEVISOR_CONFIG_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(CUSTOMIZATION_DISC_EXTEND2, MMAP_MOBILEVISOR_BASE, MMAP_MOBILEVISOR_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(CUSTOMIZATION_DISC_EXTEND3, MMAP_BOOT_IMG_BASE, MMAP_BOOT_IMG_SIZE)
#endif /* defined(FEAT_OPM_MEM_SINGLE_FILE_RECOVERY) */
#endif /* defined(VMM_MOBILEVISOR) */
  MMAP_ADD_MEMORYMAP_ENTRY(BCB, MMAP_BCB_BASE, MMAP_BCB_SIZE)
#endif /* defined(MMAP_INCLUDE_MEMORYMAP) */

/* -------------------------------------------------------------------------------------------------------- */
/* Below entries will result in the below: 
 *         MMAP_ADD_MEMORYMAP_ENTRY:
 *           1. Memory Map entry in PRG
 *           2. Partition in PRG
 *         MMAP_ADD_PARTITION:
 *           1. Partition in PRG
 */
#if !defined(MMAP_INCLUDE_MEMORYMAP) && defined(MMAP_INCLUDE_PARTITION)
  // USERDATA  should be the last entry to ensure that all the free blocks gets allocated for this. So please add any new partitions above only.
#endif  /* MMAP_INCLUDE_PARTITION */

/* -------------------------------------------------------------------------------------------------------- */
/* Below entries will update the Options in the Partitions */
#if defined(MEMORY_MAP_HEADER_GEN_ENABLED)
#include "memory_map_description.h"
#else
#include "memory_map_description_def.h"
#endif
#if defined(MMAP_INCLUDE_PARTITION_INFO)
  MMAP_UPDATE_PARTITION_INFO_OPT(SLB_IMAGE, ACCEPTABLE_BIT_ERRORS, FALSE)
  MMAP_UPDATE_PARTITION_INFO_OPT(SLB_IMAGE, ALIGNMENT, 0x0)
#endif

/* -------------------------------------------------------------------------------------------------------- */
