// BEGIN CCCT SECTION
/* -------------------------------------------------------------------------
 Copyright (C) 2013 Intel Mobile Communications GmbH
 
      Sec Class: Intel Confidential (IC)
 ----------------------------------------------------------------------
 Revision Information:
    $File name:  /mhw_drv_src/memory_management/mem/scatterfile/tools/prggen/category/cnokiac/mem_nokia_description_memorymap.h $
 ----------------------------------------------------------------------
 by CCCT (0.12e)
 ---------------------------------------------------------------------- */
// END CCCT SECTION

// mem_description_memoymap.h

#if defined(MMAP_INCLUDE_MEMORYMAP_INFO)
// refer: FlsFile.h
MMAP_UPDATE_MEMORYMAP_INFO_CLASS(PMM_PLAIN,     CUST_CLASS)
MMAP_UPDATE_MEMORYMAP_INFO_CLASS(PMM_PROT,      CUST_CLASS)
MMAP_UPDATE_MEMORYMAP_INFO_CLASS(SUSD,          CUST_CLASS)
MMAP_UPDATE_MEMORYMAP_INFO_CLASS(PPM,           CUST_CLASS)
MMAP_UPDATE_MEMORYMAP_INFO_CLASS(UPDAPP_IMAGE,  CODE_CLASS)
MMAP_UPDATE_MEMORYMAP_INFO_CLASS(DYNSW,         CUST_CLASS)
#endif  /* MMAP_INCLUDE_MEMORYMAP_INFO */
