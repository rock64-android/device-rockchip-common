// BEGIN CCCT SECTION
/* -------------------------------------------------------------------------
 Copyright (C) 2013 Intel Mobile Communications GmbH
 
      Sec Class: Intel Confidential (IC)
 ----------------------------------------------------------------------
 Revision Information:
    $File name:  /mhw_drv_src/memory_management/mem/scatterfile/tools/prggen/prggen_storage_emmc.c $
 ----------------------------------------------------------------------
 by CCCT (0.12e)
 ---------------------------------------------------------------------- */
// END CCCT SECTION

// prggen_storage_emmc.c

/* remove BOOT_PARTITION_SIZE_IN_BLOCKS and migrate to EMMC_BOOT0_PARTITION_SIZE */
  #define EMMC_BOOT0_PARTITION_SIZE   (BOOT_PARTITION_SIZE_IN_BLOCKS * FTLE_DATA_SIZE_IN_BYTES)
/* Load map alignment is 1024(0x400) which assists in faster flashing */
  #define EMMC_LOADMAP_ALIGN           1024

/****************************************************************
 * prggen_ftl_partition_init
 *  This function shall initialize the details needed for updating
 * the partition information, such as regions, etc.
 *
 * Rules:
 ****************************************************************/
void prggen_ftl_partition_init(PartitionStructType *p_partition)
{
  U32 i, r, p;

  p_partition->PartitionMarker = PARTITION_MARKER;

  /* Calculate the region size information */

  /* for each memory map entry */
  for (i = 0; i < MAX_MAP_ENTRIES; ++i)
  {
    /* no more valid entries, abort */
    if (g_p_mmap[i].Class == NOT_USED)
      break;
    /* no partition associated with this entry, skip */
    if (g_image_type[i] == PARTITION_ID_EMPTY ||
        g_image_type[i] == PARTITION_ID_PSI_RAM ||
        g_image_type[i] == PARTITION_ID_EBL)
    {
      continue;
    }

  } /* for i */
}

#if defined(FEAT_FLASH_WRITE_PROTECT_ENABLED)
  static U32 WP_alignment = 0;
#endif
/****************************************************************
 * prggen_ftl_partition_update
 *  This function shall initialize the details needed for updating
 * the partition information, such as regions, etc.
 *
 * Rules:
 ****************************************************************/
void prggen_ftl_partition_update(void *p, U32 blk_size)
{
  PartitionEntryStructType *p_part_entry = (PartitionEntryStructType *)p;
  static U32 PSI_lastblock = 0;
  static U32 BP_lastblock = 0;
  static U32 BCB_start_block = 0;
  U32 WP_start_alignment = 0;
  static PartitionEntryStructType *p_prev_part_entry = NULL;
  static U32 last_block[3 /* IT */] = {0x22 /* Image Type = 0, user partition */, /* Size of Primary GPT */
                                0     /* Image Type = 1, boot partition 1 */,
                                0     /* Image Type = 2, boot partition 2 */};
  p_part_entry->MaxBlocksInSubPartition = g_partition_info[p_part_entry->ID].d.u_attribute.attribute;

/* SEC Pack details are included in VRL partition */
#if !defined(VRL_PARTITION_ENABLED)
  if (p_part_entry->Options & PARTITION_OPT_LOADMAP)
  {
    blk_size += ALIGN_2K(sizeof(SecurityStructType)); 
  }
#endif

  /* any special handling needed? */
  switch (p_part_entry->ID)
  {
#if defined(EMMC_BOOT0_PARTITION_SIZE)
    case PARTITION_ID_PARTITION_TABLE:
            /* IMC partition table needs to at the end of boot partition-0 */
      p_part_entry->StartBlock  = NO_OF_BLOCKS(EMMC_BOOT0_PARTITION_SIZE) - NO_OF_BLOCKS(blk_size);
	  //p_part_entry->StartBlock  = 0xFFC;
      break;
    #if defined(FEAT_OPM_MEM_GPT_SUPPORT)
    case PARTITION_ID_PRIMARY_GPT:
      p_part_entry->StartBlock  = 0;
      break;

    case PARTITION_ID_SECONDARY_GPT:
      /* Secondary GPT needs to be at the end of the FLASH */
      p_part_entry->StartBlock  = MEM_PRG_USABLE_FLASH_SIZE - NO_OF_BLOCKS(blk_size);
      break;
    #endif

    case PARTITION_ID_NVM_STATIC_CALIB:
      p_part_entry->StartBlock  = 0x4000;
      break;
#else
    /* fallback partition table location */
    case PARTITION_ID_PARTITION_TABLE:
      p_part_entry->StartBlock  = 0;
      break;
#endif  /* EMMC_BOOT0_PARTITION_SIZE */


    case PARTITION_ID_BCB:
#if !defined(VMM_MOBILEVISOR)
      p_part_entry->StartBlock  = 0x22;
#else
      p_part_entry->StartBlock  = BCB_start_block;
#endif
      break;

    case PARTITION_ID_PSI_MIRROR:
        p_part_entry->StartBlock  = 0;
	break;

    default:
      p_part_entry->StartBlock  = last_block[p_part_entry->ImageType];    
      break;
  }
  

  if (((0 == p_part_entry->ImageType) && (p_part_entry->Options & PARTITION_OPT_LOADMAP)) || (p_part_entry->ID == PARTITION_ID_SYSTEM)|| (p_part_entry->ID == PARTITION_ID_CACHE) || (p_part_entry->ID == PARTITION_ID_USERDATA) || (p_part_entry->ID == PARTITION_ID_USER_DISC_EXTEND2) || (p_part_entry->ID == PARTITION_ID_USER_DISC_EXTEND3) || (p_part_entry->ID == PARTITION_ID_USER_DISC_EXTEND4)) 
  {
      /* Aligning the partitions which has loadmap bit set by 1024(0x400) block size and -4(2k) is for the secure pack */
#if defined(VRL_PARTITION_ENABLED)
     if (p_part_entry->StartBlock % EMMC_LOADMAP_ALIGN)
          p_part_entry->StartBlock  = p_part_entry->StartBlock + (EMMC_LOADMAP_ALIGN - (p_part_entry->StartBlock % EMMC_LOADMAP_ALIGN)); 
#else
          p_part_entry->StartBlock  = p_part_entry->StartBlock + (EMMC_LOADMAP_ALIGN - (p_part_entry->StartBlock % EMMC_LOADMAP_ALIGN)) - 4; 
#endif
  } 

#if defined(FEAT_FLASH_WRITE_PROTECT_ENABLED) 
  if (p_part_entry->ID == PARTITION_ID_BCB)
  ;
  else if (WP_alignment == 0 && !(p_part_entry->MaxBlocksInSubPartition & PARTITION_ATTR_WRITE_PROTECT_GROUP_MASK)) 
  {
     /* Aligning the first non-WP partition by 8MB(0x4000) */
     WP_alignment = 1;
     WP_start_alignment = 1;
  }
  else if (WP_alignment == 1 && (p_part_entry->MaxBlocksInSubPartition & PARTITION_ATTR_WRITE_PROTECT_GROUP_MASK)) 
  {
     /* Aligning the first WP partition by 8MB(0x4000) */
     WP_alignment = 0;
     WP_start_alignment = 1;
  }
  else if (p_part_entry->MaxBlocksInSubPartition & PARTITION_ATTR_WRITE_PROTECT_GROUP_MASK) 
  {
    /* Pack the blocks without any gap inside the Write protection chunks */
     if (p_prev_part_entry && (p_prev_part_entry->ImageType == p_part_entry->ImageType))
        if ((p_prev_part_entry->ID != PARTITION_ID_PRIMARY_GPT) && (p_prev_part_entry->ID != PARTITION_ID_PARTITION_TABLE) &&  
            (p_prev_part_entry->ID != PARTITION_ID_SLB_IMAGE) && (p_part_entry->ID != PARTITION_ID_BCB))
                p_prev_part_entry->EndBlock = p_part_entry->StartBlock;
  }

  if (WP_start_alignment && (p_part_entry->ID != PARTITION_ID_BCB) && (p_part_entry->ID != PARTITION_ID_PARTITION_TABLE))
  {
     WP_start_alignment = 0;
     if (p_part_entry->StartBlock % EMMC_WP_BLOCK_ALIGN_SIZE)
         p_part_entry->StartBlock = ALIGN_BLOCK_FOR_WP(p_part_entry->StartBlock);
     if (p_prev_part_entry)
         p_prev_part_entry->EndBlock = p_part_entry->StartBlock;
  }
#endif

  p_prev_part_entry = p_part_entry;
  p_part_entry->EndBlock    = p_part_entry->StartBlock + NO_OF_BLOCKS(blk_size);
 
#if defined(VMM_MOBILEVISOR)
  if (p_part_entry->ID == BCB_PREVIOUS_BLOCK_PARTITION_ID) {
      BCB_start_block = p_part_entry->EndBlock;
  }
#endif

  /* warnings if any */
//  if (p_part_entry->ID == PARTITION_ID_PSI_IMAGE &&
//      p_part_entry->StartBlock != 0)
//  {
//    printf("Warning: PSI doesn't start at block-0\n");
//    printf("         Check region configuration.\n");
//  }

  if ((p_part_entry->ID != PARTITION_ID_PRIMARY_GPT) &&
      (p_part_entry->ID != PARTITION_ID_SECONDARY_GPT) &&
      (p_part_entry->ID != PARTITION_ID_PARTITION_TABLE) 
#if defined(VMM_MOBILEVISOR)
      && (p_part_entry->ID != BCB_PREVIOUS_BLOCK_PARTITION_ID)&&
      (p_part_entry->ID != PARTITION_ID_BCB)
#endif
      )
  {
    last_block[p_part_entry->ImageType] = p_part_entry->EndBlock;
  }
#if defined(VMM_MOBILEVISOR)
  else if (p_part_entry->ID == BCB_PREVIOUS_BLOCK_PARTITION_ID) {
    last_block[p_part_entry->ImageType] = p_part_entry->EndBlock + NO_OF_BLOCKS(MMAP_BCB_SIZE);
  }
#endif
}

/****************************************************************
 * prggen_ftl_sanity
 *  This function shall perform the sanity tests on the assumptions it
 * has on memory-map, partition information, such as attributes, options, etc.
 *
 ****************************************************************/
void prggen_ftl_sanity(void)
{
  PERFORM_SANITY_OPT(VERIFICATION_REQUIRED, TRUE, PARTITION_OPT_VERIFICATION_REQUIRED);
  PERFORM_SANITY_OPT(READONLYIMAGE, TRUE, PARTITION_OPT_READONLYIMAGE);
  PERFORM_SANITY_OPT(COPYTORAM, TRUE, PARTITION_OPT_COPYTORAM);
  PERFORM_SANITY_OPT(WIPE_OUT_DATA, TRUE, PARTITION_OPT_WIPE_OUT_DATA);
  PERFORM_SANITY_OPT(LOADMAP, TRUE, PARTITION_OPT_LOADMAP);
  PERFORM_SANITY_OPT(SHORT_TIMEOUT, TRUE, PARTITION_OPT_SHORT_TIMEOUT);
  PERFORM_SANITY_OPT(PAGING_DATA, TRUE, PARTITION_OPT_PAGING_DATA);
  PERFORM_SANITY_OPT(RO_BIT_CORRECTION_OK, TRUE, PARTITION_OPT_RO_BIT_CORRECTION_OK);
  PERFORM_SANITY_OPT(ACCEPTABLE_BIT_ERRORS, 0x7, PARTITION_OPT_ACCEPTABLE_BIT_ERRORS);
  PERFORM_SANITY_OPT(TEST_ACCEPTED, TRUE, PARTITION_OPT_TEST_ACCEPTED);
  PERFORM_SANITY_OPT(ALLOW_UTA_BLOCK_IF_ACCESS, TRUE, PARTITION_OPT_ALLOW_UTA_BLOCK_IF_ACCESS);
  PERFORM_SANITY_OPT(ALLOW_UTA_BYTE_IF_ACCESS, TRUE, PARTITION_OPT_ALLOW_UTA_BYTE_IF_ACCESS);
  PERFORM_SANITY_OPT(FS_DEVICE_ACCESS_INTERNAL, TRUE, PARTITION_OPT_FS_DEVICE_ACCESS_INTERNAL);
  PERFORM_SANITY_OPT(FS_DEVICE_ACCESS_EXTERNAL, TRUE, PARTITION_OPT_FS_DEVICE_ACCESS_EXTERNAL);
  PERFORM_SANITY_OPT(FS_DEVICE_PLP_DISABLE, TRUE, PARTITION_OPT_FS_DEVICE_PLP_DISABLE);
  PERFORM_SANITY_OPT(FS_DEVICE_PLP_RUN_CHECK_DISK, TRUE, PARTITION_OPT_FS_DEVICE_PLP_RUN_CHECK_DISK);
  PERFORM_SANITY_OPT(FS_DEVICE_PLP_ENABLE, TRUE, PARTITION_OPT_FS_DEVICE_PLP_ENABLE);
  PERFORM_SANITY_OPT(FS_DEVICE_SYSTEM_DISK, TRUE, PARTITION_OPT_FS_DEVICE_SYSTEM_DISK);
  PERFORM_SANITY_OPT(FS_DEVICE_NOT_SYSTEM_DISK, TRUE, PARTITION_OPT_FS_DEVICE_NOT_SYSTEM_DISK);
  PERFORM_SANITY_OPT(FS_PARTITION_REGISTER, TRUE, PARTITION_OPT_FS_PARTITION_REGISTER);
  PERFORM_SANITY_OPT(ALIGNMENT, 0xF, PARTITION_OPT_ALIGNMENT_MASK);

  PERFORM_SANITY_ATTRIBUTE(GPT_PARTITION, TRUE, PARTITION_ATTR_GPT_PARTITION);
  PERFORM_SANITY_ATTRIBUTE(GPT_SYSTEM_PARTITION, TRUE, PARTITION_ATTR_GPT_SYSTEM_PARTITION);
  PERFORM_SANITY_ATTRIBUTE(GPT_LEGACY_BIOS_BOOTABLE, TRUE, PARTITION_ATTR_GPT_LEGACY_BIOS_BOOTABLE);
  PERFORM_SANITY_ATTRIBUTE(GPT_READ_ONLY, TRUE, PARTITION_ATTR_GPT_READ_ONLY);
  PERFORM_SANITY_ATTRIBUTE(GPT_HIDDEN, TRUE, PARTITION_ATTR_GPT_HIDDEN);
  PERFORM_SANITY_ATTRIBUTE(GPT_DO_NOT_AUTOMOUNT, TRUE, PARTITION_ATTR_GPT_DO_NOT_AUTOMOUNT);
  PERFORM_SANITY_ATTRIBUTE(GPT_TYPE_GUID_a, TRUE, PARTITION_ATTR_GPT_TYPE_GUID_a);
  PERFORM_SANITY_ATTRIBUTE(GPT_TYPE_GUID_b, TRUE, PARTITION_ATTR_GPT_TYPE_GUID_b);
  PERFORM_SANITY_ATTRIBUTE(GPT_TYPE_GUID_c, TRUE, PARTITION_ATTR_GPT_TYPE_GUID_c);
  PERFORM_SANITY_ATTRIBUTE(POWER_ON_WP_NORMAL_MODE, TRUE, PARTITION_ATTR_POWER_ON_WP_NORMAL_MODE);
  PERFORM_SANITY_ATTRIBUTE(POWER_ON_WP_UPDATE_MODE, TRUE, PARTITION_ATTR_POWER_ON_WP_UPDATE_MODE);
  PERFORM_SANITY_ATTRIBUTE(POWER_ON_WP_PRODUCTION_MODE, TRUE, PARTITION_ATTR_POWER_ON_WP_PRODUCTION_MODE);
  PERFORM_SANITY_ATTRIBUTE(POWER_ON_WP_SPARE_MODE, TRUE, PARTITION_ATTR_POWER_ON_WP_SPARE_MODE);
}
