/*  -------------------------------------------------------------------------
    Copyright (C) 2011-2013 Intel Mobile Communications GmbH
    
         Sec Class: Intel Confidential (IC)
    
    All rights reserved.
    -------------------------------------------------------------------------
    This document contains proprietary information belonging to IMC.
    Passing on and copying of this document, use and communication of its
    contents is not permitted without prior written authorization.
    -------------------------------------------------------------------------
    Revision Information:
       $File name:  /mhw_drv_src/memory_management/mem/scatterfile/layout/mem_layout_modem.h $
    ------------------------------------------------------------------------- */

  #define MMAP_MODEM_SW_FLASH_BASE               MMAP_MODEM_SW_FLASH_CS_BASE
  #define MMAP_MODEM_SW_EXTRAM_BASE              MMAP_MODEM_SW_EXTRAM_CS_BASE

/* --------------------------------------------------------------------------------------------------------------------------------
   Memory Layout - RW Data in External RAM
   -------------------------------------------------------------------------------------------------------------------------------- */
  #define MMAP_MODEM_SW_EXTRAM_PAGETABLE_BASE    MMAP_MODEM_SW_EXTRAM_BASE
  #define MMAP_MODEM_SW_RW_BASE                  (MMAP_MODEM_SW_EXTRAM_PAGETABLE_BASE + MMAP_MODEM_SW_EXTRAM_PAGETABLE_SIZE)

/* --------------------------------------------------------------------------------------------------------------------------------
   Memory Layout - RO Code/Data
   -------------------------------------------------------------------------------------------------------------------------------- */
#if MMAP_MODEM_SW_FLASH_BASE > 0
#if !defined(FEAT_SLB_IN_PCM)
  /* NOR/PCM flash configuration */
  #define MMAP_MODEM_SW_RO_BASE                  MMAP_MODEM_SW_FLASH_BASE
#else
  #define MMAP_MODEM_SW_RO_BASE                  MMAP_MODEM_SW_FLASH_BASE + MMAP_MODEM_SW_BOOT_VECT_SLB_SIZE
#endif
#else
  /* NAND/eMMC flash configuration */
#if defined(__x86)
  // In x86 based projects, Code is currently placed at the base of External RAM.
  #define MMAP_MODEM_SW_RO_BASE                  (MODEM_EXTRAM_BASE)
#else
  #define MMAP_MODEM_SW_RO_BASE                  (MMAP_MODEM_SW_EXTRAM_BASE + MMAP_MODEM_SW_RW_LIMIT)
#endif
#endif  /* MMAP_MODEM_SW_FLASH_BASE */

#if !defined(FEAT_SLB_IN_PCM)
  #if !defined(MM_BOOT_LDR_IMAGE_HDR_AT_START)
    #define MMAP_MODEM_SW_RO_SIGNATURE_BASE          (MMAP_MODEM_SW_RO_BASE + MMAP_MODEM_SW_BOOT_VECT_SLB_SIZE)
  #else
    #define MMAP_MODEM_SW_RO_SIGNATURE_BASE          (MMAP_MODEM_SW_RO_BASE)
    #define MMAP_MODEM_SW_DSLB_BASE                  (MMAP_MODEM_SW_RO_BASE + MMAP_MODEM_SW_SIGNATURE_SIZE)
  #endif
#else
  #define MMAP_MODEM_SW_RO_SIGNATURE_BASE          (MMAP_MODEM_SW_RO_BASE)
#endif

#if !defined(MM_BOOT_LDR_IMAGE_HDR_AT_START)
  #define MMAP_MODEM_SW_BOOT_VECT_BASE             (MMAP_MODEM_SW_RO_SIGNATURE_BASE + MMAP_MODEM_SW_SIGNATURE_SIZE)
  #define MMAP_MODEM_SW_STATIC_CFG_BASE            (MMAP_MODEM_SW_BOOT_VECT_BASE + MMAP_MODEM_SW_BOOT_VECT_SIZE)
  #define MMAP_MODEM_SW_SYS_VER_BASE               (MMAP_MODEM_SW_RO_SIGNATURE_BASE + 0x1000)
#else
  #define MMAP_MODEM_SW_BOOT_VECT_BASE             (MMAP_MODEM_SW_RO_BASE + MMAP_MODEM_SW_BOOT_VECT_SLB_SIZE + MMAP_MODEM_SW_SIGNATURE_SIZE)
  #define MMAP_MODEM_SW_STATIC_CFG_BASE            (MMAP_MODEM_SW_BOOT_VECT_BASE + MMAP_MODEM_SW_BOOT_VECT_SIZE)
  #define MMAP_MODEM_SW_SYS_VER_BASE               (MMAP_MODEM_SW_BOOT_VECT_BASE + 0x1000)
#endif

#define MMAP_MODEM_SW_RO_CODE_FI_BASE            (MMAP_MODEM_SW_STATIC_CFG_BASE + MMAP_MODEM_SW_STATIC_CFG_SIZE)

#if defined(RELOCATE_CODE_TO_RAM)
  #define MMAP_MODEM_SW_RO_CODE_BI_BASE          (MMAP_MODEM_SW_RW_BASE + MMAP_MODEM_SW_RW_SIZE)
#else
  #define MMAP_MODEM_SW_RO_CODE_BI_BASE          (MMAP_MODEM_SW_RO_CODE_FI_BASE + MMAP_MODEM_SW_RO_CODE_FI_SIZE)
#endif  /* RELOCATE_CODE_TO_RAM */

/* if paging is enabled, PPI & PNPI shall be placed outside the physical RAM address space */
#if defined(DEMAND_PAGING_ENABLED)
  #define MMAP_MODEM_SW_RO_CODE_PPI_BASE         (MMAP_MODEM_SW_EXTRAM_BASE + MMAP_MODEM_SW_EXTRAM_SIZE)
#else
  #define MMAP_MODEM_SW_RO_CODE_PPI_BASE         +0
#endif  /* DEMAND_PAGING_ENABLED */

/* --------------------------------------------------------------------------------------------------------------------------------
   Memory Layout - RW Data Load area
   -------------------------------------------------------------------------------------------------------------------------------- */
/* if RWPI is enabled, setup the load address for the same */
#if defined(FEAT_MM_RWPI)
  #define MMAP_MODEM_SW_RWPI_LOAD_BASE           (MMAP_MODEM_SW_RO_BASE + MMAP_MODEM_SW_RO_SIZE)
  #define MMAP_MODEM_SW_RWPI_VARS_LOAD_BASE      (MMAP_MODEM_SW_RWPI_LOAD_BASE + MMAP_MODEM_SW_RWPI_LOAD_SIZE)
  #define MMAP_MODEM_SW_RW_LOAD_BASE             (MMAP_MODEM_SW_RWPI_VARS_LOAD_BASE + MMAP_MODEM_SW_RWPI_VARS_LOAD_SIZE)
#else  /* FEAT_MM_RWPI */
  #define MMAP_MODEM_SW_RW_LOAD_BASE             (MMAP_MODEM_SW_RO_BASE + MMAP_MODEM_SW_RO_SIZE)
#endif

#if MMAP_MODEM_SW_FLASH_BASE > 0
  #if defined(RELOCATE_CODE_TO_RAM)
    #define MMAP_MODEM_SW_EXTRAM_END             (MMAP_MODEM_SW_EXTRAM_BASE + MMAP_MODEM_SW_RW_LIMIT + MMAP_MODEM_SW_RO_NON_FI_SIZE)
  #else
    #define MMAP_MODEM_SW_EXTRAM_END             (MMAP_MODEM_SW_EXTRAM_BASE + MMAP_MODEM_SW_RW_LIMIT)
  #endif
#else
  #define MMAP_MODEM_SW_EXTRAM_END               (MMAP_MODEM_SW_RW_LOAD_BASE + MMAP_MODEM_SW_RW_LOAD_SIZE)
  
#endif

#define MMAP_MODEM_SW_IRAM_DATA_BASE             (MMAP_MODEM_SW_IRAM_BASE + 0x8000) /* 0x8000 is allocated for Code in IRAM. Rest is all data */
#define MMAP_MODEM_SW_IRAM_PAGETABLE_BASE        MMAP_MODEM_SW_IRAM_DATA_BASE
#define MODEM_SW_PREINIT_BASE                    (MMAP_MODEM_SW_IRAM_WAKEUP_DATA_ADDRESS + MMAP_MODEM_SW_IRAM_WAKEUP_DATA_SIZE)

#define MMAP_DUMMY_BASE                          (0x00000000)

/* --------------------------------------------------------------------------------------------------------------------------------
   Base addresses for all other binary data, grows in opposite direction
   -------------------------------------------------------------------------------------------------------------------------------- */
#if MMAP_MODEM_SW_FLASH_BASE > 0
  #define MMAP_RO_EXTEND_BASE                    (MMAP_MODEM_SW_FLASH_BASE + MMAP_MODEM_SW_FLASH_SIZE - MMAP_FLASH_BLOCK_SIZE)
#else
  #define MMAP_RO_EXTEND_BASE                    (MMAP_MODEM_SW_EXTRAM_BASE + MMAP_MODEM_SW_EXTRAM_SIZE)
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

/* --------------------------------------------------------------------------------------------------------------------------------
   Fixed Flash defination
   -------------------------------------------------------------------------------------------------------------------------------- */
#if defined(RELOCATE_CODE_TO_RAM)
  #define FIXED_FLASH
#else
  #define FIXED_FLASH                            FIXED
#endif  /* RELOCATE_CODE_TO_RAM */
