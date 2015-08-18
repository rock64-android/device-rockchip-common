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
       $File name:  /mhw_drv_src/memory_management/mem/scatterfile/mem_partition_cfg.h $
    ------------------------------------------------------------------------- */

// #include "nvm_cfg.h"

#define K       * 1024
#define M       * 1024 K

#if defined(FEAT_NAND_PRGGEN)

/* All the macro configs in this file can be overwritten by configuring them in mem_static_cfg.h
 * If ROOT_DISC_TWO is needed in PRG, ROOT_DISC_TWO_IN_PRG macro has to be set in mem_static_cfg.h */
#if !defined(ROOT_DISC_TWO_LENGTH)
#define ROOT_DISC_TWO_LENGTH 0
#endif
#if defined(ROOT_DISC_TWO_INCLUSION_IN_PRG)
MMAP_ADD_PARTITION(ROOT_DISC_TWO, 0x20000000,                     ROOT_DISC_TWO_LENGTH /* sizeof(PARTITION_ID_ROOT_DISC_TWO) */)
#endif

/* New changes */
#if !defined(FTL_LENGTH)
#define FTL_LENGTH 1638 K
#endif
MMAP_ADD_PARTITION(FTL_STARTUP_DISC, 0,                     FTL_LENGTH)

#if defined(FEAT_OPM_MEM_SECTION_NVM)
MMAP_ADD_PARTITION(DYN_SDS_IMAGE,    MMAP_EEP_SECTION_BASE, 0)
#endif

#if !defined(BBT_LENGTH)
#define BBT_LENGTH 630 K
#endif
#if PRGGEN_VERSION == 1
MMAP_ADD_PARTITION(BAD_BLOCK_TABLE,  0,                     BBT_LENGTH)
#endif

#if !defined(NVM_LOAD_ADDR_IN_PRG)
#define NVM_STATIC_CAL_ADDR 0
#define NVM_STATIC_FIX_ADDR 0
#define NVM_STATIC_DYN_ADDR 0
#endif

#if !defined(NVM_STATIC_CAL_LENGTH)
#define NVM_STATIC_CAL_LENGTH 882 K
#define NVM_STATIC_FIX_LENGTH 882 K
#define NVM_STATIC_DYN_LENGTH 882 K	
#endif

MMAP_ADD_PARTITION(NVM_STATIC_CALIB, NVM_STATIC_CAL_ADDR,                     NVM_STATIC_CAL_LENGTH /* sizeof(T_NVM_STATIC_CAL) */)
MMAP_ADD_PARTITION(NVM_STATIC_FIX,   NVM_STATIC_FIX_ADDR,                     NVM_STATIC_FIX_LENGTH /* sizeof(T_NVM_STATIC_FIX) */)
MMAP_ADD_PARTITION(NVM_DYNAMIC,      NVM_STATIC_DYN_ADDR,                     NVM_STATIC_DYN_LENGTH /* sizeof(T_NVM_STATIC_FIX) */)

#else  /* FEAT_NAND_PRGGEN */

#if !defined(FEAT_OPM_MEM_SECTION_NVM_MIRROR)
#if defined(MM_CUST_SCF_SUPPORT)
MMAP_ADD_PARTITION(NVM_STATIC_CALIB, 0,                     882 K /* sizeof(T_NVM_STATIC_CAL) */)
MMAP_ADD_PARTITION(NVM_STATIC_FIX,   0,                     882 K /* sizeof(T_NVM_STATIC_FIX) */)
MMAP_ADD_PARTITION(NVM_DYNAMIC,      0,                     882 K /* sizeof(T_NVM_STATIC_FIX) */)
#else   /* MM_CUST_SCF_SUPPORT */
MMAP_ADD_PARTITION(NVM_STATIC_CALIB, 0,                     756 K /* sizeof(T_NVM_STATIC_CAL) */)
MMAP_ADD_PARTITION(NVM_STATIC_FIX,   0,                     756 K /* sizeof(T_NVM_STATIC_FIX) */)
MMAP_ADD_PARTITION(NVM_DYNAMIC,      0,                     756 K /* sizeof(T_NVM_STATIC_FIX) */)
#endif  /* MM_CUST_SCF_SUPPORT */
#endif  /* FEAT_OPM_MEM_SECTION_NVM_MIRROR */

#endif //FEAT_NAND_PRGGEN


#if defined(FEAT_FTLE_PRGGEN)
  #if defined(FEAT_OPM_MEM_GPT_SUPPORT)
    #if defined(FEAT_OPM_MEM_BLK_NUM_REL_EMMC_END) 
      MMAP_ADD_PARTITION(BCB,                 0,                 2 K)
      MMAP_ADD_PARTITION(PARTITION_TABLE,     0,                 2 K)
      MMAP_ADD_PARTITION(PRIMARY_GPT,         0,                 17 K)
      MMAP_ADD_PARTITION(SECONDARY_GPT,       0,                 16896)
    #else
      MMAP_ADD_PARTITION(PARTITION_TABLE,     0,                 2 K)
      MMAP_ADD_PARTITION(PRIMARY_GPT,         0,                 17 K)
      MMAP_ADD_PARTITION(SECONDARY_GPT,       0,                 16896)
      #if defined(_65536MBIT_FLASH)
       // MMAP_ADD_PARTITION(USER_DISC,         0,                 4095 M)
      #elif defined(_32768MBIT_FLASH)
       // MMAP_ADD_PARTITION(USER_DISC,           0,                 1445 M)
      #endif
      MMAP_ADD_PARTITION(BCB,                 0,                 2 K)
   #endif
  #else
  #if defined(MM_CUST_SCF_SUPPORT)
    MMAP_ADD_PARTITION(PARTITION_TABLE,     0x01800000,                 2 K)
  #else
    MMAP_ADD_PARTITION(PARTITION_TABLE,     0x0,                 512 K)
  #endif
  #endif /* FEAT_OPM_MEM_GPT_SUPPORT */
#endif   /* FEAT_FTLE_PRGGEN */
	
#if defined(FEAT_NAND_PRGGEN)	
#if PRGGEN_VERSION == 2
MMAP_ADD_PARTITION(BAD_BLOCK_TABLE,  0,                     BBT_LENGTH)
#endif
MMAP_ADD_PARTITION(PARTITION_TABLE,  0x01800000,                     126 K)	
#if !defined(RAW_NAND_IMAGE_LENGTH)
#define RAW_NAND_IMAGE_LENGTH 252 K
#endif
MMAP_ADD_PARTITION(RAW_NAND_IMAGE,   0x38000000,                     RAW_NAND_IMAGE_LENGTH)

/* This condition is added to remove the DYN_SDS_IMAGE inclusion from PRG. This has to be checked and removed if not needed 
 * In mem_static_cfg.h this DYN_SDS_IMAGE_REMOVAL_IN_PRG has to be defined to remove it from PRG */
#if !defined(DYN_SDS_IMAGE_REMOVAL_IN_PRG)	
MMAP_ADD_PARTITION(DYN_SDS_IMAGE,    0x80080000,                     0)	
#endif

/* Since the latest Lepton Memory Budget Excel sheet does not contain Preflash in the Flash layout and also there is an open question on whether this is needed for Lepton, plan is to remove this for Lepton. Another UTP shall be raised to correct the entries in the Memory map for Lepton and this shall be added back then.*/
#if !defined(MM_CUST_SCF_SUPPORT)	
MMAP_ADD_PARTITION(PREFLASH,         0x30000000,                     0)	
#endif	
#endif

