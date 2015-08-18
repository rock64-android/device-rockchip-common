/* ----------------------------------------------------------------------------

   Copyright (C) 2011-2014 Intel Mobile Communications GmbH

        Sec Class: Intel Confidential (IC)

   All rights reserved.
   ----------------------------------------------------------------------------

   This document contains proprietary information belonging to IMC.
   Technologies Denmark A/S. Passing on and copying of this document, use
   and communication of its contents is not permitted without prior written
   authorisation.

   Revision Information:
  $File name:  /mhw_drv_src/nv_memory_driver/ftle/src/ftle_cfg.h $
   Responsible: gjoerup
   Comment:
     Update for UTA OS.
  ---------------------------------------------------------------------------*/

#ifndef FTLE_CFG_H
#define FTLE_CFG_H

//TestInterface buffer size, increased to 4K because of paging
#define FTLE_TESTBUFFER_SIZE           (512*8)

//Paging size in bytes
#define FTLE_PAGING_SIZE               (4*1024)

#if defined(DWD_MODULE_HOST_TEST)
  #define FTLE_TMP_DATA_BUFFER_LENGTH   (512*256*8)
#else
  #define FTLE_TMP_DATA_BUFFER_LENGTH   (512*4)
#endif

#if defined(DEMAND_PAGING_ENABLED)
  #define FTLE_PAGING
#endif

//UTA INTERFACES
//IF FTLE_UTA_FLASH_BLOCK are enabled (from makefile), UTA are able to R/W using block interface.
//#define FTLE_UTA_FLASH_BLOCK
#if defined(FTLE_UTA_FLASH_IF_PRESENT)
//IF FTLE_UTA_FLASH_BYTE are enabled, UTA are able to R/W using byte interface.
#define FTLE_UTA_FLASH_BYTE
//IF FTLE_UTA_FLASH_INFO are enabled, it is possible to extract eMMC device details (serial no, flash size, etc.)
#define FTLE_UTA_FLASH_INFO
#endif

#if !defined(FTLE_MAC_PRESENT)
//If FTLE_ACCESS_CONTROL are enabled FTLE blocks multiple requests to ensure that one are handled at a time.
#define FTLE_ACCESS_CONTROL
// Use mutex for FTLE access control.
//#define FTLE_ACCESS_CONTROL_USE_MUTEX
#endif
//If FTLE_MAC_PREFLASH is enabled the CLONE feature will be included in the build
//#define FTLE_MAC_CLONE
//If FTLE_MAC_PREFLASH is enabled the PREFLASH feature will be included in the build
//#define FTLE_MAC_PREFLASH

//If FTLE_PERFORMANCE_TESTING is enabled, FTLe is able to measure read/write/erase performance on eMMC hardware.
//#define FTLE_PERFORMANCE_TESTING -NOT IMPEMENTED YET

#endif //FTLE_CFG_H
