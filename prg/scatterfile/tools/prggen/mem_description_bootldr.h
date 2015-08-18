// BEGIN CCCT SECTION
/* -------------------------------------------------------------------------
 Copyright (C) 2013 Intel Mobile Communications GmbH
 
      Sec Class: Intel Confidential (IC)
 ----------------------------------------------------------------------
 Revision Information:
    $File name:  /mhw_drv_src/memory_management/mem/scatterfile/tools/prggen/mem_description_bootldr.h $
 ----------------------------------------------------------------------
 by CCCT (0.12e)
 ---------------------------------------------------------------------- */
// END CCCT SECTION

// bootloader memory map description
  MMAP_ADD_MEMORYMAP_ENTRY(PSI_RAM, MMAP_PSI_RAM_BASE, MMAP_PSI_RAM_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(EBL, MMAP_EBL_BASE, MMAP_EBL_SIZE)

#if defined(FEAT_NAND_PRGGEN) || defined(FEAT_FTLE_PRGGEN)
  MMAP_ADD_MEMORYMAP_ENTRY(PSI_IMAGE, MMAP_PSI_FLASH_BASE, MMAP_PSI_FLASH_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(UCODE_PATCH, MMAP_UCODE_PATCH_BASE, MMAP_UCODE_PATCH_SIZE)
#if defined(MMAP_BOOTROM_PATCH_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(BOOTROM_PATCH, MMAP_BOOTROM_PATCH_BASE, MMAP_BOOTROM_PATCH_SIZE)
#endif
#if defined(VRL_PARTITION_ENABLED) && defined(MMAP_VRL_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(VRL, MMAP_VRL_BASE, MMAP_VRL_SIZE)
#endif
  #if defined(FEAT_OPM_MEM_SECTION_CERT)
  MMAP_ADD_MEMORYMAP_ENTRY(CERTIFICATE_DATA, MMAP_CERT_SECTION_BASE, MMAP_CERT_SIZE)
  #endif
#if defined(MMAP_PSI_MIRROR_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(PSI_MIRROR, MMAP_PSI_MIRROR_BASE, MMAP_PSI_MIRROR_SIZE)
#endif
#if defined(MMAP_BOOTROM_PATCH_MIRROR_SIZE)
  MMAP_ADD_MEMORYMAP_ENTRY(BOOTROM_PATCH_MIRROR, MMAP_BOOTROM_PATCH_MIRROR_BASE, MMAP_BOOTROM_PATCH_MIRROR_SIZE)
#endif
  MMAP_ADD_MEMORYMAP_ENTRY(SLB_IMAGE, MMAP_SLB_BASE, MMAP_SLB_SIZE)
#else /* NOR/PCM? */
  MMAP_ADD_MEMORYMAP_ENTRY(PSI_IMAGE, MMAP_SLB_BASE, MMAP_SLB_SIZE)
  #if defined(FEAT_SLB_IN_PCM)
    MMAP_ADD_MEMORYMAP_ENTRY(SLB_IMAGE, MMAP_SLB_BASE + (MMAP_SLB_SIZE >> 1), (MMAP_SLB_SIZE >> 1))
  #endif
#endif
