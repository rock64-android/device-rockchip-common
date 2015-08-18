/*  -------------------------------------------------------------------------
    Copyright (C) 2014 Intel Mobile Communications GmbH
    
         Sec Class: Intel Confidential (IC)
    
     ----------------------------------------------------------------------
     Revision Information:
       $$File name:  /mhw_drv_src/memory_management/mem/scatterfile/tools/prggen/mem_region_block_cfg_default.h $
     ---------------------------------------------------------------------- */

/* This file is used for NAND Partition calculations
 * Reservoir extra blocks(reservoir_extra_blk), Region extra blocks(region_extra_blk), Reservoir percent(reservoir_p) 
 * and Reservoir blocks(reservoir_c) are defined in this file
 * Reservoir extra blocks and Reservoir percent are used for calculation of total reservoir blocks using the below formula :
 * Reservoir block =  (2*number_of_regions_using_this_reservoir) + (Reservoir percent * Region size) +  Reservoir extra blocks
 * Region extra blocks are used to add some extra blocks to each region if needed (SLB uses this as if now)
 * Reservoir blocks are used directly hard code the reservoir blocks for each region without any calculation
 */

#if !defined __RESERVOIR_AND_EXTRA_BLOCK_CFG__

#define PSI_RESERVOIR_EXTRA_BLOCKS          0
#define PSI_RESERVIOR_PERCENT               0
#define PSI_RESERVIOR_BLOCKS                0

#define SLB_RESERVOIR_EXTRA_BLOCKS          0
#define SLB_RESERVIOR_PERCENT               0
#define SLB_RESERVIOR_BLOCKS                1
#define SLB_REGION_EXTRA_BLOCK              0

#define RO_RESERVOIR_EXTRA_BLOCKS           0
#define RO_RESERVIOR_PERCENT                0
#define RO_RESERVIOR_BLOCKS                17

#define RW_RESERVOIR_EXTRA_BLOCKS           0
#define RW_RESERVIOR_PERCENT                0
#define RW_RESERVIOR_BLOCKS                29

#define NVM_RESERVOIR_EXTRA_BLOCKS          0
#define NVM_RESERVIOR_PERCENT               0
#define NVM_RESERVIOR_BLOCKS                5

#define FTL_RESERVOIR_EXTRA_BLOCKS          0
#define FTL_RESERVIOR_PERCENT               0
#define FTL_RESERVIOR_BLOCKS                3

#define BB_RESERVOIR_EXTRA_BLOCKS           0
#define BB_RESERVIOR_PERCENT                0
#define BB_RESERVIOR_BLOCKS                 1

#define __RESERVOIR_AND_EXTRA_BLOCK_CFG__
#endif
