/*  -------------------------------------------------------------------------
    Copyright (C) 2014 Intel Mobile Communications GmbH
    
         Sec Class: Intel Confidential (IC)
    
    All rights reserved.
    -------------------------------------------------------------------------
    This document contains proprietary information belonging to IMC.
    Passing on and copying of this document, use and communication of its
    contents is not permitted without prior written authorization.
    -------------------------------------------------------------------------
    Revision Information:
       $File name:  /mhw_drv_src/memory_management/mem/scatterfile/tools/prggen/v1/mem_region_cfg.h $
    ------------------------------------------------------------------------- */

#if defined(MM_CUST_SCF_SUPPORT) 
#include "modem_cust_internal.h"
#endif

#define MAX_PARTITION_PER_REGION 8

typedef struct {
  U32 nand_id[MAX_PARTITION_PER_REGION];
  // the below ones are calculated automatically in PRGGEN
  U32 reservoir_p; /* Percentage for reservoir of used blocks*/
  U32 reservoir_c; /* Fixed blocks for reservoir */
  U32 Size;
  U32 StartBlock;
  U32 EndBlock;
  U32 StartReservoir;
  U32 EndReservoir;
  U32 NextFreeBlock;
}T_REGION;

#if defined(FEAT_NAND_PRGGEN)
static T_REGION region[] = {
#if defined(MM_CUST_SCF_SUPPORT)	
  /* REGION_PSI */
  { 
    { PARTITION_ID_PSI_IMAGE
    }, 
    0, PSI_RESERVIOR_BLOCKS
  },
  /* REGION_SLB */
  {   
    { PARTITION_ID_SLB_IMAGE,
      PARTITION_ID_CERTIFICATE_DATA, 
    }, 
    0, SLB_RESERVIOR_BLOCKS
  },
  /* REGION_RO  */	
  {
    {  PARTITION_ID_PARTITION_TABLE
      ,PARTITION_ID_CODE_IMAGE
      ,PARTITION_ID_PAGING_IMAGE
      ,PARTITION_ID_DSP_IMAGE
#if defined(FEAT_OPM_MEM_SECTION_FOTA)
      ,PARTITION_ID_UA_IMAGE
#endif /* FEAT_OPM_MEM_SECTION_FOTA */
      ,PARTITION_ID_PPM
      ,PARTITION_ID_UPDAPP_IMAGE
      ,PARTITION_ID_UPDAPP_IMAGE_2      
      ,PARTITION_ID_DYNSW
    }, 
    0, RO_RESERVIOR_BLOCKS 
  }, 
   /* REGION_RW */  
  {
    { PARTITION_ID_ROOT_DISC,
    }, 
    0, RW_RESERVIOR_BLOCKS 
  },
  /* REGION_FTL */
  {
    { PARTITION_ID_FTL_STARTUP_DISC,
    }, 
    0, FTL_RESERVIOR_BLOCKS
  },  
  /* REGION_BAD_BLOCK */	
  {
    { PARTITION_ID_BAD_BLOCK_TABLE 
    }, 
    0, BB_RESERVIOR_BLOCKS
  },      
  /* REGION_NVM */  
  {
    {  PARTITION_ID_NVM_STATIC_CALIB
      ,PARTITION_ID_NVM_STATIC_FIX
      ,PARTITION_ID_NVM_DYNAMIC
#if defined(MM_CUST_SCF_SUPPORT)
      ,PARTITION_ID_PMM_PLAIN
      ,PARTITION_ID_PMM_PROT
      ,PARTITION_ID_SUSD
#endif	      
    }, 
    0, NVM_RESERVIOR_BLOCKS 
  },

#else
  /* REGION_PSI */
  { 
    { PARTITION_ID_PSI_IMAGE
    }, 
    0, 0
  },  
    /* REGION_SLB */
  {   
    { PARTITION_ID_SLB_IMAGE
    }, 
    0, 1
  },
  /* REGION_CERT_RO */
  {   
    { PARTITION_ID_CERTIFICATE_DATA, 
      PARTITION_ID_PARTITION_TABLE
    }, 
    0, 3
  },
  /* REGION_RO  */	
  {
    {  PARTITION_ID_CODE_IMAGE
      ,PARTITION_ID_PAGING_IMAGE
      ,PARTITION_ID_DSP_IMAGE
#if defined(FEAT_OPM_MEM_SECTION_FOTA)
      ,PARTITION_ID_UA_IMAGE
#endif /* FEAT_OPM_MEM_SECTION_FOTA */
#if defined(FEAT_VMM_SUPPORT)
      ,PARTITION_ID_VLX
      ,PARTITION_ID_LINUX_KERNEL
      ,PARTITION_ID_INITRD
      ,PARTITION_ID_INITRD_RECOVERY
#endif
    }, 
    0, 17 
  },  
  /* REGION_RW */  
  {
    { PARTITION_ID_ROOT_DISC, 
      PARTITION_ID_USER_DISC,
      PARTITION_ID_CUSTOMIZATION_DISC
    }, 
    0, 29 
  },
  /* REGION_NVM */  
  {
    {  PARTITION_ID_NVM_STATIC_CALIB
      ,PARTITION_ID_NVM_STATIC_FIX
      ,PARTITION_ID_NVM_DYNAMIC
    }, 
    0, 5 
  },
  /* REGION_FTL */
  {
    { PARTITION_ID_FTL_STARTUP_DISC,
    }, 
    0, 3
  },  
  /* REGION_BAD_BLOCK */	
  {
    { PARTITION_ID_BAD_BLOCK_TABLE
    }, 
    0, 1
  }, 
#endif  
  /* REGION_DUMMY of 0 size - Don't add any region after this*/	
  {
    { PARTITION_ID_DYN_SDS_IMAGE,
      PARTITION_ID_PREFLASH,
	  PARTITION_ID_RAW_NAND_IMAGE
    }, 
    0, 0
  },     
};
#endif

#if defined(FEAT_FTLE_PRGGEN) && defined(MM_CUST_SCF_SUPPORT)
static T_REGION region[] = {
   /* REGION_PSI */
  { 
    { PARTITION_ID_PSI_IMAGE
    }, 
    0, 0
  },
  /* REGION_SLB */
  {   
    { PARTITION_ID_SLB_IMAGE,
    }, 
    0, 0
  },
   /* REGION_CERTIFICATE_DATA */
  { 
    { PARTITION_ID_CERTIFICATE_DATA
    }, 
    0, 0
  },
  
  { 
    { PARTITION_ID_PARTITION_TABLE
    }, 
    0, 0
  },
  { 
    { PARTITION_ID_CODE_IMAGE
    }, 
    0, 0
  },
  { 
    { PARTITION_ID_PAGING_IMAGE
    }, 
    0, 0
  },
  { 
    { PARTITION_ID_DSP_IMAGE
    }, 
    0, 0
  },
  #if defined(FEAT_OPM_MEM_SECTION_FOTA)
  { 
    { PARTITION_ID_UA_IMAGE
    }, 
    0, 0
  },
  #endif  
  { 
    { PARTITION_ID_PPM
    }, 
    0, 0
  },
  { 
    { PARTITION_ID_UPDAPP_IMAGE
    }, 
    0, 0
  },   
  { 
    { PARTITION_ID_UPDAPP_IMAGE_2
    }, 
    0, 0
  },
  { 
    { PARTITION_ID_DYNSW
    }, 
    0, 0
  }, 
  { 
    { PARTITION_ID_SUSD
    }, 
    0, 0
  },   
   /* REGION_RW */  
  {
    { PARTITION_ID_ROOT_DISC,
    }, 
    0, 0 
  },  
  {
    { PARTITION_ID_NVM_STATIC_CALIB, 
    }, 
    0, 0 
  },  
  {
    { PARTITION_ID_NVM_STATIC_FIX, 
    }, 
    0, 0 
  },  
  {
    { PARTITION_ID_NVM_DYNAMIC, 
    }, 
    0, 0 
  },
  {
    { PARTITION_ID_PMM_PLAIN, 
    }, 
    0, 0 
  },
    {
    { PARTITION_ID_PMM_PROT, 
    }, 
    0, 0 
  },         
};
#endif

