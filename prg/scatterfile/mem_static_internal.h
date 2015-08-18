/*  -------------------------------------------------------------------------
    Copyright (C) 2011-2014 Intel Mobile Communications GmbH
    
         Sec Class: Intel Confidential (IC)
    
    All rights reserved.
    -------------------------------------------------------------------------
    This document contains proprietary information belonging to IMC.
    Passing on and copying of this document, use and communication of its
    contents is not permitted without prior written authorization.
    -------------------------------------------------------------------------
    Revision Information:
       $File name:  /mhw_drv_src/memory_management/mem/scatterfile/mem_static_internal.h $
    ------------------------------------------------------------------------- */
#if !defined(MEM_STATIC_INTERNAL_H)
#define MEM_STATIC_INTERNAL_H

#if defined(DUAL_SIM_SUPPORT) /* backward compatibility */
  #define FEAT_MM_RWPI
#endif  /* DUAL_SIM_SUPPORT */

/* system configurable memory map */
#include <mem_static_cfg.h>
#include <mem_static_chip_cfg.h>

#define CAT_CS0(CS) MMAP_EBU_PORT_##CS##_BASE
#define CAT_CS(CS_NAME) CAT_CS0(CS_NAME)



#if 0
/* --------------------------------------------------------------------------------------------------------------------------------
   some error checks
   -------------------------------------------------------------------------------------------------------------------------------- */
#if (MMAP_MODEM_SW_RO_CODE_PNPI_SIZE + MMAP_MODEM_SW_RO_CODE_PPI_SIZE + MMAP_MODEM_SW_RO_CODE_BI_SIZE + MMAP_MODEM_SW_RO_CODE_FI_SIZE + \
    + MMAP_MODEM_SW_SIGNATURE_SIZE > MMAP_MODEM_SW_RO_SIZE)
  #error  Insufficient space for RO. Change MMAP_MODEM_SW_RO_SIZE or FI_SIZE, BI_SIZE, PPI_SIZE, PNPI_SIZE accordingly.
#endif

#if (MMAP_MODEM_SW_FLASH_BASE > 0)
  #if (MMAP_MODEM_SW_RO_SIZE > MMAP_MODEM_SW_FLASH_SIZE)
    #error  Insufficient flash size. Change MMAP_MODEM_SW_FLASH_SIZE or MMAP_MODEM_SW_RO_SIZE accordingly.
  #endif
#else
  #if (MMAP_MODEM_SW_RO_SIZE + MMAP_MODEM_SW_RW_SIZE > MMAP_MODEM_SW_EXTRAM_SIZE)
    #error  Insufficient RAM size. Change MMAP_MODEM_SW_EXTRAM_SIZE or MMAP_MODEM_SW_RO_SIZE or MMAP_MODEM_SW_RW_SIZE accordingly.
  #endif
#endif

#if (MMAP_MODEM_SW_RW_DATA_SIZE + MMAP_MODEM_SW_RW_NOCLR_SIZE + MMAP_MODEM_SW_RW_NOCACHE_SIZE > MMAP_MODEM_SW_RW_SIZE)
  #error  Insufficient RW size. Change MMAP_MODEM_SW_EXTRAM_SIZE, MMAP_MODEM_SW_RW_SIZE, MMAP_MODEM_SW_RW_NOCLR_SIZE, MMAP_MODEM_SW_RW_NOCACHE_SIZE accordingly.
#endif

#if (MMAP_MODEM_SW_EXTRAM_SIZE > 0 && MMAP_MODEM_SW_RW_SIZE > MMAP_MODEM_SW_EXTRAM_SIZE)
  #error  Insufficient RAM size. Change MMAP_MODEM_SW_EXTRAM_SIZE or MMAP_MODEM_SW_RW_SIZE accordingly.
#endif
#endif

/* --------------------------------------------------------------------------------------------------------------------------------
   macros
   -------------------------------------------------------------------------------------------------------------------------------- */
#if !defined(MMAP_ADD_BINARY_SECTION_EMPTY)
  #define MMAP_ADD_LOAD_REGION(_PRG_NAME_, _CLASS_, _REGION_, _BASE_, _SIZE_)
  #define MMAP_ADD_LOAD_REGION_EXTEND(_PRG_NAME_, _CLASS_, _REGION_, _SCF_ATTR_, _BASE_, _SIZE_)
  #define MMAP_ADD_BINARY_SECTION_EMPTY(_PRG_NAME_, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, _ATTR_, _DOMAIN_)
  #define MMAP_ADD_EXEC_SECTION(_SECTION_, _BASE_, _SIZE_)
  #define MMAP_ADD_EXEC_SECTION_EXTEND(_SECTION_, _SCF_ATTR_, _BASE_, _SIZE_)
  #define MMAP_ADD_BINARY_SECTION(_PRG_NAME_, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, _ATTR_, _DOMAIN_)
  #define MMAP_ADD_BINARY_SECTION_EXTEND(_PRG_NAME_, _PRG_CLASS_, _SECTION_, _BASE_, _SIZE_, _ATTR_, _DOMAIN_, _EXTEND_)
#endif  /* MMAP_ADD_BINARY_SECTION_EMPTY */

/* --------------------------------------------------------------------------------------------------------------------------------
   Base addresses and sizes that are calculated based on inputs
   -------------------------------------------------------------------------------------------------------------------------------- */
#define MMAP_MODEM_SW_EXTRAM_CS_BASE             CAT_CS(MMAP_MODEM_SW_EXTRAM_CS)
#define MMAP_MODEM_SW_FLASH_CS_BASE              CAT_CS(MMAP_MODEM_SW_FLASH_CS)

#define KB                                       * 1024                          
#define MB                                       * 1024 KB                       

#if defined(FEAT_MM_RWPI)
  #define EMPTY_FILE                             {empty_file}
  #define EMPTY_KEYWORD
#else
  #define EMPTY_FILE                             {}
  #define EMPTY_KEYWORD                          EMPTY
#endif  /* FEAT_MM_RWPI */

#define MMAP_MODEM_SW_IRAM_PAGETABLE_SIZE        0x00004800
/* 164 coarse page tables, + MMU_DATA size */
#define MMAP_MODEM_SW_EXTRAM_PAGETABLE_SIZE      (1 MB)
#define MMAP_MODEM_SW_IRAM_WAKEUP_DATA_SIZE      (12)

#if !defined(MMAP_CUST_MCU_SW_START_OFFSET)
#define MMAP_MODEM_SW_BOOT_VECT_SLB_SIZE         0x40000
#else
/* This define available if customer specific offset from flash is available */
#define MMAP_MODEM_SW_BOOT_VECT_SLB_SIZE         MMAP_CUST_MCU_SW_START_OFFSET
#endif
#define MMAP_MODEM_SW_BOOT_VECT_SIZE             0x14

#if !defined(CDS)                                
  #define MMAP_MODEM_SW_SIGNATURE_SIZE           0x400
#else                                            
  #define MMAP_MODEM_SW_SIGNATURE_SIZE           0
#endif                                           

/* If needed this can be configured in mem_static_cfg.h to required size */
#if !defined(MMAP_CERT_SIZE) 
#if defined(FEAT_OPM_MEM_SECTION_CERT)           
  #define MMAP_CERT_SIZE                         0x40000
#else                                                    
  #define MMAP_CERT_SIZE                         0
#endif
#endif

#define MMAP_MODEM_SW_RO_CODE_FI_SIZE            (64 KB)

#if !defined(CDS) && !defined(MSY)
  #if !defined(MMAP_CUST_MCU_SW_SIZE)
    #define MMAP_MODEM_SW_RO_SIZE                    (MMAP_MODEM_SW_BOOT_VECT_SLB_SIZE + MMAP_MODEM_SW_SIGNATURE_SIZE + MMAP_MODEM_SW_BOOT_VECT_SIZE + MMAP_MODEM_SW_STATIC_CFG_SIZE + MMAP_MODEM_SW_RO_CODE_FI_SIZE + MMAP_MODEM_SW_RO_NON_FI_SIZE)
  #else
/* Customer defined RO size */
    #if defined(FEAT_MM_RWPI)
      #define MMAP_MODEM_SW_RO_SIZE                MMAP_CUST_MCU_SW_SIZE -  MMAP_MODEM_SW_RWPI_LOAD_SIZE - MMAP_MODEM_SW_RWPI_VARS_LOAD_SIZE - MMAP_MODEM_SW_RW_LOAD_SIZE
    #else
      #define MMAP_MODEM_SW_RO_SIZE                MMAP_CUST_MCU_SW_SIZE - MMAP_MODEM_SW_RW_LOAD_SIZE
    #endif
  #endif
#endif

#if defined(MM_CUST_SCF_SUPPORT) && (defined(FEAT_NAND_PRGGEN)||(defined(FEAT_FTLE_PRGGEN)))
#define MMAP_MODEM_SW_RO_CODE_FREE_MEMORY_END  (OS_MEM_AREA_END) /* This is given for OS area in case of customer */
#else
#if !defined(NAND_ONLY) && !defined(FLASHLESS)
  #define MMAP_MODEM_SW_RO_CODE_FREE_MEMORY_END  (MMAP_MODEM_SW_EXTRAM_BASE + MMAP_MODEM_SW_EXTRAM_SIZE)
#else
  #define MMAP_MODEM_SW_RO_CODE_FREE_MEMORY_END  (MMAP_RO_EXTEND_BASE - MMAP_EEP_SECTION_SIZE - MMAP_CERT_SIZE - MMAP_FOTA_SECTION_SIZE - MMAP_CDS_SECTION_SIZE - MMAP_FFS_SECTION_SIZE - MMAP_CUST_SECTION_SIZE - MMAP_FIRMWARE_IMAGE_SIZE)
#endif
#endif

/* --------------------------------------------------------------------------------------------------------------------------------
   Define the memory layout
   -------------------------------------------------------------------------------------------------------------------------------- */
#if defined(CDS) || defined(MSY)
  #include <layout/mem_layout_cds.h>
#else
  #include <layout/mem_layout_modem.h>
#endif

/* --------------------------------------------------------------------------------------------------------------------------------
   Download element details
   -------------------------------------------------------------------------------------------------------------------------------- */
#if !defined(MMAP_MODEM_SW_RO_CODE_FI_TAG)
  #define MMAP_MODEM_SW_RO_CODE_FI_TAG           CODE_IMAGE
#endif

#if !defined(MMAP_MODEM_SW_RO_CODE_BI_TAG)
  #define MMAP_MODEM_SW_RO_CODE_BI_TAG           CODE_IMAGE
#endif

#if !defined(MMAP_MODEM_SW_RO_CODE_PPI_TAG)
  #define MMAP_MODEM_SW_RO_CODE_PPI_TAG          CODE_IMAGE
#endif

#if !defined(MMAP_MODEM_SW_RO_CODE_PNPI_TAG)
  #define MMAP_MODEM_SW_RO_CODE_PNPI_TAG         CODE_IMAGE
#endif

#if !defined(MMAP_MODEM_SW_STATIC_CFG_TAG)
  #define MMAP_MODEM_SW_STATIC_CFG_TAG           CODE_IMAGE
#endif

#if !defined(BOOT_TYPE)
  #if defined(FEAT_FTLE_PRGGEN)
    #define BOOT_TYPE                            "EMMC_OPEN_BOOT"
  #elif defined(FEAT_NAND_PRGGEN)                
    #define BOOT_TYPE                            "NAND_OPEN_BOOT"
  #else
    #define BOOT_TYPE                            "PCM_OPEN_BOOT"
  #endif
#endif  /* BOOT_TYPE */
#endif  /* MEM_STATIC_INTERNAL_H */
